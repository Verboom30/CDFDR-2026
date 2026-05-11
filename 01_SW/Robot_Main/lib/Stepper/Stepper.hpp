#ifndef STEPPER_H
#define STEPPER_H

#include "mbed.h"

class Stepper : public Ticker
{
public:
    Stepper(PinName clk, PinName dir, bool inv);

    void goesTo(int position);
    void move(int steps);
    void rotate(bool direction);
    void stop(void);
    void run(void);

    void pause(float deceleration = -1.0f);
    void resume(float acceleration = -1.0f);
    bool paused(void) const;

    void setSpeed(float speed);
    void setAcceleration(float acc);
    void setDeceleration(float dec);
    void setPosition(int position);
    void setPositionZero(void);

    bool stopped(void);
    bool getPosCibleDone(void);

    int getPosition(void);
    int getStep(void);

    float getAcceleration(void);
    float getDeceleration(void);
    float getSpeed(void);

    typedef enum {CW=1,CCW=0} direction;

private:
    float _acc;
    float _dec;
    float _spd;
    unsigned int _steps;
    DigitalOut _clk;
    DigitalOut _dirPin;
    bool _dir;
    bool _inv;
    enum { STOP, ACCEL, CRUISE, DECEL, PAUSE } _state, _prev_state;
    bool _pauseRequested;
    bool _pauseActive;
    bool _pausedWasContinuous;
    unsigned int _pauseOriginalTargetSteps;
    int _pausedResumeSteps;
    float _pauseDecOverride;
    float _resumeAccOverride;
    unsigned int _dt0;
    unsigned int _dtmin;
    unsigned int _dtn;
    int _Pos_Cible_Done;
    float _i;
    int _pos;
    unsigned int _n;
    unsigned int _nStartDec;
    virtual void handler();
    unsigned int nTo(float speed, float acc);
};

#endif
