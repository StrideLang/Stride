# -*- coding: utf-8 -*-
"""
Created on Tue Jul 12 19:15:03 2016

@author: andres
"""

class Code(object):
    def __init__(self):
        self.code = ''
        self.scope = 0
        self.domain = None
    
    def get_code(self):
        return self.code
        
    def get_scope(self):
        return self.scope
        
    def get_domain(self):
        return self.domain
    
class Instance(Code):
    def __init__(self, code, scope, domain, vartype, handle, post = True):
        self.code = code
        self.scope = scope
        self.domain = domain
        self.vartype = vartype
        self.handle = handle
        self.post = post
        
    def get_code(self):
        return self.code
        
    def get_type(self):
        return self.vartype
        
    def get_handle(self):
        return self.handle
        
    def get_post(self):
        return self.post
        
    def get_domain(self):
        return self.domain
        
    def get_scope(self):
        return self.scope

class BundleInstance(Instance):
    def __init__(self, code, scope, domain, vartype, handle, size, post = True):
        super(BundleInstance, self).__init__(code, scope, domain, vartype, handle, post)
        self.size = size
    
    def get_type(self):
        return 'bundle'
    
    def get_size(self):
        return self.size
        
    def get_bundle_type(self):
        return self.vartype

class ModuleInstance(Instance):
    def __init__(self, scope, domain, vartype, handle, post = True):
        super(ModuleInstance, self).__init__('', scope, domain, vartype, handle, post)
    
    def get_type(self):
        return 'module'

    def get_module_type(self):
        return self.vartype
        
        
class Declaration(Code):
    def __init__(self, scope, domain, name, code):
        self.scope = scope
        self.domain = domain
        self.name = name
        self.code = code
        
    def get_name(self):
        return self.name