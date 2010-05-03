#include "GarbageCleaner.h"
#include <behaviours/AvoidObstacle.h>
#include <behaviours/CollectGarbage.h>
#include <behaviours/FocusGarbage.h>
#include <behaviours/GoToBaseGroup.h>
#include <behaviours/GoToDisposal.h>
#include <behaviours/Recharge.h>
#include <behaviours/UnloadGarbage.h>
#include <behaviours/Wander.h>
#include <robotapi/IDistanceSensor.h>
#include <robotapi/IServo.h>
#include <robotapi/IBattery.h>
#include <robotapi/IDifferentialWheels.h>
#include <robotapi/ITrashBin.h>
#include <vector>
#include <sstream>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

std::vector<robotapi::IDistanceSensor*> dss;
std::vector<robotapi::IDistanceSensor*> fss;
robotapi::IServo* servoFront;
robotapi::IServo* servoBottom;
robotapi::IServo* servoRear;
robotapi::IServo* servoInside;
robotapi::ITrashBin* trashBin;
robotapi::IDifferentialWheels* wheels;
robotapi::IBattery* robotBattery;
robotapi::IBattery* pcBattery;
robotapi::ICamera* camera;
robotapi::IRobot* robot;

GarbageCleaner::GarbageCleaner(WorldInfo * wi, robotapi::IRobot & robot) : myIRobot(robot)
// don't delete the following line as it's needed to preserve source code of this autogenerated element
// section -64--88-1-100--1230920c:125cb7ecd6f:-8000:0000000000000DB6 begin
{
	this->initializeSensors();
	this->initializeBehaviours(wi);
}
// section -64--88-1-100--1230920c:125cb7ecd6f:-8000:0000000000000DB6 end
// don't delete the previous line as it's needed to preserve source code of this autogenerated element

void GarbageCleaner::initializeSensors(){
	for( int i=0 ; i < DISTANCE_SENSORS ; i++ ){
		std::stringstream sstr;
		sstr << "ps" << i;
		robotapi::IDistanceSensor * ids = &myIRobot.getDistanceSensor(sstr.str());
		ids->enable(TIME_STEP);
		dss.push_back(ids);
	}

	for( int i=0 ; i < FLOOR_SENSORS ; i++ ){
		std::stringstream sstr;
		sstr << "fs" << i;
		robotapi::IDistanceSensor * fds = &myIRobot.getDistanceSensor(sstr.str());
		fds->enable(TIME_STEP);
		fss.push_back(fds);
	}

	wheels = &myIRobot.getDifferentialWheels("df");
	wheels->enableEncoders(TIME_STEP);

	servoBottom = &myIRobot.getServo("servo0");
	servoBottom->enablePosition(TIME_STEP);
	servoFront = &myIRobot.getServo("servo2");
	servoFront->enablePosition(TIME_STEP);
//	servoFront->setForce(90);
	servoFront->setMotorForce(90);
	servoFront->setVelocity(4);
	servoRear = &myIRobot.getServo("servo1");
	servoRear->enablePosition(TIME_STEP);
	servoInside = &myIRobot.getServo("servo3");
	servoInside->enablePosition(TIME_STEP);
	servoInside->setMotorForce(90);
	servoInside->setVelocity(4);
/*
	trashBin = &myIRobot.getTrashBin("ts0");
	trashBin->enable(TIME_STEP);
*/
	robotBattery = &myIRobot.getBattery("b0");
	robotBattery->enable(TIME_STEP);
	
	pcBattery = &myIRobot.getBattery("b1");
	pcBattery->enable(TIME_STEP);
/*
	camera = &myIRobot.getCamera("camera0");
	camera->enable(TIME_STEP);
	for( int i=0 ; i < FLOOR_SENSORS ; i++ ){
		std::stringstream sstr;
		sstr << "fs" << i;
		robotapi::IDistanceSensor * ifs = &myIRobot.getDistanceSensor(sstr.str());
		ifs->enable(TIME_STEP);
		fss->push_back(ifs);
	}




*/
	
}

void GarbageCleaner::initializeBehaviours(WorldInfo * wi){
    behaviours::AbstractBehaviour * ab;
/*
	ab = new behaviours::Wander(wheels);
	myAbstractBehaviours.push_back(ab);

	ab = new behaviours::FocusGarbage( camera, wheels );
	myAbstractBehaviours.push_back(ab);

	ab = new behaviours::CollectGarbage( servoFront , servoBottom );
	myAbstractBehaviours.push_back(ab);

	ab = new behaviours::UnloadGarbage( trashBin , servoRear );
	myAbstractBehaviours.push_back(ab);

	ab = new behaviours::GoToBaseGroup( wi, &myIRobot, robotBattery , pcBattery , wheels, fss );
	myAbstractBehaviours.push_back(ab);

	ab = new behaviours::AvoidObstacle(wheels,dss);
	myAbstractBehaviours.push_back(ab);
*/
}

void GarbageCleaner::cleanGarbage()
// don't delete the following line as it's needed to preserve source code of this autogenerated element
// section -64--88-1-100--1230920c:125cb7ecd6f:-8000:0000000000000DBA begin
{
    std::list<behaviours::AbstractBehaviour*>::iterator it;
	for ( int i = 0 ; i < 100 ; i ++ ){
		this->myIRobot.step(TIME_STEP);
	}

	servoFront->setPosition(1.57);
	for ( int i = 0 ; i < 100 ; i ++ ){
		this->myIRobot.step(TIME_STEP);
	}

	double currentAngle = 0.0;
	while ( currentAngle < 0.5 ){
		this->myIRobot.step(TIME_STEP);
		servoBottom->setPosition(currentAngle);
		currentAngle += 0.005;
	}

    wheels->setSpeed(10,10);
	for ( int i = 0 ; i < 1200 ; i ++ ){
  // 		servoBottom->setPosition(currentAngle);
        printf("\n ------------ \n %d \n",i);
		this->myIRobot.step(TIME_STEP);
	}

//	servoBottom->setPosition(0.73);

	for ( int i = 0 ; i < 40 ; i ++ ){
		this->myIRobot.step(TIME_STEP);
	}

//	servoFront->setPosition(0.1);

	currentAngle = 1.57;
	while ( currentAngle > 0.07 ){
		this->myIRobot.step(TIME_STEP);
		servoFront->setPosition(currentAngle);
		currentAngle -= 0.005;
	}

	for ( int i = 0 ; i < 40 ; i ++ ){
		this->myIRobot.step(TIME_STEP);
	    wheels->setSpeed(0,0);
	}
	system("PAUSE");

	currentAngle = 0;
	while ( currentAngle < 0.01 ){
		this->myIRobot.step(TIME_STEP);
		servoInside->setPosition(currentAngle);
		servoFront->setPosition(0.005*( 1 - (currentAngle/0.01)) + 0.095);
		currentAngle += 0.005;
	}

	while ( 1 ){
        		servoInside->setPosition(currentAngle);
        				servoFront->setPosition(0.005*( 1 - (currentAngle/0.01)) + 0.095);
		std::list<behaviours::AbstractBehaviour*>::iterator it;
		behaviours::AbstractBehaviour::resetStimulusPresent();
		//std::cout << "mylist contains:";

		for ( it=myAbstractBehaviours.begin() ; it != myAbstractBehaviours.end(); it++ ){
			//std::cout << "\n" << (*it)->toString() << "\n";
			(*it)->sense();
		}
		wheels->computeOdometry();
		for ( it=myAbstractBehaviours.begin() ; it != myAbstractBehaviours.end(); it++ ){
			//std::cout << "\n" << (*it)->toString() << "\n";
			(*it)->act();
		}

		/*
		std::cout << "Robot Battery : " << robotBattery->getValue() << "\n";
		std::cout << "PC Battery : " << pcBattery->getValue() << "\n";
		*/
		this->myIRobot.step(TIME_STEP);


	}
}
// section -64--88-1-100--1230920c:125cb7ecd6f:-8000:0000000000000DBA end
// don't delete the previous line as it's needed to preserve source code of this autogenerated element

