# -*- coding: utf-8 -*-
"""
    Stride is licensed under the terms of the 3-clause BSD license.

    Copyright (C) 2017. The Regents of the University of California.
    All rights reserved.
    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

        Redistributions of source code must retain the above copyright notice,
        this list of conditions and the following disclaimer.

        Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

        Neither the name of the copyright holder nor the names of its
        contributors may be used to endorse or promote products derived from
        this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

    Authors: Andres Cabrera and Joseph Tilbian
"""

import os
from subprocess import check_output as ck_out
import platform
import shutil
import glob

from strideplatform import GeneratorBase


from platformTemplates import templates # Perhaps we should acces this through PlatformFunctions ?

from platformConfiguration import configuration

class Generator(GeneratorBase):
    def __init__(self, out_dir = '',
                 strideroot = '',
                 platform_dir = '',
                 tree = None,
                 debug = False):

        super(Generator, self).__init__(out_dir, strideroot, platform_dir, tree, debug)

        self.log("Building Wiring project")
        self.log("Buiding in directory: " + self.out_dir)

        self.project_dir = platform_dir + "/project"
        if not os.path.exists(self.out_dir + "/template"):
            os.mkdir(self.out_dir + "/template")

        self.out_file = self.out_dir + "/template/template.ino"

    def generate_code(self):

        self.log("Platform code generation starting...")

        #domain = "WiringDomain"
        code = self.platform.generate_code(self.tree)

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
            # FIXME find better way to detect board...
            port = "/dev/ttyUSB0"
            for i in range(64):
                if os.path.exists("/dev/ttyACM%i"%i):
                    port = "/dev/ttyACM%i"%i

            flags =  '--upload --board arduino:avr:leonardo --port ' + port + ' --pref sketchbook.path=' + self.platform_dir + '/sketchbook'
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
            flags =  '--upload --board arduino:avr:leonardo --port /dev/cu.usbmodem1421' # --pref sketchbook.path=' + self.platform_dir + '/sketchbook'
            args = [cpp_compiler] + flags.split() + [self.out_file]

            outtext = ck_out(args)
            self.log(outtext)

            self.log("Done building and uploading.")

        else:
            self.log("Platform '%s' not supported!"%platform.system())

        self.log("Platform code compilation finished!")

    def run(self):
        self.log("Run has no effect on Wiring framework.")
