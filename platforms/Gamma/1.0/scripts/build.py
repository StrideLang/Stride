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
    log(text)
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

dsp_code = ''
num_chnls = 2;

def get_function_code(function_name, properties, token):
    print(properties)
    init_code = '';
    dsp_code = '';
    if function_name == 'level':
        dsp_code = token + ' = ' + token + ' * ' + str(properties['gain']['value']) + ';\n'

    return init_code, dsp_code

for node in tree:
    if 'stream' in node:
        for parts in node['stream']:
            if parts['type'] == 'Bundle':
                if parts['name'] == 'AudioIn':
                    chan_index = parts['index'] - 1;
                    dsp_code += 'float sig_%02i = io.in(%i);\n'%(stream_index, chan_index)

                elif parts['name'] == 'AudioOut':
                    chan_index = parts['index'] - 1;
                    dsp_code += 'io.out(%i) = sig_%02i;\n'%(chan_index, stream_index)
            elif parts['type'] == 'Name':
                if parts['name'] == 'AudioIn':
                    for chan_index in range(num_chnls):
                        dsp_code += 'float sig_%02i_%02i = io.in(%i);\n'%(stream_index, chan_index, chan_index)
                elif parts['name'] == 'AudioIn':
                    for chan_index in range(num_chnls):
                        dsp_code += 'io.out(%i) = sig_%02i_%02i;\n'%(chan_index, stream_index, chan_index)
            elif parts['type'] == 'Function':
                new_init_code, new_dsp_code = get_function_code(parts['name'], parts["properties"], 'sig_%02i'%stream_index)
                dsp_code += new_dsp_code
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
