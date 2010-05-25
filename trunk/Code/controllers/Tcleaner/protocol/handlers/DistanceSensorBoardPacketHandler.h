// Class automatically generated by Dev-C++ New Class wizard

#ifndef DISTANCESENSORBOARDPACKETHANDLER_H
#define DISTANCESENSORBOARDPACKETHANDLER_H

#include <protocol/BoardPacketHandler.h> // inheriting class's header file
#include <protocol/Packet.h>
#include <protocol/PacketServer.h>

namespace protocol {
namespace handlers {

/**
 * No description
 */
class DistanceSensorBoardPacketHandler : public protocol::BoardPacketHandler
{
	public:
		// class constructor
		DistanceSensorBoardPacketHandler(PacketServer * ps, char groupid, char boardid);
		// class destructor
		~DistanceSensorBoardPacketHandler();
		
		void handlePacket(Packet * p);
		
		void enable(int dsId);

		void disable(int dsId);
		
		int getValue(int dsIds);
		
		int getOneValue(int dsIds);
		
		void setMask(int dsId);
		
		int getMask();
		
	private:
		char groupid;
		char boardid;
		
		PacketServer * ps;

#ifdef LINUX
		Mutex::Mutex dsValueMutex;
		Mutex::Mutex dsMaskMutex;
#endif
		int dsValue[6];
		int dsMask;
		
};

}
}


#endif // DISTANCESENSORBOARDPACKETHANDLER_CPP
