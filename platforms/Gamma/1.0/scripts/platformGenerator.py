# -*- coding: utf-8 -*-
"""
Created on Mon Apr 25 08:29:07 2016

@author: andres
"""
from subprocess import check_output as ck_out
from subprocess import call
import shutil
import json
import os
import sys
from strideplatform import PlatformFunctions

from platformTemplates import templates # Perhaps we should acces this through PlatformFunctions ?

class Generator:
    def __init__(self, out_dir = '',
                 platform_dir = '',
                 debug = False):
        
        self.platform_dir = platform_dir
        self.out_dir = out_dir
        
        self.project_dir = platform_dir + '/project'
        
        jsonfile = open(self.out_dir + '/tree.json')
        self.tree = json.load(jsonfile)
        
        self.platform = PlatformFunctions(self.tree, debug)
        
        self.last_num_outs = 0
        
        # TODO get gamma sources and build and install if not available
        # Question: building requires cmake, should binaries be distributed instead?
        # What about secondary deps like portaudio and libsndfile?
        self.log("Building Gamma project")
        self.log("Buiding in directory: " + self.out_dir)

        
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
    
    def generate_code(self):
        # Generate code from tree
        # TODO These defaults should be set from the platform definition file
        self.block_size = 512
        self.sample_rate = 44100
        self.num_out_chnls = 2
        self.num_in_chnls = 2
        self.audio_device = -1
        
        code = self.platform.generate_code(self.tree)
                
        print("OK.")
        #var_declaration = ''.join(['double stream_%02i;\n'%i for i in range(stream_index)])
        #declare_code = var_declaration + declare_code
        
        template_init_code = templates.get_config_code(self.sample_rate, self.block_size,
                        self.num_out_chnls, self.num_in_chnls, self.audio_device)

        globals_code = templates.get_globals_code(code['global_groups'])
        config_code = templates.get_configuration_code(code['global_groups']['initializations'])
        
        shutil.copyfile(self.project_dir + "/template.cpp", self.out_dir + "/main.cpp")
        self.write_section_in_file('Includes', globals_code)
        self.write_section_in_file('Init Code', code['header_code'])
        self.write_section_in_file('Config Code', code['init_code'] + template_init_code + config_code)
        self.write_section_in_file('Dsp Code', code['processing_code'])
        
        try:
            ck_out(['astyle', self.out_file ])
        except:
            print("Error running astyle")
        
        self.link_flags = []
        for link_target in code['global_groups']['linkTo']:
            new_flag = "-l" + link_target
            if not new_flag in self.link_flags:
                self.link_flags.append(new_flag)
                
        for link_dir in code['global_groups']['linkDir']:
            new_flag = "-L" + link_dir
            if not new_flag in self.link_flags:
                self.link_flags.append(new_flag)
                
        self.build_flags = []
        for include_dir in code['global_groups']['includeDir']:
            new_flag = "-I" + include_dir
            if not new_flag in self.build_flags:
                self.build_flags.append(new_flag)
                
        
        
        
# Compile --------------------------
    def compile(self):
        import platform
        
        if platform.system() == "Windows":
            cpp_compiler = "c++"
        
            flags = "-std=c++11 -I"+ self.platform_dir +"include -O3 -DNDEBUG -o " \
                + self.out_dir +"/main.cpp.o -c "+ self.out_dir +"/main.cpp"
            args = [cpp_compiler] + flags.split()
            self.log(' '.join(args))
            outtext = ck_out(args)
        
            self.log(outtext)
        
            # Link ------------------------
            flags = "-O3 -DNDEBUG "+ self.out_dir + "/main.cpp.o -o " \
                + self.out_dir +"/app -L " \
                + self.platform_dir + "/lib -lGamma -lportaudio_x86 -lsndfile-1"
            args = [cpp_compiler] + flags.split()
            outtext = ck_out(args)
        
            self.log(outtext)
            self.log("Done.")
        
        
        elif platform.system() == "Linux":
            cpp_compiler = "/usr/bin/g++"
        
            args = [cpp_compiler,
                    "-I"+ self.platform_dir +"/include",  "-O3" , "-std=c++11", "-DNDEBUG",
                     "-o" + self.out_dir +"/main.cpp.o",
                     "-c",
                     self.out_dir + "/main.cpp"]

            
            #self.log(' '.join(args))
            #os.system(' '.join(args))
            
            print(args)
            outtext = ck_out(args)
        
            self.log(outtext)
        
            # Link ------------------------
            gamma_flags = ["-rdynamic", "-lGamma", "-lpthread", "-lportaudio", "-lsndfile"]
            args = [cpp_compiler,
                    "-O3",
                    "-DNDEBUG",
                    self.out_dir +"/main.cpp.o",
                    "-o" + self.out_dir +"/app",
                    "-rdynamic",
                    "-L" + self.platform_dir + "/lib"]
                    
            
            args += gamma_flags + self.build_flags + self.link_flags 
            
            #self.log(' '.join(args))
            print(args)

            outtext = ck_out(args)
        
            self.log(outtext)
            self.log("Done.")
        
        elif platform.system() == "Darwin":
            
            self.log("Start build.")
            cpp_compiler = "/usr/bin/c++"
            
            args = [cpp_compiler,
                    "-I"+ self.platform_dir +"/include",  "-O3" , "-std=c++11", "-DNDEBUG",
                     "-o" + self.out_dir +"/main.cpp.o",
                     "-c",
                     self.out_dir + "/main.cpp"]

            
            #self.log(' '.join(args))
            #os.system(' '.join(args))
            
            print(args)
            outtext = ck_out(args)
        
            self.log(outtext)
        
            # Link ------------------------
            args = [cpp_compiler,
                    "-O3",
                    "-std=c++11",
                    "-DNDEBUG",
                    "-isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.11.sdk",
                    "-Wl,-search_paths_first",
                    "-Wl,-headerpad_max_install_names",
                    self.out_dir +"/main.cpp.o",
                    "-o" + self.out_dir +"/app",
                    "/usr/local/lib/libportaudio.dylib",
                    "/usr/local/lib/libsndfile.dylib",
                    "-framework AudioUnit",
                    "-framework CoreAudio",
                    "-framework CoreServices",
                    "-framework AudioToolbox ", 
                    "-L" + self.platform_dir + "/lib",
                    "-lGamma"
                    ]
                    
            
            args += self.build_flags + self.link_flags 
            
            self.log(' '.join(args))
            #print(args)

            #outtext = ck_out(args, shell=True)
            #call(args, shell=True)
            os.system(' '.join(args))
        
            self.log(outtext)
            self.log("Done.")
        
#            flags = "-I"+ self.platform_dir +"/include -O3 -DNDEBUG -o " \
#                + self.out_dir +"/main.cpp.o -c "+ self.out_dir +"/main.cpp"
#            args = [cpp_compiler] + flags.split()
#            try:
#                outtext = ck_out(args)
#            except:
#                pass
#        
#            self.log(outtext)
#        
#            # Link ------------------------
#            flags = "-O3 -DNDEBUG "+ self.out_dir +"/main.cpp.o -o "+ self.out_dir +"/app -rdynamic -L " \
#                + self.platform_dir + "/lib -lGamma -lpthread -lportaudio -lsndfile -lpthread -lportaudio -lsndfile"
#            args = [cpp_compiler] + flags.split()
#            try:
#                outtext = ck_out(args)
#            except:
#                pass
#        
#            self.log(outtext)
#            self.log("Done.")
        else:
            self.log("Platform '%s' not supported!"%platform.system())
