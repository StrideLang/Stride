cmake_minimum_required(VERSION 3.13) 
project(StrideTests)  

set(CMAKE_CXX_STANDARD 11)

set(TESTS_SRC
  tst_parsertest.cpp
  buildtester.cpp
  )

set(TESTS_HEADERS
  buildtester.hpp
  )

find_package(Qt5 5.7.0 REQUIRED COMPONENTS
  Core Test
)

set(CMAKE_AUTOMOC ON)

add_executable(StrideTests ${TESTS_SRC} ${TESTS_HEADERS})

target_compile_definitions(StrideTests PUBLIC "-DBUILDPATH=\"${CMAKE_SOURCE_DIR}\"")
target_compile_definitions(StrideTests PUBLIC "-DSTRIDEROOT=\"${CMAKE_SOURCE_DIR}/strideroot\"")
target_compile_definitions(StrideTests PUBLIC "-DSTRIDEBIN=\"${CMAKE_CURRENT_BINARY_DIR}\"")
target_include_directories(StrideTests PUBLIC ../parser ../codegen)
target_link_libraries(StrideTests StrideParser StrideCodegen Qt5::Core Qt5::Test)
