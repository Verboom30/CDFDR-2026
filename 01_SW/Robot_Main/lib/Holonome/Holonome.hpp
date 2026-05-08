#ifndef HOLONOME_HPP
#define HOLONOME_HPP

#include "mbed.h"
#include "pinout.hpp"
#include "Stepper.hpp"
#include <cmath>

#define RADIUS       121.0f
#define RSTEP        198
#define RWHEEL       27.5f
#define REDUC        0.5f

#define MSTEP_DRIVE  4

#define KSTP float((M_PI * 2.0f * RWHEEL / (RSTEP * MSTEP_DRIVE)) * REDUC)

#define SPEED        1000.0f
#define ACC          4.0f
#define DEC          4.0f

class Holonome
{
public:

    Holonome(
        Stepper* moteurA,
        Stepper* moteurB,
        Stepper* moteurC,
        bool* StopLidar = nullptr
    );

    //==================================================
    // CONTROL
    //==================================================

    void run();
    void pause();
    void resume();
    void stop();

    //==================================================
    // POSITION
    //==================================================

    void setPosition(
        int positionX,
        int positionY,
        int theta,
        bool team = false
    );

    void setPositionZero();
    void resetPosition();

    //==================================================
    // MOVEMENT
    //==================================================

    void move(
        int moveX,
        int moveY,
        int moveTheta,
        float coefSpeed = 1.0f
    );

    void Robotmove(
        int moveX,
        int moveY,
        int moveTheta,
        bool enableLidar = false,
        float coefSpeed = 1.0f
    );

    void Robotgoto(
        int positionX,
        int positionY,
        int theta,
        bool team = false,
        float coefSpeed = 1.0f
    );

    //==================================================
    // ODOMETRY
    //==================================================

    void updatePosition();

    //==================================================
    // STATUS
    //==================================================

    bool stopped();
    bool PosCibleDone();

    //==================================================
    // GETTERS POSITION
    //==================================================

    float getPositionX();
    float getPositionY();
    float getTheta();

    float getPosCibleX();
    float getPosCibleY();

    //==================================================
    // GETTERS SPEED
    //==================================================

    float getSpeedA();
    float getSpeedB();
    float getSpeedC();

    //==================================================
    // GETTERS POSITION MOTEURS
    //==================================================

    int getPosA();
    int getPosB();
    int getPosC();

    //==================================================
    // GETTERS DELTA
    //==================================================

    int getDeltaA();
    int getDeltaB();
    int getDeltaC();

private:

    //==================================================
    // STEPPERS
    //==================================================

    Stepper* StepperA;
    Stepper* StepperB;
    Stepper* StepperC;

    //==================================================
    // LIDAR
    //==================================================

    bool* _stopLidar;

    //==================================================
    // THREADS
    //==================================================

    Thread routineMove;
    Thread threadOdometrie;

    EventFlags flags;

    //==================================================
    // MUTEX
    //==================================================

    Mutex mutexData;

    //==================================================
    // POSITION ROBOT
    //==================================================

    float _positionX;
    float _positionY;

    float _cibleposX;
    float _cibleposY;

    float _Theta;

    //==================================================
    // CONSIGNES
    //==================================================

    float _MoveX;
    float _MoveY;
    float _MoveTheta;

    float _SpeedX;
    float _SpeedY;
    float _SpeedTheta;

    //==================================================
    // CINEMATIQUE
    //==================================================

    float _SpeedA;
    float _SpeedB;
    float _SpeedC;

    int _StepA;
    int _StepB;
    int _StepC;

    //==================================================
    // ODOMETRIE
    //==================================================

    int lastPosA;
    int lastPosB;
    int lastPosC;

    int _deltaA;
    int _deltaB;
    int _deltaC;

private:

    //==================================================
    // THREADS
    //==================================================

    void routine_mouvement();
    void routine_odometrie();

    //==================================================
    // INTERNALS
    //==================================================

    void computeKinematics();

    float normalizeAngle(float angle);
};

#endif