#include "UART_TMC.hpp"
//***********************************/************************************
//                         Constructors                                 //
//***********************************/************************************
Uart_TMC::Uart_TMC(PinName Uart_TX_pin, PinName Uart_RX_pin,float RS)

{
    SWSerialTMC=new SerialTMC(Uart_TX_pin, Uart_RX_pin);
    UART_StepperMotorG = new TMC2209Stepper(SWSerialTMC, RS, 0x00);
    UART_StepperMotorD = new TMC2209Stepper(SWSerialTMC, RS, 0x01);
}

bool Uart_TMC::setup_all_stepper(void)
{
  SWSerialTMC->beginSerial(155200);
  wait_us(10*1000);

  UART_StepperMotorG->begin();
  UART_StepperMotorD->begin();
  //***********************************/************************************
  // UART_StepperMotorG                                                              /
  //***********************************/************************************
  wait_us(10*1000);
  UART_StepperMotorG->toff(TOFF);                // Enables driver in software - 3, 5 ????
  UART_StepperMotorG->rms_current(RMSCURRENT);   // Set motor RMS current in mA / min 500 for 24V/speed:3000
                                       // 1110, 800
                                                // working: 800 12V/0,6Amax,  Speed up to 5200=4U/min
  UART_StepperMotorG->microsteps(MSTEP_DRIVE);    // Set microsteps to 1:Fullstep ... 256: 1/256th
  UART_StepperMotorG->en_spreadCycle(EN_SPREADCYCLE);     // Toggle spreadCycle on TMC2208/2209/2224: default false, true: much faster!!!!
  UART_StepperMotorG->pwm_autoscale(PWM_AUTOSCALE);       // Needed for stealthChop
  

  //***********************************/************************************
  // UART_StepperMotorD                                                              /
  //***********************************/************************************
  wait_us(10*1000);
  UART_StepperMotorD->toff(TOFF);                // Enables driver in software - 3, 5 ????
  UART_StepperMotorD->rms_current(RMSCURRENT);   // Set motor RMS current in mA / min 500 for 24V/speed:3000
                                       // 1110, 800
                                       // working: 800 12V/0,6Amax,  Speed up to 5200=4U/min
  UART_StepperMotorD->microsteps(MSTEP_DRIVE);    // Set microsteps to 1:Fullstep ... 256: 1/256th
  UART_StepperMotorD->en_spreadCycle(EN_SPREADCYCLE);     // Toggle spreadCycle on TMC2208/2209/2224: default false, true: much faster!!!!
  UART_StepperMotorD->pwm_autoscale(PWM_AUTOSCALE);       // Needed for stealthChop

  return true;
}

