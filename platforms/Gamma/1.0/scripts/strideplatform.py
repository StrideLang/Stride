# -*- coding: utf-8 -*-
"""
Created on Sat Apr  9 11:16:55 2016

@author: andres
"""

from __future__ import print_function
from __future__ import division

import re
import json   
    
class Atom:
    def __init__(self, index):
        self.index = 0
        self.rate = -1
        self.inline = False
        
    def get_handle(self):
        if self.inline:
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
    
    def get_init_list(self):
        return None
        
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
        if self.inline:
            return self.get_inline_processing_code([])
        else:
            return self.handle
            
    def get_out_tokens(self):
        if self.inline:
            return [str(self.value)]
        else:
            return [self.handle]
    
    def get_declarations(self):
        return {}
    
    def get_instances(self):
        if self.inline:
            return []
        else:
            return [{'type' : 'real',
                 'handle' : self.handle}]
    
    def get_init_list(self):
        if self.inline:
            inits = []
        else:
            inits = [{'handle' : self.handle,
                      'code' : self.get_inline_processing_code([])
                      }]
        return inits
        
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
        self.inline = False
        self.rate = -1
        
        self._process_atoms()
        
    def get_handle(self):
        if self.inline:
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
        if not self.inline:
            instances.append({'handle' : self.handle,
                              'type' : 'real'})
        return instances
    
    def get_init_list(self):
        inits = self.left_atom.get_init_list()
        if self.right_atom:
            inits += self.right_atom.get_init_list()
        return inits
        
    def get_inline_processing_code(self, in_tokens):
        code = '(' + self.left_atom.get_out_tokens()[0]
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
        code += self.right_atom.get_out_tokens()[0] + ')'
        return code
    
    def get_processing_code(self, in_tokens):
        if self.inline:
            code = ''
            out_tokens = [self.get_inline_processing_code(in_tokens)]
        else:
            code = ''
            code += self.left_atom.get_processing_code([])[0]
            code += self.right_atom.get_processing_code([])[0]
            code += self.handle + ' = (' + self.get_inline_processing_code(in_tokens) + ');\n'
        
            out_tokens = [self.handle]
        return code, out_tokens
        
    def _process_atoms(self):
        
#        tree = streams + blocks
        # We need to pass the name of the input block because we will handle declaration and init
#        self.code = self.scoped_platform.generate_code(tree, [], [self._input_block['name']])
        return
        

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
        return [{'type' : 'real',
                 'handle' : self.handle}]
    
    def get_init_list(self):
        if 'default' in self.declaration:
            default_value = self.declaration['default']
        else:
            if 'default' in self.platform_type['block']:
                default_value = self.platform_type['block']['default'] # FIXME inheritance is not being handled here
            else:
                default_value = 0.0
        inits = [{'handle' : self.handle,
                  'code' : str(default_value)
                  }]
        return inits
        
    def get_inline_processing_code(self, in_tokens):
        return in_tokens[0]
    
    def get_processing_code(self, in_tokens):
        code = ''
        if len(in_tokens) > 0:
            code = self.handle + ' = ' + self.get_inline_processing_code(in_tokens) + ';\n'
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
        instances = [{'type' : 'bundle',
                      'bundletype': 'real',
                      'handle' : self.handle,
                      'size' : self.declaration['size']}]
        return instances
        
    def get_init_list(self):
        if 'default' in self.declaration:
            default_value = self.declaration['default']

        if 'blockbundle' in self.platform_type:
            #Stride definition
            if 'default' in self.platform_type['blockbundle']:
                default_value = self.platform_type['blockbundle']['default'] # FIXME inheritance is not being handled here
            else:
                default_value = 0.0
            inits = [{'handle' : self.handle,
                      'code' : str(default_value)
                      }]
        else:
#            # Hardware platform definition
#            code = self.platform_type['code']['init_code']['code']
#            code = code.replace('%%token%%', self._get_token_name(self.index))            
#            code = code.replace('%%bundle_index%%', str(self.index))
#            
#            inits = [{'handle' : self.handle,
#                      'code' : code
#                      }]
#                      
            inits = []
                
        return inits
    
    def get_inline_processing_code(self, in_tokens):
        code = in_tokens[0] 
        return code
        
    def get_processing_code(self, in_tokens):
        code = ''
        if len(in_tokens) > 0:
            code = self._get_token_name(self.index) + ' = ' + self.get_inline_processing_code(in_tokens) + ';\n'
        if 'code' in self.platform_type:
            # Hardware platform code. This needs to be improved...
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
    
    def get_init_code(self):
        if 'default' in self.declaration:
            default_value = self.declaration
        else:
            if 'default' in self.platform_type['block']:
                default_value = self.platform_type['block']['default'] # FIXME inheritance is not being handled here
            else:
                default_value = 0.0
        code = self.handle + ' = ' + str(default_value) + ';\n'
        return code
    
    def get_processing_code(self, in_tokens):
        code = ''
        if len(in_tokens) > 0:
            code = self.handle + ' = ' + in_tokens[0] + ';\n'
        out_tokens = [self.handle]
        return code, out_tokens
        
    def get_generated_code(self):
        
        stream_index = 0
        declared = []
        instanced = []
        for i, stream in enumerate(self._streams.itervalues()):
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
        self.out_tokens = [self.name + '_out_%03i'%token_index]
        self._streams = module["streams"]
        self._properties = module["properties"]
        self.current_scope = module["internalBlocks"]
        self._platform_code = platform_code
        self._input_block = None
        self._output_block = None
        self._index = token_index
        self.platform = platform
        self.stride_type = self.platform.find_stride_type("module")
        self.rate = -1 # Should modules have rates?
        
        self.inline = False
        
        self._init_blocks(module["internalBlocks"],
                          module["input"], module["output"])
                          
        self._process_module(module["streams"], module["internalBlocks"])
            
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
                    
        declaration = 'struct %s {\n %s %s() {\n%s}\nfloat process(%s) \n{%s\n}\n};'%(
                self.name, declarations_code + instantiation_code + properties_code, 
                self.name, init_code, input_declaration, process_code)
        return {self.name :  declaration}
    
    def get_instances(self):
        return [{'type' : 'module',
                 'handle': self.handle,
                 'moduletype' : self.name},
                 { 'type' : 'real',
                   'handle' : self.out_tokens[0]
                 }]
    
    def get_init_list(self):
        return []
        
    def get_inline_processing_code(self, in_tokens):
        if len(in_tokens) > 0:
            code = self.handle + '.process(' + in_tokens[0] + ')'
        else:
            code = self.handle + '.process()'
        return code
        
    def get_processing_code(self, in_tokens):
        code = ''
        code = self.out_tokens[0] + ' = ' + self.get_inline_processing_code(in_tokens) +  ';\n'
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
        if self._output_block:
            code += 'return %s;\n'%(self._output_block['name']) 
        return code
        
    def _get_internal_properties_code(self):
        code = ''
        for prop in self._properties:
            if 'block' in prop:
                code += 'void set_' + prop['block']['name'] + '(float amp) {\n'
                code += prop['block']['block']['name']['name'] + '= amp;\n'
                code += '\n}\n'
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
    
    def get_init_list(self):
        return []
        
    def get_inline_processing_code(self, in_tokens):
        code = self.handle + '.process(' + in_tokens[0] + ')'
        return code
        
    def get_processing_code(self, in_tokens):
        code = ''
        if len(in_tokens) > 0:
            code = self.out_tokens[0] + ' = ' + self.get_inline_processing_code(in_tokens) +  ';\n'
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
        if self._output_block:
            code += 'return %s;\n'%(self._output_block['name']) 
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
        self.rate_stack = []
        
        self.stream_begin_code = '// Starting stream %02i -------------------------\n{\n'
        self.stream_end_code = '} // Stream End %02i\n'
        self.rate_begin_code = '{ // Start new rate %i\n' 
        self.rate_end_code = '\n}  // Close Rate %i\n' 
        
        self.sample_rate = 44100 # FIXME: This is a hack. This should be brought in from the stream's domain and rate
    
    def find_platform_function(self, name):
        for func in self._platform_funcs:
            if func['functionName'] == name:
                return func
    
    def push_scope(self, scope):
        self.scope_stack.append(scope)
    
    def pop_scope(self):
        self.scope_stack.pop()
    
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
    
    #def find_builtin_object(platform_objs, name):
    #    for obj in platform_objs:
    #        if obj['objectName'] == name:
    #            return obj
    
    def bool_to_str(self, bool_val):
        if not type(bool_val) == bool:
            raise ValueError("Only boolean values accepted.")
        elif bool_val:
            return "true"
        else:
            return "false"

    # ------------------------- Text processing functions
        
    def put_property_values(self, template_code, properties, func):
        final_code = template_code
    
        p = re.compile(r"%%property:[a-zA-Z]+%%")
        for match in p.findall(final_code):
            prop_name = match[match.rindex(':') + 1: -2]
    
            if prop_name in properties:
                prop_value = properties[prop_name]
                prop_type = prop_value["type"]
                if prop_type == "Value":
                    if type(prop_value["value"]) == bool:
                        value = self.bool_to_str(prop_value["value"])
                    else:
                        value = str(prop_value["value"])
                elif prop_type == "Name":
                    default_value = self.find_function_property(func, prop_name)["default"]
                    if type(default_value) == bool:
                        value = self.bool_to_str(default_value)
                    else:
                        value = str(default_value)
            else:
                default_value = self.find_function_property(func, prop_name)["default"]
                if type(default_value) == bool:
                    value = self.bool_to_str(default_value)
                else:
                    value = str(default_value)
    #        print(match + '     ' + value)
            final_code = final_code.replace(match, value)
        return final_code
    
    def put_port_values(self, template_code, ports):
        final_code = template_code
    
        p = re.compile(r"%%port:[a-zA-Z]+%%")
        for match in p.findall(final_code):
            port_name = match[match.rindex(':') + 1: -2]
    
            if port_name in ports:
                if type(ports[port_name]) == dict:
                    port_value = ports[port_name]['default']
                else:
                    port_value = ports[port_name]
                final_code = final_code.replace(match, str(port_value))
            else:
                raise ValueError("Property in code not matched")
        return final_code
    
    # Code generators
    def get_obj_code(self, tree, obj_name, var_name, bundle_index = 1):
        platform_type, declaration = self.find_block(obj_name, tree)
        if not declaration:
            raise ValueError("Declaration not found.")
        new_code = self.get_type_code(platform_type, var_name, bundle_index)
    
        ports = platform_type["ports"].copy()
    
        if "ports" in declaration:
            ports.update(declaration["ports"])
    
        new_code["init_code"] = self.put_port_values(new_code["init_code"], ports)
        new_code["dsp_code"] = self.put_port_values(new_code["dsp_code"], ports)
        new_code['setup_code'] = ''
        return new_code
    
    def get_type_code(self, platform_type, var_name, bundle_index):
        code = {}
        new_init_code = '';
        new_dsp_code = '';
    
        new_init_code = platform_type["code"]["init_code"]["code"]
        if "%%token%%" in new_init_code:
            new_init_code = new_init_code.replace("%%token%%", var_name)
            
        if "%%intoken%%" in new_init_code:
            new_init_code = new_init_code.replace("%%intoken%%", var_name)
        if bundle_index >= 0:
            new_init_code = new_init_code.replace("%%bundle_index%%", str(bundle_index - 1))
    
        new_dsp_code = platform_type['code']['dsp_code']['code']
        if "%%token%%" in new_dsp_code:
            new_dsp_code = new_dsp_code.replace("%%token%%", var_name)
        if "%%intoken%%" in new_dsp_code:
            new_dsp_code = new_dsp_code.replace("%%intoken%%", var_name)
        if bundle_index > 0:
            new_dsp_code = new_dsp_code.replace("%%bundle_index%%", str(bundle_index - 1))
    
        code["dsp_code"] = new_dsp_code
        code["init_code"] = new_init_code
        code['setup_code'] = ''
        if "includes" in platform_type["code"]:
            if len(platform_type["code"]["includes"]["code"]) > 0:
                code["includes"] = "#include <%s>"%platform_type["code"]["includes"]["code"]
    
        return code       
    
    def get_function_property_setup_code(self, func, prop_name, var_name, ugen_name):
        new_dsp_code_control = func["code"]["dsp_code"][prop_name]
        new_dsp_code_control = new_dsp_code_control.replace("%%token%%", var_name)
        new_dsp_code_control = new_dsp_code_control.replace("%%intoken%%", var_name)
        new_dsp_code_control = new_dsp_code_control.replace("%%identifier%%", ugen_name)
        return new_dsp_code_control
    
    def get_function_code(self, func, properties, var_name, ugen_name, intokens):
        code = {}
        new_init_code = '';
        new_dsp_code = '';

        new_init_code = func["code"]["init_code"]["code"]
        new_init_code = new_init_code.replace("%%intoken%%", var_name)
        new_init_code = new_init_code.replace("%%identifier%%", ugen_name)
    
        # Insert code for variable (non-constant) properties
        new_dsp_code_control = ''
        for prop_name in properties:
            prop_value = properties[prop_name]
            prop_type = prop_value["type"]
            if prop_type == "Name":
                # FIXME This should be inserted where the intoken changes, to make sure that it is updated at the right rate
                intoken = intokens[prop_value["name"]][-1] # FIXME: What to do with multiple inputs
                #print ("------------" + intoken)
                if prop_name in func["code"]["dsp_code"]:
                    new_dsp_code_control += self.get_function_property_setup_code(func, prop_name, intoken, ugen_name)
    
        new_dsp_code = new_dsp_code_control + func["code"]["dsp_code"]["code"]
    
        if func["num_inputs"] == 1:
            new_dsp_code = new_dsp_code.replace("%%intoken%%", var_name + '_00')
        else:
            for i in range(func["num_inputs"]):
                new_dsp_code = new_dsp_code.replace("%%%%intoken:%i%%%%"%(i + 1), var_name + '_%02i'%i)
        if func["num_outputs"] == 1:
            new_dsp_code = new_dsp_code.replace("%%token%%", var_name + '_00')
        else:
            for i in range(func["num_outputs"]):
                new_dsp_code = new_dsp_code.replace("%%%%token:%i%%%%"%(i + 1), var_name + '_%02i'%i)
        new_dsp_code = new_dsp_code.replace("%%identifier%%", ugen_name)
    
        new_init_code = self.put_property_values(new_init_code, properties, func)
        new_dsp_code = self.put_property_values(new_dsp_code, properties, func)
    
        code["dsp_code"] = new_dsp_code
        code["init_code"] = new_init_code
        code['setup_code'] = ''
        if "includes" in func["code"]:
            if len(func["code"]["includes"]["code"]) > 0:
                code["includes"] = "#include <%s>"%func["code"]["includes"]["code"]
        return code        
        
    def generate_bundle_code(self, bundle, out_var_name):
        in_var_name = out_var_name + '_00'
        self.used_signals.add(in_var_name)
        new_code = self.get_obj_code(self.tree, bundle["name"], in_var_name, bundle["index"])
        new_dsp_code = new_code["dsp_code"]
        new_init_code = new_code["init_code"]
        self.last_num_outs = 1
        return {"init_code" : new_init_code, "dsp_code" : new_dsp_code, 'setup_code' : ''}
    
    def generate_name_code(self, name, out_var_name):
        new_dsp_code = ''
        new_init_code = ''
        platform_type, declaration = self.find_block(name['name'], self.tree)
        token_name = None
        if declaration["type"] == "signal":
            token_name = 'ctl_%s'%name["name"]
            if not name["name"] in self._intokens:
                new_init_code = "double %s;\n"%token_name;
                self._intokens[name["name"]] = [token_name]
            else:
                self._intokens[name["name"]].append(out_var_name)
                
            if not "size" in declaration or declaration["size"] == 1:
                in_var_name = out_var_name + '_00'
                self.used_signals.add(in_var_name)
                new_code = self.get_obj_code(self.tree, name["name"], in_var_name)
                new_init_code += new_code["init_code"]
                new_dsp_code += new_code["dsp_code"]
                if token_name:
                    new_dsp_code += "%s = %s;\n"%(token_name, in_var_name)
                self.last_num_outs = 1
            else:
                out_counter = 0
                for i in range(declaration["size"]):
                    in_var_name = out_var_name + '_%02i'%out_counter
                    out_counter += 1
                    if out_counter == self.last_num_outs:
                        out_counter = 0
                    self.used_signals.add(in_var_name)
    #                        _intokens[parts["name"]] = [out_var_name]
                    new_code = self.get_obj_code(self.tree, name["name"], in_var_name, i + 1)
                    new_init_code += new_code["init_code"]
                    new_dsp_code += new_code["dsp_code"]
                    #new_dsp_code += "%s = %s;\n"%(token_name, out_var_name)
                self.last_num_outs = declaration["size"]                

        return {"init_code" : new_init_code, "dsp_code": new_dsp_code, 'setup_code' : ''}
            #intoken = out_var_name    
        
    def get_instantiation_code(self, instance):
        if instance['type'] == 'real':
            code = 'float ' + instance['handle'] + ';\n'
        elif instance['type'] =='bundle':
            if instance['bundletype'] == 'real':
                code = 'float ' + instance['handle'] + '[%i];\n'%instance['size']
        elif instance['type'] == 'module':
            code = instance['moduletype'] + ' ' + instance['handle'] + ';\n'
        elif instance['type'] == 'reaction':
            code = instance['reactiontype'] + ' ' + instance['handle'] + ';\n'
        else:
            raise ValueError('Unsupported type for instance')
        return code
        
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
        
    def push_rate(self, rate):
        print('push_rate %i'%rate)
        self.rate_stack.append(rate)
        
    def get_current_rate(self):
        if len(self.rate_stack) > 0:
            return self.rate_stack[-1]
        else:
            return None
    
    def pop_rate(self):
        print('pop_rate %i'%self.rate_stack[-1])
        if len(self.rate_stack) > 0:
            return self.rate_stack.pop()
        else:
            return None
        
    def make_stream_nodes(self, stream):
        self.unique_id = 0 # This uid should be further up!
        node_groups = [[]] # Nodes are grouped by rate
        
        for member in stream: #Only instantiate whatever is used in streams. Discard the rest
            cur_group = node_groups[-1]  
            # Check rates and rate changes
            new_atom = self.make_atom(member)
#            if not processor["name"] in self.defined_modules:
#                if "includes" in new_code:
#                    self.includes_list.append(new_code["includes"])
#                new_init_code += new_code["init_code"]
#                self.defined_modules.appen                  

            cur_group.append(new_atom)
            # To connect components within the same rate, the input token must be the internal rate token
#            if rate == previous_rate:
#                intoken = out_var_name
#            else:
#                intoken = "stream_%02i"%(stream_index)
            #print("%s - %i %i"%(member["name"], rate_index, rate))
            #previous_rate = rate
            self.unique_id += 1
        return node_groups
        
    def generate_code_from_groups(self, node_groups, declared, instanced, initialized):
        declare_code = ''
        instantiation_code = ''
        init_code = ''
        processing_code = ''
        
    
#            if not rate in self._rates:
#                self._rates.append(rate) # Previously unused rate
#            rate_index = self._rates.index(rate)
#            if previous_rate == -1: # Unresolved rate
#                previous_rate = rate
#                if not rate == self.sample_rate:
#                    new_dsp_code += 'if (counter_%02i_trig == 1)\n'%rate_index
#                    out_var_name += '_rate_%02i'%self._rates.index(rate)
#                new_dsp_code += stream_begin_code%(rate)
##                intoken = out_var_name
#            if not rate == previous_rate:
#                print("Rate changed from %f to %f"%(previous_rate, rate))
##                node_groups.append([])
##                cur_group = node_groups[-1]
#                out_var_name = "stream_%02i"%(stream_index)
#                new_dsp_code += '\n}  // Close %i \n %s_00 = stream_%02i_rate_%02i_00;\n'%(previous_rate, out_var_name, stream_index, self._rates.index(previous_rate))
#                if not rate == self.sample_rate:
#                    new_dsp_code += 'if (counter_%02i_trig == 1)\n'%rate_index
#                    out_var_name += '_rate_%02i'%self._rates.index(rate)
#                new_dsp_code += '{ // New Rate %i\n'%rate        
        counters = 0    
        
        parent_rates_size = len(self.rate_stack)
        
        
        for group in node_groups:
            in_tokens = []
            previous_atom = None
            for atom in group:
                #Process declaration code
                declares = atom.get_declarations()
                for dec in declares.iterkeys():
                    if not dec in declared:
                        declare_code += declares[dec] + '\n'
                        declared.append(dec)
                # Process instantiation code
                instances = atom.get_instances()
                for inst in instances:
                    if not inst['handle'] in instanced:
                        instantiation_code += self.get_instantiation_code(inst)
                        instanced.append(inst['handle'])
                        
                        init_list = atom.get_init_list()
                        for init_member in init_list:
                            if not init_member['handle'] in initialized:
                            # Process initialization code only if declared
                                init_code +=  init_member['handle'] + ' = ' + init_member['code'] + '\n'
                                initialized.append(init_member['handle'])
                # Process processing code
                code, out_tokens = atom.get_processing_code(in_tokens)
                if atom.rate > 0 and not atom.rate == self.get_current_rate():
                    old_rate = self.pop_rate()
                    if not old_rate == self.sample_rate:
                        counter_inc = old_rate/self.sample_rate # Float division
                        counter_number = len(self.rate_stack)
                        processing_code += '}\n'
                        processing_code += self.rate_end_code%old_rate
                        processing_code += 'counter_%02i += %.10f;\n'%(counter_number,counter_inc)
                    else:
                        processing_code += self.rate_end_code%old_rate
                        
                    self.push_rate(atom.rate)
                    processing_code += self.rate_begin_code%atom.rate
                    if previous_atom:
                        previous_atom.inline = False
                    if not atom.rate == self.sample_rate:
                        instantiation_code += 'float counter_%02i;\n'%(counters)
                        init_code +=  'counter_%02i = 1.0\n'%(counters)
                        processing_code += 'if (counter_%02i >= 1.0) {\ncounter_%02i -= 1.0;\n'%(counters, counters)

                if code:
                    processing_code += code + '\n'
                in_tokens = out_tokens
            
            previous_atom = atom
              
        while not parent_rates_size == len(self.rate_stack):
            old_rate = self.pop_rate()
            processing_code += self.rate_end_code%old_rate
            if not old_rate == self.sample_rate:
                counter_inc = old_rate/self.sample_rate # Float division
                counter_number = len(self.rate_stack)
                processing_code += 'counter_%02i += %.8f;\n'%(counter_number,counter_inc)

        return [declare_code, instantiation_code, init_code, processing_code]
        
    def generate_stream_code(self, stream, stream_index, declared, instanced, initialized):
        #out_var_name = "stream_%02i"%(stream_index)
        node_groups = self.make_stream_nodes(stream)

        processing_code = self.stream_begin_code%stream_index
        
        parent_rate = self.get_current_rate()
        if not parent_rate == self.sample_rate:
            self.push_rate(self.sample_rate)
            processing_code += self.rate_begin_code%self.sample_rate
        
        declare_code, instantiation_code, init_code, new_processing_code = self.generate_code_from_groups(node_groups, declared, instanced, initialized)
        processing_code += new_processing_code
        
        if not parent_rate == self.sample_rate:
            processing_code += self.rate_end_code%self.get_current_rate()
            self.pop_rate()
            
        processing_code += self.stream_end_code%stream_index
        
#        domain_code = ''
#        domain_setup_code = ''
#        rate_setup_code = ''
#        rate_counter_inc = ''
#        
#        for i, rate in enumerate(self._rates):
#            if not rate == self.sample_rate: # TODO don't call this domain as it now means something dofferent
#                domain_code += "Domain rate%02i(%f);\n"%(i, rate)
#                domain_code += 'double counter_%02i;\nint counter_%02i_trig;\n'%(i,i)
#                domain_setup_code += 'counter_%02i = 0;\ncounter_%02i_trig = 0;\n'%(i, i)
#                counter_inc = rate/self.sample_rate # Float division
#                rate_counter_inc += 'counter_%02i_trig = 0;\ncounter_%02i += %.24f;\nif (counter_%02i >= 1.0) { counter_%02i -= 1.0; counter_%02i_trig = 1;}\n'%(i, i, counter_inc, i,i, i)
#        
#        for rated_ugen in self._rated_ugens:
#            domain_setup_code += "%s.domain(rate%02i); // Rate %.2f\n"%(rated_ugen, self._rated_ugens[rated_ugen], self._rates[self._rated_ugens[rated_ugen]])
#        
#        declare_code += domain_code
#        init_code += rate_setup_code + domain_setup_code
#        processing_code += rate_counter_inc        
        
        # TODO return global code
        return {"declare_code" : declare_code,
                "instantiation_code" : instantiation_code,
                "init_code" : init_code,
                "processing_code" : processing_code}
                
    def generate_code(self, tree, declared = [], instanced = [], initialized = []):
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