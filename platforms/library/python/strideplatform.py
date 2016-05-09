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
    
    def get_declarations(self):
        return []
    
    def get_instances(self):
                # Hardware platform definition
        return []
        
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
    
    def get_declarations(self):
        return []
    
    def get_instances(self):
        if self.is_inline():
            return []
        else:
            return [{'type' : 'real',
                     'handle' : self.handle,
                     'code' : self.get_inline_processing_code([]),
                     'scope' : self.scope_index
                     }]
        
    def get_inline_processing_code(self, in_tokens):
        return str(self.value)
        
    def get_processing_code(self, in_tokens):
        return '', in_tokens        

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
            instances.append({'handle' : self.handle,
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

            
        if self.right_atom.is_inline():
            right_token = self.right_atom.get_inline_processing_code([])
        else:
            right_token = self.right_atom.get_out_tokens()[0]
            
        code = '(' + self._operator_symbol(left_token, right_token) + ')'
        return code
    
    def get_preprocessing_code(self, in_tokens):
        left_code = self.left_atom.get_preprocessing_code([])
        right_code = self.right_atom.get_preprocessing_code([])
        return left_code + right_code
    
    def get_processing_code(self, in_tokens):       
        code = self.get_preprocessing_code(in_tokens)
        if self.is_inline():
            out_tokens = [self.get_inline_processing_code(in_tokens)]
        else:
            left_code = self.left_atom.get_processing_code([])[0]
            right_code = self.right_atom.get_processing_code([])[0]
            code += left_code + right_code
            code += templates.assignment(self.handle,
                                         self._operator_symbol(self.left_atom.get_out_tokens()[0],
                                                               self.right_atom.get_out_tokens()[0]))
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
            out_type = 'bool'
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
        self.declarations = [elem.get_declarations() for elem in list_node]
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
        return self.declarations[index]
    
    def get_instances(self, index = -1):
        if index == -1:
            flat_instances = []
            for instance in self.instances:
                flat_instances.extend(instance)
            return flat_instances
        return self.instances[index]
        
    def get_inline_processing_code(self, in_tokens):
        return str(self.value)
        
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
        if 'include' in platform_type['block']:
            if 'include' in self.globals:
                self.globals['include'].extend([inc['value'] for inc in platform_type['block']['include']])
            else:
                self.globals['include'] = [inc['value'] for inc in platform_type['block']['include']]
        if 'initialization' in platform_type['block']:
            if 'initialization' in self.globals:
                self.globals['initialization'] += platform_type['block']['initialization']
            else:
                self.globals['initialization'] = platform_type['block']['initialization']
                
        
        self.set_inline(False)
        
        if 'numOutputs' in platform_type['block']:
            if platform_type['block']['numOutputs'] > 0:
                self.set_inline(True)
        
        if 'rate' in declaration:
            if type(declaration['rate']) == dict:
                self.rate = -1
            else:
                self.rate = declaration['rate']
        else:
            raise ValueError("Parser must fill defaults.")
            #this should never happen... The parser should fill defaults...
        
    def get_declarations(self):
        if 'declarations' in self.platform_type['block']:
            return templates.get_platform_declarations(self.platform_type['block']['declarations'],
                                                       self.scope_index)
        return []
    
    def get_instances(self):
        default_value = self._get_default_value()
        if 'type' in self.declaration and self.declaration['type'] == 'signal':
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
                inits = [{'handle' : self.handle,
                          'type' : 'real',
                          'code' : str(default_value),
                          'scope' : self.declaration['stack_index']
                          }]
            elif self.platform_type['block']['type'] == 'platformType':
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
        return inits
        
    def get_inline_processing_code(self, in_tokens):
        code = ''
        if 'processing' in self.platform_type['block']:
            code = templates.get_platform_inline_processing_code(
                            self.platform_type['block']['processing'],
                            in_tokens,
                            self.platform_type['block']['numInputs'],
                            self.platform_type['block']['numOutputs'])
        else:
            if len(in_tokens) > 0:
                code = in_tokens[0]
            else:
                code = self.handle
            
        return  code
    
    def get_processing_code(self, in_tokens):
        code = ''
        out_tokens = [self.handle]
        if 'processing' in self.platform_type['block']:
            if self.inline:
                out_tokens = [self.get_inline_processing_code(in_tokens)]
            else:
                if self.platform_type['block']['numOutputs'] > 0:
                    code = templates.assignment(self.handle, self.get_inline_processing_code(in_tokens))
                else:
                    code = templates.expression(self.get_inline_processing_code(in_tokens))
        else:
            code = templates.assignment(self.handle, self.get_inline_processing_code(in_tokens))
        return code, out_tokens
        
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
#        if not 'blockbundle' in self.platform_type and not 'platformType' in self.platform_type['block']['type']:
#            raise ValueError("Need a block bundle platform type to make a Bundle Atom.")
        
    
    def get_instances(self):
        default_value = self._get_default_value()

        instances = [{'handle' : self.handle,
                      'code' : str(default_value),
                      'type' : 'bundle',
                      'bundletype' : 'real',
                      'size' : self.declaration['size'],
                      'scope' : self.scope_index
                      }]     
            
                
        return instances
        
    def get_inline_processing_code(self, in_tokens): 
        code = super(BundleAtom, self).get_inline_processing_code(in_tokens)
        if 'processing' in self.platform_type['block']:
            code = templates.get_platform_inline_processing_code(
                            self.platform_type['block']['processing'],
                            in_tokens,
                            self.platform_type['block']['numInputs'],
                            self.platform_type['block']['numOutputs'],
                            self.index)
        return  code
    
    def get_processing_code(self, in_tokens):
        code = ''
        if len(in_tokens) > 0:
            code = templates.assignment(self._get_token_name(self.index),
                                        self.get_inline_processing_code(in_tokens))
                                       
        if 'processing' in self.platform_type['block']:
            code = self.get_inline_processing_code(in_tokens)
        
        out_tokens = [self._get_token_name(self.index)]
        return code, out_tokens       
    
        
    def _get_token_name(self, index):
        return '%s[%i]'%(self.handle, index)
    

class ModuleAtom(Atom):
    def __init__(self, module, function, platform_code, token_index, platform, scope_index):
        super(ModuleAtom, self).__init__()
        self.scope_index = scope_index
        self.name = module["name"]
        self.handle = self.name + '_%03i'%token_index;
        self.current_scope = module["internalBlocks"]
        self._platform_code = platform_code
        self._input_block = None
        self._output_block = None
        self._index = token_index
        self.platform = platform
        self.module = module
        self.rate = -1 # Should modules have rates?
        self.function = function
        
        self._init_blocks(module["internalBlocks"],
                          module["input"], module["output"])
        self.set_inline(False)
                         
            
    def set_inline(self, inline):
        if inline:
            self.out_tokens = []
        else:
            if self._output_block and 'size' in self._output_block:
                self.out_tokens = ['_' + self.name + '_%03i_out[%i]'%(self._index, i) for i in range(self._output_block['size'])]
            else:
                self.out_tokens = ['_' + self.name + '_%03i_out'%self._index]
        self._process_module(self.module["streams"])
        self.inline = inline
        
    def get_declarations(self):
        declarations = []
        if "other_scope_declarations" in self.code:
            declarations += self.code["other_scope_declarations"]
        header_code = self._get_internal_header_code()
        init_code = self._get_internal_init_code()
        process_code = self._get_internal_processing_code()
        properties_code = self._get_internal_properties_code()
        
        declaration = templates.module_declaration(
                self.name, header_code + properties_code, 
                init_code, self._output_block , self._input_block, process_code)
        declarations += [{"name": self.name,
                         "code" :  declaration,
                         'scope' : self.module['stack_index']}]
        return declarations
    
    def get_instances(self):
        instances = []
        if "other_scope_instances" in self.code:
            for inst in self.code["other_scope_instances"]:
                inst['post'] = False
                instances.append(inst)
        instances += [{'type' : 'module',
                     'handle': self.handle,
                     'moduletype' : self.name,
                     'scope' : self.scope_index,
                     'post' : True}
                     ]
        if len(self.out_tokens) > 0:
            instances += [{ 'type' : 'real',
                             'handle' : self.out_tokens[0],
                             'scope' : self.scope_index,
                             'post' : True
                             }]
        return instances
        
    def get_inline_processing_code(self, in_tokens):
        code = ''
        if self._output_block:
            if 'size' in self._output_block:
                code = templates.module_processing_code(self.handle, in_tokens, '_' + self.name + '_%03i_out'%self._index)
            else:
                code = templates.module_processing_code(self.handle, in_tokens, '')
        return code
        
    def get_preprocessing_code(self, in_tokens):
        code = ''
        ports = self.function['ports']
        for port_name in ports:
            if 'value' in ports[port_name]:
                port_in_token = [str(ports[port_name]['value'])]
            elif 'name' in ports[port_name]:
                port_in_token = [ports[port_name]['name']['name']]
            elif 'expression' in ports[port_name]:
                port_in_token = ['']
            else:
                port_in_token = ['____XXX___'] # TODO implement
            code += templates.module_set_property(self.handle, port_name, port_in_token)
        return code
        
    def get_processing_code(self, in_tokens):
        code = self.get_preprocessing_code(in_tokens)
        out_tokens = []
        if 'output' in self._platform_code['block'] and not self._platform_code['block']['output'] is None:
            if self.inline:
                code += self.get_handles()[0]
            else:
                if 'size' in self._output_block:
                    code += templates.expression(self.get_inline_processing_code(in_tokens))
                else:
                    code += templates.assignment(self.out_tokens[0],self.get_inline_processing_code(in_tokens))
            out_tokens = self.out_tokens
        else:
            code += templates.expression(self.get_inline_processing_code(in_tokens))
            out_tokens = self.out_tokens
        return code, out_tokens
        
    def _get_internal_header_code(self):
        code = self.code['header_code']
        return code
        
    def _get_internal_init_code(self):
        code = self.code['init_code']
        return code
        
    def _get_internal_processing_code(self):
        code = self.code['processing_code']
        code += templates.module_output_code(self._output_block) 
        return code
        
    def _get_internal_properties_code(self):
        members = ''
        functions = ''
        if self.module['properties']:
            for prop in self.module['properties']:
                if 'block' in prop:
                    functions += templates.module_property_setter(prop['block']['name'],
                                             prop['block']['block']['name']['name'],
                                             'real')
        return members + functions
        
    def _process_module(self, streams):
        tree = streams
        instanced = []
        if self._input_block:
            instanced = [[self._input_block['name'], self.scope_index]]
        
        self.code = self.platform.generate_code(tree, self.current_scope,
                                                instanced = instanced)

        if 'include' in self.globals and 'include' in self.code['global_groups']:
            self.globals['include'].extend(self.code['global_groups']['include'])
        else:
            self.globals['include'] = self.code['global_groups']['include']


        
    def _init_blocks(self, blocks, input_name, output_name):
        self._blocks = []
        for block in blocks:
            if 'block' in block:
                self._blocks.append(block['block'])
            elif 'blockbundle' in block:
                self._blocks.append(block['blockbundle'])
            if input_name and 'name' in input_name and self._blocks[-1]['name'] == input_name["name"]['name']:
                self._input_block = self._blocks[-1]
            if output_name and 'name' in output_name and  self._blocks[-1]['name'] == output_name["name"]["name"]:
                self._output_block = self._blocks[-1]
                
    def find_internal_block(self, block_name):
        for block in self._blocks:
            if block['name'] == block_name:
                return block

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
        self.current_scope = reaction["internalBlocks"]
        self._platform_code = platform_code
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
        declarations = []
        if "other_scope_declarations" in self.code:
            declarations += self.code["other_scope_declarations"]
        header_code = self._get_internal_header_code()
        init_code = self._get_internal_init_code()
        process_code = self._get_internal_processing_code()
        properties_code = self._get_internal_properties_code()
        
        declaration = templates.reaction_declaration(
                self.name, header_code + properties_code, 
                init_code, self._output_block , process_code)
        declarations += [{"name": self.name,
                         "code" :  declaration,
                         'scope' : self.reaction['stack_index']}]
        return declarations
    
    def get_instances(self):
        instances = []       
        if "other_scope_instances" in self.code:
            for inst in self.code["other_scope_instances"]:
                inst['post'] = False
                instances.append(inst)
        if len(self.out_tokens) > 0:
            instances += [{'type' : 'module',
                           'handle': self.handle,
                           'moduletype' : self.name,
                           'scope' : self.scope_index,
                           'post' : True
                           },
                           { 'type' : 'real',
                             'handle' : self.out_tokens[0],
                             'scope' : self.scope_index,
                             'post' : True
                             }]
        else:
            instances += [{'type' : 'module',
                     'handle': self.handle,
                     'moduletype' : self.name,
                     'scope' : self.scope_index,
                     'post' : True}
                     ]
        return instances
        
    def get_inline_processing_code(self, in_tokens):
        if self._output_block:
            if 'size' in self._output_block:
                code = templates.reaction_processing_code(self.handle, in_tokens, '_' + self.name + '_%03i_out'%self._index)
            else:
                code = templates.reaction_processing_code(self.handle, in_tokens, '')
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
        code = self.get_preprocessing_code(in_tokens)
        out_tokens = []
        if 'output' in self._platform_code['block'] and not self._platform_code['block']['output'] is None:
            if self.inline:
                code += self.get_handles()[0]
            else:
                if 'size' in self._output_block:
                    code += templates.expression(self.get_inline_processing_code(in_tokens))
                else:
                    code += templates.assignment(self.out_tokens[0],self.get_inline_processing_code(in_tokens))
            out_tokens = self.out_tokens
        else:
            code += templates.expression(self.get_inline_processing_code(in_tokens))
            out_tokens = self.out_tokens
        return code, out_tokens
        
    def _get_internal_header_code(self):
        code = self.code['header_code']
        return code
        
        
    def _get_internal_init_code(self):
        code = self.code['init_code']
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
        self.code = self.platform.generate_code(tree,self.current_scope,
                                                instanced = [])
        if 'include' in self.globals and 'include' in self.code['global_groups']:
            self.globals['include'].extend(self.code['global_groups']['include'])
        else:
            self.globals['include'] = self.code['global_groups']['include']


        
    def _init_blocks(self, blocks, output_name):
        self._blocks = []
        for block in blocks:
            if 'block' in block:
                self._blocks.append(block['block'])
            elif 'blockbundle' in block:
                self._blocks.append(block['blockbundle'])
            if output_name and 'name' in output_name and  self._blocks[-1]['name'] == output_name["name"]["name"]:
                self._output_block = self._blocks[-1]
                
    def find_internal_block(self, block_name):
        for block in self._blocks:
            if block['name'] == block_name:
                return block
    

# --------------------- Common platform functions
class PlatformFunctions:
    def __init__(self, platform_dir, tree):
        
        self.defined_modules =[]
    
        self.tree = tree
        self.scope_stack = []
        
        self.sample_rate = 44100 # Set this as default but this should be overriden by platform:
        for elem in tree:
            if 'block' in elem:
                if elem['block']['name'] == 'PlatformRate':
                    self.sample_rate = elem['block']['value']
        templates.domain_rate = self.sample_rate
        self.unique_id = 0
    
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
        for i, scope in enumerate(self.scope_stack):
            for node in scope: # Now look within scope
                if 'block' in node:
                    if node["block"]["name"] == block_name:
                        node["block"]['stack_index'] = len(self.scope_stack) + i
                        return node["block"]
                if 'blockbundle' in node:
                    if node["blockbundle"]["name"] == block_name:
                        node["blockbundle"]['stack_index'] = len(self.scope_stack) + 1
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
                if 'platformType' in platform_type['block']:
                    new_atom = PlatformTypeAtom(declaration, member['function'], platform_type, self.unique_id, self, scope_index)
                elif 'type' in platform_type['block']:
                    new_atom = ModuleAtom(declaration, member['function'], platform_type, self.unique_id, self, scope_index)
                else:
                    raise ValueError("Invalid or unavailable platform type.")
            elif declaration['type'] == 'reaction':
                new_atom = ReactionAtom(declaration, platform_type, self.unique_id, self, scope_index)
            elif declaration['type'] == 'platformModule':
                new_atom = PlatformModuleAtom(declaration, member['function'], platform_type, self.unique_id, self, scope_index)
        elif "expression" in member:
            if 'value' in member['expression']: # Unary expression
                left_atom = self.make_atom(member['expression']['left'])
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
        else:
            raise ValueError("Unsupported type")
        return new_atom
      
    def push_scope(self, scope):
        self.scope_stack.append(scope)
    
    def pop_scope(self):
        self.scope_stack.pop()
        
    def make_stream_nodes(self, stream):
        node_groups = [[]] # Nodes are grouped by rate
        
        for member in stream: #Only instantiate whatever is used in streams. Discard the rest
            cur_group = node_groups[-1]  
            new_atom = self.make_atom(member)             
            cur_group.append(new_atom)

            self.unique_id += 1
        return node_groups
        
    def generate_code_from_groups(self, node_groups, global_groups, declared, instanced, initialized):
#        declare_code = ''
#        instantiation_code = ''
        init_code = ''
        header_code = ''
        processing_code = '' 
        parent_rates_size = templates.rate_stack_size() # To know now much we need to pop for this stream
        
        header = []
        for group in node_groups:
            in_tokens = []
            previous_atom = None
            for atom in group:
                #Process Inlcudes
                new_globals = atom.get_globals()
                if len(new_globals) > 0:
                    for group in new_globals:
                        if group == 'include':
                             global_groups['include'] += new_globals[group]
                        elif group == 'linkTo':
                             global_groups['linkTo']  += new_globals[group]
                        elif group == 'initialization':
                             global_groups['initialization']  += new_globals[group]

                declares = atom.get_declarations()
                new_instances = atom.get_instances()
                header.append([declares, new_instances])
                
                # Process processing code
                code, out_tokens = atom.get_processing_code(in_tokens)
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
        for new_header in header:
            for new_dec in new_header[0]:
                # FIXME This should not be >= but == instead. This is what is working now though...
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
            
            for new_inst in new_header[1]:
                # FIXME This should not be >= but == instead. This is what is working now though...
                if new_inst['scope'] >= len(self.scope_stack) - 1: # if instance is declared in this scope
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

        return [header_code, init_code, processing_code,
                other_scope_instances, other_scope_declarations]
        
    def generate_stream_code(self, stream, stream_index, global_groups, declared, instanced, initialized):
        node_groups = self.make_stream_nodes(stream)
        
        processing_code = templates.stream_begin_code%stream_index
        
        new_code = self.generate_code_from_groups(node_groups, global_groups, declared, instanced, initialized)
        header_code, init_code, new_processing_code, other_scope_instances, other_scope_declarations = new_code
        processing_code += new_processing_code
            
        processing_code += templates.stream_end_code%stream_index
        
        return {"global_groups" : global_groups,
                "header_code" : header_code,
                "init_code" : init_code,
                "processing_code" : processing_code,
                "other_scope_instances" : other_scope_instances,
                "other_scope_declarations" : other_scope_declarations}
                
    def generate_code(self, tree, current_scope = [],
                      global_groups = {'include':[], 'initialization' : [], 'linkTo' : []},
                      declared = [], instanced = [], initialized = []):  
        stream_index = 0
        header_code = ''
        init_code = ''
        processing_code = ''
        other_scope_instances = []
        other_scope_declarations = []
        
        self.push_scope(current_scope)

        for node in tree:
            if 'stream' in node: # Everything grows from streams.
                code = self.generate_stream_code(node["stream"], stream_index,
                                                 global_groups, declared, instanced,
                                                 initialized)
                header_code += code["header_code"]
                init_code  += code["init_code"]
                processing_code += code["processing_code"]
                stream_index += 1
                other_scope_instances += code['other_scope_instances']
                other_scope_declarations += code["other_scope_declarations"]
        
        self.pop_scope()
        return {"global_groups" : code['global_groups'],
                "header_code" : header_code,
                "init_code" : init_code,
                "processing_code" : processing_code,
                "other_scope_instances" : other_scope_instances,
                "other_scope_declarations" : other_scope_declarations}
     
if __name__ == '__main__':
    pass