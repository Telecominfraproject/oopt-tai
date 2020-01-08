#
# Copyright (C) 2018-2019 Nippon Telegraph and Telephone Corporation.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.

from prompt_toolkit.document import Document
from prompt_toolkit.completion import Completion, WordCompleter
from prompt_toolkit.completion import Completer as PromptCompleter
from prompt_toolkit import print_formatted_text as print

import sys

class InvalidInput(Exception):
    def __init__(self, msg, candidates=[]):
        self.msg = msg
        self.candidates = candidates

    def __str__(self):
        return self.msg

class Completer(PromptCompleter):
    def __init__(self, attrnames, valuenames=[], hook=None):
        if type(attrnames) == list:
            self._attrnames = lambda : attrnames
        else:
            self._attrnames = attrnames

        if type(valuenames) == list:
            self._valuenames = lambda _ : valuenames
        else:
            self._valuenames = valuenames

        self._hook = hook

    def get_completions(self, document, complete_event=None):
        t = document.text.split()
        if len(t) == 0 or (len(t) == 1 and document.text[-1] != ' '):
            # attribute name completion
            for c in self._attrnames():
                if c.startswith(document.text):
                    yield Completion(c, start_position=-len(document.text))
        elif len(t) > 2 or (len(t) == 2 and document.text[-1] == ' '):
            # invalid input for both get() and set(). no completion possible
            return
        else:
            # value completion

            if self._hook and self._hook():
                return

            # complete the first argument
            doc = Document(t[0])
            c = list(self.get_completions(doc))
            if len(c) == 0:
                return
            elif len(c) > 1:
                # search perfect match
                l = [v.text for v in c if v.text == t[0]]
                if len(l) == 0:
                    return
                attrname = l[0]
            else:
                attrname = c[0].text

            text = t[1] if len(t) > 1 else ''

            for c in self._valuenames(attrname):
                if c.startswith(text):
                    yield Completion(c, start_position=-len(text))


class Object(object):
    XPATH = ''

    def __init__(self, parent):
        self.parent = parent
        self._commands = {}

        @self.command()
        def quit(line):
            return self.parent if self.parent else sys.exit(0)

    def add_command(self, handler, completer=None, name=None):
        self.command(completer, name)(handler)

    def del_command(self, name):
        del self._commands[name]

    def command(self, completer=None, name=None):
        def f(func):
            def _inner(line):
                v = func(line)
                return v if v else self
            self._commands[name if name else func.__name__] = {'func': _inner, 'completer': completer}
        return f

    def help(self, text='', short=True):
        text = text.lstrip()
        try:
            v = text.split()
            if len(text) > 0 and text[-1] == ' ':
                # needs to show all candidates for the next argument
                v.append(' ')
            line = self.complete_input(v)
        except InvalidInput as e:
            return ', '.join(e.candidates)
        return line[-1].strip()

    def commands(self):
        return list(self._commands.keys())

    def completion(self, document, complete_event=None):
        # complete_event is None when this method is called by complete_input()
        if complete_event == None and len(document.text) == 0:
            return
        t = document.text.split()
        if len(t) == 0 or (len(t) == 1 and document.text[-1] != ' '):
            # command completion
            for cmd in self.commands():
                if cmd.startswith(document.text):
                    yield Completion(cmd, start_position=-len(document.text))
        else:
            # argument completion
            # complete command(t[0]) first
            try:
                cmd = self.complete_input([t[0]])[0]
            except InvalidInput:
                return
            v = self._commands.get(cmd)
            if not v:
                return
            c = v['completer']
            if c:
                # do argument completion with text after the command (t[0])
                new_document = Document(document.text[len(t[0]):].lstrip())
                for v in c.get_completions(new_document, complete_event):
                    yield v

    def complete_input(self, line):

        if len(line) == 0:
            raise InvalidInput('invalid command. available commands: {}'.format(self.commands()), self.commands())

        for i in range(len(line)):
            doc = Document(' '.join(line[:i+1]))
            c = list(self.completion(doc))
            if len(c) == 0:
                if i == 0:
                    raise InvalidInput('invalid command. available commands: {}'.format(self.commands()), self.commands())
                else:
                    # t[0] must be already completed
                    v = self._commands.get(line[0])
                    assert(v)
                    cmpl = v['completer']
                    if cmpl:
                        doc = Document(' '.join(line[:i] + [' ']))
                        candidates = list(v.text for v in self.completion(doc))
                        # if we don't have any candidates with empty input, it means the value needs
                        # to be passed as an opaque value
                        if len(candidates) == 0:
                            continue

                        raise InvalidInput('invalid argument. candidates: {}'.format(candidates), candidates)
                    else:
                        # no command completer, the command doesn't take any argument
                        continue
            elif len(c) > 1:
                # search for a perfect match
                t = [v for v in c if v.text == line[i]]
                if len(t) == 0:
                    candidates = [v.text for v in c]
                    raise InvalidInput('ambiguous {}. candidates: {}'.format('command' if i == 0 else 'argument', candidates), candidates)
                c[0] = t[0]
            line[i] = c[0].text
        return line 

    def exec(self, cmd):
        line = cmd.split()
        try:
            line = self.complete_input(line)
            if line[0] in self._commands:
                return self._commands[line[0]]['func'](line=line[1:])
            else:
                raise InvalidInput('invalid command. available commands: {}'.format(self.commands()))
        except InvalidInput as e:
            print(str(e))
        return self
