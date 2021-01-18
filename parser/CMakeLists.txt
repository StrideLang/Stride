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


include_directories(${CMAKE_CURRENT_SOURCE_DIR})

if (WIN32)

  # On Windows you need a lot of magic to get flex and bison working.
  # Download and install flex and bisno from:
  #  http://gnuwin32.sourceforge.net/packages/flex.htm
  # http://gnuwin32.sourceforge.net/packages/bison.htm
  # Bison is broken, must replace with: http://marin.jb.free.fr/bison/bison-2.4.1-modified.zip

  set(BISON_EXECUTABLE "C:/Program Files (x86)/GnuWin32/bin/bison.exe")
  set(FLEX_EXECUTABLE "C:/Program Files (x86)/GnuWin32/bin/flex.exe")
  set(FLEX_MyScanner_OUTPUTS ${CMAKE_CURRENT_BINARY_DIR}/lang_stride.lexer.cpp)
  set(BISON_MyParser_OUTPUTS ${CMAKE_CURRENT_BINARY_DIR}/lang_stride.parser.cpp)

  if (BISON_FOUND)
      add_custom_command(
        COMMENT "Generating BISON: -o${BISON_MyParser_OUTPUTS} ${CMAKE_CURRENT_SOURCE_DIR}/lang_stride.y"
              OUTPUT ${BISON_MyParser_OUTPUTS}
              COMMAND "${BISON_EXECUTABLE}"
              -d
              -o"${BISON_MyParser_OUTPUTS}"
              "${CMAKE_CURRENT_SOURCE_DIR}/lang_stride.y"
      )
  endif ()

  if (FLEX_FOUND)
      add_custom_command(
              OUTPUT ${FLEX_MyScanner_OUTPUTS}
              COMMAND ${FLEX_EXECUTABLE}
              -o"${FLEX_MyScanner_OUTPUTS}"
              "${CMAKE_CURRENT_SOURCE_DIR}/lang_stride.l"
              COMMENT "Generating lang_stride.lexer.cpp"
      )
  endif ()

  include_directories("C:/Program Files (x86)/GnuWin32/include")
else()
  # On Linux you can use the standard cmake scripts
  find_package(BISON)
  find_package(FLEX)
  BISON_TARGET(MyParser ${CMAKE_CURRENT_SOURCE_DIR}/lang_stride.y ${CMAKE_CURRENT_BINARY_DIR}/lang_stride.parser.cpp)
  FLEX_TARGET(MyScanner ${CMAKE_CURRENT_SOURCE_DIR}/lang_stride.l ${CMAKE_CURRENT_BINARY_DIR}/lang_stride.lexer.cpp)
  ADD_FLEX_BISON_DEPENDENCY(MyScanner MyParser)
endif()

add_library(StrideParser 
  ${PARSER_SRC} ${PARSER_HEADERS}
  ${BISON_MyParser_OUTPUTS}
  ${FLEX_MyScanner_OUTPUTS}
)

target_compile_options(StrideParser PRIVATE -fPIC)
