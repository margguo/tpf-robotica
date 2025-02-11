// Class automatically generated by Dev-C++ New Class wizard

#include <protocol/Packet.h> // class's header file
#include <string.h>
#include <stdio.h>

namespace protocol {

// class constructor
Packet::Packet()
{
	this->actualLength = (char)MIN_PACKET_SIZE;
	this->packet[0] = this->actualLength - 1;
	this->resetDataIdx();
	// insert your code here
}

Packet::Packet(char * data, unsigned char length){
	memcpy(this->packet,data,length);
	this->actualLength = data[0] + 1;
	this->resetDataIdx();
}

void Packet::refresh(){
	this->actualLength = packet[0] + 1;
	this->resetDataIdx();
}

// class destructor
Packet::~Packet()
{
	// insert your code here
}

char * Packet::getPacket(){
	return packet;
}

char Packet::getActualLength(){
	return actualLength;
}

char Packet::getOriginGroup(){
	return this->getGroup(ORIGIN_FIELD);
}

char Packet::getOriginId(){
	return this->getId(ORIGIN_FIELD);
}

char Packet::getDestinationGroup(){
	return this->getGroup(DESTINY_FIELD);
}

char Packet::getDestinationId(){
	return this->getId(DESTINY_FIELD);
}

void Packet::setOriginGroup(char group){
	return this->setGroup(ORIGIN_FIELD, group);
}

void Packet::setDestinationGroup(char group){
	return this->setGroup(DESTINY_FIELD, group);
}

void Packet::setOriginId(char id){
	return this->setId(ORIGIN_FIELD,id);
}

void Packet::setDestinationId(char id){
	return this->setId(DESTINY_FIELD, id);
}

char Packet::getGroup(int field){
	char aux = packet[field];
	return aux >> 4;
}

char Packet::getId(int field){
	char aux = packet[field];
	return aux & 0x0F;
}

void Packet::setGroup(int field, char group){
	// Clear Group
	packet[field] &= 0x0F;

	// Set Group
	char aux = group << 4;
	packet[field] |= aux;
}

void Packet::setId(int field, char id){
	// Clear Id
	packet[field] &= 0xF0;

	// Set Id
	char aux = id & 0x0F;
	packet[field] |= aux;
}

bool Packet::checkCRC(){
	int length = this->actualLength - 1;
	char calculated = this->packet[0];
	int i;
	for ( i = 1 ; i < length ; i++ )
	    calculated ^= this->packet[i];
	return this->packet[i] == calculated;
}

void Packet::calculateCRC(){
	int length = this->actualLength - 1;
	char calculated = this->packet[0];
	int i;
	for ( i = 1 ; i < length ; i++ )
	    calculated ^= this->packet[i];
	this->packet[i] = calculated;
}

void Packet::setBroadcast(){
	this->setDestinationGroup(0x0F);
	this->setGroupBroadcast();
}

void Packet::setGroupBroadcast(){
   	this->setDestinationId(0x0F);
}

bool Packet::isResponse(){
	return (this->packet[COMMAND_FIELD] & 0x80 ) == 0x80;
}

void Packet::setResponse(){
	this->packet[COMMAND_FIELD] = this->packet[COMMAND_FIELD] | 0x80;
}

void Packet::setCommand(char command){
	this->packet[COMMAND_FIELD] = command & 0x7F;
}

char Packet::getCommand(){
	return this->packet[COMMAND_FIELD];// & 0x7F;
}

//TODO: Habria qeu tomar ACK como cualquier respuesta y no solo las qeu vienen como dato
bool Packet::isACK(){
	return this->getDataLength() == 0;
}

char * Packet::getData(){
	return &(this->packet[DATA_FIELD]);
}

char Packet::getDataLength(){
	return this->actualLength - MIN_PACKET_SIZE;
}

void Packet::addData(char * data, char length){
	if ( length < 1 )
	    return;

	char i;
	for( i = 0 ; i < length ; i++ )
	    this->addData(data[i]);

}

void Packet::addData(short data){
	this->addData((char)(data & 0x00FF));
	this->addData((char)((data & 0xFF00) >> 8 ));
}

void Packet::addData(int data){
	this->addData((char)(data & 0x000000FF));
	this->addData((char)((data & 0x0000FF00) >> 8));
	this->addData((char)((data & 0x00FF0000) >> 16));
	this->addData((char)((data & 0xFF000000) >> 24));
}

void Packet::addData(char data){
	//printf(" addData char %c \n",30+data);
    packet[this->dataIdx] = data;
    this->dataIdx++;
	this->actualLength += 1;
	this->packet[0]++;
}

void Packet::resetDataIdx(){
	this->dataIdx = DATA_FIELD;
}

void Packet::resetDataIdx(int offset){
	this->dataIdx = offset + DATA_FIELD;
}

void Packet::clear(){
	memset(this->packet+COMMAND_FIELD,'\0',this->actualLength-COMMAND_FIELD);
	this->resetDataIdx();
}

char Packet::getCharData(){
    return packet[this->dataIdx++];
}

short Packet::getShortData(){
    short up = packet[this->dataIdx++] & 0x00FF;
    short down = packet[this->dataIdx++] & 0x00FF;
    short ret = 0xFFFF;
    ret &= (( down << 8  ) | 0x00FF);
    ret &= up | 0xFF00;
    return ret;
}

//TODO
int Packet::getIntData(){
    int upup = packet[this->dataIdx++] & 0x00FF;
    int up = packet[this->dataIdx++] & 0x00FF;
    int down = packet[this->dataIdx++] & 0x00FF;
    int downdown = packet[this->dataIdx++] & 0x00FF;
    int ret = 0xFFFFFFFF;
    ret &= (( downdown << 24  ) | 0x00FFFFFF);
    ret &= (( down << 16 )| 0xFF00FFFF);
    ret &= (( up << 8 )| 0xFFFF00FF);
    ret &= (upup |  0xFFFFFF00);
    return ret;
}

void Packet::print(){
	for(int i = 0; i <this->getActualLength(); i++ )
		printf("%02X:",( (char) this->packet[i]) & 0x000000FF);
	putchar('\n');
	fflush(stdout);
}

}
