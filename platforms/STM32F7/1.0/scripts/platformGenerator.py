# -*- coding: utf-8 -*-
"""
Created on Mon Apr 25 08:29:07 2016

@author: andres

STM32F7
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
                 platform_dir = '',
                 debug = False):
                     
        super(Generator, self).__init__(out_dir, platform_dir, debug)

        self.out_file = self.out_dir + "/project/Src/main.cpp"   

        self.log("Building STM32F7 project")
        self.log("Buiding in directory: " + self.out_dir)

        if not os.path.exists(self.out_dir + "/project"):
            shutil.copytree(self.platform_dir + '/project', self.out_dir + "/project")

    def generate_code(self):
        # Generate code from tree
        # TODO These defaults should be set from the platform definition file
        self.block_size = 2048
        self.sample_rate = 48000
        self.num_out_chnls = 2
        self.num_in_chnls = 2
        self.audio_device = 0

        self.log("Platform code generation starting...")
        
        domain = "STM32F7_Domain"
        code = self.platform.generate_code(self.tree,domain)

        #shutil.rmtree(self.out_dir + "/project")
        if not os.path.exists(self.out_dir + "/project"):
            shutil.copytree(self.project_dir, self.out_dir + "/project")

        filename = self.out_file
        
        self.write_code(code, filename)

        if platform.system() == "Linux":
            try:
                self.log("Running astyle...")
                ck_out(['astyle', self.out_file ])
            except:
                self.log("Error running astyle!")
        elif platform.system() == "Darwin":
            try:
                self.log("Running astyle...")
                ck_out(['/usr/local/bin/astyle', self.out_file ])
            except:
                self.log("Error running astyle!")
        else:
            self.log("Astyle not supported on '%s'!"%platform.system())

        self.log("Platform code generation finished!")

# Compile --------------------------
    def compile(self):

        self.log("Platform code compilation started...")

        if platform.system() == "Windows":

            build_dir = self.out_dir + '/project'

            if not os.path.exists(build_dir + '/Build'):
                os.mkdir(build_dir + '/Build')

            object_list = open(build_dir + '/objects.list', 'w')

            c_compiler = ['arm-none-eabi-gcc']
            cxx_compiler = ['arm-none-eabi-g++']
            cxx_linker = ['arm-none-eabi-g++']
            asm_compiler = ['arm-none-eabi-as']

            include_dir = ['-I' + build_dir + '/Inc']
            include_hal = ['-I' + self.platform_dir + '/STM32Cube_FW_F7_V1.3.0/Drivers/STM32F7xx_HAL_Driver/Inc']
            include_hal_legacy = ['-I' + self.platform_dir + '/STM32Cube_FW_F7_V1.3.0/Drivers/STM32F7xx_HAL_Driver/Inc/Legacy']
            include_cmsis = ['-I' + self.platform_dir + '/STM32Cube_FW_F7_V1.3.0/Drivers/CMSIS/Include']
            include_cmsis_stm = ['-I' + self.platform_dir + '/STM32Cube_FW_F7_V1.3.0/Drivers/CMSIS/Device/ST/STM32F7xx/Include']

            source_dir = build_dir + '/Src/'
            source_hal_dir = self.platform_dir + '/STM32Cube_FW_F7_V1.3.0/Drivers/STM32F7xx_HAL_Driver/Src/'
            source_cmsis_system_dir = self.platform_dir + '/STM32Cube_FW_F7_V1.3.0/Drivers/CMSIS/Device/ST/STM32F7xx/Source/Templates/'
            source_cmsis_startup_dir = self.platform_dir + '/STM32Cube_FW_F7_V1.3.0/Drivers/CMSIS/Device/ST/STM32F7xx/Source/Templates/gcc/'

            library_file = ['-lm', '-larm_cortexM7lfsp_math']
            library_dir = ['-L' + self.platform_dir +'/STM32Cube_FW_F7_V1.3.0/Drivers/CMSIS/Lib/GCC']

            linker_script = ['-TSTM32F746NGHx_FLASH.ld']

            common_flags = ['-mcpu=cortex-m7', '-mthumb', '-mfloat-abi=hard', '-mfpu=fpv5-sp-d16']
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
                    + ['-o', 'app.elf'] \
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
                        'app.elf',
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

            openOCD_dirs = ["/opt/gnuarmeclipse/openocd/0.10.0-201601101000-dev/", "/home/andres/Documents/src/openocd/0.10.0-201601101000-dev"]
            openOCD_bin = "bin/openocd"
            for directory in openOCD_dirs:
                if os.path.exists(directory + "/" + openOCD_bin):
                    openOCD_dir = directory
                    break
                     
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
#           THE FOLLOWING CODE CAN BE REMOVED // STLINK REPLACED WITH OPENOCD
#

#            args = [self.platform_dir + '/stlink/st-flash', '--reset', 'write', 'app.bin', '0x8000000']
#            outtext = ck_out(args, cwd=self.out_dir + "/project")
#            self.log(outtext)
#
#            args = [self.platform_dir + '/stlink/st-flash', '-rst']
#            outtext = ck_out(args, cwd=self.out_dir + "/project")
#            self.log(outtext)

        elif platform.system() == "Darwin":

            build_dir = self.out_dir + '/project'

            # The following two lines work when run from: Spyder
            # StreamStacker >> Completes execution without having run cmake
            # args = ['cmake', '.', '-DSTRIDE_PLATFORM_ROOT=' + self.platform_dir]
            # outtext = ck_out(args, cwd=build_dir)

            # The following two lines work when run from: Spyder and StreamStacker
            args = ['/usr/local/bin/cmake', '.', '-DSTRIDE_PLATFORM_ROOT=' + self.platform_dir]
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

            # Since 'make' is executing in a "differet" shell we need to wait for it to complete
            # 50 * 0.1 sec = 5 sec
            time_out = 50
            while (not os.path.isfile(build_dir + '/app.elf') and time_out):
                time.sleep(0.1)
                time_out -= 1

            # Wait for the file to be written to
            # TODO THIS IS NOT A PROPER SOLUTION!
            # 1 second might not be enough
            if os.stat(build_dir + '/app.elf').st_size == 0:
                time.sleep(1)

            # The following two lines work when run from: Spyder and StreamStacker
            args = ['/usr/local/bin/arm-none-eabi-objcopy', '-O', 'binary', 'app.elf', 'app.bin']
            outtext = ck_out(args, cwd=build_dir)
            self.log(outtext)

            # The following lines work when run from: Spyder and StreamStacker
            openOCD_dir = "/Applications/GNU ARM Eclipse/OpenOCD/0.10.0-201601101000-dev/scripts"
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

            outtext = ck_out(args, cwd=openOCD_dir  )
            self.log(outtext)

        else:

            self.log("Platform '%s' not supported!"%platform.system())

        self.log("Platform code compilation finished!")