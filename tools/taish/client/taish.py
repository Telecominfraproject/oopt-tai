#!/usr/bin/env python
#
# Copyright (C) 2018 Nippon Telegraph and Telephone Corporation.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.

import grpc
import sys

import tai_pb2
import tai_pb2_grpc

from optparse import OptionParser

from prompt_toolkit import PromptSession
from prompt_toolkit.completion import FuzzyCompleter, Completer, Completion
from tabulate import tabulate

TAI_ATTR_CUSTOM_RANGE_START = 0x10000000

def show_help(hidden=False):
    d = [['list', 'list detected TAI objects'],
    ['list-attr', 'list attributes of the selected TAI object'],
    ['set <attr-name> <attr-value>', 'set an attribute'],
    ['get <attr-name> [<attr-value>]', 'get an attribute'],
    ['monitor [<attr-name>]', 'monitor state change'],
    ['log-level <level> [<api-type>]', 'set log level']]
    print(tabulate(d, headers=['command', 'description']))

def show_modules(modules):
    for k, m in modules.items():
        stroid = lambda oid : '0x{:08x}'.format(oid)
        print('module:', k, stroid(m.oid))
        for v in m.hostifs:
            print(' hostif:', v.index, stroid(v.oid))

        for v in m.netifs:
            print(' netif:', v.index, stroid(v.oid))

def get_attribute_metadata(stub, module, netif, hostif):
    if not module:
        return []

    object_type = tai_pb2.MODULE
    if netif:
        object_type = tai_pb2.NETIF
    elif hostif:
        object_type = tai_pb2.HOSTIF

    req = tai_pb2.ListAttributeMetadataRequest()
    req.object_type = object_type
    return [res.metadata for res in stub.ListAttributeMetadata(req)]

def list_attr(stub, module, netif, hostif):
    d = []
    for m in get_attribute_metadata(stub, module, netif, hostif):
        d.append([m.short_name, 'ro' if m.is_readonly else 'r/w', m.usage, 'custom' if m.attr_id > TAI_ATTR_CUSTOM_RANGE_START else 'official'])
    print(tabulate(d, headers=['name', 'type', 'value', 'range']))

def monitor(stub, module, netif, hostif, cmds):
    req = tai_pb2.MonitorRequest()
    req.oid = module.oid
    if netif:
        req.oid = netif.oid
    elif hostif:
        req.oid = hostif.oid

    m = get_attribute_metadata(stub, module, netif, hostif)

    nattrname = 'notify' if len(cmds) < 2 else cmds[1]
    n = [ v.attr_id for v in m if v.short_name == nattrname ]
    if len(n) != 1:
        print('error: failed to find metadata of notification attribute {}'.format(nattrname))

    req.notification_attr_id = n[0]

    try:
        for res in stub.Monitor(req):
            for attr in res.attrs:
                a = [ v.short_name for v in m if v.attr_id == attr.attr_id ]
                if len(a) == 1:
                    print('{} | {}'.format(a[0], attr.value))
                elif len(a) == 0:
                    print('0x{:x} | {}'.format(attr.attr_id, attr.value))
                else:
                    print('error: more than one metadata matched for id 0x{:x}: {}'.format(attr.attr_id, a))
    except KeyboardInterrupt:
        pass

def get_attr(stub, module, netif, hostif, cmds):
    if not module:
        print ('no module selected.')
        return
    if len(cmds) < 2:
        print ('invalid input: get <attr-name> [<attr-value>]')
        return
    object_type = tai_pb2.MODULE
    obj = module
    if netif:
        object_type = tai_pb2.NETIF
        obj = netif
    elif hostif:
        object_type = tai_pb2.HOSTIF
        obj = hostif

    req = tai_pb2.GetAttributeMetadataRequest()
    req.object_type = object_type
    req.attr_name = cmds[1]

    try:
        res = stub.GetAttributeMetadata(req)
    except grpc.RpcError as e:
        print(e)
        return

    m = res.metadata
    if not m:
        print('failed to find metadata for {}'.format(cmds[1]))
        return

    req = tai_pb2.GetAttributeRequest()
    req.oid = obj.oid
    req.attribute.attr_id = m.attr_id
    if len(cmds) > 2:
        req.attribute.value = cmds[2]

    try:
        res = stub.GetAttribute(req)
        return res.attribute.value
    except grpc.RpcError as e:
        return e

def set_attr(stub, module, netif, hostif, cmds):
    if not module:
        print ('no module selected.')
        return
    if len(cmds) < 2:
        print('invalid input: set <attr-name> <attr-value>')
        return

    object_type = tai_pb2.MODULE
    obj = module
    if netif:
        object_type = tai_pb2.NETIF
        obj = netif
    elif hostif:
        object_type = tai_pb2.HOSTIF
        obj = hostif

    req = tai_pb2.GetAttributeMetadataRequest()
    req.object_type = object_type
    req.attr_name = cmds[1]
    res = stub.GetAttributeMetadata(req)

    m = res.metadata
    if not m:
        print('failed to find metadata for {}'.format(cmds[1]))
        return

    if m.is_readonly:
        print('attribute {} is read-only'.format(cmds[1]))
        return

    if len(cmds) < 3:
        print('usage: set {} {}'.format(cmds[1], m.usage))
        return

    req = tai_pb2.SetAttributeRequest()
    req.oid = obj.oid
    req.attribute.attr_id = m.attr_id
    req.attribute.value = cmds[2]

    try:
        stub.SetAttribute(req)
    except grpc.RpcError as e:
        print(e)


def set_log_level(stub, cmds):
    if len(cmds) < 2:
        print('invalid input: log-level <level> [<api-type>]')
        return
    if cmds[1] == 'debug':
        level = tai_pb2.DEBUG
    elif cmds[1] == 'info':
        level = tai_pb2.INFO
    elif cmds[1] == 'notice':
        level = tai_pb2.NOTICE
    elif cmds[1] == 'warn':
        level = tai_pb2.WARN
    elif cmds[1] == 'error':
        level = tai_pb2.ERROR
    elif cmds[1] == 'critical':
        level = tai_pb2.CRITICAL
    else:
        print('invalid log level. choose from [debug, info, notice, warn, error, critical]')
        return

    api = tai_pb2.UNSPECIFIED_API
    if len(cmds) > 2:
        if cmds[2] == 'module':
            api = tai_pb2.MODULE_API
        elif cmds[2] == 'netif':
            api = tai_pb2.NETIF_API
        elif cmds[2] == 'hostif':
            api = tai_pb2.HOSTIF_API
        else:
            print('invalid api type. choose from [module, netif, hostif]')
            return

    req = tai_pb2.SetLogLevelRequest()
    req.level = level
    req.api = api
    return stub.SetLogLevel(req)


class TAIShellCompleter(Completer):
    def __init__(self, stub, modules, module, netif, hostif):
        self.stub = stub
        self.modules = modules
        self.module = module
        self.netif = netif
        self.hostif = hostif
        self.metadata = get_attribute_metadata(stub, module, netif, hostif)

    def get_completions(self, document, complete_event):
        t = document.text.split(' ')
        if len(t) == 1:
            cmds = ['list', 'module', 'help', 'quit', 'exit', 'log-level']
            if self.module != None:
                cmds += ['list-attr', 'netif', 'hostif', 'set', 'get', 'monitor']

            for c in cmds:
                if c.startswith(t[0]):
                    yield Completion(c, start_position=-len(t[0]))

        elif len(t) == 2:
            if t[0] in ['set', 'get', 'monitor']:
                for m in self.metadata:
                    if t[0] == 'set' and m.is_readonly:
                        continue
                    elif t[0] == 'monitor' and m.usage != '<notification>':
                        continue
                    if m.short_name.startswith(t[1]):
                        yield Completion(m.short_name, start_position=-len(t[1]))
            elif self.module != None and t[0] in ['netif', 'hostif']:
                l = self.module.netifs
                if t[0] == 'hostif':
                    l = self.module.hostifs
                for k in l:
                    k = str(k.index)
                    if k.startswith(t[1]):
                        yield Completion(k, start_position=-len(t[1]))
            elif t[0] == 'module':
                for k in self.modules.keys():
                    if k.startswith(t[1]):
                        yield Completion(k, start_position=-len(t[1]))
            elif t[0] == 'log-level':
                for k in ['debug', 'info', 'notice', 'warn', 'error', 'critical']:
                    if k.startswith(t[1]):
                        yield Completion(k, start_position=-len(t[1]))

        elif len(t) == 3 and t[0] == 'set':
            for m in self.metadata:
                if t[1] == m.short_name:
                    if m.usage[0] == '[' and m.usage[-1] == ']':
                        enums = m.usage[1:-1].split('|')
                        for enum in enums:
                            if enum.startswith(t[2]):
                                yield Completion(enum, start_position=-len(t[2]))

        return


def loop(stub, modules, module, netif, hostif):

    session = PromptSession()

    while True:
        p = "> "
        if module:
            loc = module.location
            p = "module({})> ".format(loc)
            if netif:
                p = "module({})/netif({})> ".format(loc, netif.index)
            elif hostif:
                p = "module({})/hostif({})> ".format(loc, hostif.index)

        c = FuzzyCompleter(TAIShellCompleter(stub, modules, module, netif, hostif))
        line = session.prompt(p, completer=c).strip()
        cmds = [ e.strip() for e in line.split() ]
        if len(cmds) == 0:
            continue

        cmd = cmds[0]
        try:
            if cmd == 'module':
                if len(cmds) != 2:
                    print('invalid input: module <location>')
                    continue
                loc = cmds[1]
                if loc not in modules:
                    print('no module whose location is {}'.format(loc))
                    continue
                module = modules[loc]
                netif = None
                hostif = None
            elif cmd == 'netif' or cmd == 'hostif':
                if not module:
                    print ('no module selected.')
                    continue
                if len(cmds) != 2:
                    print('invalid input: {} <index>'.format(cmd))
                    continue

                index = int(cmds[1])

                l = module.netifs
                if cmd == 'hostif':
                    l = module.hostifs

                if len(l) < index:
                    print('invalid index: len: {}'.format(len(l)))
                    continue

                if cmd == 'netif':
                    netif = l[index]
                    hostif = None
                else:
                    hostif = l[index]
                    netif = None

            elif cmd == 'list':
                show_modules(modules)
            elif cmd == 'list-attr':
                list_attr(stub, module, netif, hostif)
            elif cmd == 'get':
                print(get_attr(stub, module, netif, hostif, cmds))
            elif cmd == 'set':
                set_attr(stub, module, netif, hostif, cmds)
            elif cmd == 'monitor':
                monitor(stub, module, netif, hostif, cmds)
            elif cmd == 'log-level':
                set_log_level(stub, cmds)
            elif cmd in ['exit', 'quit', 'q']:
                if netif:
                    netif = None
                elif hostif:
                    hostif = None
                elif module:
                    module = None
                else:
                    sys.exit(0)
            elif cmd in ['help', 'h', '?']:
                show_help()
            else:
                print('unknown cmd: {}'.format(cmd))
                show_help()
        except Exception as e:
            print(e)

def main():
    parser = OptionParser()
    parser.add_option('--addr', default='localhost')
    parser.add_option('--module')
    parser.add_option('--netif', type='int')
    parser.add_option('--hostif', type='int')
    parser.add_option('-v', '--verbose', action='store_true')
    (options, args) = parser.parse_args()

    with grpc.insecure_channel('{}:50051'.format(options.addr)) as channel:
        stub = tai_pb2_grpc.TAIStub(channel)

        req = tai_pb2.ListModuleRequest()
        try:
            modules = { res.module.location: res.module for res in stub.ListModule(req) }
        except grpc._channel._Rendezvous as e:
            if options.verbose:
                print(e)
            else:
                print(e._state.details)
                print('specify a reachable host by using --addr option')
            sys.exit(1)

        module = None
        if options.module != None:
            if options.module not in modules:
                print('no module whose location is {}'.format(options.module))
                return
            module = modules[options.module]

        if options.netif != None and options.hostif != None:
            print('can\'t specify both netif and hostif')
            return

        netif = None
        if module != None and options.netif != None:
            if len(module.netifs) <= options.netif:
                print('invalid index: len: {}'.format(len(module.netifs)))
                return
            netif = module.netifs[options.netif]

        hostif = None
        if module != None and options.hostif != None:
            if len(module.hostifs) <= options.hostif:
                print('invalid index: len: {}'.format(len(module.hostifs)))
                return
            hostif = module.hostifs[options.hostif]

        if len(args) == 0:
            loop(stub, modules, module, netif, hostif)

        if args[0] == 'list':
            show_modules(modules)
            return

        if module == None:
            module = modules.values()[0]

        if args[0] == 'list-attr':
            list_attr(stub, module, netif, hostif)
        elif args[0] == 'get':
            print(get_attr(stub, module, netif, hostif, args))
        elif args[0] == 'set':
            set_attr(stub, module, netif, hostif, args)
        elif args[0] == 'help':
            show_help()
        elif args[0] == 'monitor':
            monitor(stub, module, netif, hostif, args)
        elif args[0] == 'log-level':
            set_log_level(stub, args)
        else:
            print('unknown cmd: {}'.format(args[0]))
            show_help()

if __name__ == '__main__':
    main()
