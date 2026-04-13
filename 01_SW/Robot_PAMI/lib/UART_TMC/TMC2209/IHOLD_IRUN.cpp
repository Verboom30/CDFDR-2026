#include "TMC2209.hpp"

#define SET_REG(SETTING) IHOLD_IRUN_register.SETTING = B; SWSerial->write(TMC2209_SLAVE_ADDR,IHOLD_IRUN_register.address, IHOLD_IRUN_register.sr);
#define GET_REG(SETTING) return IHOLD_IRUN_register.SETTING;

// IHOLD_IRUN
uint32_t TMC2209Stepper::IHOLD_IRUN() { return IHOLD_IRUN_register.sr; }
void TMC2209Stepper::IHOLD_IRUN(uint32_t input) {
    IHOLD_IRUN_register.sr = input;
    SWSerial->write(TMC2209_SLAVE_ADDR,IHOLD_IRUN_register.address, IHOLD_IRUN_register.sr);
}

void    TMC2209Stepper::ihold(uint8_t B)        { SET_REG(ihold);       }
void    TMC2209Stepper::irun(uint8_t B)         { SET_REG(irun);        }
void    TMC2209Stepper::iholddelay(uint8_t B)   { SET_REG(iholddelay);  }

uint8_t TMC2209Stepper::ihold()                 { GET_REG(ihold);       }
uint8_t TMC2209Stepper::irun()                  { GET_REG(irun);        }
uint8_t TMC2209Stepper::iholddelay()            { GET_REG(iholddelay);  }