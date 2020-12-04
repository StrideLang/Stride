cmake_minimum_required(VERSION 3.13) 
project(StrideCompiler)  

set(CMAKE_CXX_STANDARD 11)

set(COMPILER_SRC
  main.cpp
  )

#set(COMPILER_HEADERS
#  )

find_package(Qt5 5.7.0 REQUIRED COMPONENTS
  Core 
)

#set(CMAKE_AUTOMOC ON)

add_executable(stridec ${COMPILER_SRC})

#target_include_directories(stridec PUBLIC ../parser)
target_link_libraries(stridec StrideCodegen StrideParser Qt5::Core)
