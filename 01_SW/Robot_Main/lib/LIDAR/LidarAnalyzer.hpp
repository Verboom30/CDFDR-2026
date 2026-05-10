#ifndef LIDAR_ANALYZER_HPP
#define LIDAR_ANALYZER_HPP

#include "mbed.h"
#include "lidar.hpp"
#include "CommandAsserv.hpp"

class LidarAnalyzer
{
public:
    LidarAnalyzer(Lidar* lidar,
                  CommandAsserv* asserv,
                  DigitalOut* status_led = nullptr);

    void update();

    bool isObstacleDetected() const;

    float getObstacleDistance() const;
    float getObstacleAngle() const;
    float getObstacleAngleCible() const;

    float getObstacleX() const;
    float getObstacleY() const;

private:
    bool isAngleInRange(float angle, float min, float max) const;
    float normalize360(float angle) const;

private:
    Lidar* lidar_;
    CommandAsserv* asserv_;
    DigitalOut* led_;

    bool stop_;
    int stableStopCounter_;

    int NbDetecLidarPack;
    int NbNoDetecLidarPack;

    float DistanceLidar;
    float AngleLidar;
    float AngleLidarCible;

    float PointLidarX;
    float PointLidarY;

    float lastMoveAngle_;
};

#endif