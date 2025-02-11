// Class automatically generated by Dev-C++ New Class wizard

#include <utils/ArenaGrid.h> // class's header file
#include <utils/MyAngle.h>
#include <ctime>
#include <stdio.h>
#include <math.h>

#define EXCLUDE_ZONE_X -0.15
#define EXCLUDE_ZONE_Z -0.15

namespace utils {

// class constructor
ArenaGrid::ArenaGrid(double arenaMinX, double arenaMinZ, double arenaMaxX, double arenaMaxZ,
					int resolutionX,int resolutionZ)
{
	this->xMin = arenaMinX;
	this->zMin = arenaMinZ;
	this->dx = (arenaMaxX - arenaMinX) / ((double)resolutionX);
	this->dz = (arenaMaxZ - arenaMinZ) / ((double)resolutionZ);
	
	int maxI = this->getI(arenaMaxX);
	int maxJ = this->getJ(arenaMaxZ);

	time_t timestamp = time(NULL);
	for( int i = 0 ; i < maxI ; i++ ){
		for( int j = 0 ; j < maxJ ; j++ ){
			this->slots[i][j] = new utils::ArenaGridSlot(arenaMinX + i*dx,arenaMinZ + j*dz,i,j,timestamp);
		}
	}

	this->excludeSlots = (int)ceil( ((arenaMaxX - EXCLUDE_ZONE_X) / dx) * ((arenaMaxZ - EXCLUDE_ZONE_Z) / dz) );
	this->totalSlots = resolutionX * resolutionZ;
	this->visitedSlots = 0;
	this->initialTimeStamp = timestamp;

	printf("arenaMaxX %g, arenaMinX %g, arenaMaxZ %g, arenaMinZ %g\n",arenaMaxX,arenaMinX,arenaMaxZ,arenaMinZ);
	printf("resolutionX %d, dx %g, resolutionZ %d, dz %g\n",resolutionX,dx,resolutionZ,dz);
	printf("maxI %d, maxJ %d\n",maxI,maxJ);
	this->maxI = maxI;
	this->maxJ = maxJ;

}

// class destructor
ArenaGrid::~ArenaGrid()
{
}

int ArenaGrid::getI(double x){
    //printf("VALOR X: %g - %d\n",floor((x - this->xMin)/this->dx),(int)floor((x - this->xMin)/this->dx));
    return (int)floor((x - this->xMin)/this->dx);
}

int ArenaGrid::getJ(double z){
    //printf("VALOR Z: %g - %d\n",floor((z - this->zMin)/this->dz),(int)floor((z - this->zMin)/this->dz));
	return (int)floor((z - this->zMin)/this->dz);
}

utils::ArenaGridSlot * ArenaGrid::getSlotAt(utils::MyPoint * p){
	int i = this->getI(p->getX());
	int j = this->getJ(p->getY());
	//printf("requested %d, %d\n",i,j);
	return this->getSlotAt(i,j);
}

utils::ArenaGridSlot * ArenaGrid::getSlotAt(int i, int j){
	if ( this->cellOutOfBounds(i,j) )
		return NULL;
 	return this->slots[i][j];
}

int ArenaGrid::getSlotXIdx(utils::ArenaGridSlot * ags){
	if ( ags == NULL )
		return 0;
	return this->getI(ags->getX());
}

int ArenaGrid::getSlotZIdx(utils::ArenaGridSlot * ags){
	if ( ags == NULL )
		return 0;
	return this->getJ(ags->getZ());
}

/* i, j-1 ; i, j+1 ; i-1, j ; i+1, j
	Returns four neighbours, the ones at the left, right, top and bottom, in that order
	Some of they may be NULL, indicating that neighbour is out of the arena
*/

/*
std::vector<utils::ArenaGridSlot *> ArenaGrid::getNeighboursAt(utils::MyPoint * p){
	int i = this->getI(p->getX());
	int j = this->getJ(p->getY());
	std::vector<utils::ArenaGridSlot *> out;

	out.push_back(this->slots[i][j-1]);
	out.push_back(this->slots[i][j+1]);
	out.push_back(this->slots[i-1][j]);
	out.push_back(this->slots[i+1][j]);

	return out;
}
*/

std::vector<utils::ArenaGridSlot *> ArenaGrid::getNeighboursAt(utils::MyPoint * p, double orientation){
	int i = this->getI(p->getX());
	int j = this->getJ(p->getY());
	std::vector<utils::ArenaGridSlot *> out;

	double m = orientation / (PI / 4);
	int n = (int)m;

	if ( m - n > 0.5 )
		n++;

	int a = n%8;
	int b = (n+1)%8;
	int c = (n+2)%8;

	utils::ArenaGridSlot * leftSlot, * centerSlot, * rightSlot = NULL;

	if ( ! this->cellOutOfBounds(i+getIncX(b),j+getIncZ(b)) ){
		centerSlot = new utils::ArenaGridSlot(this->slots[i+getIncX(b)][j+getIncZ(b)],0);
		out.push_back(centerSlot);
	}
	if ( ! this->cellOutOfBounds(i+getIncX(a),j+getIncZ(a)) ){
		rightSlot = new utils::ArenaGridSlot(this->slots[i+getIncX(a)][j+getIncZ(a)],-1);
		out.push_back(rightSlot);
	}
	if ( ! this->cellOutOfBounds(i+getIncX(c),j+getIncZ(c)) ){
		leftSlot = new utils::ArenaGridSlot(this->slots[i+getIncX(c)][j+getIncZ(c)],1);
		out.push_back(leftSlot);
	}

	return out;
}

int ArenaGrid::getIncX(int slot){
	if ( slot > 5 || slot < 1 )
		return 1;

	if ( slot > 1 && slot < 5 )
		return -1;

	return 0;
}

int ArenaGrid::getIncZ(int slot){
	if ( slot < 3 )
		return -1;

	if ( slot > 3 && slot < 7 )
		return 1;

	return 0;
}

bool ArenaGrid::cellOutOfBounds(int i, int j){
	return ( i < 0 || j < 0 || i >= this->maxI || j >= this->maxJ );
}

time_t ArenaGrid::getInitialTimeStamp(){
	return this->initialTimeStamp;
}

void ArenaGrid::setSlotVisited(utils::ArenaGridSlot * ags){
	if ( ags != NULL ){
		this->visitedSlots = this->visitedSlots + 1;
		int left = this->totalSlots - this->excludeSlots - this->visitedSlots;
		printf("Slots left %d\n",left);
/*
		if ( left <= 0 )
			system("pause");
*/
	}
}

int ArenaGrid::getSlotsLeft(){
	return this->totalSlots - this->excludeSlots - this->visitedSlots;
}

int ArenaGrid::getSlotsLeftWOExcluded(){
	return this->totalSlots - this->visitedSlots;
}

int ArenaGrid::getSlotsVisited(){
	return this->visitedSlots;
}

int ArenaGrid::getMaxI(){
	return this->maxI;
}

int ArenaGrid::getMaxJ(){
	return this->maxJ;
}

int ArenaGrid::getIForPoint(utils::MyPoint * p){
	int i = this->getI(p->getX());
	if ( i < 0 )
		return 0;
	if ( i >= this->maxI )
		return this->maxI - 1;
	return i;
}

int ArenaGrid::getJForPoint(utils::MyPoint * p){
	int j = this->getJ(p->getY());
	if ( j < 0 )
		return 0;
	if ( j >= this->maxJ )
		return this->maxJ - 1;
	return j;
}

}
