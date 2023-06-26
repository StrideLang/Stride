cmake_minimum_required(VERSION 3.13) 
project(StrideTests)  

set(CMAKE_CXX_STANDARD 17)

set(TESTS_SRC
  tst_parsertest.cpp
  buildtester.cpp
#  ${CODEGEN_SRC}
  )

set(TESTS_HEADERS
  buildtester.hpp
#  ${CODEGEN_HEADERS}
  )

find_package(Qt6 6.5.0 REQUIRED COMPONENTS
  Core Test
)

qt_standard_project_setup()
#set(CMAKE_AUTOMOC ON)

add_executable(StrideTests ${TESTS_SRC} ${TESTS_HEADERS})

if (STRIDE_CODE_COVERAGE)
  message("Building code coverage for tests")
  set_target_properties(StrideTests PROPERTIES COMPILE_FLAGS "-g -O0 -Wall -fprofile-arcs -ftest-coverage")
  target_link_libraries(StrideTests gcov)
endif()

# To generate coverage reports:
# gcov tst_parser.cpp
# lcov --capture --directory . --output-file main_coverage.info
# genhtml main_coverage.info --output-directory out

target_compile_definitions(StrideTests PUBLIC "-DBUILDPATH=\"${CMAKE_SOURCE_DIR}\"")
target_compile_definitions(StrideTests PUBLIC "-DSTRIDEROOT=\"${CMAKE_SOURCE_DIR}/strideroot\"")
target_compile_definitions(StrideTests PUBLIC "-DSTRIDEBIN=\"${CMAKE_CURRENT_BINARY_DIR}\"")
target_link_libraries(StrideTests StrideParser StrideCodegen Qt6::Core Qt6::Test)

set_property(TARGET StrideTests PROPERTY CXX_STANDARD 17)
set_property(TARGET StrideTests PROPERTY CXX_STANDARD_REQUIRED ON)

enable_testing()
add_test(StrideTests ${CMAKE_CURRENT_BINARY_DIR}/testing)
