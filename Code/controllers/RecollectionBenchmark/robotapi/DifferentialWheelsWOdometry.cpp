#include "DifferentialWheelsWOdometry.h"
#include <math.h>

namespace robotapi {

DifferentialWheelsWOdometry::DifferentialWheelsWOdometry(WorldInfo * wi){
 		this->distanceBetweenWheels = wi->getDistanceBetweenWheels() * wi->getDistanceBetweenWheelsFactor();
		this->leftWheelRadius = wi->getLeftWheelRadius() * wi->getLeftWheelFactor();
		this->rightWheelRadius = wi->getRightWheelRadius() * wi->getRightWheelFactor();
		this->encoderResolution = wi->getEncoderResolution();
		this->lastPosition = wi->getInitialPosition();
		this->lastAngle = wi->getInitialOrientation();
		this->lastLeftEncoder = this->getLeftEncoder();
		this->lastRightEncoder = this->getRightEncoder();
}

double DifferentialWheelsWOdometry::getLeftEncoder(){return 0.0;}
double DifferentialWheelsWOdometry::getRightEncoder(){return 0.0;}

void DifferentialWheelsWOdometry::computeOdometry() {
	double l = this->getLeftEncoder();
	double r = this->getRightEncoder();
	/*
	printf("COMPUTE START\n");
	printf("%g - %g\n",this->lastLeftEncoder,this->lastLeftEncoder);
	printf("%g - %g\n",l,r);
	printf("%g - %g - %g\n",this->leftWheelRadius, this->leftWheelFactor, this->encoderResolution);
	*/
	double dl = (l - this->lastLeftEncoder ) * this->leftWheelRadius / this->encoderResolution ; // distance covered by left wheel in meter
	double dr = (r - this->lastRightEncoder ) * this->rightWheelRadius / this->encoderResolution ; // distance covered by right wheel in meter
	//printf("%g - %g\n",dl,dr);
	this->computePosition(dl, dr);

	this->lastLeftEncoder = l;
	this->lastRightEncoder = r;
	this->distanceCovered = 0;

	//printf("estimated distance covered by left wheel: %g m.\n",dl);
	//printf("estimated distance covered by right wheel: %g m.\n",dr);
	//printf("estimated change of orientation: %g rad.\n",da);
//	printf("COMPUTE END\n");
}

void DifferentialWheelsWOdometry::computePosition(double ldist, double rdist){
	/*
	double lc = 0;
	lc = (ldist+rdist)/2.0;

	utils::MyAngle * dtita = new utils::MyAngle((rdist-ldist)/this->distanceBetweenWheels); // delta orientation
	this->lastAngle->add(dtita);
	delete dtita;

	this->lastPosition->add(lc * cos(this->lastAngle->getNormalizedValue()), lc * sin(this->lastAngle->getNormalizedValue()));
	return;
	*/
	double lc = 0;
	lc = (ldist+rdist)/2.0;

	utils::MyAngle * dtita = new utils::MyAngle((rdist-ldist)/this->distanceBetweenWheels); // delta orientation
	this->lastAngle->add(dtita);
	delete dtita;

	double xdiff = -lc * sin(this->lastAngle->getNormalizedValue());
	double zdiff = -lc * cos(this->lastAngle->getNormalizedValue());
	this->distanceCovered = sqrt( xdiff * xdiff + zdiff * zdiff );

	this->lastPosition->add(xdiff, zdiff);
	return;
}

double DifferentialWheelsWOdometry::getOrientation(){
	return this->lastAngle->getNormalizedValue();
}

utils::MyPoint * DifferentialWheelsWOdometry::getPosition(){
	return this->lastPosition;
}

void DifferentialWheelsWOdometry::setOrientation(double angle){
	delete(this->lastAngle);
	this->lastAngle = new utils::MyAngle(angle);
}

void DifferentialWheelsWOdometry::setPosition(double x, double z, bool s){
    this->lastPosition->setX(x);
	this->lastPosition->setY(z);
}

double DifferentialWheelsWOdometry::getDistanceCovered(){
	return this->distanceCovered;
}

void DifferentialWheelsWOdometry::resetDistanceCovered(){
    this->distanceCovered = 0;
}

} /* End of namespace robotapi */
