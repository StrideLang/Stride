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
from subprocess import Popen
import platform
import shutil
import time
from strideplatform import GeneratorBase

from platformTemplates import templates # Perhaps we should acces this through PlatformFunctions ?

class Generator(GeneratorBase):
    def __init__(self, out_dir = '',
                 strideroot = '',
                 platform_dir = '',
                 tree = None,
                 debug = False):

        super(Generator, self).__init__(out_dir, strideroot, platform_dir, tree, debug)

        self.project_dir = platform_dir + '/project'
        self.out_file = self.out_dir + "/STM32F7/Src/main.cpp"
        self.target_name = 'app.elf'

        self.log("Building STM32F7 project")
        self.log("Buiding in directory: " + self.out_dir)

    def generate_code(self):

        self.log("Platform code generation starting...")

        #domain = "STM32F7_Domain"
        code = self.platform.generate_code(self.tree)

        #shutil.rmtree(self.out_dir + "/project")
        if not os.path.exists(self.out_dir + "/STM32F7"):
            shutil.copytree(self.project_dir, self.out_dir + "/STM32F7")

        filename = self.out_file

        self.write_code(code, filename)

        self.make_code_pretty()

        self.log("Platform code generation finished!")

# Compile --------------------------
    def compile(self):

        self.log("Platform code compilation started...")

        if platform.system() == "Windows":

            build_dir = self.out_dir + '/STM32F7'

            if not os.path.exists(build_dir + '/Build'):
                os.mkdir(build_dir + '/Build')

            object_list = open(build_dir + '/objects.list', 'w')

            c_compiler = ['arm-none-eabi-gcc']
            cxx_compiler = ['arm-none-eabi-g++']
            cxx_linker = ['arm-none-eabi-g++']
            asm_compiler = ['arm-none-eabi-as']

            include_dir = ['-I' + build_dir + '/Inc']
            include_hal = ['-I' + self.platform_dir + '/STM32Cube_FW_F7_V1.7.0/Drivers/STM32F7xx_HAL_Driver/Inc']
            include_hal_legacy = ['-I' + self.platform_dir + '/STM32Cube_FW_F7_V1.7.0/Drivers/STM32F7xx_HAL_Driver/Inc/Legacy']
            include_cmsis = ['-I' + self.platform_dir + '/STM32Cube_FW_F7_V1.7.0/Drivers/CMSIS/Include']
            include_cmsis_stm = ['-I' + self.platform_dir + '/STM32Cube_FW_F7_V1.7.0/Drivers/CMSIS/Device/ST/STM32F7xx/Include']

            source_dir = build_dir + '/Src/'
            source_hal_dir = self.platform_dir + '/STM32Cube_FW_F7_V1.7.0/Drivers/STM32F7xx_HAL_Driver/Src/'
            source_cmsis_system_dir = self.platform_dir + '/STM32Cube_FW_F7_V1.7.0/Drivers/CMSIS/Device/ST/STM32F7xx/Source/Templates/'
            source_cmsis_startup_dir = self.platform_dir + '/STM32Cube_FW_F7_V1.7.0/Drivers/CMSIS/Device/ST/STM32F7xx/Source/Templates/gcc/'

            library_file = ['-lm', '-larm_cortexM7lfsp_math']
            library_dir = ['-L' + self.platform_dir +'/STM32Cube_FW_F7_V1.7.0/Drivers/CMSIS/Lib/GCC']

            linker_script = ['-TSTM32F769BITx_FLASH.ld']

            common_flags = ['-mcpu=cortex-m7', '-mthumb', '-mfloat-abi=hard', '-mfpu=fpv5-d16']
            compiler_flags = ['-Os', '-g3', '-Wall', '-fmessage-length=0', '-ffunction-sections', '-c', '-fmessage-length=0']
            linker_flags = ['-specs=nosys.specs', '-specs=nano.specs', '-Wl,-Map=output.map', '-Wl,--gc-sections']

            compiler_definitions =  [
                    '-DARM_MATH_CM7',
                    '-DUSE_HAL_DRIVER',
                    '-DSTM32F746xx',
                    '-D__weak=__attribute__((weak))',
                    '-D__packed=__attribute__((__packed__))',
                ]

            sources = []
            for root, dirs, files in os.walk(source_dir):
                for file in files:
                    if file.endswith('.c') or file.endswith('.cpp'):
                        sources.append(file)

            sources_hal = [ 'stm32f7xx_hal.c',
                            'stm32f7xx_hal_cortex.c',
                            'stm32f7xx_hal_dma.c',
                            'stm32f7xx_hal_flash.c',
                            'stm32f7xx_hal_gpio.c',
                            'stm32f7xx_hal_pwr.c',
                            'stm32f7xx_hal_pwr_ex.c',
                            'stm32f7xx_hal_rcc.c',
                            'stm32f7xx_hal_rcc_ex.c',
                            'stm32f7xx_hal_tim.c',
                            'stm32f7xx_hal_tim_ex.c',
                            'stm32f7xx_hal_sai.c',
                            'stm32f7xx_hal_sai_ex.c',
                            'stm32f7xx_hal_i2c.c',
                            'stm32f7xx_hal_i2c_ex.c',
                        ]
            sources_cmsis_system = ['system_stm32f7xx.c']
            sources_cmsis_startup = ['startup_stm32f746xx.s']

            compiler_flags_all =    compiler_definitions \
                                    + include_dir \
                                    + include_hal \
                                    + include_hal_legacy \
                                    + include_cmsis \
                                    + include_cmsis_stm \
                                    + common_flags \
                                    + compiler_flags

            for source in sources:
                # TODO IF FILE DIDN't CHANGE FROM LAST BUILD NO NEED TO BUILD
                # IF ALL FILES DIDN't CHANGE NO NEED TO CALL LINKER EITHER
                if source.split('.')[-1] == 'c':
                    args =  c_compiler \
                            + compiler_flags_all \
                            + ['-o', build_dir + '/Build/' + source + '.o'] \
                            + ['-c', source_dir + source]
                else:
                    args =  cxx_compiler \
                            + compiler_flags_all \
                            + ['-o', build_dir + '/Build/' + source + '.o'] \
                            + ['-c', source_dir + source]
                # self.log(' '.join(args))
                object_list.write('Build/' + source + '.o' + '\n')
                outtext = ck_out(args)

            for source in sources_hal:
                if not os.path.isfile(build_dir + '/Build/' + source + '.o'):
                    args =  c_compiler \
                            + compiler_flags_all \
                            + ['-o', build_dir + '/Build/' + source + '.o'] \
                            + ['-c', source_hal_dir + source]
                    # self.log(' '.join(args))
                    outtext = ck_out(args)
                object_list.write('Build/' + source + '.o' + '\n')

            for source in sources_cmsis_system:
                if not os.path.isfile(build_dir + '/Build/' + source + '.o'):
                    args =  c_compiler \
                            + compiler_flags_all \
                            + ['-o', build_dir + '/Build/' + source + '.o'] \
                            + ['-c', source_cmsis_system_dir + source]
                    # self.log(' '.join(args))
                    outtext = ck_out(args)
                object_list.write('Build/' + source + '.o' + '\n')

            for source in sources_cmsis_startup:
                if not os.path.isfile(build_dir + '/Build/' + source + '.o'):
                    # THE FOLLOWING CALL IS BASED ON SYSTEM WORKBENCH ON WINDOWS
                    # args =  asm_compiler \
                    #         + common_flags \
                    #         + ['-g'] \
                    #         + ['-o', build_dir + '/Build/' + source + '.o'] \
                    #         + [source_cmsis_startup_dir + source]

                    # THE FOLLOWING CODE IS BASED ON LINUX / CMAKE / MAKE
                    args =  c_compiler \
                            + compiler_flags_all \
                            + ['-o', build_dir + '/Build/' + source + '.o'] \
                            + ['-c', source_cmsis_startup_dir + source]
                    # self.log(' '.join(args))
                    outtext = ck_out(args)
                object_list.write('Build/' + source + '.o' + '\n')

            object_list.close()

            args =  cxx_linker \
                    + ['@objects.list'] \
                    + ['-o', self.target_name] \
                    + common_flags \
                    + library_dir \
                    + linker_flags \
                    + linker_script \
                    + library_file
            # self.log(' '.join(args))
            outtext = ck_out(args, cwd=build_dir)

            args =  [   'arm-none-eabi-objcopy',
                        '-O',
                        'binary',
                        self.target_name,
                        'app.bin'
                    ]
            # self.log(' '.join(args))
            outtext = ck_out(args, cwd=build_dir)

            openOCD_bin = 'C:/Program Files/GNU ARM Eclipse/OpenOCD/0.10.0-201601101000-dev/bin/openocd'
            openOCD_cfg_file = self.platform_dir + '/openOCD/stm32f746g_disco.cfg'

            openOCD_bin_file = build_dir + '/app.bin'
            openOCD_bin_file = openOCD_bin_file.split('\\')
            openOCD_bin_file = '/'.join(openOCD_bin_file)

            args = [openOCD_bin,
                    '-f' + openOCD_cfg_file,
                    '-c init',
                    '-c "reset init"',
                    '-c halt',
                    '-c "flash write_image erase ' + openOCD_bin_file + ' 0x08000000" ',
                    '-c reset',
                    '-c shutdown']

            self.log(' '.join(args))
            Popen(' '.join(args))

        elif platform.system() == "Darwin" or platform.system() == "Linux":

            run_prefix = ''
            if platform.system() == "Darwin":
                run_prefix = '/usr/local/bin/'
            build_dir = self.out_dir + '/STM32F7'
            if os.path.exists(build_dir + "/" + self.target_name):
                os.remove(build_dir + "/" + self.target_name)

            # The following two lines work when run from: Spyder
            # StreamStacker >> Completes execution without having run cmake
            # args = ['cmake', '.', '-DSTRIDE_PLATFORM_ROOT=' + self.platform_dir]
            # outtext = ck_out(args, cwd=build_dir)

            # The following two lines work when run from: Spyder and StreamStacker
            args = [run_prefix + 'cmake', '.', '-DSTRIDE_PLATFORM_ROOT=' + self.platform_dir]
            outtext = ck_out(args, cwd=build_dir)
            self.log(outtext)

            # The following two lines work when run from: StreamStacker and Spyder
            # cmake_cmd = 'PATH=$PATH\:/usr/local/bin/ ; export PATH & cmake . -DSTRIDE_PLATFORM_ROOT=' + self.platform_dir
            # Popen(cmake_cmd, cwd=build_dir, shell=True)

            # The following two lines DO NOT WORK with Spyder
            # OSError: [Errno 2] No such file or directory
            # This might be due to not finding gcc and g++
            # The following two lines DO NOT WORK with StreamStacker
            # StreamStacker >> Completes execution without having run make
            # args = ['usr/bin/make']
            # outtext = ck_out(args, cwd=build_dir)

            # The following two lines work when run from: Spyder and StreamStacker
            # TODO Find out how to capture output
            make_cmd =  'PATH=$PATH\:/usr/local/bin/:/usr/bin/ ; export PATH & make -j4'
            Popen(make_cmd, cwd=build_dir, shell=True)

            # Since 'make' is executing in a "different" shell we need to wait for it to complete
            # 50 * 0.1 sec = 5 sec
            time_out = 50
            while (not os.path.isfile(build_dir + '/' + self.target_name) and time_out):
                time.sleep(0.1)
                time_out -= 1

            # Wait for the file to be written to
            # TODO THIS IS NOT A PROPER SOLUTION!
            # 1 second might not be enough
            if not os.path.exists(build_dir + '/' + self.target_name) or os.stat(build_dir + '/' + self.target_name).st_size == 0:
                time.sleep(2)

            # The following two lines work when run from: Spyder and StreamStacker
            args = [run_prefix + 'arm-none-eabi-objcopy', '-O', 'binary', build_dir +  '/' + self.target_name, build_dir + '/app.bin']

            self.log(' '.join(args))
            outtext = ck_out(args, cwd=build_dir)
            self.log(outtext)

            # The following lines work when run from: Spyder and StreamStacker
#            openOCD_dir = "/Applications/GNU ARM Eclipse/OpenOCD/0.10.0-201701241841/scripts"
            openOCD_dir = self.strideroot + "/utilities/openocd/0.10.0-201701241841/scripts/"
            openOCD_bin = openOCD_dir + "../bin/openocd"

            openOCD_cfg_file = openOCD_dir + "/board/saturnM7.cfg"
#            openOCD_cfg_file = self.platform_dir + "/openOCD/stm32f746g_disco.cfg"
            openOCD_bin_file = self.out_dir + "/STM32F7/app.bin"

            args = [openOCD_bin,
                    '-f' + openOCD_cfg_file,
                    '-c init',
                    '-c reset init',
                    '-c halt',
                    '-c flash write_image erase ' + openOCD_bin_file + ' 0x08000000',
                    '-c reset',
                    '-c shutdown']

            self.log(' '.join(args))

            outtext = ck_out(args, cwd=openOCD_dir  )
            self.log(outtext)

        else:

            self.log("Platform '%s' not supported!"%platform.system())

        self.log("Platform code compilation finished!")

    def run(self):
        self.log("Run has no effect on STM32F7 framework.")
