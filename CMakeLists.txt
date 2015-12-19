project(gpsynth)
cmake_minimum_required(VERSION 2.8)

SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-std=c++11 -gdwarf-3")
SET(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-std=c++11 -gdwarf-3")

aux_source_directory(. SRC_LIST)
#link_directories(/home/amir/git/amirhammad/gpsynth/libgamepad)
set(GAMEPAD_PATH ${CMAKE_SOURCE_DIR}/libgamepad)
link_directories(${GAMEPAD_PATH})
add_executable(${PROJECT_NAME} ${SRC_LIST})
add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD COMMAND make -C ${GAMEPAD_PATH})
target_link_libraries(${PROJECT_NAME} gamepad jack)

