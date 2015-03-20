# -*- coding: utf-8 -*-

from __future__ import print_function
from __future__ import division

import sys
import os
import shutil
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

stream_index = 0;

init_code = ''
dsp_code = ''
num_chnls = 2;
platform_funcs = json.load(open(platform_dir + '/functions.json'))['functions']
platform_objs = json.load(open(platform_dir + '/objects.json'))['objects']
# --------------------- Common platform functions

def find_function(platform_funcs, name):
    for func in platform_funcs:
        if func['functionName'] == name:
            return func

def find_builtin_object(platform_objs, name):
    for obj in platform_objs:
        if obj['objectName'] == name:
            return obj

def get_function_code(function_name, properties, token):
    code = {}
    new_init_code = '';
    new_dsp_code = '';

    obj = find_function(platform_funcs, function_name)
    if not obj:
        raise ValueError("Function not found.")

    new_init_code = obj["code"]["init_code"]["code"]
    new_init_code = new_init_code.replace("%%token%%", var_name)

    new_dsp_code = obj["code"]["dsp_code"]["code"]
    new_dsp_code = new_dsp_code.replace("%%token%%", var_name)

    for prop_name, prop_value in properties.iteritems():
        template = '%%property:' + prop_name + '%%'
        if template in new_init_code:
            new_init_code = new_init_code.replace(template, str(prop_value["value"]))
        if template in new_dsp_code:
            new_dsp_code = new_dsp_code.replace(template, str(prop_value["value"]))

    code["dsp_code"] = new_dsp_code
    code["init_code"] = new_init_code
    return code

def get_builtin_obj_code(obj_name, var_name):
    code = {}
    new_init_code = '';
    new_dsp_code = '';

    obj = find_builtin_object(platform_objs, obj_name)
    if not obj:
        raise ValueError("Builtin onject not found.")

    new_init_code = obj["code"]["init_code"]["code"]
    new_init_code = new_init_code.replace("%%token%%", var_name)
    new_init_code = new_init_code.replace("%%bundle_index%%", str(parts["index"] - 1))

    new_dsp_code = obj["code"]["dsp_code"]["code"]
    new_dsp_code = new_dsp_code.replace("%%token%%", var_name)
    new_dsp_code = new_dsp_code.replace("%%bundle_index%%", str(parts["index"] - 1))

    code["dsp_code"] = new_dsp_code
    code["init_code"] = new_init_code
    return code

# ---------------------
for node in tree:
    if 'stream' in node:
        for parts in node['stream']:
            if parts['type'] == 'Bundle':
                var_name = "sig_%02i"%(stream_index)
                new_code = get_builtin_obj_code(parts["name"], var_name)
                dsp_code += new_code["dsp_code"]
                init_code += new_code["init_code"]
            elif parts['type'] == 'Name':
                pass
#                if parts['name'] == 'AudioIn':
#                    for chan_index in range(num_chnls):
#                        dsp_code += 'float sig_%02i_%02i = io.in(%i);\n'%(stream_index, chan_index, chan_index)
#                elif parts['name'] == 'AudioIn':
#                    for chan_index in range(num_chnls):
#                        dsp_code += 'io.out(%i) = sig_%02i_%02i;\n'%(chan_index, stream_index, chan_index)
            elif parts['type'] == 'Function':
                func = find_function(platform_funcs, parts["name"])
                if not func:
                    raise ValueError("Function not found")
                var_name = "sig_%02i"%(stream_index)

                new_code = get_function_code(parts['name'], parts["properties"], 'sig_%02i'%stream_index)

                dsp_code += new_code["dsp_code"]
                init_code += new_code["init_code"]
        stream_index += 1

#log(dsp_code)

write_section('Dsp Code', dsp_code)

cpp_compiler = "/usr/bin/c++"

flags = "-I"+ platform_dir +"/include -O3 -DNDEBUG -o "+ out_dir +"/main.cpp.o -c "+ out_dir +"/main.cpp"
args = [cpp_compiler] + flags.split()
outtext = ck_out(args)

log(outtext)
flags = "-O3 -DNDEBUG "+ out_dir +"/main.cpp.o -o "+ out_dir +"/app -rdynamic -L " + platform_dir + "/lib -lGamma -lpthread -lportaudio -lsndfile -lpthread -lportaudio -lsndfile"
args = [cpp_compiler] + flags.split()
outtext = ck_out(args)

log(outtext)
