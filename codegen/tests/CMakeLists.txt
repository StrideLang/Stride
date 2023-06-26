cmake_minimum_required(VERSION 3.8)
project(StrideCodegen_tests)

option(STRIDEPARSER_BUILD_TESTS "Build and run unit test for the Stride parser" OFF)

# Get and build googletest
if (NOT gtest)
    message("Building googletest with StrideCodegen build")
    include(FetchContent)
    FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        release-1.12.0
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()


set(TEST_SOURCES main.cpp
  schema.cpp
  library.cpp
)


add_executable(stridecodegen_tests ${TEST_SOURCES})

set_target_properties(stridecodegen_tests PROPERTIES
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
)

add_test(NAME stridecodegen_tests COMMAND stridecodegen_tests)

target_link_libraries(stridecodegen_tests PUBLIC StrideParser StrideCodegen gtest)
target_compile_definitions(stridecodegen_tests PUBLIC
  "-DTESTS_SOURCE_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/data/\"")

if (STRIDECODEGEN_CODE_COVERAGE)
  set_target_properties(stridecodegen_tests PROPERTIES COMPILE_FLAGS "-fPIE -g -O0 -Wall -fprofile-arcs -ftest-coverage")
  target_link_libraries(stridecodegen_tests PUBLIC gcov)
endif()
