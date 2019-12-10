//
//	Servo Control Publisher/Subscriber
//
#include <iostream>
#include <mutex>
#include <time.h>
#include <sys/time.h>
#include <mosquitto.hpp>		// c++ wrapper of mosquitto
#include "DynamixelWrapper.hpp"	// 
#include "ccv_servo_structure.hpp"	// my data structure
using namespace std;
// using namespace dynamixel;
using namespace servo;

CcvServoStructure servo_data;

std::mutex mutex_servo;

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

	mutex_servo.lock();
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
	mutex_servo.unlock();
}

void CcvServo::run(Mosquitto* talker) {
	mutex_servo.lock();
    svo[ROLL ]->torque_enable();
    svo[FORE ]->torque_enable();
    svo[REAR ]->torque_enable();
    svo[STEER]->torque_enable();

    svo[ROLL ]->goal_position_deg(0);   // reset to the home position
    svo[FORE ]->goal_position_deg(0);
    svo[REAR ]->goal_position_deg(0);
    svo[STEER]->goal_position_deg(0);
	mutex_servo.unlock();

	// for ...
	servo_data.id = 0;
	while(1) {
		servo_data.id++;

		mutex_servo.lock();
//    	servo_data.present_position[ROLL ] = svo[ROLL ]->present_position_deg();
		mutex_servo.unlock();

		mutex_servo.lock();
//    	servo_data.present_position[FORE ] = svo[FORE ]->present_position_deg();
		mutex_servo.unlock();

		mutex_servo.lock();
//    	servo_data.present_position[REAR ] = svo[REAR ]->present_position_deg();
		mutex_servo.unlock();

		mutex_servo.lock();
    	servo_data.present_position[STEER] = svo[STEER]->present_position_deg();
		mutex_servo.unlock();

		talker->publish(servo::topic_read,&servo_data,sizeof(servo_data));

		//servo_data.print_read();
		//usleep(100*1000);	// dummy, it shoud be ommitted
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
//	servo_data.print_command();

	// Copying data from publisher
	mutex_servo.lock();
//    ccvservo->goal_position_rad(servo::ROLL , servo_data.command_position[servo::ROLL ]);
	mutex_servo.unlock();

	mutex_servo.lock();
//    ccvservo->goal_position_rad(servo::FORE , servo_data.command_position[servo::FORE ]);
	mutex_servo.unlock();

	mutex_servo.lock();
//    ccvservo->goal_position_rad(servo::REAR , servo_data.command_position[servo::REAR ]);
	mutex_servo.unlock();

	mutex_servo.lock();
    ccvservo->goal_position_rad(servo::STEER, servo_data.command_position[servo::STEER]);
	mutex_servo.unlock();
}





int main()
{

	//
	// MQTT Subscriber section
	//
	const char* name_listener	= "servo_command_listener";
	ServoSubscriber servo_command_listener(topic_write);
	servo_command_listener.set_username_password(name_listener,servo::password);
	servo_command_listener.connect("localhost");
	//servo_command_listener.connect("192.168.0.62");


	//
	// MQTT Publisher section
	//
	const char* name_talker		= "servo_data_talker";
    Mosquitto servo_data_talker;
    servo_data_talker.set_username_password(name_talker,servo::password);
    servo_data_talker.connect("localhost");
    //servo_data_talker.connect("192.168.0.62");
    servo_data_talker.subscribe(servo::topic_read);


	//
	// Servo section
	//
    DynamixelNetwork::create
        ("/dev/ttyUSB0", DynamixelNetwork::PROTOCOL2, DynamixelNetwork::BAUDRATE_4M);
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


