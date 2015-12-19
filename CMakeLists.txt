project(gpsynth)
cmake_minimum_required(VERSION 2.8)

SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-std=c++11 -gdwarf-3")
SET(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-std=c++11 -gdwarf-3")

aux_source_directory(. SRC_LIST)
#include_directories(.)
link_directories(/home/amir/git/amirhammad/gpsynth/libgamepad)
add_executable(${PROJECT_NAME} ${SRC_LIST})
target_link_libraries(${PROJECT_NAME} gamepad jack)

