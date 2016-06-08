# -*- coding: utf-8 -*-
"""
Created on Mon Apr 25 08:29:07 2016

@author: andres

Discovery_M7
"""

import os
from subprocess import check_output as ck_out
import shutil
import json
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
        self.log("Building Discovery_M7 project")
        self.log("Buiding in directory: " + self.out_dir)
        
        if not os.path.exists(self.out_dir + "/project"):
            shutil.copytree(self.platform_dir + '/project', self.out_dir + "/project")
        
        self.out_file = self.out_dir + "/project/Src/main.cpp"
        
    def log(self, text):
        print(text)
    
    def write_section_in_file(self, sec_name, code):
        filename = self.out_file
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
        self.block_size = 2048
        self.sample_rate = 48000
        self.num_out_chnls = 2
        self.num_in_chnls = 2
        self.audio_device = 0
        
        code = self.platform.generate_code(self.tree)
                
        #var_declaration = ''.join(['double stream_%02i;\n'%i for i in range(stream_index)])
        #declare_code = var_declaration + declare_code
        
        template_init_code = templates.get_config_code(self.sample_rate, self.block_size,
                        self.num_out_chnls, self.num_in_chnls, self.audio_device)

        globals_code = templates.get_globals_code(code['global_groups'])
        config_code = templates.get_configuration_code(code['global_groups']['initializations'])
       
        #shutil.rmtree(self.out_dir + "/project")
        if not os.path.exists(self.out_dir + "/project"):
            shutil.copytree(self.project_dir, self.out_dir + "/project")
        
        additional_init = "static float32_t Fs = %f;\n"%self.sample_rate       
        
        self.write_section_in_file('Includes', globals_code)
        self.write_section_in_file('Init Code', additional_init + code['header_code'])
        self.write_section_in_file('Config Code', code['init_code'] + template_init_code + config_code)
        self.write_section_in_file('Dsp Code', code['processing_code'])
        
        ck_out(['astyle', self.out_file ])
        
        
# Compile --------------------------
    def compile(self):
        import platform
        
        if platform.system() == "Linux":
            
#            cpp_compiler = self.platform_dir + "/arduino-1.6.8/arduino"
#        
#            flags =  '--upload --board arduino:avr:uno --port /dev/ttyACM0 --pref sketchbook.path=' + self.platform_dir + '/sketchbook'            

            args = ['cmake', '.', '-DSTRIDE_PLATFORM_ROOT=' + self.platform_dir ] #+ flags.split() + [self.out_file]
            outtext = ck_out(args, cwd=self.out_dir + "/project")
            self.log(outtext)
            
            args = ['make', '-j4'] #+ flags.split() + [self.out_file]
            outtext = ck_out(args, cwd=self.out_dir + "/project")
            self.log(outtext)
            
            args = ['arm-none-eabi-objcopy', '-O',  'binary', 'blinky.elf', 'blinky.bin'] #+ flags.split() + [self.out_file]
            outtext = ck_out(args, cwd=self.out_dir + "/project")
            self.log(outtext)
            
            args = [self.platform_dir + '/stlink/st-flash', '--reset', 'write', 'blinky.bin', '0x8000000'] #+ flags.split() + [self.out_file]
            outtext = ck_out(args, cwd=self.out_dir + "/project")
            self.log(outtext)
            
            args = [self.platform_dir + '/stlink/st-flash', '-rst'] #+ flags.split() + [self.out_file]
            outtext = ck_out(args, cwd=self.out_dir + "/project")
            self.log(outtext)
            
#../stlink/st-flash write blinky.bin 0x8000000
            self.log("Done.")
        else:
            self.log("Platform '%s' not supported!"%platform.system())