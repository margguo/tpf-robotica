// Class automatically generated by Dev-C++ New Class wizard

#include "GoToBaseGroup.h" // class's header file
#include "GarbageCleaner.h"
#include <math.h>

namespace behaviours {

// class constructor
GoToBaseGroup::GoToBaseGroup(WorldInfo * wi,robotapi::IRobot * robot, robotapi::IBattery * robotBatt, robotapi::IBattery * pcBatt, robotapi::IDifferentialWheels * wheels, std::vector<robotapi::IDistanceSensor*> & fss) : AbstractBehaviour("Go To Base Group")
{
	this->robotBattery = robotBatt;
	this->pcBattery = pcBatt;
	this->wi = wi;
	this->fss = &fss;
	this->wheels = wheels;
	
	this->robotBattery->setEmptyBias(GO_TO_BASE_EMPTY_BIAS);
	this->robotBattery->setFullBias(GO_TO_BASE_FULL_BIAS);
	behaviours::AbstractBehaviour * ab = new behaviours::FindLine( wi, wheels, fss );
	myBehaviours[0] = ab;

	ab = new behaviours::PositionInLine( wi, wheels, fss );
	myBehaviours[1] = ab;

	ab = new behaviours::GoToBase( wheels, fss );
	myBehaviours[2] = ab;
	
	ab = new behaviours::Recharge( robot, wheels, robotBatt, pcBatt, fss);
	myBehaviours[3] = ab;
}

// class destructor
GoToBaseGroup::~GoToBaseGroup()
{
	// insert your code here
}

void GoToBaseGroup::sense(){
//		this->setStimulusPresent();
	if ( this->robotBattery->isEmpty() || this->pcBattery->isEmpty() )
		this->setStimulusPresent();

/*
        for (int j = 0; j < FLOOR_SENSORS; j++){
			printf("Floor sensor %d: %d\n", j, (*this->fss).at(j)->getValue() );
		}
*/
}

int following = false;
bool beenOnMark = false;

void GoToBaseGroup::action(){
	
	double xpos = this->wheels->getPosition()->getX();

	if ( xpos < GO_TO_BASE_BASE_POSITION && fabs( this->wheels->getOrientation() - 3*(PI/2)) < GO_TO_BASE_BASE_FIX_ORIENTATION_TOLE ){
		this->myBehaviours[3]->action();
		following = false;
		return;
	}

	if ( ! following ){
		beenOnMark = false;
		if ( !this->inLine() || xpos > GO_TO_BASE_LINE_X_POSITION_THRESHOLD ){
		    this->myBehaviours[0]->action();
	    	printf("Going to line\n");
		    return;
		}

		if ( this->inLine() && !this->inPosition() ){
		    this->myBehaviours[1]->action();
	    	printf("On Line, Positioning...\n");
		    return;
		}
	}
	following = true;

	if ( !this->inLine() ){
		if ( xpos < GO_TO_BASE_BASE_POSITION ){
	    	printf("Recharging\n");
			this->myBehaviours[3]->action();
		}
		following = false;
	}else{
		if( xpos < GO_TO_BASE_PASSAGE_BEGIN_X && this->onMark() && ! beenOnMark ){
			printf("On Mark!");
			beenOnMark = true;
			this->wheels->setPosition(GO_TO_BASE_LINE_MARK_X,GO_TO_BASE_PASSAGE_LINE_Z,true);
		}
	    this->myBehaviours[2]->action();
		this->correctOrientation();
	    printf("Following Line\n");
	}
    return;
}

void GoToBaseGroup::correctOrientation(){
	double z = this->wheels->getPosition()->getY();

	if ( this->wheels->getPosition()->getY() > GO_TO_BASE_FROM_Z_LEFT ){
		this->wheels->setOrientation(0);
		this->wheels->setPosition(GO_TO_BASE_X_CORRECTION_LINE,z,true);
		printf("correcting left\n");
	}

	if ( this->wheels->getPosition()->getY() < GO_TO_BASE_FROM_Z_RIGHT ){
		this->wheels->setOrientation(GO_TO_BASE_LINE_ORIENTATION_CORRECTION);
		this->wheels->setPosition(GO_TO_BASE_X_CORRECTION_LINE,z,true);
		printf("correcting right\n");
	}
}

bool GoToBaseGroup::onMark(){
	return AbstractBehaviour::inLine((*this->fss).at(0)->getValue()) &&
			AbstractBehaviour::inLine((*this->fss).at(1)->getValue()) &&
			AbstractBehaviour::inLine((*this->fss).at(2)->getValue());
}

bool GoToBaseGroup::inLine(){
	return AbstractBehaviour::inLine((*this->fss).at(0)->getValue()) ||
			AbstractBehaviour::inLine((*this->fss).at(1)->getValue()) ||
			AbstractBehaviour::inLine((*this->fss).at(2)->getValue());
}

bool GoToBaseGroup::inPosition(){
	double targetAngle = PI/2;
	if ( this->wheels->getPosition()->getY() < 0.06 )
	    targetAngle = PI;
	else if ( this->wheels->getPosition()->getY() > 0.15 )
	   	targetAngle = 0;

	return fabs( this->wheels->getOrientation() - targetAngle ) < GO_TO_BASE_ORIENTATION_TOLE;
}

} /* End of namespace behaviours */
