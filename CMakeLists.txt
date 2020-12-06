cmake_minimum_required(VERSION 3.13) 
project(Stride)           

add_subdirectory(parser)
add_subdirectory(codegen)
add_subdirectory(tests)
add_subdirectory(compiler)
add_subdirectory(stridemanager)
add_subdirectory(editor)

if(EXISTS "${CMAKE_SOURCE_DIR}/plugins/pufferfishplugin")
  add_subdirectory(plugins/pufferfishplugin)
endif()
