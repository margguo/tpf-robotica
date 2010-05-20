// Class automatically generated by Dev-C++ New Class wizard

#ifndef utils_ArenaGrid_h
#define utils_ArenaGrid_h
#include <iostream>
#include <map>
#include <vector>
#include <utils/ArenaGridSlot.h>
#include <utils/MyPoint.h>

namespace utils {

/**
 * No description
 */
class ArenaGrid
{
	public:
		// class constructor
		ArenaGrid( double arenaMinX, double arenaMinZ, double arenaMaxX, double arenaMaxZ,
					int resolutionX,int resolutionZ);
		// class destructor
		~ArenaGrid();

		utils::ArenaGridSlot * getSlotAt(utils::MyPoint * p);
		std::vector<utils::ArenaGridSlot *> getNeighboursAt(utils::MyPoint * p, double orientation);
/*
		std::vector<utils::ArenaGridSlot *> getNeighboursAt(utils::MyPoint * p);
*/

	private:
		double dx;
		double dz;
		double xMin;
		double zMin;


		int getI(double x);
		int getJ(double z);

		int getIncX(int slot);
		int getIncZ(int slot);
		std::map<int, std::map<int, utils::ArenaGridSlot *> > slots;
		int maxI;
		int maxJ;
		bool cellOutOfBounds(int i, int j);
};

}
#endif // utils_ArenaGrid_h
