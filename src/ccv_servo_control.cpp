//
//	Servo Control Publisher/Subscriber
//
#include <iostream>
#include <time.h>
#include <sys/time.h>
#include <mosquitto.hpp>		// c++ wrapper of mosquitto
#include "DynamixelWrapper.hpp"	// 
#include "ccv_servo_structure.hpp"	// my data structure
using namespace std;
// using namespace dynamixel;
using namespace servo;

CcvServoStructure servo_data;

//
// Dynamixel Servo section
//

class CcvServo : public DynamixelRobotSystem {
  public:
    enum DXLID { DXLID_ROLL=1, DXLID_FORE, DXLID_REAR, DXLID_STEER };
//    enum INDEX { ROLL=0, FORE, REAR, STEER };
    void setup();
	void run(){};
    void run(Mosquitto* talker);
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

void CcvServo::run(Mosquitto* talker) {
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
    	servo_data.present_position[ROLL ] = svo[ROLL ]->present_position_rad();
    	servo_data.present_position[FORE ] = svo[FORE ]->present_position_rad();
    	servo_data.present_position[REAR ] = svo[REAR ]->present_position_rad();
    	servo_data.present_position[STEER] = svo[STEER]->present_position_rad();
		talker->publish(servo::topic_read,&servo_data,sizeof(servo_data));

		servo_data.print_read();
		sleep(1);	// dummy, it shoud be ommitted
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
	servo_data.print_command();

	// Copying data from publisher
	// ...
    ccvservo->goal_position_rad(servo::ROLL , servo_data.command_position[servo::ROLL ]);
    ccvservo->goal_position_rad(servo::FORE , servo_data.command_position[servo::FORE ]);
    ccvservo->goal_position_rad(servo::REAR , servo_data.command_position[servo::REAR ]);
    ccvservo->goal_position_rad(servo::STEER, servo_data.command_position[servo::STEER]);
}





int main()
{

	//
	// MQTT Subscriber section
	//

	ServoSubscriber servo_command_listener(topic_write);
	servo_command_listener.set_username_password(servo::name_listener,servo::password);
	servo_command_listener.connect("localhost");
	//servo_command_listener.connect("192.168.0.62");


	//
	// MQTT Publisher section
	//

    Mosquitto servo_data_talker;
    servo_data_talker.set_username_password(servo::name_talker,servo::password);
    servo_data_talker.connect("localhost");
    //servo_data_talker.connect("192.168.0.62");
    servo_data_talker.subscribe(servo::topic_read);


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
    ccvservo->run(&servo_data_talker);

	//
	// Termination
	//
    dxlnet->destroy();
	servo_command_listener.cleanup_library();
}


