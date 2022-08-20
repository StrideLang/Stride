cmake_minimum_required(VERSION 3.13) 
project(StrideCompiler)  

set(COMPILER_SRC
  main.cpp
  )

#set(COMPILER_HEADERS
#  )

find_package(Qt6 6.3.0 REQUIRED COMPONENTS
  Core 
)

qt_standard_project_setup()
#set(CMAKE_AUTOMOC ON)

add_executable(stridec ${COMPILER_SRC})

#target_include_directories(stridec PUBLIC ../parser)
target_link_libraries(stridec StrideCodegen StrideParser Qt6::Core)
