# -*- coding: utf-8 -*-
"""
Created on Sat Apr  9 11:16:55 2016

@author: andres
"""

from __future__ import print_function
from __future__ import division


from platformTemplates import templates
from code_objects import Instance, BundleInstance, ModuleInstance, Declaration

try:
    unicode_exists_test = type('a') == unicode
except:
    unicode = str # for python 3  
  
def signal_type_string(signal_declaration):
    return type(signal_declaration['default']) == unicode
    
class Atom(object):
    def __init__(self):
        self.rate = -1
        self.inline = False
        self.globals = {}
        self.handle = ''
        self.domain = None
        self.line = -1
        self.filename = ""
        self.writes = 0
        self.reads = 0
        
        # FIXME these sections should be driven by the platform definition
        self.global_sections = ['include', 'includeDir', 'linkTo', 'linkDir']
        
    def set_inline(self, inline):
        self.inline = inline
    
    def is_inline(self):
        return self.inline
        
    def get_handles(self):
        if self.is_inline():
            return [self.get_inline_processing_code([])]
        else:
            return [self.handle]

    def get_out_tokens(self):
        if hasattr(self, 'out_tokens'):
            return self.out_tokens
        else:
            return [self.handle]
            
    def get_globals(self):
        return self.globals
    
    def get_declarations(self):
        return []
    
    def get_instances(self):
        return []
        
    def get_initialization_code(self, in_tokens):
        '''Returns code that should be executed only once per construction of
        the block'''
        return ''
        
    def get_preprocessing_code(self, in_tokens):
        ''' Returns code that needs to be run asynchronously but can't be
        inlined, so needs to be run separately previous to the processing
        code'''
        return ''    
        
    def get_processing_code(self, in_tokens):
        '''Processing code includes both the pre-processing and the
        inline processing code. The inline processing code will be provided
        ready to insert rather than ready to inline.'''
        return None
    
    def get_postproc_once(self):
        return None
        
    def get_inline_processing_code(self, in_tokens):
        ''' This returns the processing code itself, so this can be used
        when the output is used only once, and an intermediate symbol to 
        represent it is not needed'''
        return None
        
    def get_rate(self):
        return self.rate
        
    def get_scope_index(self):
        return self.scope_index
        
    def get_domain(self):
        return self.domain
    
    def get_line(self):
        return self.line
        
    def get_filename(self):
        return self.filename
        
    def get_num_writes(self):
        return self.writes
        
    def get_num_reads(self):
        return self.reads
         
class PlatformTypeAtom(Atom):
    def __init__(self, module, function, platform_type, token_index, platform, scope_index):
        super(PlatformTypeAtom, self).__init__()
        self.module = module
        self.platform_type = platform_type
        self.index = token_index
        self.platform = platform
        self.function = function
        self.scope_index = scope_index
        
        self.set_inline(False)
        
    def get_handles(self):
        if self.is_inline():
            return [self.get_inline_processing_code([])]
        else:
            return [self.handle]

    def get_out_tokens(self):
        if hasattr(self, 'out_tokens'):
            return self.out_tokens
        else:
            return [self.handle]
        
    def get_processing_code(self, in_tokens):
        return 'PROC_CODEEEEE'
        
    def get_inline_processing_code(self, in_tokens):
        return 'INLINE_CODEEEEE'
        
class ValueAtom(Atom):
    def __init__(self, value_node, index, scope_index):
        super(ValueAtom, self).__init__()
        self.index = index
        self.value = value_node
        self.handle = '__value_%03i'%index
        self.rate = 0
        self.inline = True
        self.scope_index = scope_index
        
    def get_handles(self):
        if self.is_inline():
            return [self.get_inline_processing_code([])]
        else:
            return [self.handle]
            
    def get_out_tokens(self):
        if self.is_inline():
            return [str(self.value)]
        else:
            return [self.handle]
    
    def get_instances(self):
        if self.is_inline():
            return []
        else:
            if type(self.value) == unicode:
                return [Instance('"' +self.get_inline_processing_code([]) + '"',
                                 self.scope_index,
                                 self.domain,
                                 'string',
                                 self.handle),
                                 self]
            else:
                return [Instance(self.get_inline_processing_code([]),
                                 self.scope_index,
                                 self.domain,
                                 'real',
                                 self.handle,
                                 self)]
        
    def get_inline_processing_code(self, in_token):
        return templates.value_real(self.value)
        
    def get_processing_code(self, in_tokens):
        return { None : ['', [self.get_inline_processing_code(in_tokens)] ]}       

class ExpressionAtom(Atom):
    def __init__(self, expr_type, left_atom, right_atom, index, scope_index):
        super(ExpressionAtom, self).__init__()
        self.scope_index = scope_index
        self.expr_type = expr_type
        self.left_atom = left_atom
        self.right_atom = right_atom
        self.index = index
        self.handle = '__expr_%03i'%index
        self.rate = -1
        if isinstance(self.left_atom, ModuleAtom) or isinstance(self.right_atom, ModuleAtom):
            self.set_inline(False)
        else:
            self.set_inline(True)
        
        self.domain = left_atom.get_domain()
        if not self.domain == right_atom.get_domain():
            print ("ERROR! domains must match inside expressions!")
        
    def set_inline(self, inline):
        self.left_atom.set_inline(inline)
        if self.right_atom:
            self.right_atom.set_inline(inline)
        self.inline = inline
            
    def get_declarations(self):
        declarations = self.left_atom.get_declarations()
        if self.right_atom:
            declarations += self.right_atom.get_declarations()
        return declarations
    
    def get_instances(self):
        instances = self.left_atom.get_instances()
        if self.right_atom:
            instances += self.right_atom.get_instances()
        if not self.is_inline():
            instances.append(Instance('',
                                 self.scope_index,
                                 self.domain,
                                 self._expression_out_type(),
                                 self.handle,
                                 self))
        return instances
        
    def get_inline_processing_code(self, in_tokens):
        if self.left_atom.is_inline():
            left_token = self.left_atom.get_inline_processing_code([])
        else:
            left_token = self.left_atom.get_out_tokens()[0]

            
        if self.right_atom:
            if self.right_atom.is_inline():
                right_token = self.right_atom.get_inline_processing_code([])
            else:
                right_token = self.right_atom.get_out_tokens()[0]
        else:
            right_token = None
            
        code = '(' + self._operator_symbol(left_token, right_token) + ')'
        return code
    
    
    def get_initialization_code(self, in_tokens):
        left_code = self.left_atom.get_initialization_code(in_tokens)
        right_code = ''
        if self.right_atom:
            right_code = self.right_atom.get_initialization_code(in_tokens)
        return left_code + right_code

        
    def get_preprocessing_code(self, in_tokens):
        left_code = self.left_atom.get_preprocessing_code([])
        right_code = ''
        if self.right_atom:
            right_code = self.right_atom.get_preprocessing_code([])
        return left_code + right_code
    
    def get_processing_code(self, in_tokens):       
        #code = self.get_preprocessing_code(in_tokens)
        code = ''
        domain = None
        if self.is_inline():
            out_tokens = [self.get_inline_processing_code(in_tokens)]
        else:
            left_code = self.left_atom.get_processing_code([])[0]
            if self.right_atom:
                right_code = self.right_atom.get_processing_code([])[0]
                right_tokens = self.right_atom.get_out_tokens()[0]
            else:
                right_code = ''
                right_tokens = None
            code += left_code + right_code
            
            code += templates.assignment(self.handle,
                                         self._operator_symbol(self.left_atom.get_out_tokens()[0],
                                                               right_tokens))
            out_tokens = [self.handle]
        return {domain : [code, out_tokens] }
        
        
    def _expression_out_type(self):
        if self.expr_type == 'Add':
            out_type = 'real'
        elif self.expr_type == 'Subtract':
            out_type = 'real'
        elif self.expr_type == 'Multiply':
            out_type = 'real'
        elif self.expr_type == 'Divide':
            out_type = 'real'
        elif self.expr_type == 'And':
            out_type = 'bool'
        elif self.expr_type == 'Or':
            out_type = 'bool'
        elif self.expr_type == 'UnaryMinus':
            out_type = 'real'
        elif self.expr_type == 'LogicalNot':
            out_type = 'bool'
        elif self.expr_type == 'Greater':
            out_type = 'bool'
        elif self.expr_type == 'Lesser':
            out_type = 'bool'
        elif self.expr_type == 'Equal':
            out_type = 'bool'
        elif self.expr_type == 'NotEqual':
            out_type = 'bool'
        elif self.expr_type == 'GreaterEqual':
            out_type = 'bool'
        elif self.expr_type == 'LesserEqual':
            out_type = 'bool'
        return out_type
        
    def _operator_symbol(self, left, right = None):
        code = '' if right is None else left
        if self.expr_type == 'Add':
            code += ' + '
        elif self.expr_type == 'Subtract':
            code += ' - '
        elif self.expr_type == 'Multiply':
            code += ' * '
        elif self.expr_type == 'Divide':
            code += ' / '
        elif self.expr_type == 'And':
            code += ' & '
        elif self.expr_type == 'Or':
            code += ' | '
        elif self.expr_type == 'UnaryMinus':
            code = ' - '
        elif self.expr_type == 'LogicalNot':
            code += ' ~ '
        elif self.expr_type == 'Greater':
            code += ' > '
        elif self.expr_type == 'Lesser':
            code += ' < '
        elif self.expr_type == 'Equal':
            code += ' == '
        elif self.expr_type == 'NotEqual':
            code += ' != '
        elif self.expr_type == 'GreaterEqual':
            code += ' >= '
        elif self.expr_type == 'LesserEqual':
            code += ' <= '
        code += left if right is None else right
        return code
        
        
class ListAtom(Atom):
    def __init__(self, list_node, scope_index):
        super(ListAtom, self).__init__()
        self.scope_index = scope_index
        self.list_node = list_node
        self.rate = -1
        self.inline = True
        
        self.handles = [elem.get_handles() for elem in list_node] # TODO make this recursive
        self.out_tokens = [elem.get_out_tokens() for elem in list_node]
        self.instances = []
        for elem in list_node:
            self.instances += elem.get_instances()
        
        self.globals = {}
        for atom in self.list_node:
            new_globals = atom.get_globals()
            self.globals.update(new_globals)
            
        
    def get_handles(self, index = -1):
        return self.handles
            
    def get_out_tokens(self, index = -1):
        return self.out_tokens[index]
        
    def get_declarations(self, index = -1):
        declarations = []
        for elem in self.list_node:
            elem_declarations = elem.get_declarations()
            declarations += elem_declarations
        return declarations
    
    def get_instances(self, index = -1):
        if index == -1:
            return self.instances
        return self.instances[index]
        
    def get_inline_processing_code(self, in_tokens):
        return str(self.value)
        
    def get_initialization_code(self, in_tokens):
        code = ''
        for i,elem in enumerate(self.list_node):
            new_code = elem.get_initialization_code(in_tokens)
            code += new_code
        return code
        
    def get_preprocessing_code(self, in_tokens):
        code = ''
        for i,elem in enumerate(self.list_node):
            if len(in_tokens) > 0:
                index = i%len(in_tokens)
                new_code = elem.get_preprocessing_code([in_tokens[index]])
                code += new_code
            else:
                 new_code = elem.get_preprocessing_code([])
                 code += new_code
        return code
        
    def get_postproc_once(self):
        postproc = []
        for i,elem in enumerate(self.list_node):
            new_postproc = elem.get_postproc_once()
            if new_postproc:
                postproc += new_postproc
        return postproc
        
    def get_processing_code(self, in_tokens):
        proc_code = {}
        for i,elem in enumerate(self.list_node):
            if len(in_tokens) > 0:
                index = i%len(in_tokens)
                elem_proc_code = elem.get_processing_code([in_tokens[index]])
            else:
                elem_proc_code = elem.get_processing_code([])
            for domain in elem_proc_code:
                if not domain in proc_code:
                    proc_code[domain] = ['', []]
                new_code, new_out_tokens = elem_proc_code[domain]
                proc_code[domain][0] += new_code
                proc_code[domain][1] += new_out_tokens
        return proc_code
        

class NameAtom(Atom):
    def __init__(self, platform_type, declaration, token_index, scope_index, line, filename):
        super(NameAtom, self).__init__()
        self.scope_index = scope_index
        self.name = declaration['name']
        self.handle = self.name # + '_%03i'%token_index;
        self.platform_type = platform_type
        self.declaration = declaration
        self.domain = None
        self.line = line
        self.filename = filename
        if 'domain' in self.declaration:
            if type(self.declaration['domain']) == dict:
                #FIXME this should be set by the code validator
                self.domain = self.declaration['domain']['name']['name'] 
            else:
                self.domain = self.declaration['domain']
            
        
        for section in self.global_sections:
            if section in platform_type['block']:
                if section in self.globals:
                    self.globals[section].extend([inc['value'] for inc in platform_type['block'][section]])
                else:
                    if section in platform_type['block']:
                        self.globals[section] = [inc['value'] for inc in platform_type['block'][section]]
        if 'initializations' in platform_type['block']:
            if 'initializations' in self.globals:
                self.globals['initializations'] += platform_type['block']['initializations']
            else:
                self.globals['initializations'] = platform_type['block']['initializations']
                
        
        self.set_inline(False)
        
        if 'outputs' in platform_type['block']:
            if len(platform_type['block']['outputs']) > 0:
                self.set_inline(True)
        
        if 'rate' in declaration:
            if type(declaration['rate']) == dict:
                self.rate = -1
            else:
                self.rate = declaration['rate']
                
        if '_writes' in declaration:
            self.writes = len(declaration['_writes'])
            
        print(self.writes)
        if '_reads' in declaration:
            self.reads = len(declaration['_reads'])
       # else:
            #raise ValueError("Parser must fill defaults.")
            #this should never happen... The parser should fill defaults...
        
    def get_declarations(self):
        declarations = []
        if 'declarations' in self.platform_type['block']:
            for i,dec in enumerate(self.platform_type['block']['declarations']):
                declarations.append(Declaration(self.scope_index,
                                                self.domain,
                                                "_dec_%03i"%i,
                                                dec['value'] + '\n'))
        return declarations
    
    def get_instances(self):
        
        if 'port_block' in self.declaration and self.declaration['port_block']:
            return [] # Atoms for main ports need not be de
        default_value = self._get_default_value()
        if 'type' in self.declaration and self.declaration['type'] == 'signal':
            if signal_type_string(self.declaration):
                inits = [Instance(default_value,
                                 self.declaration['stack_index'],
                                 self.domain,
                                 'string',
                                 self.handle,
                                 self
                                 )]
            else:
                inits = [Instance(str(default_value),
                                 self.declaration['stack_index'],
                                 self.domain,
                                 'real',
                                 self.handle,
                                 self)]
        elif 'type' in self.declaration and self.declaration['type'] == 'signalbridge':
            if signal_type_string(self.declaration):
                inits = [Instance(default_value,
                                 self.declaration['stack_index'],
                                 self.domain,
                                 'string',
                                 self.handle,
                                 self
                                 )]
            else:
                inits = [Instance(str(default_value),
                                 self.declaration['stack_index'],
                                 self.domain,
                                 'real',
                                 self.handle,
                                 self)]
        elif 'type' in self.declaration and self.declaration['type'] == 'constant':
            inits = [Instance(str(default_value),
                                 self.declaration['stack_index'],
                                 self.domain,
                                 'real',
                                 self.handle,
                                 self)]
        elif 'type' in self.declaration and self.declaration['type'] == 'switch':
            inits = [Instance(templates.str_true if default_value['value'] else templates.str_false,
                             self.declaration['stack_index'],
                             self.domain,
                             'bool',
                             self.handle,
                             self)]
        elif 'block' in self.platform_type:
            inherits = self.platform_type['block']['inherits']
            if inherits == 'signal':
                if 'default' in self.declaration and signal_type_string(self.declaration):
                    inits = [Instance(default_value,
                                 self.declaration['stack_index'],
                                 self.domain,
                                 'string',
                                 self.handle,
                                 self)]
                else:
                    inits = [Instance(str(default_value),
                                 self.declaration['stack_index'],
                                 self.domain,
                                 'real',
                                 self.handle,
                                 self)]
            elif self.platform_type['block']['type'] == 'platformType':
                if 'default' in self.declaration and signal_type_string(self.declaration):
                    inits = [Instance(default_value,
                                 self.declaration['stack_index'],
                                 self.domain,
                                 'string',
                                 self.handle,
                                 self)]
                else:
                    inits = [Instance(str(default_value),
                                 self.declaration['stack_index'],
                                 self.domain,
                                 'real',
                                 self.handle,
                                 self)]

            else:
                print("Don't know how to declare type " + ' '.join(self.declaration.keys()))
                inits = []
        else:
            print("Don't know how to declare type " + ' '.join(self.declaration.keys()))
            inits = []
        return inits
        
    def get_inline_processing_code(self, in_tokens):
        code = ''
        if 'processing' in self.platform_type['block']:
            code = templates.get_platform_inline_processing_code(
                            self.platform_type['block']['processing'],
                            in_tokens,
                            len(self.platform_type['block']['inputs']),
                            len(self.platform_type['block']['outputs']) )
        else:
            if len(in_tokens) > 0:
                code = in_tokens[0]
            else:
                code = self.handle
            
        return  code
    
    def get_initialization_code(self, in_tokens):
        code = ''
        if 'initializations' in self.platform_type['block']:
            code = self.platform_type['block']['initializations']
            if type(code) == list:
                merged_code = ''
                for value in code:
                    merged_code += value['value'] + '\n'
                code = merged_code
            code = templates.get_platform_initialization_code(code, 
                            in_tokens,
                            len(self.platform_type['block']['inputs']),
                            [self.handle]
                            )
        return code
    
    def get_preprocessing_code(self, in_tokens):
        code = ''
        if 'preProcessing' in self.platform_type['block']:
            code = self.platform_type['block']['preProcessing']
            code = templates.get_platform_preprocessing_code(code, 
                            in_tokens,
                            len(self.platform_type['block']['inputs']),
                            [self.handle]
                            )
        return code
    
    def get_processing_code(self, in_tokens):
        code = ''
        out_tokens = [self.handle]
        proc_code = self.get_inline_processing_code(in_tokens)
        domain = None
        if len(proc_code) > 0:
            if 'processing' in self.platform_type['block']:
                if self.inline:
                    out_tokens = [proc_code]
                else:
                    if len(self.platform_type['block']['outputs']) > 0:
                        code = templates.assignment(self.handle, proc_code)
                    else:
                        code = templates.expression(proc_code)
            else:
                code = templates.assignment(self.handle, proc_code)
        return {domain : [code, out_tokens] }
    
    def get_postproc_once(self):
        if 'block' in self.platform_type and self.platform_type['block']['type'] == "platformType":
            if not self.platform_type['block']['postProcessingOnce'] == '':
                return [[self.platform_type['block']['name'], self.platform_type['block']['postProcessingOnce']]]
        return None
            
    
    def _get_default_value(self):
        if 'default' in self.declaration:
            default_value = self.declaration['default']
        elif 'block' in self.platform_type:
            if 'default' in self.platform_type['block']:
                default_value = self.platform_type['block']['default'] # FIXME inheritance is not being handled here
            else:
                default_value = 0.0
        elif 'blockbundle' in self.platform_type:
            #Stride definition
            if 'default' in self.platform_type['blockbundle']:
                default_value = self.platform_type['blockbundle']['default']
            else:
                default_value = 0.0
        else:
            print("Forced default value to 0 for " + self.name)
            default_value = 0.0
        return default_value

class BundleAtom(NameAtom):
    def __init__(self, platform_type, declaration, index, token_index, scope_index, line, filename):
        ''' index indexes from 1, internal index from 0
        '''
        super(BundleAtom, self).__init__(platform_type, declaration, token_index, scope_index, line, filename)
        self.scope_index = scope_index
        if type(index) == int:
            self.index = index - 1
        else:
            ## FIXME we need to get handle for index object
            self.index = index
        self.set_inline(False)
#        if not 'blockbundle' in self.platform_type and not 'platformType' in self.platform_type['block']['type']:
#            raise ValueError("Need a block bundle platform type to make a Bundle Atom.")
        
    def get_handles(self):
        return [templates.bundle_indexing(self.handle, self.index + 1)]    
        
    def get_instances(self):
        default_value = self._get_default_value()

        if 'default' in self.declaration and signal_type_string(self.declaration):
            instances = [BundleInstance(default_value,
                                 self.scope_index,
                                 self.domain,
                                 'string',
                                 self.handle,
                                 self.declaration['size'],
                                 self) ]
                                  
        else:
            if 'size' in self.declaration:
                instances = [BundleInstance(str(default_value),
                                     self.scope_index,
                                     self.domain,
                                     'real',
                                     self.handle,
                                     self.declaration['size'],
                                     self) ]
            
                
        return instances
        
    def get_inline_processing_code(self, in_tokens): 
        code = ''
 
        if 'processing' in self.platform_type['block']:
            code = templates.get_platform_inline_processing_code(
                            self.platform_type['block']['processing'],
                            in_tokens,
                            len(self.platform_type['block']['inputs']),
                            len(self.platform_type['block']['outputs']),
                            self.index)
        else:
            if len(in_tokens) > 0:
                code = in_tokens[0]
            else:
                code = self._get_token_name(self.index)  
        return  code
    
    def get_processing_code(self, in_tokens):
        code = ''
        out_tokens = [self._get_token_name(self.index)]
        proc_code = self.get_inline_processing_code(in_tokens)
        domain = None
        if len(proc_code) > 0:
            if 'processing' in self.platform_type['block']:
                if self.inline:
                    out_tokens = [proc_code]
                else:
                    if len(self.platform_type['block']['outputs']) > 0:
                        code = templates.assignment(self._get_token_name(self.index), proc_code)
                    else:
                        code = templates.expression(proc_code)
            else:
                for token in out_tokens:
                    code += templates.assignment(token, proc_code) 
            
#        if len(in_tokens) > 0:
#            code = templates.assignment(self._get_token_name(self.index),
#                                        self.get_inline_processing_code(in_tokens))
#                                       
#        if 'processing' in self.platform_type['block']:
#            code = self.get_inline_processing_code(in_tokens)
#        
#        out_tokens = [self._get_token_name(self.index)]
        return {domain : [code, out_tokens] }       
        
    def _get_token_name(self, index):
        if type(index) == int:
            return '%s[%i]'%(self.handle, index)
        else:
            return '%s[%s]'%(self.handle, index)
    

class ModuleAtom(Atom):
    def __init__(self, module, function, platform_code, token_index, platform, scope_index, connected_blocks):
        super(ModuleAtom, self).__init__()
        self.scope_index = scope_index
        self.name = module["name"]
        self.handle = self.name + '_%03i'%token_index;
        #self._platform_code = platform_code
        self._input_blocks = []
        self._output_blocks = []
        self._index = token_index
        self.platform = platform
        self.module = module
        self.rate = -1 # Should modules have rates?
        self.function = function
        self.domain = None
        self.connected_blocks = connected_blocks
        if 'domain' in self.function['ports']:
            if 'value' in self.function['ports']['domain']:
                self.domain = self.function['ports']['domain']['value']
            else:
                # FIXME we need to read the value from the name (not get its name)
                self.domain = self.function['ports']['domain']['name']['name'] 
            self.function['ports'].pop('domain')

        self.port_name_atoms = {}
        
        self._process_module(self.module["streams"])
        self.set_inline(False)
        
            
    def set_inline(self, inline):
        if inline == True:
            print("Warning: Inlining modules not supported")
            return
        self.inline = inline
        
    def get_declarations(self):
        declarations = []
        outer_declarations = []
        secondary_domain = ''
        #FIXME do we need to support more than 1 output block?
        if len(self._output_blocks) > 0:
            secondary_domain = self._output_blocks[0]['domain']

        if "other_scope_declarations" in self.code:
            for other_declaration in self.code["other_scope_declarations"]:
                if not other_declaration.domain or other_declaration.domain == secondary_domain:
                    other_declaration.domain = self.domain
                outer_declarations.append(other_declaration)

        declarations += outer_declarations
        domain_code = self.code['domain_code']

        for block in self.connected_blocks:
            declarations += block.get_declarations()
            
        header_code = ''
        init_code = ''
        process_code = {}
        #properties_domain_code = ''
        for domain, code in domain_code.items():
            if domain is not None: # To get rid of domains from constants
                header_code += code['header_code'] 
                init_code += code['init_code']
                if not domain in process_code:
                    process_code[domain] = {"code": '', "input_blocks" : [], "output_blocks" : []}
    
    #            if 'input_blocks' in self.code['domain_code'][domain]:
    #                for input_block in self.code['domain_code'][domain]['input_blocks']:
    #                    if type(input_block.atom) == NameAtom or type(input_block.atom) == BundleAtom:
    #                        process_code[domain]['input_blocks'].append(input_block.atom.declaration)
    #            if 'output_blocks' in self.code['domain_code'][domain]:
    #                for output_block in self.code['domain_code'][domain]['output_blocks']:
    #                    if type(output_block.atom) == NameAtom or type(output_block.atom) == BundleAtom:
    #                        process_code[domain]['output_blocks'].append(output_block.atom.declaration)
                
                process_code[domain]['code'] += '\n'.join(code['processing_code'])
                for block in self._input_blocks:
                    if block['domain'] == domain:
                        process_code[domain]['input_blocks'].append(block)
                
                for block in self._output_blocks:
                    if block['domain'] == domain:
                        process_code[domain]['output_blocks'].append(block)
        
        declaration_text = templates.module_declaration(
                self.name, header_code, 
                init_code, process_code)
                
        declaration = Declaration(self.module['stack_index'],
                                        self.domain,
                                        self.name,
                                        declaration_text)

        for outer_dec in outer_declarations:
            outer_dec.add_dependent(declaration)
                
        declarations.append(declaration)
        return declarations
    
    def get_instances(self):
        instances = []
        module_instance = ModuleInstance(self.scope_index,
                                 self.domain,
                                 self.name,
                                 self.handle,
                                 self)
        if "other_scope_instances" in self.code:
            for inst in self.code["other_scope_instances"]:
                inst.post = False
                inst.add_dependent(module_instance)
                instances.append(inst)
                
        for block in self.connected_blocks:
            instances += block.get_instances()
                
#        for atoms in self.port_name_atoms.values():
#            for atom in atoms:
#                instances += atom.get_instances()
                #FIXME do we need to support multiple output blocks?
        if len(self.out_tokens) > 0:
            block_types = self.get_block_types(self._output_blocks[0]);
            default_value = ''
            instances += [Instance(default_value,
                                 self.scope_index,
                                 self.domain,
                                 block_types[0],
                                 self.out_tokens[0],
                                 self) ]
            
        for name, atoms in self.port_name_atoms.items():
            for atom in atoms:
                decl = self.platform.find_declaration_in_tree(atom.get_handles(),
                                                              self.platform.tree + self.module['blocks'])
                if not decl:
                    if type(atom) is NameAtom:
                        default_value = 0.0
                        if not self.platform.find_instance_by_handle(atom.get_handles()[0], instances):
                            instances += atom.get_instances()
                    elif type(atom) is ExpressionAtom:
                        for new_inst in self._get_expression_instances(atom):
                            if not self.platform.find_instance_by_handle(new_inst.get_name(), instances):
                                instances += [new_inst]
                                 
        instances += [module_instance ]
        return instances
        
    def get_inline_processing_code(self, in_tokens):
        code = ''
        out_tokens = []
        
        if len(self._output_blocks) > 0:
            if 'size' in self._output_blocks[0]:
                out_tokens = ['_' + self.name + '_%03i_out'%self._index]
            else:
                out_tokens = self.out_tokens
        
        domain = ''
        for in_block in self._input_blocks:
            domain = in_block['domain']
#            if not in_block['main'] and in_block['domain'] == self._input_blocks[0]['domain']:
#                if 'size' in in_block:
#                    in_tokens.append('_%s_in'%in_block['name'])
#                else:
#                    in_tokens.append(in_block['name'])
            
        for out_block in self._output_blocks:
            domain = out_block['domain']
#            if not out_block['main']and out_block['domain'] == self._output_blocks[0]['domain']:
#                if 'size' in out_block:
#                    out_tokens.append('_%s_%03i_out'%(out_block['name'], out_block['index']))
#                else:
#                    out_tokens.append(out_block['name'])
            
        code = templates.module_processing_code(self.handle, 
                                                in_tokens,
                                                out_tokens,
                                                domain
                                                )
        return code
    
    def get_initialization_code(self, in_tokens):
        code = ''
        # Go through the module ports and check if the function is setting any by
        # value. This means that the call needs to be put into the initialization
        # of the domain
        if not self.module['ports']:
            return ''
        
        for module_port in self.module['ports']:
            module_port_domain = ''
            if 'block' in module_port:
                module_block = module_port['block']
            elif 'blockbundle' in module_port:
                module_block = module_port['blockbundle']
            if  'domain' in module_block and module_block['domain']:
                if type(module_block['domain']) == str or type(module_block['domain']) == unicode:
                    module_port_domain = module_block['domain']
                else:
                    module_port_domain = module_block['domain']['name']['name']
            module_port_name = module_block['name']
            module_port_direction = module_block['direction']
            for port_atom_name in self.port_name_atoms:
                if port_atom_name == module_port_name:
                    for port_value in self.port_name_atoms[port_atom_name]:
                        # TODO implement for output ports
                        if type(port_value) is ValueAtom and module_port_direction == 'input' and not module_block['domain'] == self._output_blocks[0]['domain']:
                            #if port_atom.
                            module_call = templates.module_processing_code(self.handle, port_value.get_handles(), [], module_port_domain)
                            code += templates.expression(module_call)
                    pass
            
        # FIXME this needs fixing
#        for in_block in self._input_blocks:
#            if 'size' in in_block:
#                new_code = templates.declaration_bundle_real('_%s_in'%self.handle, len(in_tokens)) + '\n'
#                for i in range(len(in_tokens)):
#                    new_code += templates.assignment('_%s_in[%i]'%(self.handle, i), in_tokens[i])
#                code += new_code
        return code
    
    def get_preprocessing_code(self, in_tokens):
        code = ''

        # FIXME this needs fixing
#        for in_block in self._input_blocks:
#            if 'size' in in_block:
#                new_code = templates.declaration_bundle_real('_%s_in'%self.handle, len(in_tokens)) + '\n'
#                for i in range(len(in_tokens)):
#                    new_code += templates.assignment('_%s_in[%i]'%(self.handle, i), in_tokens[i])
#                code += new_code
        return code
        
    def get_processing_code(self, in_tokens):
        #code = self.get_preprocessing_code(in_tokens)
        code = '' 
        out_tokens = self.out_tokens
        domain = self.domain
    
        # First adjust input tokens to match input block sizes/bundles     
        for input_block in self._input_blocks:
            if 'size' in input_block:
                # TODO check type of input ports
                connector_name = '_bundle_connector_' + str(self.platform.unique_id)
                self.platform.unique_id += 1
                code += templates.declaration_bundle_real(connector_name, input_block['size'])
                for i in range(input_block['size']):
                    if len(in_tokens) > 0:
                        code += templates.assignment(templates.bundle_indexing(connector_name, i+ 1),
                                                     in_tokens[0])
                        in_tokens.pop(0)
                in_tokens.insert(0, connector_name)
        
        
        # Process port domains
        domain_proc_code = {}
        if self.module['ports']:
            for module_port in self.module['ports']:
                module_port_domain = ''
                if 'block' in module_port:
                    module_block = module_port['block']
                elif 'blockbundle' in module_port:
                    module_block = module_port['blockbundle']
                if  'domain' in module_block and module_block['domain']:
                    if type(module_block['domain']) == str or type(module_block['domain']) == unicode:
                        module_port_domain = module_block['domain']
                    else:
                        module_port_domain = module_block['domain']['name']['name']
                module_port_name = module_block['name']
                module_port_direction = module_block['direction']
                for port_atom_name in self.port_name_atoms:
                    if port_atom_name == module_port_name:
                        for port_value in self.port_name_atoms[port_atom_name]:
                            # TODO implement for output ports
                            if not type(port_value) is ValueAtom and module_port_direction == 'input':
                                #if port_atom.
                                if not module_port_domain in domain_proc_code:
                                    domain_proc_code[module_port_domain] = {'handles': []}
                                domain_proc_code[module_port_domain]['handles'] += port_value.get_handles()
                        pass        
                
        for module_port_domain, values  in domain_proc_code.iteritems():
            if module_port_domain == self._output_blocks[0]['domain']:
                in_tokens += values['handles']
            else:
                module_call = templates.module_processing_code(self.handle, values['handles'], [], module_port_domain)
                code += templates.expression(module_call)
        
        if 'output' in self.module and not self.module['output'] is None: #For Platform types
            if self.inline:
                code += self.get_handles()[0]
            else:
                code += templates.expression(self.get_inline_processing_code(in_tokens)) 
        else:
            code += templates.expression(self.get_inline_processing_code(in_tokens))
        for port_atom in self.port_name_atoms:
            pass
            
        return {domain : [code, out_tokens] }
        
#    def _get_internal_header_code(self):
#        code = self.code['domain_code']['header_code']
#        return code
        
    def _get_internal_processing_code(self):
        code = self.code['domain_code']['processing_code']
        code += templates.module_output_code(self._output_block) 
        return code

    def _process_module(self, streams):
        
        self._init_blocks(self.module["blocks"])
        
        self.out_tokens = []
        #FIXME do we need to support more than 1 output block?
        if len(self._output_blocks) > 0:
            if 'size' in self._output_blocks[0]:
                self.out_tokens = ['_' + self.name + '_%03i_out[%i]'%(self._index, i) for i in range(self._output_blocks[0]['size'])]
            else:
                self.out_tokens = ['_' + self.name + '_%03i_out'%self._index]
        if not type(self.module["streams"]) == list:
            # TODO Should this be handled by the code validator before getting here?
            self.module["streams"] = [self.module["streams"]]
        
        tree = streams
        instanced = []
#        if self._input_block: # Mark input block as instanced so it doesn't get instantiated as other blocks
#            instanced = [[self._input_block['name'], self.scope_index]]
            
        if 'ports' in self.function:
            for name,port_value in self.function['ports'].items():
                new_atom = self.platform.make_atom(port_value)
                new_atom.scope_index += 1 # Hack to bring it up to the right scope...
                if name in self.port_name_atoms:
                    self.port_name_atoms[name].append(new_atom)
                else:
                    self.port_name_atoms[name] = [new_atom]
        
        self.code = self.platform.generate_code(tree, self._blocks,
                                                instanced = instanced)
        
#        if self.module['ports']:
#                for prop in self.module['ports']:
#                    if 'block' in prop:
#                        if type(prop['block']['block']) == dict:
#                            decl = self.platform.find_declaration_in_tree(prop['block']['block']['name']['name'],
#                                                                          self.platform.tree + self._blocks)
#                                     #FIXME implement for bundles
#                            domain = decl['domain']
#                            property_name = prop['block']['name']
#        #                    domain = "_Property:" + property_name
#                            if type(domain) == dict:
#                                domain = domain['name']['name']
#                                # FIXME need to resolve domain name from "name"
#                            prop_type = self.get_block_types(decl)[0]
#                        else:
#                            # Property is not a block but a constant value.
#                            pass  

        
        self.globals = {}
        for section in self.global_sections:
            if section in self.code['global_groups']:
                self.globals[section] = self.code['global_groups'][section]
                

    def _init_blocks(self, blocks):
        self._blocks = []
        for block in blocks:
            self._blocks.append(block)
        
        if not self.module['ports']:
            return
            
        for port in self.module['ports']: 
            internal_block = self.find_internal_block(port['block']['block']['name']['name'])
            if 'main' in port['block'] and port['block']['main']['value'] == True:
                internal_block['main'] = True
                internal_block['port_block'] = True
#                for block in self._blocks:
#                    name = ''
#                    if 'block' in block:
#                        name = block['block']['name']
#                    elif 'blockbundle' in block:
#                        name = block['blockbundle']['name']
#                    if internal_block['name'] == name:
#                        self._blocks.remove(block)
#                        break
                if 'direction' in port['block'] and port['block']['direction'] == 'input':
                    self._input_blocks.append(internal_block)
                elif 'direction' in port['block'] and port['block']['direction'] == 'output':
                    self._output_blocks.append(internal_block)
            else: # Not a main port
                internal_block['main'] = False
                internal_block['port_block'] = True
                if 'direction' in port['block'] and port['block']['direction'] == 'input':
                    self._input_blocks.append(internal_block)
                elif 'direction' in port['block'] and port['block']['direction'] == 'output':
                    self._output_blocks.append(internal_block)
            
                
    def find_internal_block(self, block_name):
        for block in self._blocks:
            if 'block' in block and block['block']['name'] == block_name:
                return block['block']
            elif 'blockbundle' in block and block['blockbundle']['name'] == block_name:
                return block['blockbundle']
                
    def get_block_types(self, block):
        # FIXME implement for bundles
        if 'default' in block and type(block['default']) == unicode:
            block_type = 'string'
        elif block['type'] == 'switch':
            block_type = 'bool'
        else:
            block_type = 'real'
        return [block_type]
    
    def _get_expression_instances(self, atom):
        if not type(atom) is ExpressionAtom:
            return []
        instances = []
        left = atom.left_atom
        right = atom.right_atom
        if type(left) is NameAtom:
            instances += left.get_instances()
        elif type(left) is ExpressionAtom:
            instances += self._get_expression_instances(left)
        
        if type(right) is NameAtom:
            instances += right.get_instances()
        elif type(right) is ExpressionAtom:
            instances += self._get_expression_instances(right)
        return instances

class PlatformModuleAtom(ModuleAtom):
    def __init__(self, module, function, platform_code, token_index, platform, scope_index):
        super(PlatformModuleAtom, self).__init__(module, function, platform_code, token_index, platform, scope_index)
        
    
# TODO complete work on reaction block
class ReactionAtom(ModuleAtom):
    def __init__(self, reaction, function, platform_code, token_index, platform, scope_index, connected_blocks):
        super(ReactionAtom, self).__init__(reaction, function, platform_code, token_index, platform, scope_index, connected_blocks)
        self.reaction = reaction
        
        
    def get_inline_processing_code(self, in_tokens):
        code = ''
            
        code = templates.module_processing_code(self.handle, 
                                                in_tokens,
                                                self.out_tokens,
                                                'TriggerDomain'
                                                )
        return code                       
                       
                       
    def get_processing_code(self, in_tokens):
        #code = self.get_preprocessing_code(in_tokens)
        #processing_code = super(ReactionAtom, self).get_processing_code(in_tokens)
    
        processing_code = {}
            
        for each_domain in self.code['domain_code']:
            if not each_domain in processing_code:
                processing_code[each_domain] = ['', []]
            processing_code[each_domain][0] += "if (" + in_tokens[0] + ") {\n"
            processing_code[each_domain][0] += templates.expression(
                    templates.module_processing_code(self.handle, 
                                                    [],
                                                    self.out_tokens,
                                                    each_domain
                                                    ))
                                                    
            processing_code[each_domain][0] += "}\n"
                                                    
        return processing_code
         

#    def _process_module(self, streams):
        
    

# --------------------- Common platform functions
class PlatformFunctions:
    def __init__(self, tree, debug_messages=False):
        
        self.defined_modules =[]
        self.debug_messages = debug_messages
    
        self.tree = tree
        self.scope_stack = []
        
        self.sample_rate = 44100 # Set this as default but this should be overriden by platform:
        for elem in tree:
            if 'block' in elem:
                if elem['block']['name'] == 'PlatformRate':
                    self.sample_rate = elem['block']['value']
        templates.domain_rate = self.sample_rate
        self.unique_id = 0
        
    def log_debug(self, text):
        if self.debug_messages:
            print(text)
            
    
    def find_declaration_in_tree(self, block_name, tree):
        for node in tree:
            if 'block' in node:
                if node["block"]["name"] == block_name:
                    node["block"]['stack_index'] = 0
                    return node["block"]
            if 'blockbundle' in node:
                if node["blockbundle"]["name"] == block_name:
                    node["blockbundle"]['stack_index'] = 0
                    return node["blockbundle"]
        for i, scope in enumerate(self.scope_stack[::-1]):
            for node in scope: # Now look within scope
                if 'block' in node:
                    if node["block"]["name"] == block_name:
                        node["block"]['stack_index'] = len(self.scope_stack) - 1 - i
                        return node["block"]
                if 'blockbundle' in node:
                    if node["blockbundle"]["name"] == block_name:
                        node["blockbundle"]['stack_index'] = len(self.scope_stack) - 1 - i
                        return node["blockbundle"]
#        raise ValueError("Declaration not found for " + block_name)
        return None
        
    def find_stride_type(self, type_name):
        for element in self.tree:
            if 'block' in element:
                element["block"]['stack_index'] = 0
                if element['block']['type'] == 'module':
                    if element['block']['name'] == type_name:
                        return element
                elif element['block']['type'] == 'type':
                    if element['block']['typeName'] == type_name:
                        return element
                elif element['block']['type'] == 'platformType':
                    if element['block']['typeName'] == type_name:
                        return element
                elif element['block']['type'] == 'platformModule':
                    if element['block']['name'] == type_name:
                        return element
    
    def find_block(self, name, tree):
        block_declaration = self.find_declaration_in_tree(name, tree)
        if 'type' in block_declaration:
            platform_type = self.find_stride_type(block_declaration["type"])
        else:
            platform_type =  self.find_stride_type(block_declaration["platformType"])
        return platform_type, block_declaration
    
    def find_port_value(self, object_name, port_name, tree):
        platform_type, block_declaration = self.find_block(object_name, tree)
        # first look in declaration
        for port in block_declaration["ports"]:
            if port == port_name:
                return block_declaration["ports"][port]["value"]
        #then in type for default values
    #    for type in platform_type:
    #        for key in type["ports"]:
    #            if key == "name" and type["ports"]["name"] == port_name:
    #                return type[
        return None
    
    def find_port_direction(self, prot_block):
        return port_block['block']['direction']
    
    def find_function_property(self, func, property_name):
        return func["ports"][property_name]
    
    def find_instance_by_handle(self,handle, instance_list):
        for instance in instance_list:
            if instance.get_name() == handle:
                return instance
        return None
    
    
    def bool_to_str(self, bool_val):
        if not type(bool_val) == bool:
            raise ValueError("Only boolean values accepted.")
        elif bool_val:
            return "true"
        else:
            return "false"
    
    # Code generation functions
        
    def make_atom(self, member):
        scope_index = len(self.scope_stack) -1 
        if "name" in member:
            platform_type, declaration = self.find_block(member['name']['name'], self.tree)
            new_atom = NameAtom(platform_type, declaration, self.unique_id, scope_index, member['name']['line'], member['name']['filename'])
        elif "bundle" in member:
            platform_type, declaration = self.find_block(member['bundle']['name'], self.tree)
            new_atom = BundleAtom(platform_type, declaration, member['bundle']['index'], self.unique_id, scope_index, member['bundle']['line'],member['bundle']['filename'])
        elif "function" in member:
            platform_type, declaration = self.find_block(member['function']['name'], self.tree)
            connected_blocks = []            
            for port_name, value in member['function']['ports'].items():
                if 'block' or 'bundle' in value:
                    connected_blocks.append(self.make_atom(value))
            if declaration['type'] == 'module':
                if 'type' in platform_type['block']:
                    new_atom = ModuleAtom(declaration, member['function'], platform_type, self.unique_id, self, scope_index, connected_blocks)
                else:
                    raise ValueError("Invalid or unavailable platform type.")
            elif declaration['type'] == 'reaction':
                new_atom = ReactionAtom(declaration, member['function'], platform_type, self.unique_id, self, scope_index, connected_blocks)
        elif "expression" in member:
            if 'value' in member['expression']: # Unary expression
                left_atom = self.make_atom(member['expression']['value'])
                right_atom = None
            else:
                left_atom = self.make_atom(member['expression']['left'])
                self.unique_id += 1
                right_atom = self.make_atom(member['expression']['right'])
            expression_type = member['expression']['type']
            new_atom = ExpressionAtom(expression_type, left_atom, right_atom, self.unique_id, scope_index)
        elif "value" in member:
            new_atom = ValueAtom(member['value'], self.unique_id, scope_index)
        elif "list" in member:
            list_atoms = []
            for element in member['list']:
                element_atom = self.make_atom(element)
                list_atoms.append(element_atom)
            new_atom = ListAtom(list_atoms, scope_index)
#        elif "block" in member:
#            if 'type' in member['block']:
#                platform_type = self.find_stride_type(member['block']["type"])
#            else:
#                platform_type =  self.find_stride_type(member['block']["platformType"])
#            new_atom = NameAtom(platform_type, member['block'], self.unique_id, scope_index)
#        elif "blockbundle" in member:
#            if 'type' in member:
#                platform_type = self.find_stride_type(member['blockbundle']["type"])
#            else:
#                platform_type =  self.find_stride_type(member['blockbundle']["platformType"])
#            new_atom = NameAtom(platform_type, member['blockbundle'], self.unique_id, scope_index)
        else:
            raise ValueError("Unsupported type")
        return new_atom
      
    def push_scope(self, scope):
        self.scope_stack.append(scope)
    
    def pop_scope(self):
        self.scope_stack.pop()
        
    def make_stream_nodes(self, stream):
        node_groups = [[]] # Nodes are grouped by domain
        current_domain = None
        cur_group = node_groups[-1]
        
        for member in stream: #Only instantiate whatever is used in streams. Discard the rest
            
            new_atom = self.make_atom(member) 
            if hasattr("domain", "new_atom"):
                if not current_domain == new_atom.domain:
                    current_domain = new_atom.domain
                    print("New domain!" + current_domain)
                    node_groups.append([])
                    cur_group = node_groups[-1]
            else:
                #print("No domain... Bad.")
                pass
            self.log_debug("New atom: " + str(new_atom.handle) + " domain: :" + str(current_domain) + ' (' + str(type(new_atom)) + ')')
            cur_group.append(new_atom)

            self.unique_id += 1
        return node_groups
        
    def instantiation_code(self, instance):
        if instance.get_type() == 'real':
            code = templates.declaration_real(instance.get_name())
        elif instance.get_type() == 'bool':
            code = templates.declaration_bool(instance.get_name())
        elif instance.get_type() == 'string':
            code = templates.declaration_string(instance.get_name())
        elif instance.get_type() =='bundle':
            if instance.get_bundle_type() == 'real':
                code = templates.declaration_bundle_real(instance.get_name(), instance.size)
            elif instance.get_bundle_type() == 'bool':
                code = templates.declaration_bundle_bool(instance.get_name(), instance.size)
            else:
                raise ValueError("Unsupported bundle type.")
        elif instance.get_type() == 'module':
            code = templates.declaration_module(instance.get_module_type(), instance.get_name())
        elif instance.get_type() == 'reaction':
            code = templates.declaration_reaction(instance.get_module_type(), instance.get_name())
        else:
            raise ValueError('Unsupported type for instance')
        code += templates.source_marker(instance.get_line(), instance.get_filename())
        return code
        
    def initialization_code(self, instance):
        code = ''
        if not instance.get_code() == '':
            if instance.get_type() == 'real':
                value = instance.get_code()
                if value:
                    code = templates.assignment(instance.get_name(), value)
            elif instance.get_type() == 'bool':
                value = instance.get_code()
                if value:
                    code = templates.assignment(instance.get_name(), value)
            elif instance.get_type() == 'string':
                if instance.get_code():
                    value = '"' + instance.get_code() + '"'
                    code = templates.assignment(instance.get_name(), value)
            elif instance.get_type() == 'bundle':
                for i in range(instance.get_size()):
                    elem_instance = Instance(instance.get_code(),
                                             instance.get_scope(),
                                             instance.get_domain(),
                                             instance.get_bundle_type(),
                                             instance.get_name() + '[%i]'%i,
                                             instance)
                    code += self.initialization_code(elem_instance)
            else:
                ValueError("Unsupported type for initialization: " + instance.get_type())
        return code
        
    def generate_code_from_groups(self, node_groups, global_groups):

        init_code = {}
        header_code = {}
        processing_code = {} 
        post_processing = {}
        writes = {}
        reads = {}
        parent_rates_size = templates.rate_stack_size() # To know now much we need to pop for this stream
        
#        header = []
        scope_declarations = []
        scope_instances = []
        current_domain = None
        
        self.log_debug(">>> Start stream generation")
        for group in node_groups:
            in_tokens = []
            previous_atom = None
            for atom in group:
                self.log_debug("Processing atom: " + str(atom.handle))
                # TODO check if domain has changed to handle in tokens in domain change
                if atom.domain: # Make "None" domain reuse existing domain
                    if type(atom.domain) == dict:
                        # FIMXE this is for the case where domains are internal
                        # Should be resolved by the code validator
                        current_domain = atom.domain['name']['name']
                    else:
                        current_domain = atom.domain
                
                # Accumulate reads and writes within domains
                if not current_domain in writes:
                    writes[current_domain] = []
                if not current_domain in reads:
                    reads[current_domain] = []
                
                if (atom.get_num_writes() > 0):
                    writes[current_domain].append(atom.get_handles());
                if (atom.get_num_reads() > 0):
                    reads[current_domain].append(atom.get_handles());
                #Process Inlcudes
                new_globals = atom.get_globals()
                if len(new_globals) > 0:
                    for global_group in new_globals:
                        for new_global in new_globals[global_group]:
                            if not new_global in global_groups[global_group]:
                                global_groups[global_group].append(new_global)

                declares = atom.get_declarations()                
                new_instances = atom.get_instances()
                
                if not current_domain in header_code:
                    header_code[current_domain] = "" 
                
                # append new declarations if not there in the list already
                for new_dec in declares:
                    if self.find_instance_by_handle(new_dec.get_name(), scope_declarations):
                        print ("Declaration already queued: " + new_dec.get_name())
                    else:
                        scope_declarations.append(new_dec)
                        
                for new_inst in new_instances:
                    if self.find_instance_by_handle(new_inst.get_name(), scope_instances):
                        print ("Instance already queued: " + new_inst.handle)
                    else:
                        scope_instances.append(new_inst)
                        
                #scope_instances += new_instances
#                header.append([declares, new_instances])
                
                if not current_domain in init_code:
                    init_code[current_domain] = "" 
                init_code[current_domain] += atom.get_initialization_code(in_tokens)
                
                if not current_domain in processing_code:
                    processing_code[current_domain] = "" 
                processing_code[current_domain] += atom.get_preprocessing_code(in_tokens)
                # Process processing code
                new_processing_code = atom.get_processing_code(in_tokens)
                new_postprocs = atom.get_postproc_once()
                
                if not current_domain in post_processing:
                    post_processing[current_domain] = [] 
                if new_postprocs:
                    for new_postproc in new_postprocs:
                        if new_postproc:
                            postproc_present = False
                            for postproc in post_processing[current_domain]:
                                if postproc[0] == new_postproc[0]:
                                    postproc_present = True
                                    break
                            # Do we need to order the post processing code?
                            if not postproc_present:
                                post_processing[current_domain].append(new_postproc)
                if atom.rate > 0:
                    new_inst, new_init, new_proc = templates.rate_start(atom.rate)
                    processing_code[current_domain] += new_proc
                    header_code[current_domain] += new_inst
                    init_code[current_domain] += new_init
                    # We want to avoid inlining across rate boundaries
                    if previous_atom:
                        previous_atom.set_inline(False)
                    atom.set_inline(False)
                
                for domain in new_processing_code:
                    code, out_tokens = new_processing_code[domain]
                    #self.log_debug("Code:  " + str(code))
                    if not domain:
                        domain = current_domain
                    if not domain in processing_code:
                        processing_code[domain] = ''
                    processing_code[domain] += code + '\n'
                    if domain == current_domain:
                        in_tokens = out_tokens
            
            previous_atom = atom

        for domain in post_processing:
            for postprocdomain in post_processing[domain]:
                processing_code[domain] += postprocdomain[1] + '\n'
            
        # Close pending rates in this stream
        while not parent_rates_size == templates.rate_stack_size():
            # FIXME rates should be closed in their specific domain
            processing_code[current_domain] += templates.rate_end_code()

        self.log_debug(">>> End stream generation")
        return [header_code, init_code, processing_code,
                scope_instances, scope_declarations, reads, writes]
        
    def generate_stream_code(self, stream, stream_index, global_groups):
        self.log_debug("-- Start stream")       
        node_groups = self.make_stream_nodes(stream)
        
        first_line = node_groups[0][0].get_line()
        #last_line = node_groups[0][-1].get_line()
        stream_filename = node_groups[0][0].get_filename()

        new_code = self.generate_code_from_groups(node_groups, global_groups)
        header_code, init_code, new_processing_code, scope_instances, scope_declarations, reads, writes = new_code

        self.log_debug("READS------ " + str(reads) )
        self.log_debug("WRITES------ " + str(writes) )
        self.log_debug("-- End stream")
        
        for domain in new_processing_code.keys():
            wrapper_begin = templates.stream_begin_code%stream_index + templates.source_marker(first_line, stream_filename)
            wrapper_end =  templates.stream_end_code%stream_index
            new_processing_code[domain] = wrapper_begin + new_processing_code[domain] + wrapper_end
        
        return {"global_groups" : global_groups,
                "header_code" : header_code,
                "init_code" : init_code,
                "processing_code" : new_processing_code,
                "scope_instances": scope_instances,
                "scope_declarations": scope_declarations
                }
                
    def get_domains(self):
        domains = []
        for node in self.tree:
            if 'block' in node:
                if 'type' in node['block'] and node['block']['type'] == '_domainDefinition':
                    domains.append(node['block'])
        return domains
        
    def get_platform_domain(self):
        domain = ''
        declaration = self.find_declaration_in_tree("PlatformDomain", self.tree)
        if declaration:
            domain = declaration['value']
            
        return domain
        
                
    def generate_code(self, tree, current_scope = [],
                      global_groups = {'include':[], 'includeDir':[], 'initializations' : [], 'linkTo' : [], 'linkDir' : []},
                      instanced = []):  
        stream_index = 0
        other_scope_instances = []
        other_scope_declarations = []
        scope_declarations = []
        scope_instances = []
        
        self.log_debug("* New Generation ----- scopes: " + str(len(self.scope_stack)))
        self.push_scope(current_scope)
        
        domain_code = {}
        global_groups_code = {}
        
        
        for node in tree:
            if 'stream' in node: # Everything grows from streams.
                code = self.generate_stream_code(node["stream"], stream_index,
                                                 global_groups)
                # merge global groups from different streams...
                for global_section in code['global_groups']:
                    if global_section in global_groups_code:
                        for new_global in code['global_groups'][global_section]:
                            if not new_global in global_groups_code[global_section]:
                                global_groups_code[global_section].append(new_global)
                    else:
                        global_groups_code[global_section] = code['global_groups'][global_section]
                for domain, header_code in code["header_code"].items():
                    if not domain in domain_code:
                        domain_code[domain] = { "header_code": '',
                        "init_code" : '',
                        "processing_code" : [] }
                    domain_code[domain]["header_code"] += header_code
                
                for domain, init_code in code["init_code"].items():
                    if not domain in domain_code:
                        domain_code[domain] =  { "header_code": '',
                        "init_code" : '',
                        "processing_code" : [] }
                    domain_code[domain]["init_code"] += init_code
                
                for domain, processing_code in code["processing_code"].items():
                    if not domain in domain_code:
                        domain_code[domain] =  { "header_code": '',
                        "init_code" : '',
                        "processing_code" : [] }
                    domain_code[domain]["processing_code"].append(processing_code)
                    
                scope_declarations += code["scope_declarations"]
                scope_instances += code["scope_instances"]
                stream_index += 1
        
        declared = []

        header_elements = scope_declarations + scope_instances 
        
        is_sorted = False
        # TODO do a more efficient sort
        if len(header_elements) > 0:
            while not is_sorted:
                for i in range(len(header_elements)):
                    for j in range(i):
                        if header_elements[i].depended_by(header_elements[j]):
                            header_elements[i], header_elements[j] = header_elements[j], header_elements[i]
                            break
                    if i == len(header_elements) - 1:  
                        is_sorted = True
                
        for new_element in header_elements:
            if new_element.get_scope() >= len(self.scope_stack) - 1: # if declaration in this scope
                self.log_debug(':::--- Domain : '+ str(new_element.get_domain()) + ":::" + new_element.get_name() + '::: scope ' + str(new_element.get_scope()) )
                tempdict = new_element.__dict__  # For debugging. This shows the contents in the spyder variable explorer              
                #self.log_debug(new_element.get_code())
                if type(new_element) == Declaration or issubclass(type(new_element), Declaration):
                    is_declared = False
                    # TODO can this go? Shouldn't we have chacked if already declared by now?
                    for d in declared:
                        if d[0] == new_element.get_name() and d[1] == new_element.get_scope():
                            is_declared = True
                            break
                    if not is_declared:
                        if not new_element.get_domain() in domain_code:
                            domain_code[new_element.get_domain()] =  { "header_code": '',
                                "init_code" : '',
                                "processing_code" : [] }
                        declared.append( [new_element.get_name(), new_element.get_scope() ])
                        domain_code[new_element.get_domain()]['header_code'] += new_element.get_code()
                elif type(new_element) == Instance or issubclass(type(new_element), Instance):
                    is_declared = False
                    for i in instanced:
                        if i[0] == new_element.get_name() and i[1] == new_element.get_scope():
                            is_declared = True
                            break
                    if not is_declared:
                        new_inst_code = self.instantiation_code(new_element)
                        if not new_element.get_domain() in domain_code:
                            domain_code[new_element.get_domain()] =  { "header_code": '',
                                "init_code" : '',
                                "processing_code" : [] }
                        if new_element.post:
                            domain_code[new_element.get_domain()]["header_code"] += new_inst_code
                        else:
                            domain_code[new_element.get_domain()]["header_code"] = new_inst_code + domain_code[new_element.get_domain()]["header_code"]
                        domain_code[new_element.get_domain()]["init_code"] +=  self.initialization_code(new_element)
                        instanced.append([new_element.get_name(), new_element.get_scope() ])
#                    if not "input_blocks" in domain_code[new_element.get_domain()]:
#                        domain_code[new_element.atom.get_domain()]["input_blocks"] = []
#                    contained = False
#                    for elem in domain_code[new_element.atom.get_domain()]["input_blocks"]:
#                        if new_element.get_name() == elem.get_name():
#                            contained = True
#                            break
#                        
#                    if not contained:
#                        domain_code[new_element.atom.get_domain()]["input_blocks"].append(new_element)
#                        
#                    if not "output_blocks" in domain_code[new_element.get_domain()]:
#                        domain_code[new_element.atom.get_domain()]["output_blocks"] = []
#                    contained = False
#                    for elem in domain_code[new_element.atom.get_domain()]["output_blocks"]:
#                        if new_element.get_name() == elem.get_name():
#                            contained = True
#                            break
##                    # Don't include as output if it is already an input
#                    for elem in domain_code[new_element.atom.get_domain()]["input_blocks"]:
#                        if new_element.get_name() == elem.get_name():
#                            contained = True
#                            break
#                        
#                    if not contained:
#                        domain_code[new_element.atom.get_domain()]["output_blocks"].append(new_element)
            else:
                other_scope_declarations.append(new_element) 

        self.pop_scope()
        self.log_debug("* Ending Generation ----- scopes: " + str(len(self.scope_stack)))
        
        return {"global_groups" : global_groups_code,
                "domain_code": domain_code,
                "other_scope_instances" : other_scope_instances,
                "other_scope_declarations" : other_scope_declarations}
     

import json

class GeneratorBase(object):
    def __init__(self, out_dir = '',
                 platform_dir = '',
                 debug = False):

        self.out_dir = out_dir
        self.platform_dir = platform_dir

        self.project_dir = platform_dir + '/project'
        self.out_file = self.out_dir + '/main.cpp'

        jsonfile = open(self.out_dir + '/tree.json')
        self.tree = json.load(jsonfile)

        self.platform = PlatformFunctions(self.tree, debug)

        self.last_num_outs = 0        
        
        self.written_sections = []
    
    def log(self, text):
        print(text)

    def write_section_in_file(self, sec_name, code, filename):
        f = open(filename, 'r')
        text = f.read()
        f.close()
    #    log(text)
        start_index = text.find("//[[%s]]"%sec_name)
        end_index = text.find("//[[/%s]]"%sec_name, start_index)
        if start_index <0 or end_index < 0:
            raise ValueError("Error finding [[%s]]  section"%sec_name)
            return
        if sec_name in self.written_sections:
            code = text[start_index + len("//[[%s]]"%sec_name):end_index] + code
        else:
            self.written_sections.append(sec_name)
        text = text[:start_index] + '//[[%s]]\n'%sec_name + code + text[end_index:]
        f = open(filename, 'w')
        f.write(text)
        f.close()
        
    def write_code(self, code, filename):
        templates.set_property('sample_rate', self.sample_rate)
        templates.set_property('block_size', self.block_size)
        templates.set_property('num_out_chnls', self.num_out_chnls)
        templates.set_property('num_in_chnls', self.num_in_chnls)
        templates.set_property('audio_device', self.audio_device)
        domains = self.platform.get_domains()
        globals_code = templates.get_globals_code(code['global_groups'])
        for platform_domain in domains:
            if platform_domain['domainName'] == self.platform.get_platform_domain():
               break
        self.write_section_in_file(platform_domain['globalsTag'], globals_code, filename)
        
        template_init_code = templates.get_config_code()
        config_code = templates.get_configuration_code(code['global_groups']['initializations'])
        self.write_section_in_file(platform_domain['initializationTag'], template_init_code + config_code, filename)
        processing_code = {}
        for domain,sections in code['domain_code'].items():
            domain_matched = False
            for platform_domain in domains:
                if platform_domain['domainName'] == domain or not domain:
                    if domain:
                        print("--- Domain found:" + domain)
                        print('\n'.join(sections['processing_code']))
                    else:
                        domain = self.platform.get_platform_domain()
                        print("--- Domain none.")
                        print('\n'.join(sections['processing_code']))
                    if not domain in processing_code:
                        processing_code[domain] = ""
                    processing_code[domain] += '\n'.join(sections['processing_code'])
                    
                    self.write_section_in_file(platform_domain['declarationsTag'], sections['header_code'], filename)
                    self.write_section_in_file(platform_domain['initializationTag'], sections['init_code'], filename)
                    if 'cleanup_code' in sections:
                        self.write_section_in_file(platform_domain['cleanupTag'], sections['cleanup_code'], filename)
                    domain_matched = True
                    break
            if not domain_matched:
                print('WARNING: Domain not matched: ' + str(domain))
        
        for domain in processing_code:
            for platform_domain in domains: 
                if platform_domain['domainName'] == domain:
                    code = processing_code[domain]
                    if not platform_domain['domainFunction'] == '':
                        code = platform_domain['domainFunction'].replace("%%domainCode%%", code)
                    
                    self.write_section_in_file(platform_domain['processingTag'], code, filename)
    
    
if __name__ == '__main__':
    pass
