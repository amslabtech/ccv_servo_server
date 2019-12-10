cmake_minimum_required(VERSION 2.8)
project(ccv_servo_control)

set(CMAKE_CXX_FLAGS "-O2 -std=c++11 -Wall")

set(COMMON_SOURCES src/ccv_servo_control.cpp)
set(DXL_INC_DIR /home/parallels/dev/DynamixelSDK/c++/include/dynamixel_sdk /home/pi/dev/DynamixelSDK/c++/include/dynamixel_sdk)
set(DXL_WRAPPER_INC_DIR /home/parallels/dev/DynamixelWrapper/include /home/pi/dev/DynamixelWrapper/include)
set(DXL_WRAPPER /home/parallels/dev/DynamixelWrapper/build)

include_directories(${DXL_INC_DIR} ${DXL_WRAPPER_INC_DIR} ./src ./include)
link_directories(${DXL_WRAPPER})


add_executable(ccv_servo_control src/ccv_servo_control.cpp)
add_executable(ccv_remote src/ccv_remote.cpp)
add_executable(ccv_remote_control src/ccv_remote_control.cpp)

 target_link_libraries(ccv_servo_control mosquitto dxl_wrapper dxl_x64_cpp rt) # RegularPC
#target_link_libraries(ccv_servo_control mosquitto dxl_wrapper dxl_sbc_cpp rt)	# Raspberry Pi
target_link_libraries(ccv_remote mosquitto)
target_link_libraries(ccv_remote_control mosquitto)

INSTALL (
	TARGETS ccv_servo_control
	RUNTIME DESTINATION /usr/local/bin
)


