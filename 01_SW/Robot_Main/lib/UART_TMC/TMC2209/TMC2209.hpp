#ifndef __TMC2209_H_
#define __TMC2209_H_
#include "mbed.h"
#include "TMC2209_bitfields.hpp"
#include "Serial/SerialTMC.hpp"
#include "main_pck.hpp"
#include <inttypes.h>
#define INIT_REGISTER(REG) REG##_t REG##_register = REG##_t
#define INIT2209_REGISTER(REG) TMC2209_n::REG##_t REG##_register = TMC2209_n::REG##_t



class TMC2209Stepper 
{
  public:  
    //***********************************/************************************
    //                         Constructors                                 //
    //***********************************/************************************    
    TMC2209Stepper (SerialTMC*SWSerial, float RS, const uint8_t Slave_Addr);
    //***********************************/************************************
    //                             Public Methods                           //
    //***********************************/************************************
    bool begin();
   
    void rms_current(uint16_t mA);
    void microsteps(uint16_t ms);

    // R: IOIN
    uint32_t IOIN();
    bool enn();
    bool ms1();
    bool ms2();
    bool diag();
    bool pdn_uart();
    bool step();
    bool spread_en();
    bool dir();
    uint8_t version();

    // RW: GCONF
    void GCONF(uint32_t input);
    void I_scale_analog(bool B);
    void internal_Rsense(bool B);
    void en_spreadCycle(bool B);
    void shaft(bool B);
    void index_otpw(bool B);
    void index_step(bool B);
    void pdn_disable(bool B);
    void mstep_reg_select(bool B);
    void multistep_filt(bool B);
    uint32_t GCONF();
    bool I_scale_analog();
    bool internal_Rsense();
    bool en_spreadCycle();
    bool shaft();
    bool index_otpw();
    bool index_step();
    bool pdn_disable();
    bool mstep_reg_select();
    bool multistep_filt();

    // RW: CHOPCONF
    void CHOPCONF(uint32_t input);
    void toff(uint8_t B);
    void hstrt(uint8_t B);
    void hend(uint8_t B);
    void tbl(uint8_t B);
    void vsense(bool B);
    void mres(uint8_t B);
    void intpol(bool B);
    void dedge(bool B);
    void diss2g(bool B);
    void diss2vs(bool B);
    uint32_t CHOPCONF();
    uint8_t toff();
    uint8_t hstrt();
    uint8_t hend();
    uint8_t tbl();
    bool vsense();
    uint8_t mres();
    bool intpol();
    bool dedge();
    bool diss2g();
    bool diss2vs();

    // W: IHOLD_IRUN
    void IHOLD_IRUN(uint32_t input);
    uint32_t IHOLD_IRUN();
    void ihold(uint8_t B);
    void irun(uint8_t B);
    void iholddelay(uint8_t B);
    uint8_t ihold();
    uint8_t irun();
    uint8_t iholddelay();

    // RW: PWMCONF
    void PWMCONF(uint32_t input);
    void pwm_ofs(uint8_t B);
    void pwm_grad(uint8_t B);
    void pwm_freq(uint8_t B);
    void pwm_autoscale(bool B);
    void pwm_autograd(bool B);
    void freewheel(uint8_t B);
    void pwm_reg(uint8_t B);
    void pwm_lim(uint8_t B);
    uint32_t PWMCONF();
    uint8_t pwm_ofs();
    uint8_t pwm_grad();
    uint8_t pwm_freq();
    bool pwm_autoscale();
    bool pwm_autograd();
    uint8_t freewheel();
    uint8_t pwm_reg();
    uint8_t pwm_lim();

    // R: PWM_SCALE
    uint32_t PWM_SCALE();
    uint8_t pwm_scale_sum();
    int16_t pwm_scale_auto();

    // R: PWM_AUTO (0x72)
    uint32_t PWM_AUTO();
    uint8_t pwm_ofs_auto();
    uint8_t pwm_grad_auto();

  protected:
    //***********************************/************************************
    //                          Protected Methods                           //
    //***********************************/************************************
    INIT2209_REGISTER(GCONF)            {{.sr=0}}; //GCONF_register
    INIT2209_REGISTER(CHOPCONF)         {{.sr=0}}; //CHOPCONF_register
    INIT2209_REGISTER(PWMCONF)          {{.sr=0}}; //PWMCONF_register

    INIT_REGISTER(IHOLD_IRUN)            {{.sr=0}}; // IHOLD_IRUN_register 32b
    
    SerialTMC * SWSerial = nullptr;
    
   

    static constexpr uint8_t TMC_READ = 0x00, TMC_WRITE = 0x80;
    static constexpr uint8_t  TMC2209_SYNC = 0x05;
    const uint8_t TMC2209_SLAVE_ADDR;
    static constexpr uint8_t replyDelay = 4;  //ms
    static constexpr uint8_t max_retries = 4;
    static constexpr uint8_t abort_window = 5; //5ms timeout
    const float Rsense;
    float holdMultiplier = 0.25;

   
  
  private : 
    //***********************************/************************************
    //                            Private Methods                           //
    //***********************************/************************************
     
};


#endif // __TMC2209_H_