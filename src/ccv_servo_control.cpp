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
static volatile uint32_t command_updated = 0;

//
// Dynamixel Servo section
//

class CcvServo : public DynamixelRobotSystem {
  public:
    enum DXLID { DXLID_ROLL=1, DXLID_FORE, DXLID_REAR, DXLID_STRR, DXLID_STRL };
	CcvServo(DynamixelNetwork* _dnet):DynamixelRobotSystem(_dnet){}
    void setup();
	void run(){};
    void run(Mosquitto* talker);
};

void CcvServo::setup()
{
    // disable anyway, for safety
    torque_disable();
    profile_acceleration( 500.0F);

//  svo[STEER]->position_p_gain(0);

	// Stand-up operation
    torque_enable();

	float goal[] = { -1, 3+1.5, 3-1.5, 0, 0 };
	goal_position_deg(goal);

	usleep(3000*1000);	// waiting for standing up

    profile_acceleration(1500.0F);
}

void CcvServo::run(Mosquitto* talker)
{
	for(int i=0; ; i++) {
//		std::cout << "send present position" << std::endl;

		servo_data.id = i;
//		sync_present_position_rad(servo_data.present_position);
		servo_data.present_position[ROLL] = svo[ROLL]->present_position_rad();
		servo_data.present_position[FORE] = svo[FORE]->present_position_rad();
		servo_data.present_position[REAR] = svo[REAR]->present_position_rad();
		servo_data.present_position[STRR] = svo[STRR]->present_position_rad();
		servo_data.present_position[STRL] = svo[STRL]->present_position_rad();

		talker->publish(servo::topic_read,&servo_data,sizeof(servo_data));

		usleep(1000);	// dummy, it shoud be ommitted

		// Copying data from publisher
		if(command_updated>0) {
   			//ccvservo->sync_goal_position_rad(servo_data.command_position);
   			goal_position_rad(servo_data.command_position);
			command_updated = 0;
		}

		usleep(1000);	// dummy, it shoud be ommitted
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
	command_updated++;
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
        ("/dev/DXLSERVO", DynamixelNetwork::PROTOCOL2, DynamixelNetwork::BAUDRATE_4M);
    DynamixelNetwork* dxlnet = DynamixelNetwork::getNetworkPointer();

	CcvServo* ccvservo = new CcvServo(dxlnet);
    ccvservo->add(new Dynamixel_H54P(dxlnet, CcvServo::DXLID_ROLL));
    ccvservo->add(new Dynamixel_H54P(dxlnet, CcvServo::DXLID_FORE));
    ccvservo->add(new Dynamixel_H54P(dxlnet, CcvServo::DXLID_REAR));
    ccvservo->add(new Dynamixel_H42P(dxlnet, CcvServo::DXLID_STRR));
    ccvservo->add(new Dynamixel_H42P(dxlnet, CcvServo::DXLID_STRL));
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


