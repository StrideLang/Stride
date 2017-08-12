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


try:
    unicode_exists_test = type('a') == unicode
except:
    unicode = str # for python 3

import re

class BaseCTemplate(object):
    def __init__(self):

        self.properties = {}
        self.included = [] # Accumulates include statements

        self.rate_stack = []
        self.rate_nested = 0
        self.rate_counter = 0
        self.domain_rate = None

        self.str_true = "true"
        self.str_false = "false"
        self.stream_begin_code = '// Starting stream %02i -------------------------\n ' #{\n'
#        self.stream_end_code = '} // Stream End %02i\n'
        self.stream_end_code = '// Stream End %02i'

        self.string_type = "std::string"
        self.real_type = 'float'
        self.bool_type = 'bool'
        self.int_type = 'int'

        # Internal templates
        self.str_rate_begin_code = '{ // Start new rate %f old: %f\n'
        self.str_rate_end_code = '\n}  // Close Rate %i\n'
        self.str_assignment = '%s = %s;\n'
        self.str_increment = '%s += %s;\n'
        self.str_module_declaration = '''

class %s {
public:
    %s
    %s(%s) {
        %s
    }
    %s
};
'''

        self.str_function_declaration = '''%s %s(%s) {
        %s
    }
'''
        self.str_while_declaration = '''while (%s) {
        %s
    }
'''

        pass


    def process_code(self, code):
        ''' This function should be overridden to do text replacement for hardware properties '''
        return code

    def source_marker(self, line, filename):
        if line == -1:
            marker = ''
        else:
            marker = "#line " + str(line) + ' "' + filename + '"\n'
        return marker

    def number_to_string(self, number):
        if type(number) == int:
            s = '%i;\n'%number
        elif type(number) == float:
            s = '%.8f;'%number
        else:
            raise ValueError(u"Unsupported type '%s' in assignment."%type(number).__name__)
        return s;

    def get_platform_initialization_code(self, code, token_names, num_inputs, out_tokens, bundle_index = -1):
        p = re.compile("%%intoken:[a-zA-Z0-9_]+%%") ## TODO tweak this better
        matches = p.findall(code)
        if num_inputs > 0: # has inputs
            if bundle_index >= 0:
                code = code.replace('%%bundle_index%%', str(bundle_index))
            for match in matches:
                index = int(match[match.rfind(":") + 1:-2])
                code = code.replace(match, token_names[index])
                code = code.replace('%%bundle_index%%', str(bundle_index))
        else: # Output only
            if bundle_index >= 0:
                code = code.replace('%%bundle_index%%', str(bundle_index))
        for token in out_tokens:
            code = code.replace('%%token%%', token)
        return code

    def get_platform_preprocessing_code(self, code, token_names, num_inputs, out_tokens, bundle_index = -1):
        p = re.compile("%%intoken:[a-zA-Z0-9_]+%%") ## TODO tweak this better
        matches = p.findall(code)
        if num_inputs > 0: # has inputs
            if bundle_index >= 0:
                code = code.replace('%%bundle_index%%', str(bundle_index))
            for match in matches:
                index = int(match[match.rfind(":") + 1:-2])
                code = code.replace(match, token_names[index])
                code = code.replace('%%bundle_index%%', str(bundle_index))
        else: # Output only
            if bundle_index >= 0:
                code = code.replace('%%bundle_index%%', str(bundle_index))
        for token in out_tokens:
            code = code.replace('%%token%%', token)
        return code

    def get_platform_postprocessing_code(self, code, token_names, num_inputs, out_tokens, bundle_index = -1):
        p = re.compile("%%intoken:[a-zA-Z0-9_]+%%") ## TODO tweak this better
        matches = p.findall(code)
        if num_inputs > 0: # has inputs
            if bundle_index >= 0:
                code = code.replace('%%bundle_index%%', str(bundle_index))
            for match in matches:
                index = int(match[match.rfind(":") + 1:-2])
                code = code.replace(match, token_names[index])
                code = code.replace('%%bundle_index%%', str(bundle_index))
        else: # Output only
            if bundle_index >= 0:
                code = code.replace('%%bundle_index%%', str(bundle_index))
        for token in out_tokens:
            code = code.replace('%%token%%', token)
        return code


    def get_platform_inline_processing_code(self, code, token_names, num_inputs, num_outputs, bundle_index = -1):
        p = re.compile("%%intoken:[a-zA-Z0-9_]+%%") ## TODO tweak this better
        matches = p.findall(code)
        if num_inputs > 0: # has inputs
            if bundle_index >= 0:
                code = code.replace('%%bundle_index%%', str(bundle_index))
            for match in matches:
                index = int(match[match.rfind(":") + 1:-2])
                code = code.replace(match, token_names[index])
                code = code.replace('%%bundle_index%%', str(bundle_index))
        else: # Output only
            if bundle_index >= 0:
                code = code.replace('%%bundle_index%%', str(bundle_index))
        return code

    def get_platform_processing_code(self, code, token_names, handle, num_inputs, num_outputs, bundle_index = -1, prop_tokens = {}):
        code = self.get_platform_inline_processing_code(code, token_names, num_inputs, num_outputs,
                                                        bundle_index, prop_tokens)
        if num_outputs > 0:
            code = code.replace('%%token%%', handle)
        return code

    def declaration_bundle_real(self, name, size, close=True):
        declaration = self.real_type + " %s[%i]"%(name, size)
        if close:
            declaration += ';\n'
        return declaration

    def declaration_bundle_int(self, name, size, close=True):
        declaration = self.int_type + " %s[%i]"%(name, size)
        if close:
            declaration += ';\n'
        return declaration

    def declaration_bundle_bool(self, name, size, close=True):
        declaration = self.bool_type + " %s[%i]"%(name, size)
        if close:
            declaration += ';\n'
        return declaration

    def declaration_bundle_string(self, name, size, close=True):
        declaration = self.string_type + " %s[%i]"%(name, size)
        if close:
            declaration += ';\n'
        return declaration

    def declaration(self, block, close=True):
        declaration = ''
        vartype = self.get_block_type(block)
        if 'block' in block:
            block = block['block']
        elif 'blockbundle' in block:
            block = block['blockbundle']
        name = block['name']
        if 'size' in block:
            if vartype == 'real':
                declaration = self.declaration_bundle_real(name, block['size'], close)
            elif vartype == 'string':
                declaration = self.declaration_bundle_string(name, block['size'],close)
            elif vartype == 'bool':
                declaration = self.declaration_bundle_bool(name, block['size'],close)
            elif vartype == 'int':
                declaration = self.declaration_bundle_int(name, block['size'],close)
        else:
            if vartype == 'real':
                declaration = self.declaration_real(name, close)
            elif vartype == 'string':
                declaration = self.declaration_string(name, close)
            elif vartype == 'bool':
                declaration = self.declaration_bool(name, close)
            elif vartype == 'int':
                declaration = self.declaration_int(name, close)

        return declaration

    def declaration_reference(self, block, close=True):
        declaration = ''
        vartype = self.get_block_type(block)
        name = block['name']
        if 'size' in block:
            if vartype == 'real':
                declaration = self.declaration_bundle_real(name, block['size'], close)
            elif vartype == 'string':
                declaration = self.declaration_bundle_string(name, block['size'],close)
            elif vartype == 'bool':
                declaration = self.declaration_bundle_bool(name, block['size'],close)
            elif vartype == 'int':
                declaration = self.declaration_bundle_int(name, block['size'],close)
        else:
            name = "&" + name
            if vartype == 'real':
                declaration = self.declaration_real(name, close)
            elif vartype == 'string':
                declaration = self.declaration_string(name, close)
            elif vartype == 'bool':
                declaration = self.declaration_bool(name, close)
            elif vartype == 'int':
                declaration = self.declaration_int(name, close)
        return declaration

    def declaration_reference_from_instance(self, instance, close=True):
        declaration = ''
        vartype = instance.get_type()
        name = instance.get_name()
        # FIXME support bundles
        if vartype == 'bundle':
            vartype = instance.get_bundle_type()
            if vartype == 'real':
                declaration = self.declaration_real(name, close)
            elif vartype == 'string':
                declaration = self.declaration_string(name, close)
            elif vartype == 'bool':
                declaration = self.declaration_bool(name, close)
            elif vartype == 'int':
                declaration = self.declaration_int(name, close)
            declaration = self.bundle_indexing(declaration, instance.get_size())
        else:
            name = "&" + name
            if vartype == 'real':
                declaration = self.declaration_real(name, close)
            elif vartype == 'string':
                declaration = self.declaration_string(name, close)
            elif vartype == 'bool':
                declaration = self.declaration_bool(name, close)
            elif vartype == 'int':
                declaration = self.declaration_int(name, close)
            else:
                throw(ValueError("Unsupported type for reference"))
        return declaration

    def declaration_real(self, name, close=True, default = None):
        declaration = self.real_type + " " + name
        if default:
            declaration += ' = ' + str(default)
        if close:
            declaration += ';\n'
        return declaration

    def declaration_int(self, name, close=True, default = None):
        declaration = self.int_type + " " + name
        if default:
            declaration += ' = ' + str(default)
        if close:
            declaration += ';\n'
        return declaration

    def declaration_bool(self, name, close=True, default = None):
        declaration = self.bool_type + " " + name
        if default:
            declaration += ' = ' + str(default)
        if close:
            declaration += ';\n'
        return declaration

    def declaration_string(self, name, close=True, default = None):
        declaration = self.string_type + " " + name
        if default:
            declaration += ' = ' + str(default)
        if close:
            declaration += ';\n'
        return declaration

    def declaration_module(self, moduletype, handle, instance_consts = [], close=True):
        declaration = moduletype + ' ' + handle
        if len(instance_consts) > 0:
            declaration += '{'
            for value in instance_consts:
                declaration += str(value)
            declaration += '}'
        if close:
            declaration += ';\n'
        return declaration

    def declaration_reaction(self, reactiontype, handle, close=True):
        declaration = reactiontype + ' ' + handle
        if close:
            declaration += ';\n'
        return declaration


    def declaration_const_real(self, name, default, close=True):
        declaration = "const " + self.real_type + " " + name + " = " + str(default)
        if close:
            declaration += ';\n'
        return declaration

    def value_real(self, value):
        value = str(value)
        return value

    def value_bool(self, value):
        return self.str_true if value else self.str_false

    def bundle_indexing(self, bundle_name, index):
        if type(index) == int:
            return '%s[%i]'%(bundle_name, index)
        else:
            return '%s[%s]'%(bundle_name, str(index))

    def expression(self, expression):
        return expression + ';\n'

    def assignment(self, assignee, value):
        code = ''
        if not type(value) == str and not type(value) == unicode:
            value = self.number_to_string(value)
        if not value == assignee:
            code = self.str_assignment%(assignee, value)
        return code

    def increment(self, assignee, value):
        if not type(value) == str:
            value = self.number_to_string(value)
        code = self.str_increment%(assignee, value)
        return code

    def get_block_type(self, block):
        if 'block' in block:
            block = block['block']
        elif 'blockbundle' in block:
            block = block['blockbundle']
        if block['type'] == 'signal':
            if 'default' in block:
                if type(block['default']) == unicode:
                    return 'string'

            return 'real'
        elif block['type'] == 'switch':
            return 'bool'
        elif block['type'] == 'trigger':
            return 'bool'
        elif block['type'] == 'Unsupported':
            return 'real'

    def get_globals_code(self, global_groups):
        code = ''
        self.included = []
        for group in global_groups:
            if group == 'include':
                code += self.includes_code(global_groups['include'])
        return code

    def get_includes_code(self, includes):
        code = ''
        code += self.includes_code(includes)
        return code

    def get_configuration_code(self, inits):
        init_code = ''
        configured = []
        for elem in inits:
            if not elem in configured and type(elem) == unicode:
                init_code += elem + '\n'
                configured.append(elem)
        return init_code

    def includes_code(self, includes):
        includes_code = ''
        for include in includes:
            if not include in self.included:
                if type(include) == dict and 'value' in include:
                    include = include['value']
                if not include == '':
                    includes_code += "#include <%s>\n"%include
                    self.included.append(include)
        return includes_code

    # Handling of rate changes within a stream -------------------------------
    def rate_init_code(self):
        code = ''
        rate = self.rate_stack[-1]
        index = self.rate_counter
        if not rate == self.domain_rate:
            if rate < self.domain_rate:
                code = '_counter_%03i = 1.0;\n'%(index)
            else:
                code = '_counter_%03i = 0.0;\n'%(index)
        return code

    def rate_instance_code(self):
        code = ''
        rate = self.rate_stack[-1]
        index = self.rate_counter
        if not rate == self.domain_rate:
            code = 'float _counter_%03i;\n'%(index)
        return code

    def rate_start(self, rate):
        inst_code = ''
        init_code = ''
        proc_code = self.rate_end_code()
        if not rate == self.domain_rate:
            self.rate_stack.append(rate)
            inst_code = self.rate_instance_code()
            init_code =  self.rate_init_code()
            proc_code += self.rate_start_code()
            self.rate_nested += 1
        return inst_code, init_code, proc_code

    def rate_start_code(self):
        code = ''
        rate = self.rate_stack[-1]
        if len(self.rate_stack) > 1:
            parent_rate = self.rate_stack[-2]
        else:
            parent_rate = self.domain_rate
        index = self.rate_counter
        if not rate == parent_rate:
            if rate < parent_rate:
                code += self.str_rate_begin_code%(rate, parent_rate)
                code += 'if (_counter_%03i >= 1.0) {\n_counter_%03i -= 1.0;\n'%(index, index)
            else:
                code += self.str_rate_begin_code%(rate, parent_rate)
                code += 'while (_counter_%03i < 1.0) {\n'%(index)
            self.rate_nested += 1
        return code

    def rate_stack_size(self):
        return len(self.rate_stack)

    def rate_end_code(self):
        if len(self.rate_stack) > 0:
            code = ''
            rate = self.rate_stack.pop()
            index = self.rate_counter
            if len(self.rate_stack) > 1:
                parent_rate = self.rate_stack[-2]
            else:
                parent_rate = self.domain_rate
            if rate < parent_rate:
                code += '}\n' # Closes counter check above
                code += self.str_rate_end_code%rate
                code += '_counter_%03i += %.10f;\n'%(index, float(rate)/parent_rate)
            else:
                code += '_counter_%03i += %.10f;\n'%(index, parent_rate/ float(rate))
                code += '}\n' # Closes counter check above
                code += '_counter_%03i -= 1.0;\n'%(index)
                code += self.str_rate_end_code%rate
            self.rate_counter += 1
            return code
        else:
            return ''

    # Module code ------------------------------------------------------------
    def module_declaration(self, name, header_code, init_code, process_code, instance_consts = {}):

        out_type = 'void'

        process_functions = ''
        constructor_args = ''
        for domain, domain_components in process_code.items():
            domain_proc_code = domain_components['code']
            input_declaration = ''
            for input_block in domain_components['input_blocks']:
    #            if 'blockbundle' in input_block:
    #                for i in range(input_block['blockbundle']['size']):
    #                    block_type = self.get_block_type(input_block)
    #                    block_name = input_block['blockbundle']['name']
    #                    if block_type == 'real':
    #                        input_declaration += self.declaration_real(block_name + str(i), close = False) + ", "
    #                input_declaration = input_declaration[:-2]
    #            else:
                input_declaration += self.declaration(input_block, close = False) + ", "
            for output_block in domain_components['output_blocks']:
                input_declaration +=  self.declaration_reference(output_block, False) + ", "

            if len(input_declaration) > 0:
                input_declaration = input_declaration[:-2]

            process_functions += self.str_function_declaration%(out_type, 'process_' + str(domain), input_declaration, domain_proc_code)

        for const_name, props in instance_consts.items():
            constructor_args += "float _" + const_name + ","
        if len(constructor_args) > 0 and constructor_args[-1] == ',':
            constructor_args = constructor_args[:-1]
        declaration = self.str_module_declaration%(name, header_code, name, constructor_args, init_code, process_functions)
        return declaration

    def module_set_property(self, handle, port_name, in_tokens):
        code = handle + '.set_' + port_name + '(' + in_tokens[0] + ');'
        return code

    def module_processing_code(self, handle, in_tokens, out_tokens, domain_name):
        code = handle + '.process_' + str(domain_name) + '('
        for in_token in in_tokens:
            code += in_token + ", "

        for out_token in out_tokens:
            code += out_token + ", "
        if (len(in_tokens) > 0 and len(out_tokens) == 0) or (len(out_tokens) > 0):
            code = code[:-2] # Chop off extra comma
        code += ')'
        return code

    def module_output_code(self, output_block):
        code = ''
        if output_block and 'block' in output_block:
            block_type = 'block'
        else:
            block_type = 'blockbundle'
        if not block_type == 'blockbundle' : #When a bundle, then output is passed as reference in the arguments
            code += 'return %s;\n'%(output_block['block']['name'])
        return code

    # Reactions code

    def reaction_declaration(self, name, header_code, init_code,
                             process_code, references = []):

        out_type = 'void'

        process_functions = ''
        declared_references = []
#        constructor_args = ''
        for domain, domain_components in process_code.items():
            domain_proc_code = domain_components['code']
            input_declaration = ''
#            for output_block in domain_components['output_blocks']:
#                input_declaration +=  self.declaration_reference(output_block, False) + ", "

            for ref in references:
                if not ref.get_name() in declared_references:
                    input_declaration +=  self.declaration_reference_from_instance(ref, False) + ", "
                    declared_references.append(ref.get_name())

            if len(input_declaration) > 0:
                input_declaration = input_declaration[:-2]


            process_functions += self.str_function_declaration%(out_type, name + '_process_' + str(domain), input_declaration, domain_proc_code)
#
#        for const_name, props in instance_consts.items():
#            constructor_args += "float _" + const_name + ","
#        if len(constructor_args) > 0 and constructor_args[-1] == ',':
#            constructor_args = constructor_args[:-1]
        declaration = process_functions
        return declaration


    def reaction_processing_code(self, reaction_name, in_tokens, out_tokens, domain_name):
#        code = handle + '.process_' + str(domain_name) + '('
        code =  reaction_name + '_process_' + str(domain_name) + '('
        for in_token in in_tokens:
            code += in_token + ", "

        for out_token in out_tokens:
            code += out_token + ", "
        if (len(in_tokens) > 0 and len(out_tokens) == 0) or (len(out_tokens) > 0):
            code = code[:-2] # Chop off extra comma
        code += ')'
        return code
        return code

    def loop_declaration(self, name, header_code, condition, init_code,
                             process_code, references = []):

        out_type = 'void'

        process_functions = ''
        declared_references = []
#        constructor_args = ''
        for domain, domain_components in process_code.items():
            domain_proc_code = domain_components['code']
            input_declaration = ''
#            for output_block in domain_components['output_blocks']:
#                input_declaration +=  self.declaration_reference(output_block, False) + ", "

            for ref in references:
                if not ref.get_name() in declared_references:
                    input_declaration +=  self.declaration_reference_from_instance(ref, False) + ", "
                    declared_references.append(ref.get_name())

            if len(input_declaration) > 0:
                input_declaration = input_declaration[:-2]


            internal_loop = self.str_while_declaration%(condition, domain_proc_code)

            process_functions += self.str_function_declaration%(out_type, name + '_process_' + str(domain),
                                                                input_declaration, header_code + init_code + internal_loop)
#
#        for const_name, props in instance_consts.items():
#            constructor_args += "float _" + const_name + ","
#        if len(constructor_args) > 0 and constructor_args[-1] == ',':
#            constructor_args = constructor_args[:-1]
        declaration = process_functions
        return declaration


    def loop_processing_code(self, reaction_name, in_tokens, out_tokens, domain_name):
#        code = handle + '.process_' + str(domain_name) + '('
        code =  reaction_name + '_process_' + str(domain_name) + '('
        for in_token in in_tokens:
            code += in_token + ", "

        for out_token in out_tokens:
            code += out_token + ", "
        if (len(in_tokens) > 0 and len(out_tokens) == 0) or (len(out_tokens) > 0):
            code = code[:-2] # Chop off extra comma
        code += ')'
        return code
        return code

    # Configuration code -----------------------------------------------------
    def get_config_code(self):
        config_template_code = '''
        '''

        return config_template_code

    def set_domain_rate(self, rate):
        self.domain_rate = rate

    def set_property(self, name, value):
        self.properties[name] = value

    def set_properties(self, properties):
        if type(properties) == dict:
            self.properties = properties

