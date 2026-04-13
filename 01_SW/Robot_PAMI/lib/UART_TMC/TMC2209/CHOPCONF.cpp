#include "TMC2209.hpp"

#define SET_REG(SETTING) CHOPCONF_register.SETTING = B; SWSerial->write(TMC2209_SLAVE_ADDR, CHOPCONF_register.address, CHOPCONF_register.sr)

void TMC2209Stepper::CHOPCONF(uint32_t input) {
    CHOPCONF_register.sr = input;
    SWSerial->write(TMC2209_SLAVE_ADDR,CHOPCONF_register.address, CHOPCONF_register.sr);
}
uint32_t TMC2209Stepper::CHOPCONF() {
    return  SWSerial->read(TMC2209_SLAVE_ADDR,CHOPCONF_register.address);
}
void TMC2209Stepper::toff   ( uint8_t  B )  { SET_REG(toff);    }
void TMC2209Stepper::hstrt  ( uint8_t  B )  { SET_REG(hstrt);   }
void TMC2209Stepper::hend   ( uint8_t  B )  { SET_REG(hend);    }
void TMC2209Stepper::tbl    ( uint8_t  B )  { SET_REG(tbl);     }
void TMC2209Stepper::vsense ( bool     B )  { SET_REG(vsense);  }
void TMC2209Stepper::mres   ( uint8_t  B )  { SET_REG(mres);    }
void TMC2209Stepper::intpol ( bool     B )  { SET_REG(intpol);  }
void TMC2209Stepper::dedge  ( bool     B )  { SET_REG(dedge);   }
void TMC2209Stepper::diss2g ( bool     B )  { SET_REG(diss2g);  }
void TMC2209Stepper::diss2vs( bool     B )  { SET_REG(diss2vs); }

uint8_t TMC2209Stepper::toff()      { TMC2209_n::CHOPCONF_t r{0}; r.sr = CHOPCONF(); return r.toff;     }
uint8_t TMC2209Stepper::hstrt()     { TMC2209_n::CHOPCONF_t r{0}; r.sr = CHOPCONF(); return r.hstrt;    }
uint8_t TMC2209Stepper::hend()      { TMC2209_n::CHOPCONF_t r{0}; r.sr = CHOPCONF(); return r.hend;     }
uint8_t TMC2209Stepper::tbl()       { TMC2209_n::CHOPCONF_t r{0}; r.sr = CHOPCONF(); return r.tbl;      }
bool    TMC2209Stepper::vsense()    { TMC2209_n::CHOPCONF_t r{0}; r.sr = CHOPCONF(); return r.vsense;   }
uint8_t TMC2209Stepper::mres()      { TMC2209_n::CHOPCONF_t r{0}; r.sr = CHOPCONF(); return r.mres;     }
bool    TMC2209Stepper::intpol()    { TMC2209_n::CHOPCONF_t r{0}; r.sr = CHOPCONF(); return r.intpol;   }
bool    TMC2209Stepper::dedge()     { TMC2209_n::CHOPCONF_t r{0}; r.sr = CHOPCONF(); return r.dedge;    }
bool    TMC2209Stepper::diss2g()    { TMC2209_n::CHOPCONF_t r{0}; r.sr = CHOPCONF(); return r.diss2g;   }
bool    TMC2209Stepper::diss2vs()   { TMC2209_n::CHOPCONF_t r{0}; r.sr = CHOPCONF(); return r.diss2vs;  }
