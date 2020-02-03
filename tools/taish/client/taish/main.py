#
# Copyright (C) 2018-2019 Nippon Telegraph and Telephone Corporation.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.

import taish
from taish import TAIException
from taish.cli import Object, InvalidInput, Completer

from optparse import OptionParser

from prompt_toolkit import PromptSession
from prompt_toolkit.completion import FuzzyCompleter, Completion, WordCompleter
from prompt_toolkit.key_binding import KeyBindings
from tabulate import tabulate

from itertools import chain

import json

TAI_ATTR_CUSTOM_RANGE_START = 0x10000000

class TAICompleter(Completer):
    def __init__(self, metadata, set_):
        self.metadata = metadata
        self.set_ = set_
        hook = lambda : not set_
        super(TAICompleter, self).__init__(self.attrnames, self.valuenames, hook)

    def attrnames(self):
        return [v.short_name for v in self.metadata if not self.set_ or not v.is_readonly]

    def valuenames(self, attrname):
        for v in self.metadata:
            if attrname == v.short_name:
                if v.usage == 'bool':
                    return ['true', 'false']
                elif v.usage[0] == '[' and v.usage[-1] == ']':
                    return v.usage[1:-1].split('|')
                else:
                    return []
        return []

class TAIShellObject(Object):
    def __init__(self, client, name, parent):
        super(TAIShellObject, self).__init__(parent)
        self.client = client
        self.name = name

        m = self.client.list_attribute_metadata()

        @self.command(TAICompleter(m, set_=False))
        def get(args):
            if len(args) != 1:
                raise InvalidInput('usage: get <name>')
            try:
                print(self.client.get(args[0], json=JSON_OUTPUT))
            except TAIException as e:
                print('err: {} (code {:x})'.format(e.msg, e.code))

        @self.command(TAICompleter(m, set_=True))
        def set(args):
            if len(args) != 2:
                raise InvalidInput('usage: set <name> <value>')
            try:
                self.client.set(args[0], args[1])
            except TAIException as e:
                print('err: {} (code: {:x})'.format(e.msg, e.code))

        @self.command()
        def monitor(args):
            if len(args) == 0:
                args = ['notify'] #TODO default value handling
            if len(args) != 1:
                raise InvalidInput('usage: monitor <name>')

            l = self.client.list_attribute_metadata()

            def cb(res):
                for attr in res.attrs:
                    a = [ v.short_name for v in l if v.attr_id == attr.attr_id ]
                    if len(a) == 1:
                        if JSON_OUTPUT:
                            print(json.dumps({"name": a[0], "value": json.loads(attr.value)}))
                        else:
                            print('{} | {}'.format(a[0], attr.value))
                    elif len(a) == 0:
                        if JSON_OUTPUT:
                            print(json.dumps({"name": attr.attr_id, "value": json.loads(attr.value)}))
                        else:
                            print('0x{:x} | {}'.format(attr.attr_id, attr.value))
                    else:
                        print('error: more than one metadata matched for id 0x{:x}: {}'.format(attr.attr_id, a))

            try:
                self.client.monitor(args[0], cb, json=JSON_OUTPUT)
                print()
            except TAIException as e:
                print('err: {} (code: {:x})'.format(e.msg, e.code))

        @self.command(name='list-attr')
        def list(args):
            if len(args) > 1 or (len(args) == 1 and args[0] not in ['simple']):
                raise InvalidInput('usage: list-attr [simple]')
            simple = len(args) == 1
            if simple:
                for m in self.client.list_attribute_metadata():
                    print(m.short_name)
            else:
                d = []
                for m in self.client.list_attribute_metadata():
                    d.append([m.short_name, 'ro' if m.is_readonly else 'r/w', m.usage, 'custom' if m.attr_id > TAI_ATTR_CUSTOM_RANGE_START else 'official'])
                print(tabulate(d, headers=['name', 'type', 'value', 'range']))


class HostIf(TAIShellObject):
    def __init__(self, client, name, parent):
        super(HostIf, self).__init__(client, name, parent)

    def __str__(self):
        return 'hostif({})'.format(self.name)

class NetIf(TAIShellObject):
    def __init__(self, client, name, parent):
        super(NetIf, self).__init__(client, name, parent)

    def __str__(self):
        return 'netif({})'.format(self.name)

class Module(TAIShellObject):
    def __init__(self, client, name, parent):
        super(Module, self).__init__(client, name, parent)

        @self.command(WordCompleter([str(v) for v in range(len(self.client.obj.netifs))]))
        def netif(line):
            if len(line) != 1:
                raise InvalidInput('usage: netif <index>')
            return NetIf(self.client.get_netif(int(line[0])), line[0], self)

        @self.command(WordCompleter([str(v) for v in range(len(self.client.obj.hostifs))]))
        def hostif(line):
            if len(line) != 1:
                raise InvalidInput('usage: hostif <index>')
            return HostIf(self.client.get_hostif(int(line[0])), line[0], self)

    def __str__(self):
        return 'module({})'.format(self.name)

class Root(Object):
    def __init__(self, client):
        super(Root, self).__init__(None)
        self.client = client

        @self.command(WordCompleter(self.client.list().keys(), WORD=True))
        def module(line):
            if len(line) != 1:
                raise InvalidInput('usage: module <name>')
            return Module(self.client.get_module(line[0]), line[0], self)

        @self.command(name='list')
        def _list(line):
            if len(line) != 0:
                raise InvalidInput('usage: list')
            for k, m in self.client.list().items():
                stroid = lambda oid : '0x{:08x}'.format(oid)
                print('module:', k, stroid(m.oid) if oid else 'not present')
                for v in m.hostifs:
                    print(' hostif:', v.index, stroid(v.oid))

                for v in m.netifs:
                    print(' netif:', v.index, stroid(v.oid))

        @self.command(WordCompleter(['debug', 'info', 'notice', 'warn', 'error', 'critical']), 'log-level')
        def set_log_level(line):
            if len(line) != 1:
                raise InvalidInput('usage: log-level <level>')
            self.client.set_log_level(line[0])

        @self.command()
        def create(line):
            if len(line) < 1:
                raise InvalidInput('invalid input: create [module| netif <module-oid> | hostif <module-oid>] [<attr-name>:<attr-value> ]...')
            object_type = line[0]
            line.pop(0)
            module_id = 0
            if object_type in ['netif', 'hostif']:
                if len(line) < 1:
                    print('invalid input: create [module| netif <module-oid> | hostif <module-oid>] [<attr-name>:<attr-value> ]...')
                    return
                module_id = int(line[0], 0)
                line.pop(0)

            attrs = [tuple(attr.split(':')) for attr in line]
            try:
                print('oid: 0x{:x}'.format(self.client.create(object_type, attrs, module_id)))
            except TAIException as e:
                print('err: {} (code: {:x})'.format(e.msg, e.code))

        get_oids = lambda : ('0x{:x}'.format(elem.oid) for elem in chain.from_iterable([v] + list(v.netifs) + list(v.hostifs) for v in self.client.list().values() if v.oid > 0))

        @self.command(WordCompleter(get_oids))
        def remove(line):
            if len(line) != 1:
                raise InvalidInput('usage: remove <oid>')
            try:
                self.client.remove(int(line[0], 0))
            except TAIException as e:
                print('err: {} (code: {:x})'.format(e.msg, e.code))

    def __str__(self):
        return ''


class TAIShellCompleter(Completer):
    def __init__(self, context):
        self.context = context 

    def get_completions(self, document, complete_event):
        return self.context.completion(document, complete_event)

class TAIShell(object):
    def __init__(self, addr, port):
        client = taish.Client(addr, port)
        self.client = client
        self.context = Root(client)
        self.completer = TAIShellCompleter(self.context)
        self.default_input = ''

    def prompt(self):
        c = self.context
        l = [str(c)]
        while c.parent:
            l.insert(0, str(c.parent))
            c = c.parent
        if len(l) == 1:
            return '> '
        return '/'.join(l)[1:] + '> '

    def exec(self, cmd: list):
        self.context = self.context.exec(cmd)
        self.completer.context = self.context
        self.default_input = ''

    def bindings(self):
        b = KeyBindings()

        @b.add('?')
        def _(event):
            buf = event.current_buffer
            original_text = buf.text
            help_msg = event.app.shell.context.help(buf.text)
            buf.insert_text('?')
            buf.insert_line_below(copy_margin=False)
            buf.insert_text(help_msg)
            event.app.exit('')
            event.app.shell.default_input = original_text

        return b

def loop(addr, port):
    session = PromptSession()

    default_prompt = '> '
    prompt = default_prompt

    shell = TAIShell(addr, port)

    while True:
        c = shell.completer
        p = shell.prompt()
        b = shell.bindings()
        session.app.shell = shell
        line = session.prompt(p, completer=c, key_bindings=b, default=shell.default_input)
        if len(line) > 0:
            shell.exec(line)

def main():
    parser = OptionParser()
    parser.add_option('--addr', default='localhost')
    parser.add_option('--port', default=50051)
    parser.add_option('-c', '--command-string')
    parser.add_option('-j', '--json', action="store_true", default=False)

    (options, args) = parser.parse_args()

    global JSON_OUTPUT
    JSON_OUTPUT = options.json

    if options.command_string:
        shell = TAIShell(options.addr, options.port)
        for line in options.command_string.split(';'):
            shell.exec(line)
        return

    loop(options.addr, options.port)

if __name__ == '__main__':
    main()
