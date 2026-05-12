#ifndef HOLONOME_HPP
#define HOLONOME_HPP

#include "mbed.h"
#include "pinout.hpp"
#include "Stepper.hpp"
#include "main_pck.hpp"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

#define RADIUS       130.0f
#define RSTEP        198
#define RWHEEL       27.5f
#define REDUC        0.5f
#define MSTEP_DRIVE  8

#define KSTP_RAW float((M_PI * 2.0f * RWHEEL / (RSTEP * MSTEP_DRIVE)) * REDUC)
#define KSTP     (KSTP_RAW * 1.02086f)

#define SPEED        1000.0f
#define ACC          4.0f
#define DEC          4.0f

class Holonome
{
public:
    Holonome(Stepper* moteurA, Stepper* moteurB, Stepper* moteurC, bool* StopLidar = nullptr);

    void run();
    void pause();
    void resume();
    void stop();

    void setPosition(int positionX, int positionY, int theta, Team team = YELLOW);
    void setPositionZero();
    void resetPosition();

    void move(int moveX, int moveY, int moveTheta, float coefSpeed = 1.0f);
    void Robotmove(int moveX, int moveY, int moveTheta, bool enableLidar = false, float coefSpeed = 1.0f);
    void Robotgoto(int positionX, int positionY, int theta, Team team = YELLOW, float coefSpeed = 1.0f);

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

    Thread routineMove;
    Thread threadOdometrie;

    EventFlags flags;
    Mutex mutexData;

    float _positionX;
    float _positionY;
    float _Theta;

    float _positionX_Save;
    float _positionY_Save;
    float _Theta_Save;

    float _cibleposX;
    float _cibleposY;

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
    void routine_mouvement();
    void routine_odometrie();
    void computeKinematics();

    float normalizeAngle(float angle);
};

#endif