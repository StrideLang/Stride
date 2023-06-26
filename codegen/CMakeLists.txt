cmake_minimum_required(VERSION 3.13) 
project(StrideCodegen) 

########## Code generation

set(CODEGEN_SRC 
#  pythonproject.cpp
  src/astfunctions.cpp
  src/astquery.cpp
  src/astvalidation.cpp
  src/astruntime.cpp
  src/codevalidator.cpp
  src/coderesolver.cpp
  src/codeanalysis.cpp
  src/codequery.cpp
  src/strideframework.cpp
  src/stridelibrary.cpp
  src/stridesystem.cpp
  src/systemconfiguration.cpp
  src/languagesyntax.cpp
  src/toolmanager.cpp
)

set(CODEGEN_HEADERS
#  pythonproject.h
#  builder.h
  include/stride/codegen/astfunctions.hpp
  include/stride/codegen/astquery.hpp
  include/stride/codegen/astruntime.hpp
  include/stride/codegen/astvalidation.hpp
  include/stride/codegen/builder.hpp
  include/stride/codegen/codevalidator.hpp
  include/stride/codegen/coderesolver.hpp
  include/stride/codegen/codeanalysis.hpp
  include/stride/codegen/codequery.hpp
  include/stride/codegen/strideframework.hpp
  include/stride/codegen/stridelibrary.hpp
  include/stride/codegen/stridesystem.hpp
  include/stride/codegen/systemconfiguration.hpp
  include/stride/codegen/languagesyntax.hpp
  include/stride/codegen/codeentities.hpp
  include/stride/codegen/toolmanager.hpp
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

add_subdirectory(tests)

if (STRIDE_CODE_COVERAGE)
  message("Building code coverage for StrideCodegen")
  set_target_properties(StrideCodegen PROPERTIES COMPILE_FLAGS "-g -O0 -Wall -fprofile-arcs -ftest-coverage")
  target_link_libraries(StrideCodegen gcov)
endif()

target_include_directories(StrideCodegen PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")

if ("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
    target_link_libraries(StrideCodegen dl StrideParser)
else()
    target_link_libraries(StrideCodegen StrideParser)

endif()
