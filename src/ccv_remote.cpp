//
//	CCV Remote Controller
//
#include <iostream>
#include <time.h>
#include <sys/time.h>
#include <mosquitto.hpp>		// c++ wrapper of mosquitto
//#include "DynamixelWrapper.hpp"	// 
#include "ccv_servo_structure.hpp"	// my data structure
#include <math.h>

using namespace std;
using namespace servo;

CcvServoStructure servo_data;

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
	servo_data.print_read();
}





int main()
{
	//
	// MQTT Subscriber section
	//
	const char* name_catcher	= "servo_data_catcher";
	ServoSubscriber servo_data_catcher(topic_read);
	servo_data_catcher.set_username_password(name_catcher,servo::password);
	servo_data_catcher.connect("localhost");
	//servo_command_listener.connect("192.168.0.62");

	//
	// MQTT Publisher section
	//
	const char* name_commander		= "servo_servo_commander";
    Mosquitto servo_commander;
    servo_commander.set_username_password(name_commander,servo::password);
    servo_commander.connect("localhost");
    //servo_data_talker.connect("192.168.0.62");
    servo_commander.subscribe(servo::topic_write);


	//
	// Start event loops
	//
	servo_data_catcher.loop_start();

	for(int i=0; ; i++) {
		servo_data.id = i;
		servo_data.command_position[servo::ROLL ] = 0;
		servo_data.command_position[servo::FORE ] = 0;
		servo_data.command_position[servo::REAR ] = 0;
		servo_data.command_position[servo::STEER] = 20.0F*M_PI/180*sin(i/10);
		servo_commander.publish(servo::topic_write,&servo_data,sizeof(servo_data));
		usleep(1000*1000);
	}

	//
	// Termination
	//
	servo_data_catcher.cleanup_library();
//	servo_commander.cleanup_library();
}


