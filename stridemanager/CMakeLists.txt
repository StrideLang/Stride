cmake_minimum_required(VERSION 3.13) 
project(StrideManager)  

set(CMAKE_CXX_STANDARD 11)

set(MANAGER_SRC
  main.cpp
  )

#set(COMPILER_HEADERS
#  )

find_package(Qt5 5.7.0 REQUIRED COMPONENTS
  Core 
)

#set(CMAKE_AUTOMOC ON)

add_executable(stridemngr ${MANAGER_SRC})
target_link_libraries(stridemngr StrideCodegen StrideParser Qt5::Core)
