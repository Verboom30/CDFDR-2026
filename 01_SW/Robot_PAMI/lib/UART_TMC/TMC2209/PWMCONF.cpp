#include "TMC2209.hpp"

#define SET_REG(SETTING) PWMCONF_register.SETTING = B; SWSerial->write(TMC2209_SLAVE_ADDR, PWMCONF_register.address, PWMCONF_register.sr)
#define GET_REG(SETTING) return PWMCONF_register.SETTING

uint32_t TMC2209Stepper::PWMCONF() {
	return SWSerial->read(TMC2209_SLAVE_ADDR, PWMCONF_register.address);
}
void TMC2209Stepper::PWMCONF(uint32_t input) {
	PWMCONF_register.sr = input;
	SWSerial->write(TMC2209_SLAVE_ADDR, PWMCONF_register.address, PWMCONF_register.sr);
}

void TMC2209Stepper::pwm_ofs		( uint8_t B ) { PWMCONF_register.pwm_ofs = B; SWSerial->write(TMC2209_SLAVE_ADDR, PWMCONF_register.address, PWMCONF_register.sr); }
void TMC2209Stepper::pwm_grad		( uint8_t B ) { PWMCONF_register.pwm_grad = B; SWSerial->write(TMC2209_SLAVE_ADDR, PWMCONF_register.address, PWMCONF_register.sr); }
void TMC2209Stepper::pwm_freq		( uint8_t B ) { PWMCONF_register.pwm_freq = B; SWSerial->write(TMC2209_SLAVE_ADDR, PWMCONF_register.address, PWMCONF_register.sr); }
void TMC2209Stepper::pwm_autoscale	( bool 	  B ) { PWMCONF_register.pwm_autoscale = B; SWSerial->write(TMC2209_SLAVE_ADDR, PWMCONF_register.address, PWMCONF_register.sr); }
void TMC2209Stepper::pwm_autograd	( bool    B ) { PWMCONF_register.pwm_autograd = B; SWSerial->write(TMC2209_SLAVE_ADDR, PWMCONF_register.address, PWMCONF_register.sr); }
void TMC2209Stepper::freewheel		( uint8_t B ) { PWMCONF_register.freewheel = B; SWSerial->write(TMC2209_SLAVE_ADDR, PWMCONF_register.address, PWMCONF_register.sr); }
void TMC2209Stepper::pwm_reg		( uint8_t B ) { PWMCONF_register.pwm_reg = B; SWSerial->write(TMC2209_SLAVE_ADDR, PWMCONF_register.address, PWMCONF_register.sr); }
void TMC2209Stepper::pwm_lim		( uint8_t B ) { PWMCONF_register.pwm_lim = B; SWSerial->write(TMC2209_SLAVE_ADDR, PWMCONF_register.address, PWMCONF_register.sr); }

uint8_t TMC2209Stepper::pwm_ofs()		{ TMC2209_n::PWMCONF_t r{0}; r.sr = PWMCONF(); return r.pwm_ofs;		}
uint8_t TMC2209Stepper::pwm_grad()		{ TMC2209_n::PWMCONF_t r{0}; r.sr = PWMCONF(); return r.pwm_grad;		}
uint8_t TMC2209Stepper::pwm_freq()		{ TMC2209_n::PWMCONF_t r{0}; r.sr = PWMCONF(); return r.pwm_freq;		}
bool 	TMC2209Stepper::pwm_autoscale()	{ TMC2209_n::PWMCONF_t r{0}; r.sr = PWMCONF(); return r.pwm_autoscale;	}
bool 	TMC2209Stepper::pwm_autograd()	{ TMC2209_n::PWMCONF_t r{0}; r.sr = PWMCONF(); return r.pwm_autograd;	}
uint8_t TMC2209Stepper::freewheel()		{ TMC2209_n::PWMCONF_t r{0}; r.sr = PWMCONF(); return r.freewheel;		}
uint8_t TMC2209Stepper::pwm_reg()		{ TMC2209_n::PWMCONF_t r{0}; r.sr = PWMCONF(); return r.pwm_reg;		}
uint8_t TMC2209Stepper::pwm_lim()		{ TMC2209_n::PWMCONF_t r{0}; r.sr = PWMCONF(); return r.pwm_lim;		}

