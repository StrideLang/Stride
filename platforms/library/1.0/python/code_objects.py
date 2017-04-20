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

class Code(object):
    def __init__(self):
        self.code = ''
        self.name = ''
        self.scope = 0
        self.domain = None
        self.dependents = []
        self.line = -1
        self.filename = ''

    def get_code(self):
        return self.code

    def get_name(self):
        return self.name

    def get_scope(self):
        return self.scope

    def get_domain(self):
        return self.domain

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

class Instance(Code):
    def __init__(self, code, scope, domain, vartype, handle, atom, post = True):
        self.code = code
        self.scope = scope
        self.domain = domain
        self.vartype = vartype
        self.handle = handle
        self.line = atom.get_line()
        self.filename = atom.get_filename()
        self.post = post
        self.dependents = []
        self.atom = atom

    def get_type(self):
        return self.vartype

    def get_name(self):
        return self.handle

    def get_post(self):
        return self.post


class BundleInstance(Instance):
    def __init__(self, code, scope, domain, vartype, handle, size, atom, post = True,
                 reads = [], writes = []):
        super(BundleInstance, self).__init__(code, scope, domain, vartype, handle, atom, post)
        self.size = size

    def get_type(self):
        return 'bundle'

    def get_size(self):
        return self.size

    def get_bundle_type(self):
        return self.vartype

class ModuleInstance(Instance):
    def __init__(self, scope, domain, vartype, handle, atom, instance_consts, post = True):
        super(ModuleInstance, self).__init__('', scope, domain, vartype, handle, atom, post)
        self.instance_consts = instance_consts


    def get_type(self):
        return 'module'

    def get_module_type(self):
        return self.vartype

    def get_instance_consts(self):
        return self.instance_consts


class Declaration(Code):
    def __init__(self, scope, domain, name, code):
        self.scope = scope
        self.domain = domain
        self.name = name
        self.code = code
        self.dependents = []

    def get_name(self):
        return self.name