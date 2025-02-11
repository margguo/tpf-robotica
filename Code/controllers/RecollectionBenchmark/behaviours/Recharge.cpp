#include "Recharge.h"
#include <behaviours/GoToBaseGroup.h>
#include <math.h>
#include "GarbageCleaner.h"

namespace behaviours {

	Recharge::Recharge(robotapi::IRobot * robot, robotapi::IDifferentialWheels * wheels, robotapi::IBattery * robotBatt, robotapi::IBattery * pcBatt, std::vector<robotapi::IDistanceSensor*> & fss) : AbstractBehaviour("Recharge"){
       	this->robotBattery = robotBatt;
		this->pcBattery = pcBatt;
		this->robot = robot;
		this->wheels = wheels;
		this->fss = &fss;
	}

	void Recharge::sense(){
	}

	void Recharge::action(){
		// This is a tricky one
		// The robot has to:
		// Go a little bit forward
		// then turn 180 degrees till the middle sensor is on the line
		// and wait till the battery is full

		this->positionSelf();
/*
		printf("HOLA4\n");
		system("pause");
*/

		this->lastRobotValue = this->robotBattery->getValue();
		this->lastPCValue = this->pcBattery->getValue();

		// Go back a little bit till the sensor is not on the line anymore
   		this->wheels->setSpeed(-behaviours::BehavioursParameters::getParameter(RECHARGE_BACKWARD_SPD),-behaviours::BehavioursParameters::getParameter(RECHARGE_BACKWARD_SPD));
		while( this->inLine((*this->fss).at(1)->getValue()) ){
			this->robot->step(behaviours::BehavioursParameters::getParameter(RECHARGE_TIME_STEP));
		}
		
		while ( !this->isCharging() ){
			this->robot->step(behaviours::BehavioursParameters::getParameter(RECHARGE_TIME_STEP));
		}

		this->wheels->setSpeed(0,0);
		while( !this->robotBattery->isFull() || !this->pcBattery->isFull() ){
			this->robot->step(behaviours::BehavioursParameters::getParameter(RECHARGE_TIME_STEP));
		}

		// Go forward till the sensor is on the line again
   		this->wheels->setSpeed(behaviours::BehavioursParameters::getParameter(RECHARGE_FORWARD_SPD),behaviours::BehavioursParameters::getParameter(RECHARGE_FORWARD_SPD));
		while( !this->inLine((*this->fss).at(1)->getValue()) ){
			this->robot->step(behaviours::BehavioursParameters::getParameter(RECHARGE_TIME_STEP));
		}

	}

	bool Recharge::isCharging(){
		double currentRobot = this->robotBattery->getValue();
		double currentPC = this->pcBattery->getValue();
		bool charging = this->lastRobotValue < currentRobot || this->lastPCValue < currentPC;
		this->lastRobotValue = currentRobot;
		this->lastPCValue = currentPC;
		return charging;
	}

	void Recharge::positionSelf(){
		double currentAngle = this->wheels->getOrientation();
		if ( ! ( currentAngle > 0 && currentAngle < PI ) )
			return;

   		this->wheels->setSpeed(behaviours::BehavioursParameters::getParameter(RECHARGE_FORWARD_SPD),behaviours::BehavioursParameters::getParameter(RECHARGE_FORWARD_SPD));
		this->goDistance(behaviours::BehavioursParameters::getParameter(RECHARGE_E_PUCK_DIAMETER)/2.0);
		
		bool leftOnLine = this->inLine((*this->fss).at(0)->getValue());
		bool middleOnLine = this->inLine((*this->fss).at(1)->getValue());
		bool rightOnLine = this->inLine((*this->fss).at(2)->getValue());
		
		// Turn till the middle sensor is on the line
		this->wheels->setSpeed(behaviours::BehavioursParameters::getParameter(RECHARGE_BASE_SPD),-behaviours::BehavioursParameters::getParameter(RECHARGE_BASE_SPD));
		while( leftOnLine || !middleOnLine || rightOnLine ){
			/*
	        for (int j = 0; j < behaviours::BehavioursParameters::getParameter(FLOOR_SENSORS); j++){
				printf("Floor sensor %d: %d\n", j, (*this->fss).at(j)->getValue() );
			}
			*/
			this->robot->step(behaviours::BehavioursParameters::getParameter(RECHARGE_TIME_STEP));
			//currentAngle = this->wheels->getOrientation();
			leftOnLine = this->inLine((*this->fss).at(0)->getValue());
			middleOnLine = this->inLine((*this->fss).at(1)->getValue());
			rightOnLine = this->inLine((*this->fss).at(2)->getValue());
		}
		
		int steps = 0;
		// Continue turning till the middle sensor out of the line. Count Steps
		while( middleOnLine ){
			/*
	        for (int j = 0; j < behaviours::BehavioursParameters::getParameter(FLOOR_SENSORS); j++){
				printf("Floor sensor %d: %d\n", j, (*this->fss).at(j)->getValue() );
			}
			*/

			this->robot->step(behaviours::BehavioursParameters::getParameter(RECHARGE_TIME_STEP));
			//currentAngle = this->wheels->getOrientation();
			middleOnLine = this->inLine((*this->fss).at(1)->getValue());
			steps++;
		}
		
		// Now, turn steps/2 in the other direction, middle sensor should be in the middle of the line
		this->wheels->setSpeed(-behaviours::BehavioursParameters::getParameter(RECHARGE_BASE_SPD),behaviours::BehavioursParameters::getParameter(RECHARGE_BASE_SPD));
		for( int i = 0 ; i < (int)(ceil(steps/2.0)) ; i++ ){
            for (int j = 0; j < FLOOR_SENSORS; j++){
				printf("Floor sensor %d: %d\n", j, (*this->fss).at(j)->getValue() );
			}

			this->robot->step(behaviours::BehavioursParameters::getParameter(RECHARGE_TIME_STEP));
		}

		this->alignWithLine();

	}

	void Recharge::alignWithLine(){
		this->followLine(false,behaviours::BehavioursParameters::getParameter(RECHARGE_E_PUCK_DIAMETER));
		this->followLine(true,behaviours::BehavioursParameters::getParameter(RECHARGE_E_PUCK_DIAMETER));
    }

	void Recharge::followLine(bool backwards, double distance){
		double distanceCovered = 0;
	
		utils::MyPoint * lastPosition = this->wheels->getPosition();
		double initialX = lastPosition->getX();
		double initialY = lastPosition->getY();
	
		double currentX, currentY;
		utils::MyPoint * currentPosition;
		while ( distanceCovered < distance ){
    		this->followLineSpd(backwards);
			this->robot->step(behaviours::BehavioursParameters::getParameter(RECHARGE_TIME_STEP));
			currentPosition = this->wheels->getPosition();
			currentX = currentPosition->getX();
			currentY = currentPosition->getY();
			distanceCovered = sqrt( pow(currentX-initialX,2) + pow(currentY-initialY,2) );
		}
	}

    void Recharge::followLineSpd(bool backwards){
		double lspd = behaviours::BehavioursParameters::getParameter(RECHARGE_BASE_SPD);
		double rspd = behaviours::BehavioursParameters::getParameter(RECHARGE_BASE_SPD);
		
		if ( backwards ){
			lspd = -1 * lspd;
			rspd = -1 * rspd;
		}

		// Left sensor is on line?
		if ( this->inLine((*this->fss).at(0)->getValue()) ){
			lspd = lspd * ( 1 - behaviours::BehavioursParameters::getParameter(RECHARGE_SPD_FACTOR) );
			rspd = rspd * ( 1 + behaviours::BehavioursParameters::getParameter(RECHARGE_SPD_FACTOR) );
		}
		// Right sensor is on line?
		if ( this->inLine((*this->fss).at(2)->getValue()) ){
			lspd = lspd * ( 1 + behaviours::BehavioursParameters::getParameter(RECHARGE_SPD_FACTOR) );
			rspd = rspd * ( 1 - behaviours::BehavioursParameters::getParameter(RECHARGE_SPD_FACTOR) );
		}
		this->wheels->setSpeed(lspd,rspd);
	}

	void Recharge::goDistance(double distance){
		double distanceCovered = 0;
	
		utils::MyPoint * lastPosition = this->wheels->getPosition();
		double initialX = lastPosition->getX();
		double initialY = lastPosition->getY();
	
		double currentX, currentY;
		utils::MyPoint * currentPosition;
		while ( distanceCovered < distance ){
			this->robot->step(behaviours::BehavioursParameters::getParameter(RECHARGE_TIME_STEP));
			currentPosition = this->wheels->getPosition();
			currentX = currentPosition->getX();
			currentY = currentPosition->getY();
			distanceCovered = sqrt( pow(currentX-initialX,2) + pow(currentY-initialY,2) );
		}
	}

} /* End of namespace behaviours */
