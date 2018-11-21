#!/usr/bin/env python
#
# Copyright (C) 2018 Nippon Telegraph and Telephone Corporation.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
# implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import grpc
import sys

import tai_pb2
import tai_pb2_grpc

from optparse import OptionParser

from prompt_toolkit import PromptSession
from tabulate import tabulate

def loop(stub):

    module = None
    netif = None
    hostif = None
    modules = {}
    session = PromptSession()

    req = tai_pb2.ListModuleRequest()
    for res in stub.ListModule(req):
        key = res.module.location
        modules[key] = res.module

    while True:
        p = "> "
        if module:
            loc = module.location
            p = "module({})> ".format(loc)
            if netif:
                p = "module({})/netif({})> ".format(loc, netif.index)
            elif hostif:
                p = "module({})/hostif({})> ".format(loc, hostif.index)

        line = session.prompt(p).strip()
        cmds = [ e.strip() for e in line.split() ]
        if len(cmds) == 0:
            continue

        cmd = cmds[0]
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
            for k, m in modules.items():
                stroid = lambda oid : '0x{:08x}'.format(oid)
                print('module:', k, stroid(m.oid))
                for v in m.hostifs:
                    print(' hostif:', v.index, stroid(v.oid))

                for v in m.netifs:
                    print(' netif:', v.index, stroid(v.oid))

        elif cmd == 'list-attr':
            if not module:
                print ('no module selected.')
                continue

            object_type = tai_pb2.MODULE
            if netif:
                object_type = tai_pb2.NETIF
            elif hostif:
                object_type = tai_pb2.HOSTIF

            req = tai_pb2.ListAttributeMetadataRequest()
            req.object_type = object_type

            d = []
            for res in stub.ListAttributeMetadata(req):
                m = res.metadata
                d.append([m.short_name, 'true' if m.is_readonly else 'false', m.usage])

            print(tabulate(d, headers=['name', 'readonly', 'value']))

        elif cmd == 'get':
            if not module:
                print ('no module selected.')
                continue
            if len(cmds) != 2:
                print ('invalid input: get <attr-name>')
                continue

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
                continue

            m = res.metadata
            if not m:
                print('failed to find metadata for {}'.format(cmds[1]))
                continue

            req = tai_pb2.GetAttributeRequest()
            req.oid = obj.oid
            req.attr_id = m.attr_id

            try:
                res = stub.GetAttribute(req)
                print(res.attribute.value)
            except grpc.RpcError as e:
                print(e)

        elif cmd == 'set':

            if not module:
                print ('no module selected.')
                continue
            if len(cmds) < 2:
                print('invalid input: set <attr-name> <attr-value>')
                continue

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
                continue

            if m.is_readonly:
                print('attribute {} is read-only'.format(cmds[1]))
                continue
            
            if len(cmds) < 3:
                print('usage: set {} {}'.format(cmds[1], m.usage))
                continue

            req = tai_pb2.SetAttributeRequest()
            req.oid = obj.oid
            req.attribute.attr_id = m.attr_id
            req.attribute.value = cmds[2]

            try:
                stub.SetAttribute(req)
            except grpc.RpcError as e:
                print(e)

        elif cmd == 'exit' or cmd == 'quit' or cmd == 'q':
            if netif:
                netif = None
            elif hostif:
                hostif = None
            elif module:
                module = None
            else:
                sys.exit(0)
        else:
            print('invalid cmd: {}'.format(cmd))

if __name__ == '__main__':
    parser = OptionParser()
    parser.add_option('--addr', default='localhost')
    (options, args) = parser.parse_args()

    with grpc.insecure_channel('{}:50051'.format(options.addr)) as channel:
        stub = tai_pb2_grpc.TAIStub(channel)
        loop(stub)
