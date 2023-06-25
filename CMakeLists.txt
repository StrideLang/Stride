cmake_minimum_required(VERSION 3.13) 
project(Stride)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(STRIDE_CODE_COVERAGE "Enable Code coverage for tests" OFF)
option(STRIDE_BUILD_JIT "Build Stride JIT compiler library" ON)

# Stride Core
add_subdirectory(strideparser)
add_subdirectory(codegen)

# Unit tests
add_subdirectory(tests)

# Tools
add_subdirectory(compiler)
add_subdirectory(stridemanager)
add_subdirectory(editor)

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

# Plugins
if(EXISTS "${CMAKE_SOURCE_DIR}/plugins/pufferfishplugin")
  add_subdirectory(plugins/pufferfishplugin)
endif()


# Install
set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/build")
install(TARGETS StrideParser StrideCodegen  DESTINATION lib)
install(TARGETS stridec stridemngr StrideIDE DESTINATION bin)

file(GLOB STRIDEPARSER_HEADERS "strideparser/include/*.h*" )
install(FILES ${STRIDEPARSER_HEADERS} DESTINATION include)
file(GLOB STRIDECODEGEN_HEADERS "strideparser/include/*.h*" )
install(FILES ${STRIDECODEGEN_HEADERS} DESTINATION include)


