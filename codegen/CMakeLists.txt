cmake_minimum_required(VERSION 3.13) 
project(StrideCodegen) 

########## Code generation

set(CODEGEN_SRC 
#  pythonproject.cpp
  codevalidator.cpp
  coderesolver.cpp
  codeanalysis.cpp
  codequery.cpp
  strideframework.cpp
  stridesystem.cpp
  systemconfiguration.cpp
  languagesyntax.cpp
  toolmanager.cpp
)

set(CODEGEN_HEADERS
#  pythonproject.h
  codevalidator.h
  coderesolver.h
  codeanalysis.hpp
  codequery.hpp
  builder.h
  strideframework.hpp
  stridesystem.hpp
  systemconfiguration.hpp
  languagesyntax.hpp
  codeentities.hpp
  toolmanager.hpp
)

#find_package(Qt5 5.7.0 REQUIRED COMPONENTS
#    Core
#)
if ("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
    add_definitions(-DQ_OS_LINUX)
elseif("${CMAKE_SYSTEM_NAME}" MATCHES "Windows")
    add_definitions(-DQ_OS_WINDOWS)
elseif("${CMAKE_SYSTEM_NAME}" MATCHES "Darwin")
    add_definitions(-DQ_OS_MACOS)
endif()

#set(CMAKE_AUTOMOC ON)

add_library(StrideCodegen ${CODEGEN_SRC} ${CODEGEN_HEADERS})

set_property(TARGET StrideCodegen PROPERTY POSITION_INDEPENDENT_CODE ON) #-fPIC
set_property(TARGET StrideCodegen PROPERTY CXX_STANDARD 17)
set_property(TARGET StrideCodegen PROPERTY CXX_STANDARD_REQUIRED ON)

if (STRIDE_CODE_COVERAGE)
  message("Building code coverage for StrideCodegen")
  set_target_properties(StrideCodegen PROPERTIES COMPILE_FLAGS "-g -O0 -Wall -fprofile-arcs -ftest-coverage")
  target_link_libraries(StrideCodegen gcov)
endif()

target_include_directories(StrideCodegen PUBLIC ../parser .)

if ("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
    target_link_libraries(StrideCodegen dl StrideParser)
else()
    target_link_libraries(StrideCodegen StrideParser)

endif()
