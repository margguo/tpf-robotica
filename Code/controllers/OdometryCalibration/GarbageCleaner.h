#ifndef GarbageCleaner_h
#define GarbageCleaner_h

#include <list>

#include <behaviours/AbstractBehaviour.h>
#include <robotapi/IRobot.h>
#include <WorldInfo.h>

#define DISTANCE_SENSORS 8
#define FLOOR_SENSORS 3
#define TIME_STEP 32

class GarbageCleaner {

 public:

    GarbageCleaner(WorldInfo * wi, robotapi::IRobot &robot);

    void cleanGarbage();

    robotapi::IRobot &myIRobot;

    /**
     * @element-type AbstractBehaviour
     */
    std::list< behaviours::AbstractBehaviour* > myAbstractBehaviours;
    
 private:
	void initializeBehaviours(WorldInfo * wi);
	void initializeSensors();

};

#endif // GarbageCleaner_h
