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
log("On directory " + out_dir)

#shutil.rmtree(out_dir)
#shutil.copytree(gamma_directory, out_dir)

shutil.copyfile(project_dir + "/template.cpp", out_dir + "/main.cpp")

cpp_compiler = "/usr/bin/c++"

flags = "-I"+ platform_dir +"/include -O3 -DNDEBUG -o "+ out_dir +"/main.cpp.o -c "+ out_dir +"/main.cpp"
args = [cpp_compiler] + flags.split()
outtext = ck_out(args)

print(outtext)
flags = "-O3 -DNDEBUG "+ out_dir +"/main.cpp.o -o "+ out_dir +"/app -rdynamic -L " + platform_dir + "/lib -lGamma -lpthread -lportaudio -lsndfile -lpthread -lportaudio -lsndfile"
args = [cpp_compiler] + flags.split()
outtext = ck_out(args)

print(outtext)
