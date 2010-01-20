#ifndef robotapi_IDifferentialWheels_h
#define robotapi_IDifferentialWheels_h

#include "IDevice.h"

namespace robotapi {

class IDifferentialWheels : virtual public IDevice {

 public:

    virtual void setSpeed(double left, double right)  = 0;

    virtual void enableEncoders(int ms)  = 0;

    virtual void disableEncoders()  = 0;

    virtual double getLeftEncoder()  = 0;

    virtual double getRightEncoder()  = 0;

    virtual void moveWheels(double left, double right)  = 0;

    virtual void moveLeftWheel(double left)  = 0;

    virtual void moveRightWheel(double right)  = 0;

    virtual void moveLeftWheel(double left, double speed)  = 0;

    virtual void moveRightWheel(double right, double speed)  = 0;

    virtual double getMotorConsumption()  = 0;

    virtual double getAverageConsumption()  = 0;

    // virtual destructor for interface
    virtual ~IDifferentialWheels() { }
};

} /* End of namespace robotapi */

#endif // robotapi_IDifferentialWheels_h
