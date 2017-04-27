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

from subprocess import check_output as ck_out
import platform
import shutil
import os
from strideplatform import GeneratorBase

from platformTemplates import templates # Perhaps we should acces this through PlatformFunctions ?

class Generator(GeneratorBase):
    def __init__(self, out_dir = '',
                 strideroot = '',
                 platform_dir = '',
                 tree = None,
                 debug = False):

        super(Generator, self).__init__(out_dir, strideroot, platform_dir, tree, debug)

        self.project_dir = platform_dir + "/project"
        self.out_dir += "/RtAudio"
        if not os.path.isdir(self.out_dir):
            os.mkdir(self.out_dir)
        self.log("Building RtAudio project")
        self.log("Buiding in directory: " + self.out_dir)

    def generate_code(self):
        # Generate code from tree

        self.log("Platform code generation starting...")

        #domain = "AudioDomain"
        code = self.platform.generate_code(self.tree)

        #var_declaration = ''.join(['double stream_%02i;\n'%i for i in range(stream_index)])
        #declare_code = var_declaration + declare_code


        self.out_file = self.out_dir + "/main.cpp"
        shutil.copyfile(self.project_dir + "/template.cpp", self.out_file)
        if os.path.isdir(self.out_dir + "/rtaudio"):
            shutil.rmtree(self.out_dir + "/rtaudio")
        shutil.copytree(self.project_dir + "/rtaudio-4.1.2", self.out_dir + "/rtaudio")

        self.write_code(code,self.out_file)

        self.make_code_pretty()


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

        self.build_flags.append("-I" + self.out_dir + "/rtaudio")

        self.log("Platform code generation finished!")

# Compile --------------------------
    def compile(self):

        self.log("Platform code compilation started...")

        os.chdir(self.out_dir)

        if platform.system() == "Windows":

            source_files = [self.out_file, self.out_dir + "/rtaudio/RtAudio.cpp"]
            cpp_compiler = "c++"

            for f in source_files:
                short_f = f[f.rindex("/") + 1:]
                flags = ["-std=c++11",
                         "-I"+ self.platform_dir +"include",
                         "-O3",
                         "-DNDEBUG",
                         "-D__WINDOWS_WASAPI__",
                         "-Irtaudio/include"
                         "-o " + short_f + ".o",
                         "-c "+ f]

                args = [cpp_compiler] + flags

                outtext = ck_out(args)
                self.log(outtext)

            # Link ------------------------
            flags = ["-O3",
                     "-DNDEBUG",
                     "-D__WINDOWS_WASAPI__",
                     "-Irtaudio/include"]


            flags += [f[f.rindex("/") + 1:] + ".o" for f in source_files]
            flags += ["-o " + self.out_dir +"/app",
                     "-lole32",
                     "-lwinmm",
                     "-lksuser",
                     "-luuid"
                     ]

            args = [cpp_compiler] + flags

            outtext = ck_out(args)
            self.log(outtext)

        elif platform.system() == "Linux":

            source_files = [self.out_file, self.out_dir + "/rtaudio/RtAudio.cpp"]

            cpp_compiler = "/usr/bin/g++"

            pulse_defines = ['-D__LINUX_PULSE__']
            pulse_link_flags = ['-lpthread', '-lpulse-simple', '-lpulse']

            alsa_defines = ['-D__LINUX_ALSA__']
            alsa_link_flags = [ "-lasound", '-lpthread']

            jack_defines = ['-D__UNIX_JACK__']
            jack_link_flags = [ "-ljack", '-lpthread']

            modules = ['jack', 'alsa', 'pulse']

            defines = []
            link_flags = []

            if modules.count('pulse') > 0:
                defines += pulse_defines
                link_flags += pulse_link_flags
            elif modules.count('alsa') > 0:
                defines += alsa_defines
                link_flags += alsa_link_flags
            elif modules.count('jack') > 0:
                defines += jack_defines
                link_flags = jack_link_flags

            for f in source_files:
                short_f = f[f.rindex("/") + 1:]
                args = [cpp_compiler,
#                        "-I" + self.platform_dir + "/include",
                        "-I"+ self.out_dir + "/rtaudio",
                        "-O3" ,
                        "-std=c++11",
                        "-DNDEBUG"]
                args += defines
                args += ["-o" + short_f + ".o",
                         "-c",
                         f]

                self.log(args)

                outtext = ck_out(args)
                self.log(outtext)

             # Link ------------------------
            args = [cpp_compiler,
                    "-O3",
                    "-std=c++11",
                    "-DNDEBUG"
                    ]


            args += [f[f.rindex("/") + 1:] + ".o" for f in source_files]
            args += ["-o" + self.out_dir + "/app"]
            args += link_flags

            args += self.build_flags + self.link_flags

            self.log(args)

            outtext = ck_out(args)

            self.log(outtext)


        elif platform.system() == "Darwin":

            source_files = [self.out_file, self.out_dir + "/rtaudio/RtAudio.cpp"]

            cpp_compiler = "/usr/bin/c++"

            for f in source_files:
                short_f = f[f.rindex("/") + 1:]
                args = [cpp_compiler,
                        "-I"+ self.out_dir + "/rtaudio",
                        "-O3" ,
                        "-std=c++11",
                        "-DNDEBUG",
                        "-D__MACOSX_CORE__",
                        "-Irtaudio",
                         "-o" + short_f + ".o",
                         "-c",
                         f]

                self.log(args)

                outtext = ck_out(args)
                self.log(outtext)

            # Link ------------------------
            args = [cpp_compiler,
                    "-O3",
                    "-std=c++11",
                    "-DNDEBUG",
                    "-isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.12.sdk",
                    "-Wl,-search_paths_first",
                    "-Wl,-headerpad_max_install_names"]


            args += [f[f.rindex("/") + 1:] + ".o" for f in source_files]
            args += ["-o" + self.out_dir + "/app",
                    "-framework CoreFoundation",
                    "-framework CoreAudio",
                    "-lpthread"
                    ]

            args += self.build_flags + self.link_flags

            self.log(args)

            # ck_out didn't work properly on OS X
            os.system(' '.join(args))

        else:
            self.log("Platform '%s' not supported!"%platform.system())

        self.log("Platform code compilation finished!")
