// Class automatically generated by Dev-C++ New Class wizard

#include "trashbinpacket.h" // class's header file

// class constructor
TrashBinPacket::TrashBinPacket(char groupid, char boardid) : BoardPacket(groupid, boardid)
{
}

// class destructor
TrashBinPacket::~TrashBinPacket()
{
	// insert your code here
}

void TrashBinPacket::senseTrashBin(){
	this->setCommand(CMD_TRASHBIN_VALUE);
}

bool TrashBinPacket::isTrashBinFull(){
	return this->getCommand() == CMD_FULL_TRASHBIN_ALARM;
}

void TrashBinPacket::setTrashBinFullThreshold(short value){
	this->setCommand(CMD_FULL_TRASHBIN_VALUE);
}

short TrashBinPacket::getTrashBinValue(){
	return this->getShortData();
}
