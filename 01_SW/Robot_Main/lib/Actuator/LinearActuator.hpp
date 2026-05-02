#ifndef LINEARACTUATOR_H
#define LINEARACTUATOR_H

#include "mbed.h"
#include "pinout.hpp"
#include "Stepper.hpp"
#include "main_pck.hpp"
#include <string.h>

#define ACC_ACT   0.2
#define DEC_ACT   0.2
#define SPEED_ACT 14000.0 // max 50000 Mstepper 16 3200Ma
#define MSTEP_ACT 16


class LinearActuator
{
  public:
    LinearActuator(PinName step, PinName dir,PinName SW_up, PinName SW_down, bool reverse);
    Stepper* StepperAct= nullptr;
   
    bool InitDir(void);
    bool goUp(void);
    bool goDown(void);
    void move(int step);
    void InitLinearActuator(void);
    Thread routine;
    void routine_Actuator(void);
    DigitalIn _sw_up;
    DigitalIn _sw_down;
    int _step_move;
  private:
    int _reverse;
    string _Cmd;
    bool _Ack;
  protected:
    
};

#endif