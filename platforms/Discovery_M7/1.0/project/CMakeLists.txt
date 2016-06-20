cmake_minimum_required(VERSION 3.5)

INCLUDE(CMakeForceCompiler)

#SET(CMAKE_SYSTEM_NAME Generic)
#SET(CMAKE_SYSTEM_VERSION 1)

# specify the cross compiler
#sudo add-apt-repository ppa:team-gcc-arm-embedded/ppa
#sudo apt-get install gcc-arm-embedded

#set(CXX11 -std=c++11)

set(CMAKE_ASM_COMPILER arm-none-eabi-as) # Has to be before enabling ASM language
enable_language(ASM)

CMAKE_FORCE_C_COMPILER(arm-none-eabi-gcc GNU)
CMAKE_FORCE_CXX_COMPILER(arm-none-eabi-g++ GNU)
#SET(CMAKE_C_LINK_EXECUTABLE arm-none-eabi-gcc)
SET(COMMON_FLAGS "-mcpu=cortex-m7 -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16")

SET(CMAKE_LINKER arm-none-eabi-g++)
set(LINK_FLAGS ${COMMON_FLAGS})
set(CMAKE_CXX_LINK_EXECUTABLE "<CMAKE_LINKER> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")

#list(APPEND CMAKE_CXX_FLAGS "${COMMON_FLAGS} -DUSE_HAL_DRIVER -DSTM32F746xx  -Os -g3 -Wall -fmessage-length=0 -ffunction-sections -c -fmessage-length=0 ${CXX11}")
#list(APPEND CMAKE_C_FLAGS "${COMMON_FLAGS} -DUSE_HAL_DRIVER -DSTM32F746xx  -Os -g3 -Wall -fmessage-length=0 -ffunction-sections -c -fmessage-length=0")
#list(APPEND CMAKE_EXE_LINKER_FLAGS " -specs=nosys.specs -specs=nano.specs -u _printf_float -T../app/STM32F746NGHx_FLASH.ld -Wl,-Map=output.map -Wl,--gc-sections -lm")


SET(CMAKE_C_FLAGS "${COMMON_FLAGS} -Os -g3 -Wall -fmessage-length=0 -ffunction-sections -c -fmessage-length=0")
SET(CMAKE_CXX_FLAGS "${COMMON_FLAGS} -Os -g3 -Wall -fmessage-length=0 -ffunction-sections -c -fmessage-length=0 ${CXX11}")

#SET(CMAKE_ASM_FLAGS "-mthumb -mcpu=cortex-m7 -mfpu=fpv5-sp-d16 -x assembler-with-cpp" CACHE INTERNAL "asm compiler flags")
#SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--gc-sections -mthumb -mcpu=cortex-m7 -mfpu=fpv5-sp-d16 -mfloat-abi=softfp -mabi=aapcs")
#SET(CMAKE_MODULE_LINKER_FLAGS "-mthumb -mcpu=cortex-m7 -mfpu=fpv5-sp-d16 -mfloat-abi=softfp -mabi=aapcs")
#SET(CMAKE_SHARED_LINKER_FLAGS "-mthumb -mcpu=cortex-m7 -mfpu=fpv5-sp-d16 -mfloat-abi=softfp -mabi=aapcs")

SET(CMAKE_ASM_FLAGS ${COMMON_FLAGS})

set(STM_COMMON ${STRIDE_PLATFORM_ROOT}/STM32Cube_FW_F7_V1.3.0)

include_directories( Inc
${STM_COMMON}/Drivers/STM32F7xx_HAL_Driver/Inc
${STM_COMMON}/Drivers/STM32F7xx_HAL_Driver/Inc/Legacy
${STM_COMMON}/Drivers/CMSIS/Include
${STM_COMMON}/Drivers/CMSIS/Device/ST/STM32F7xx/Include
)

#arm-none-eabi-objcopy -O binary ../app-build/app.elf app.bin
#arm-none-eabi-objcopy -O ihex ../app-build/app.elf app.hex
#../stlink/st-flash write app.bin 0x8000000


set(STM_PLATFORM
${STM_COMMON}/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal.c
${STM_COMMON}/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_cortex.c
${STM_COMMON}/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_dma.c
${STM_COMMON}/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_flash.c
#${STM_COMMON}/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_ex.c
${STM_COMMON}/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_gpio.c
${STM_COMMON}/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_pwr.c
${STM_COMMON}/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_pwr_ex.c
${STM_COMMON}/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_rcc.c
${STM_COMMON}/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_rcc_ex.c
${STM_COMMON}/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_tim.c
${STM_COMMON}/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_tim_ex.c
${STM_COMMON}/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_sai.c
${STM_COMMON}/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_sai_ex.c
${STM_COMMON}/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_i2c.c
${STM_COMMON}/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_i2c_ex.c
${STM_COMMON}/Drivers/CMSIS/Device/ST/STM32F7xx/Source/Templates/system_stm32f7xx.c
#${STM_COMMON}/Drivers/CMSIS/system_stm32f7xx.c
${STM_COMMON}/Drivers/CMSIS/Device/ST/STM32F7xx/Source/Templates/gcc/startup_stm32f746xx.s
)

#set_source_file_properties(${STM_COMMON}/Drivers/CMSIS/Device/ST/STM32F7xx/Source/Templates/gcc/startup_stm32f746xx.s
#PROPERTIES LANGUAGE ASM)

set_property(SOURCE ${STM_COMMON}/Drivers/CMSIS/Device/ST/STM32F7xx/Source/Templates/gcc/startup_stm32f746xx.s PROPERTY LANGUAGE C)

file(GLOB SOURCES
    "Src/*.cpp"
    "Src/*.c"
)

add_executable(app.elf ${SOURCES} ${STM_PLATFORM})

SET_TARGET_PROPERTIES(app.elf PROPERTIES COMPILE_DEFINITIONS "USE_HAL_DRIVER;STM32F746xx;ARM_MATH_CM7")

#set_target_properties(app.elf PROPERTIES COMPILE_FLAGS "${COMMON_FLAGS} -Os -g3 -Wall -fmessage-length=0 -ffunction-sections -c -fmessage-length=0 ${CXX11}")
#target_compile_options(app.elf PRIVATE ${COMMON_FLAGS} )

add_definitions( "-D__weak=__attribute__((weak))" "-D__packed=__attribute__((__packed__))" )

link_directories(${STM_COMMON}/Drivers/CMSIS/Lib/GCC)

target_link_libraries(app.elf ${COMMON_FLAGS} -L${STM_COMMON}/Drivers/CMSIS/Lib/GCC -specs=nosys.specs -specs=nano.specs -TSTM32F746NGHx_FLASH.ld -Wl,-Map=output.map -Wl,--gc-sections -lm arm_cortexM7lfsp_math ${CXX11})
