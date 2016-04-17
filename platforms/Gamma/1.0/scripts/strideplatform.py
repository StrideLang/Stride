# -*- coding: utf-8 -*-
"""
Created on Sat Apr  9 11:16:55 2016

@author: andres
"""

from __future__ import print_function
from __future__ import division

import re
import json

from platformTemplates import templates
  
    
class Atom:
    def __init__(self, index):
        self.index = 0
        self.rate = -1
        self.inline = False
        
    def set_inline(self, inline):
        self.inline = inline
    
    def is_inline(self):
        return self.inline
        
    def get_handle(self):
        if self.is_inline():
            return self.get_inline_processing_code([])
        else:
            return self.handle

    def get_out_tokens(self):
        if hasattr(self, 'out_tokens'):
            return self.out_tokens
        else:
            return [self.handle]
    
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
    
    def get_globals(self): # Global instances, declarations and includes
        return None
        
    def get_rate(self):
        return self.rate
        
        
class ValueAtom(Atom):
    def __init__(self, value_node, index):
        self.index = index
        self.value = value_node['value']
        self.handle = '__value_%03i'%index
        self.rate = 0
        self.inline = True
        
    def get_handle(self):
        if self.is_inline():
            return self.get_inline_processing_code([])
        else:
            return self.handle
            
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
    
    def get_globals(self): # Global instances, declarations and includes
        return None
        

class ExpressionAtom(Atom):
    def __init__(self, expr_type, left_atom, right_atom, index):
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
        
    def get_handle(self):
        if self.is_inline():
            return self.get_inline_processing_code([])
        else:
            return self.handle
            
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
        if self.is_inline():
            code = ''
            out_tokens = [self.get_inline_processing_code(in_tokens)]
        else:
            code = ''
            code += self.left_atom.get_processing_code([])[0]
            code += self.right_atom.get_processing_code([])[0]
            code += templates.assignment(self.handle, self.get_inline_processing_code(in_tokens))
        
            out_tokens = [self.handle]
        return code, out_tokens
        

class NameAtom(Atom):
    def __init__(self, platform_type, declaration, token_index):
        self.name = declaration['name']
        self.handle = self.name # + '_%03i'%token_index;
        self.platform_type = platform_type
        self.declaration = declaration
        
        self.inline = False
        
        if 'rate' in declaration:
            if type(declaration['rate']) == dict:
                self.rate = -1
            else:
                self.rate = declaration['rate']
        else:
            self.rate = 44100 #FIXME this should never happen... The parser should fill defaults...
        
    def get_declarations(self):
        return {}
    
    def get_instances(self):
        if 'default' in self.declaration:
            default_value = self.declaration['default']
        else:
            if 'default' in self.platform_type['block']:
                default_value = self.platform_type['block']['default'] # FIXME inheritance is not being handled here
            else:
                default_value = 0.0
        inits = [{'handle' : self.handle,
                  'type' : 'real',
                  'code' : str(default_value)
                  }]
        return inits
        
    def get_inline_processing_code(self, in_tokens):
        code = self.handle
        if len(in_tokens) > 0:
            code = in_tokens[0]
        return  code
    
    def get_processing_code(self, in_tokens):
        code = ''
        if len(in_tokens) > 0:
            code = templates.assignment(self.handle, self.get_inline_processing_code(in_tokens))
        out_tokens = [self.handle]
        return code, out_tokens

class BundleAtom(Atom):
    def __init__(self, platform_type, declaration, index, token_index):
        ''' index indexes from 1
        '''
        self.name = declaration['name']
        self.index = index - 1
        self.handle = self.name  #+ '_%03i'%(token_index);
        self.platform_type = platform_type
        self.declaration = declaration
        
        if 'rate' in declaration:
            if type(declaration['rate']) == dict:
                self.rate = -1
            else:
                self.rate = declaration['rate']
        else:
            self.rate = -1
        
    def get_declarations(self):
        return {}
    
    def get_instances(self):
        if 'default' in self.declaration:
            default_value = self.declaration['default']

        if 'blockbundle' in self.platform_type:
            #Stride definition
            if 'default' in self.platform_type['blockbundle']:
                default_value = self.platform_type['blockbundle']['default'] # FIXME inheritance is not being handled here
            else:
                default_value = 0.0
            instances = [{'handle' : self.handle,
                      'code' : str(default_value),
                      'type' : 'bundle',
                      'bundletype' : 'real',
                      'size' : self.declaration['size']
                      }]
        else:
            default_value = 0.0 # TODO put actual default
            instances = [{'handle' : self.handle,
                      'code' : str(default_value),
                      'type' : 'bundle',
                      'bundletype' : 'real',
                      'size' : self.declaration['size']
                      }]
#            # Hardware platform definition
#            code = self.platform_type['code']['init_code']['code']
#            code = code.replace('%%token%%', self._get_token_name(self.index))            
#            code = code.replace('%%bundle_index%%', str(self.index))
#            
#            inits = [{'handle' : self.handle,
#                      'code' : code
#                      }]
                
        return instances
    
    def get_inline_processing_code(self, in_tokens):
        code = ''
        if len(in_tokens) > 0:
            code = in_tokens[0] 
        return code
        
    def get_processing_code(self, in_tokens):
        code = ''
        if len(in_tokens) > 0:
            code = templates.assignment(self._get_token_name(self.index),
                                        self.get_inline_processing_code(in_tokens))
        if 'code' in self.platform_type:
            # FIXME Hardware platform code. This needs to be improved...
            code += self.platform_type['code']['dsp_code']['code']
            code = code.replace('%%token%%', self._get_token_name(self.index))            
            code = code.replace('%%bundle_index%%', str(self.index))

        out_tokens = [self._get_token_name(self.index)]
        return code, out_tokens
        
    def _get_token_name(self, index):
        return '%s[%i]'%(self.handle, index)


class PlatformModuleAtom(Atom):
    def __init__(self, platform_type, declaration, token_index):
        self.name = declaration['name']
        self.handle = self.name + '_%03i'%token_index;
        self.platform_type = platform_type
        self.declaration = declaration
        
        ugen_name = "ugen_%02i"%(token_index)
        new_code = self.get_function_code(module, processor["ports"], out_var_name, ugen_name, self._intokens)

        new_dsp_code += new_code["dsp_code"]
        new_init_code += new_code["init_code"]
        if "includes" in new_code:
            self.includes_list.append(new_code["includes"])
        
        if not rate == self.sample_rate:
            self._rated_ugens[ugen_name] = rate_index
        properties = processor["ports"]
        for prop in properties:
            prop_value = properties[prop]
            prop_type = prop_value["type"]
            if prop_type == "Value":
                if prop in module["code"]["dsp_code"]:
                    new_setup_code += self.get_function_property_setup_code(module, prop, str(prop_value["value"]), ugen_name)
            if prop_type == "Name":
                pass
                # FIXME set name token
                #if prop in module["code"]["dsp_code"]:
                #    setup_code += self.platform.get_function_property_setup_code(module, prop, str(prop_value["value"]), ugen_name)
        self.last_num_outs = module["num_outputs"]
       # return {"init_code": new_init_code, "dsp_code" : new_dsp_code, "setup_code" : new_setup_code}
        
        
    def get_declarations(self):
        return {}
    
    def get_instances(self):
        return [{'type' : 'real', 'handle' : self.handle}]
    
#    def get_init_code(self):
#        if 'default' in self.declaration:
#            default_value = self.declaration
#        else:
#            if 'default' in self.platform_type['block']:
#                default_value = self.platform_type['block']['default'] # FIXME inheritance is not being handled here
#            else:
#                default_value = 0.0
#        code = self.handle + ' = ' + str(default_value) + ';\n'
#        return code
    
    def get_processing_code(self, in_tokens):
        code = ''
        if len(in_tokens) > 0:
            code = templates.assignment(self.handle, in_tokens[0])
        out_tokens = [self.handle]
        return code, out_tokens
        
    def get_generated_code(self):
        
        stream_index = 0
        declared = []
        instanced = []
        for i, stream in enumerate(self._streams.values()):
            new_code = self.platform.generate_stream_code(stream, stream_index, declared, instanced)
            stream_index += 1

        new_init_code = self._template_code["init_code"]["code"]
        pattern = re.compile(r'%%:[A-Za-z]+%%')
        for result in re.findall(pattern, new_init_code):
            field = result[3: -2]
            if field == "name":
                new_init_code = new_init_code.replace(result, self.name)  
            else:
                print("unsupported template field :" + result)
#        new_init_code = platform_type["code"]["init_code"]["code"]
        
        # Process init_code
        pattern = re.compile(r'%%\?[A-Za-z_]+%%')
        for result in re.findall(pattern, new_init_code):
            field = result[3: -2]
            if field == "init_code":
                new_init_code = new_init_code.replace(result, new_code['init_code'])
            if field == "dsp_code":
                new_init_code = new_init_code.replace(result, new_code['dsp_code'])
                
        # Process dsp code        
        new_dsp_code = self._template_code["dsp_code"]["code"];
        pattern = re.compile(r'%%:[A-Za-z]+%%')
        for result in re.findall(pattern, new_dsp_code):
            field = result[3: -2]
            if field == "name":
                new_dsp_code = new_dsp_code.replace(result, self.name)  
            else:
                print("unsupported template field :" + result)
        out_code = {}
        out_code["init_code"] = new_init_code
        out_code["dsp_code"] = new_dsp_code
        out_code['setup_code'] = ''
        return out_code

class ModuleAtom(Atom):
    def __init__(self, module, platform_code, token_index, platform):
        self.name = module["name"]
        self.handle = self.name + '_%03i'%token_index;
        self.out_tokens = [self.name + '_%03i_out'%token_index]
        self.current_scope = module["internalBlocks"]
        self._platform_code = platform_code
        self._input_block = None
        self._output_block = None
        self._index = token_index
        self.platform = platform
        self.stride_type = self.platform.find_stride_type("module")
        self.module = module
        self.rate = -1 # Should modules have rates?
        
        
        self._init_blocks(module["internalBlocks"],
                          module["input"], module["output"])
                          
        self.set_inline(False)
                         
            
    def set_inline(self, inline):
        if inline:
            self.out_tokens = []
        else:
            self.out_tokens = [self.name + '_%03i_out'%self._index]
        self._process_module(self.module["streams"], self.module["internalBlocks"])
        self.inline = inline
        
    def get_declarations(self):
        declarations_code = self._get_internal_declarations_code()
        
        instantiation_code = self._get_internal_instantiation_code()
        init_code = self._get_internal_init_code()
        process_code = self._get_internal_processing_code()
        properties_code = self._get_internal_properties_code()
        
        if self._input_block:
            # TODO this needs to be generalized for other types apart from float
            input_declaration = 'float %s'%self._input_block['name']
        else:
            input_declaration = ''
            
        declaration = templates.module_declaration(
                self.name, declarations_code + instantiation_code + properties_code, 
                init_code, input_declaration, process_code)
                    
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
        code = templates.assignment(self.out_tokens[0],self.get_inline_processing_code(in_tokens))
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
        self.code = self.platform.generate_code(tree, [],
                                                [self._input_block['name'] if self._input_block else None])
        self.platform.pop_scope()

        
    def _init_blocks(self, blocks, input_name, output_name):
        self._blocks = []
        for block in blocks:
            self._blocks.append(block['block'])
            if 'name' in input_name and self._blocks[-1]['name'] == input_name["name"]['name']:
                self._input_block = self._blocks[-1]
            if 'name' in output_name and  self._blocks[-1]['name'] == output_name["name"]["name"]:
                self._output_block = self._blocks[-1]
                
    def find_internal_block(self, block_name):
        for block in self._blocks:
            if block['name'] == block_name:
                return block

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
        
        if self._input_block:
            # TODO this needs to be generalized for other types apart from float
            input_declaration = 'float %s'%self._input_block['name']
        else:
            input_declaration = ''
                    
        declaration = 'struct %s {\n %s %s() {\n%s}\nfloat process(%s) \n{%s\n}\n};'%(
                self.name, declarations_code + instantiation_code, 
                self.name, init_code, input_declaration, process_code)
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
        self.code = self.platform.generate_code(tree, [],
                                                [self._input_block['name'] if self._input_block else None])
        self.platform.pop_scope()
     
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

        self._platform_funcs = json.load(open(platform_dir + '/functions.json'))['functions']
        self._platform_types = json.load(open(platform_dir + '/types.json'))['types']
        
        self.defined_modules =[]
    
        self.tree = tree
        self.scope_stack = []
        
        self.sample_rate = 44100 # FIXME: This is a hack. This should be brought in from the stream's domain and rate
    
    def find_platform_function(self, name):
        for func in self._platform_funcs:
            if func['functionName'] == name:
                return func
    
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
                if element['block']['type'] == 'type':
                    if element['block']['typeName'] == type_name:
                        return element
    
    def find_platform_type(self, type_name):
        found_type = None
        for platform_type in self._platform_types:
            if platform_type['typeName'] == type_name:
                found_type = platform_type
                break
        if not found_type:
            raise ValueError("Type " + type_name + " not found.")
        return platform_type
    
    def find_block(self, name, tree):
        block_declaration = self.find_declaration_in_tree(name, tree)
        platform_type = self.find_stride_type(block_declaration["type"])
        if not platform_type:
            platform_type = self.find_platform_type(block_declaration["type"])
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
            rate = member['name']["rate"]
            platform_type, declaration = self.find_block(member['name']['name'], self.tree)
            new_atom = NameAtom(platform_type, declaration, self.unique_id)
        elif "bundle" in member:
            rate = member['bundle']["rate"]
            platform_type, declaration = self.find_block(member['bundle']['name'], self.tree)
            new_atom = BundleAtom(platform_type, declaration, member['bundle']['index'], self.unique_id)
        elif "function" in member:
            rate = member['function']["rate"]
            module = self.find_platform_function(member['function']["name"])
            if module:
                new_atom = PlatformModuleAtom(platform_type, declaration, self.unique_id)
            else:
                module = self.find_declaration_in_tree(member['function']["name"], self.tree)
                if module['type'] == 'module':
                    platform_type = self.find_platform_function(member['function']["name"])
                    new_atom = ModuleAtom(module, platform_type, self.unique_id, self)
                elif module['type'] == 'reaction':
                    new_atom = ReactionAtom(module, self.unique_id, self)
        elif "expression" in member:
            rate = member['expression']["rate"]
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
            # TODO Check rates and rate changes
            new_atom = self.make_atom(member)             
            cur_group.append(new_atom)

            self.unique_id += 1
        return node_groups
        
    def generate_code_from_groups(self, node_groups, declared, instanced, initialized):
        declare_code = ''
        instantiation_code = ''
        init_code = ''
        processing_code = '' 
        parent_rates_size = templates.rate_stack_size() # To know now much we need to pop for this stream
        
        for group in node_groups:
            in_tokens = []
            previous_atom = None
            for atom in group:
                #Process declaration code
                declares = atom.get_declarations()
                for dec in declares:
                    if not dec in declared:
                        declare_code += declares[dec] + '\n'
                        declared.append(dec)
                # Process instantiation code
                instances = atom.get_instances()
                for inst in instances:
                    if not inst['handle'] in instanced:
                        instantiation_code += templates.instantiation_code(inst)
                        instanced.append(inst['handle'])
                        if 'code' in inst:
                            init_code +=  templates.initialization_code(inst)
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
              
        # Close pending rates in this stream
        while not parent_rates_size == templates.rate_stack_size():
            processing_code += templates.rate_end_code()

        return [declare_code, instantiation_code, init_code, processing_code]
        
    def generate_stream_code(self, stream, stream_index, declared, instanced, initialized):
        #out_var_name = "stream_%02i"%(stream_index)
        node_groups = self.make_stream_nodes(stream)
        
        processing_code = templates.stream_begin_code%stream_index
#        templates.rate_start(self.sample_rate)
#        processing_code += templates.rate_start_code()
        
        declare_code, instantiation_code, init_code, new_processing_code = self.generate_code_from_groups(node_groups, declared, instanced, initialized)
        processing_code += new_processing_code
        
#        processing_code += templates.rate_end_code()
            
        processing_code += templates.stream_end_code%stream_index
        
        # TODO return global code
        return {"declare_code" : declare_code,
                "instantiation_code" : instantiation_code,
                "init_code" : init_code,
                "processing_code" : processing_code}
                
    def generate_code(self, tree, declared = [], instanced = [], initialized = []):  
        self.unique_id = 0
        stream_index = 0
        includes_code = '' # TODO: How to handle globals like includes?
        declare_code = ''
        instantiation_code = ''
        init_code = ''
        processing_code = ''

        
        for node in tree:
            if 'stream' in node: # Everything grows from streams.
                code = self.generate_stream_code(node["stream"], stream_index, declared, instanced, initialized)
                declare_code += code["declare_code"]
                instantiation_code += code["instantiation_code"]
                init_code  += code["init_code"]
                processing_code += code["processing_code"]
                stream_index += 1
    
        
        return {"declare_code" : declare_code,
                "instantiation_code" : instantiation_code,
                "init_code" : init_code,
                "processing_code" : processing_code}
     
if __name__ == '__main__':
    pass