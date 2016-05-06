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
        self.rate_counter = 0
        self.domain_rate = domain_rate # TODO set this from domain configuration
        
        self.stream_begin_code = '// Starting stream %02i -------------------------\n{\n'
        self.stream_end_code = '} // Stream End %02i\n'
        
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

        pass
    
    def number_to_string(self, number):
        if type(number) == int:
            s = '%i;\n'%number
        elif type(number) == float:
            s = '%.8f;'%number
        else:
            raise ValueError(u"Unsupported type '%s' in assignment."%type(number).__name__)
        return s;
        
        
    def get_platform_inline_processing_code(self, code, token_names, direction, bundle_index = -1):
        p = re.compile("%%intoken:[a-zA-Z0-9_]+%%") ## TODO tweak this better
        matches = p.findall(code)
        if direction == 'in':
            for i, match in enumerate(matches):
                code = code.replace(match, token_names[i])
            if bundle_index >= 0:           
                code = code.replace('%%bundle_index%%', str(bundle_index))
        elif direction == 'out':
            if bundle_index >= 0:           
                code = code.replace('%%bundle_index%%', str(bundle_index))
        elif direction == 'thru': 
            if bundle_index >= 0:           
                code = code.replace('%%bundle_index%%', str(bundle_index))
            for i, match in enumerate(matches):
                code = code.replace(match, token_names[i])
        return code        
        
    def get_platform_processing_code(self, code, token_names, handle, direction, bundle_index = -1, prop_tokens = {}):
        code = self.get_platform_inline_processing_code(code, token_names, direction,
                                                        bundle_index, prop_tokens)
        if direction == 'out':
            code = code.replace('%%token%%', handle) 
        elif direction == 'thru':
            code = code.replace('%%token%%', handle) 
        return code
        
    def get_platform_declarations(self, declaration_list, scope_index):
        declarations = []
        for i,dec in enumerate(declaration_list):
            declarations.append({"name": "_dec_%03i"%i,
                                 "code" : dec['value']['value'] + '\n',
                                 'scope' : scope_index}
                                )
        return declarations
        
    def declaration_real(self, name, close=True):
        declaration = "float %s"%name
        if close:
            declaration += ';\n'
        return declaration

    def declaration_bundle(self, name, size, close=True):
        declaration = "float %s[%i]"%(name, size)
        if close:
            declaration += ';\n'
        return declaration
        
    def declaration_bool(self, name, close=True):
        declaration = "bool %s"%name
        if close:
            declaration += ';\n'
        return declaration 
        
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
        for elem in inits:
            init_code += elem['value']['value'] + '\n'
        return init_code
            
    def includes_code(self, includes):
        includes_code = ''
        for include in includes:
            if not include in self.included: 
                includes_code += "#include <%s>\n"%include
                self.included.append(include)
        return includes_code
        
    def instantiation_code(self, instance):
        if instance['type'] == 'real':
            code = 'float ' + instance['handle'] + ';\n'
        elif instance['type'] == 'bool':
            code = 'bool ' + instance['handle'] + ';\n'
        elif instance['type'] =='bundle':
            if instance['bundletype'] == 'real':
                code = 'float ' + instance['handle'] + '[%i];\n'%instance['size']
            elif instance['bundletype'] == 'bool':
                code = 'bool ' + instance['handle'] + '[%i];\n'%instance['size']
            else:
                raise ValueError("Unsupported bundle type.")
        elif instance['type'] == 'module':
            code = instance['moduletype'] + ' ' + instance['handle'] + ';\n'
        elif instance['type'] == 'reaction':
            code = instance['reactiontype'] + ' ' + instance['handle'] + ';\n'
        else:
            raise ValueError('Unsupported type for instance')
        return code
                
    def initialization_code(self, instance):
        code = ''
        if not instance['code'] == '':
            if instance['type'] == 'real':
                code = self.assignment(instance['handle'], instance['code'])
            if instance['type'] == 'bool':
                code = self.assignment(instance['handle'], instance['code'])
            elif instance['type'] == 'bundle':
                for i in range(instance['size']):
                    elem_instance = {'type': instance['bundletype'] ,
                                     'handle' : instance['handle'] + '[%i]'%i,
                                     'code' : instance['code']
                                     }
                    code += self.initialization_code(elem_instance)
            else:
                ValueError("Unsupported type for initialization: " + instance['type'])
        return code
      
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
        self.rate_stack.append(rate)
        
    def rate_start_code(self):
        code = ''
        rate = self.rate_stack[-1]
        index = self.rate_counter
        if not rate == self.domain_rate:
            if rate < self.domain_rate:
                code += self.str_rate_begin_code%rate
                code += 'if (_counter_%03i >= 1.0) {\n_counter_%03i -= 1.0;\n'%(index, index)
            else:
                code += self.str_rate_begin_code%rate
                code += 'while (_counter_%03i < 1.0) {\n'%(index)
        return code
        
    def rate_stack_size(self):
        return len(self.rate_stack)
    
    def rate_end_code(self):
        if len(self.rate_stack) > 0:
            code = ''
            rate = self.rate_stack.pop()
            index = self.rate_counter
            if not rate == self.domain_rate:
                if rate < self.domain_rate:
                    code += '}\n' # Closes counter check above
                    code += self.str_rate_end_code%rate
                    code += '_counter_%03i += %.10f;\n'%(index, float(rate)/self.domain_rate)
                else:
                    code += '_counter_%03i += %.10f;\n'%(index, self.domain_rate/ float(rate))
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
        if input_block:
            if input_block['type'] == 'signal':
                if output_block and 'size' in output_block:
                    input_declaration = self.declaration_real(input_block['name'],
                                                              close = False)
                    input_declaration += ", float %s[%i]"%(output_block['name'], output_block['size'])
                else:
                    input_declaration = self.declaration_real(input_block['name'],
                                                           close = False)
            elif input_block['type'] == 'switch':
                input_declaration = self.declaration_bool(input_block['name'],
                                                           close = False)
            elif input_block['type'] == 'Unsupported':
                input_declaration = self.declaration_real(input_block['name'],
                                                           close = False)
            else:
                raise ValueError("Unknown type")
        else:
            input_declaration = ''
        if output_block:
            if output_block['type'] == 'signal':
                if 'size' in output_block:
                    out_type = 'void'
                else:
                    out_type = 'float'
            elif output_block['type'] == 'switch':
                out_type = 'bool'
            else:
                raise ValueError("Unknown type")
        else:
            out_type = 'void'
            
        declaration = self.str_module_declaration%(name, header_code, name, init_code,
                                                   out_type, input_declaration, process_code)
        return declaration

    def module_set_property(self, handle, port_name, in_tokens):
        code = handle + '.set_' + port_name + '(' + in_tokens[0] + ');'
        return code
        
    def module_processing_code(self, handle, in_tokens, out_token):
        if len(in_tokens) > 0:
            code = handle + '.process(' + in_tokens[0]
            if not out_token == '':
                code += ', ' + out_token
            code += ')'
        else:
            code = handle + '.process()'
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
        return code
        
    def module_output_code(self, output_block):
        code = ''        
        if output_block and not 'size' in output_block: #When a bundle, then output is passed as reference in the arguments
            code += 'return %s;\n'%(output_block['name']) 
        return code
        
    # Configuration code -----------------------------------------------------
    def get_config_code(self, sample_rate = 44100, block_size = 512,
                        num_out_chnls = 2, num_in_chnls = 2, device = -1):
        config_template_code = '''
        '''

        return config_template_code
