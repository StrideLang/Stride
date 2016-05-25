cmake_minimum_required(VERSION 3.5)

INCLUDE(CMakeForceCompiler)

#SET(CMAKE_SYSTEM_NAME Generic)
#SET(CMAKE_SYSTEM_VERSION 1)
 
# specify the cross compiler
#sudo add-apt-repository ppa:team-gcc-arm-embedded/ppa
#sudo apt-get install gcc-arm-embedded

set(CXX11 -std=gnu++0x)

enable_language(ASM)

CMAKE_FORCE_C_COMPILER(arm-none-eabi-gcc GNU)
CMAKE_FORCE_CXX_COMPILER(arm-none-eabi-g++ GNU)
set (CMAKE_ASM_COMPILER arm-none-eabi-as)
 
SET(COMMON_FLAGS "-mcpu=cortex-m7 -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16")
#list(APPEND CMAKE_CXX_FLAGS "${COMMON_FLAGS} -DUSE_HAL_DRIVER -DSTM32F746xx  -Os -g3 -Wall -fmessage-length=0 -ffunction-sections -c -fmessage-length=0 ${CXX11}")

#list(APPEND CMAKE_C_FLAGS "${COMMON_FLAGS} -DUSE_HAL_DRIVER -DSTM32F746xx  -Os -g3 -Wall -fmessage-length=0 -ffunction-sections -c -fmessage-length=0")

#list(APPEND CMAKE_EXE_LINKER_FLAGS " -specs=nosys.specs -specs=nano.specs -u _printf_float -T../blinky/STM32F746NGHx_FLASH.ld -Wl,-Map=output.map -Wl,--gc-sections -lm")


SET(CMAKE_C_FLAGS "-mthumb -fno-builtin -mcpu=cortex-m7 -mfpu=fpv5-sp-d16 -mfloat-abi=softfp -Wall -std=gnu99 -ffunction-sections -fdata-sections -fomit-frame-pointer -mabi=aapcs -fno-unroll-loops -ffast-math -ftree-vectorize" CACHE INTERNAL "c compiler flags")
SET(CMAKE_CXX_FLAGS "-mthumb -fno-builtin -mcpu=cortex-m7 -mfpu=fpv5-sp-d16 -mfloat-abi=softfp -Wall -std=c++11 -ffunction-sections -fdata-sections -fomit-frame-pointer -mabi=aapcs -fno-unroll-loops -ffast-math -ftree-vectorize" CACHE INTERNAL "cxx compiler flags")
SET(CMAKE_ASM_FLAGS "-mthumb -mcpu=cortex-m7 -mfpu=fpv5-sp-d16 -mfloat-abi=softfp -x assembler-with-cpp" CACHE INTERNAL "asm compiler flags")

#SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--gc-sections -mthumb -mcpu=cortex-m7 -mfpu=fpv5-sp-d16 -mfloat-abi=softfp -mabi=aapcs" CACHE INTERNAL "executable linker flags")
#SET(CMAKE_MODULE_LINKER_FLAGS "-mthumb -mcpu=cortex-m7 -mfpu=fpv5-sp-d16 -mfloat-abi=softfp -mabi=aapcs" CACHE INTERNAL "module linker flags")
#SET(CMAKE_SHARED_LINKER_FLAGS "-mthumb -mcpu=cortex-m7 -mfpu=fpv5-sp-d16 -mfloat-abi=softfp -mabi=aapcs" CACHE INTERNAL "shared linker flags")


SET(CMAKE_ASM_FLAGS ${COMMON_FLAGS})


set(STM_COMMON ${STRIDE_PLATFORM_ROOT}/STM32Cube_FW_F7_V1.3.0)

include_directories( Inc
${STM_COMMON}/Drivers/STM32F7xx_HAL_Driver/Inc
${STM_COMMON}/Drivers/STM32F7xx_HAL_Driver/Inc/Legacy
${STM_COMMON}/Drivers/CMSIS/Include
${STM_COMMON}/Drivers/CMSIS/Device/ST/STM32F7xx/Include
)

#arm-none-eabi-objcopy -O binary ../blinky-build/blinky.elf blinky.bin
#arm-none-eabi-objcopy -O ihex ../blinky-build/blinky.elf blinky.hex
#../stlink/st-flash write blinky.bin 0x8000000


set(STM_PLATFORM 
${STM_COMMON}/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal.c
${STM_COMMON}/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_cortex.c
${STM_COMMON}/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_dma.c
${STM_COMMON}/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_flash.c
#${STM_COMMON}/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_ex.c
${STM_COMMON}/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_gpio.c
#${STM_COMMON}/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_power.c
#${STM_COMMON}/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_power_ex.c
${STM_COMMON}/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_rcc.c
${STM_COMMON}/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_rcc_ex.c
${STM_COMMON}/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_tim.c
${STM_COMMON}/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_tim_ex.c
${STM_COMMON}/Drivers/CMSIS/Device/ST/STM32F7xx/Source/Templates/system_stm32f7xx.c
#${STM_COMMON}/Drivers/CMSIS/system_stm32f7xx.c
${STM_COMMON}/Drivers/CMSIS/Device/ST/STM32F7xx/Source/Templates/gcc/startup_stm32f746xx.s
)

set_property(SOURCE ${STM_COMMON}/Drivers/CMSIS/Device/ST/STM32F7xx/Source/Templates/gcc/startup_stm32f746xx.s PROPERTY LANGUAGE C)

add_executable(blinky.elf Src/main.cpp Src/gpio.c Src/stm32f7xx_hal_msp.c Src/stm32f7xx_it.c ${STM_PLATFORM})

SET_TARGET_PROPERTIES(blinky.elf PROPERTIES COMPILE_DEFINITIONS "USE_HAL_DRIVER;STM32F746xx")

#set_target_properties(blinky.elf PROPERTIES COMPILE_FLAGS "${COMMON_FLAGS} -Os -g3 -Wall -fmessage-length=0 -ffunction-sections -c -fmessage-length=0 ${CXX11}")
#target_compile_options(blinky.elf PRIVATE ${COMMON_FLAGS} )

#add_definitions( "-D__weak=__attribute__((weak))" "-D__packed=__attribute__((__packed__))" )

target_link_libraries(blinky.elf -specs=nosys.specs -specs=nano.specs -u _printf_float -TSTM32F746NGHx_FLASH.ld -Wl,-Map=output.map -Wl,--gc-sections -lm ${CXX11})


