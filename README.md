# servo_server サーボシステムへのMQTTゲートウェイ
MQTT Service for servo system: publisher and subscriber



## サーボシステムへのMQTTゲートウェイ
- MQTTサブスクライバを起動し，servo_writeトピックに受けたデータを，サーボシステムに対して出力する．
- MQTTパプリッシャからサーボシステムのカレントデータを servo_read　トピックへ発信する．


```
//
//	imu_structure.hpp
//
#ifndef _SERVO_STRUCTURE_HPP_
#define _SERVO_STRUCTURE_HPP_

#include <iostream>
#include <iomanip>
#include <time.h>
#include <sys/time.h>

struct ServoStructure {
	int32_t id;
//	struct timeval ts;

	// present/commanding positions
	// position[0] ... ROLL
	// position[1] ... PITCH FORE
	// position[2] ... PITCH REAR
	// position[3] ... STEER

	float  position[4];	// servo positions

	void print() {
		std::cout
		<< std::setw( 6) << id
//		<< std::setw(12) << ts.tv_sec
//		<< std::setw(10) << ts.tv_usec
		<< std::setw(12) << position[0]
		<< std::setw(12) << position[1]
		<< std::setw(12) << position[2]
		<< std::setw(12) << position[3]
		<< std::endl;
	}
};

namespace servo {
	enum { ROLL=0, FORE, REAR, STEER };
	const char* topic_read  = "servo_read";
	const char* topic_write = "servo_write";
	const char* password = "mqtt";
};

#endif	// _SERVO_STRUCTURE_HPP_
```


