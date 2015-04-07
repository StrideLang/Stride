# -*- coding: utf-8 -*-

from __future__ import print_function
from __future__ import division

import sys
import os
import shutil
import re
from subprocess import check_output as ck_out

def log(text):
    print(text)

gamma_directory = '/home/andres/Documents/src/Allostuff/Gamma'
platform_dir = '/home/andres/Documents/src/XMOS/Odo/StreamStack/platforms/Gamma/1.0'

project_dir = platform_dir + '/project'

# TODO get gamma sources and build and install if not available
# Question: building requires cmake, should binaries be distributed instead?
# What about secondary deps like portaudio and libsndfile?
log("Building Gamma project")

out_dir = sys.argv[1]
log("Buiding in directory: " + out_dir)

#shutil.rmtree(out_dir)
#shutil.copytree(gamma_directory, out_dir)

shutil.copyfile(project_dir + "/template.cpp", out_dir + "/main.cpp")

def write_section(sec_name, code):
    filename = out_dir + "/main.cpp"
    f = open(filename, 'r')
    text = f.read()
    f.close()
#    log(text)
    start_index = text.find("//[[%s]]"%sec_name)
    end_index = text.find("//[[/%s]]"%sec_name, start_index)
    if start_index <0 or end_index < 0:
        log("Error finding [[%s]]  section"%sec_name)
        log(str(start_index) + ' ' + str(end_index))
        return
    text = text[:start_index] + '//[[%s]]\n'%sec_name + code + text[end_index:]
    f = open(filename, 'w')
    f.write(text)
    f.close()

# Generate code from tree

# Load tree in json format
import json

jsonfile = open(out_dir + '/tree.json')
tree = json.load(jsonfile)

includes_code = ''
init_code = ''
dsp_code = ''

num_chnls = 2;

platform_funcs = json.load(open(platform_dir + '/functions.json'))['functions']
platform_objs = json.load(open(platform_dir + '/objects.json'))['objects']
platform_types = json.load(open(platform_dir + '/types.json'))['types']

# --------------------- Common platform functions

def find_function(platform_funcs, name):
    for func in platform_funcs:
        if func['functionName'] == name:
            return func

def find_definition_in_tree(block_name, tree):
    for node in tree:
        if 'block' in node:
            if node["block"]["name"] == block_name:
                return node["block"]
        if 'blockbundle' in node:
            if node["blockbundle"]["name"] == block_name:
                return node["blockbundle"]
    return None

def find_block(platform_types, name, tree):
    block_declaration = find_definition_in_tree(name, tree)
    if not block_declaration:
        print("Declaration not found for " + name)
    block_type = None
    for platform_type in platform_types:
#        print(str(block_declaration))
        if block_declaration and platform_type['typeName'] == block_declaration["type"]:
            block_type = platform_type
            break
    if not block_type:
        print("Declaration not found for " + name + ' of type ' + block_declaration["type"])
#        print(str(platform_types))
    return block_type, block_declaration

def find_function_property(func, property_name):
    return func["properties"][property_name]

def find_builtin_object(platform_objs, name):
    for obj in platform_objs:
        if obj['objectName'] == name:
            return obj

def bool_to_str(bool_val):
    if not type(bool_val) == bool:
        raise ValueError("Only boolean values accepted.")
    elif bool_val:
        return "true"
    else:
        return "false"

def put_property_values(template_code, properties, func):
    final_code = template_code

    p = re.compile(r"%%property:[a-zA-Z]+%%")
    for match in p.findall(final_code):
        prop_name = match[match.rindex(':') + 1: -2]

        if prop_name in properties:
            prop_value = properties[prop_name]
            if type(prop_value["value"]) == bool:
                value = bool_to_str(prop_value["value"])
            else:
                value = str(prop_value["value"])
        else:
            default_value = find_function_property(func, prop_name)["default"]
            if type(default_value) == bool:
                value = bool_to_str(default_value)
            else:
                value = str(default_value)
#        print(match + '     ' + value)
        final_code = final_code.replace(match, value)
    return final_code

def get_type_code(block_type, var_name, intoken, bundle_index = -1):
    code = {}
    new_init_code = '';
    new_dsp_code = '';

    print(block_type)
    new_init_code = block_type["code"]["init_code"]["code"]
    if "%%token%%" in new_init_code:
        new_init_code = new_init_code.replace("%%token%%", var_name)
    if "%%intoken%%" in new_init_code:
        new_init_code = new_init_code.replace("%%intoken%%", intoken)
    if bundle_index > 0:
        new_init_code = new_init_code.replace("%%bundle_index%%", str(bundle_index - 1))
#    if ugen_name:
#        new_init_code = new_init_code.replace("%%identifier%%", ugen_name)

    new_dsp_code = block_type['code']['dsp_code']['code']
    if "%%token%%" in new_dsp_code:
        new_dsp_code = new_dsp_code.replace("%%token%%", var_name)
    if "%%intoken%%" in new_dsp_code:
        new_dsp_code = new_dsp_code.replace("%%intoken%%", intoken)
    if bundle_index > 0:
        new_dsp_code = new_dsp_code.replace("%%bundle_index%%", str(bundle_index - 1))
#    if ugen_name:
#        new_dsp_code = new_dsp_code.replace("%%identifier%%", ugen_name)

    code["dsp_code"] = new_dsp_code
    code["init_code"] = new_init_code
    if "includes" in block_type["code"]:
        if len(block_type["code"]["includes"]["code"]) > 0:
            code["includes"] = "#include <%s>"%block_type["code"]["includes"]["code"]

    return code

def get_function_code(function_name, properties, token, intoken, ugen_name, func):
    code = {}
    new_init_code = '';
    new_dsp_code = '';

    obj = find_function(platform_funcs, function_name)
    if not obj:
        raise ValueError("Function not found.")

    new_init_code = obj["code"]["init_code"]["code"]
    new_init_code = new_init_code.replace("%%token%%", token)
    new_init_code = new_init_code.replace("%%intoken%%", intoken)
    new_init_code = new_init_code.replace("%%identifier%%", ugen_name)

    new_dsp_code = obj["code"]["dsp_code"]["code"]
    new_dsp_code = new_dsp_code.replace("%%token%%", token)
    new_dsp_code = new_dsp_code.replace("%%intoken%%", intoken)
    new_dsp_code = new_dsp_code.replace("%%identifier%%", ugen_name)

    new_init_code = put_property_values(new_init_code, properties, func)
    new_dsp_code = put_property_values(new_dsp_code, properties, func)


#    for prop_name, prop_value in properties.iteritems():
#        template = '%%property:' + prop_name + '%%'
#        if template in new_init_code:
#            if type(prop_value["value"]) == bool:
#                new_init_code = new_init_code.replace(template, bool_to_str(prop_value["value"]))
#            else:
#                new_init_code = new_init_code.replace(template, str(prop_value["value"]))
#        if template in new_dsp_code:
#            if type(prop_value["value"]) == bool:
#                new_dsp_code = new_dsp_code.replace(template, bool_to_str(prop_value["value"]))
#            else:
#                new_dsp_code = new_dsp_code.replace(template, str(prop_value["value"]))

    code["dsp_code"] = new_dsp_code
    code["init_code"] = new_init_code
    if "includes" in obj["code"]:
        if len(obj["code"]["includes"]["code"]) > 0:
            code["includes"] = "#include <%s>"%obj["code"]["includes"]["code"]
    return code

def get_obj_code(obj_name, var_name, intoken, bundle_index = -1):
    code = {}
    new_init_code = '';
    new_dsp_code = '';

    obj = find_builtin_object(platform_objs, obj_name)
    if not obj:
        obj = find_definition_in_tree(obj_name, tree)
        block_type, declaration = find_block(platform_types, obj_name, tree)
        if not declaration:
            raise ValueError("Declaration not found.")
        new_code = get_type_code(block_type, var_name, intoken)
        return new_code

    if not obj:
        raise ValueError("Object declaration not found.")

    block_type, declaration = find_block(platform_types, obj_name, tree)
    new_code = get_type_code(block_type, var_name, intoken)
    return new_code

# ---------------------


stream_index = 0;
ugen_index = 0;
includes_list = []

config_code = '''
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

for node in tree:
    if 'block' in node:
        if node['block']['type'] == 'config':
            block_size = 256
            sample_rate = 44100.
            num_out_chnls = 2
            num_in_chnls = 2


            config_code = config_code.replace("%%block_size%%", str(block_size))
            config_code = config_code.replace("%%sample_rate%%", str(sample_rate))
            config_code = config_code.replace("%%num_out_chnls%%", str(num_out_chnls))
            config_code = config_code.replace("%%num_in_chnls%%", str(num_in_chnls))

            write_section('Config Code', config_code)
    if 'stream' in node:
        for parts in node['stream']:
            var_name = "stream_%02i"%(stream_index)
            intoken = "stream_%02i"%(stream_index-1) if stream_index > 0 else ''

            if parts['type'] == 'Bundle':
                new_code = get_obj_code(parts["name"], var_name, intoken, parts["index"])
                dsp_code += new_code["dsp_code"]
                init_code += new_code["init_code"]
            elif parts['type'] == 'Name':
                new_code = get_obj_code(parts["name"], var_name, intoken)
                dsp_code += new_code["dsp_code"]
                init_code += new_code["init_code"]
            elif parts['type'] == 'Function':
                func = find_function(platform_funcs, parts["name"])
                if not func:
                    raise ValueError("Function not found")
                ugen_name = "ugen_%02i"%(ugen_index)
                new_code = get_function_code(parts['name'], parts["properties"], var_name, intoken, ugen_name, func)

                dsp_code += new_code["dsp_code"]
                init_code += new_code["init_code"]
                if "includes" in new_code:
                    includes_list.append(new_code["includes"])
                ugen_index += 1
        stream_index += 1

var_declaration = ''.join(['float stream_%02i;\n'%i for i in range(stream_index)])
dsp_code = var_declaration + dsp_code

includes_code = '\n'.join(set(includes_list))
write_section('Includes', includes_code)
write_section('Init Code', init_code)
write_section('Dsp Code', dsp_code)

# Compile --------------------------
cpp_compiler = "/usr/bin/c++"

flags = "-I"+ platform_dir +"/include -O3 -DNDEBUG -o "+ out_dir +"/main.cpp.o -c "+ out_dir +"/main.cpp"
args = [cpp_compiler] + flags.split()
outtext = ck_out(args)

log(outtext)

# Link ------------------------
flags = "-O3 -DNDEBUG "+ out_dir +"/main.cpp.o -o "+ out_dir +"/app -rdynamic -L " + platform_dir + "/lib -lGamma -lpthread -lportaudio -lsndfile -lpthread -lportaudio -lsndfile"
args = [cpp_compiler] + flags.split()
outtext = ck_out(args)

log(outtext)
