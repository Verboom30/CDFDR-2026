#ifndef HOLONOME_HPP
#define HOLONOME_HPP

#include "mbed.h"
#include "pinout.hpp"
#include "Stepper.hpp"
#include <cmath>

#define RADIUS  121.0 // robot wheel-base radius
#define RSTEP   198
#define RWHEEL  27.5 
#define REDUC   0.5f
#define KSTP    float((M_PI*2.0f*RWHEEL/(RSTEP*MSTEP_DRIVE))*REDUC)
#define SPEED   1000.0f // max 50000 Mstepper 16 3200Ma
#define MSTEP_DRIVE 4
#define ACC    4.0f
#define DEC    4.0f

class Holonome
{
public:
    Holonome(Stepper* moteurA, Stepper* moteurB, Stepper* moteurC, bool* StopLidar = nullptr);

    void run();
    void pause();
    void resume();
    void stop();

    void setPosition(int positionX, int positionY, int theta, bool team = false);
    void setPositionZero();
    void resetPosition();

    void move(int moveX, int moveY, int moveTheta, float coefSpeed = 1.0f);
    void Robotmove(int moveX, int moveY, int moveTheta, bool enableLidar = false, float coefSpeed = 1.0f);
    void Robotgoto(int positionX, int positionY, int theta, bool team = false, float coefSpeed = 1.0f);

    void updatePosition();

    bool stopped();
    bool PosCibleDone();

    float getPositionX();
    float getPositionY();
    float getTheta();

    float getPosCibleX();
    float getPosCibleY();

    float getSpeedA();
    float getSpeedB();
    float getSpeedC();

    int getPosA();
    int getPosB();
    int getPosC();

    int getDeltaA();
    int getDeltaB();
    int getDeltaC();

private:
    Stepper* StepperA;
    Stepper* StepperB;
    Stepper* StepperC;

    bool* _stopLidar;

    Thread routineA;
    Thread routineB;
    Thread routineC;
    Thread threadOdometrie;

    EventFlags flags;

    Mutex mutexData;
    Mutex syncMutex;

    Semaphore semA;
    Semaphore semB;
    Semaphore semC;

    int readyCount;

    float _positionX;
    float _positionY;
    float _cibleposX;
    float _cibleposY;
    float _Theta;

    float _MoveX;
    float _MoveY;
    float _MoveTheta;

    float _SpeedX;
    float _SpeedY;
    float _SpeedTheta;

    float _SpeedA;
    float _SpeedB;
    float _SpeedC;

    int _StepA;
    int _StepB;
    int _StepC;

    int lastPosA;
    int lastPosB;
    int lastPosC;

    int _deltaA;
    int _deltaB;
    int _deltaC;

private:
    void synchroniser();

    void routine_moteur_A();
    void routine_moteur_B();
    void routine_moteur_C();
    void routine_odometrie();

    void computeKinematics();

    float normalizeAngle(float angle);
};

#endif