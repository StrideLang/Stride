# -*- coding: utf-8 -*-
"""
Created on Sun Apr 17 11:28:56 2016

@author: andres
"""



from BaseCTemplate import BaseCTemplate


class Templates(BaseCTemplate):
    def __init__(self, domain_rate = 44100):
        super(Templates, self).__init__(domain_rate)
    
    def get_config_code(self, sample_rate = 44100, block_size = 512,
                        num_out_chnls = 2, num_in_chnls = 2, device = 2):
        config_template_code = '''
    %%device%%
    
    AudioIO io(%%block_size%%, %%sample_rate%%, audioCB, NULL, %%num_out_chnls%%, %%num_in_chnls%%);
        '''
        device_code = '''
    AudioDevice adevi = AudioDevice::defaultInput();
    AudioDevice adevo = AudioDevice::defaultOutput();
    '''
        if device >= 0:
            device_code = '''
    AudioDevice adevi(%i);
    AudioDevice adevo(%i);
    '''%(device, device)
            
        device_code += '''
        adevi.print();
        adevo.print();
        '''
        
        config_template_code = config_template_code.replace("%%device%%", str(device_code))
        config_template_code = config_template_code.replace("%%block_size%%", str(block_size))
        config_template_code = config_template_code.replace("%%sample_rate%%", str(sample_rate))
        config_template_code = config_template_code.replace("%%num_out_chnls%%", str(num_out_chnls))
        config_template_code = config_template_code.replace("%%num_in_chnls%%", str(num_in_chnls))

        return config_template_code

templates = Templates()
#
#class Templates(object):
#    def __init__(self):
#        self.stream_begin_code = '// Starting stream %02i -------------------------\n{\n'
#        self.stream_end_code = '} // Stream End %02i\n'
#        
#        # Internal templates
#        self.str_rate_begin_code = '{ // Start new rate %i\n' 
#        self.str_rate_end_code = '\n}  // Close Rate %i\n' 
#        self.str_assignment = '%s = %s;\n'
#        self.str_increment = '%s += %s;\n'
#        
#        self.rate_stack = []
#        self.rate_counter = 0
#        self.domain_rate = 44100 # TODO set this from domain configuration
#        pass
#    
#    def number_to_string(self, number):
#        if type(number) == int:
#            s = '%i;\n'%number
#        elif type(number) == float:
#            s = '%.8f;'%number
#        else:
#            raise ValueError(u"Unsupported type '%s' in assignment."%type(number).__name__)
#        return s;
#        
#    def get_platform_processing_code(self, code, token_names, handle, direction, bundle_index = -1):
#        if direction == 'in':
#            code = code.replace('%%token%%', token_names[0]) 
#            if bundle_index >= 0:           
#                code = code.replace('%%bundle_index%%', str(bundle_index))
#        elif direction == 'out':
#            code = code.replace('%%token%%', handle) 
#            if bundle_index >= 0:           
#                code = code.replace('%%bundle_index%%', str(bundle_index))
#        elif direction == 'thru':
#            code = code.replace('%%token%%', handle) 
#            code = code.replace('%%intoken%%', token_names[0]) 
#            if bundle_index >= 0:           
#                code = code.replace('%%bundle_index%%', str(bundle_index))
#        
#        return code
#                
#    def get_platform_declarations(self, declaration_list):
#        declarations = {}
#        for i,dec in enumerate(declaration_list):
#            declarations["_dec_%03i"%i] = dec['value']['value'] + '\n'
#        return declarations
#        
#    
#    def declaration_real(self, name, close=True):
#        declaration = "float %s"%name
#        if close:
#            declaration += ';\n'
#        return declaration
#
#    def assignment(self, assignee, value):
#        code = ''
#        if not type(value) == str and not type(value) == unicode:
#            value = self.number_to_string(value)
#        if not value == assignee:
#            code = self.str_assignment%(assignee, value)
#        return code
#        
#    def increment(self, assignee, value):
#        if not type(value) == str:
#            value = self.number_to_string(value)
#        code = self.str_increment%(assignee, value)
#        return code
#        

#    
#    def includes_code(self, includes):
#        includes_code = ''
#        for include in includes:
#            if not include in self.included: 
#                includes_code += "#include <%s>\n"%include
#                self.included.append(include)
#        return includes_code
#        
#    def instantiation_code(self, instance):
#        if instance['type'] == 'real':
#            code = 'float ' + instance['handle'] + ';\n'
#        elif instance['type'] =='bundle':
#            if instance['bundletype'] == 'real':
#                code = 'float ' + instance['handle'] + '[%i];\n'%instance['size']
#            else:
#                raise ValueError("Unsupported bundle type.")
#        elif instance['type'] == 'module':
#            code = instance['moduletype'] + ' ' + instance['handle'] + ';\n'
#        elif instance['type'] == 'reaction':
#            code = instance['reactiontype'] + ' ' + instance['handle'] + ';\n'
#        else:
#            raise ValueError('Unsupported type for instance')
#        return code
#                
#    def initialization_code(self, instance):
#        code = ''
#        if instance['type'] == 'real':
#            code = instance['handle'] + ' = ' + instance['code'] + ';\n'
#        elif instance['type'] == 'bundle':
#            for i in range(instance['size']):
#                elem_instance = {'type': instance['bundletype'] ,
#                                 'handle' : instance['handle'] + '[%i]'%i,
#                                 'code' : instance['code']
#                                 }
#                code += self.initialization_code(elem_instance)
#        else:
#            ValueError("Unsupported type for initialization: " + instance['type'])
#        return code
#      
#    # Handling of rate changes within a stream -------------------------------
#    def rate_init_code(self):
#        code = ''
#        rate = self.rate_stack[-1]
#        index = self.rate_counter
#        if not rate == self.domain_rate:
#            if rate < self.domain_rate:
#                code = '_counter_%03i = 1.0;\n'%(index)
#            else:
#                code = '_counter_%03i = 0.0;\n'%(index)
#        return code
#        
#    def rate_instance_code(self):
#        code = ''
#        rate = self.rate_stack[-1]
#        index = self.rate_counter
#        if not rate == self.domain_rate:
#            code = 'float _counter_%03i;\n'%(index)
#        return code
#        
#    def rate_start(self, rate):
#        self.rate_stack.append(rate)
#        
#    def rate_start_code(self):
#        code = ''
#        rate = self.rate_stack[-1]
#        index = self.rate_counter
#        if not rate == self.domain_rate:
#            if rate < self.domain_rate:
#                code += self.str_rate_begin_code%rate
#                code += 'if (_counter_%03i >= 1.0) {\n_counter_%03i -= 1.0;\n'%(index, index)
#            else:
#                code += self.str_rate_begin_code%rate
#                code += 'while (_counter_%03i < 1.0) {\n'%(index)
#        return code
#        
#    def rate_stack_size(self):
#        return len(self.rate_stack)
#    
#    def rate_end_code(self):
#        if len(self.rate_stack) > 0:
#            code = ''
#            rate = self.rate_stack.pop()
#            index = self.rate_counter
#            if not rate == self.domain_rate:
#                if rate < self.domain_rate:
#                    code += '}\n' # Closes counter check above
#                    code += templates.str_rate_end_code%rate
#                    code += '_counter_%03i += %.10f;\n'%(index, float(rate)/self.domain_rate)
#                else:
#                    code += '_counter_%03i += %.10f;\n'%(index, self.domain_rate/ float(rate))
#                    code += '}\n' # Closes counter check above
#                    code += '_counter_%03i -= 1.0;\n'%(index)
#                    code += templates.str_rate_end_code%rate
#            self.rate_counter += 1
#            return code
#        else:
#            return ''
#        
#    # Module code ------------------------------------------------------------
#    def module_declaration(self, name, header_code, init_code,
#                           input_block, process_code):       
#        if input_block:
#            # TODO this needs to be generalized for other types apart from float
#            input_declaration = templates.declaration_real(input_block['name'],
#                                                           close = False)
#        else:
#            input_declaration = ''
#        declaration = '''
#struct %s {
#    %s %s() {
#        %s
#    }
#    float process(%s) {
#        %s
#    }
#};
#'''%(name, header_code, name, init_code, input_declaration, process_code)
#        return declaration
#
#        
#    def module_processing_code(self, handle, in_tokens):
#        if len(in_tokens) > 0:
#            code = handle + '.process(' + in_tokens[0] + ')'
#        else:
#            code = handle + '.process()'
#        return code
#        
#    def module_property_setter(self, name, block_name, prop_type):
#        code = ''
#        if prop_type == 'real':
#            code += 'void set_' + name + '(float value) {\n'
#            code += block_name + ' = value;\n'
#            code += '\n}\n'
#        return code
#        
#    def module_output_code(self, output_block):
#        code = ''        
#        if output_block:
#            code += 'return %s;\n'%(output_block['name']) 
#        return code
#        
#    # Configuration code -----------------------------------------------------

#
#templates = Templates()