
# Pull in PICO SDK (must be before project)
include(pico_sdk_import.cmake)
set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")

project(${STRIDE_PROJECT_NAME} C CXX ASM)
cmake_minimum_required(VERSION 3.12)


#  PICO SDK location was not specified.  Please set PICO_SDK_PATH or set
#  PICO_SDK_FETCH_FROM_GIT to on to fetch from git.

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the SDK
pico_sdk_init()

include(example_auto_set_url.cmake)

add_executable(${STRIDE_PROJECT_NAME}
        stride_code.cpp
        )

# Pull in our pico_stdlib which pulls in commonly used features
target_link_libraries(${STRIDE_PROJECT_NAME} pico_stdlib)

# create map/bin/hex file etc.
pico_add_extra_outputs(${STRIDE_PROJECT_NAME})

# add url via pico_set_program_url
example_auto_set_url(${STRIDE_PROJECT_NAME})
