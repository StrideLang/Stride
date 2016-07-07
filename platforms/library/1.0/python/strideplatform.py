# -*- coding: utf-8 -*-
"""
Created on Sat Apr  9 11:16:55 2016

@author: andres
"""

from __future__ import print_function
from __future__ import division


from platformTemplates import templates
  
    
class Atom(object):
    def __init__(self):
        self.rate = -1
        self.inline = False
        self.globals = {}
        self.handle = ''
        self.domain = 'AudioDomain' #FIXME this needs to be set correctly from source and platform
        
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
        return {}
    
    def get_instances(self):
        return {}
        
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
            return {}
        else:
            if type(self.value) == unicode:
                return {self.domain: {'type' : 'string',
                     'handle' : self.handle,
                     'code' : '"' + self.get_inline_processing_code([]) + '"',
                     'scope' : self.scope_index
                     }}
            else:
                return {self.domain: {'type' : 'real',
                         'handle' : self.handle,
                         'code' : self.get_inline_processing_code([]),
                         'scope' : self.scope_index
                         }}
        
    def get_inline_processing_code(self, in_token):
        return templates.value_real(self.value)
        
    def get_processing_code(self, in_tokens):
        return '', self.get_inline_processing_code(in_tokens)        

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
        
    def set_inline(self, inline):
        self.left_atom.set_inline(inline)
        if self.right_atom:
            self.right_atom.set_inline(inline)
        self.inline = inline
            
    def get_declarations(self):
        declarations = self.left_atom.get_declarations()
        if self.right_atom:
            right_declarations = self.right_atom.get_declarations()
            for domain in right_declarations:
                if domain in declarations:
                    declarations[domain] += right_declarations[domain]
                else:
                    declarations[domain] = right_declarations[domain]
        return declarations
    
    def get_instances(self):
        instances = self.left_atom.get_instances()
        if self.right_atom:
            right_instances = self.right_atom.get_instances()
            for domain in right_instances:
                if domain in instances:
                    instances[domain] += right_instances[domain]
                else:
                    instances[domain] = right_instances[domain]
        if not self.is_inline():
            if not self.domain in instances:
                instances[self.domain] = []
            instances[self.domain].append({'handle' : self.handle,
                              'type' : self._expression_out_type(),
                              'code' : '',
                              'scope' : self.scope_index
                              })
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
        left_code = self.left_atom.get_initialization_code([])
        right_code = ''
        if self.right_atom:
            right_code = self.right_atom.get_initialization_code([])
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
        return code, out_tokens
        
        
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
        self.instances = [elem.get_instances() for elem in list_node]
        
    def get_handles(self, index = -1):
        return self.handles
            
    def get_out_tokens(self, index = -1):
        return self.out_tokens[index]
        
    def get_globals(self):
        self.globals = {}
        for atom in self.list_node:
            new_globals = atom.get_globals()
            self.globals.update(new_globals)
        return self.globals
        
    def get_declarations(self, index = -1):
        declarations = {}
        for elem in self.list_node:
            elem_declarations = elem.get_declarations()
            for domain in elem_declarations:
                if domain in declarations:
                    declarations[domain] += elem_declarations[domain]
                else:
                    declarations[domain] = elem_declarations[domain]
        return declarations
    
    def get_instances(self, index = -1):
        if index == -1:
            flat_instances = {}
            for instances in self.instances:
                for domain, inst in instances.items():
                    if not domain in flat_instances:
                        flat_instances[domain] = []
                    flat_instances[domain] += inst
            return flat_instances
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
        code = ''
        out_tokens = []
        for i,elem in enumerate(self.list_node):
            if len(in_tokens) > 0:
                index = i%len(in_tokens)
                new_code, new_out_tokens = elem.get_processing_code([in_tokens[index]])
                code += new_code
                out_tokens += new_out_tokens
            else:
                 new_code, new_out_tokens = elem.get_processing_code([])
                 code += new_code
                 out_tokens += new_out_tokens
        return code, out_tokens
        

class NameAtom(Atom):
    def __init__(self, platform_type, declaration, token_index, scope_index):
        super(NameAtom, self).__init__()
        self.scope_index = scope_index
        self.name = declaration['name']
        self.handle = self.name # + '_%03i'%token_index;
        self.platform_type = platform_type
        self.declaration = declaration
        self.domain = ''
        if 'domain' in self.declaration:
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
       # else:
            #raise ValueError("Parser must fill defaults.")
            #this should never happen... The parser should fill defaults...
        
    def get_declarations(self):
        declarations = []
        if 'declarations' in self.platform_type['block']:
            declarations = templates.get_platform_declarations(self.platform_type['block']['declarations'],
                                                       self.scope_index)
        return {self.domain : declarations}
    
    def get_instances(self):
        default_value = self._get_default_value()
        if 'type' in self.declaration and self.declaration['type'] == 'signal':
            if type(self.declaration['default']) == unicode:
                inits = [{'handle' : self.handle,
                      'type' : 'string',
                      'code' : default_value,
                      'scope' : self.declaration['stack_index']
                      }]
            else:
                inits = [{'handle' : self.handle,
                          'type' : 'real',
                          'code' : str(default_value),
                          'scope' : self.declaration['stack_index']
                          }]
        elif 'type' in self.declaration and self.declaration['type'] == 'constant':
            inits = [{'handle' : self.handle,
                      'type' : 'real',
                      'code' : str(default_value),
                      'scope' : self.declaration['stack_index']
                      }]
        elif 'type' in self.declaration and self.declaration['type'] == 'switch':
            inits = [{'handle' : self.handle,
                      'type' : 'bool',
                      'code' : templates.str_true if default_value['value'] else templates.str_false,
                      'scope' : self.declaration['stack_index']
                      }]
        elif 'block' in self.platform_type:
            inherits = self.platform_type['block']['inherits']
            if inherits == 'signal':
                if 'default' in self.declaration and type(self.declaration['default']) == unicode:
                    inits = [{'handle' : self.handle,
                          'type' : 'string',
                          'code' : default_value,
                          'scope' : self.declaration['stack_index']
                          }]
                else:
                    inits = [{'handle' : self.handle,
                              'type' : 'real',
                              'code' : str(default_value),
                              'scope' : self.declaration['stack_index']
                              }]
            elif self.platform_type['block']['type'] == 'platformType':
                if 'default' in self.declaration and type(self.declaration['default']) == unicode:
                    inits = [{'handle' : self.handle,
                              'type' : 'string',
                              'code' : default_value,
                              'scope' : self.declaration['stack_index']
                              }]
                else:
                    inits = [{'handle' : self.handle,
                              'type' : 'real',
                              'code' : str(default_value),
                              'scope' : self.declaration['stack_index']
                              }]

            else:
                print("Don't know how to declare type " + ' '.join(self.declaration.keys()))
                inits = []
        else:
            print("Don't know how to declare type " + ' '.join(self.declaration.keys()))
            inits = []
        return {self.domain : inits}
        
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
        if 'processing' in self.platform_type['block']:
            if self.inline:
                out_tokens = [self.get_inline_processing_code(in_tokens)]
            else:
                if len(self.platform_type['block']['outputs']) > 0:
                    code = templates.assignment(self.handle, self.get_inline_processing_code(in_tokens))
                else:
                    code = templates.expression(self.get_inline_processing_code(in_tokens))
        else:
            code = templates.assignment(self.handle, self.get_inline_processing_code(in_tokens))
        return code, out_tokens
    
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
    def __init__(self, platform_type, declaration, index, token_index, scope_index):
        ''' index indexes from 1, internal index from 0
        '''
        super(BundleAtom, self).__init__(platform_type, declaration, token_index, scope_index)
        self.scope_index = scope_index
        self.index = index - 1
        self.set_inline(False)
#        if not 'blockbundle' in self.platform_type and not 'platformType' in self.platform_type['block']['type']:
#            raise ValueError("Need a block bundle platform type to make a Bundle Atom.")
        
    
    def get_instances(self):
        default_value = self._get_default_value()

        if 'default' in self.declaration and type(self.declaration['default']) == unicode:
            instances = [{'handle' : self.handle,
                          'code' : default_value,
                          'type' : 'bundle',
                          'bundletype' : 'string',
                          'size' : self.declaration['size'],
                          'scope' : self.scope_index
                          }]   
        else:
            instances = [{'handle' : self.handle,
                          'code' : str(default_value),
                          'type' : 'bundle',
                          'bundletype' : 'real',
                          'size' : self.declaration['size'],
                          'scope' : self.scope_index
                          }] 
            
                
        return {self.domain : instances}
        
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
        if 'processing' in self.platform_type['block']:
            if self.inline:
                out_tokens = [self.get_inline_processing_code(in_tokens)]
            else:
                if len(self.platform_type['block']['outputs']) > 0:
                    code = templates.assignment(self._get_token_name(self.index), self.get_inline_processing_code(in_tokens))
                else:
                    code = templates.expression(self.get_inline_processing_code(in_tokens))
        else:
            for token in out_tokens:
                code += templates.assignment(token, self.get_inline_processing_code(in_tokens)) 
            
#        if len(in_tokens) > 0:
#            code = templates.assignment(self._get_token_name(self.index),
#                                        self.get_inline_processing_code(in_tokens))
#                                       
#        if 'processing' in self.platform_type['block']:
#            code = self.get_inline_processing_code(in_tokens)
#        
#        out_tokens = [self._get_token_name(self.index)]
        return code, out_tokens       
        
    def _get_token_name(self, index):
        return '%s[%i]'%(self.handle, index)
    

class ModuleAtom(Atom):
    def __init__(self, module, function, platform_code, token_index, platform, scope_index):
        super(ModuleAtom, self).__init__()
        self.scope_index = scope_index
        self.name = module["name"]
        self.handle = self.name + '_%03i'%token_index;
        #self._platform_code = platform_code
        self._input_block = None
        self._output_block = None
        self._index = token_index
        self.platform = platform
        self.module = module
        self.rate = -1 # Should modules have rates?
        self.function = function
        self.domain = None
        if 'domain' in self.function['ports']:
            self.domain = self.function['ports']['domain']['value']
            self.function['ports'].pop('domain')
        
        self.port_name_atoms = {}
        
        self._init_blocks(module["internalBlocks"],
                          module["input"], module["output"])
                          
        if self._output_block and 'size' in self._output_block:
            self.out_tokens = ['_' + self.name + '_%03i_out[%i]'%(self._index, i) for i in range(self._output_block['size'])]
        else:
            self.out_tokens = ['_' + self.name + '_%03i_out'%self._index]
        self._process_module(self.module["streams"])
        self.set_inline(False)
        
            
    def set_inline(self, inline):
        if inline == True:
            print("Warning: Inlining modules not supported")
            return
        self.inline = inline
        
    def get_declarations(self):
        declarations = {}
        if "other_scope_declarations" in self.code:
            declarations[self.domain] = self.code["other_scope_declarations"]

        domain_code = self.code['domain_code']
        
        properties_code = self._get_internal_properties_code()
        for domain, code in domain_code.items():
            header_code = code['header_code'] 
            init_code = code['init_code']
            process_code = code['processing_code']
            properties_domain_code = ''
            if domain in properties_code:
                properties_domain_code = '\n'.join(properties_code[domain])
            if domain == self.domain or self.domain == None:
                properties_domain_code = '\n'.join(properties_code[None])
            
        
            declaration = templates.module_declaration(
                    self.name, header_code + properties_domain_code, 
                    init_code, self._output_block , self._input_block, process_code)
            if not domain in declarations:
                declarations[domain] = []
            declarations[domain].append({"name": self.name,
                             "code" :  declaration,
                             'scope' : self.module['stack_index']})
        
        return declarations
    
    def get_instances(self):
        instances = {}
        instances[self.domain] = []
        if "other_scope_instances" in self.code:
            for inst in self.code["other_scope_instances"]:
                if inst:
                    inst['post'] = False
                    instances[self.domain].append(inst)
        for atoms in self.port_name_atoms.itervalues():
            for atom in atoms:
                for domain, inst in atom.get_instances().items():
                    if not domain in instances:
                        instances[domain] = []
                    instances[domain] += inst
                
        instances[self.domain] += [{'type' : 'module',
                     'handle': self.handle,
                     'moduletype' : self.name,
                     'scope' : self.scope_index,
                     'post' : True}
                     ]
        if len(self.out_tokens) > 0 and self.module['output']:
            out_block = self.find_internal_block(self.module['output']['name']['name'])
            # FIXME support bundles
            block_types = self.get_block_types(out_block);
            instances[self.domain] += [{ 'type' : block_types[0],
                             'handle' : self.out_tokens[0],
                             'scope' : self.scope_index,
                             'post' : True
                             }]
        return instances
        
    def get_inline_processing_code(self, in_tokens):
        code = ''
        if self._input_block and 'blockbundle' in self._input_block:
            in_tokens = ['_%s_in'%self.handle]
        if self._output_block:
            if 'size' in self._output_block:
                code = templates.module_processing_code(self.handle, in_tokens, '_' + self.name + '_%03i_out'%self._index)
            else:
                code = templates.module_processing_code(self.handle, in_tokens, self.out_tokens[0])
        else:
            code = templates.module_processing_code(self.handle, in_tokens, '')
        return code
    
    def get_initialization_code(self, in_tokens):
        code = ''
        ports = self.function['ports']
        for port_name in ports:
            if 'value' in ports[port_name]:
                if type(ports[port_name]['value']) == unicode:
                     port_in_token = [ '"' + ports[port_name]['value'] + '"']
                else:
                    port_in_token = [str(ports[port_name]['value'])]
            elif 'name' in ports[port_name]:
                port_in_token = [ports[port_name]['name']['name']]
            elif 'expression' in ports[port_name]:
                port_in_token = [self.port_name_atoms[port_name][0].get_handles()[0]]
            else:
                port_in_token = ['____XXX___'] # TODO implement
            code += templates.module_set_property(self.handle, port_name, port_in_token)
            
        if self._input_block and 'blockbundle' in self._input_block:
            new_code = templates.declaration_bundle_real('_%s_in'%self.handle, len(in_tokens)) + '\n'
            for i in range(len(in_tokens)):
                new_code += templates.assignment('_%s_in[%i]'%(self.handle, i), in_tokens[i])
            code += new_code
        return code
    
    def get_preprocessing_code(self, in_tokens):
        code = ''
        ports = self.function['ports']
        for port_name in ports:
            if 'value' in ports[port_name]:
                if type(ports[port_name]['value']) == unicode:
                     port_in_token = [ '"' + ports[port_name]['value'] + '"']
                else:
                    port_in_token = [str(ports[port_name]['value'])]
            elif 'name' in ports[port_name]:
                port_in_token = [ports[port_name]['name']['name']]
            elif 'expression' in ports[port_name]:
                port_in_token = [self.port_name_atoms[port_name][0].get_handles()[0]]
            else:
                port_in_token = ['____XXX___'] # TODO implement
            code += templates.module_set_property(self.handle, port_name, port_in_token)
            
        if self._input_block and 'blockbundle' in self._input_block:
            new_code = templates.declaration_bundle_real('_%s_in'%self.handle, len(in_tokens)) + '\n'
            for i in range(len(in_tokens)):
                new_code += templates.assignment('_%s_in[%i]'%(self.handle, i), in_tokens[i])
            code += new_code
        return code
        
    def get_processing_code(self, in_tokens):
        #code = self.get_preprocessing_code(in_tokens)
        code = '' 
        out_tokens = self.out_tokens
        if 'output' in self.module and not self.module['output'] is None: #For Platform types
            if self.inline:
                code += self.get_handles()[0]
            else:
                code += templates.expression(self.get_inline_processing_code(in_tokens)) 
        else:
            code += templates.expression(self.get_inline_processing_code(in_tokens))
        return code, out_tokens
        
#    def _get_internal_header_code(self):
#        code = self.code['domain_code']['header_code']
#        return code
        
    def _get_internal_processing_code(self):
        code = self.code['domain_code']['processing_code']
        code += templates.module_output_code(self._output_block) 
        return code
        
    def _get_internal_properties_code(self):
        prop_code = {}
        if self.module['properties']:
            for prop in self.module['properties']:
                if 'block' in prop:
                    decl = self.platform.find_declaration_in_tree(prop['block']['block']['name']['name'],
                                                                  self.platform.tree + self._blocks)
                    #FIXME implement for bundles
                    domain = decl['domain']
                    if not domain in prop_code:
                        prop_code[domain] = []
                    prop_type = self.get_block_types(decl)[0]
                    functions = templates.module_property_setter(prop['block']['name'],
                                             prop['block']['block']['name']['name'],
                                             prop_type)
                    prop_code[domain].append(functions)
        return prop_code
        
    def _process_module(self, streams):
        tree = streams
        instanced = []
        if self._input_block: # Mark input block as instanced so it doesn't get instantiated as other blocks
            if 'block' in self._input_block:
                block_type = 'block'
            else:
                block_type = 'blockbundle'
            instanced = [[self._input_block[block_type]['name'], self.scope_index]]
            
        if 'ports' in self.function:
            for name,port_value in self.function['ports'].iteritems():
                new_atom = self.platform.make_atom(port_value)
                new_atom.scope_index += 1 # Hack to bring it up to the right scope...
                if name in self.port_name_atoms:
                    self.port_name_atoms[name].append(new_atom)
                else:
                    self.port_name_atoms[name] = [new_atom]
        
        self.code = self.platform.generate_code(tree, self._blocks,
                                                instanced = instanced)
                                                

        for section in self.global_sections:
            if section in self.globals:
                if section in self.code['global_groups']:
                    self.globals[section].extend(self.code['global_groups'][section])
                else:
                    pass
            else:
                self.globals[section] = self.code['global_groups'][section]



    def _init_blocks(self, blocks, input_name, output_name):
        self._blocks = []
        for block in blocks:
            self._blocks.append(block)
            if 'block' in block:
                block_type = 'block'
            elif 'blockbundle' in block:
                block_type = 'blockbundle'
            if input_name and 'name' in input_name and self._blocks[-1][block_type]['name'] == input_name["name"]['name']:
                self._input_block = self._blocks[-1]
            if output_name and 'name' in output_name and  self._blocks[-1][block_type]['name'] == output_name["name"]["name"]:
                self._output_block = self._blocks[-1][block_type]
                
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

class PlatformModuleAtom(ModuleAtom):
    def __init__(self, module, function, platform_code, token_index, platform, scope_index):
        super(PlatformModuleAtom, self).__init__(module, function, platform_code, token_index, platform, scope_index)
        
    

# TODO complete work on reaction block
class ReactionAtom(Atom):
    def __init__(self, reaction, platform_code, token_index, platform, scope_index):
        super(ReactionAtom, self).__init__()
        self.scope_index = scope_index
        self.name = reaction["name"]
        self.handle = self.name + '_%03i'%token_index;
#        self._platform_code = platform_code
        self._output_block = None
        self._index = token_index
        self.platform = platform
        self.reaction = reaction
        self.rate = -1 # Should reactions have rates?
        
        self._init_blocks(reaction["internalBlocks"],
                          reaction["output"])
        self.set_inline(False)
                         
            
    def set_inline(self, inline):
        if inline:
            self.out_tokens = []
        else:
            if self._output_block and 'size' in self._output_block:
                self.out_tokens = ['_' + self.name + '_%03i_out[%i]'%(self._index, i) for i in range(self._output_block['size'])]
            else:
                self.out_tokens = ['_' + self.name + '_%03i_out'%self._index]
        self._process_reaction(self.reaction["streams"])
        self.inline = inline
        
    def get_declarations(self):
        declarations = {}
        if "other_scope_declarations" in self.code:
            declarations[self.domain] = self.code["other_scope_declarations"]

        domain_code = self.code['domain_code']
        
        properties_code = self._get_internal_properties_code()
        for domain, code in domain_code.items():
            header_code = code['header_code'] 
            init_code = code['init_code']
            process_code = code['processing_code']
            properties_domain_code = ''
            if domain in properties_code:
                properties_domain_code = properties_code[domain]
            
        
            declaration = templates.module_declaration(
                    self.name, header_code + properties_domain_code, 
                    init_code, self._output_block , self._input_block, process_code)
            if not domain in declarations:
                declarations[domain] = []
            declarations[domain].append({"name": self.name,
                             "code" :  declaration,
                             'scope' : self.module['stack_index']})
        
        return declarations
    
    def get_instances(self):
        instances = []
        if "other_scope_instances" in self.code:
            for inst in self.code["other_scope_instances"]:
                if inst:
                    inst['post'] = False
                    instances.append(inst)
        for atoms in self.port_name_atoms.itervalues():
            for atom in atoms:
                instances += atom.get_instances()
                
        instances += [{'type' : 'module',
                     'handle': self.handle,
                     'moduletype' : self.name,
                     'scope' : self.scope_index,
                     'post' : True}
                     ]
        if len(self.out_tokens) > 0 and self.module['output']:
            out_block = self.find_internal_block(self.module['output']['name']['name'])
            # FIXME support bundles
            block_types = self.get_block_types(out_block);
            instances += [{ 'type' : block_types[0],
                             'handle' : self.out_tokens[0],
                             'scope' : self.scope_index,
                             'post' : True
                             }]
        return {self.domain: instances}        
        
        
        
    def get_inline_processing_code(self, in_tokens):
        if self._output_block:
            if 'size' in self._output_block:
                code = templates.reaction_processing_code(self.handle, in_tokens, '_' + self.name + '_%03i_out'%self._index)
            else:
                code = templates.reaction_processing_code(self.handle, in_tokens, self.out_tokens[0])
        else:
            code = templates.reaction_processing_code(self.handle, in_tokens, '')
        return code
        
    def get_preprocessing_code(self, in_tokens):
        # TODO Reactions and properties??
        code = ''
#        ports = self.function['ports']
#        for port_name in ports:
#            if 'value' in ports[port_name]:
#                port_in_token = [str(ports[port_name]['value']['value'])]
#            elif 'name' in ports[port_name]:
#                port_in_token = [ports[port_name]['name']['name']]
#            elif 'expression' in ports[port_name]:
#                port_in_token = ['']
#            else:
#                port_in_token = ['____XXX___'] # TODO implement
           # code += templates.module_set_property(self.handle, port_name, port_in_token)
        return code
        
    def get_processing_code(self, in_tokens):
        #code = self.get_preprocessing_code(in_tokens)
        code = ''
        out_tokens = []
        if 'output' in self.reaction and not self.reaction['output'] is None:
            if self.inline:
                code += self.get_handles()[0]
            else:
                if 'size' in self._output_block:
                    code += templates.expression(self.get_inline_processing_code(in_tokens))
                else:
                    code += templates.expression(self.get_inline_processing_code(in_tokens))
            out_tokens = self.out_tokens
        else:
            code += templates.expression(self.get_inline_processing_code(in_tokens))
            out_tokens = self.out_tokens
        return code, out_tokens
        
    def _get_internal_header_code(self):
        code = self.code['header_code']
        return code
        
    def _get_internal_processing_code(self):
        code = self.code['processing_code']
        code += templates.module_output_code(self._output_block) 
        return code
        
    def _get_internal_properties_code(self):
        members = ''
        functions = ''
#        if self.module['properties']:
#            for prop in self.module['properties']:
#                if 'block' in prop:
#                    functions += templates.module_property_setter(prop['block']['name'],
#                                             prop['block']['block']['name']['name'],
#                                             'real')
        return members + functions
        
    def _process_reaction(self, streams):
        tree = streams
        
        # This is from ModuleAtom._process_module(). Needed here?
#        instanced = []
#        if self._input_block:
#            if 'block' in self._input_block:
#                block_type = 'block'
#            else:
#                block_type = 'blockbundle'
#            instanced = [[self._input_block[block_type]['name'], self.scope_index]]
#            
#        if 'ports' in self.function:
#            for name,port in self.function['ports'].iteritems():
#                new_atom = self.platform.make_atom(port)
#                self.port_name_atoms.append(new_atom)
        
        self.code = self.platform.generate_code(tree,self._blocks,
                                                instanced = [])
        
        for section in self.global_sections:
            if section in self.globals:
                if section in self.code[section]:
                    self.globals[section].extend(self.code['global_groups'][section])
                else:
                    pass
            else:
                self.globals[section] = self.code['global_groups'][section]
                                                          

        for section in self.global_sections:
            if section in self.globals:
                if section in self.code['global_groups']:
                    self.globals[section].extend(self.code['global_groups'][section])
                else:
                    pass
            else:
                self.globals[section] = self.code['global_groups'][section]                
                
                
                


        
    def _init_blocks(self, blocks, output_name):
        self._blocks = []
        if blocks:
            for block in blocks:
                self._blocks.append(block)
                if 'block' in block:
                    block_type = 'block'
                elif 'blockbundle' in block:
                    block_type = 'blockbundle'
                if output_name and 'name' in output_name and  self._blocks[-1][block_type]['name'] == output_name["name"]["name"]:
                    self._output_block = self._blocks[-1]
                
    def find_internal_block(self, block_name):
        for block in self._blocks:
            if block['name'] == block_name:
                return block
    

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
                if element['block']['type'] == 'type':
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
    
    def find_function_property(self, func, property_name):
        return func["ports"][property_name]
    
    
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
            new_atom = NameAtom(platform_type, declaration, self.unique_id, scope_index)
        elif "bundle" in member:
            platform_type, declaration = self.find_block(member['bundle']['name'], self.tree)
            new_atom = BundleAtom(platform_type, declaration, member['bundle']['index'], self.unique_id, scope_index)
        elif "function" in member:
            platform_type, declaration = self.find_block(member['function']['name'], self.tree)
            if declaration['type'] == 'module':
                if 'type' in platform_type['block']:
                    new_atom = ModuleAtom(declaration, member['function'], platform_type, self.unique_id, self, scope_index)
                else:
                    raise ValueError("Invalid or unavailable platform type.")
            elif declaration['type'] == 'reaction':
                new_atom = ReactionAtom(declaration, platform_type, self.unique_id, self, scope_index)
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
        elif "block" in member:
            if 'type' in member['block']:
                platform_type = self.find_stride_type(member['block']["type"])
            else:
                platform_type =  self.find_stride_type(member['block']["platformType"])
            new_atom = NameAtom(platform_type, member['block'], self.unique_id, scope_index)
        elif "blockbundle" in member:
            if 'type' in member:
                platform_type = self.find_stride_type(member['blockbundle']["type"])
            else:
                platform_type =  self.find_stride_type(member['blockbundle']["platformType"])
            new_atom = NameAtom(platform_type, member['blockbundle'], self.unique_id, scope_index)
        else:
            raise ValueError("Unsupported type")
        return new_atom
      
    def push_scope(self, scope):
        self.scope_stack.append(scope)
    
    def pop_scope(self):
        self.scope_stack.pop()
        
    def make_stream_nodes(self, stream):
        node_groups = [[]] # Nodes are grouped by domain
        current_domain = ''
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
            self.log_debug("New atom: " + str(new_atom.handle) + " domain: :" + current_domain)
            cur_group.append(new_atom)

            self.unique_id += 1
        return node_groups
        
    def generate_code_from_groups(self, node_groups, global_groups, declared, instanced, initialized):
#        declare_code = ''
#        instantiation_code = ''
        init_code = ''
        header_code = ''
        processing_code = '' 
        post_processing = []
        parent_rates_size = templates.rate_stack_size() # To know now much we need to pop for this stream
        
        header = []
        
        self.log_debug(">>> Start stream generation")
        for group in node_groups:
            in_tokens = []
            previous_atom = None
            for atom in group:
                self.log_debug("Processing atom: " + str(atom.handle))
                #Process Inlcudes
                new_globals = atom.get_globals()
                if len(new_globals) > 0:
                    for global_group in new_globals:
                        global_groups[global_group] += new_globals[global_group]

                declares = atom.get_declarations()
                
                self.log_debug("Declarations " + str(declares))
                new_instances = atom.get_instances()
                self.log_debug("New instances " + str(new_instances))
                header.append([declares, new_instances])
                
                init_code += atom.get_initialization_code(in_tokens)
                
                processing_code += atom.get_preprocessing_code(in_tokens)
                # Process processing code
                code, out_tokens = atom.get_processing_code(in_tokens)
                new_postprocs = atom.get_postproc_once()
                if new_postprocs:
                    for new_postproc in new_postprocs:
                        if new_postproc:
                            postproc_present = False
                            for postproc in post_processing:
                                if postproc[0] == new_postproc[0]:
                                    postproc_present = True
                            # Do we need to order the post processing code?
                            if not postproc_present:
                                post_processing.append(new_postproc)
                self.log_debug("Code:  " + str(code))
                if atom.rate > 0:
                    new_inst, new_init, new_proc = templates.rate_start(atom.rate)
                    processing_code += new_proc
                    header_code += new_inst
                    init_code += new_init
                    # We want to avoid inlining across rate boundaries
                    if previous_atom:
                        previous_atom.set_inline(False)
                    atom.set_inline(False)

                if code:
                    processing_code += code + '\n'
                in_tokens = out_tokens
            
            previous_atom = atom
            
        #It might be useful in the future to process instance code and declarations 
        # together at once, e.g. to keep them in a large memory block or struct
        other_scope_declarations = []
        other_scope_instances = []

        for postproc in post_processing:
            processing_code += postproc[1] + '\n'
        
        for new_header in header:
            for domain, new_decs in new_header[0].items():
                # FIXME doing >= solves the issue of using a Level instance
                # within an Oscillator. The scope for the Output signal is
                # currently marked as within the Oscillator instead of the 
                # Level declared scope.
                for new_dec in new_decs:
                    if new_dec['scope'] >= len(self.scope_stack) - 1: # if declaration in this scope
                        is_declared = False
                        for d in declared:
                            if d[0] == new_dec['name'] and d[1] == new_dec['scope']:
                                is_declared = True
                                break
                        if not is_declared:
                            declared.append( [new_dec['name'], new_dec['scope'] ])
                            header_code += new_dec['code']
                    else:
                        other_scope_declarations.append(new_dec)
            
            
            for domain, new_instances in new_header[1].items():
                # FIXME doing >= solves the issue of using a Level instance
                # within an Oscillator. The scope for the Output signal is
                # currently marked as within the Oscillator instead of the 
                # Level declared scope.
                for new_inst in new_instances:
                    if new_inst and new_inst['scope'] >= len(self.scope_stack) - 1: # if instance is declared in this scope
                        is_declared = False
                        for i in instanced:
                            if i[0] == new_inst['handle'] and i[1] == new_inst['scope']:
                                is_declared = True
                                break
                        if not is_declared:
                            instanced.append([new_inst['handle'], new_inst['scope'] ])
                            new_inst_code = templates.instantiation_code(new_inst)
                            if 'post' in new_inst and new_inst['post']:
                                header_code += new_inst_code
                            else:
                                header_code = new_inst_code + header_code
                            if 'code' in new_inst:
                                init_code +=  templates.initialization_code(new_inst)
                    else:
                        other_scope_instances.append(new_inst)
            
        # Close pending rates in this stream
        while not parent_rates_size == templates.rate_stack_size():
            processing_code += templates.rate_end_code()

        self.log_debug(">>> End stream generation")
        return [header_code, init_code, processing_code,
                other_scope_instances, other_scope_declarations]
        
    def generate_stream_code(self, stream, stream_index, global_groups, declared, instanced, initialized):
        self.log_debug("-- Start stream")       
        node_groups = self.make_stream_nodes(stream)
        
        processing_code = templates.stream_begin_code%stream_index
        
        new_code = self.generate_code_from_groups(node_groups, global_groups, declared, instanced, initialized)
        header_code, init_code, new_processing_code, other_scope_instances, other_scope_declarations = new_code
        processing_code += new_processing_code
            
        processing_code += templates.stream_end_code%stream_index
        
        domain = "AudioDomain" #TODO use actual domain from platform and domain declarations.

        self.log_debug("-- End stream")
        
        return {"global_groups" : global_groups,
                "header_code" : header_code,
                "init_code" : init_code,
                "processing_code" : processing_code,
                "other_scope_instances" : other_scope_instances,
                "other_scope_declarations" : other_scope_declarations,
                "domain" : domain}
                
    def get_domains(self):
        domains = []
        for node in self.tree:
            if 'block' in node:
                if 'type' in node['block'] and node['block']['type'] == '_domainDefinition':
                    domains.append(node['block'])
        return domains
                
    def generate_code(self, tree, current_scope = [],
                      global_groups = {'include':[], 'includeDir':[], 'initializations' : [], 'linkTo' : [], 'linkDir' : []},
                      declared = [], instanced = [], initialized = []):  
        stream_index = 0
        other_scope_instances = []
        other_scope_declarations = []
        
        self.log_debug("* New Generation ----- scopes: " + str(len(self.scope_stack)))
        self.push_scope(current_scope)
        
        domain_code = {}
        

        for node in tree:
            if 'stream' in node: # Everything grows from streams.
                code = self.generate_stream_code(node["stream"], stream_index,
                                                 global_groups, declared, instanced,
                                                 initialized)
                if not code['domain'] in domain_code:
                    domain_code[code['domain']] = { "header_code": '',
                        "init_code" : '',
                        "processing_code" : '' }
                domain_code[code['domain']]["header_code"] += code["header_code"]
                domain_code[code['domain']]["init_code"]  += code["init_code"]
                domain_code[code['domain']]["processing_code"] += code["processing_code"]
                stream_index += 1
                other_scope_instances += code['other_scope_instances']
                other_scope_declarations += code["other_scope_declarations"]
                
        self.log_debug("* Ending Generation -----")
        
        self.pop_scope()
        return {"global_groups" : code['global_groups'],
                "domain_code": domain_code,
                "other_scope_instances" : other_scope_instances,
                "other_scope_declarations" : other_scope_declarations}
     
if __name__ == '__main__':
    pass