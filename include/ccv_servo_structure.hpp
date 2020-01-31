//
//	imu_structure.hpp
//
#ifndef _CCV_SERVO_STRUCTURE_HPP_
#define _CCV_SERVO_STRUCTURE_HPP_

#include <iostream>
#include <iomanip>
#include <time.h>
#include <sys/time.h>

namespace servo {
	enum { ROLL=0, FORE, REAR, STRR, STRL, NSERVOS };
//	const char* name_talker		= "servo_data_talker";
//	const char* name_listener	= "servo_command_listener";
	const char* topic_read		= "servo_read";
	const char* topic_write		= "servo_write";
	const char* password		= "mqtt";
};


struct CcvServoStructure {
	int32_t id;
//	struct timeval ts;

	// present/commanding positions
	// position[0] ... ROLL
	// position[1] ... PITCH FORE
	// position[2] ... PITCH REAR
	// position[3] ... STEER R
	// position[4] ... STEER L

	float  command_position[servo::NSERVOS];	// servo positions
	float  present_position[servo::NSERVOS];	// servo positions

	void print_command() {
		std::cout
		<< std::setw( 6) << id
//		<< std::setw(12) << ts.tv_sec
//		<< std::setw(10) << ts.tv_usec
		<< std::setw(12) << command_position[0]
		<< std::setw(12) << command_position[1]
		<< std::setw(12) << command_position[2]
		<< std::setw(12) << command_position[3]
		<< std::setw(12) << command_position[4]
		<< std::endl;
	}
	void print_read() {
		std::cout
		<< std::setw( 6) << id
//		<< std::setw(12) << ts.tv_sec
//		<< std::setw(10) << ts.tv_usec
		<< std::setw(12) << present_position[0]
		<< std::setw(12) << present_position[1]
		<< std::setw(12) << present_position[2]
		<< std::setw(12) << present_position[3]
		<< std::setw(12) << present_position[4]
		<< std::endl;
	}
};



#endif	// _CCV_SERVO_STRUCTURE_HPP_
