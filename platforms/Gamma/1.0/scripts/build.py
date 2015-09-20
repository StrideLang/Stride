# -*- coding: utf-8 -*-

from __future__ import print_function
from __future__ import division

import sys
import os
import shutil
import re
from subprocess import check_output as ck_out
import json

# --------------------- Common platform functions
class PlatformFunctions:
    def __init__(self, platform_dir):

        self._platform_funcs = json.load(open(platform_dir + '/functions.json'))['functions']
        #_platform_objs = json.load(open(platform_dir + '/objects.json'))['objects']
        self._platform_types = json.load(open(platform_dir + '/types.json'))['types']
    
    def find_function(self, name):
        for func in self._platform_funcs:
            if func['functionName'] == name:
                return func
    
    def find_definition_in_tree(self, block_name, tree):
        for node in tree:
            if 'block' in node:
                if node["block"]["name"] == block_name:
                    return node["block"]
            if 'blockbundle' in node:
                if node["blockbundle"]["name"] == block_name:
                    return node["blockbundle"]
        raise ValueError("Declaration not found for " + block_name)
        return None
    
    def find_platform_type(self, type_name):
        for platform_type in self._platform_types:
            if platform_type['typeName'] == type_name:
                platform_type = platform_type
                break
        if not platform_type:
            raise ValueError("Type " + type_name + " not found.")
        return platform_type
    
    def find_block(self, tree, name):
        block_declaration = self.find_definition_in_tree(name, tree)
        platform_type = self.find_platform_type(block_declaration["type"])
        return platform_type, block_declaration
    
    def find_port_value(self, object_name, port_name, tree):
        platform_type, block_declaration = self.find_block(tree, object_name)
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
        platform_type, declaration = self.find_block(tree, obj_name)
        if not declaration:
            raise ValueError("Declaration not found.")
        new_code = self.get_type_code(platform_type, var_name, bundle_index)
    
        ports = platform_type["ports"].copy()
    
        ports.update(declaration["ports"])
    
        new_code["init_code"] = self.put_port_values(new_code["init_code"], ports)
        new_code["dsp_code"] = self.put_port_values(new_code["dsp_code"], ports)
        return new_code
    
    def get_type_code(self, platform_type, var_name, bundle_index):
        code = {}
        new_global_code = '';
        new_dsp_code = '';
    
        new_global_code = platform_type["code"]["init_code"]["code"]
        if "%%token%%" in new_global_code:
            new_global_code = new_global_code.replace("%%token%%", var_name)
            
        if "%%intoken%%" in new_global_code:
            new_global_code = new_global_code.replace("%%intoken%%", var_name)
        if bundle_index >= 0:
            new_global_code = new_global_code.replace("%%bundle_index%%", str(bundle_index - 1))
    
        new_dsp_code = platform_type['code']['dsp_code']['code']
        if "%%token%%" in new_dsp_code:
            new_dsp_code = new_dsp_code.replace("%%token%%", var_name)
        if "%%intoken%%" in new_dsp_code:
            new_dsp_code = new_dsp_code.replace("%%intoken%%", var_name)
        if bundle_index > 0:
            new_dsp_code = new_dsp_code.replace("%%bundle_index%%", str(bundle_index - 1))
    
        code["dsp_code"] = new_dsp_code
        code["init_code"] = new_global_code
        if "includes" in platform_type["code"]:
            if len(platform_type["code"]["includes"]["code"]) > 0:
                code["includes"] = "#include <%s>"%platform_type["code"]["includes"]["code"]
    
        return code
    
    def get_function_property_config_code(self, func, prop_name, var_name, ugen_name):
        new_dsp_code_control = func["code"]["dsp_code"][prop_name]
        new_dsp_code_control = new_dsp_code_control.replace("%%token%%", var_name)
        new_dsp_code_control = new_dsp_code_control.replace("%%intoken%%", var_name)
        new_dsp_code_control = new_dsp_code_control.replace("%%identifier%%", ugen_name)
        return new_dsp_code_control
    
    def get_function_code(self, func, properties, var_name, ugen_name, intokens):
        code = {}
        new_global_code = '';
        new_dsp_code = '';

        new_global_code = func["code"]["init_code"]["code"]
        new_global_code = new_global_code.replace("%%intoken%%", var_name)
        new_global_code = new_global_code.replace("%%identifier%%", ugen_name)
    
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
                    new_dsp_code_control += self.get_function_property_config_code(func, prop_name, intoken, ugen_name)
    
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
    
        new_global_code = self.put_property_values(new_global_code, properties, func)
        new_dsp_code = self.put_property_values(new_dsp_code, properties, func)
    
        code["dsp_code"] = new_dsp_code
        code["init_code"] = new_global_code
        if "includes" in func["code"]:
            if len(func["code"]["includes"]["code"]) > 0:
                code["includes"] = "#include <%s>"%func["code"]["includes"]["code"]
        return code

# ---------------------
class Generator:
    def __init__(self, out_dir = '/home/andres/Documents/src/XMOS/Odo/StreamStack/platforms/Gamma/examples/filtering.stream_Products',
                 platform_dir = '/home/andres/Documents/src/XMOS/Odo/StreamStack/platforms/Gamma/1.0'):
        
        self.platform = PlatformFunctions(platform_dir)
    
        self.platform_dir = platform_dir
        self.out_dir = out_dir
        #out_dir = '/home/andres/Documents/src/XMOS/Odo/StreamStack/platforms/Gamma/1.0/test'
        #out_dir = sys.argv[1]
        self.project_dir = platform_dir + '/project'
        
        jsonfile = open(self.out_dir + '/tree.json')
        self.tree = json.load(jsonfile)
        
        self._intokens = {}
        self.last_num_outs = 0
        
        # TODO get gamma sources and build and install if not available
        # Question: building requires cmake, should binaries be distributed instead?
        # What about secondary deps like portaudio and libsndfile?
        self.log("Building Gamma project")
        self.log("Buiding in directory: " + self.out_dir)
            
        shutil.copyfile(self.project_dir + "/template.cpp", self.out_dir + "/main.cpp")
        
    def log(self, text):
        print(text)
    
    def write_section_in_file(self, sec_name, code):
        filename = self.out_dir + "/main.cpp"
        f = open(filename, 'r')
        text = f.read()
        f.close()
    #    log(text)
        start_index = text.find("//[[%s]]"%sec_name)
        end_index = text.find("//[[/%s]]"%sec_name, start_index)
        if start_index <0 or end_index < 0:
            raise ValueError("Error finding [[%s]]  section"%sec_name)
            return
        text = text[:start_index] + '//[[%s]]\n'%sec_name + code + text[end_index:]
        f = open(filename, 'w')
        f.write(text)
        f.close()
        
    def set_configuration(self):
        self.block_size = 16
        self.sample_rate = 44100.
        self.num_out_chnls = 2
        self.num_in_chnls = 2
        for node in self.tree:
            if 'block' in node:
                if node['block']['type'] == 'config':
                    # FIXME set from config node
                    self.block_size = 16
                    self.sample_rate = 44100.
                    self.num_out_chnls = 2
                    self.num_in_chnls = 2
    
    def generate_code(self):
        # Generate code from tree
        includes_code = ''
        global_code = ''
        dsp_code = ''
        
        stream_index = 0;
        ugen_index = 0;
        includes_list = []
        
        config_template_code = '''
        AudioDevice adevi = AudioDevice::defaultInput();
        AudioDevice adevo = AudioDevice::defaultOutput();
        //AudioDevice adevi = AudioDevice("firewire_pcm");
        //AudioDevice adevo = AudioDevice("firewire_pcm");
        
        //int maxOChans = adevo.channelsOutMax();
        //int maxIChans = adevi.channelsOutMax();
        //printf("Max input channels:  %d\\n", maxIChans);
        //printf("Max output channels: %d\\n", maxOChans);
        
        // To open the maximum number of channels, we can hand in the queried values...
        //AudioIO io(256, 44100., audioCB, NULL, maxOChans, maxIChans);
        
        // ... or just use -1
        
        AudioIO io(%%block_size%%, %%sample_rate%%, audioCB, NULL, %%num_out_chnls%%, %%num_in_chnls%%);
        '''
        
        # FIXME fix nomenclature
        config_code = ''
        dsp_code = ''
        global_code = ''
        setup_code = ''
        
        _rates = []
        _rated_ugens = {}
        used_signals = set()
        
        self.set_configuration()
        block_size = 16
        sample_rate = 44100.
        num_out_chnls = 2
        num_in_chnls = 2
    
        for node in self.tree:
            if 'block' in node:
                if node['block']['type'] == 'config':
                    block_size = 256
                    sample_rate = 44100.
                    num_out_chnls = 2
                    num_in_chnls = 2
        
            if 'stream' in node:
                previous_rate = -1
                dsp_code += '// Starting stream %02i -------------------------\n'%stream_index
                out_var_name = "stream_%02i"%(stream_index)
                for parts in node['stream']:
                    # Check rates and rate changes
                    rate = parts["rate"]
                    if not rate in _rates:
                        _rates.append(rate) # Previously unused rate
                    rate_index = _rates.index(rate)
                    if previous_rate == -1: # Unresolved rate
                        previous_rate = rate
                        if not rate == self.sample_rate:
                            dsp_code += 'if (counter_%02i_trig == 1)\n'%rate_index
                            out_var_name += '_rate_%02i'%_rates.index(rate)
                        dsp_code += '{// Starting rate: %i\n'%(rate)
        #                intoken = out_var_name
                    if not rate == previous_rate:
                        #print("Rate changed from %f to %f"%(previous_rate, rate))
                        out_var_name = "stream_%02i"%(stream_index)
                        dsp_code += '\n}  // Close %i \n %s_00 = stream_%02i_rate_%02i_00;\n'%(previous_rate, out_var_name, stream_index, _rates.index(previous_rate))
                        if not rate == self.sample_rate:
                            dsp_code += 'if (counter_%02i_trig == 1)\n'%rate_index
                            out_var_name += '_rate_%02i'%_rates.index(rate)
                        dsp_code += '{ // New Rate %i\n'%rate;

                    # To connect components within the same rate, the input token must be the internal rate token
        #            if rate == previous_rate:
        #                intoken = out_var_name
        #            else:
        #                intoken = "stream_%02i"%(stream_index)
                    #print("%s - %i %i"%(parts["name"], rate_index, rate))
                    previous_rate = rate
                    
        
                    # Now insert generated code
                    if parts['type'] == 'Bundle':
                        in_var_name = out_var_name + '_00'
                        used_signals.add(in_var_name)
                        new_code = self.platform.get_obj_code(self.tree, parts["name"], in_var_name, parts["index"])
                        dsp_code += new_code["dsp_code"]
                        global_code += new_code["init_code"]
                        self.last_num_outs = 1
                    elif parts['type'] == 'Name':
                        platform_type, declaration = self.platform.find_block(self.tree, parts["name"])
                        token_name = None
                        if declaration["type"] == "signal" or declaration["type"] == "control":
                            token_name = 'ctl_%s'%parts["name"]
                            if not parts["name"] in self._intokens:
                                global_code += "double %s;\n"%token_name;
                                self._intokens[parts["name"]] = [token_name]
                            else:
                                self._intokens[parts["name"]].append(out_var_name)
                            #intoken = out_var_name
        
                        if not "size" in declaration or declaration["size"] == 1:
                            in_var_name = out_var_name + '_00'
                            used_signals.add(in_var_name)
                            new_code = self.platform.get_obj_code(self.tree, parts["name"], in_var_name)
                            global_code += new_code["init_code"]
                            dsp_code += new_code["dsp_code"]
                            if token_name:
                                dsp_code += "%s = %s;\n"%(token_name, in_var_name)
                            self.last_num_outs = 1
                        else:
                            out_counter = 0
                            for i in range(declaration["size"]):
                                in_var_name = out_var_name + '_%02i'%out_counter
                                out_counter += 1
                                if out_counter == self.last_num_outs:
                                    out_counter = 0
                                used_signals.add(in_var_name)
        #                        _intokens[parts["name"]] = [out_var_name]
                                new_code = self.platform.get_obj_code(self.tree, parts["name"], in_var_name, i + 1)
                                global_code += new_code["init_code"]
                                dsp_code += new_code["dsp_code"]
                                #dsp_code += "%s = %s;\n"%(token_name, out_var_name)
                            self.last_num_outs = declaration["size"]
        
                    elif parts['type'] == 'Function':
                        func = self.platform.find_function(parts["name"])
                        if not func:
                            raise ValueError("Function not found")
                        ugen_name = "ugen_%02i"%(ugen_index)
        
                        new_code = self.platform.get_function_code(func, parts["ports"], out_var_name, ugen_name, self._intokens)
        
                        dsp_code += new_code["dsp_code"]
                        global_code += new_code["init_code"]
                        if "includes" in new_code:
                            includes_list.append(new_code["includes"])
                        ugen_index += 1
                        if not rate == sample_rate:
                            _rated_ugens[ugen_name] = rate_index
                        properties = parts["ports"]
                        for prop in properties:
                            prop_value = properties[prop]
                            prop_type = prop_value["type"]
                            if prop_type == "Value":
                                if prop in func["code"]["dsp_code"]:
                                    setup_code += self.platform.get_function_property_config_code(func, prop, str(prop_value["value"]), ugen_name)
                            if prop_type == "Name":
                                pass
                                # FIXME set name token
                                #if prop in func["code"]["dsp_code"]:
                                #    setup_code += self.platform.get_function_property_config_code(func, prop, str(prop_value["value"]), ugen_name)
                        self.last_num_outs = func["num_outputs"]
                dsp_code += '} // Stream End \n'
                stream_index += 1
        
        #var_declaration = ''.join(['double stream_%02i;\n'%i for i in range(stream_index)])
        #global_code = var_declaration + global_code
        
        config_code = config_template_code
        config_code = config_code.replace("%%block_size%%", str(block_size))
        config_code = config_code.replace("%%sample_rate%%", str(sample_rate))
        config_code = config_code.replace("%%num_out_chnls%%", str(num_out_chnls))
        config_code = config_code.replace("%%num_in_chnls%%", str(num_in_chnls))
        
        domain_code = ''
        domain_config_code = ''
        rate_config_code = ''
        rate_counter_inc = ''
        
        for i, rate in enumerate(_rates):
            if not rate == sample_rate: 
                domain_code += "Domain rate%02i(%f);\n"%(i, rate)
                domain_code += 'double counter_%02i;\nint counter_%02i_trig;\n'%(i,i)
                domain_config_code += 'counter_%02i = 0;\ncounter_%02i_trig = 0;\n'%(i, i)
                counter_inc = rate/sample_rate # Float division
                rate_counter_inc += 'counter_%02i_trig = 0;\ncounter_%02i += %.24f;\nif (counter_%02i >= 1.0) { counter_%02i -= 1.0; counter_%02i_trig = 1;}\n'%(i, i, counter_inc, i,i, i)
        
        interm_sig_code = ''
        for name in used_signals:
            interm_sig_code += 'double %s;\n'%name
        
        for rated_ugen in _rated_ugens:
            domain_config_code += "%s.domain(rate%02i); // Rate %.2f\n"%(rated_ugen, _rated_ugens[rated_ugen], _rates[_rated_ugens[rated_ugen]])
        
        
        global_code = interm_sig_code + domain_code + global_code
        config_code = rate_config_code + domain_config_code + setup_code + config_code
        dsp_code += rate_counter_inc
        
        includes_code = '\n'.join(set(includes_list)) + '\n'
        self.write_section_in_file('Includes', includes_code)
        self.write_section_in_file('Init Code', global_code)
        self.write_section_in_file('Dsp Code', dsp_code)
        self.write_section_in_file('Config Code', config_code)

# Compile --------------------------
    def compile(self):
        import platform
        
        if platform.system() == "Windows":
            cpp_compiler = "/usr/bin/c++"
        
            flags = "-I"+ self.platform_dir +"/include -O3 -DNDEBUG -o " \
                + self.out_dir +"/main.cpp.o -c "+ self.out_dir +"/main.cpp"
            args = [cpp_compiler] + flags.split()
            try:
                outtext = ck_out(args)
            except:
                pass
        
            self.log(outtext)
        
            # Link ------------------------
            flags = "-O3 -DNDEBUG "+ self.out_dir + "/main.cpp.o -o " \
                + self.out_dir +"/app -rdynamic -L " \
                + self.platform_dir + "/lib -lGamma -lpthread -lportaudio -lsndfile -lpthread -lportaudio -lsndfile"
            args = [cpp_compiler] + flags.split()
            try:
                outtext = ck_out(args)
            except:
                pass
        
            self.log(outtext)
            self.log("Done.")
        
        
        elif platform.system() == "Linux":
            cpp_compiler = "/usr/bin/c++"
        
            flags = "-I"+ self.platform_dir +"/include -O3 -DNDEBUG -o" \
                + self.out_dir +"/main.cpp.o -c "+ self.out_dir + "/main.cpp"
            args = [cpp_compiler] + flags.split()
            
            #self.log(' '.join(args))
            #os.system(' '.join(args))
            outtext = ck_out(args)
        
            self.log(outtext)
        
            # Link ------------------------
            flags = "-O3 -DNDEBUG "+ self.out_dir +"/main.cpp.o -o"+ self.out_dir +"/app -rdynamic -L" \
                + self.platform_dir + "/lib -lGamma -lpthread -lportaudio -lsndfile -lpthread -lportaudio -lsndfile"
            args = [cpp_compiler] + flags.split()
            
            #self.log(' '.join(args))
            outtext = ck_out(args)
        
            self.log(outtext)
            self.log("Done.")
        
        elif platform.system() == "Darwin":
            cpp_compiler = "/usr/bin/c++"
        
            flags = "-I"+ self.platform_dir +"/include -O3 -DNDEBUG -o " \
                + self.out_dir +"/main.cpp.o -c "+ self.out_dir +"/main.cpp"
            args = [cpp_compiler] + flags.split()
            try:
                outtext = ck_out(args)
            except:
                pass
        
            self.log(outtext)
        
            # Link ------------------------
            flags = "-O3 -DNDEBUG "+ self.out_dir +"/main.cpp.o -o "+ self.out_dir +"/app -rdynamic -L " \
                + self.platform_dir + "/lib -lGamma -lpthread -lportaudio -lsndfile -lpthread -lportaudio -lsndfile"
            args = [cpp_compiler] + flags.split()
            try:
                outtext = ck_out(args)
            except:
                pass
        
            self.log(outtext)
            self.log("Done.")
        else:
            self.log("Platform '%s' not supported!"%platform.system())

   
if __name__ == '__main__':
    if len(sys.argv) < 2:
        gen = Generator('/home/andres/Documents/src/XMOS/Odo/StreamStack/platforms/Gamma/examples/rates.stream_Products')
    else:
        gen = Generator(sys.argv[1])
    gen.generate_code()
    gen.compile()
