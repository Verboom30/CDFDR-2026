#ifndef __UART_TMC_H_
#define __UART_TMC_H_

#include "mbed.h"
#include "pinout.hpp"
#include "TMC2209/TMC2209.hpp"
#include "Differentiel.hpp"
class Uart_TMC
{
    public:
    Uart_TMC(PinName Uart_TX_pin, PinName Uart_RX_pin,float RS);

    SerialTMC * SWSerialTMC;
  
    TMC2209Stepper* UART_StepperMotorG;
    TMC2209Stepper* UART_StepperMotorD;
    bool setup_all_stepper();
    private : 

    protected:
};

#endif // __TMC2209_H_