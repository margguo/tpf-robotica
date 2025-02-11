// Class automatically generated by Dev-C++ New Class wizard

#include <protocol/handlers/TrashBinBoardPacketHandler.h> // class's header file
#include <protocol/packets/TrashBinPacket.h>
#include <math.h>

namespace protocol {
namespace handlers {

// class constructor
TrashBinBoardPacketHandler::TrashBinBoardPacketHandler(PacketServer * ps, char groupid, char boardid)
{
	this->groupid = groupid;
	this->boardid = boardid;
	this->ps = ps;
	this->currentValue = 0;
	this->full = false;

#ifdef LINUX
	this->currentValueMutex = new Mutex();
	this->fullMutex = new Mutex();
#endif
}

// class destructor
TrashBinBoardPacketHandler::~TrashBinBoardPacketHandler()
{
	// insert your code here
}

void TrashBinBoardPacketHandler::handlePacket(Packet * p){
	packets::TrashBinPacket * tbp = new packets::TrashBinPacket(groupid,boardid);
	tbp->analysePacket(p);
	
	if ( tbp->getCommand() == CMD_TRASHBIN_VALUE ){
		short value = tbp->getTrashBinValue();
		// TODO convert from short to int
		// Lock Mutex
		#ifdef LINUX
		this->currentMutex->enterMutex();
		#endif
		
		this->currentValue = value;
		
		// Release Mutex
		#ifdef LINUX
		this->currentMutex->leaveMutex();
		#endif

	}
	if ( tbp->getCommand() == CMD_FULL_TRASHBIN_ALARM ){
		// Lock Mutex
		#ifdef LINUX
		this->fullMutex->enterMutex();
		#endif
		this->full = true;
		// Release Mutex
		#ifdef LINUX
		this->fullMutex->leaveMutex();
		#endif

	}
}

int TrashBinBoardPacketHandler::getValue(){
 	packets::TrashBinPacket * p = new packets::TrashBinPacket(groupid,boardid);
	p->senseTrashBin();
	p->prepareToSend();
	this->ps->sendPacket(p);
	return this->currentValue;
}

bool TrashBinBoardPacketHandler::isFull(){
	// Lock Mutex
	#ifdef LINUX
	this->fullMutex->enterMutex();
	#endif

	bool full = this->full;

	// Release mutex
	#ifdef LINUX
	this->fullMutex->leaveMutex();
	#endif

	return full;
}

void TrashBinBoardPacketHandler::setFullBias(double bias){
	packets::TrashBinPacket * p = new packets::TrashBinPacket(groupid,boardid);
	// TODO convert from double to char
	p->setTrashBinFullThreshold(bias);
	p->prepareToSend();
	this->ps->sendPacket(p);
}

}
}

