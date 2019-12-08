//
//	Servo Control Publisher/Subscriber
//
#include <iostream>
#include <time.h>
#include <sys/time.h>
#include <mosquitto.hpp>		// c++ wrapper of mosquitto
#include "DynamixelWrapper.hpp"	// 
#include "servo_structure.hpp"	// my data structure
using namespace std;
// using namespace dynamixel;
using namespace servo;

ServoStructure servo_data;

//
// Dynamixel Servo section
//

class CcvServo : public DynamixelRobotSystem {
  public:
    enum DXLID { DXLID_ROLL=1, DXLID_FORE, DXLID_REAR, DXLID_STEER };
    enum INDEX { ROLL=0, FORE, REAR, STEER };
    void setup();
    void run();
};

void CcvServo::setup() {
    // disable anyway, for safety
    svo[ROLL ]->torque_disable();
    svo[FORE ]->torque_disable();
    svo[REAR ]->torque_disable();
    svo[STEER]->torque_disable();

    svo[ROLL ]->profile_acceleration(1800.0F);
    svo[FORE ]->profile_acceleration(1800.0F);
    svo[REAR ]->profile_acceleration(1800.0F);
    svo[STEER]->profile_acceleration(1800.0F);
//  svo[STEER]->position_p_gain(0);
}

void CcvServo::run() {
    svo[ROLL ]->torque_enable();
    svo[FORE ]->torque_enable();
    svo[REAR ]->torque_enable();
    svo[STEER]->torque_enable();

    svo[ROLL ]->goal_position_deg(0);   // reset to the home position
    svo[FORE ]->goal_position_deg(0);
    svo[REAR ]->goal_position_deg(0);
    svo[STEER]->goal_position_deg(0);
    usleep(3000*1000);

	// for ...
	while(1) {
		sleep(1);
	}
}


CcvServo* ccvservo = new CcvServo;






//
// MQTT section
//

class ServoSubscriber : public Mosquitto {
  protected:
//	ServoStructure data;
	const char* topic;
	void onConnected();
	void onError(const char* _msg) { std::cout << _msg; }
	void onMessage(std::string _topic, void* _data, int _len);
	struct timeval ts;

  public:
	ServoSubscriber(const char* _topic):topic(_topic){}
};

void ServoSubscriber::onConnected()
{
	std::cout << "ServoSubscriber is Connected.\n";
	subscribe(topic);
}

#include <strings.h>
void ServoSubscriber::onMessage(std::string _topic, void* _data, int _len)
{
//	gettimeofday(&ts,NULL);
	bcopy(_data, (char*)&servo_data, sizeof(servo_data));		
//	int32_t diff = (ts.tv_sec-data.ts.tv_sec)*1000000 + ts.tv_usec-data.ts.tv_usec;
//	std::cout << std::setw(5) << diff << " usec,";
	servo_data.print();

	// Copying data from publisher
	// ...
    ccvservo->goal_position_rad(CcvServo::ROLL , servo_data.position[servo::ROLL ]);
    ccvservo->goal_position_rad(CcvServo::FORE , servo_data.position[servo::FORE ]);
    ccvservo->goal_position_rad(CcvServo::REAR , servo_data.position[servo::REAR ]);
    ccvservo->goal_position_rad(CcvServo::STEER, servo_data.position[servo::STEER]);
}





int main()
{

	//
	// MQTT Subscriber section
	//
	const char* ip_addr  = "localhost";
	// const char* ip_addr  = "192.168.0.62";
	const char* username = "servo_command_listener";

	ServoSubscriber servo_command_listener(topic);
	servo_command_listener.set_username_password(username,password);
	servo_command_listener.connect(ip_addr);

	//
	// Servo section
	//
    DynamixelNetwork::create
        ("/dev/ttyUSB0", DynamixelNetwork::PROTOCOL2, DynamixelNetwork::BAUDRATE_1M);
    DynamixelNetwork* dxlnet = DynamixelNetwork::getNetworkPointer();

//    CcvServo* ccvservo = new CcvServo;
    ccvservo->add(new Dynamixel_H54P(dxlnet, CcvServo::DXLID_ROLL));
    ccvservo->add(new Dynamixel_H54P(dxlnet, CcvServo::DXLID_FORE));
    ccvservo->add(new Dynamixel_H54P(dxlnet, CcvServo::DXLID_REAR));
    ccvservo->add(new Dynamixel_H42P(dxlnet, CcvServo::DXLID_STEER));
    ccvservo->setup();

	//
	// Start event loops
	//
	servo_command_listener.loop_start();
    ccvservo->run();

	//
	// Termination
	//
    dxlnet->destroy();
	servo_command_listener.cleanup_library();
}


