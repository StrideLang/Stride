# -*- coding: utf-8 -*-
"""
    Stride is licensed under the terms of the 3-clause BSD license.

    Copyright (C) 2017. The Regents of the University of California.
    All rights reserved.
    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

        Redistributions of source code must retain the above copyright notice,
        this list of conditions and the following disclaimer.

        Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

        Neither the name of the copyright holder nor the names of its
        contributors may be used to endorse or promote products derived from
        this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

    Authors: Andres Cabrera and Joseph Tilbian
"""

from __future__ import print_function
from __future__ import division


from platformTemplates import templates
from code_objects import Instance, BundleInstance, ModuleInstance, Declaration

try:
    unicode_exists_test = type('a') == unicode
except:
    unicode = str  # for python 3


def signal_type_string(signal_declaration):
    return type(signal_declaration['default']) == unicode


class Atom(object):
    def __init__(self, line = -1, filename = ''):
        self.rate = -1
        self.inline = False
        self.globals = {}
        self.handle = ''
        self.domain = None
        self.line = line
        self.filename = filename

        # FIXME these sections should be driven by the platform definition
        self.global_sections = ['include', 'includeDir', 'linkTo', 'linkDir']

    def set_inline(self, inline):
        self.inline = inline


    def get_handles(self):
        if self.is_inline():
            return [self.get_inline_processing_code([])]
        else:
            return [self.handle]

    def get_out_tokens(self):
        if hasattr(self, 'out_tokens'):
            return self.out_tokens
        else:
            return [self.handle]

    def get_declarations(self):
        return []

    def get_instances(self):
        return []

    def get_header_code(self):
        return ''

    def get_initialization_code(self, in_tokens):
        '''Returns code that should be executed only once per construction of
        the block'''
        return ''

    def get_preproc_once(self):
        return None

    def get_preprocessing_code(self, in_tokens):
        ''' Returns code that needs to be run asynchronously but can't be
        inlined, so needs to be run separately previous to the processing
        code'''
        return ''

    def get_processing_code(self, in_tokens):
        '''Processing code includes both the pre-processing and the
        inline processing code. The inline processing code will be provided
        ready to insert rather than ready to inline.'''
        return None

    def get_postprocessing_code(self, in_tokens):
        return ''

    def get_postproc_once(self):
        return None

    def get_inline_processing_code(self, in_tokens):
        ''' This returns the processing code itself, so this can be used
        when the output is used only once, and an intermediate symbol to
        represent it is not needed'''
        return None

    def is_inline(self):
        return self.inline

    def get_globals(self):
        return self.globals

    def get_rate(self):
        return self.rate

    def get_scope_index(self):
        return self.scope_index

    def get_domain(self):
        return self.domain

    def get_line(self):
        return self.line

    def get_filename(self):
        return self.filename


class PlatformTypeAtom(Atom):
    def __init__(self, module, function, platform_type, token_index,
                 platform, scope_index):
        super(PlatformTypeAtom, self).__init__()
        self.module = module
        self.platform_type = platform_type
        self.index = token_index
        self.platform = platform
        self.function = function
        self.scope_index = scope_index

        self.set_inline(False)

    def get_handles(self):
        if self.is_inline():
            return [self.get_inline_processing_code([])]
        else:
            return [self.handle]

    def get_out_tokens(self):
        if hasattr(self, 'out_tokens'):
            return self.out_tokens
        else:
            return [self.handle]

    def get_processing_code(self, in_tokens):
        return 'PROC_CODEEEEE'

    def get_inline_processing_code(self, in_tokens):
        return 'INLINE_CODEEEEE'


class ValueAtom(Atom):
    def __init__(self, value_node, index, scope_index):
        super(ValueAtom, self).__init__()
        self.index = index
        self.value = value_node
        self.handle = '__value_%03i' % index
        self.rate = 0
        self.inline = True
        self.scope_index = scope_index

    def get_handles(self):
        if self.is_inline():
            return [self.get_inline_processing_code([])]
        else:
            return [self.handle]

    def get_out_tokens(self):
        if self.is_inline():
            return [str(self.value)]
        else:
            return [self.handle]

    def get_instances(self):
        if self.is_inline():
            return []
        else:
            if type(self.value) == unicode:
                return [Instance('"' + self.get_inline_processing_code([]) + '"',
                                 self.scope_index,
                                 self.domain,
                                 'string',
                                 self.handle),
                        self]
            else:
                return [Instance(self.get_inline_processing_code([]),
                                 self.scope_index,
                                 self.domain,
                                 'real',
                                 self.handle,
                                 self)]

    def get_inline_processing_code(self, in_token):
        if type(self.value) == str or type(self.value) == unicode:
            return self.value
        if type(self.value) == bool:
            return templates.value_bool(self.value)
        else:
            return templates.value_real(self.value)

    def get_processing_code(self, in_tokens):
        return {None: ['', [self.get_inline_processing_code(in_tokens)]]}


class ExpressionAtom(Atom):
    def __init__(self, expr_type, left_atom, right_atom, index, scope_index):
        super(ExpressionAtom, self).__init__()
        self.scope_index = scope_index
        self.expr_type = expr_type
        self.left_atom = left_atom
        self.right_atom = right_atom
        self.index = index
        self.handle = '__expr_%03i' % index
        self.rate = -1
        if isinstance(self.left_atom, ModuleAtom) or isinstance(self.right_atom, ModuleAtom):
            self.set_inline(False)
        else:
            self.set_inline(True)

        self.domain = left_atom.get_domain()
        if right_atom:
            if not self.domain == right_atom.get_domain() and right_atom.get_domain() is not None:
                print("ERROR! domains must match inside expressions!")

    def set_inline(self, inline):
        self.left_atom.set_inline(inline)
        if self.right_atom:
            self.right_atom.set_inline(inline)
        self.inline = inline

    def get_declarations(self):
        declarations = self.left_atom.get_declarations()
        if self.right_atom:
            declarations += self.right_atom.get_declarations()
        return declarations

    def get_instances(self):
        instances = self.left_atom.get_instances()
        if self.right_atom:
            instances += self.right_atom.get_instances()
        if not self.is_inline():
            instances.append(Instance('',
                                      self.scope_index,
                                      self.domain,
                                      self._expression_out_type(),
                                      self.handle,
                                      self))
        return instances

    def get_inline_processing_code(self, in_tokens):
        if self.left_atom.is_inline():
            left_token = self.left_atom.get_inline_processing_code([])
        else:
            left_token = self.left_atom.get_out_tokens()[0]

        if self.right_atom:
            if self.right_atom.is_inline():
                right_token = self.right_atom.get_inline_processing_code([])
            else:
                right_token = self.right_atom.get_out_tokens()[0]
        else:
            right_token = None

        code = '(' + self._operator_symbol(left_token, right_token) + ')'
        return code

    def get_initialization_code(self, in_tokens):
        left_code = self.left_atom.get_initialization_code(in_tokens)
        right_code = ''
        if self.right_atom:
            right_code = self.right_atom.get_initialization_code(in_tokens)
        return left_code + right_code

    def get_preprocessing_code(self, in_tokens):
        left_code = self.left_atom.get_preprocessing_code([])
        right_code = ''
        if self.right_atom:
            right_code = self.right_atom.get_preprocessing_code([])
        return left_code + right_code

    def get_processing_code(self, in_tokens):
        # code = self.get_preprocessing_code(in_tokens)
        code = ''
        #domain = None
        processing_code = {self.domain: ['', []]}
        right_tokens = []
        if self.is_inline():
            out_tokens = [self.get_inline_processing_code(in_tokens)]
            processing_code[self.domain] = ['', out_tokens]
        else:
            left_proc_code = self.left_atom.get_processing_code([])
            for proc_domain, [code, tokens] in left_proc_code.items():
                if not proc_domain in processing_code:
                    processing_code[proc_domain] = ['', []]
                processing_code[proc_domain][0] += code
                #processing_code[proc_domain][1] += tokens
            if self.right_atom:
                for proc_domain, [code, tokens] in self.right_atom.get_processing_code([]).items():
                    if not proc_domain in processing_code:
                        processing_code[proc_domain] = ['', []]

                    processing_code[proc_domain][0] += code
                    #processing_code[proc_domain][1] += tokens
                    right_tokens = self.right_atom.get_out_tokens()[0]


            processing_code[self.domain][0] += templates.assignment(self.handle,
                                         self._operator_symbol(self.left_atom.get_out_tokens()[0],
                                                               right_tokens))
            processing_code[self.domain][1] += [self.handle]
        return processing_code #{domain: [code, out_tokens]}

    def get_postprocessing_code(self, in_tokens):
        left_code = self.left_atom.get_postprocessing_code([])
        right_code = ''
        if self.right_atom:
            right_code = self.right_atom.get_postprocessing_code([])
        return left_code + right_code

    def _expression_out_type(self):
        if self.expr_type == 'Add':
            out_type = 'real'
        elif self.expr_type == 'Subtract':
            out_type = 'real'
        elif self.expr_type == 'Multiply':
            out_type = 'real'
        elif self.expr_type == 'Divide':
            out_type = 'real'
        elif self.expr_type == 'And':
            out_type = 'bool'
        elif self.expr_type == 'Or':
            out_type = 'bool'
        elif self.expr_type == 'UnaryMinus':
            out_type = 'real'
        elif self.expr_type == 'LogicalNot':
            out_type = 'bool'
        elif self.expr_type == 'Greater':
            out_type = 'bool'
        elif self.expr_type == 'Lesser':
            out_type = 'bool'
        elif self.expr_type == 'Equal':
            out_type = 'bool'
        elif self.expr_type == 'NotEqual':
            out_type = 'bool'
        elif self.expr_type == 'GreaterEqual':
            out_type = 'bool'
        elif self.expr_type == 'LesserEqual':
            out_type = 'bool'
        return out_type

    def _operator_symbol(self, left, right=None):
        code = '' if right is None else left
        if self.expr_type == 'Add':
            code += ' + '
        elif self.expr_type == 'Subtract':
            code += ' - '
        elif self.expr_type == 'Multiply':
            code += ' * '
        elif self.expr_type == 'Divide':
            code += ' / '
        elif self.expr_type == 'And':
            code += ' & '
        elif self.expr_type == 'Or':
            code += ' | '
        elif self.expr_type == 'UnaryMinus':
            code = ' - '
        elif self.expr_type == 'LogicalNot':
            code += ' ~ '
        elif self.expr_type == 'Greater':
            code += ' > '
        elif self.expr_type == 'Lesser':
            code += ' < '
        elif self.expr_type == 'Equal':
            code += ' == '
        elif self.expr_type == 'NotEqual':
            code += ' != '
        elif self.expr_type == 'GreaterEqual':
            code += ' >= '
        elif self.expr_type == 'LesserEqual':
            code += ' <= '
        code += left if right is None else right
        return code


class ListAtom(Atom):
    def __init__(self, list_node, scope_index, domain):
        super(ListAtom, self).__init__()
        self.scope_index = scope_index
        self.list_node = list_node
        self.rate = -1
        self.inline = True

        self.handles = [elem.get_handles() for elem in list_node] # TODO make this recursive
        self.out_tokens = []
        for elem in list_node:
            self.out_tokens += elem.get_out_tokens()
        self.instances = []
        for elem in list_node:
            self.instances += elem.get_instances()

        element_rates = [elem.get_rate() for elem in list_node]

        if (len(set(element_rates)) <= 1):
            if len(element_rates) > 0:
                self.rate = element_rates[0]


        self.globals = {}
        for atom in self.list_node:
            new_globals = atom.get_globals()
            self.globals.update(new_globals)

        self.domain = domain

    def get_handles(self, index = -1):
        return self.handles

    def get_out_tokens(self, index = -1):
        return self.out_tokens[index]

    def get_declarations(self, index = -1):
        declarations = []
        for elem in self.list_node:
            elem_declarations = elem.get_declarations()
            declarations += elem_declarations
        return declarations

    def get_instances(self, index = -1):
        if index == -1:
            return self.instances
        return self.instances[index]

    def get_inline_processing_code(self, in_tokens):
        return str(self.value)

    def get_initialization_code(self, in_tokens):
        code = ''
        for i,elem in enumerate(self.list_node):
            new_code = elem.get_initialization_code(in_tokens)
            code += new_code
        return code

    def get_preproc_once(self):
        preproc = []
        for i,elem in enumerate(self.list_node):
            new_preproc = elem.get_preproc_once()
            if new_preproc:
                preproc += new_preproc
        return preproc

    def get_preprocessing_code(self, in_tokens):
        code = ''
        for i,elem in enumerate(self.list_node):
            if len(in_tokens) > 0:
                index = i % len(in_tokens)
                new_code = elem.get_preprocessing_code([in_tokens[index]])
                code += new_code
            else:
                new_code = elem.get_preprocessing_code([])
                code += new_code
        return code

    def get_processing_code(self, in_tokens):
        proc_code = {}
        list_domain = self._get_list_domain()
        for i,elem in enumerate(self.list_node):
            if len(in_tokens) > 0:
                index = i % len(in_tokens)
                elem_proc_code = elem.get_processing_code([in_tokens[index]])
            else:
                elem_proc_code = elem.get_processing_code([])
            for domain in elem_proc_code:
                domain_proc_code = elem_proc_code[domain]
                if not domain and isinstance(elem, ValueAtom):
                    domain = list_domain
                if not domain in proc_code:
                    proc_code[domain] = ['', []]
                new_code, new_out_tokens = domain_proc_code
                proc_code[domain][0] += new_code
                proc_code[domain][1] += new_out_tokens
        return proc_code

    def get_postprocessing_code(self, in_tokens):
        code = ''
        for i,elem in enumerate(self.list_node):
            if len(in_tokens) > 0:
                index = i % len(in_tokens)
                new_code = elem.get_postprocessing_code([in_tokens[index]])
                code += new_code
            else:
                new_code = elem.get_postprocessing_code([])
                code += new_code
        return code

    def get_postproc_once(self):
        postproc = []
        for i,elem in enumerate(self.list_node):
            new_postproc = elem.get_postproc_once()
            if new_postproc:
                postproc += new_postproc
        return postproc

    def _get_list_domain(self):
        for elem in self.list_node:
            new_domain = elem.get_domain()
            if new_domain and len(new_domain) > 0:
                return new_domain
        return None


class NameAtom(Atom):
    def __init__(self, platform_type, declaration, token_index,
                 platform, scope_index, line, filename, previous_atom, next_atom):
        super(NameAtom, self).__init__(line, filename)
        self.platform = platform
        self.scope_index = scope_index
        self.handle = declaration['name']  # + '_%03i'%token_index;
        self.platform_type = platform_type
        self.declaration = declaration
        self.domain = None
        self.signalbridge = None # Stores signalbridge name if applicable
        self.token_index = token_index

        if 'domain' in self.declaration:
            if type(self.declaration['domain']) == dict:
                # FIXME this should be set by the code validator
                self.domain = self.declaration['domain']['name']['name']
            else:
                self.domain = self.declaration['domain']

        if self.declaration['type'] == "signalbridge":
            if not previous_atom:
                domainProp = self.declaration['outputDomain']
            else:
                domainProp = self.declaration['inputDomain']

            if type(domainProp) == dict:
                self.domain = domainProp['name']['name']
            else:
                self.domain = domainProp

        if self.declaration['type'] == 'signal':
            # TODO we need checking of scope and domain here
            for scope_decl in self.platform.scope_stack[-1]:
                # Do we need to support blockbundle here or are all signal bridges blocks?
                if 'block' in scope_decl and 'type' in scope_decl['block'] and scope_decl['block']['type'] == "signalbridge":
                    if scope_decl['block']['signal'] == self.handle:
                        print(scope_decl['block']['signal'])
                        # Do we actually need to store this here? It seems that this is not really accesible
                        # where needed and we still have to go through the scope declarations anyway...
                        self.signalbridge = scope_decl['block']['name']


        # TODO we should just add all sections found in platform_type['block'][section]
        # And not worry wat exists already in self.global_sections
        for section in self.global_sections:
            if section in platform_type['block']:
                if section in self.globals:
                    self.globals[section].extend([inc['value'] for inc in platform_type['block'][section]])
                else:
                    if section in platform_type['block']:
                        self.globals[section] = [inc['value'] for inc in platform_type['block'][section]]
        if 'initializations' in platform_type['block']:
            if 'initializations' in self.globals:
                self.globals['initializations'] += platform_type['block']['initializations']
            else:
                self.globals['initializations'] = platform_type['block']['initializations']

        self.set_inline(False)

        if 'outputs' in platform_type['block']:
            if len(platform_type['block']['outputs']) > 0:
                self.set_inline(True)

        if 'rate' in declaration:
            if type(declaration['rate']) == dict:
                self.rate = -1
            else:
                self.rate = declaration['rate']


    def get_declarations(self):
        declarations = []
        if 'declarations' in self.platform_type['block']:
            for i,dec in enumerate(self.platform_type['block']['declarations']):
                declarations.append(Declaration(self.scope_index,
                                                self.domain,
                                                "_dec_%03i"%self.token_index, # This gives it a unique "id"... hacky
                                                dec['value'] + '\n'))
                self.platform_type['block']['declarations'] = [] # declarations have been consumed
        return declarations

    def get_instances(self):

#        if 'port_block' in self.declaration and self.declaration['port_block']:
#            return [] # Atoms for main ports need not be declared
                # But we need to know the declarations for reactions to
                # to known what they ned to be passed...
        default_value = self._get_default_value()
        if 'type' in self.declaration and self.declaration['type'] == 'signal':
            if signal_type_string(self.declaration):
                inits = [Instance(default_value,
                                 self.declaration['stack_index'],
                                 self.domain,
                                 'string',
                                 self.handle,
                                 self
                                 )]
            else:
                inits = [Instance(str(default_value),
                                 self.declaration['stack_index'],
                                 self.domain,
                                 'real',
                                 self.handle,
                                 self)]
        elif 'type' in self.declaration and self.declaration['type'] == 'signalbridge':
            if self.declaration['bridgeType'] == 'switch':
                inits = [Instance(default_value,
                                 self.declaration['stack_index'],
                                 self.domain,
                                 'bool',
                                 self.handle,
                                 self
                                 )]

            elif signal_type_string(self.declaration):
                inits = [Instance(default_value,
                                 self.declaration['stack_index'],
                                 self.domain,
                                 'string',
                                 self.handle,
                                 self
                                 )]
            else:
                inits = [Instance(str(default_value),
                                 self.declaration['stack_index'],
                                 self.domain,
                                 'real',
                                 self.handle,
                                 self)]
        elif 'type' in self.declaration and self.declaration['type'] == 'constant':
            inits = [Instance(str(default_value),
                                 self.declaration['stack_index'],
                                 self.domain,
                                 'real',
                                 self.handle,
                                 self)]
        elif 'type' in self.declaration and self.declaration['type'] == 'switch':
            inits = [Instance(templates.str_true if default_value else templates.str_false,
                             self.declaration['stack_index'],
                             self.domain,
                             'bool',
                             self.handle,
                             self)]
        elif 'type' in self.declaration and self.declaration['type'] == 'trigger':
            inits = [Instance(templates.str_true if default_value else templates.str_false,
                             self.declaration['stack_index'],
                             self.domain,
                             'bool',
                             self.handle,
                             self)]
        elif 'block' in self.platform_type:
            inherits = self.platform_type['block']['inherits']
            if inherits == 'signal':
                if 'default' in self.declaration and signal_type_string(self.declaration):
                    inits = [Instance(default_value,
                                 self.declaration['stack_index'],
                                 self.domain,
                                 'string',
                                 self.handle,
                                 self)]
                else:
                    inits = [Instance(str(default_value),
                                 self.declaration['stack_index'],
                                 self.domain,
                                 'real',
                                 self.handle,
                                 self)]
            elif self.platform_type['block']['type'] == 'platformType':
                if 'default' in self.declaration and signal_type_string(self.declaration):
                    inits = [Instance(default_value,
                                 self.declaration['stack_index'],
                                 self.domain,
                                 'string',
                                 self.handle,
                                 self)]
                else:
                    inits = [Instance(str(default_value),
                                 self.declaration['stack_index'],
                                 self.domain,
                                 'real',
                                 self.handle,
                                 self)]

            else:
                print("Don't know how to declare type " + ' '.join(self.declaration.keys()))
                inits = []
        else:
            print("Don't know how to declare type " + ' '.join(self.declaration.keys()))
            inits = []
        return inits

    def get_inline_processing_code(self, in_tokens):
        code = ''
        if 'processing' in self.platform_type['block']:
            code = templates.get_platform_inline_processing_code(
                            self.platform_type['block']['processing'],
                            in_tokens,
                            len(self.platform_type['block']['inputs']),
                            len(self.platform_type['block']['outputs']) )
        else:
            if len(in_tokens) > 0:
                code = in_tokens[0]
            else:
                code = self.handle

        return  code

    def get_initialization_code(self, in_tokens):
        code = ''
        if 'initializations' in self.platform_type['block']:
            code = self.platform_type['block']['initializations']
            if type(code) == list:
                merged_code = ''
                for value in code:
                    merged_code += value['value'] + '\n'
                code = merged_code

            self.platform_type['block']['initializations'] = [] # initializations have been consumed
            code += templates.get_platform_initialization_code(code,
                            in_tokens,
                            len(self.platform_type['block']['inputs']),
                            [self.handle]
                            )
        return code

    def get_preproc_once(self):
        if 'block' in self.platform_type and self.platform_type['block']['type'] == "platformType":
            if not self.platform_type['block']['preProcessingOnce'] == '':
                return [[self.platform_type['block']['name'], self.platform_type['block']['preProcessingOnce']]]
        return None

    def get_preprocessing_code(self, in_tokens):
        code = ''
        if 'preProcessing' in self.platform_type['block']:
            code = self.platform_type['block']['preProcessing']
            code = templates.get_platform_preprocessing_code(code,
                            in_tokens,
                            len(self.platform_type['block']['inputs']),
                            [self.handle]
                            )
        return code

    def get_processing_code(self, in_tokens):
        code = ''
        out_tokens = [self.handle]
        proc_code = self.get_inline_processing_code(in_tokens)
        domain = self.domain
        domain_proc_code = {}
        domain_proc_code[domain] = ['', []]

        outdomain = None
        if 'outputDomain' in self.declaration:
            if type(self.declaration['outputDomain']) == dict:
                # FIXME this should be set by the code validator
                outdomain = self.declaration['outputDomain']['name']['name']
            else:
                outdomain = self.declaration['outputDomain']
            out_tokens = []
            domain_proc_code[outdomain] = ['', [self.handle]]

        if len(proc_code) > 0:
            if 'processing' in self.platform_type['block']:
                if self.inline:
                    out_tokens = [proc_code]
                else:
                    if len(self.platform_type['block']['outputs']) > 0:
                        code = templates.assignment(self.handle, proc_code)
                    else:
                        code = templates.expression(proc_code)
            else:
                code = templates.assignment(self.handle, proc_code)

            domain_proc_code[domain][0] += code
            domain_proc_code[domain][1] += out_tokens
        return domain_proc_code

    def get_postprocessing_code(self, in_tokens):
        code = ''
        if 'postProcessing' in self.platform_type['block']:
            code = self.platform_type['block']['postProcessing']
            code = templates.get_platform_postprocessing_code(code,
                            in_tokens,
                            len(self.platform_type['block']['inputs']),
                            [self.handle]
                            )
        return code

    def get_postproc_once(self):
        if 'block' in self.platform_type and self.platform_type['block']['type'] == "platformType":
            if not self.platform_type['block']['postProcessingOnce'] == '':
                return [[self.platform_type['block']['name'], self.platform_type['block']['postProcessingOnce']]]
        return None


    def _get_default_value(self):
        if self.declaration['type'] == "signal" or self.declaration['type'] == "signalbridge":
            if 'default' in self.declaration:
                if type(self) == NameAtom:
                    default_value = self.declaration['default']
                elif type(self) == BundleAtom:
                    if type(self.declaration['default']) == list:
                        default_value = [value['value'] for value in self.declaration['default']]
                    else:
                        default_value = [self.declaration['default'] for i in range(self.declaration['size'])]
            elif 'block' in self.platform_type:
                if 'default' in self.platform_type['block']:
                    default_value = self.platform_type['block']['default'] # FIXME inheritance is not being handled here
                else:
                    default_value = 0.0
            elif 'blockbundle' in self.platform_type:
                #Stride definition
                if 'default' in self.platform_type['blockbundle']:
                    default_value = self.platform_type['blockbundle']['default']
                else:
                    default_value = 0.0
            else:
                print("Forced default value to 0 for " + self.handle)
                default_value = 0.0
            if type(default_value) == dict:
                # FIXME this is a hoack while we decide how to handle boolean bridge signals
                default_value = 1 if default_value['value'] else 0

        elif self.declaration['type'] == "constant":
                if type(self) == NameAtom:
                    default_value = self.declaration['value']
                elif type(self) == BundleAtom:
                    if type(self.declaration['value']) == list:
                        default_value = [value['value'] for value in self.declaration['value']]
                    else:
                        default_value = [self.declaration['value'] for i in range(self.declaration['size'])]
        elif self.declaration['type'] == "switch":
            if 'default' in self.declaration:
                if type(self) == NameAtom:
                    if type(self.declaration['default']['value']) == dict:
                        default_value = self.declaration['default']['value']['value']
                    else:
                        default_value = self.declaration['default']['value']
                elif type(self) == BundleAtom:
                    if type(self.declaration['default']) == list:
                        default_value = [value['value'] for value in self.declaration['default']]
                    else:
                        default_value = [self.declaration['default'] for i in range(self.declaration['size'])]
            elif 'block' in self.platform_type:
                if 'default' in self.platform_type['block']:
                    default_value = self.platform_type['block']['default'] # FIXME inheritance is not being handled here
                else:
                    default_value = False
            elif 'blockbundle' in self.platform_type:
                #Stride definition
                if 'default' in self.platform_type['blockbundle']:
                    default_value = self.platform_type['blockbundle']['default']
                else:
                    default_value = False
            else:
                print("Forced default value to False for " + self.handle)
                default_value = False
        elif self.declaration['type'] == "trigger":
             default_value = False
        else:
            if 'size' in self.declaration:
                default_value = [0.0 for i in range(self.declaration['size'])]
                print("Forced default value to 0 for " + self.handle)
            else:
                default_value = 0.0
                print("Forced default value to 0 for " + self.handle)
        return default_value



class PortPropertyAtom(Atom):
    def __init__(self, portproperty, platform, scope_index):
        super(PortPropertyAtom, self).__init__()
#        self.rate = -1
#        self.inline = False
#        self.globals = {}
#        self.handle = ''
#        self.domain = None
#        self.writes = 0
#        self.reads = 0

        self.portproperty = portproperty

        self.scope_index = scope_index
        self.platform = platform

        self.handle = "__%s_%s"%(self.portproperty['portname'], self.portproperty['name'])


        parent = platform.parent_stack[-1]
        if parent._input:
            for name, in_block in parent._input.items():
                print(in_block)


        self.resolve_output_port_property(platform.parent_stack, self, self.handle)

        # FIXME we need to look through the input port and also through the property ports

    #        declaration = self.platform.find_declaration_in_tree(self.portproperty['portname'])
    #        if declaration:
    #            if self.portproperty['name'] in declaration:
    #                pass


    def resolve_output_port_property(self, parent_stack, atom, handle):
        portproperty = atom.portproperty
        if len(parent_stack) == 0:
            return None
        parent = parent_stack[-1]
        # TODO add support for instance constants that are not integer

        if parent._output:
            for name, out_block in parent._output.items():
                if isinstance(out_block, NameAtom) or isinstance(out_block, BundleAtom):
                    if portproperty['name'] in out_block.declaration:
                        resolved_value = out_block.declaration[portproperty['name']]
                        if not resolved_value:
                            self.resolve_output_port_property(parent_stack[:-1], atom, handle)
                        else:
                            parent.add_instance_const(handle, 'integer', resolved_value)
                        break
                elif isinstance(out_block, ListAtom):
                # FIXME There needs to be a way to know what member of the list we are actually connectiong to...
                    for node in out_block.list_node:
                        if isinstance(node, NameAtom) or isinstance(node, BundleAtom):
                            if portproperty['name'] in node.declaration:
                                resolved_value = node.declaration[portproperty['name']]
                                if not resolved_value:
                                    self.resolve_output_port_property(parent_stack[:-1], atom, handle)
                                else:
                                    parent.add_instance_const(handle, 'integer', resolved_value)
                elif isinstance(out_block, ModuleAtom):
                    if portproperty['name'] in out_block.function:
                        resolved_value = out_block.function[portproperty['name']]
                        if not resolved_value:
                            self.resolve_output_port_property(parent_stack[:-1], atom, handle)
                        else:
                            parent.add_instance_const(handle, 'integer', resolved_value)
                else:
                    self.platform.log_debug("ERROR: Can't resolve port property value")
        else:
            self.resolve_output_port_property(parent_stack[:-1], atom, handle)

    def get_declarations(self):
        return []

    def get_instances(self):
        return []

    def get_initialization_code(self, in_tokens):
        '''Returns code that should be executed only once per construction of
        the block'''
        return ''

    def get_preprocessing_code(self, in_tokens):
        ''' Returns code that needs to be run asynchronously but can't be
        inlined, so needs to be run separately previous to the processing
        code'''
        return ''

    def get_processing_code(self, in_tokens):
        code = 'XXXX'
        return {None: code}

    def get_inline_processing_code(self, in_tokens):
        ''' This returns the processing code itself, so this can be used
        when the output is used only once, and an intermediate symbol to
        represent it is not needed'''
        return self.handle

class BundleAtom(NameAtom):
    def __init__(self, platform_type, declaration, index, token_index, platform, scope_index, line, filename, previous_atom, next_atom):
        ''' index indexes from 1, internal index from 0
        '''
        super(BundleAtom, self).__init__(platform_type, declaration, token_index, platform, scope_index, line, filename, previous_atom, next_atom)
        self.scope_index = scope_index
        if type(index) == int:
            self.index = index - 1
        else:
            decl = platform.find_declaration_in_tree(index)
            ## FIXME we need to get correct handle for index object
            self.index = '(int)(' + decl['name'] + "-1)"
        self.set_inline(False)
#        if not 'blockbundle' in self.platform_type and not 'platformType' in self.platform_type['block']['type']:
#            raise ValueError("Need a block bundle platform type to make a Bundle Atom.")

    def get_handles(self):
        return [templates.bundle_indexing(self.handle, self.index)]

    def get_instances(self):
        default_values = self._get_default_value()

        if 'default' in self.declaration and signal_type_string(self.declaration):
            instances = [BundleInstance(default_values,
                                 self.scope_index,
                                 self.domain,
                                 'string',
                                 self.handle,
                                 self.declaration['size'],
                                 self) ]

        else:
            if 'size' in self.declaration:
                instances = [BundleInstance([str(default) for default in default_values],
                                     self.scope_index,
                                     self.domain,
                                     'real',
                                     self.handle,
                                     self.declaration['size'],
                                     self) ]


        return instances

    def get_inline_processing_code(self, in_tokens):
        code = ''

        if 'processing' in self.platform_type['block']:
            code = templates.get_platform_inline_processing_code(
                            self.platform_type['block']['processing'],
                            in_tokens,
                            len(self.platform_type['block']['inputs']),
                            len(self.platform_type['block']['outputs']),
                            self.index)
        else:
            if len(in_tokens) > 0:
                code = in_tokens[0]
            else:
                code = self._get_token_name(self.index)
        return  code

    def get_preprocessing_code(self, in_tokens):
        code = ''
        if 'preProcessing' in self.platform_type['block']:
            code = self.platform_type['block']['preProcessing']
            code = templates.get_platform_preprocessing_code(code,
                            in_tokens,
                            len(self.platform_type['block']['inputs']),
                            [self.handle],
                            self.index
                            )
        return code

    def get_processing_code(self, in_tokens):
        code = ''
        out_tokens = [self._get_token_name(self.index)]
        proc_code = self.get_inline_processing_code(in_tokens)
        domain = self.domain
        if len(proc_code) > 0:
            if 'processing' in self.platform_type['block']:
                if self.inline:
                    out_tokens = [proc_code]
                else:
                    if len(self.platform_type['block']['outputs']) > 0:
                        code = templates.assignment(self._get_token_name(self.index), proc_code)
                    else:
                        code = templates.expression(proc_code)
            else:
                for token in out_tokens:
                    code += templates.assignment(token, proc_code)

#        if len(in_tokens) > 0:
#            code = templates.assignment(self._get_token_name(self.index),
#                                        self.get_inline_processing_code(in_tokens))
#
#        if 'processing' in self.platform_type['block']:
#            code = self.get_inline_processing_code(in_tokens)
#
#        out_tokens = [self._get_token_name(self.index)]
        return {domain : [code, out_tokens] }

    def get_postprocessing_code(self, in_tokens):
        code = ''
        if 'postProcessing' in self.platform_type['block']:
            code = self.platform_type['block']['postProcessing']
            code = templates.get_platform_postprocessing_code(code,
                            in_tokens,
                            len(self.platform_type['block']['inputs']),
                            [self.handle],
                            self.index
                            )
        return code

    def _get_token_name(self, index):
        return templates.bundle_indexing(self.handle, index)


class ModuleAtom(Atom):
    def __init__(self, module, function, platform_code, token_index, platform,
                 scope_index, connected_blocks, line, filename,
                 previous_atom, next_atom):
        super(ModuleAtom, self).__init__(line, filename)
        self.scope_index = scope_index
        self.name = module["name"]
        self.handle = self.name + '_%03i'%token_index
        #self._platform_code = platform_code
        self._input_blocks = []
        self._output_blocks = []
        self._index = token_index
        self.platform = platform
        self.module = module
        self.rate = function['rate']
        self.function = function
        self.domain = None
        self.connected_blocks = connected_blocks
        self.instance_consts = {}
        self.input_atom = previous_atom
        self.output_atom = next_atom
        self._input = None
        self._output = None
        self.declaration = None
        self.instance = None

        self.constructor_consts = {}

        self._process_flags()


        if 'domain' in self.function['ports']:
            if 'value' in self.function['ports']['domain']:
                self.domain = self.function['ports']['domain']['value']
            else:
                # FIXME we need to read the value from the name (not get its name)
                self.domain = self.function['ports']['domain']['name']['name']
            #self.function['ports'].pop('domain')

        self.port_name_atoms = {}

        self._process_module(self.module["streams"])

#        if self.code['writes']:
#            for domain, atoms in self.code['writes'].items():
#                print(domain)
#                for atom in atoms:
#                    if atom.get_scope() < self.scope_index:
#                        self.add_instance_const()

        self._prepare_declaration()

        self.set_inline(False)


    def set_inline(self, inline):
        if inline == True:
            print("Warning: Inlining modules not supported")
            return
        self.inline = inline

    def _prepare_declaration(self):
        header_code = ''
        init_code = ''
        process_code = {}

        domain_code = self.code['domain_code']

        for domain, code in domain_code.items():
            if domain is not None: # To get rid of domains from constants
                header_code += code['header_code']
                init_code += code['init_code']
                if not domain in process_code:
                    process_code[domain] = {"code": '', "input_blocks" : [], "output_blocks" : []}

    #            if 'input_blocks' in self.code['domain_code'][domain]:
    #                for input_block in self.code['domain_code'][domain]['input_blocks']:
    #                    if type(input_block.atom) == NameAtom or type(input_block.atom) == BundleAtom:
    #                        process_code[domain]['input_blocks'].append(input_block.atom.declaration)
    #            if 'output_blocks' in self.code['domain_code'][domain]:
    #                for output_block in self.code['domain_code'][domain]['output_blocks']:
    #                    if type(output_block.atom) == NameAtom or type(output_block.atom) == BundleAtom:
    #                        process_code[domain]['output_blocks'].append(output_block.atom.declaration)

                process_code[domain]['code'] += '\n'.join(code['processing_code'])
                for block in self._input_blocks:
                    if block['domain'] == domain:
                        process_code[domain]['input_blocks'].append(block)

                for block in self._output_blocks:
                    if block['domain'] == domain:
                        process_code[domain]['output_blocks'].append(block)

        for const_name, const_info in self.instance_consts.items():
            init_code += templates.assignment(const_name, "_" + const_name)
            header_code += templates.declaration_real(const_name);


        for domain, code in domain_code.items():
            if domain is None: # Process constants
                header_code += code['header_code']
                init_code += code['init_code']


        declaration_text = templates.module_declaration(
                self.name, header_code,
                init_code, process_code,
                self.instance_consts)

        self.code_declaration = Declaration(self.module['stack_index'],
                                        self.domain,
                                        self.name,
                                        declaration_text)

    def get_declarations(self):
        declarations = []
        outer_declarations = []
        secondary_domain = ''
        #FIXME do we need to support more than 1 output block?
        if len(self._output_blocks) > 0:
            secondary_domain = self._output_blocks[0]['domain']

        #FIXME check for scope match
        if "other_scope_declarations" in self.code:
            for other_declaration in self.code["other_scope_declarations"]:
                if not other_declaration.domain or other_declaration.domain == secondary_domain:
                    other_declaration.domain = self.domain
                other_declaration.add_dependent(self.code_declaration)
                outer_declarations.append(other_declaration)
#                self.code_declaration.add_dependent(other_declaration)

        declarations += outer_declarations

        for block in self.connected_blocks:
            declarations += block.get_declarations()

        for const_name, const_info in self.instance_consts.items():
            dec = Declaration(self.module['stack_index'],
                        None,
                        const_name,
                        '')
            declarations.append(dec) #templates.declaration_real(const_name)


        declarations.append(self.code_declaration)
        return declarations

    def get_instances(self):
        instances = []
        instance_consts = []
        for name, info in self.instance_consts.items():
            instance_consts.append(info['value'])
        self.instance = ModuleInstance(self.scope_index,
                                 self.domain,
                                 self.name,
                                 self.handle,
                                 self,
                                 instance_consts
                                 )
        self.code_declaration.add_dependent(self.instance)
        if "other_scope_instances" in self.code:
            for inst in self.code["other_scope_instances"]:
                inst.post = False
                self.code_declaration.add_dependent(inst)
                instances.append(inst)

        for block in self.connected_blocks:
            instances += block.get_instances()

#        for atoms in self.port_name_atoms.values():
#            for atom in atoms:
#                instances += atom.get_instances()
                #FIXME do we need to support multiple output blocks?
        if len(self.out_tokens) > 0:
            block_types = self.get_block_types(self._output_blocks[0])
            token_name = self.out_tokens[0]
            if 'size' in self._output_blocks[0]:
                out_token_name = '_' + self.name + '_%03i_out'%self._index
                token_name = templates.bundle_indexing(out_token_name, self._output_blocks[0]['size'])

            default_value = ''
            instances += [Instance(default_value,
                                 self.scope_index,
                                 self.domain,
                                 block_types[0],
                                 token_name,
                                 self) ]
            self.code_declaration.add_dependent(instances[-1])

        for name, atoms in self.port_name_atoms.items():
            for atom in atoms:
                decl = self.platform.find_declaration_in_tree(atom.get_handles(),
                                                              self.module['blocks'] + self.platform.tree)
                if not decl:
                    if type(atom) is NameAtom:
                        default_value = 0.0
                        if not self.platform.find_instance_by_handle(atom.get_handles()[0], instances):
                            instances += atom.get_instances()
                            instances[-1].add_dependent(self.code_declaration)
                    elif type(atom) is ExpressionAtom:
                        for new_inst in self._get_expression_instances(atom):
                            if not self.platform.find_instance_by_handle(new_inst.get_name(), instances):
                                instances += [new_inst]
                                instances[-1].add_dependent(self.code_declaration)

        instances += [self.instance]
        return instances

    def get_inline_processing_code(self, in_tokens):
        code = ''
        out_tokens = []

        if len(self._output_blocks) > 0:
            if 'size' in self._output_blocks[0]:
                out_tokens = ['_' + self.name + '_%03i_out'%self._index]
            else:
                out_tokens = self.out_tokens

        domain = ''
        for in_block in self._input_blocks:
            domain = in_block['domain']
#            if not in_block['main'] and in_block['domain'] == self._input_blocks[0]['domain']:
#                if 'size' in in_block:
#                    in_tokens.append('_%s_in'%in_block['name'])
#                else:
#                    in_tokens.append(in_block['name'])

        for out_block in self._output_blocks:
            domain = out_block['domain']
#            if not out_block['main']and out_block['domain'] == self._output_blocks[0]['domain']:
#                if 'size' in out_block:
#                    out_tokens.append('_%s_%03i_out'%(out_block['name'], out_block['index']))
#                else:
#                    out_tokens.append(out_block['name'])

        code = templates.module_processing_code(self.handle,
                                                in_tokens,
                                                out_tokens,
                                                domain
                                                )
        return code

    def get_initialization_code(self, in_tokens):
        return self.initialization_code

    def get_preprocessing_code(self, in_tokens):
        code = ''

        # FIXME this needs fixing
#        for in_block in self._input_blocks:
#            if 'size' in in_block:
#                new_code = templates.declaration_bundle_real('_%s_in'%self.handle, len(in_tokens)) + '\n'
#                for i in range(len(in_tokens)):
#                    new_code += templates.assignment('_%s_in[%i]'%(self.handle, i), in_tokens[i])
#                code += new_code
        return code

    def get_processing_code(self, in_tokens):
        code = ''
        out_tokens = self.out_tokens
        domain = self.domain

        # First adjust input tokens to match input block sizes/bundles
        for input_block in self._input_blocks:
            if 'size' in input_block:
                # TODO check type of input ports
                connector_name = '_bundle_connector_' + str(self.platform.unique_id)
                self.platform.unique_id += 1
                code += templates.declaration_bundle_real(connector_name, input_block['size'])
                for i in range(input_block['size']):
                    if len(in_tokens) > 0:
                        code += templates.assignment(templates.bundle_indexing(connector_name, i),
                                                     in_tokens[0])
                        in_tokens.pop(0)
                in_tokens.insert(0, connector_name)


        # Process port domains
        domain_proc_code = {}
        if self.module['ports']:
            for module_port in self.module['ports']:
                if 'block' in module_port:
                    module_block = module_port['block']
                elif 'blockbundle' in module_port:
                    module_block = module_port['blockbundle']
                module_port_name = module_block['name']

                module_port_domain = ''
                block_decl = self.find_internal_block(module_block['block']['name']['name'])
                if block_decl:
                    module_port_domain = block_decl['domain']
                module_port_direction = 'input' if 'Input' in module_block['type'] else 'output'
                for port_atom_name in self.port_name_atoms:
                    # Check if domain name matches and domain has code
                    if port_atom_name == module_port_name:
                        for port_value in self.port_name_atoms[port_atom_name]:
                            # TODO implement for output ports
                            if module_port_direction == 'input':
                                if module_port_domain in self.code['domain_code'].keys():
                                    #if port_atom.
                                    if not module_port_domain in domain_proc_code:
                                        domain_proc_code[module_port_domain] = {'handles': []}
                                    code += templates.assignment(port_value.get_handles()[0], port_value.get_inline_processing_code(port_value.get_handles()))
                                    domain_proc_code[module_port_domain]['handles'] += port_value.get_handles()
                        pass

        for module_port_domain, values  in domain_proc_code.items():
            if module_port_domain == self._output_blocks[0]['domain']:
                in_tokens += values['handles']
            else:
                module_call = templates.module_processing_code(self.handle, values['handles'], [], module_port_domain)
                code += templates.expression(module_call)

        if 'output' in self.module and not self.module['output'] is None: #For Platform types
            if self.inline:
                code += self.get_handles()[0]
            else:
                code += templates.expression(self.get_inline_processing_code(in_tokens))
        else:
            code += templates.expression(self.get_inline_processing_code(in_tokens))
        for port_atom in self.port_name_atoms:
            pass

        return {domain : [code, out_tokens] }

#    def _get_internal_header_code(self):
#        code = self.code['domain_code']['header_code']
#        return code


    def get_postprocessing_code(self, in_tokens):
        code = ''

        # FIXME this needs fixing
#        for in_block in self._input_blocks:
#            if 'size' in in_block:
#                new_code = templates.declaration_bundle_real('_%s_in'%self.handle, len(in_tokens)) + '\n'
#                for i in range(len(in_tokens)):
#                    new_code += templates.assignment('_%s_in[%i]'%(self.handle, i), in_tokens[i])
#                code += new_code
        return code

    def _get_internal_processing_code(self):
        code = self.code['domain_code']['processing_code']
        code += templates.module_output_code(self._output_block)
        return code

    def _process_module(self, streams):

        self._init_blocks(self.module["blocks"])

        self.out_tokens = []
        #FIXME do we need to support more than 1 output block?
        if len(self._output_blocks) > 0:
            if 'size' in self._output_blocks[0]:
                self.out_tokens = ['_' + self.name + '_%03i_out[%i]'%(self._index, i) for i in range(self._output_blocks[0]['size'])]
            else:
                self.out_tokens = ['_' + self.name + '_%03i_out'%self._index]
        if not type(self.module["streams"]) == list:
            # TODO Should this be handled by the code validator before getting here?
            self.module["streams"] = [self.module["streams"]]

        tree = streams
        instanced = []
#        if self._input_block: # Mark input block as instanced so it doesn't get instantiated as other blocks
#            instanced = [[self._input_block['name'], self.scope_index]]


        if 'ports' in self.function:
            for name,port_value in self.function['ports'].items():
                new_atom = self.platform.make_atom(port_value)
                if type(new_atom) == LoopAtom:
                    pass
                else:
                    new_atom.scope_index += 1 # Hack to bring it up to the right scope...
                if name in self.port_name_atoms:
                    self.port_name_atoms[name].append(new_atom)
                else:
                    self.port_name_atoms[name] = [new_atom]
        # FIXME This should be handled with scope index not with this boolean flag
        defer_header = self.module['type'] == 'reaction'
        self.code = self.platform.generate_code(tree, self._blocks,
                                                instanced = instanced,
                                                parent = self,
                                                defer_header = defer_header)

#        if self.module['ports']:
#                for prop in self.module['ports']:
#                    if 'block' in prop:
#                        if type(prop['block']['block']) == dict:
#                            decl = self.platform.find_declaration_in_tree(prop['block']['block']['name']['name'],
#                                                                          self.platform.tree + self._blocks)
#                                     #FIXME implement for bundles
#                            domain = decl['domain']
#                            property_name = prop['block']['name']
#        #                    domain = "_Property:" + property_name
#                            if type(domain) == dict:
#                                domain = domain['name']['name']
#                                # FIXME need to resolve domain name from "name"
#                            prop_type = self.get_block_types(decl)[0]
#                        else:
#                            # Property is not a block but a constant value.
#                            pass


        self.globals = {}
        for section in self.global_sections:
            if section in self.code['global_groups']:
                self.globals[section] = self.code['global_groups'][section]


        # Initialization Code
        # Go through the module ports and check if the function is setting any by
        # value. This means that the call needs to be put into the initialization
        # of the domain

        self.initialization_code = ''
        if self.module['ports']:
            for module_port in self.module['ports']:
                if 'block' in module_port:
                    module_block = module_port['block']
                elif 'blockbundle' in module_port:
                    module_block = module_port['blockbundle']
                module_port_name = module_block['name']

                module_port_domain = ''
                block_decl = self.find_internal_block(module_block['block']['name']['name'])
                if block_decl:
                    module_port_domain = block_decl['domain']
                module_port_direction = 'input' if 'Input' in module_block['type'] else 'output'
                for port_atom_name in self.port_name_atoms:
                    if port_atom_name == module_port_name:
                        for port_value in self.port_name_atoms[port_atom_name]:
                            # TODO implement for output ports
                            if module_port_direction == 'input':
                                if type(port_value) is ValueAtom and not module_port_domain in self.code['domain_code'].keys():
                                    #if port_atom.
                                    module_call = templates.module_processing_code(self.handle, port_value.get_handles(), [], module_port_domain)
                                    self.initialization_code += templates.expression(module_call)
                        pass

        # FIXME this needs fixing
#        for in_block in self._input_blocks:
#            if 'size' in in_block:
#                new_code = templates.declaration_bundle_real('_%s_in'%self.handle, len(in_tokens)) + '\n'
#                for i in range(len(in_tokens)):
#                    new_code += templates.assignment('_%s_in[%i]'%(self.handle, i), in_tokens[i])
#                code += new_code


    def _init_blocks(self, blocks):
        self._blocks = []
        for block in blocks:
            self._blocks.append(block)

        if not self.module['ports']:
            return

        for port in self.module['ports']:
            internal_block = self.find_internal_block(port['block']['block']['name']['name'])
            if port['block'] and 'main' in port['block']['type']:
                internal_block['main'] = True
                internal_block['port_block'] = True
#                for block in self._blocks:
#                    name = ''
#                    if 'block' in block:
#                        name = block['block']['name']
#                    elif 'blockbundle' in block:
#                        name = block['blockbundle']['name']
#                    if internal_block['name'] == name:
#                        self._blocks.remove(block)
#                        break
                if 'Input' in port['block']['type']:
                    self._input_blocks.append(internal_block)
                    self._input = {internal_block['name'] : self.input_atom }
                elif 'Output' in port['block']['type']:
                    self._output_blocks.append(internal_block)
                    self._output = {internal_block['name'] : self.output_atom }
            else: # Not a main port
                internal_block['main'] = False
                internal_block['port_block'] = True
                if 'Input' in port['block']['type']:
                    self._input_blocks.append(internal_block)
                elif 'Output' in port['block']['type']:
                    self._output_blocks.append(internal_block)
#            self._blocks.append(port)


    def find_internal_block(self, block_name):
        for block in self._blocks:
            if 'block' in block and block['block']['name'] == block_name:
                return block['block']
            elif 'blockbundle' in block and block['blockbundle']['name'] == block_name:
                return block['blockbundle']

    def get_block_types(self, block):
        return [templates.get_block_type(block)]

    def _get_expression_instances(self, atom):
        if not type(atom) is ExpressionAtom:
            return []
        instances = []
        left = atom.left_atom
        right = atom.right_atom
        if type(left) is NameAtom:
            instances += left.get_instances()
        elif type(left) is ExpressionAtom:
            instances += self._get_expression_instances(left)

        if type(right) is NameAtom:
            instances += right.get_instances()
        elif type(right) is ExpressionAtom:
            instances += self._get_expression_instances(right)
        return instances

    def add_instance_const(self, name, consttype, value):
        if not name in self.instance_consts:
            self.instance_consts[name] = {'type': consttype, 'value' : value}

    def _process_flags(self):
        if not '_flags' in self.module:
            return
        flags = self.module['_flags']
        for flag in flags:
            pass

class ReactionAtom(ModuleAtom):
    def __init__(self, reaction, function, platform_code, token_index,
                 platform, scope_index, connected_blocks, line, filename,
                 previous_atom, next_atom):
        # We need to force a scope to avoid having declarations and instances
        # generate code in the header. Unlike modules, reactions don't have an
        # internal scope where these things will go, they need to go in the
        # global scope, so their code generation needs to be postponed.
        self.reaction = reaction
        super(ReactionAtom, self).__init__(reaction, function, platform_code, token_index,
             platform, scope_index, connected_blocks, line, filename, previous_atom, next_atom)



    def get_header_code(self):
        domain_code = self.code['domain_code']
        header_code = ''

        for domain,code in domain_code.items():
            header_code += code['header_code']
        return header_code

    def get_inline_processing_code(self, in_tokens):
        code = ''

        code = templates.reaction_processing_code(self.handle,
                                                in_tokens,
                                                self.out_tokens,
                                                'TriggerDomain'
                                                )
        return code


    def get_processing_code(self, in_tokens):
        processing_code = {}

        out_tokens = self.out_tokens

        domain = self.input_atom.domain # Reaction is triggered/called in the domain of the input signal

        parameter_tokens = []
        out_names = [block['name'] for block in self._output_blocks]

        out_tokens = self.out_tokens
        for ref in self.references:
            if not ref.get_name() in out_names:
                parameter_tokens.append(ref.get_name())
            else:
                parameter_tokens.append(out_tokens[0])
#                out_tokens.remove(out_tokens[0])
        for i in range(len(parameter_tokens)):
            # TODO we need checking of scope and domain here
            for scope_decl in self.platform.scope_stack[-1]:
                # Do we need to support blockbundle here or are all signal bridges blocks?
                if 'block' in scope_decl and 'type' in scope_decl['block'] and scope_decl['block']['type'] == "signalbridge":
                    if scope_decl['block']['signal'] == parameter_tokens[i]:
                        print(scope_decl['block']['signal'])
                        parameter_tokens[i] = scope_decl['block']['name']


        if not domain in processing_code:
            processing_code[domain] = ['', []]
        processing_code[domain][0] += "if (" + in_tokens[0] + ") {\n"
        processing_code[domain][0] += templates.expression(
                templates.reaction_processing_code(self.reaction['name'],
                                                parameter_tokens,
                                                [], # out tokens should already be included in the parameter tokens.
                                                domain
                                                ))

        processing_code[domain][0] += "}\n"
        processing_code[domain][1] = out_tokens

        return processing_code

    def _prepare_declaration(self):
        header_code = ''
        init_code = ''
        process_code = {}

        domain_code = self.code['domain_code']
        # out_tokens for reaction are different
        if len(self._output_blocks) > 0:
            if 'size' in self._output_blocks[0]:
                self.out_tokens = ['_' + self.name + '_%03i_out[%i]'%(self._index, i) for i in range(self._output_blocks[0]['size'])]
            else:
                self.out_tokens = ['_' + self.name + '_%03i_out'%self._index]

        self.references = []
        referenced = []
        for domain, writes in self.code['writes'].items():
            for write in writes:
                if not write.get_name() in referenced:
                    referenced += [write.get_name()]
                    self.references.append(write)
        for domain, reads in self.code['reads'].items():
            for read in reads:
                if not read.get_name() in referenced:
                    referenced += [read.get_name()]
                    self.references.append(read)

        for domain, code in domain_code.items():
            if domain is not None: # To get rid of domains from constants
                header_code += code['header_code']
                init_code += code['init_code']
                if not domain in process_code:
                    # Hack to turn all computation in a reaction within the right domain
                    if self.input_atom:
                        # self.input_atom can be None when computing "next_atom"
                        # On next pass it should get the right value
                        # This should be optimized out. When computing "next_atom" not everything needs to

                        domain = self.input_atom.get_domain()
                    process_code[domain] = {"code": '', "input_blocks" : [], "output_blocks" : []}

    #            if 'input_blocks' in self.code['domain_code'][domain]:
    #                for input_block in self.code['domain_code'][domain]['input_blocks']:
    #                    if type(input_block.atom) == NameAtom or type(input_block.atom) == BundleAtom:
    #                        process_code[domain]['input_blocks'].append(input_block.atom.code_declaration)
    #            if 'output_blocks' in self.code['domain_code'][domain]:
    #                for output_block in self.code['domain_code'][domain]['output_blocks']:
    #                    if type(output_block.atom) == NameAtom or type(output_block.atom) == BundleAtom:
    #                        process_code[domain]['output_blocks'].append(output_block.atom.code_declaration)

                process_code[domain]['code'] += '\n'.join(code['processing_code'])
                for block in self._input_blocks:
                    if block['domain'] == domain:
                        process_code[domain]['input_blocks'].append(block)

                for block in self._output_blocks:
                    if block['domain'] == domain:
                        process_code[domain]['output_blocks'].append(block)

        if self.code['reads']:
            for domain, declarations in self.code['reads'].items():
                for decl in declarations:
                    if decl.get_scope() < self.scope_index:
                        self.add_instance_const(decl.get_name(), decl.get_type(), 0)

        for const_name, const_info in self.instance_consts.items():
            init_code += templates.assignment(const_name, "_" + const_name)
            if const_info['type'] == 'real':
                header_code += templates.declaration_real(const_name);
            elif const_info['type'] == 'integer':
                header_code += templates.declaration_int(const_name);
            elif const_info['type'] == 'string':
                header_code += templates.declaration_string(const_name);

        for domain, code in domain_code.items():
            if domain is None: # Process constants
                header_code += code['header_code']
                init_code += code['init_code']

        declaration_text = templates.reaction_declaration(
                self.reaction['name'], header_code,
                init_code, process_code,
                self.references)

        self.code_declaration = Declaration(self.module['stack_index'],
                                        self.domain,
                                        self.name,
                                        declaration_text)

    def get_declarations(self):
        declarations = []
        outer_declarations = []
        secondary_domain = ''
        #FIXME do we need to support more than 1 output block?
        if len(self._output_blocks) > 0:
            secondary_domain = self._output_blocks[0]['domain']

        if "other_scope_declarations" in self.code:
            for other_declaration in self.code["other_scope_declarations"]:
                if not other_declaration.domain or other_declaration.domain == secondary_domain:
                    other_declaration.domain = self.domain
                outer_declarations.append(other_declaration)
                other_declaration.add_dependent(self.code_declaration)

        declarations += outer_declarations

        for block in self.connected_blocks:
            declarations += block.get_declarations()

        for const_name, const_info in self.instance_consts.items():
            dec = Declaration(self.module['stack_index'],
                        None,
                        const_name,
                        '')
            declarations.append(dec) #templates.declaration_real(const_name)

        declarations.append(self.code_declaration)
        return declarations

    def get_instances(self):
        instances = []

        # A reaction is not instanced. It is a function. We only need to return internally generated instances

        port_atom_names = []
        for atoms in self.port_name_atoms.values():
            for atom in atoms:
                port_atom_names += atom.get_handles();

        if self.output_atom:
            port_atom_names += self.output_atom.get_handles()

        # A reaction should trigger no instances from internals. It is stateless.
        if "other_scope_instances" in self.code:
            for inst in self.code["other_scope_instances"]:
                inst.post = False
                inst.add_dependent(self.code_declaration)
#                inst.scope -= 1 # Force instances  and declarations to be deferred to upper scope
                instances.append(inst)
                if inst.get_scope() > self.scope_index and not type(inst) == ModuleInstance:
                    if inst.get_name() in port_atom_names:
                        inst.enabled = False

        for block in self.connected_blocks:
            instances += block.get_instances()


#                instances += atom.get_instances()
# #               FIXME do we need to support multiple output blocks?
        if len(self.out_tokens) > 0:
            block_types = self.get_block_types(self._output_blocks[0])
            token_name = self.out_tokens[0]
            if 'size' in self._output_blocks[0]:
                out_token_name = '_' + self.name + '_%03i_out'%self._index
                token_name = templates.bundle_indexing(out_token_name, self._output_blocks[0]['size'])

            default_value = ''
            instances += [Instance(default_value,
                                 self.scope_index,
                                 self.domain,
                                 block_types[0],
                                 token_name,
                                 self) ]
            self.code_declaration.add_dependent(instances[-1])

        for name, atoms in self.port_name_atoms.items():
            for atom in atoms:
                decl = self.platform.find_declaration_in_tree(atom.get_handles(),
                                                              self.module['blocks'] + self.platform.tree)
                if not decl:
                    if type(atom) is NameAtom:
                        default_value = 0.0
                        if not self.platform.find_instance_by_handle(atom.get_handles()[0], instances):
                            instances += atom.get_instances()
                            instances[-1].add_dependent(self.code_declaration)
                    elif type(atom) is ExpressionAtom:
                        for new_inst in self._get_expression_instances(atom):
                            if not self.platform.find_instance_by_handle(new_inst.get_name(), instances):
                                instances += [new_inst]
                                instances[-1].add_dependent(self.code_declaration)

        return instances

class LoopAtom(ModuleAtom):
    def __init__(self, loop, function, platform_code, token_index,
                 platform, scope_index, connected_blocks, line, filename,
                 previous_atom, next_atom):
        # We need to force a scope to avoid having declarations and instances
        # generate code in the header. Unlike modules, reactions don't have an
        # internal scope where these things will go, they need to go in the
        # global scope, so their code generation needs to be postponed.
        self.loop = loop
        self._on_terminate = loop['terminateWhen']['name']['name']
        super(LoopAtom, self).__init__(loop, function, platform_code, token_index, platform,
             scope_index, connected_blocks, line, filename, previous_atom, next_atom)


    def get_header_code(self):
#        domain_code = self.code['domain_code']
        header_code = ''

        # All header code has been consumed already and put inside the reaction, right?
#        for domain,code in domain_code.items():
#            header_code += code['header_code']
        return header_code

    def get_inline_processing_code(self, in_tokens):
        code = ''

        code = templates.reaction_processing_code(self.handle,
                                                in_tokens,
                                                self.out_tokens,
                                                'TriggerDomain'
                                                )
        return code


    def get_processing_code(self, in_tokens):
        processing_code = {}

        domain = self.input_atom.domain # Loop is triggered/called in the domain of the input signal
        parameter_tokens = [ref.get_name() for ref in self.references]
        for i in range(len(parameter_tokens)):
            # TODO we need checking of scope and domain here
            for scope_decl in self.platform.scope_stack[-1]:
                # Do we need to support blockbundle here or are all signal bridges blocks?
                if 'block' in scope_decl and 'type' in scope_decl['block'] and scope_decl['block']['type'] == "signalbridge":
                    if scope_decl['block']['signal'] == parameter_tokens[i]:
                        print(scope_decl['block']['signal'])
                        parameter_tokens[i] = scope_decl['block']['name']


        if not domain in processing_code:
            processing_code[domain] = ['', []]
        processing_code[domain][0] += "if (" + in_tokens[0] + ") {\n"
        processing_code[domain][0] += templates.expression(
                templates.loop_processing_code(self.loop['name'],
                                                parameter_tokens,
                                                [], # out tokens should already be included in the parameter tokens.
                                                domain
                                                ))

        processing_code[domain][0] += "}\n"

        return processing_code

    def _prepare_declaration(self):
        header_code = ''
        init_code = ''
        process_code = {}

        domain_code = self.code['domain_code']

        self.references = []
        referenced = []
        for domain, writes in self.code['writes'].items():
            # Don't include internal blocks in the input/output args
            if domain: # Is this a good way to identify internal blocks?
                for write in writes:
                    if not write.get_name() in referenced:
                        referenced += [write.get_name()]
                        self.references.append(write)
        for domain, reads in self.code['reads'].items():
            if domain: # Is this a good way to identify internal blocks?
                for read in reads:
                    if not read.get_name() in referenced:
                        referenced += [read.get_name()]
                        self.references.append(read)

        for domain, code in domain_code.items():
            if domain is not None: # To get rid of domains from constants
                header_code += code['header_code']
                init_code += code['init_code']
                if not domain in process_code:
                    # Hack to turn all computation in a reaction within the right domain
                    if self.input_atom:
                        # self.input_atom can be None when computing "next_atom"
                        # On next pass it should get the right value
                        # This should be optimized out. When computing "next_atom" not everything needs to

                        domain = self.input_atom.get_domain()
                    process_code[domain] = {"code": '', "input_blocks" : [], "output_blocks" : []}

    #            if 'input_blocks' in self.code['domain_code'][domain]:
    #                for input_block in self.code['domain_code'][domain]['input_blocks']:
    #                    if type(input_block.atom) == NameAtom or type(input_block.atom) == BundleAtom:
    #                        process_code[domain]['input_blocks'].append(input_block.atom.code_declaration)
    #            if 'output_blocks' in self.code['domain_code'][domain]:
    #                for output_block in self.code['domain_code'][domain]['output_blocks']:
    #                    if type(output_block.atom) == NameAtom or type(output_block.atom) == BundleAtom:
    #                        process_code[domain]['output_blocks'].append(output_block.atom.code_declaration)

                process_code[domain]['code'] += '\n'.join(code['processing_code'])
                for block in self._input_blocks:
                    if block['domain'] == domain:
                        process_code[domain]['input_blocks'].append(block)

                for block in self._output_blocks:
                    if block['domain'] == domain:
                        process_code[domain]['output_blocks'].append(block)

        if self.code['reads']:
            for domain, declarations in self.code['reads'].items():
                for decl in declarations:
                    if decl.get_scope() < self.scope_index:
                        self.add_instance_const(decl.get_name(), decl.get_type(), 0)

        for const_name, const_info in self.instance_consts.items():
            init_code += templates.assignment(const_name, "_" + const_name)
            if const_info.type == 'real':
                header_code += templates.declaration_real(const_name);
            elif const_info.type == 'integer':
                header_code += templates.declaration_int(const_name);
            elif const_info.type == 'string':
                header_code += templates.declaration_string(const_name);

        for domain, code in domain_code.items():
            if domain is None: # Process constants
                header_code += code['header_code']
                init_code += code['init_code']

        condition = "!" + self._on_terminate
        declaration_text = templates.loop_declaration(
                self.loop['name'], header_code,
                condition,
                init_code, process_code,
                self.references)

        self.code_declaration = Declaration(self.module['stack_index'],
                                        self.domain,
                                        self.name,
                                        declaration_text)

    def get_declarations(self):
        declarations = []
        outer_declarations = []
        secondary_domain = ''
        #FIXME do we need to support more than 1 output block?
        if len(self._output_blocks) > 0:
            secondary_domain = self._output_blocks[0]['domain']

        if "other_scope_declarations" in self.code:
            for other_declaration in self.code["other_scope_declarations"]:
                if not other_declaration.domain or other_declaration.domain == secondary_domain:
                    other_declaration.domain = self.domain
                outer_declarations.append(other_declaration)
                other_declaration.add_dependent(self.code_declaration)

        declarations += outer_declarations

        for block in self.connected_blocks:
            declarations += block.get_declarations()

        for const_name, const_info in self.instance_consts.items():
            dec = Declaration(self.module['stack_index'],
                        None,
                        const_name,
                        '')
            declarations.append(dec) #templates.declaration_real(const_name)

        declarations.append(self.code_declaration)
        return declarations

    def get_instances(self):
        instances = []

        # A reaction is not instanced. It is a function. We only need to return internally generated instances

        port_atom_names = []
        for atoms in self.port_name_atoms.values():
            for atom in atoms:
                port_atom_names += atom.get_handles();

        if self.output_atom:
            port_atom_names += self.output_atom.get_handles()

        # A reaction should trigger no instances from internals. It is stateless.
        if "other_scope_instances" in self.code:
            for inst in self.code["other_scope_instances"]:
                inst.post = False
                inst.add_dependent(self.code_declaration)
#                inst.scope -= 1 # Force instances  and declarations to be deferred to upper scope
                instances.append(inst)
                if inst.get_scope() > self.scope_index and not type(inst) == ModuleInstance:
                    if inst.get_name() in port_atom_names:
                        inst.enabled = False

#        for block in self.connected_blocks:
#            instances += block.get_instances()


#                instances += atom.get_instances()
# #               FIXME do we need to support multiple output blocks?
#        if len(self.out_tokens) > 0:
#            block_types = self.get_block_types(self._output_blocks[0])
#            token_name = self.out_tokens[0]
#            if 'size' in self._output_blocks[0]:
#                out_token_name = '_' + self.name + '_%03i_out'%self._index
#                token_name = templates.bundle_indexing(out_token_name, self._output_blocks[0]['size'])
#
#            default_value = ''
#            instances += [Instance(default_value,
#                                 self.scope_index,
#                                 self.domain,
#                                 block_types[0],
#                                 token_name,
#                                 self) ]
#            self.code_declaration.add_dependent(instances[-1])

#        for name, atoms in self.port_name_atoms.items():
#            for atom in atoms:
#                decl = self.platform.find_declaration_in_tree(atom.get_handles(),
#                                                              self.module['blocks'] + self.platform.tree)
#                if not decl:
#                    if type(atom) is NameAtom:
#                        default_value = 0.0
#                        if not self.platform.find_instance_by_handle(atom.get_handles()[0], instances):
#                            instances += atom.get_instances()
#                            instances[-1].add_dependent(self.code_declaration)
#                    elif type(atom) is ExpressionAtom:
#                        for new_inst in self._get_expression_instances(atom):
#                            if not self.platform.find_instance_by_handle(new_inst.get_name(), instances):
#                                instances += [new_inst]
#                                instances[-1].add_dependent(self.code_declaration)

        return instances



# --------------------- Common platform functions
class PlatformFunctions:
    def __init__(self, tree, debug_messages=False):

        self.defined_modules =[]
        self.debug_messages = debug_messages

        self.tree = tree
        self.scope_stack = []
        self.parent_stack = []

        self.sample_rate = 44100 # Set this as default but this should be overriden by platform:

        decl = self.find_declaration_in_tree('PlatformRate')

        if decl:
            self.sample_rate = decl['value']
        templates.domain_rate = self.sample_rate
        self.unique_id = 0

    def log_debug(self, text):
        if self.debug_messages:
            print(text)

    def append_eol(self, text):
        if len(text) > 0 and not text[-1] == '\n':
            text += '\n'


    def find_declaration_in_tree(self, block_name, tree = None):
        if not tree:
            tree = self.tree
        # First look within scope stack
        for i, scope in enumerate(self.scope_stack[::-1]):
            for node in scope:
                if 'block' in node:
                    if node["block"]["name"] == block_name:
                        node["block"]['stack_index'] = len(self.scope_stack) - 1 - i
                        if (not 'namespace' in node['block']) or node['block']['namespace'] == "" or node['block']['namespace'] == templates.framework:
                            return node["block"]
                if 'blockbundle' in node:
                    if node["blockbundle"]["name"] == block_name:
                        node["blockbundle"]['stack_index'] = len(self.scope_stack) - 1 - i
                        if (not 'namespace' in node['blockbundle']) or node['blockbundle']['namespace'] == "" or node['blockbundle']['namespace'] == templates.framework:
                            return node["blockbundle"]
        # Then look for declarations in tree root
        for node in tree:
            if 'block' in node:
                if node["block"]["name"] == block_name:
                    if (not 'namespace' in node['block']) or (node['block']['namespace'] == "") or (node['block']['namespace'] == templates.framework):
                        node["block"]['stack_index'] = 0
                        return node["block"]
            if 'blockbundle' in node:
                if node["blockbundle"]["name"] == block_name:
                    if (not 'namespace' in node['blockbundle']) or node['blockbundle']['namespace'] == "" or node['blockbundle']['namespace'] == templates.framework:
                        node["blockbundle"]['stack_index'] = 0
                        return node["blockbundle"]
#        raise ValueError("Declaration not found for " + block_name)
        return None

    def find_stride_type(self, type_name, tree=None):
        if not tree:
            tree = self.tree
        for element in self.tree:
            if 'block' in element:
                element["block"]['stack_index'] = 0
                if element['block']['type'] == 'module':
                    if element['block']['name'] == type_name:
                        return element
                elif element['block']['type'] == 'type':
                    if element['block']['typeName'] == type_name:
                        return element
                elif element['block']['type'] == 'platformType':
                    if element['block']['typeName'] == type_name:
                        return element
                elif element['block']['type'] == 'platformModule':
                    if element['block']['name'] == type_name:
                        return element

    def find_block(self, name, tree=None):
        if not tree:
            tree = self.tree
        block_declaration = self.find_declaration_in_tree(name, tree)
        if 'type' in block_declaration:
            platform_type = self.find_stride_type(block_declaration["type"])
        else:
            platform_type =  self.find_stride_type(block_declaration["platformType"])
        return platform_type, block_declaration

    def find_port_value(self, object_name, port_name, tree=None):
        if not tree:
            tree = self.tree
        platform_type, block_declaration = self.find_block(object_name, tree)
        # first look in declaration
        for port in block_declaration["ports"]:
            if port == port_name:
                return block_declaration["ports"][port]["value"]
        #then in type for default values
    #    for type in platform_type:
    #        for key in type["ports"]:
    #            if key == "name" and type["ports"]["name"] == port_name:
    #                return type[
        return None



    def find_function_property(self, func, property_name):
        return func["ports"][property_name]

    def find_instance_by_handle(self,handle, instance_list):
        for instance in instance_list:
            if instance.get_name() == handle:
                return instance
        return None

    def get_domain_default_rate(self, domain_name, tree=None):
        rate_node = None
        if not tree:
            tree = self.tree
        for node in tree:
            if 'block' in node:
                if node["block"]["type"] == "_domainDefinition":
                    if "domainName" in node["block"] and node["block"]["domainName"] == domain_name:
                        if "rate" in node["block"]:
                            rate_node = node["block"]["rate"]
                            break

        #if not rate_node:
        #    rate_node = self.find_declaration_in_tree("PlatformRate", tree) # Horrible horrible hack for now
        domain_rate = -1
        if type(rate_node) == dict:
            domain_rate = rate_node['value']
        else:
            domain_rate = rate_node

        return domain_rate


    def bool_to_str(self, bool_val):
        if not type(bool_val) == bool:
            raise ValueError("Only boolean values accepted.")
        elif bool_val:
            return "true"
        else:
            return "false"

    # Code generation functions

    def make_atom(self, member, previous_atom = None, next_atom = None):
        scope_index = len(self.scope_stack) -1
        if "name" in member:
            platform_type, declaration = self.find_block(member['name']['name'], self.tree)
            new_atom = NameAtom(platform_type, declaration, self.unique_id, self, scope_index, member['name']['line'], member['name']['filename'], previous_atom, next_atom)
        elif "bundle" in member:
            platform_type, declaration = self.find_block(member['bundle']['name'], self.tree)
            new_atom = BundleAtom(platform_type, declaration, member['bundle']['index'], self.unique_id, self, scope_index, member['bundle']['line'],member['bundle']['filename'], previous_atom, next_atom)
        elif "function" in member:
            platform_type, declaration = self.find_block(member['function']['name'], self.tree)
            connected_blocks = []
            for port_name, value in member['function']['ports'].items():
                if 'block' or 'bundle' in value:
                    connected_blocks.append(self.make_atom(value))
            if declaration['type'] == 'module':
                if 'type' in platform_type['block']:
                    new_atom = ModuleAtom(declaration, member['function'], platform_type, self.unique_id, self, scope_index, connected_blocks, member['function']['line'], member['function']['filename'],previous_atom, next_atom)
                else:
                    raise ValueError("Invalid or unavailable platform type.")
            elif declaration['type'] == 'reaction':
                new_atom = ReactionAtom(declaration, member['function'], platform_type, self.unique_id, self, scope_index, connected_blocks, member['function']['line'], member['function']['filename'],previous_atom, next_atom)
            elif declaration['type'] == 'loop':
                new_atom = LoopAtom(declaration, member['function'], platform_type, self.unique_id, self, scope_index, connected_blocks, member['function']['line'], member['function']['filename'],previous_atom, next_atom)
            else: # Assume we are using a platform type (allow function notation for platform types)
                # This should be exactly the same as the if "name" in member branch
                platform_type, declaration = self.find_block(member['function']['name'], self.tree)
                new_atom = NameAtom(platform_type, declaration, self.unique_id, self, scope_index, member['function']['line'], member['function']['filename'], previous_atom, next_atom)
        elif "expression" in member:
            if 'value' in member['expression']: # Unary expression
                left_atom = self.make_atom(member['expression']['value'], previous_atom, next_atom)
                right_atom = None
            else:
                left_atom = self.make_atom(member['expression']['left'], previous_atom, next_atom)
                self.unique_id += 1
                right_atom = self.make_atom(member['expression']['right'], previous_atom, next_atom)
            expression_type = member['expression']['type']
            new_atom = ExpressionAtom(expression_type, left_atom, right_atom, self.unique_id, scope_index)
        elif "value" in member:
            new_atom = ValueAtom(member['value'], self.unique_id, scope_index)
        elif "list" in member:
            list_atoms = []
            # FIXME this assumes that atoms connect one to one...
            previous_elements = []
            if previous_atom:
                if isinstance(previous_atom, ListAtom):
                    previous_elements = previous_atom.list_node
                else:
                    previous_elements.append(previous_atom)
            while len(previous_elements) < len(member['list']):
                previous_elements.append(None)
            next_elements = []
            if next_atom:
                if isinstance(next_atom, ListAtom):
                    next_elements = next_atom.list_node
                else:
                    next_elements.append(next_atom)
            while len(next_elements) < len(member['list']):
                next_elements.append(None)
            for element, previous_element, next_element in zip(member['list'], previous_elements, next_elements):
                element_atom = self.make_atom(element, previous_element, next_element)
                list_atoms.append(element_atom)
            domain = self.get_stream_member_domain(member)
            new_atom = ListAtom(list_atoms, scope_index, domain)
#        elif "block" in member:
#            if 'type' in member['block']:
#                platform_type = self.find_stride_type(member['block']["type"])
#            else:
#                platform_type =  self.find_stride_type(member['block']["platformType"])
#            new_atom = NameAtom(platform_type, member['block'], self.unique_id, scope_index)
#        elif "blockbundle" in member:
#            if 'type' in member:
#                platform_type = self.find_stride_type(member['blockbundle']["type"])
#            else:
#                platform_type =  self.find_stride_type(member['blockbundle']["platformType"])
#            new_atom = NameAtom(platform_type, member['blockbundle'], self.unique_id, scope_index)
        elif "portproperty" in member:
            new_atom = PortPropertyAtom(member['portproperty'], self, scope_index)
        else:
            raise ValueError("Unsupported type")
        self.unique_id += 1
        return new_atom

    def push_scope(self, scope, parent):
        if not type(scope) == list:
            raise ValueError("Scopes must be lists")
        self.scope_stack.append(scope)
        self.parent_stack.append(parent)

    def pop_scope(self):
        self.scope_stack.pop()
        self.parent_stack.pop()

    def make_stream_nodes(self, stream):
        node_groups = [[]] # Nodes are grouped by domain
        current_domain = None
        cur_group = node_groups[-1]
        previous_atom = None
        new_atom = None

        for member in stream: #Only instantiate whatever is used in streams. Discard the rest
            previous_atom = new_atom
            next_atom = None
            if 'function' in member or 'expression' in member:
                index = stream.index(member)
                if index < len(stream) - 1:
                    next_atom = self.make_atom(stream[index + 1])
            if 'list' in member:
                index = stream.index(member)
                if index < len(stream) - 1:
                    next_atom = self.make_atom(stream[index + 1])

            new_atom = self.make_atom(member, previous_atom, next_atom)
            if hasattr("domain", "new_atom"):
                if not current_domain == new_atom.domain:
                    current_domain = new_atom.domain
                    # TODO this can be simplified as we can assume that streams belong to the same domain
                    print("New domain!" + current_domain)
                    node_groups.append([])
                    cur_group = node_groups[-1]
            else:
                #print("No domain... Bad.")
                pass
#            self.log_debug("New atom: " + str(new_atom.handle) + " domain: :" + str(current_domain) + ' (' + str(type(new_atom)) + ')')
            cur_group.append(new_atom)

        return node_groups

    def instantiation_code(self, instance):
        if instance.get_type() == 'real':
            code = templates.declaration_real(instance.get_name())
        elif instance.get_type() == 'bool':
            code = templates.declaration_bool(instance.get_name())
        elif instance.get_type() == 'string':
            code = templates.declaration_string(instance.get_name())
        elif instance.get_type() =='bundle':
            if instance.get_bundle_type() == 'real':
                code = templates.declaration_bundle_real(instance.get_name(), instance.size)
            elif instance.get_bundle_type() == 'bool':
                code = templates.declaration_bundle_bool(instance.get_name(), instance.size)
            else:
                raise ValueError("Unsupported bundle type.")
        elif instance.get_type() == 'module':
            code = templates.declaration_module(instance.get_module_type(), instance.get_name(), instance.get_instance_consts())
        elif instance.get_type() == 'reaction':
            code = templates.declaration_reaction(instance.get_module_type(), instance.get_name())
        else:
            raise ValueError('Unsupported type for instance')
#        code += templates.source_marker(instance.get_line(), instance.get_filename())
        return code

    def initialization_code(self, instance):
        code = ''
        if not instance.get_code() == '':
            if instance.get_type() == 'real':
                value = instance.get_code()
                if value:
                    code = templates.assignment(instance.get_name(), value)
            elif instance.get_type() == 'bool':
                value = instance.get_code()
                if value:
                    code = templates.assignment(instance.get_name(), instance.get_code())
            elif instance.get_type() == 'string':
                if instance.get_code():
                    value = '"' + instance.get_code() + '"'
                    code = templates.assignment(instance.get_name(), value)
            elif instance.get_type() == 'bundle':
                for i in range(instance.get_size()):
                    elem_instance = Instance(instance.get_code()[i],
                                             instance.get_scope(),
                                             instance.get_domain(),
                                             instance.get_bundle_type(),
                                             instance.get_name() + '[%i]'%i,
                                             instance)
                    code += self.initialization_code(elem_instance)
            else:
                ValueError("Unsupported type for initialization: " + instance.get_type())
        return code

    def generate_code_from_groups(self, node_groups, global_groups):

        init_code = {}
        header_code = {}
        pre_processing = {}
        processing_code = {}
        post_processing = {}
        writes = {}
        reads = {}
        parent_rates_size = templates.rate_stack_size() # To know now much we need to pop for this stream

#        header = []
        scope_declarations = []
        scope_instances = []
        current_domain = None

        current_rate = -1;

#        self.log_debug(">>>>>>>>")

        for group in node_groups:
            streamdomain = self.get_stream_domain(group)
            current_rate = self.get_domain_default_rate(streamdomain)

            in_tokens = []
            previous_atom = None
            for atom in group:
#                self.log_debug("Processing atom: " + str(atom.handle))
                # TODO check if domain has changed to handle in tokens in domain change
                if atom.domain: # Make "None" domain reuse existing domain
                    if type(atom.domain) == dict:
                        # FIMXE this is for the case where domains are internal
                        # Should have been already resolved by the code validator
                        current_domain = atom.domain['name']['name']
                    else:
                        current_domain = atom.domain

                # Accumulate reads and writes within domains
                if not current_domain in writes:
                    writes[current_domain] = []
                if not current_domain in reads:
                    reads[current_domain] = []

                if group.index(atom) > 0 and (type(atom) == NameAtom or type(atom) == BundleAtom):
                    writes[current_domain] += atom.get_instances()
                if group.index(atom) < len(group) -1 and (type(atom) == NameAtom or type(atom) == BundleAtom):
                    reads[current_domain] += atom.get_instances()

                #Process Inlcudes
                new_globals = atom.get_globals()
                if len(new_globals) > 0:
                    for global_group in new_globals:
                        for new_global in new_globals[global_group]:
                            if not new_global in global_groups[global_group]:
                                global_groups[global_group].append(new_global)

                declares = atom.get_declarations()
                new_instances = atom.get_instances()

                for new_dec in declares:
#                    # add this atom's declaration as a dependent of the existing declaration
#                    if hasattr(atom, "code_declaration") and atom.code_declaration:
#                        new_dec.add_dependent(atom.declaration)
                    scope_declarations.append(new_dec)

                for new_inst in new_instances:
#                    if hasattr(atom, "code_declaration") and atom.code_declaration:
#                        new_inst.add_dependent(atom.declaration)
                    scope_instances.append(new_inst)

                #scope_instances += new_instances
#                header.append([declares, new_instances])

                if not current_domain in header_code:
                    header_code[current_domain] = ""
                if not current_domain in init_code:
                    init_code[current_domain] = ""
                if not current_domain in pre_processing:
                    pre_processing[current_domain] = []
                if not current_domain in processing_code:
                    processing_code[current_domain] = ""
                if not current_domain in post_processing:
                    post_processing[current_domain] = []

                header_code[current_domain] += atom.get_header_code()

                init_code[current_domain] += atom.get_initialization_code(in_tokens)

                if atom.rate and atom.rate > 0:
                    if current_rate == -1 or not current_rate:
                        current_rate = atom.rate
                    elif atom.rate != current_rate:
                        templates.set_domain_rate(self.get_domain_default_rate(current_domain))
                        new_inst, new_init, new_proc = templates.rate_start(atom.rate)
                        processing_code[current_domain] += new_proc
                        header_code[current_domain] += new_inst
                        init_code[current_domain] += new_init
                        # We want to avoid inlining across rate boundaries
                        if previous_atom:
                            previous_atom.set_inline(False)
                        atom.set_inline(False)
                        current_rate = atom.rate

                # Pre-processing-once code
                new_preprocs = atom.get_preproc_once()
                if new_preprocs:
                    for new_preproc in new_preprocs:
                        if new_preproc:
                            preproc_present = False
                            for preproc in pre_processing[current_domain]:
                                if preproc[0] == new_preproc[0]:
                                    preproc_present = True
                                    break
                            # Do we need to order the post processing code?
                            if not preproc_present:
                                pre_processing[current_domain].append(new_preproc)

                # Processing code
                processing_code[current_domain] += atom.get_preprocessing_code(in_tokens)
                self.append_eol(processing_code[current_domain])

                new_processing_code = atom.get_processing_code(in_tokens)

                next_in_tokens = []
                for domain in new_processing_code:
                    code, out_tokens = new_processing_code[domain]
                    #self.log_debug("Code:  " + str(code))
                    if not domain:
                        domain = current_domain
                    if not domain in processing_code:
                        processing_code[domain] = ''
                    processing_code[domain] += code
                    self.append_eol(processing_code[domain])
                    if domain == current_domain or not current_domain:
                        next_in_tokens += out_tokens

                processing_code[current_domain] += atom.get_postprocessing_code(in_tokens)
                self.append_eol(processing_code[current_domain])

                # Post processing code
                processing_code[current_domain] += atom.get_postprocessing_code(in_tokens)
                self.append_eol(processing_code[current_domain])

                # Post processing code
                new_postprocs = atom.get_postproc_once()

                if new_postprocs:
                    for new_postproc in new_postprocs:
                        if new_postproc:
                            postproc_present = False
                            for postproc in post_processing[current_domain]:
                                if postproc[0] == new_postproc[0]:
                                    postproc_present = True
                                    break
                            # Do we need to order the post processing code?
                            if not postproc_present:
                                post_processing[current_domain].append(new_postproc)


                in_tokens = next_in_tokens
                previous_atom = atom



        for domain in pre_processing:
            for preprocdomain in pre_processing[domain]:
                processing_code[domain] = preprocdomain[1] + processing_code[domain]
                self.append_eol(processing_code[domain])

        for domain in post_processing:
            for postprocdomain in post_processing[domain]:
                processing_code[domain] += postprocdomain[1]
                self.append_eol(processing_code[domain])

        # Close pending rates in this stream
        while not parent_rates_size == templates.rate_stack_size():
            # FIXME rates should be closed in their specific domain
            processing_code[current_domain] += templates.rate_end_code()

        return [header_code, init_code, processing_code,
                scope_instances, scope_declarations, reads, writes]

    def generate_stream_code(self, stream, stream_index, global_groups):
#        self.log_debug("----------")
        streamdomain = None

# FIXME Bridge signals throuw this check off... But this check should be here
#        for member in stream:
#            member_domain = self.get_stream_member_domain(member)
#            if not streamdomain == None:
#                if not streamdomain == member_domain:
#                    return None
#            else:
#                streamdomain = member_domain

        domain_decl = self.find_declaration_in_tree(streamdomain)
        if domain_decl and type(domain_decl) == dict:
            decl = None
            domain_framework = domain_decl['framework']
            if domain_framework:
                decl = self.find_declaration_in_tree(domain_decl['framework']['name']['name'])
            if decl and 'frameworkName' in decl:
                framework_name = decl['frameworkName']
            else:
                framework_name = None
        else:
            framework_name = None
        if not framework_name or (framework_name == templates.framework):
            node_groups = self.make_stream_nodes(stream)
            first_line = node_groups[0][0].get_line()
            #last_line = node_groups[0][-1].get_line()
            stream_filename = node_groups[0][0].get_filename()

            new_code = self.generate_code_from_groups(node_groups, global_groups)
            header_code, init_code, new_processing_code, scope_instances, scope_declarations, reads, writes = new_code
        else:
            stream_filename = ''
            header_code, init_code, new_processing_code, scope_instances, scope_declarations, reads, writes = [{} for i in range(7)]
#        self.log_debug("READS------ " + str(reads) )
#        self.log_debug("WRITES------ " + str(writes) )
#        self.log_debug("-- End stream")

        for domain in new_processing_code.keys():
            wrapper_begin = templates.stream_begin_code%stream_index + templates.source_marker(first_line, stream_filename)
            wrapper_end =  templates.stream_end_code%stream_index
            new_processing_code[domain] = wrapper_begin + new_processing_code[domain] + wrapper_end

        return {"global_groups" : global_groups,
                "header_code" : header_code,
                "init_code" : init_code,
                "processing_code" : new_processing_code,
                "scope_instances": scope_instances,
                "scope_declarations": scope_declarations,
                "reads" : reads,
                "writes" : writes
                }

    def get_domains(self):
        domains = []
        for node in self.tree:
            if 'block' in node:
                if 'type' in node['block'] and node['block']['type'] == '_domainDefinition':
                    domains.append(node['block'])
        return domains

    def get_platform_domain(self):
        domain = ''
        declaration = self.find_declaration_in_tree("PlatformDomain", self.tree)
        if declaration:
            domain = declaration['value']

        return domain

    def get_stream_member_domain(self, stream_member):
        member_domain = None
        if "name" in stream_member:
            declaration = self.find_declaration_in_tree(stream_member['name']['name'])
            if declaration and 'domain' in declaration:
                if type(declaration['domain']) == dict:
                    # FIXME this should be set by the code validator
                    domain = declaration['domain']['name']['name']
                else:
                    domain = declaration['domain']
                if not member_domain == None:
                    if not member_domain == domain:
                        return None
                else:
                    member_domain = domain
        elif "bundle" in stream_member:
            declaration = self.find_declaration_in_tree(stream_member['bundle']['name'])
            if declaration and 'domain' in declaration:
                if type(declaration['domain']) == dict:
                    # FIXME this should be set by the code validator
                    domain = declaration['domain']['name']['name']
                else:
                    domain = declaration['domain']
                if not member_domain == None:
                    if not member_domain == domain:
                        return None
                else:
                    member_domain = domain
        elif "function" in stream_member:
            if 'domain' in stream_member['function']['ports']:
                if 'value' in stream_member['function']['ports']['domain']:
                    domain = stream_member['function']['ports']['domain']['value']
                else:
                    # FIXME we need to read the value from the name (not get its name)
                    domain = stream_member['function']['ports']['domain']['name']['name']
                if not member_domain == None:
                    if not member_domain == domain:
                        return None
                else:
                    member_domain = domain
        elif "block" in stream_member:
            declaration =  stream_member['block']
            if declaration and 'domain' in declaration:
                if type(declaration['domain']) == dict:
                    # FIXME this should be set by the code validator
                    domain = declaration['domain']['name']['name']
                else:
                    domain = declaration['domain']
                if not member_domain == None:
                    if not member_domain == domain:
                        return None
                else:
                    member_domain = domain
        elif "expression" in stream_member:
            left_domain = self.get_stream_member_domain(stream_member['expression']['left'])
            right_domain = self.get_stream_member_domain(stream_member['expression']['right'])
            if not left_domain :
                left_domain = right_domain
            elif not right_domain:
                right_domain = left_domain
            if left_domain == right_domain:
                member_domain = left_domain
        elif "value" in stream_member:
            pass
        elif "list" in stream_member:
            for inner_member in stream_member['list']:
                new_domain = self.get_stream_member_domain(inner_member)
                if not member_domain == None:
                    if not member_domain == new_domain:
                        return None
                else:
                    member_domain = new_domain
        return member_domain


    def get_stream_domain(self, atomstream):
        domain = None
        for atom in atomstream:
            atom_domain = atom.get_domain()
            if not domain:
                domain = atom_domain;
            elif atom_domain and not atom_domain == domain:
                self.log_debug("ERROR: Domains don't match within stream!")
                return None
        return domain

    def make_platform_directory(self, strideroot):
        platform_dir = None
        for node in self.tree:
            if "system" in node:
                return node['system']['platforms'][0]['path']


        return platform_dir

    def visit_element(self, element, marked, temp_marked, sorted_list, elements):
        if element in temp_marked:
            print("ERROR! not a DAG.")
            return
        if not element in marked:
            temp_marked.append(element)
            for e in elements:
                if e is not element and element in e.get_dependents():
                    self.visit_element(e, marked, temp_marked, sorted_list, elements)
            marked.append(element)
            temp_marked.remove(element)
            sorted_list.insert(0, element)

    def sort_elements(self, elements):
        sorted_list = []
        marked = []
        temp_marked = []
#        L ← Empty list that will contain the sorted nodes
        for element in elements:
            if not element in marked:
                self.visit_element(element, marked, temp_marked, sorted_list, elements)
        return sorted_list[::-1]

    def generate_code(self, tree, current_scope = [],
                      global_groups = {'include':[], 'includeDir':[], 'initializations' : [], 'linkTo' : [], 'linkDir' : []},
                      instanced = [], parent = None, defer_header = False):
        stream_index = 0

        self.log_debug("* New Generation ----- scopes: " + str(len(self.scope_stack)))
        self.push_scope(current_scope, parent)

        other_scope_instances = []
        other_scope_declarations = []
        scope_declarations = []
        scope_instances = []
        domain_code = {}
        global_groups_code = {}
        writes = {}
        reads = {}

        for node in tree:
            if 'stream' in node: # Everything grows from streams.
                # TODO this can be cleaned up by not passing global_groups to generate_code_stream. It's not needed inside these functions
                # TODO Since the CodeResolver is making sure that streams belong to a single domain, we can simplfy code generation through this assumption
                # TODO processing code does not need to be a list any more?
                code = self.generate_stream_code(node["stream"], stream_index,
                                                 global_groups)
                # merge global groups from different streams...
                for global_section in code['global_groups']:
                    if global_section in global_groups_code:
                        for new_global in code['global_groups'][global_section]:
                            if not new_global in global_groups_code[global_section]:
                                global_groups_code[global_section].append(new_global)
                    else:
                        global_groups_code[global_section] = code['global_groups'][global_section]
                for domain, header_code in code["header_code"].items():
                    if not domain:
                        domain = self.get_platform_domain()
                    if not domain in domain_code:
                        domain_code[domain] = { "header_code": '',
                        "init_code" : '',
                        "processing_code" : [] }
                    domain_code[domain]["header_code"] += header_code

                for domain, init_code in code["init_code"].items():
                    if not domain:
                        domain = self.get_platform_domain()
                    if not domain in domain_code:
                        domain_code[domain] =  { "header_code": '',
                        "init_code" : '',
                        "processing_code" : [] }
                    domain_code[domain]["init_code"] += init_code

                for domain, processing_code in code["processing_code"].items():
                    if not domain:
                        domain = self.get_platform_domain()
                    if not domain in domain_code:
                        domain_code[domain] =  { "header_code": '',
                        "init_code" : '',
                        "processing_code" : [] }
                    domain_code[domain]["processing_code"].append(processing_code)

                scope_declarations += code["scope_declarations"]
                scope_instances += code["scope_instances"]
                stream_index += 1

                reads.update(code['reads'])
                writes.update(code['writes'])

        header_elements = scope_declarations + scope_instances

# Use this line when things are not decalred in the right order on a
# platform and not on another. It's likely due to ordering.
#        header_elements = header_elements[::-1]

        # Remove duplicate elements while keeping dependencies
        clean_list = []
        for new_element in header_elements:
            is_declared = False
            matched_declaration = None
            for d in clean_list:
                # FIXME we need to check scope here to... It's tricky because of the hacks in module, reaction and loop to bring the declarations to the right scope
                if d.get_name() == new_element.get_name(): # and d.get_scope() == new_element.get_scope():
                    is_declared = True
                    matched_declaration = d
                    break
            if not is_declared:
                # FIXME This assigns the platform domain when domain is not specified
                # The right way is to know which domains it is required in.
                # e.g. a module declaration might need to be put in various
                # domains where it is used. For example, each use of a module
                # needs to mark the input and output domains -and perhaps even
                # internally used domains- and then assign here to each of these
                # domains.
                new_element_domain = new_element.get_domain()
                if not new_element_domain:
                    new_element.domain = self.get_platform_domain()
                clean_list.append(new_element)
            else:
                for dep in new_element.get_dependents():
                    if not dep in matched_declaration.get_dependents():
                        matched_declaration.add_dependent(dep)
 #               self.log_debug("Element already queued: " + new_element.get_name())

        # Topological sort https://en.wikipedia.org/wiki/Topological_sorting
        sorted_elements = self.sort_elements(clean_list)

        # Generate code from elements
        for new_element in sorted_elements:
            if not defer_header and (new_element.get_scope() >= len(self.scope_stack) - 1) and new_element.get_enabled(): # if declaration in this scope
#                self.log_debug(':::--- Domain : '+ str(new_element.get_domain()) + ":::" + new_element.get_name() + '::: scope ' + str(new_element.get_scope()) )
                tempdict = new_element.__dict__  # For debugging. This shows the contents in the spyder variable explorer
                #self.log_debug(new_element.get_code())
                if type(new_element) == Declaration or issubclass(type(new_element), Declaration):
                    new_element_domain = new_element.get_domain()
                    if not new_element_domain in domain_code:
                        domain_code[new_element_domain] =  { "header_code": '',
                            "init_code" : '',
                            "processing_code" : [] }
                    domain_code[new_element_domain]['header_code'] += new_element.get_code()
#                    self.log_debug('////// ' + new_element.get_name() + ' // Dependents : '+ ' '.join([e.get_name() for e in new_element.get_dependents()]))

                elif type(new_element) == Instance or issubclass(type(new_element), Instance):
                    new_element_domain = new_element.get_domain()
                    if not new_element.get_domain() in domain_code:
                        domain_code[new_element_domain] =  { "header_code": '',
                            "init_code" : '',
                            "processing_code" : [] }
                    new_inst_code = self.instantiation_code(new_element)
                    domain_code[new_element_domain]["header_code"] += new_inst_code
                    domain_code[new_element_domain]["init_code"] +=  self.initialization_code(new_element)
                    instanced.append(new_element)
#                    self.log_debug('////// ' + new_element.get_name() + ' // Dependents : '+ ' '.join([e.get_name() for e in new_element.get_dependents()]))

            else:
                if type(new_element) == Instance or issubclass(type(new_element), Instance):
                    other_scope_instances.append(new_element)
                else:
                    other_scope_declarations.append(new_element)

        self.pop_scope()
#        self.log_debug("* Ending Generation ----- scopes: " + str(len(self.scope_stack)))

        return {"global_groups" : global_groups_code,
                "domain_code": domain_code,
                "other_scope_instances" : other_scope_instances,
                "other_scope_declarations" : other_scope_declarations,
                "writes" : writes,
                "reads" : reads}

# ----------------------------------------------------------
from subprocess import check_output as ck_out
import platform
import os
import json

class GeneratorBase(object):
    def __init__(self, out_dir = '',
                 strideroot = '',
                 platform_dir = '',
                 tree = None,
                 debug = False):

        self.out_dir = out_dir
        self.strideroot = strideroot
        self.platform_dir = platform_dir
        self.tree = tree

        if os.path.exists(out_dir + "/config.json"):
            configfile = open(out_dir + "/config.json")
            self.config = json.load(configfile)
        else:
            self.config = {}

        self.templates = templates
        self.platform = PlatformFunctions(self.tree, debug)

        self.last_num_outs = 0

        self.written_sections = []

    def log(self, text):
        print(text)

    def write_section_in_file(self, sec_name, code, filename):
        f = open(filename, 'r')
        text = f.read()
        f.close()
    #    log(text)
        start_index = text.find("//[[%s]]"%sec_name)
        end_index = text.find("//[[/%s]]"%sec_name, start_index)
        if start_index <0 or end_index < 0:
            raise ValueError("Error finding [[%s]]  section"%sec_name)
            return
        if sec_name in self.written_sections:
            code = text[start_index + len("//[[%s]]"%sec_name):end_index] + code
        else:
            self.written_sections.append(sec_name)
        text = text[:start_index] + '//[[%s]]\n'%sec_name + code + text[end_index:]
        f = open(filename, 'w')
        f.write(text)
        f.close()

    def write_code(self, code, filename):
        # First write globals to the platform default domain
        domains = self.platform.get_domains()
        globals_code = templates.get_globals_code(code['global_groups'])
        for platform_domain in domains:
            if platform_domain['domainName'] == self.platform.get_platform_domain(): # Platform domain found
                break
        self.write_section_in_file(platform_domain['globalsTag'], globals_code, filename)

        template_init_code = templates.get_config_code()
        config_code = templates.get_configuration_code(code['global_groups']['initializations'])
        self.write_section_in_file(platform_domain['initializationTag'], template_init_code + config_code, filename)
        processing_code = {}

        # Write generated code
        for domain,sections in code['domain_code'].items():
            domain_matched = False
            for platform_domain in domains:
                if platform_domain['domainName'] == domain or not domain:
                    if domain:
                        self.platform.log_debug("--- Domain found:" + domain)
                        #self.platform.log_debug('\n'.join(sections['processing_code']))
                    else:
                        domain = self.platform.get_platform_domain()
                        self.platform.log_debug("--- Domain none.")
                        #self.platform.log_debug('\n'.join(sections['processing_code']))
                    if not domain in processing_code:
                        processing_code[domain] = ""
                    processing_code[domain] += '\n'.join(sections['processing_code'])

                    self.write_section_in_file(platform_domain['declarationsTag'], sections['header_code'], filename)
                    self.write_section_in_file(platform_domain['initializationTag'], sections['init_code'], filename)
                    if 'cleanup_code' in sections:
                        self.write_section_in_file(platform_domain['cleanupTag'], sections['cleanup_code'], filename)
                    domain_matched = True
                    break
            if not domain_matched:
                self.platform.log_debug('WARNING: Domain not matched: ' + str(domain))

        # First insert domain specific code (except processing code that depends on code generation)
        for domain in processing_code:
            for platform_domain in domains:
                if platform_domain['domainName'] == domain: # Check if domain is used in code (perhaps this should be cleanup by by the code resolver instread of having to check here?)
                    if platform_domain['domainIncludes']:
                        inc_text = templates.get_includes_code(platform_domain['domainIncludes'])
                        self.write_section_in_file(platform_domain['declarationsTag'], inc_text, filename)
                    if platform_domain['domainDeclarations']:
                        for declaration in platform_domain['domainDeclarations']:
                            self.write_section_in_file(platform_domain['declarationsTag'], templates.process_code(declaration['value']) + '\n', filename)
                    if platform_domain['domainInitialization']:
                        self.write_section_in_file(platform_domain['initializationTag'], templates.process_code(platform_domain['domainInitialization']) + '\n', filename)
                    if platform_domain['domainCleanup']:
                        self.write_section_in_file(platform_domain['cleanupTag'], templates.process_code(platform_domain['domainCleanup']) + '\n', filename)

        # Now join processing code from code generation with processing function from domain declaration
        for domain in processing_code:
            for platform_domain in domains:
                if platform_domain['domainName'] == domain:
                    code = processing_code[domain]
                    if not platform_domain['domainFunction'] == '':
                        code = platform_domain['domainFunction'].replace("%%domainCode%%", code)

                    self.write_section_in_file(platform_domain['processingTag'], code, filename)


    def make_code_pretty(self):
        if platform.system() == "Linux":
            try:
                self.log("Running astyle...")
                ck_out(['astyle', self.out_file ])
            except:
                self.log("Error running astyle!")
        elif platform.system() == "Darwin":
            try:
                self.log("Running astyle...")
                ck_out(['/usr/local/bin/astyle', self.out_file ])
            except:
                self.log("Error running astyle!")
        else:
            self.log("Platform '%s' not supported!"%platform.system())\

    def run(self):
        pass

    def stop(self):
        pass

    def custom_command(self, command):
        self.log("Custom command:" + command)

if __name__ == '__main__':
    pass
