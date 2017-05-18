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

from BaseCTemplate import BaseCTemplate


class Templates(BaseCTemplate):
    def __init__(self):
        super(Templates, self).__init__()

        self.real_type = 'float32_t'

        self.framework = "STM32F7"

    def get_config_code(self, sample_rate = 48000, block_size = 512,
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


    def value_real(self, value):
        value = '%.10f'%value + "f"
        return value


templates = Templates()
