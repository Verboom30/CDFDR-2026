#include "LidarAnalyzer.hpp"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

// Distance minimale de détection obstacle (mm)
#define LIDAR_DIS_MIN        200.0f

// Distance maximale de détection obstacle (mm)
#define LIDAR_DIS_MAX        800.0f

// Demi-angle de surveillance autour de la direction du déplacement (°)
#define LIDAR_ANGLE_MARGIN   60.0f

// Nombre de paquets LiDAR analysés par cycle
#define NB_LIDAR_PACK_READ   60

// Pourcentage minimum de points détectés pour valider un obstacle
#define LIDAR_PC_ON          5.0f

// Nombre de cycles de détection avant pause robot
#define STOP_ON_THRESHOLD    1

// Nombre de cycles sans obstacle avant reprise robot
#define STOP_OFF_THRESHOLD   20

// Vitesse minimale robot pour activer l'analyse LiDAR
#define SPEED_MIN_LIDAR      1.0f

LidarAnalyzer::LidarAnalyzer(Lidar* lidar,
                             CommandAsserv* asserv,
                             DigitalOut* status_led)
    : lidar_(lidar),
      asserv_(asserv),
      led_(status_led),
      stop_(false),
      stableStopCounter_(0),
      NbDetecLidarPack(0),
      NbNoDetecLidarPack(0),
      DistanceLidar(0.0f),
      AngleLidar(0.0f),
      AngleLidarCible(0.0f),
      PointLidarX(0.0f),
      PointLidarY(0.0f),
      lastMoveAngle_(0.0f)
{
}

float LidarAnalyzer::normalize360(float angle) const
{
    while (angle < 0.0f)
        angle += 360.0f;

    while (angle >= 360.0f)
        angle -= 360.0f;

    return angle;
}

bool LidarAnalyzer::isAngleInRange(float angle, float min, float max) const
{
    angle = normalize360(angle);
    min = normalize360(min);
    max = normalize360(max);

    if (min <= max)
        return angle >= min && angle <= max;

    return angle >= min || angle <= max;
}

void LidarAnalyzer::update()
{
    if (!lidar_ || !asserv_)
        return;

    CommandAsserv::BroadcastData data = asserv_->getBroadcast();

    if (!data.valid)
        return;

    float posX  = (float)data.x;
    float posY  = (float)data.y;
    float alpha = (float)data.alpha;

    float vx = (float)data.vx;
    float vy = (float)data.vy;

    float speedNorm = hypotf(vx, vy);

    /*
     * Si le robot bouge, on met à jour la direction réelle
     * de déplacement dans le repère robot.
     *
     * Si le robot est immobile, on garde le dernier angle connu.
     * Cela permet de continuer à surveiller la zone bloquée et de
     * faire resume() quand l'obstacle disparaît.
     */
    if (speedNorm >= SPEED_MIN_LIDAR)
    {
        float angleMoveTable = atan2f(vy, vx) * 180.0f / M_PI;
        float angleMoveRobot = angleMoveTable - alpha;

        lastMoveAngle_ = normalize360(angleMoveRobot);
    }

    AngleLidarCible = lastMoveAngle_;

    float angleMin = normalize360(AngleLidarCible - LIDAR_ANGLE_MARGIN);
    float angleMax = normalize360(AngleLidarCible + LIDAR_ANGLE_MARGIN);

    NbDetecLidarPack = 0;
    NbNoDetecLidarPack = 0;

    for (uint8_t j = 0; j < NB_LIDAR_PACK_READ; j++)
    {
        LiDARFrameTypeDef points = lidar_->GetPoints();

        for (uint8_t i = 0; i < POINT_PER_PACK; i++)
        {
            const auto& pt = points.point[i];

            float angleDeg = pt.angle / 100.0f;
            float distance = pt.distance;

            if (distance <= LIDAR_DIS_MIN || distance >= LIDAR_DIS_MAX)
            {
                NbNoDetecLidarPack++;
                continue;
            }

            if (!isAngleInRange(angleDeg, angleMin, angleMax))
                continue;

            float globalAngleRad = (angleDeg + alpha) * M_PI / 180.0f;

            float lidarX = posX + cosf(globalAngleRad) * distance;
            float lidarY = posY + sinf(globalAngleRad) * distance;

            if (lidarX < 0.0f || lidarX > 3000.0f ||
                lidarY < 0.0f || lidarY > 2000.0f)
            {
                continue;
            }

            NbDetecLidarPack++;

            DistanceLidar = distance;
            AngleLidar = angleDeg;
            PointLidarX = lidarX;
            PointLidarY = lidarY;
        }
    }

    int total = NbDetecLidarPack + NbNoDetecLidarPack;

    if (total > 0)
    {
        float pctOn = (NbDetecLidarPack * 100.0f) / total;
        bool detection = pctOn > LIDAR_PC_ON;

        if (detection)
        {
            if (stableStopCounter_ < STOP_ON_THRESHOLD)
                stableStopCounter_ += 20;
        }
        else
        {
            if (stableStopCounter_ > -STOP_OFF_THRESHOLD)
                stableStopCounter_--;
        }

        if (stableStopCounter_ >= STOP_ON_THRESHOLD && !stop_)
        {
            stop_ = true;
            asserv_->pause();
        }
        else if (stableStopCounter_ <= -STOP_OFF_THRESHOLD && stop_)
        {
            stop_ = false;
            asserv_->resume();
        }
    }

    if (led_)
        *led_ = stop_;
}

bool LidarAnalyzer::isObstacleDetected() const
{
    return stop_;
}

float LidarAnalyzer::getObstacleDistance() const
{
    return DistanceLidar;
}

float LidarAnalyzer::getObstacleAngle() const
{
    return AngleLidar;
}

float LidarAnalyzer::getObstacleAngleCible() const
{
    return AngleLidarCible;
}

float LidarAnalyzer::getObstacleX() const
{
    return PointLidarX;
}

float LidarAnalyzer::getObstacleY() const
{
    return PointLidarY;
}