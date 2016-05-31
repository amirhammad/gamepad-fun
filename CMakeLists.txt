project(throttle2pwm)
cmake_minimum_required(VERSION 2.8)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -gdwarf-3")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c++11 -gdwarf-3")

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} gamepad)

install(
	TARGETS ${PROJECT_NAME}
	RUNTIME DESTINATION bin
	)

