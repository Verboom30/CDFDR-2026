#ifndef __TMC2009_BITFIELDS_H_
#define __TMC2009_BITFIELDS_H_
#include "mbed.h"
#pragma pack(push, 1)
//###############################################
//#      GENERAL CONFIGURATION REGISTERS        #
//###############################################
//***********************************/************************************
// R/W Register GCONF: Addr 0x00     /                                   /
//***********************************/************************************
namespace TMC2209_n {
  struct GCONF_t {
    constexpr static uint8_t address = 0x00;
    union {
      uint16_t sr : 10; // size 10
      struct {
        bool  i_scale_analog    : 1, //bit(0) //size
              internal_rsense   : 1, //bit(1) //size
              en_spreadcycle    : 1, //bit(2) //size
              shaft             : 1, //bit(3) //size 
              index_otpw        : 1, //bit(4) //size
              index_step        : 1, //bit(5) //size
              pdn_disable       : 1, //bit(6) //size
              mstep_reg_select  : 1, //bit(4) //size
              multistep_filt    : 1, //bit(8) //size
              test_mode         : 1; //bit(9) //size
      };
    };
  };
}
//***********************************/************************************
// R+WC Register GCONF: Addr 0x01    /                                   /
//***********************************/************************************
namespace TMC2209_n {
  struct GSTAT_t {
    constexpr static uint8_t address = 0x01;
    union {
      uint16_t sr : 3; // size 3
      struct {
        bool  reset   : 1, //bit(0) //size
              drv_err : 1, //bit(1) //size
              uc_cp   : 1; //bit(2) //size
      };
    };
  };
}
//***********************************/************************************
// R  Register IFCNT: Addr 0x02      /                                   /
//***********************************/************************************
struct IFCNT_t { constexpr static uint8_t address = 0x02; };

//***********************************/************************************
// W Register SLAVECONF: Addr 0x03   /                                   /
//***********************************/************************************
namespace TMC2209_n {
  struct SLAVECONF_t {
    constexpr static uint8_t address = 0x03;
    union {
      uint16_t sr : 4; // size 4
      struct {
         uint8_t senddelay : 4; //size
      }; 
    };
  };
}
//***********************************/************************************
// W Register OTP_PROG: Addr 0x04    /                                   /
//***********************************/************************************
namespace TMC2209_n {
  struct OTP_PROG_t {
    constexpr static uint8_t address = 0x04;
    union {
      uint16_t sr : 16; // size 16
      struct {
         uint8_t otpbit   : 3, //size
                 otpbyte  : 2, //size
                 otpmagic : 8; //size
      };
    };
  };
}
//***********************************/************************************
// R Register OTP_READ: Addr 0x05    /                                   /
//***********************************/************************************
struct OTP_READ_t   { constexpr static uint8_t address = 0x05; };
//***********************************/************************************
// R Register IOIN: Addr 0x06        /                                   /
//***********************************/************************************
namespace TMC2209_n {
  struct IOIN_t {
    constexpr static uint8_t address = 0x06;
    union {
      uint32_t sr;
      struct {
        bool  enn       : 1,
              : 1,
              ms1       : 1,
              ms2       : 1,
              diag      : 1,
              : 1,
              pdn_uart  : 1,
              step      : 1,
              spread_en : 1,
              dir       : 1;
        uint16_t: 14;
        uint8_t version : 8;
      };
    };
  };
}
//***********************************/************************************
// RW Register FACTORY_CONF: Addr 0x07                                   /
//***********************************/************************************
namespace TMC2209_n {
  struct FACTORY_CONF_t {
    constexpr static uint8_t address = 0x07;
    union {
      uint32_t sr;
      struct {
        uint8_t fclktrim  : 5,
                : 3,
                ottrim    : 2;
      };
    };
  };
}
//######################################################################
//#      VELOCITY DEPENDENT DRIVER FEATURE CONTROL REGISTER SET        #
//######################################################################
//***********************************/************************************
// W Register IHOLD_IRUN: Addr 0x10                                      /
//***********************************/************************************
struct IHOLD_IRUN_t {
  constexpr static uint8_t address = 0x10;
  union {
    uint32_t sr : 20;
    struct {
      uint8_t ihold       : 5,
              : 3,
              irun        : 5,
              : 3,
              iholddelay  : 4;
    };
  };
};
//***********************************/************************************
// W Register TPOWER DOWN: Addr 0x11                                     /
//***********************************/************************************
struct TPOWER_DOWN_t   { constexpr static uint8_t address = 0x11; };
//***********************************/************************************
// R Register TSTEP: Addr 0x12                                           /
//***********************************/************************************
struct TSTEP_t { 
  constexpr static uint8_t address = 0x12; 
  uint32_t sr : 20;
};
//***********************************/************************************
// W Register TPWMTHRS: Addr 0x13                                        /
//***********************************/************************************
struct TPWMTHRS_t {
  constexpr static uint8_t address = 0x13;
  uint32_t sr : 20;
};
//***********************************/************************************
// W Register VACTUAL: Addr 0x22                                         /
//***********************************/************************************
namespace TMC2209_n {
  struct VACTUAL_t {
    constexpr static uint8_t address = 0x22;
    uint32_t sr;
  };
}
//######################################################################
//#      COOLSTEP AND STALLGUARD CONTROL REGISTER SET                  #
//######################################################################
//***********************************/************************************
// W Register TCOOLTHRS: Addr 0x14                                       /
//***********************************/************************************
struct TCOOLTHRS_t {
  constexpr static uint8_t address = 0x14;
  uint32_t sr : 20;
};
//***********************************/************************************
// W Register SGTHRS: Addr 0x40                                          /  
//***********************************/************************************
struct SGTHRS_t {
  constexpr static uint8_t address = 0x40;
  uint32_t sr : 8;
};
//***********************************/************************************
// R Register SG_RESULT: Addr 0x41                                       /  
//***********************************/************************************
struct SG_RESULT_t {
  constexpr static uint8_t address = 0x41;
  uint32_t sr : 10;
};
//***********************************/************************************
// W Register COOLCONF: Addr 0x42                                        /  
//***********************************/************************************
struct COOLCONF_t {
  constexpr static uint8_t address = 0x42;
  uint32_t sr : 16;
};
//######################################################################
//#      MICROSTEPPING CONTROL REGISTER SET                            #
//######################################################################
//***********************************/************************************
// R Register MSCNT: Addr 0x6A                                           /  
//***********************************/************************************
struct MSCNT_t {
  constexpr static uint8_t address = 0x6A;
  uint32_t sr : 10;
};
//***********************************/************************************
// R Register MSCURACT: Addr 0x6B                                           /  
//***********************************/************************************
struct MSCURACT_t {
  constexpr static uint8_t address = 0x6B;
  union {
    uint32_t sr : 25;
    struct {
      uint8_t cur_b : 9,
              : 7,
              cur_a : 9;
    };
  };
};

//######################################################################
//#                    DRIVER REGISTER SET                             #
//######################################################################
//***********************************/************************************
// RW Register CHOPCONF: Addr 0x6C                                       /  
//***********************************/************************************
namespace TMC2209_n {
  struct CHOPCONF_t {
    constexpr static uint8_t address = 0x6C;
    union {
      uint32_t sr;
      struct {
        uint8_t toff    : 4,
                hstrt   : 3,
                hend    : 4,
                : 4,
                tbl     : 2;
        bool    vsense  : 1;
        uint8_t : 6,
                mres    : 4;
        bool    intpol  : 1,
                dedge   : 1,
                diss2g  : 1,
                diss2vs : 1;
      };
    };
  };
};
//***********************************/************************************
// R Register DRV_STATUS: Addr 0x6F                                      /  
//***********************************/************************************
namespace TMC2209_n {
  struct DRV_STATUS_t {
    constexpr static uint8_t address = 0x6F;
    union {
      uint32_t sr;
      struct {
        bool otpw         : 1,
            ot            : 1,
            s2ga          : 1,
            s2gb          : 1,
            s2vsa         : 1,
            s2vsb         : 1,
            ola           : 1,
            olb           : 1,
            t120          : 1,
            t143          : 1,
            t150          : 1,
            t157          : 1;
        uint8_t : 4,
                cs_actual : 5,
                : 3,
                : 6;
                bool stealth  : 1,
                stst          : 1;
      };
    };
  };
};
//***********************************/************************************
// RW Register PWMCONF: Addr 0x70                                        /  
//***********************************/************************************
namespace TMC2209_n {
  struct PWMCONF_t {
    constexpr static uint8_t address = 0x70;
    union {
      uint32_t sr;
      struct {
        uint8_t pwm_ofs             : 8,
                pwm_grad            : 8,
                pwm_freq            : 2;
                bool pwm_autoscale  : 1,
                pwm_autograd        : 1;
        uint8_t freewheel           : 2,
                                    : 2,
                pwm_reg             : 4,
                pwm_lim             : 4;
      };
    };
  };
};
//***********************************/************************************
// R Register PWM_SCALE: Addr 0x71                                        /  
//***********************************/************************************
namespace TMC2209_n {
 struct PWM_SCALE_t {
    constexpr static uint8_t address = 0x71;
    union {
      uint32_t sr;
      struct {
        uint8_t pwm_scale_sum  : 8,
                : 8;
        int16_t pwm_scale_auto : 9;
      };
    };
  };
};
//***********************************/************************************
// R Register PWM_AUTO: Addr 0x72                                        /  
//***********************************/************************************
struct PWM_AUTO_t {
  constexpr static uint8_t address = 0x72;
  union {
    uint32_t sr : 24;
    struct {
      uint8_t pwm_ofs_auto  : 8,
              : 8,
              pwm_grad_auto : 8;
    };
  };
};
#endif // __TMC2009_BITFIELDS_H_