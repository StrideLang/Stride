cmake_minimum_required(VERSION 3.13) 
project(Stride)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(STRIDE_CODE_COVERAGE "Enable Code coverage for tests" OFF)

add_subdirectory(parser)
add_subdirectory(codegen)
add_subdirectory(tests)
add_subdirectory(compiler)
add_subdirectory(stridemanager)
add_subdirectory(editor)

if(EXISTS "${CMAKE_SOURCE_DIR}/plugins/pufferfishplugin")
  add_subdirectory(plugins/pufferfishplugin)
endif()

if(EXISTS "${CMAKE_SOURCE_DIR}/plugins/configgen")
  add_subdirectory(plugins/configgen)
endif()
