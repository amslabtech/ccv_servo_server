//
//	main.cpp
//
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <DynamixelWrapper.h>
using namespace std;
using namespace dynamixel;



class CCV : public DynamixelRobotSystem {
  public:
	enum DXLID { DXLID_ROLL=1, DXLID_FORE, DXLID_REAR, DXLID_STEER };
	enum INDEX { ROLL=0, FORE, REAR, STEER };
	void setup();
	void run();
};

void CCV::setup() {
	// disable anyway, for safety
	svo[ROLL ]->torque_disable();
	svo[FORE ]->torque_disable();
	svo[REAR ]->torque_disable();
	svo[STEER]->torque_disable();

	// limit value calculation
#if 0
	svo[ROLL ]->max_position_limit_deg(7);
	svo[ROLL ]->min_position_limit_deg(7);

	svo[FORE ]->max_position_limit_deg(0);
	svo[FORE ]->min_position_limit_deg(-20);

	svo[REAR ]->max_position_limit_deg(0);
	svo[REAR ]->min_position_limit_deg(-20);

	svo[STEER]->max_position_limit_deg(24);
	svo[STEER]->min_position_limit_deg(-24);
	usleep(3000000);
#endif

	svo[ROLL ]->profile_acceleration(1800.0F);
	svo[FORE ]->profile_acceleration(1800.0F);
	svo[REAR ]->profile_acceleration(1800.0F);
	svo[STEER]->profile_acceleration(1800.0F);
//	svo[STEER]->position_p_gain(0);
}

void CCV::run() {
	svo[ROLL ]->torque_enable();
	svo[FORE ]->torque_enable();
	svo[REAR ]->torque_enable();
	svo[STEER]->torque_enable();

	svo[ROLL ]->goal_position_deg(0);	// reset to the home position
	svo[FORE ]->goal_position_deg(0);
	svo[REAR ]->goal_position_deg(0);
	svo[STEER]->goal_position_deg(0);
	usleep(3000000);

	svo[FORE ]->goal_position_deg( 10);	// lift up
	svo[REAR ]->goal_position_deg( 10);
	usleep(3000000);
	svo[FORE ]->goal_position_deg( 0);	// calm down
	svo[REAR ]->goal_position_deg( 0);
	usleep(1000000);


	for(int step=0; step<7; step++) {
		svo[STEER]->led(rand()%256,rand()%256,rand()%256);

		cout << "Step: " << step << "\n";

		
		if(step == 1) {
			svo[ROLL ]->goal_position_deg(  5);	// roll right
			svo[FORE ]->goal_position_deg(  0);
			svo[REAR ]->goal_position_deg(  0);

		} 

		if(step == 2) {
			svo[ROLL ]->goal_position_deg(  0);	// stand still
			svo[FORE ]->goal_position_deg(  0);
			svo[REAR ]->goal_position_deg(  0);

		} 
		
		if(step == 3) {
			svo[ROLL ]->goal_position_deg( -7);	// roll left
			svo[FORE ]->goal_position_deg(  0);	//
			svo[REAR ]->goal_position_deg(  0);

		}
		if(step == 4) {
			svo[ROLL ]->goal_position_deg(  0);
			svo[FORE ]->goal_position_deg(  0);
			svo[REAR ]->goal_position_deg(  0);

		} 
		
		if(step == 5) {
			svo[ROLL ]->goal_position_deg(  0);
			svo[FORE ]->goal_position_deg(-10);	// nose dive for acceleration
			svo[REAR ]->goal_position_deg( 10);

		} 
		
		if(step == 6) {
			svo[ROLL ]->goal_position_deg(  0);
			svo[FORE ]->goal_position_deg( 10);	// lean behind for hard breaking!
			svo[REAR ]->goal_position_deg(-10);
		}



		float alpha = float(rand()%4801)/100 -24;	// -24 < alpha < 24
		cout << "Goal position: " << alpha << "\n";
		svo[STEER]->goal_position_deg(alpha);

		for(int i=0; i<100; i++){
			float cp = svo[STEER]->present_position_deg();
			cout << "present position: " << cp << "\n";
			
			if( abs(alpha-cp) < 1 ) break;
			usleep(100000);
		}	

		usleep(3000000);
	}

	svo[ROLL ]->goal_position_deg(0);
	svo[FORE ]->goal_position_deg(0);
	svo[REAR ]->goal_position_deg(0);
	svo[STEER]->goal_position_deg(0);
	usleep(1000000);
}

int main()
{
	DynamixelNetwork::create
		("/dev/ttyUSB0", DynamixelNetwork::PROTOCOL2, DynamixelNetwork::BAUDRATE_1M);
	DynamixelNetwork* dnet = DynamixelNetwork::getNetworkPointer();
	
	CCV* ccv = new CCV;
	ccv->add(new Dynamixel_H54P(dnet, CCV::DXLID_ROLL));
	ccv->add(new Dynamixel_H54P(dnet, CCV::DXLID_FORE));
	ccv->add(new Dynamixel_H54P(dnet, CCV::DXLID_REAR));
	ccv->add(new Dynamixel_H42P(dnet, CCV::DXLID_STEER));

	ccv->setup();
	ccv->run();

	dnet->destroy();
}


