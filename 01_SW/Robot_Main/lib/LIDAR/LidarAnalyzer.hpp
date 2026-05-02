#pragma once

#include "mbed.h"
#include "lidar.hpp"        // Ton capteur Lidar
#include "Differentiel.hpp"

class LidarAnalyzer {
public:
    LidarAnalyzer(Lidar* lidar, differentiel* robot, DigitalOut* status_led);

    void update();
    bool isObstacleDetected() const;

    float getObstacleDistance() const;
    float getObstacleAngle() const;
    float getObstacleAngleCible() const;
    float getObstacleX() const;
    float getObstacleY() const;

private:
    bool isAngleInRange(float angle, float min, float max) const;


    Lidar* lidar_;
    differentiel* robot_;
    DigitalOut* led_;

    bool stop_;
    int stableStopCounter_;
    static constexpr int STOP_ON_THRESHOLD  = 1;
    static constexpr int STOP_OFF_THRESHOLD = 20;

    int NbDetecLidarPack;
    int NbNoDetecLidarPack;

    float DistanceLidar;
    float AngleLidar;
    float AngleLidarCible;
    float PointLidarX;
    float PointLidarY;
};