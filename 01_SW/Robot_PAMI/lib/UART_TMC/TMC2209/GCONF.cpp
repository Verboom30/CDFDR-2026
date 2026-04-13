#include "TMC2209.hpp"

#define SET_REG(SETTING) GCONF_register.SETTING = B; SWSerial->write(TMC2209_SLAVE_ADDR,GCONF_register.address, GCONF_register.sr)

uint32_t TMC2209Stepper::GCONF() {
    return SWSerial->read(TMC2209_SLAVE_ADDR,GCONF_register.address);
}
void TMC2209Stepper::GCONF(uint32_t input) {
    GCONF_register.sr = input;
    SWSerial->write(TMC2209_SLAVE_ADDR,GCONF_register.address, GCONF_register.sr);
}

void TMC2209Stepper::I_scale_analog(bool B)     { SET_REG(i_scale_analog);  }
void TMC2209Stepper::internal_Rsense(bool B)    { SET_REG(internal_rsense); }
void TMC2209Stepper::en_spreadCycle(bool B)     { SET_REG(en_spreadcycle);  }
void TMC2209Stepper::shaft(bool B)              { SET_REG(shaft);           }
void TMC2209Stepper::index_otpw(bool B)         { SET_REG(index_otpw);      }
void TMC2209Stepper::index_step(bool B)         { SET_REG(index_step);      }
void TMC2209Stepper::pdn_disable(bool B)        { SET_REG(pdn_disable);     }
void TMC2209Stepper::mstep_reg_select(bool B)   { SET_REG(mstep_reg_select);}
void TMC2209Stepper::multistep_filt(bool B)     { SET_REG(multistep_filt);  }

bool TMC2209Stepper::I_scale_analog()   { TMC2209_n::GCONF_t r{0}; r.sr = GCONF(); return r.i_scale_analog;     }
bool TMC2209Stepper::internal_Rsense()  { TMC2209_n::GCONF_t r{0}; r.sr = GCONF(); return r.internal_rsense;    }
bool TMC2209Stepper::en_spreadCycle()   { TMC2209_n::GCONF_t r{0}; r.sr = GCONF(); return r.en_spreadcycle;     }
bool TMC2209Stepper::shaft()            { TMC2209_n::GCONF_t r{0}; r.sr = GCONF(); return r.shaft;              }
bool TMC2209Stepper::index_otpw()       { TMC2209_n::GCONF_t r{0}; r.sr = GCONF(); return r.index_otpw;         }
bool TMC2209Stepper::index_step()       { TMC2209_n::GCONF_t r{0}; r.sr = GCONF(); return r.index_step;         }
bool TMC2209Stepper::pdn_disable()      { TMC2209_n::GCONF_t r{0}; r.sr = GCONF(); return r.pdn_disable;        }
bool TMC2209Stepper::mstep_reg_select() { TMC2209_n::GCONF_t r{0}; r.sr = GCONF(); return r.mstep_reg_select;   }
bool TMC2209Stepper::multistep_filt()   { TMC2209_n::GCONF_t r{0}; r.sr = GCONF(); return r.multistep_filt;     }