#include "TMC2209.hpp"

//***********************************/************************************
//                         Constructors                                 //
//***********************************/************************************

TMC2209Stepper::TMC2209Stepper(SerialTMC*SWSerial, float RS, uint8_t Slave_Addr) :
  Rsense(RS),
  TMC2209_SLAVE_ADDR(Slave_Addr)
{
  // BufferedSerial *SWSerialObj = new BufferedSerial(Uart_TX_pin, Uart_RX_pin);
  //SWSerial = SWSerialObj;
  this->SWSerial =SWSerial;
}
//***********************************/************************************
//                                Get Set                               //
//***********************************/************************************


//***********************************/************************************
//                             Public Methods                           //
//***********************************/************************************
bool TMC2209Stepper::begin() {

    pdn_disable(true);
    mstep_reg_select(true);
    //Wait to initialize
    wait_us(replyDelay*1000);
    uint8_t tmc_version =version();
    printf("TMC-Version@%02X: %02X\r\n",TMC2209_SLAVE_ADDR,tmc_version);
    if (tmc_version != 0x21){
      printf("Wrong TMC-Version(not 0x21) or communication error!! STOPPING!!!\r\n");
      if(SWSerial->CRCerror){
        printf("CRC-Error!!!\r\n");
        printf("Read Uart 0x%08"PRIx32"\n",SWSerial->read(TMC2209_SLAVE_ADDR,TMC2209_n::IOIN_t::address));
        return false;
      }
    }
    return true;
}


uint32_t TMC2209Stepper::IOIN() {
    return SWSerial->read(TMC2209_SLAVE_ADDR, TMC2209_n::IOIN_t::address);
}
bool TMC2209Stepper::enn()          { TMC2209_n::IOIN_t r{0}; r.sr = IOIN(); return r.enn;      }
bool TMC2209Stepper::ms1()          { TMC2209_n::IOIN_t r{0}; r.sr = IOIN(); return r.ms1;      }
bool TMC2209Stepper::ms2()          { TMC2209_n::IOIN_t r{0}; r.sr = IOIN(); return r.ms2;      }
bool TMC2209Stepper::diag()         { TMC2209_n::IOIN_t r{0}; r.sr = IOIN(); return r.diag;     }
bool TMC2209Stepper::pdn_uart()     { TMC2209_n::IOIN_t r{0}; r.sr = IOIN(); return r.pdn_uart; }
bool TMC2209Stepper::step()         { TMC2209_n::IOIN_t r{0}; r.sr = IOIN(); return r.step;     }
bool TMC2209Stepper::spread_en()    { TMC2209_n::IOIN_t r{0}; r.sr = IOIN(); return r.spread_en;}
bool TMC2209Stepper::dir()          { TMC2209_n::IOIN_t r{0}; r.sr = IOIN(); return r.dir;      }
uint8_t TMC2209Stepper::version()   { TMC2209_n::IOIN_t r{0}; r.sr = IOIN(); return r.version;  }


void TMC2209Stepper::rms_current(uint16_t mA) {
  uint8_t CS = 32.0*1.41421*mA/1000.0*(Rsense+0.02)/0.325 - 1;
  // If Current Scale is too low, turn on high sensitivity R_sense and calculate again
  if (CS < 16) {
    vsense(true);
    CS = 32.0*1.41421*mA/1000.0*(Rsense+0.02)/0.180 - 1;
  } else { // If CS >= 16, turn off high_sense_r
    vsense(false);
  }

  if (CS > 31)
    CS = 31;

  irun(CS);
  ihold(CS*holdMultiplier);
  //val_mA = mA;
}

void TMC2209Stepper::microsteps(uint16_t ms) {
  switch(ms) {
    case 256: mres(0); break;
    case 128: mres(1); break;
    case  64: mres(2); break;
    case  32: mres(3); break;
    case  16: mres(4); break;
    case   8: mres(5); break;
    case   4: mres(6); break;
    case   2: mres(7); break;
    case   1: mres(8); break;
    default: break;
  }
}

