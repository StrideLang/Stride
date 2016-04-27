# -*- coding: utf-8 -*-
"""
Created on Sun Apr 17 11:28:56 2016

@author: andres
"""

from BaseCTemplate import BaseCTemplate


class Templates(BaseCTemplate):
    def __init__(self, domain_rate = 44100):
        super(Templates, self).__init__(domain_rate)



templates = Templates()