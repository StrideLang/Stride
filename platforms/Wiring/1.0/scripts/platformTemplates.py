# -*- coding: utf-8 -*-
"""
Created on Sun Apr 17 11:28:56 2016

@author: andres
"""

from BaseCTemplate import BaseCTemplate


class Templates(BaseCTemplate):
    def __init__(self, domain_rate = 44100):
        super(Templates, self).__init__(domain_rate)

        self.string_type = "String"
        self.properties['num_in_channels'] = 2
        self.properties['num_out_channels'] = 2
#        self.properties['audio_device'] = 0
        self.properties['sample_rate'] = 44100
        self.properties['block_size'] = 2048



templates = Templates()