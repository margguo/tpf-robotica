// Class automatically generated by Dev-C++ New Class wizard

#include <utils/ArenaGridSlot.h> // class's header file
#include <string.h>

namespace utils {

// class constructor
ArenaGridSlot::ArenaGridSlot(double x, double z, time_t timestamp)
{
	this->x = x;
	this->z = z;
	memcpy(&this->timestamp,&timestamp,sizeof(time_t));
}

time_t ArenaGridSlot::getTimeStamp(){
	return this->timestamp;
}

void ArenaGridSlot::setTimeStamp(){
	this->setTimeStamp(time(NULL));
}

void ArenaGridSlot::setTimeStamp(time_t timestamp){
	this->timestamp = timestamp;
}

double ArenaGridSlot::getX(){
	return this->x;
}

double ArenaGridSlot::getZ(){
	return this->z;
}

// class destructor
ArenaGridSlot::~ArenaGridSlot()
{
	// insert your code here
}

}
