// Class automatically generated by Dev-C++ New Class wizard

#include <protocol/BoardPacketHandler.h> // class's header file

bool isBitSet(int val,int index);


// Implementation

bool isBitSet(int val,int index) {
	if(val & ( 0x01 << index)){
		return true;
	}
	
	return false;
}
