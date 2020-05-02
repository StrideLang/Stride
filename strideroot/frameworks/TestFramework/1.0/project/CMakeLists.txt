cmake_minimum_required(VERSION 3.9)
set(APP_NAME app)
project(${APP_NAME})

include(config.cmake)

add_executable(${APP_NAME} template.cpp ${ADDITIONAL_SOURCES})
target_include_directories(${APP_NAME} PUBLIC ${INCLUDE_DIRS})
target_link_libraries(${APP_NAME} ${LINK_LIBRARIES})
add_definitions(${ADDITIONAL_DEFINES})

