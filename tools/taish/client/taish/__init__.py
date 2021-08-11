#
# Copyright (C) 2018-2019 Nippon Telegraph and Telephone Corporation.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.

from grpclib.client import Channel

from taish import taish_pb2
from taish import taish_grpc

import asyncio

from queue import Queue
from threading import Thread

import time

import functools
import inspect
from typing import Any, Optional

DEFAULT_SERVER_ADDRESS = 'localhost'
DEFAULT_SERVER_PORT = '50051'

def is_async_func(func: Any) -> bool:
    if inspect.iscoroutinefunction(func):
        return True
    if isinstance(func, functools.partial):
        return is_async_func(func.func)
    return False

def set_default_serialize_option(req):
    req.serialize_option.human = True
    req.serialize_option.value_only = True
    req.serialize_option.json = False

class TAIException(Exception):
    def __init__(self, code, msg):
        self.code = code
        self.msg = msg

def check_metadata(metadata):
    code = int(metadata.get('tai-status-code', 0))
    if code:
        msg = metadata.get('tai-status-msg', '')
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
        return self.client.list_attribute_metadata(self.object_type, self.oid)

    def get_attribute_metadata(self, attr):
        return self.client.get_attribute_metadata(self.object_type, attr, oid=self.oid)

    def get_attribute_capability(self, attr, json=False):
        return self.client.get_attribute_capability(self.oid, attr, json)

    def set(self, attr_id, value):
        return self.client.set(self.object_type, self.oid, attr_id, value)

    def get(self, attr_id, with_metadata=False, value=None, json=False):
        return self.client.get(self.object_type, self.oid, attr_id, with_metadata, value, json)

    def monitor(self, attr_id, callback, json=False):
        return self.client.monitor(self, attr_id, callback, json)


class NetIf(TAIObject):
    def __init__(self, client, obj):
        super().__init__(client, taish_pb2.NETIF, obj)

class HostIf(TAIObject):
    def __init__(self, client, obj):
        super().__init__(client, taish_pb2.HOSTIF, obj)

class Module(TAIObject):
    def __init__(self, client, obj):
        super().__init__(client, taish_pb2.MODULE, obj)

    def get_netif(self, index=0):
        return NetIf(self.client, self.obj.netifs[index])

    def get_hostif(self, index=0):
        return HostIf(self.client, self.obj.hostifs[index])

    async def create_netif(self, index=0, attrs=None):
        if attrs is None:
            attrs = []
        attrs.append(("index", index))
        await self.client.create(taish_pb2.NETIF, attrs, self.oid)
        self.obj = (await self.client.list())[self.obj.location]
        return self.get_netif(index)

    async def create_hostif(self, index=0, attrs=None):
        if attrs is None:
            attrs = []
        attrs.append(("index", index))
        await self.client.create(taish_pb2.HOSTIF, attrs, self.oid)
        self.obj = (await self.client.list())[self.obj.location]
        return self.get_hostif(index)


class AsyncClient(object):
    def __init__(self, address=DEFAULT_SERVER_ADDRESS, port=DEFAULT_SERVER_PORT):
        self.channel = Channel(address, int(port))
        self.stub = taish_grpc.TAIStub(self.channel)

    def close(self):
        self.channel.close()

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self.close()

    async def list(self):
        req = taish_pb2.ListModuleRequest()
        future = await self.stub.ListModule(req)
        return { res.module.location: res.module for res in future }

    async def list_attribute_metadata(self, object_type, oid=0, location=''):
        req = taish_pb2.ListAttributeMetadataRequest()
        req.object_type = object_type
        req.oid = oid
        req.location = location
        return [res.metadata for res in await self.stub.ListAttributeMetadata(req)]

    async def get_attribute_capability(self, oid, attr, json=False):
        async with self.stub.GetAttributeCapability.open() as stream:

            if type(attr) == int:
                attr_id = attr
            elif type(attr) == str:
                meta = await self.get_attribute_metadata(0, attr, oid=oid)
                attr_id = meta.attr_id
            else:
                attr_id = attr.attr_id

            req = taish_pb2.GetAttributeCapabilityRequest()
            set_default_serialize_option(req)
            req.oid = oid
            req.attr_id = attr_id
            req.serialize_option.json = json
            await stream.send_message(req)
            res = await stream.recv_message()
            await stream.recv_trailing_metadata()
            check_metadata(stream.trailing_metadata)
            return res.capability

    async def get_attribute_metadata(self, object_type, attr, oid=0, location=''):
        async with self.stub.GetAttributeMetadata.open() as stream:
            req = taish_pb2.GetAttributeMetadataRequest()
            req.object_type = object_type
            req.oid = oid
            req.location = location
            set_default_serialize_option(req)
            if type(attr) == int:
                req.attr_id = attr
            elif type(attr) == str:
                req.attr_name = attr
            else:
                raise Exception("invalid argument")
            await stream.send_message(req)
            res = await stream.recv_message()
            await stream.recv_trailing_metadata()
            check_metadata(stream.trailing_metadata)
            return res.metadata

    async def get_module(self, location):
        l = await self.list()
        if location not in l:
            raise Exception("no module {} found".format(location))
        m = l[location]
        if not m.present:
            raise Exception("module {} not present".format(location))
        if not m.oid:
            raise Exception("module {} not created yet".format(location))
        return Module(self, m)

    async def create_module(self, location, attrs=None):
        if attrs is None:
            attrs = []
        attrs.append(("location", location))
        await self.create(taish_pb2.MODULE, attrs)
        return await self.get_module(location)

    async def create(self, object_type, attrs, module_id=0):

        if module_id != 0:
            location = await self.get(taish_pb2.MODULE, module_id, 'location')
        else:
            for attr in attrs:
                key, value = attr
                if key == 'location':
                    location = value
                    break
            else:
                raise TAIException(0xe, 'mandatory-attribute-missing')

        async with self.stub.Create.open() as stream:
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
            set_default_serialize_option(req)
            for attr in attrs:
                attr_id, value = attr
                meta = await self.get_attribute_metadata(object_type, attr_id, location=location)
                attr_id = meta.attr_id
                a = taish_pb2.Attribute()
                a.attr_id = attr_id
                a.value = str(value)
                req.attrs.append(a)
            await stream.send_message(req)
            res = await stream.recv_message()
            await stream.recv_trailing_metadata()
            check_metadata(stream.trailing_metadata)
            return res.oid

    async def remove(self, oid):
        async with self.stub.Remove.open() as stream:
            req = taish_pb2.RemoveRequest()
            req.oid = oid
            await stream.send_message(req)
            res = await stream.recv_message()
            await stream.recv_trailing_metadata()
            check_metadata(stream.trailing_metadata)

    async def set(self, object_type, oid, attr_id, value):
        async with self.stub.SetAttribute.open() as stream:
            if type(attr_id) == int:
                pass
            elif type(attr_id) == str:
                metadata = await self.get_attribute_metadata(object_type, attr_id, oid=oid)
                attr_id = metadata.attr_id
            else:
                attr_id = attr_id.attr_id

            req = taish_pb2.SetAttributeRequest()
            req.oid = oid
            req.attribute.attr_id = attr_id
            req.attribute.value = str(value)
            set_default_serialize_option(req)
            await stream.send_message(req)
            res = await stream.recv_message()
            await stream.recv_trailing_metadata()
            check_metadata(stream.trailing_metadata)

    async def get(self, object_type, oid, attr, with_metadata=False, value=None, json=False):
        async with self.stub.GetAttribute.open() as stream:
            if type(attr) == int:
                attr_id = attr
                if with_metadata:
                    meta = await self.get_attribute_metadata(object_type, attr_id, oid=oid)
            elif type(attr) == str:
                meta = await self.get_attribute_metadata(object_type, attr, oid=oid)
                attr_id = meta.attr_id
            else:
                attr_id = attr.attr_id
                meta = attr

            req = taish_pb2.GetAttributeRequest()
            req.oid = oid
            req.attribute.attr_id = attr_id
            set_default_serialize_option(req)
            req.serialize_option.json = json
            if value:
                req.attribute.value = str(value)

            await stream.send_message(req)
            res = await stream.recv_message()
            await stream.recv_trailing_metadata()
            check_metadata(stream.trailing_metadata)
            value = res.attribute.value
            if with_metadata:
                return (value, meta)
            else:
                return value

    async def monitor(self, obj, attr_id, callback, json=False):
        async with self.stub.Monitor.open() as stream:
            m = await self.get_attribute_metadata(obj.object_type, attr_id, oid=obj.oid)
            if m.usage != '<notification>':
                raise Exception('the type of attribute {} is not notification'.format(attr_id))

            req = taish_pb2.MonitorRequest()
            req.oid = obj.oid
            req.notification_attr_id = m.attr_id
            set_default_serialize_option(req)
            req.serialize_option.json = json

            await stream.send_message(req)

            while True:
                if is_async_func(callback):
                    await callback(obj, m, await stream.recv_message())
                else:
                    callback(obj, m, await stream.recv_message())

    async def set_log_level(self, l, api='unspecified'):
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
        await self.stub.SetLogLevel(req)


class Client(object):

    def __init__(self, addr=DEFAULT_SERVER_ADDRESS, port=DEFAULT_SERVER_PORT):
        self.in_q = Queue()
        self.out_q = Queue()
        self.t = Thread(target = self.loop, args=(addr, port))
        self.t.daemon = True
        self.t.start()
        self.addr = addr
        self.port = port

    def loop(self, addr, port):
        async def _loop():
            c = AsyncClient(addr, port)
            while True:
                i = self.in_q.get()
                if i == None:
                    return
                fname, args, kwargs = i
                try:
                    f = getattr(c, fname)
                    ret = await f(*args, **kwargs)
                except Exception as e:
                    ret = e

                if isinstance(ret, TAIObject):
                    ret.client = self

                self.out_q.put(ret)

        asyncio.run(_loop())

    def close(self):
        self.in_q.put(None)
        self.t.join()

    def monitor(self, *args, **kwargs):
        try:
            c = AsyncClient(self.addr, self.port)
            loop = asyncio.get_event_loop()
            task = loop.create_task(c.monitor(*args, **kwargs))
            return loop.run_forever()
        except KeyboardInterrupt:
            task.cancel()

    def __getattr__(self, name):
        def f(*args, **kwargs):
            self.in_q.put((name, args, kwargs))
            ret = self.out_q.get()
            if isinstance(ret, Exception):
                raise ret
            return ret
        f.__name__ = name
        return f
