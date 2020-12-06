cmake_minimum_required(VERSION 3.13) 
project(StrideParser) 

set(CMAKE_CXX_STANDARD 11)

set(PARSER_SRC ast.cpp 
  streamnode.cpp 
  valuenode.cpp
  bundlenode.cpp
  propertynode.cpp
  functionnode.cpp
  expressionnode.cpp 
  listnode.cpp
  importnode.cpp
  rangenode.cpp
  langerror.cpp
  keywordnode.cpp
  declarationnode.cpp
  blocknode.cpp
  scopenode.cpp
  platformnode.cpp
  portpropertynode.cpp
)

set(PARSER_HEADERS
  ast.h
  streamnode.h
  valuenode.h
  bundlenode.h
  propertynode.h
  functionnode.h
  expressionnode.h
  listnode.h
  importnode.h
  rangenode.h
  langerror.h
  strideparser.h
  declarationnode.h
  blocknode.h
  scopenode.h
  keywordnode.h
  platformnode.h
  portpropertynode.h
)

find_package(BISON)
find_package(FLEX)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

BISON_TARGET(MyParser lang_stride.y ${CMAKE_CURRENT_BINARY_DIR}/lang_stride.parser.cpp)
FLEX_TARGET(MyScanner lang_stride.l ${CMAKE_CURRENT_BINARY_DIR}/lang_stride.lexer.cpp)
ADD_FLEX_BISON_DEPENDENCY(MyScanner MyParser)

add_library(StrideParser 
  ${PARSER_SRC} ${PARSER_HEADERS}
  ${BISON_MyParser_OUTPUTS}
  ${FLEX_MyScanner_OUTPUTS}
)

target_compile_options(StrideParser PRIVATE -fPIC)
