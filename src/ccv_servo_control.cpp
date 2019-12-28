//
//	Servo Control Publisher/Subscriber
//
#include <iostream>
//#include <mutex>
#include <time.h>
#include <sys/time.h>
#include <mosquitto.hpp>		// c++ wrapper of mosquitto
#include "DynamixelWrapper.hpp"	//
#include "ccv_servo_structure.hpp"	// my data structure
using namespace std;
using namespace servo;

CcvServoStructure servo_data;
static volatile bool command_updated = false;

//
// Dynamixel Servo section
//

class CcvServo : public DynamixelRobotSystem {
  public:
    enum DXLID { DXLID_ROLL=1, DXLID_FORE, DXLID_REAR, DXLID_STEER };
	CcvServo(DynamixelNetwork* _dnet):DynamixelRobotSystem(_dnet){}
    void setup();
	void run(){};
    void run(Mosquitto* talker);
};

void CcvServo::setup()
{
    // disable anyway, for safety
    svo[ROLL ]->torque_disable();
    svo[FORE ]->torque_disable();
    svo[REAR ]->torque_disable();
    svo[STEER]->torque_disable();

    svo[ROLL ]->profile_acceleration( 500.0F);
    svo[FORE ]->profile_acceleration( 500.0F);
    svo[REAR ]->profile_acceleration( 500.0F);
    svo[STEER]->profile_acceleration(1800.0F);

//  svo[STEER]->position_p_gain(0);

	// Stand-up operation
	//
    svo[ROLL ]->torque_enable();
    svo[FORE ]->torque_enable();
    svo[REAR ]->torque_enable();
    svo[STEER]->torque_enable();

	float goal[] = { -1, 3+1.5, 3-1.5, 0 };
	sync_goal_position_deg(goal);

	usleep(3000*1000);	// waiting for standing up

    svo[ROLL ]->profile_acceleration(1800.0F);
    svo[FORE ]->profile_acceleration(1800.0F);
    svo[REAR ]->profile_acceleration(1800.0F);
}

void CcvServo::run(Mosquitto* talker)
{
	for(int i=0; ; i++) {
//		std::cout << "send present position" << std::endl;

		servo_data.id = i;
//		sync_present_position_rad(servo_data.present_position);
		servo_data.present_position[ROLL ] = svo[ROLL ]->present_position_rad();
		servo_data.present_position[FORE ] = svo[FORE ]->present_position_rad();
		servo_data.present_position[REAR ] = svo[REAR ]->present_position_rad();
		servo_data.present_position[STEER] = svo[STEER]->present_position_rad();

		talker->publish(servo::topic_read,&servo_data,sizeof(servo_data));

		usleep(5*1000);	// dummy, it shoud be ommitted

		// Copying data from publisher
		if(command_updated==true) {
   			//ccvservo->sync_goal_position_rad(servo_data.command_position);
   			sync_goal_position_rad(servo_data.command_position);
			command_updated = false;
		}
	}
}



//
// MQTT section
//

class ServoSubscriber : public Mosquitto {
  protected:
	const char* topic;
	void onConnected();
	void onError(const char* _msg) { std::cout << _msg; }
	void onMessage(std::string _topic, void* _data, int _len);
	struct timeval ts;
	CcvServo* ccvservo;

  public:
	ServoSubscriber(const char* _topic, CcvServo* _ccvservo):
		topic(_topic), ccvservo(_ccvservo) {}
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
	// bcopy(_data, (char*)&servo_data.command_position, sizeof(servo_data.command_position));
    CcvServoStructure* received_data = NULL;
    received_data = (CcvServoStructure*)_data;
	bcopy(received_data->command_position, (char*)&servo_data.command_position, sizeof(servo_data.command_position));
//	int32_t diff = (ts.tv_sec-data.ts.tv_sec)*1000000 + ts.tv_usec-data.ts.tv_usec;
//	std::cout << std::setw(5) << diff << " usec,";
	// servo_data.print_command();

	// Copying data from publisher
//   	ccvservo->sync_goal_position_rad(servo_data.command_position);
	command_updated = true;
}





int main()
{
	//
	// MQTT Publisher section
	//
	const char* name_talker		= "servo_data_talker";
    Mosquitto servo_data_talker;
    servo_data_talker.set_username_password(name_talker,servo::password);
    servo_data_talker.connect("localhost");
    servo_data_talker.subscribe(servo::topic_read);


	//
	// Servo section
	//
    DynamixelNetwork::create
        ("/dev/ttyUSB0", DynamixelNetwork::PROTOCOL2, DynamixelNetwork::BAUDRATE_4M);
    DynamixelNetwork* dxlnet = DynamixelNetwork::getNetworkPointer();

	CcvServo* ccvservo = new CcvServo(dxlnet);
    ccvservo->add(new Dynamixel_H54P(dxlnet, CcvServo::DXLID_ROLL));
    ccvservo->add(new Dynamixel_H54P(dxlnet, CcvServo::DXLID_FORE));
    ccvservo->add(new Dynamixel_H54P(dxlnet, CcvServo::DXLID_REAR));
    ccvservo->add(new Dynamixel_H42P(dxlnet, CcvServo::DXLID_STEER));
    ccvservo->setup();

	//
	// MQTT Subscriber section
	//
	const char* name_listener	= "servo_command_listener";
	ServoSubscriber servo_command_listener(topic_write, ccvservo);
	servo_command_listener.set_username_password(name_listener,servo::password);
	servo_command_listener.connect("localhost");


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


