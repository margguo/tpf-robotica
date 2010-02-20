// Class automatically generated by Dev-C++ New Class wizard

#include <protocol/packets/DCMotorPacket.h> // class's header file

namespace protocol {
namespace packets {

// class constructor
DCMotorPacket::DCMotorPacket(char groupid, char boardid) : protocol::packets::BoardPacket(groupid, boardid)
{
	// insert your code here
}

// class destructor
DCMotorPacket::~DCMotorPacket()
{
	// insert your code here
}

void DCMotorPacket::setDirection(bool clockwise){
	if ( clockwise )
	    this->setDirection((char)DIR_CLOCKWISE);
	else
	    this->setDirection((char)DIR_COUNTERCLOCKWISE);
}

void DCMotorPacket::setDirection(char dir){
	this->setCommand(CMD_SET_DIRECTION);
	this->addData(dir);
}

void DCMotorPacket::setDCSpeed(bool clockwise, short speed){
	this->setDirection(clockwise);
	this->addData(speed);
	this->setCommand(CMD_SET_DC_SPEED);
}

void DCMotorPacket::getDCSpeed(){
	this->setCommand(CMD_GET_DC_SPEED);
}

void DCMotorPacket::setEncoder(short counts){
	this->addData(counts);
	this->setCommand(CMD_SET_ENCODER);
}

void DCMotorPacket::getEncoder(){
	this->setCommand(CMD_GET_ENCODER);
}

void DCMotorPacket::resetEncoder(){
	this->setCommand(CMD_RESET_ENCODER);
}

void DCMotorPacket::setEncoderToStop(short counts){
	this->addData(counts);
	this->setCommand(CMD_SET_ENCODER_T_S);
}

void DCMotorPacket::getEncoderToStop(){
	this->setCommand(CMD_GET_ENCODER_T_S);
}

void DCMotorPacket::setNonStop(){
	this->setCommand(CMD_DONT_STOP);
}

void DCMotorPacket::getMotorConsumption(){
	this->setCommand(CMD_MOTOR_CONSUMPTION);
}

bool DCMotorPacket::isMotorAlarm(){
	return this->getCommand() == CMD_STRESS_ALARM;
}

bool DCMotorPacket::isMotorShutDown(){
	return this->getCommand() == CMD_SHUTDOWN_ALARM;
}

short DCMotorPacket::getEncoderValue(){
	this->resetDataIdx();
	return this->getShortData();
}

short DCMotorPacket::getEncoderValueToStop(){
	return this->getEncoderValue();
}

short DCMotorPacket::getMotorConsumptionValue(){
	this->resetDataIdx();
	return this->getIntData();
}

short DCMotorPacket::getSpeedValue(){
	this->resetDataIdx();
	return this->getShortData();
}

}
}

