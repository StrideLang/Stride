# -*- coding: utf-8 -*-
"""
Created on Tue Jul 12 19:15:03 2016

@author: andres
"""

class Code(object):
    def __init__(self):
        self.code = ''
        self.name = ''
        self.scope = 0
        self.domains = {}
        self.dependents = []
        self.line = -1
        self.filename = ''
        self.enabled = True

    def get_code(self):
        return self.code

    def get_name(self):
        return self.name

    def get_scope(self):
        return self.scope

    def get_domains(self):
        return self.domains

    def get_domain_list(self):
        '''Get flat list of domains together without repetitions'''
        domain_list = []
        for d in self.domains['read']:
            if not d in domain_list:
                domain_list.append(d)
        for d in self.domains['write']:
            if not d in domain_list:
                domain_list.append(d)
        return domain_list

    def get_dependents(self):
        return self.dependents

    def add_dependent(self, dependent):
        self.dependents.append(dependent)

    def depended_by(self, code_obj):
        return code_obj in self.dependents

    def get_line(self):
        return self.line

    def get_filename(self):
        return self.filename

    def get_enabled(self):
        # Reactions need to know about instances, but these instances should
        # not generate code. This flag enables this
        return self.enabled

class Instance(Code):
    def __init__(self, code, scope, read_domains, write_domains, vartype, handle, atom, post = True):
        self.code = code
        self.scope = scope
        self.domains = {}
        self.domains['read'] = read_domains
        self.domains['write'] = write_domains
        self.vartype = vartype
        self.handle = handle
        self.line = atom.get_line()
        self.filename = atom.get_filename()
        self.enabled = True
        self.post = post
        self.dependents = []
        self.atom = atom

    def get_type(self):
        return self.vartype

    def get_name(self):
        return self.handle

    def get_post(self):
        return self.post

class BufferInstance(Instance):
    def __init__(self, code, scope, read_domains, write_domains, vartype, handle, size, atom, post = True,
                 reads = [], writes = []):
        super(BufferInstance, self).__init__(code, scope, read_domains, write_domains, vartype, handle, atom, post)
        self.size = size

    def get_type(self):
        return 'buffer'

    def get_size(self):
        return self.size

    def get_buffer_type(self):
        return self.vartype

class BundleInstance(Instance):
    def __init__(self, code, scope, read_domains, write_domains, vartype, handle, size, atom, post = True,
                 reads = [], writes = []):
        super(BundleInstance, self).__init__(code, scope, read_domains, write_domains, vartype, handle, atom, post)
        self.size = size

    def get_type(self):
        return 'bundle'

    def get_size(self):
        return self.size

    def get_bundle_type(self):
        return self.vartype

class ModuleInstance(Instance):
    def __init__(self, scope, read_domains, write_domains, vartype, handle, atom, instance_consts, post = True):
        super(ModuleInstance, self).__init__('', scope, read_domains, write_domains, vartype, handle, atom, post)
        self.instance_consts = instance_consts

    def get_type(self):
        return 'module'

    def get_module_type(self):
        return self.vartype

    def get_instance_consts(self):
        return self.instance_consts

class PlatformModuleInstance(ModuleInstance):
    def __init__(self, scope, read_domains, write_domains, vartype, handle, atom, instance_consts, post = True):
        super(PlatformModuleInstance, self).__init__(scope, read_domains, write_domains, vartype, handle, atom, instance_consts, post)
        self.instance_consts = instance_consts

    def get_type(self):
        return 'platform_module'

    def get_module_type(self):
        return self.vartype

    def get_instance_consts(self):
        return self.instance_consts


class Declaration(Code):
    def __init__(self, scope, read_domains, write_domains, name, code):
        self.scope = scope
        self.domains = {}
        self.domains['read'] = read_domains
        self.domains['write'] = write_domains
        self.name = name
        self.code = code
        self.dependents = []
        self.enabled = True

    def get_name(self):
        return self.name

class ModuleDeclaration(Declaration):
    def __init__(self, scope, read_domains, write_domains, name, code):
        super(ModuleDeclaration, self).__init__(scope, read_domains, write_domains, name, code)

class DomainProcessingCode(object):
    def __init__(self, domain_name):
        self.domain_name = domain_name
        self.code = ''
        self.tokens = []
        self.token_is_output = []

    def get_domain_name(self):
        return self.domain_name

    def set_execution_domain(self, exec_domain):
        self.execution_domain = exec_domain

    def get_execution_domain(self):
        return self.execution_domain

    def add_token(self, token_name, is_input):
        self.tokens.append(token_name)
        self.token_is_output.append(is_input)

    def get_out_tokens(self):
        out_tokens = []
        for tok, is_output in zip(self.tokens, self.token_is_output):
            if is_output:
                out_tokens.append(tok)
        return out_tokens

    def append_code(self, code):
        self.code += code

    def get_code(self):
        return self.code
