// Class automatically generated by Dev-C++ New Class wizard

#include "servomotorpacket.h" // class's header file

// class constructor
ServoMotorPacket::ServoMotorPacket() : GroupPacket(SERVOMOTORPACKET_ID)
{
}

// class destructor
ServoMotorPacket::~ServoMotorPacket()
{
	// insert your code here
}

void ServoMotorPacket::setPosition(char servoId, char angle){
	this->setCommand(CMD_SET_POSITION);
	this->addData(servoId);
	this->addData(angle);
}

void ServoMotorPacket::setPosition(char * angle, int qty){
	if ( qty < 1 )
		return;

	int i;
	for( i = 0 ; i < qty ; i++ )
		this->addData(angle[i]);
	this->setCommand(CMD_SET_ALL_POSITIONS);
}

void ServoMotorPacket::getPosition(char servoId){
	this->setCommand(CMD_GET_POSITION);
}

void ServoMotorPacket::getPosition(){
	this->setCommand(CMD_GET_ALL_POSITIONS);
}

void ServoMotorPacket::setSpeed(char servoId, char angle){
	this->setPosition(servoId,angle);
	this->setCommand(CMD_SET_SPEED);
}

void ServoMotorPacket::setSpeed(char * angle, int qty){
	this->setPosition(angle,qty);
	this->setCommand(CMD_SET_ALL_SPEEDS);
}

void ServoMotorPacket::getSpeed(char servoId){
	this->setCommand(CMD_GET_SPEED);
}

void ServoMotorPacket::getSpeed(){
	this->setCommand(CMD_GET_ALL_SPEEDS);
}

void ServoMotorPacket::freeServo(char servoId){
   	this->setCommand(CMD_FREE_SERVO);
}

void ServoMotorPacket::freeAllServos(){
	this->setCommand(CMD_FREE_ALL_SERVOS);
}


char ServoMotorPacket::getPositionValue(){
	return this->getCharData();
}

char * ServoMotorPacket::getPositionValues(){
	return this->getData();
}

char ServoMotorPacket::getSpeedValue(){
	return this->getCharData();
}

char * ServoMotorPacket::getSpeedValues(){
	return this->getData();
}
