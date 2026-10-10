cmake_minimum_required(VERSION 3.13) 
project(Stride)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(STRIDE_CODE_COVERAGE "Enable Code coverage for tests" OFF)
option(STRIDE_BUILD_JIT "Build Stride JIT compiler library" ON)
option(STRIDE_BUILD_EDITOR "Build Stride IDE / Editor" OFF)
option(STRIDE_BUILD_TESTS "Build Unit Tests" ON)

if(STRIDE_BUILD_TESTS)
    enable_testing()
endif()

if(NOT STRIDEROOT)
    set(STRIDEROOT "${CMAKE_CURRENT_SOURCE_DIR}/strideroot" CACHE PATH "Path to strideroot")
endif()

# Stride Core
add_subdirectory(strideparser)
add_subdirectory(strideutils)
add_subdirectory(codegen)

# StrideJit
if(STRIDE_BUILD_JIT)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/stridejit")
        add_subdirectory(stridejit)
        install(TARGETS StrideJit DESTINATION lib)
        file(GLOB STRIDEJIT_HEADERS "stridejit/include/*.h*" )
        install(FILES ${STRIDEJIT_HEADERS} DESTINATION include)
    else()
        message("stridejit directory not found. Not building StrideJit")
    endif()
endif()

# Unit tests
if(STRIDE_BUILD_TESTS)
    add_subdirectory(tests)
endif()

# Tools
add_subdirectory(stridelang)
add_subdirectory(stridemanager)
if(STRIDE_BUILD_EDITOR AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/editor")
    add_subdirectory(editor)
endif()

# Plugins
if(EXISTS "${CMAKE_SOURCE_DIR}/plugins/pufferfish")
  add_subdirectory(plugins/pufferfish)
endif()


# Install
set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/build")
install(TARGETS StrideParser StrideCodegen DESTINATION lib)
if(TARGET StrideIDE)
    install(TARGETS stridec stridemngr StrideIDE DESTINATION bin)
else()
    install(TARGETS stridec stridemngr DESTINATION bin)
endif()

file(GLOB STRIDEPARSER_HEADERS "strideparser/include/*.h*" )
install(FILES ${STRIDEPARSER_HEADERS} DESTINATION include)
file(GLOB STRIDECODEGEN_HEADERS "strideparser/include/*.h*" )
install(FILES ${STRIDECODEGEN_HEADERS} DESTINATION include)


