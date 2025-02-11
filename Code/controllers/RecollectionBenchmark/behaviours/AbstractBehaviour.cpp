#include "AbstractBehaviour.h"
#include <robotapi/IRobot.h>
#include <robotapi/IDistanceSensor.h>
#include <vector>
#include <sstream>
#include<stdio.h>

namespace behaviours {

	int nextid = 0;
    robotapi::IRobot * myIRobot = NULL;
	int stimuli_present = 0;

	AbstractBehaviour::AbstractBehaviour(char * name){
		behaviour_id = nextid;
		if ( nextid == 0 )
			nextid = 1;
		else
		    nextid = 2 * nextid;
   		std::stringstream lStream;
		lStream << name;
		lStream << " , Id: ";
		lStream << behaviour_id;
		this->s.append(lStream.str());

	}

    std::string AbstractBehaviour::toString(){
		return s;
	}
    
	AbstractBehaviour::~AbstractBehaviour(){}

	void AbstractBehaviour::act()
	// don't delete the following line as it's needed to preserve source code of this autogenerated element
	// section -64--88-1-100--1230920c:125cb7ecd6f:-8000:0000000000000DCB begin
	{
		if ( isMyAction() ){
			/*
			if ( lastStimuli != behaviour_id ){
				setBehaviourChanged(true);
				backupLastStimuli(lastStimuli);
			}
			else
				setBehaviourChanged(false);

			setLastStimuli(behaviour_id);
			*/
			#ifdef CONTROLLER_DEBUG
			printf("Stimulus Present: %s\n",this->s.c_str());
			#endif
			action();
		}
	}
	// section -64--88-1-100--1230920c:125cb7ecd6f:-8000:0000000000000DCB end
	// don't delete the previous line as it's needed to preserve source code of this autogenerated element

	bool AbstractBehaviour::isMyAction()
	// don't delete the following line as it's needed to preserve source code of this autogenerated element
	// section -64--88-1-100--1230920c:125cb7ecd6f:-8000:0000000000000DCD begin
	{
		int aux = stimuli_present - behaviour_id;
		return aux == 0 || ( aux > 0 && aux < behaviour_id);
	}
	// section -64--88-1-100--1230920c:125cb7ecd6f:-8000:0000000000000DCD end
	// don't delete the previous line as it's needed to preserve source code of this autogenerated element

	void AbstractBehaviour::setStimulusPresent(){
		stimuli_present = stimuli_present | behaviour_id;
	}

	void AbstractBehaviour::resetStimulusPresent(){
		stimuli_present = 0;
	}

	bool AbstractBehaviour::inLine(double value){
		return value > behaviours::BehavioursParameters::getParameter(LINE_DOWN_THRESHOLD) && value < behaviours::BehavioursParameters::getParameter(LINE_UP_THRESHOLD);
	}
} /* End of namespace behaviours */
