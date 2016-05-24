cmake_minimum_required(VERSION 3.5)

INCLUDE(CMakeForceCompiler)

SET(CMAKE_SYSTEM_NAME Generic)
SET(CMAKE_SYSTEM_VERSION 1)
 
# specify the cross compiler
#sudo add-apt-repository ppa:team-gcc-arm-embedded/ppa
#sudo apt-get install gcc-arm-embedded

#CMAKE_FORCE_C_COMPILER(arm-linux-gnueabihf-gcc-5 GNU)
CMAKE_FORCE_C_COMPILER(arm-none-eabi-gcc GNU)
CMAKE_FORCE_CXX_COMPILER(arm-none-eabi-g++ GNU)
 
#SET(COMMON_FLAGS "-mcpu=cortex-m3 -mthumb -mthumb-interwork -msoft-float -ffunction-sections -fdata-sections -g -fno-common -fmessage-length=0")
#SET(COMMON_FLAGS -mcpu=cortex-m7 -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16 -std=c++11)
SET(COMMON_FLAGS -mcpu=cortex-m7 -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16)
SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS}  "-std=gnu++0x")

# -MMD -MP -MF"Drivers/STM32F7xx_HAL_Driver/stm32f7xx_hal_tim_ex.d" -MT"Drivers/STM32F7xx_HAL_Driver/stm32f7xx_hal_tim_ex.o"


set(STM_COMMON ../STM32Cube_FW_F7_V1.3.0)

include_directories( Inc
${STM_COMMON}/Drivers/STM32F7xx_HAL_Driver/Inc
${STM_COMMON}/Drivers/STM32F7xx_HAL_Driver/Inc/Legacy
${STM_COMMON}/Drivers/CMSIS/Include
${STM_COMMON}/Drivers/CMSIS/Device/ST/STM32F7xx/Include
)

#arm-none-eabi-objcopy -O binary ../blinky-build/blinky.elf blinky.bin
#arm-none-eabi-objcopy -O ihex ../blinky-build/blinky.elf blinky.hex
#../stlink/st-flash write blinky.bin 0x8000000

#-mcpu=cortex-m7 -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16 -D__weak="__attribute__((weak))" -D__packed="__attribute__((__packed__))" -DUSE_HAL_DRIVER -DSTM32F746xx -I../Inc -IC:/Users/Pacifist/STM32Cube/Repository/STM32Cube_FW_F7_V1.3.0/Drivers/STM32F7xx_HAL_Driver/Inc -IC:/Users/Pacifist/STM32Cube/Repository/STM32Cube_FW_F7_V1.3.0/Drivers/STM32F7xx_HAL_Driver/Inc/Legacy -IC:/Users/Pacifist/STM32Cube/Repository/STM32Cube_FW_F7_V1.3.0/Drivers/CMSIS/Include -IC:/Users/Pacifist/STM32Cube/Repository/STM32Cube_FW_F7_V1.3.0/Drivers/CMSIS/Device/ST/STM32F7xx/Include -I../Inc -Os -g3 -Wall -fmessage-length=0 -ffunction-sections -c -fmessage-length=0

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

add_executable(blinky.elf Src/main.cpp Src/gpio.c Src/stm32f7xx_hal_msp.c Src/stm32f7xx_it.c ${STM_PLATFORM})

target_compile_options(blinky.elf PRIVATE ${COMMON_FLAGS} -DUSE_HAL_DRIVER -DSTM32F746xx  -Os -g3 -Wall -fmessage-length=0 -ffunction-sections -c -fmessage-length=0)

add_definitions("-D__weak=__attribute__((weak))" "-D__packed=__attribute__((__packed__))" -DUSE_HAL_DRIVER -DSTM32F746xx )

target_link_libraries(blinky.elf ${COMMON_FLAGS} -specs=nosys.specs -specs=nano.specs -u _printf_float -T../blinky/STM32F746NGHx_FLASH.ld -Wl,-Map=output.map -Wl,--gc-sections -lm)

