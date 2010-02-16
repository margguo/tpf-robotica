// Class automatically generated by Dev-C++ New Class wizard

#ifndef BATTERYPACKET_H
#define BATTERYPACKET_H

#include "BoardPacket.h" // inheriting class's header file

#define CMD_BATTERY_VALUE 0x40
#define CMD_FULL_BATTERY_ALARM 0x41
#define CMD_BATTERY_EMPTY_VALUE 0x42
#define CMD_BATTERY_EMPTY_ALARM 0x43

/**
 * No description
 */
class BatteryPacket : public BoardPacket
{
	public:
		// class constructor
		BatteryPacket(char groupid, char boardid);
		// class destructor
		~BatteryPacket();

		void senseBattery();
		bool isBatteryFull();
		void setBatteryEmptyThreshold(short value);
		bool isBatteryEmpty();
		
		short getBatteryValue();
};

#endif // BATTERYPACKET_H
