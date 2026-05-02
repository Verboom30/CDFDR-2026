#include "UART_TMC.hpp"
//***********************************/************************************
//                         Constructors                                 //
//***********************************/************************************
Uart_TMC::Uart_TMC(PinName Uart_TX_pin, PinName Uart_RX_pin, PinName Sel_uart_0, PinName Sel_uart_1, PinName Sel_uart_2,float RS): 
_sel_0(Sel_uart_0),
_sel_1(Sel_uart_1),
_sel_2(Sel_uart_2)
{
    SWSerialTMC=new SerialTMC(Uart_TX_pin, Uart_RX_pin);

    // UART_StepperA = new TMC2209Stepper(SWSerialHolonome, RS, 0x00);
    // UART_StepperB = new TMC2209Stepper(SWSerialHolonome, RS, 0x01);
    // UART_StepperC = new TMC2209Stepper(SWSerialHolonome, RS, 0x02);

    UART_StepperRG = new TMC2209Stepper(SWSerialTMC, RS, 0x00);
    UART_StepperRD = new TMC2209Stepper(SWSerialTMC, RS, 0x01);
    UART_StepperRM = new TMC2209Stepper(SWSerialTMC, RS, 0x02);
    // UART_StepperR4 = new TMC2209Stepper(SWSerialHolonome, RS, 0x03);

    // UART_StepperFork = new TMC2209Stepper(SWSerialHolonome, RS, 0x01);
    // UART_StepperSucker = new TMC2209Stepper(SWSerialHolonome, RS, 0x00);
    _sel_0 =0;
    _sel_1 =0;
    _sel_2 =0;

}

bool Uart_TMC::setup_all_stepper(void)
{
  SWSerialTMC->beginSerial(155200);
  wait_us(10*1000);

  _sel_0 =0;
  _sel_1 =1;
  _sel_2 =0;

  UART_StepperRG->begin();
  UART_StepperRD->begin();
  UART_StepperRM->begin();
  //***********************************/************************************
  // UART_StepperRG                                                              /
  //***********************************/************************************
  wait_us(10*1000);
  UART_StepperRG->toff(TOFF);                // Enables driver in software - 3, 5 ????
  UART_StepperRG->rms_current(RMSCURRENT);   // Set motor RMS current in mA / min 500 for 24V/speed:3000
                                       // 1110, 800
                                       // working: 800 12V/0,6Amax,  Speed up to 5200=4U/min
  UART_StepperRG->microsteps(MSTEP_ACT);    // Set microsteps to 1:Fullstep ... 256: 1/256th
  UART_StepperRG->en_spreadCycle(EN_SPREADCYCLE);     // Toggle spreadCycle on TMC2208/2209/2224: default false, true: much faster!!!!
  UART_StepperRG->pwm_autoscale(PWM_AUTOSCALE);       // Needed for stealthChop
  

  //***********************************/************************************
  // UART_StepperRD                                                              /
  //***********************************/************************************
  wait_us(10*1000);
  UART_StepperRD->toff(TOFF);                // Enables driver in software - 3, 5 ????
  UART_StepperRD->rms_current(RMSCURRENT);   // Set motor RMS current in mA / min 500 for 24V/speed:3000
                                       // 1110, 800
                                       // working: 800 12V/0,6Amax,  Speed up to 5200=4U/min
  UART_StepperRD->microsteps(MSTEP_ACT);    // Set microsteps to 1:Fullstep ... 256: 1/256th
  UART_StepperRD->en_spreadCycle(EN_SPREADCYCLE);     // Toggle spreadCycle on TMC2208/2209/2224: default false, true: much faster!!!!
  UART_StepperRD->pwm_autoscale(PWM_AUTOSCALE);       // Needed for stealthChop

  //***********************************/************************************
  // UART_StepperRM                                                              /
  //***********************************/************************************
  wait_us(10*1000);
  UART_StepperRM->toff(TOFF);                // Enables driver in software - 3, 5 ????
  UART_StepperRM->rms_current(RMSCURRENT);   // Set motor RMS current in mA / min 500 for 24V/speed:3000
                                       // 1110, 800
                                       // working: 800 12V/0,6Amax,  Speed up to 5200=4U/min
  UART_StepperRM->microsteps(MSTEP_ACT);    // Set microsteps to 1:Fullstep ... 256: 1/256th
  UART_StepperRM->en_spreadCycle(EN_SPREADCYCLE);     // Toggle spreadCycle on TMC2208/2209/2224: default false, true: much faster!!!!
  UART_StepperRM->pwm_autoscale(PWM_AUTOSCALE);       // Needed for stealthChop
  
  wait_us(10*1000);
  _sel_0 =0;
  _sel_1 =0;
  _sel_2 =0;
  
  return true;
}

