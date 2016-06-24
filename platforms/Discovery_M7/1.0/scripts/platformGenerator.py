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

        self.out_dir = out_dir
        self.platform_dir = platform_dir

        self.project_dir = platform_dir + '/project'
        self.out_file = self.out_dir + "/project/Src/main.cpp"

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

    def log(self, text):
        print(text)

    def write_section_in_file(self, sec_name, code):
        filename = self.out_file
        f = open(filename, 'r')
        text = f.read()
        f.close()

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

        self.log("Platform code generation starting...")

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

        #additional_init = "static float32_t Fs = %f;\n"%self.sample_rate
	additional_init = ""

        self.write_section_in_file('Includes', globals_code)
        self.write_section_in_file('Init Code', additional_init + code['header_code'])
        self.write_section_in_file('Config Code', code['init_code'] + template_init_code + config_code)
        self.write_section_in_file('Dsp Code', code['processing_code'])

        import platform

        if platform.system() == "Linux":
            try:
                ck_out(['astyle', self.out_file ])
            except:
                self.log("Error running astyle!")
        else:
            self.log("Platform '%s' not supported!"%platform.system())

        self.log("Platform code generation finished!")

# Compile --------------------------
    def compile(self):

        self.log("Platform code compilation started...")

        import platform

        if platform.system() == "Windows":

            self.log("Windows NOT supported yet.")

        elif platform.system() == "Linux":

            args = ['cmake', '.', '-DSTRIDE_PLATFORM_ROOT=' + self.platform_dir ]
            outtext = ck_out(args, cwd=self.out_dir + "/project")
            self.log(outtext)

            args = ['make', '-j4']
            outtext = ck_out(args, cwd=self.out_dir + "/project")
            self.log(outtext)

            args = ['arm-none-eabi-objcopy', '-O', 'binary', 'app.elf', 'app.bin']
            outtext = ck_out(args, cwd=self.out_dir + "/project")
            self.log(outtext)

            openOCD_dir = "/opt/gnuarmeclipse/openocd/0.10.0-201601101000-dev/scripts"
            openOCD_bin = "../bin/openocd"
            openOCD_cfg_file = self.platform_dir + "/openOCD/stm32f746g_disco.cfg"
            openOCD_bin_file = self.out_dir + "/project/app.bin"

            args = [openOCD_bin,
                    '-f' + openOCD_cfg_file,
                    '-c init',
                    '-c reset init',
                    '-c halt',
                    '-c flash write_image erase ' + openOCD_bin_file + ' 0x08000000',
                    '-c reset',
                    '-c shutdown']

            outtext = ck_out(args, cwd=openOCD_dir)
            self.log(outtext)
#
#           THE FOLLOWING CODE CAN BE REMOVED
#

#            args = [self.platform_dir + '/stlink/st-flash', '--reset', 'write', 'app.bin', '0x8000000']
#            outtext = ck_out(args, cwd=self.out_dir + "/project")
#            self.log(outtext)
#
#            args = [self.platform_dir + '/stlink/st-flash', '-rst']
#            outtext = ck_out(args, cwd=self.out_dir + "/project")
#            self.log(outtext)

        elif platform.system() == "Darwin":

            args = ['cmake', '.', '-DSTRIDE_PLATFORM_ROOT=' + self.platform_dir]
#            command_line = 'cd ' + self.out_dir + '/project' ' & ' + ' '.join(args)
#            print(command_line)
#            os.system(command_line)
            print(' '.join(args))
            outtext = ck_out(args, cwd=self.out_dir + "/project")
#            self.log(outtext)

            args = ['make', '-j4']
            outtext = ck_out(args, cwd=self.out_dir + "/project")
#            self.log(outtext)

            args = ['arm-none-eabi-objcopy', '-O', 'binary', 'app.elf', 'app.bin']
            outtext = ck_out(args, cwd=self.out_dir + "/project")
#            self.log(outtext)
            
            openOCD_dir = "/Applications/GNU\ ARM\ Eclipse/OpenOCD/0.10.0-201601101000-dev/scripts"
            openOCD_bin = "../bin/openocd"
#            openOCD_cfg_file = self.platform_dir + "/openOCD/stm32f746g_disco.cfg"
            openOCD_cfg_file = "/Users/pacifist/Documents/Qt/StreamStack/platforms/Discovery_M7/1.0/openOCD/stm32f746g_disco.cfg"            
            openOCD_bin_file = self.out_dir + "/project/app.bin"


            args = [
#                    'cd',
#                    openOCD_dir,
#                    '&',
                    openOCD_bin,
                    '-f ' + openOCD_cfg_file,
                    '-c init',
                    '-c "reset init"',
                    '-c halt',
                    '-c "flash write_image erase ' + openOCD_bin_file + ' 0x08000000"',
                    '-c reset',
                    '-c shutdown']

            print(' '.join(args))

#            outtext = ck_out(args)
            
            command_line = 'cd ' + openOCD_dir + ' & ' + ' '.join(args)
            self.log(command_line)
#
#            # ck_out is not working properly
            os.system(command_line)

            self.log("OS X NOT supported yet.")

        else:

            self.log("Platform '%s' not supported!"%platform.system())

        self.log("Platform code compilation finished!")
