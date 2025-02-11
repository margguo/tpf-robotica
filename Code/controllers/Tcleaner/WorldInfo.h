#ifndef WorldInfo_h
#define WorldInfo_h

#include <utils/MyLine.h>
#include <utils/MyWall.h>
#include <utils/MyPoint.h>
#include <utils/ArenaGrid.h>
#include <stdio.h>
#include <list>
#define CONFIG_FILE "worldInfo.cfg"

class WorldInfo {

	public:

    	WorldInfo();

		utils::MyLine * getCurrentLine();
		utils::MyPoint * getInitialPosition();
		utils::MyAngle * getInitialOrientation();
		double getDistanceBetweenWheels();
		double getLeftWheelRadius();
		double getRightWheelRadius();
		double getDistanceBetweenWheelsFactor();
		double getLeftWheelFactor();
		double getRightWheelFactor();
		double getEncoderResolution();
		double getCameraFOVH();
		double getCameraFOVV();
		double getCameraX();
		double getCameraY();
		double getCameraAngle();
		int getCameraImageWidth();
		int getCameraImageHeight();
		utils::ArenaGrid * getArenaGrid();

		double getMaximumDistance();
		double getMinimumDistance();
		double getDistance(double angle);
	private:
		std::list< utils::MyWall * > walls;
		std::list< utils::MyLine * > lines;
		utils::MyPoint * ip;
		utils::MyAngle * ia;
		double distanceBetweenWheels;
		double distanceBetweenWheelsFactor;
		double leftWheelRadius;
		double leftWheelFactor;
		double rightWheelRadius;
		double rightWheelFactor;
		double encoderResolution;
		double cameraFOV;
		double cameraX;
		double cameraY;
		double cameraAngle;
		int cameraImageHeight;
		int cameraImageWidth;
		double arenaMinX;
		double arenaMinZ;
		double arenaMaxX;
		double arenaMaxZ;
		int resolutionX;
		int resolutionZ;
		utils::ArenaGrid * arena;

		double gamma;

		void readWalls(FILE * f);
		void readLines(FILE * f);
		void readDistanceBetweenWheels(FILE * f);
		void readDistanceBetweenWheelsFactor(FILE * f);
		void readWheelsRadius(FILE * f);
		void readWheelsFactors(FILE * f);
		void readInitialPosition(FILE * f);
		void readInitialAngle(FILE * f);
		void readEncoderResolution(FILE * f);
		void readCameraFOV(FILE * f);
		void readCameraPosition(FILE * f);
		void readCameraAngle(FILE * f);
		void readCameraImageSize(FILE * f);
		void readArenaBoundaries(FILE * f);
		void readGridResolutions(FILE * f);
		
};

#endif // WorldInfo_h
