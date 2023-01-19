cmake_minimum_required(VERSION 3.13) 
project(Stride)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(STRIDE_CODE_COVERAGE "Enable Code coverage for tests" OFF)
option(STRIDE_BUILD_JIT "Build Stride JIT compiler library" OFF)

add_subdirectory(strideparser)
add_subdirectory(codegen)
add_subdirectory(tests)
add_subdirectory(compiler)
add_subdirectory(stridemanager)
add_subdirectory(editor)

if(STRIDE_BUILD_JIT)
    add_subdirectory(stridejit)
endif()

if(EXISTS "${CMAKE_SOURCE_DIR}/plugins/pufferfishplugin")
  add_subdirectory(plugins/pufferfishplugin)
endif()
