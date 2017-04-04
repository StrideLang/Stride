cmake_minimum_required(VERSION 3.5)

include(CMakeForceCompiler)

set(CXX11_FLAGS -std=gnu++11)

set(CMAKE_ASM_COMPILER arm-none-eabi-as)
enable_language(ASM)

CMAKE_FORCE_C_COMPILER(arm-none-eabi-gcc GNU)
CMAKE_FORCE_CXX_COMPILER(arm-none-eabi-g++ GNU)
set(COMMON_FLAGS "-mcpu=cortex-m7 -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16")

set(CMAKE_LINKER arm-none-eabi-g++)
set(LINK_FLAGS ${COMMON_FLAGS})
set(CMAKE_CXX_LINK_EXECUTABLE "<CMAKE_LINKER> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")

set(CMAKE_C_FLAGS "${COMMON_FLAGS} -Os -g3 -Wall -fmessage-length=0 -ffunction-sections -c -fmessage-length=0")
set(CMAKE_CXX_FLAGS "${COMMON_FLAGS} -Os -g3 -Wall -fmessage-length=0 -ffunction-sections -c -fmessage-length=0 ${CXX11_FLAGS}")

set(CMAKE_ASM_FLAGS ${COMMON_FLAGS})

set(STM_COMMON ${STRIDE_PLATFORM_ROOT}/STM32Cube_FW_F7_V1.3.0)

include_directories( Inc
${STM_COMMON}/Drivers/STM32F7xx_HAL_Driver/Inc
${STM_COMMON}/Drivers/STM32F7xx_HAL_Driver/Inc/Legacy
${STM_COMMON}/Drivers/CMSIS/Include
${STM_COMMON}/Drivers/CMSIS/Device/ST/STM32F7xx/Include
)

set(STM_PLATFORM
${STM_COMMON}/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal.c
${STM_COMMON}/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_cortex.c
${STM_COMMON}/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_dma.c
${STM_COMMON}/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_flash.c
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
${STM_COMMON}/Drivers/CMSIS/Device/ST/STM32F7xx/Source/Templates/gcc/startup_stm32f746xx.s
)

set_property(SOURCE ${STM_COMMON}/Drivers/CMSIS/Device/ST/STM32F7xx/Source/Templates/gcc/startup_stm32f746xx.s PROPERTY LANGUAGE C)

file(GLOB SOURCES
    "Src/*.cpp"
    "Src/*.c"
)

add_executable(app.elf ${SOURCES} ${STM_PLATFORM})

set_target_properties(app.elf PROPERTIES COMPILE_DEFINITIONS "USE_HAL_DRIVER;STM32F746xx;ARM_MATH_CM7")

add_definitions( "-D__weak=__attribute__((weak))" "-D__packed=__attribute__((__packed__))" )

link_directories(${STM_COMMON}/Drivers/CMSIS/Lib/GCC)

target_link_libraries(app.elf ${COMMON_FLAGS} -L${STM_COMMON}/Drivers/CMSIS/Lib/GCC -specs=nosys.specs -specs=nano.specs -TSTM32F746NGHx_FLASH.ld -Wl,-Map=output.map -Wl,--gc-sections -lm arm_cortexM7lfsp_math ${CXX11})
