# -*- coding: utf-8 -*-
"""
Created on Sun Apr 17 11:28:56 2016

@author: andres
"""


class Templates(object):
    def __init__(self):
        self.stream_begin_code = '// Starting stream %02i -------------------------\n{\n'
        self.stream_end_code = '} // Stream End %02i\n'
        
        # Internal templates
        self.str_rate_begin_code = '{ // Start new rate %i\n' 
        self.str_rate_end_code = '\n}  // Close Rate %i\n' 
        self.str_assignment = '%s = %s;\n'
        self.str_increment = '%s += %s;\n'
        
        self.rate_stack = []
        self.rate_counter = 0
        self.domain_rate = 44100 # TODO set this from domain configuration
        pass
    
    def number_to_string(self, number):
        if type(number) == int:
            s = '%i;\n'%number
        elif type(number) == float:
            s = '%.8f;'%number
        else:
            raise ValueError(u"Unsupported type '%s' in assignment."%type(number).__name__)
        return s;
    
    def assignment(self, assignee, value):
        if not type(value) == str and not type(value) == unicode:
            value = self.number_to_string(value)
        code = self.str_assignment%(assignee, value)
        return code
        
    def increment(self, assignee, value):
        if not type(value) == str:
            value = self.number_to_string(value)
        code = self.str_increment%(assignee, value)
        return code
      
    # Handling of rate changes within a stream
    def rate_init_code(self):
        code = ''
        rate = self.rate_stack[-1]
        index = self.rate_counter
        if not rate == self.domain_rate:
            code = 'counter_%03i = 1.0;\n'%(index)
        return code
        
    def rate_instance_code(self):
        code = ''
        rate = self.rate_stack[-1]
        index = self.rate_counter
        if not rate == self.domain_rate:
            code = 'float counter_%03i;\n'%(index)
        return code
        
    def rate_start(self, rate):
        self.rate_stack.append(rate)
        
    def rate_start_code(self):
        code = ''
        rate = self.rate_stack[-1]
        index = self.rate_counter
        if not rate == self.domain_rate:
            code += self.str_rate_begin_code%rate
            code += 'if (counter_%03i >= 1.0) {\ncounter_%03i -= 1.0;\n'%(index, index)
        return code
        
    def rate_stack_size(self):
        return len(self.rate_stack)
    
    def rate_end_code(self):
        if len(self.rate_stack) > 0:
            code = ''
            rate = self.rate_stack.pop()
            index = self.rate_counter
            if not rate == self.domain_rate:
                code += '}\n' # Closes counter check above
                code += templates.str_rate_end_code%rate
                code += 'counter_%03i += %.10f;\n'%(index, float(rate)/self.domain_rate) 
            self.rate_counter += 1
            return code
        else:
            return ''
        

templates = Templates()