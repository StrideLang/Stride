# -*- coding: utf-8 -*-
"""
Created on Tue Apr 26 18:38:44 2016

@author: andres
"""

from __future__ import print_function

import re

class BaseCTemplate(object):
    def __init__(self, domain_rate = 44100):
        
        self.rate_stack = []
        self.rate_nested = 0
        self.rate_counter = 0
        self.domain_rate = domain_rate # TODO set this from domain configuration
        
        self.str_true = "true"
        self.str_false = "false"
        self.stream_begin_code = '// Starting stream %02i -------------------------\n{\n'
        self.stream_end_code = '} // Stream End %02i\n'
        
        self.string_type = "std::string"
        self.real_type = 'float'
        self.bool_type = 'bool'
        self.int_type = 'int'
        
        # Internal templates
        self.str_rate_begin_code = '{ // Start new rate %i\n' 
        self.str_rate_end_code = '\n}  // Close Rate %i\n' 
        self.str_assignment = '%s = %s;\n'
        self.str_increment = '%s += %s;\n'
        self.str_module_declaration = '''
        
struct %s {
    %s %s() {
        %s
    }
    %s process(%s) {
        %s
    }
};
'''        
        self.str_reaction_declaration = '''
struct %s {
    %s %s() {
        %s
    }
    %s execute(%s) {
        %s
    }
};
'''

        pass
    
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
        
    def declaration_real(self, name, close=True):
        declaration = self.real_type + " " + name
        if close:
            declaration += ';\n'
        return declaration
        
    def declaration_int(self, name, close=True):
        declaration = self.int_type + " " + name
        if close:
            declaration += ';\n'
        return declaration 
        
    def declaration_bool(self, name, close=True):
        declaration = self.bool_type + " " + name
        if close:
            declaration += ';\n'
        return declaration 
        
    def declaration_string(self, name, close=True):
        declaration = self.string_type + " " + name
        if close:
            declaration += ';\n'
        return declaration  
        
    def declaration_module(self, moduletype, handle, close=True):
        declaration = moduletype + ' ' + handle
        if close:
            declaration += ';\n'
        return declaration 
        
    def declaration_reaction(self, reactiontype, handle, close=True):
        declaration = reactiontype + ' ' + handle
        if close:
            declaration += ';\n'
        return declaration
        
    def value_real(self, value):
        value = str(value)
        return value
        
    def bundle_indexing(self, bundle_name, index):
        return bundle_name + '[' + str(index - 1) + ']'
        
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
        self.included = []
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
                code += self.str_rate_begin_code%rate
                code += 'if (_counter_%03i >= 1.0) {\n_counter_%03i -= 1.0;\n'%(index, index)
            else:
                code += self.str_rate_begin_code%rate
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
    def module_declaration(self, name, header_code, init_code,
                           output_block, input_block, process_code):
        input_declaration = ''
        if input_block:
#            if 'blockbundle' in input_block:
#                for i in range(input_block['blockbundle']['size']):
#                    block_type = self.get_block_type(input_block)
#                    block_name = input_block['blockbundle']['name']
#                    if block_type == 'real':
#                        input_declaration += self.declaration_real(block_name + str(i), close = False) + ", "
#                input_declaration = input_declaration[:-2]
#            else:
            input_declaration = self.declaration(input_block, close = False)
            if output_block:
                input_declaration +=  ", " + self.declaration_reference(output_block, False)
        else: # No input, only output
            input_declaration = ''                              
            if output_block:
                input_declaration += self.declaration_reference(output_block, False)
        out_type = 'void'
            
        declaration = self.str_module_declaration%(name, header_code, name, init_code,
                                                   out_type, input_declaration, process_code)
        return declaration

    def module_set_property(self, handle, port_name, in_tokens):
        code = handle + '.set_' + port_name + '(' + in_tokens[0] + ');'
        return code
        
    def module_processing_code(self, handle, in_tokens, out_token):
        code = handle + '.process('
        for in_token in in_tokens:
            code += in_token + ", "
            
        if not out_token == '':
            code += out_token
        elif len(in_tokens) > 0:
            code = code[:-2] # Chop off extra comma
        code += ')'
        return code
        
    def module_property_setter(self, name, block_name, prop_type):
        code = ''
        if prop_type == 'real':
            code += 'void set_' + name + '(float value) {\n'
            code += block_name + ' = value;\n'
            code += '\n}\n'
        elif prop_type == 'bool':
            code += 'void set_' + name + '(bool value) {\n'
            code += block_name + ' = value;\n'
            code += '\n}\n'
        elif prop_type == 'string':
            code += 'void set_' + name + '(' + self.string_type + ' value) {\n'
            code += block_name + ' = value;\n'
            code += '\n}\n'
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
                           output_block, process_code):
        if output_block:
            if 'block' in output_block:
                output_block = output_block['block']
            elif 'blockbundle' in output_block:
                output_block = output_block['blockbundle']
            if output_block['type'] == 'signal':
                if 'size' in output_block:
                    out_type = 'void'
                else:
                    if type(output_block['default']) == unicode:
                        out_type = self.string_type
                    else:
                        out_type = self.real_type
            elif output_block['type'] == 'switch':
                out_type = 'bool'
            else:
                raise ValueError("Unknown type")
            out_declaration = self.declaration_reference(output_block, False)          
        else:
            out_type = 'void'
            out_declaration = ''
            
        declaration = self.str_reaction_declaration%(name, header_code, name, init_code,
                                                     out_type, out_declaration, process_code)
        return declaration        
        
    def reaction_processing_code(self, handle, in_tokens, out_token):
        code = "if ("+ in_tokens[0] + ") {\n"
        if out_token:
            code += handle + '.execute(' + out_token +  ');\n'
        else:
            code += handle + '.execute();\n'
        code += "}\n"
        return code
        
    # Configuration code -----------------------------------------------------
    def get_config_code(self, sample_rate = 44100, block_size = 512,
                        num_out_chnls = 2, num_in_chnls = 2, device = -1):
        config_template_code = '''
        '''

        return config_template_code
