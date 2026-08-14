cmake_minimum_required(VERSION 3.13) 
project(StrideManager)  

set(CMAKE_CXX_STANDARD 11)

set(MANAGER_SRC
  main.cpp
  )

#set(COMPILER_HEADERS
#  )

find_package(Qt6 6.5.0 REQUIRED COMPONENTS
  Core 
)
qt_standard_project_setup()

#set(CMAKE_AUTOMOC ON)

add_executable(stridemngr ${MANAGER_SRC})
target_link_libraries(stridemngr StrideCodegen StrideParser Qt6::Core)
