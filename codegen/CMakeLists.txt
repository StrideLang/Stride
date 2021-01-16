cmake_minimum_required(VERSION 3.13) 
project(StrideCodegen) 

set(CMAKE_CXX_STANDARD 11)

set(CODEGEN_SRC 
  pythonproject.cpp
  codevalidator.cpp
  coderesolver.cpp
  strideframework.cpp
  stridelibrary.cpp
  stridesystem.cpp
  systemconfiguration.cpp
  languagesyntax.cpp
  toolmanager.cpp
)

set(CODEGEN_HEADERS
  pythonproject.h
  codevalidator.h
  coderesolver.h
  builder.h
  strideframework.hpp
  stridelibrary.hpp
  stridesystem.hpp
  systemconfiguration.hpp
  languagesyntax.hpp
  codeentities.hpp
  toolmanager.hpp
)

find_package(Qt5 5.7.0 REQUIRED COMPONENTS
    Core
)

set(CMAKE_AUTOMOC ON)

add_library(StrideCodegen ${CODEGEN_SRC} ${CODEGEN_HEADERS})

if (STRIDE_CODE_COVERAGE)
  message("Building code coverage for StrideCodegen")
  set_target_properties(StrideCodegen PROPERTIES COMPILE_FLAGS "-g -O0 -Wall -fprofile-arcs -ftest-coverage")
  target_link_libraries(StrideCodegen gcov)
endif()

target_include_directories(StrideCodegen PUBLIC ../parser .)
target_link_libraries(StrideCodegen StrideParser Qt5::Core)
