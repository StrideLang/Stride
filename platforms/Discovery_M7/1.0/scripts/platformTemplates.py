# -*- coding: utf-8 -*-
"""
Created on Sun Apr 17 11:28:56 2016

@author: andres
"""

from BaseCTemplate import BaseCTemplate


class Templates(BaseCTemplate):
    def __init__(self, domain_rate = 44100):
        super(Templates, self).__init__(domain_rate)
        
        self.real_type = 'float32_t'
        
    def get_config_code(self, sample_rate = 44100, block_size = 512,
                        num_out_chnls = 2, num_in_chnls = 2, device = -1):
         config_template_code = ''
#        config_template_code = '''
#    %%device%%
#    
#    AudioIO io(%%block_size%%, %%sample_rate%%, audioCB, NULL, %%num_out_chnls%%, %%num_in_chnls%%);
#        '''
#        device_code = '''
#    AudioDevice adevi = AudioDevice::defaultInput();
#    AudioDevice adevo = AudioDevice::defaultOutput();
#    '''
#        if device >= 0:
#            device_code = '''
#    AudioDevice adevi(%i);
#    AudioDevice adevo(%i);
#    '''%(device, device)
#            
#        device_code += '''
#        adevi.print();
#        adevo.print();
#        '''
#        
#        config_template_code = config_template_code.replace("%%device%%", str(device_code))
#        config_template_code = config_template_code.replace("%%block_size%%", str(block_size))
#        config_template_code = config_template_code.replace("%%sample_rate%%", str(sample_rate))
#        config_template_code = config_template_code.replace("%%num_out_chnls%%", str(num_out_chnls))
#        config_template_code = config_template_code.replace("%%num_in_chnls%%", str(num_in_chnls))

         return config_template_code


templates = Templates()