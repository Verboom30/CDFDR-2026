#include "LidarAnalyzer.hpp"
#include <cmath>

#define LIDAR_DIS_MIN        200.0f
#define LIDAR_DIS_MAX        800.0f
#define LIDAR_ANGLE_MARGIN   60.0f
#define NB_LIDAR_PACK_READ   (LIDAR_ANGLE_MARGIN * 1)
#define LIDAR_PC_ON          5.0f


LidarAnalyzer::LidarAnalyzer(Lidar* lidar, differentiel* robot, DigitalOut* status_led)
    : lidar_(lidar), robot_(robot), led_(status_led), stop_(false),
      NbDetecLidarPack(0), NbNoDetecLidarPack(0),
      DistanceLidar(0), AngleLidar(0), AngleLidarCible(0), PointLidarX(0), PointLidarY(0)
{}

bool LidarAnalyzer::isAngleInRange(float angle, float min, float max) const {
    return (min < max) ? (angle >= min && angle <= max)
                       : (angle >= min || angle <= max);
}

void LidarAnalyzer::update() {
    NbDetecLidarPack = 0;
    NbNoDetecLidarPack = 0;

    float posX   = robot_->getPositionX();
    float posY   = robot_->getPositionY();
    float alpha  = robot_->getAlpha();
    float cibleX = robot_->getPosCibleX();
    float cibleY = robot_->getPosCibleY();

    float deltaX = cibleX - posX;
    float deltaY = cibleY - posY;
    float distToTarget = hypotf(deltaX, deltaY);
    printf("[LIDAR] cibleX (%f) cibleY(%f)\n", cibleX,cibleY);

    if (distToTarget <= 1.0f){
        stop_ = false;
        return;
    }  

    AngleLidarCible = atan2f(deltaX, deltaY) * 180.0f / M_PI - alpha;
    if (AngleLidarCible < 0) AngleLidarCible += 360.0f;

    float angleMin = AngleLidarCible - LIDAR_ANGLE_MARGIN;
    float angleMax = AngleLidarCible + LIDAR_ANGLE_MARGIN;
    if (angleMin < 0) angleMin += 360.0f;
    if (angleMax >= 360.0f) angleMax -= 360.0f;

    for (uint8_t j = 0; j < NB_LIDAR_PACK_READ; ++j) {
        auto points = lidar_->GetPoints();

        for (uint8_t i = 0; i < POINT_PER_PACK; ++i) {
            const auto& pt = points.point[i];
            float angleDeg = pt.angle / 100.0f;

            //if (pt.intensity <= 180) continue;
          
           
            if (pt.distance <= LIDAR_DIS_MIN || pt.distance >= LIDAR_DIS_MAX) {
                NbNoDetecLidarPack++;
                continue;
            }

            if (!isAngleInRange(angleDeg, angleMin, angleMax)) continue;
            float globalAngleRad = (angleDeg + alpha) * M_PI / 180.0f;
            float lidarX = posX + sinf(globalAngleRad) * pt.distance;
            float lidarY = posY + cosf(globalAngleRad) * pt.distance;

            if (lidarX < 0 || lidarX > 3000 || lidarY < 0 || lidarY > 2000) continue;

            
            //printf("%.2f;%5d\r\n",points.point[i].angle/100.0f,points.point[i].distance);
          
            NbDetecLidarPack ++;
            DistanceLidar = pt.distance;
            AngleLidar    = angleDeg;
            PointLidarX   = lidarX;
            PointLidarY   = lidarY;
        }
    }
    //printf("[LIDAR] NbDetecLidarPack (%d) [LIDAR] NbNoDetecLidarPack (%d) \n", NbDetecLidarPack, NbNoDetecLidarPack);
    int total = NbDetecLidarPack + NbNoDetecLidarPack;
    if (total > 0) {
        float pctOn = (NbDetecLidarPack * 100.0f) / total;
       // printf("[LIDAR] pctOn (%.1f%%)\n", pctOn);
        bool detection = (pctOn > LIDAR_PC_ON);

        if (detection) {
            if (stableStopCounter_ < STOP_ON_THRESHOLD) {
                stableStopCounter_+=20;
            }
        } else {
            if (stableStopCounter_ > -STOP_OFF_THRESHOLD) {
                stableStopCounter_--;
            }
        }
        //printf("[LIDAR] stableStopCounter_ (%d)\n", stableStopCounter_);

        if (stableStopCounter_ >= STOP_ON_THRESHOLD && !stop_) {
            stop_ = true;
            //printf("[LIDAR] Obstacle confirmé — Stop = TRUE (%.1f%%)\n", pctOn);
        } else if (stableStopCounter_ <= -STOP_OFF_THRESHOLD && stop_) {
            stop_ = false;
            //printf("[LIDAR] Zone dégagée — Stop = FALSE\n");
        }
    }

    if (led_) *led_ = stop_;
}

bool LidarAnalyzer::isObstacleDetected() const {
    return stop_;
}

float LidarAnalyzer::getObstacleDistance() const {
    return DistanceLidar;
}

float LidarAnalyzer::getObstacleAngle() const {
    return AngleLidar;
}

float LidarAnalyzer::getObstacleAngleCible() const {
    return AngleLidarCible;
}

float LidarAnalyzer::getObstacleX() const {
    return PointLidarX;
}

float LidarAnalyzer::getObstacleY() const {
    return PointLidarY;
}
