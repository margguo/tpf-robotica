#ifndef behaviours_AvoidObstacle_h
#define behaviours_AvoidObstacle_h

#include "AbstractBehaviour.h"
#include <vector>



namespace behaviours {

class AvoidObstacle : public AbstractBehaviour {

public:
	AvoidObstacle(robotapi::IDifferentialWheels * wheels, std::vector<robotapi::IDistanceSensor*> & dss);

	~AvoidObstacle();
	
    void sense();

    void action();

private:

	std::vector<robotapi::IDistanceSensor*> * dss;
	robotapi::IDifferentialWheels * wheels;

	};

} /* End of namespace behaviours */

#endif // behaviours_AvoidObstacle_h
