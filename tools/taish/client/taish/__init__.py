#
# Copyright (C) 2018-2019 Nippon Telegraph and Telephone Corporation.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.

import grpc

from taish import taish_pb2
from taish import taish_pb2_grpc

class TAIException(Exception):
    def __init__(self, code, msg):
        self.code = code
        self.msg = msg

def check_call(call):
    code = None
    for key, value in call.trailing_metadata():
        if key == 'tai-status-code':
            code = int(value)
        elif key == 'tai-status-msg':
            msg = value
    if code:
        raise TAIException(code, msg)

class TAIObject(object):
    def __init__(self, client, object_type, obj):
        self.client = client
        self.object_type = object_type
        self.obj = obj

    @property
    def oid(self):
        return self.obj.oid

    def list_attribute_metadata(self):
        return self.client.list_attribute_metadata(self.object_type)

    def get_attribute_metadata(self, attr):
        return self.client.get_attribute_metadata(self.object_type, attr)

    def set(self, attr_id, value):
        return self.client.set(self.object_type, self.oid, attr_id, value)

    def get(self, attr_id, with_metadata=False):
        return self.client.get(self.object_type, self.oid, attr_id, with_metadata)

    def monitor(self, attr_id):
        return self.client.monitor(self.object_type, self.oid, attr_id)


class NetIf(TAIObject):
    def __init__(self, client, obj):
        super(NetIf, self).__init__(client, taish_pb2.NETIF, obj)

class HostIf(TAIObject):
    def __init__(self, client, obj):
        super(HostIf, self).__init__(client, taish_pb2.HOSTIF, obj)

class Module(TAIObject):
    def __init__(self, client, obj):
        super(Module, self).__init__(client, taish_pb2.MODULE, obj)

    def get_netif(self, index=0):
        return NetIf(self.client, self.obj.netifs[index])

    def get_hostif(self, index=0):
        return HostIf(self.client, self.obj.hostifs[index])

    def create_netif(self, index=0, attrs=[]):
        attrs.append(("index", index))
        self.client.create(taish_pb2.NETIF, attrs, self.oid)
        self.obj = self.client.list()[self.obj.location]
        return self.get_netif(index)

    def create_hostif(self, index=0, attrs=[]):
        attrs.append(("index", index))
        self.client.create(taish_pb2.HOSTIF, attrs, self.oid)
        self.obj = self.client.list()[self.obj.location]
        return self.get_hostif(index)


class Client(object):
    def __init__(self, address='localhost', port='50051'):
        self.channel = grpc.insecure_channel('{}:{}'.format(address, port))
        self.stub = taish_pb2_grpc.TAIStub(self.channel)

    def close(self):
        self.channel.close()

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self.close()

    def list(self):
        req = taish_pb2.ListModuleRequest()
        mods = { res.module.location: res.module for res in self.stub.ListModule(req) }
        return mods

    def list_attribute_metadata(self, object_type):
        req = taish_pb2.ListAttributeMetadataRequest()
        req.object_type = object_type
        return [res.metadata for res in self.stub.ListAttributeMetadata(req)]

    def get_attribute_metadata(self, object_type, attr):
        req = taish_pb2.GetAttributeMetadataRequest()
        req.object_type = object_type
        if type(attr) == int:
            req.attr_id = attr
        elif type(attr) == str:
            req.attr_name = attr
        else:
            raise Exception("invalid argument")
        res, call = self.stub.GetAttributeMetadata.with_call(req)
        check_call(call)
        return res.metadata

    def get_module(self, location):
        l = self.list()
        if location not in l:
            raise Exception("no module {} found".format(location))
        m = l[location]
        if not m.present:
            raise Exception("module {} not present".format(location))
        if not m.oid:
            raise Exception("module {} not created yet".format(location))
        return Module(self, m)

    def create_module(self, location, attrs=[]):
        attrs.append(("location", location))
        self.create(taish_pb2.MODULE, attrs)
        return self.get_module(location)

    def create(self, object_type, attrs, module_id=0):
        if type(object_type) == str:
            if object_type == 'module':
                object_type = taish_pb2.MODULE
            elif object_type == 'netif':
                object_type = taish_pb2.NETIF
            elif object_type == 'hostif':
                object_type = taish_pb2.HOSTIF
        req = taish_pb2.CreateRequest()
        req.object_type = object_type
        req.module_id = module_id
        for attr in attrs:
            attr_id, value = attr
            meta = self.get_attribute_metadata(object_type, attr_id)
            attr_id = meta.attr_id
            a = taish_pb2.Attribute()
            a.attr_id = attr_id
            a.value = str(value)
            req.attrs.append(a)
        _, call = self.stub.Create.with_call(req)
        check_call(call)

    def remove(self, oid):
        req = taish_pb2.RemoveRequest()
        req.oid = oid
        _, call = self.stub.Remove.with_call(req)
        check_call(call)

    def set(self, object_type, oid, attr_id, value):
        if type(attr_id) == int:
            pass
        elif type(attr_id) == str:
            metadata = self.get_attribute_metadata(object_type, attr_id)
            attr_id = metadata.attr_id
        else:
            attr_id = attr_id.attr_id

        req = taish_pb2.SetAttributeRequest()
        req.oid = oid
        req.attribute.attr_id = attr_id
        req.attribute.value = str(value)
        _, call = self.stub.SetAttribute.with_call(req)
        check_call(call)

    def get(self, object_type, oid, attr, with_metadata=False):
        if type(attr) == int:
            attr_id = attr
            if with_metadata:
                meta = self.get_attribute_metadata(object_type, attr_id)
        elif type(attr) == str:
            meta = self.get_attribute_metadata(object_type, attr)
            attr_id = meta.attr_id
        else:
            attr_id = attr.attr_id
            meta = attr

        req = taish_pb2.GetAttributeRequest()
        req.oid = oid
        req.attribute.attr_id = attr_id
        res, call = self.stub.GetAttribute.with_call(req)
        check_call(call)
        value = res.attribute.value

        if with_metadata:
            return (value, meta)
        else:
            return value

    def monitor(self, object_type, oid, attr_id):
        m = self.get_attribute_metadata(object_type, attr_id)
        if m.usage != '<notification>':
            raise Exception('the type of attribute {} is not notification'.format(attr_id))


        req = taish_pb2.MonitorRequest()
        req.oid = oid
        req.notification_attr_id = m.attr_id

        l = self.list_attribute_metadata(object_type)

        try:
            for res in self.stub.Monitor(req):
                for attr in res.attrs:
                    a = [ v.short_name for v in l if v.attr_id == attr.attr_id ]
                    if len(a) == 1:
                        print('{} | {}'.format(a[0], attr.value))
                    elif len(a) == 0:
                        print('0x{:x} | {}'.format(attr.attr_id, attr.value))
                    else:
                        print('error: more than one metadata matched for id 0x{:x}: {}'.format(attr.attr_id, a))
        except KeyboardInterrupt:
            pass

    def set_log_level(self, l, api='unspecified'):
        if l == 'debug':
            level = taish_pb2.DEBUG
        elif l == 'info':
            level = taish_pb2.INFO
        elif l == 'notice':
            level = taish_pb2.NOTICE
        elif l == 'warn':
            level = taish_pb2.WARN
        elif l == 'error':
            level = taish_pb2.ERROR
        elif l == 'critical':
            level = taish_pb2.CRITICAL
        else:
            raise Exception('invalid log level: {}. choose from [debug, info, notice, warn, error, critical]'.format(l))
        if api == 'module':
            api = taish_pb2.MODULE_API
        elif api == 'netif':
            api = taish_pb2.NETIF_API
        elif api == 'hostif':
            api = taish_pb2.HOSTIF_API
        elif api == 'unspecified':
            api = taish_pb2.UNSPECIFIED_API
        else:
            raise Exception('invalid api type. choose from [module, netif, hostif, unspecified]')

        req = taish_pb2.SetLogLevelRequest()
        req.level = level
        req.api = api
        return self.stub.SetLogLevel(req)
