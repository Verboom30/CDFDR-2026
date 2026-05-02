#ifndef HOLONOME_H
#define HOLONOME_H

#include "mbed.h"
#include "pinout.hpp"
#include "Stepper.hpp"
#include "main_pck.hpp"
#include <string.h>
#include <cmath>



#define RADIUS  84.00f // robot wheel-base radius
#define RSTEP   198
#define RWHEEL  35.0f
#define REDUC   0.5f
#define KSTP    float((M_PI*2.0f*RWHEEL/(RSTEP*MSTEP_DRIVE))*REDUC)
#define SPEED   1500.0f // max 50000 Mstepper 16 3200Ma
#define MSTEP_DRIVE 4
#define ACC    3.0f
#define DEC    3.0f

class differentiel
{
    public:
    differentiel(Stepper* moteurGauche, Stepper* moteurDroit, bool* StopLidar);

    void run();
    void stop();
    void pause();
    void resume();
    
    void move(int distance, int Alpha, float coefSpeed); 
    void setPosition(int positionX, int positionY, int Alpha, bool team);
    void setPositionZero();
    void resetPosition(); 

    float getSpeedG();
    float getSpeedD();
    int getPosG();
    int getPosD();
    int getStepG();
    int getStepD();
    int getDeltaG();
    int getDeltaD();

    float getPositionX();
    float getPositionY();
    float getPosCibleX();
    float getPosCibleY();
    float getAlpha();
    float getSpeed();
    float getSpeedAlpha();

    bool stopped();
    bool PosCibleDone();
    void updatePosition();

    void Robotmoveto(int distance, int alpha, bool enableLidar,float coefSpeed );
    void Robotgoto(int positionX, int positionY, int alpha, bool team, float coefSpeed);
   

private : 
    // Moteurs
    Stepper* StepperG;
    Stepper* StepperD;
    bool* _stopLidar;

    // Threads et synchro
    Thread routineG;
    Thread routineD;
    Thread threadOdometrie;
    EventFlags flags;
    Mutex mutexData;
  
    int readyCount;
    Semaphore semG, semD;
    Mutex syncMutex;
   

    // Données de mouvement
    float _positionX, _positionY;
    float _cibleposX, _cibleposY;
    float _Alpha;
    float _Speed, _SpeedAlpha;
    float _Move, _MoveAlpha;
    int _deltaG, _deltaD;

    int lastPosG = 0;
    int lastPosD = 0;

    // Routines moteurs
    void routine_gauche();
    void routine_droite();
    void synchroniser();

    void routine_odometrie();
   

};

#endif // DIFFERENTIEL_HPP