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
        return {}
    
    def get_instances(self):
        return {}
        
    def get_processing_code(self, in_tokens):
        return None
        
    def get_inline_processing_code(self, in_tokens):
        ''' This returns the processing code itself, so this can be used
        when the output is used only once, and an intermediate symbol to 
        represent it is not needed'''
        return None
        
    def get_rate(self):
        return self.rate
        
         
class PlatformTypeAtom(Atom):
    def __init__(self, module, function, platform_type, token_index, platform):
        super(PlatformTypeAtom, self).__init__()
        self.module = module
        self.platform_type = platform_type
        self.index = token_index
        self.platform = platform
        self.function = function
        
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
        return {}
    
    def get_instances(self):
                # Hardware platform definition
        return {}
        
    def get_processing_code(self, in_tokens):
        return 'PROC_CODEEEEE'
        
    def get_inline_processing_code(self, in_tokens):
        return 'INLINE_CODEEEEE'
        
class ValueAtom(Atom):
    def __init__(self, value_node, index):
        super(ValueAtom, self).__init__()
        self.index = index
        self.value = value_node['value']
        self.handle = '__value_%03i'%index
        self.rate = 0
        self.inline = True
        
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
        return {}
    
    def get_instances(self):
        if self.is_inline():
            return []
        else:
            return [{'type' : 'real',
                     'handle' : self.handle,
                     'code' : self.get_inline_processing_code([])
                     }]
        
    def get_inline_processing_code(self, in_tokens):
        return str(self.value)
        
    def get_processing_code(self, in_tokens):
        return '', in_tokens        

class ExpressionAtom(Atom):
    def __init__(self, expr_type, left_atom, right_atom, index):
        super(ExpressionAtom, self).__init__()
        self.expr_type = expr_type
        self.left_atom = left_atom
        self.right_atom = right_atom
        self.index = index
        self.handle = '__expr_%03i'%index
        self.rate = -1
        self.set_inline(True)
        
    def set_inline(self, inline):
        self.left_atom.set_inline(inline)
        self.right_atom.set_inline(inline)
        self.inline = inline
            
    def get_declarations(self):
        declarations = self.left_atom.get_declarations()
        if self.right_atom:
            declarations.update(self.right_atom.get_declarations())

        return declarations
    
    def get_instances(self):
        instances = self.left_atom.get_instances()
        if self.right_atom:
            instances += self.right_atom.get_instances()
        if not self.is_inline():
            instances.append({'handle' : self.handle,
                              'type' : 'real',
                              'code' : '0.0' #TODO get this value from platform/library default
                              })
        return instances
        
    def get_inline_processing_code(self, in_tokens):
        if self.left_atom.is_inline():
            left_token = self.left_atom.get_inline_processing_code([])
        else:
            left_token = self.left_atom.get_out_tokens()[0]
        code = '(' + left_token
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
            code = ' - ' + code
        elif self.expr_type == 'LogicalNot':
            code += ' ~ ' + code
            
        if self.right_atom.is_inline():
            right_token = self.right_atom.get_inline_processing_code([])
        else:
            right_token = self.right_atom.get_out_tokens()[0]
        code += right_token + ')'
        return code
    
    def get_processing_code(self, in_tokens):
        code = ''        
        code += self.left_atom.get_processing_code([])[0]
        code += self.right_atom.get_processing_code([])[0]
        if self.is_inline():
            code = ''
            out_tokens = [self.get_inline_processing_code(in_tokens)]
        else:
            code = ''
            code += templates.assignment(self.handle, self.get_inline_processing_code(in_tokens))
        
            out_tokens = [self.handle]
        return code, out_tokens
        
class ListAtom(Atom):
    def __init__(self, list_node):
        super(ListAtom, self).__init__()
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
    def __init__(self, platform_type, declaration, token_index):
        super(NameAtom, self).__init__()
        self.name = declaration['name']
        self.handle = self.name # + '_%03i'%token_index;
        self.platform_type = platform_type
        self.declaration = declaration
        if 'include' in platform_type['block']:
            if 'include' in self.globals:
                self.globals['include'].extend([inc['value']['value'] for inc in platform_type['block']['include']])
            else:
                self.globals['include'] = [inc['value']['value'] for inc in platform_type['block']['include']]
        if 'initialization' in platform_type['block']:
            if 'initialization' in self.globals:
                self.globals['initialization'] += platform_type['block']['initialization']
            else:
                self.globals['initialization'] = platform_type['block']['initialization']
                
        
        self.set_inline(False)
        
        if 'direction' in platform_type['block']:
            if platform_type['block']['direction'] == 'out' or platform_type['block']['direction'] == 'thru':
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
            return templates.get_platform_declarations(self.platform_type['block']['declarations'])
        return {}
    
    def get_instances(self):
        default_value = self._get_default_value()
        if 'type' in self.declaration and self.declaration['type'] == 'signal':
            inits = [{'handle' : self.handle,
                      'type' : 'real',
                      'code' : str(default_value)
                      }]
        elif 'block' in self.platform_type:
            inherits = self.platform_type['block']['inherits']
            if inherits == 'signal':
                inits = [{'handle' : self.handle,
                          'type' : 'real',
                          'code' : str(default_value)
                          }]
            elif self.platform_type['block']['type'] == 'platformType':
                inits = [{'handle' : self.handle,
                          'type' : 'real',
                          'code' : str(default_value)
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
            direction = self.platform_type['block']['direction']
            code = templates.get_platform_inline_processing_code(
                            self.platform_type['block']['processing'],
                            in_tokens,
                            direction)
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
                code = templates.assignment(self.handle, self.get_inline_processing_code(in_tokens))
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
    def __init__(self, platform_type, declaration, index, token_index):
        ''' index indexes from 1, internal index from 0
        '''
        super(BundleAtom, self).__init__(platform_type, declaration, token_index)
        self.index = index - 1
        if not 'blockbundle' in self.platform_type and not 'platformType' in self.platform_type['block']['type']:
            raise ValueError("Need a block bundle platform type to make a Bundle Atom.")
        
    
    def get_instances(self):
        default_value = self._get_default_value()

        instances = [{'handle' : self.handle,
                      'code' : str(default_value),
                      'type' : 'bundle',
                      'bundletype' : 'real',
                      'size' : self.declaration['size']
                      }]     
            
                
        return instances
        
    def get_inline_processing_code(self, in_tokens): 
        code = super(BundleAtom, self).get_inline_processing_code(in_tokens)
        if 'processing' in self.platform_type['block']:
            direction = self.platform_type['block']['direction']
            code = templates.get_platform_inline_processing_code(
                            self.platform_type['block']['processing'],
                            in_tokens,
                            direction,
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
    def __init__(self, module, function, platform_code, token_index, platform):
        super(ModuleAtom, self).__init__()
        self.name = module["name"]
        self.handle = self.name + '_%03i'%token_index;
        self.out_tokens = ['_' + self.name + '_%03i_out'%token_index]
        self.current_scope = module["internalBlocks"]
        self._platform_code = platform_code
        self._input_block = None
        self._output_block = None
        self._index = token_index
        self.platform = platform
        self.stride_type = self.platform.find_stride_type("module")
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
            self.out_tokens = ['_' + self.name + '_%03i_out'%self._index]
        self._process_module(self.module["streams"], self.module["internalBlocks"])
        self.inline = inline
        
    def get_declarations(self):
        declarations_code = self._get_internal_declarations_code()
        
        instantiation_code = self._get_internal_instantiation_code()
        init_code = self._get_internal_init_code()
        process_code = self._get_internal_processing_code()
        properties_code = self._get_internal_properties_code()
        
        declaration = templates.module_declaration(
                self.name, declarations_code + instantiation_code + properties_code, 
                init_code, self._input_block, process_code)
                    
        return {self.name :  declaration}
    
    def get_instances(self):
        if len(self.out_tokens) > 0:
            return [{'type' : 'module',
                     'handle': self.handle,
                     'moduletype' : self.name},
                     { 'type' : 'real',
                       'handle' : self.out_tokens[0]
                     }]
        else:
            return [{'type' : 'module',
                     'handle': self.handle,
                     'moduletype' : self.name}
                     ]
        
    def get_inline_processing_code(self, in_tokens):
        code = templates.module_processing_code(self.handle, in_tokens)
        return code
        
    def get_processing_code(self, in_tokens):
        code = ''
        out_tokens = []
        ports = self.function['ports']
        for port_name in ports:
            if 'value' in ports[port_name]:
                port_in_token = [str(ports[port_name]['value']['value'])]
            else:
                port_in_token = '____XXX___'
            code += templates.module_set_property(self.handle, port_name, port_in_token)
        if 'output' in self._platform_code['block'] and not self._platform_code['block']['output'] is None:
            code += templates.assignment(self.out_tokens[0],self.get_inline_processing_code(in_tokens))
            out_tokens = self.out_tokens
        else:
            code += templates.expression(self.get_inline_processing_code(in_tokens))
            out_tokens = self.out_tokens
        return code, out_tokens
        
    def _get_internal_instantiation_code(self):
        code = self.code['instantiation_code']
        return code
        
    def _get_internal_declarations_code(self):
        code = self.code['declare_code']
        
        return code
        
    def _get_internal_init_code(self):
        code = self.code['init_code']
        return code
        
    def _get_internal_processing_code(self):
        code = self.code['processing_code']
        code += templates.module_output_code(self._output_block) 
        return code
        
    def _get_internal_properties_code(self):
        code = ''
        if self.module['properties']:
            for prop in self.module['properties']:
                if 'block' in prop:
                    code += templates.module_property_setter(prop['block']['name'],
                                             prop['block']['block']['name']['name'],
                                             'real')
        return code
        
    def _process_module(self, streams, blocks):
        tree = streams + blocks
        # We need to pass the name of the input block because we will handle declaration and init
        
                    #self.current_scope = module["internalBlocks"]
        self.platform.push_scope(self.current_scope)
        
        self.code = self.platform.generate_code(tree,
                                                instanced = [self._input_block['name'] if self._input_block else None])
        self.platform.pop_scope()
        if 'include' in self.globals and 'include' in self.code['global_groups']:
            self.globals['include'].extend(self.code['global_groups']['include'])
        else:
            self.globals['include'] = self.code['global_groups']['include']


        
    def _init_blocks(self, blocks, input_name, output_name):
        self._blocks = []
        for block in blocks:
            self._blocks.append(block['block'])
            if input_name and 'name' in input_name and self._blocks[-1]['name'] == input_name["name"]['name']:
                self._input_block = self._blocks[-1]
            if output_name and 'name' in output_name and  self._blocks[-1]['name'] == output_name["name"]["name"]:
                self._output_block = self._blocks[-1]
                
    def find_internal_block(self, block_name):
        for block in self._blocks:
            if block['name'] == block_name:
                return block

class PlatformModuleAtom(ModuleAtom):
    def __init__(self, module, function, platform_code, token_index, platform):
        super(PlatformModuleAtom, self).__init__(module, function, platform_code, token_index, platform)
        
    


# TODO complete work on reaction block
class ReactionAtom(Atom):
    def __init__(self, reaction, token_index, platform):
        self.name = reaction["name"]
        self.handle = self.name + '_%03i'%token_index;
        self.out_tokens = [self.name + '_out_%03i'%token_index]
        self._streams = reaction["streams"]
#        self._properties = reaction["properties"]
        self.current_scope = reaction["internalBlocks"]
        self._input_block = None
        self._output_block = None
        self._index = token_index
        self.platform = platform
        self.stride_type = self.platform.find_stride_type("reaction")
        
        for stream in self._streams:
            print(stream)
        
        self._init_blocks(reaction["internalBlocks"],
                          reaction["input"], reaction["output"])
                          
        self._process_reaction(reaction["streams"], reaction["internalBlocks"])
            
    def get_declarations(self):
        declarations_code = self._get_internal_declarations_code()
        
        instantiation_code = self._get_internal_instantiation_code()
        init_code = self._get_internal_init_code()
        process_code = self._get_internal_processing_code()
        
        declaration = templates.declaration_module(
                self.name, declarations_code + instantiation_code, 
                init_code, self._input_block['name'], process_code)
        
        return {self.handle :  declaration}
    
    def get_instances(self):
        return [{'type' : 'reaction',
                 'handle': self.handle,
                 'reactiontype' : self.name},
                 { 'type' : 'real',
                   'handle' : self.out_tokens[0]
                 }]
        
    def get_inline_processing_code(self, in_tokens):
        code = self.handle + '.process(' + in_tokens[0] + ')'
        return code
        
    def get_processing_code(self, in_tokens):
        code = ''
        if len(in_tokens) > 0:
            code = templates.assignment(self.out_tokens[0],
                                        self.get_inline_processing_code(in_tokens))
        out_tokens = self.out_tokens
        return code, out_tokens
        
    def _get_internal_instantiation_code(self):
        code = self.code['instantiation_code']
        return code
        
    def _get_internal_declarations_code(self):
        code = self.code['declare_code']
        return code
        
    def _get_internal_init_code(self):
        code = self.code['init_code']
        return code
        
    def _get_internal_processing_code(self):
        code = self.code['processing_code']
        code += templates.module_output_code(self._output_block) 
        return code
        
    def _process_reaction(self, streams, blocks):
        tree = streams + blocks
        # We need to pass the name of the input block because we will handle declaration and init
        
                    #self.current_scope = reaction["internalBlocks"]
        self.platform.push_scope(self.current_scope)
        self.code = self.platform.generate_code(tree,
                                                instanced = [self._input_block['name'] if self._input_block else None])
        self.platform.pop_scope()
        if 'include' in self.globals and 'include' in self.code['global_groups']:
            self.globals['include'].extend(self.code['include'])
        else:
            self.globals['include'] = self.code['include']
     
    def _init_blocks(self, blocks, input_name, output_name):
        self._blocks = []
        for block in blocks:
            self._blocks.append(block['block'])
            if 'name' in input_name and self._blocks[-1]['name'] == input_name["name"]['name']:
                self._input_block = self._blocks[-1]
            if 'name' in output_name and  self._blocks[-1]['name'] == output_name["name"]:
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
                    self.sample_rate = elem['block']['rate']
        templates.domain_rate = self.sample_rate

    
    def find_declaration_in_tree(self, block_name, tree):
        for node in tree:
            if 'block' in node:
                if node["block"]["name"] == block_name:
                    return node["block"]
            if 'blockbundle' in node:
                if node["blockbundle"]["name"] == block_name:
                    return node["blockbundle"]
        for scope in self.scope_stack[::-1]:
            for node in scope: # Now look within scope
                if 'block' in node:
                    if node["block"]["name"] == block_name:
                        return node["block"]
                if 'blockbundle' in node:
                    if node["blockbundle"]["name"] == block_name:
                        return node["blockbundle"]
#        raise ValueError("Declaration not found for " + block_name)
        return None
        
    def find_stride_type(self, type_name):
        for element in self.tree:
            if 'block' in element:
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
        if "name" in member:
            platform_type, declaration = self.find_block(member['name']['name'], self.tree)
            new_atom = NameAtom(platform_type, declaration, self.unique_id)
        elif "bundle" in member:
            platform_type, declaration = self.find_block(member['bundle']['name'], self.tree)
            new_atom = BundleAtom(platform_type, declaration, member['bundle']['index'], self.unique_id)
        elif "function" in member:
            module = self.find_declaration_in_tree(member['function']["name"], self.tree)
            if module['type'] == 'module':
                platform_type = self.find_stride_type(member['function']["name"])
                if 'platformType' in platform_type['block']:
                    new_atom = PlatformTypeAtom(module, member['function'], platform_type, self.unique_id, self)
                elif 'type' in platform_type['block']:
                    new_atom = ModuleAtom(module, member['function'], platform_type, self.unique_id, self)
                else:
                    raise ValueError("Invalid or unavailable platform type.")
            elif module['type'] == 'reaction':
                new_atom = ReactionAtom(module, self.unique_id, self)
            elif module['type'] == 'platformModule':
                platform_type = self.find_stride_type(member['function']["name"])
                new_atom = PlatformModuleAtom(module, member['function'], platform_type, self.unique_id, self)
        elif "expression" in member:
            if 'value' in member['expression']: # Unary expression
                left_atom = self.make_atom(member['expression']['left'])
                right_atom = None
            else:
                left_atom = self.make_atom(member['expression']['left'])
                self.unique_id += 1
                right_atom = self.make_atom(member['expression']['right'])
            expression_type = member['expression']['type']
            new_atom = ExpressionAtom(expression_type, left_atom, right_atom, self.unique_id)
        elif "value" in member:
            new_atom = ValueAtom(member['value'], self.unique_id)
        elif "list" in member:
            list_atoms = []
            for element in member['list']:
                element_atom = self.make_atom(element)
                list_atoms.append(element_atom)
            new_atom = ListAtom(list_atoms)
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
        declare_code = ''
        init_code = ''
        processing_code = '' 
        instantiation_code = ''
        parent_rates_size = templates.rate_stack_size() # To know now much we need to pop for this stream
        
        instances = []
        declarations = {}
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
                #Process declaration code
                declares = atom.get_declarations()
                for dec_name in declares:
                    if not dec_name in declared:
                        declared.append(dec_name)
                        declarations[dec_name] = declares[dec_name]
                # Process instantiation code
                new_instances = atom.get_instances()
                for inst in new_instances:
                    if not inst['handle'] in instanced:
                        instanced.append(inst['handle'])
                        instances.append(inst)
                # Process processing code
                code, out_tokens = atom.get_processing_code(in_tokens)
                if atom.rate > 0:
                    processing_code += templates.rate_end_code()
                    templates.rate_start(atom.rate)
                    instantiation_code += templates.rate_instance_code()
                    init_code +=  templates.rate_init_code()
                    processing_code += templates.rate_start_code()
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
        for dec_name in declarations:
            declare_code += declarations[dec_name] + '\n'
        
        for inst in instances:
            instantiation_code += templates.instantiation_code(inst)
            if 'code' in inst:
                init_code +=  templates.initialization_code(inst)
              
        # Close pending rates in this stream
        while not parent_rates_size == templates.rate_stack_size():
            processing_code += templates.rate_end_code()

        return [declare_code, instantiation_code, init_code, processing_code]
        
    def generate_stream_code(self, stream, stream_index, global_groups, declared, instanced, initialized):
        node_groups = self.make_stream_nodes(stream)
        
        processing_code = templates.stream_begin_code%stream_index
        
        declare_code, instantiation_code, init_code, new_processing_code = self.generate_code_from_groups(node_groups, global_groups, declared, instanced, initialized)
        processing_code += new_processing_code
            
        processing_code += templates.stream_end_code%stream_index
        
        return {"global_groups" : global_groups,
                "declare_code" : declare_code,
                "instantiation_code" : instantiation_code,
                "init_code" : init_code,
                "processing_code" : processing_code}
                
    def generate_code(self, tree, global_groups = {'include':[], 'initialization' : [], 'linkTo' : []}, declared = [], instanced = [], initialized = []):  
        self.unique_id = 0
        stream_index = 0
        declare_code = ''
        instantiation_code = ''
        init_code = ''
        processing_code = ''

        for node in tree:
            if 'stream' in node: # Everything grows from streams.
                code = self.generate_stream_code(node["stream"], stream_index,
                                                 global_groups, declared, instanced,
                                                 initialized)
                declare_code += code["declare_code"]
                instantiation_code += code["instantiation_code"]
                init_code  += code["init_code"]
                processing_code += code["processing_code"]
                stream_index += 1
        
        return {"global_groups" : code['global_groups'],
                "declare_code" : declare_code,
                "instantiation_code" : instantiation_code,
                "init_code" : init_code,
                "processing_code" : processing_code}
     
if __name__ == '__main__':
    pass