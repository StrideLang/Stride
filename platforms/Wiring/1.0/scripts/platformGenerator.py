# -*- coding: utf-8 -*-
"""
Created on Mon Apr 25 08:29:07 2016

@author: andres
"""

import os
from subprocess import check_output as ck_out
import platform
import shutil
import glob

from strideplatform import GeneratorBase

from platformTemplates import templates # Perhaps we should acces this through PlatformFunctions ?

class Generator(GeneratorBase):
    def __init__(self, out_dir = '',
                 platform_dir = '',
                 debug = False):

        super(Generator, self).__init__(out_dir, platform_dir, debug)

        self.log("Building Wiring project")
        self.log("Buiding in directory: " + self.out_dir)

        if not os.path.exists(self.out_dir + "/template"):
            os.mkdir(self.out_dir + "/template")

        self.out_file = self.out_dir + "/template/template.ino"

    def generate_code(self):

        self.log("Platform code generation starting...")

        domain = "WiringDomain"
        code = self.platform.generate_code(self.tree, domain)

        shutil.copyfile(self.project_dir + "/template.ino", self.out_file)

        self.write_code(code,self.out_file)
        self.make_code_pretty()

        self.log("Platform code generation finished!")

# Compile --------------------------
    def compile(self):

        self.log("Platform code compilation started...")

        if platform.system() == "Linux":

            self.log("Buidling and uploading Arduino on Linux...")

            arduino_dir = glob.glob(self.platform_dir + "/arduino-*")

            cpp_compiler = arduino_dir[0] + "/arduino"
            flags =  '--upload --board arduino:avr:uno --port /dev/ttyACM1 --pref sketchbook.path=' + self.platform_dir + '/sketchbook'
            args = [cpp_compiler] + flags.split() + [self.out_file]

            outtext = ck_out(args)
            self.log(outtext)

            self.log("Done building and uploading.")

        elif platform.system() == "Windows":

            self.log("Buidling and uploading Arduino on Windows...")

            cpp_compiler = "C:/Program Files (x86)/Arduino/arduino"
            flags =  '--upload --board arduino:avr:uno --port COM7'
            args = [cpp_compiler] + flags.split() + [self.out_file]

            outtext = ck_out(args)
            self.log(outtext)

            self.log("Done building and uploading.")

        elif platform.system() == "Darwin":

            self.log("Buidling and uploading Arduino on OS X..")

            cpp_compiler = "/Applications/Arduino.app/Contents/MacOS/Arduino"
            flags =  '--upload --board arduino:avr:uno --port /dev/cu.usbmodem1411' # --pref sketchbook.path=' + self.platform_dir + '/sketchbook'
            args = [cpp_compiler] + flags.split() + [self.out_file]

            outtext = ck_out(args)
            self.log(outtext)

            self.log("Done building and uploading.")

        else:
            self.log("Platform '%s' not supported!"%platform.system())

        self.log("Platform code compilation finished!")
