#ifndef SERIALTMC_H
#define SERIALTMC_H
#include "mbed.h"


class SerialTMC 
{
  public:  
    //***********************************/************************************
    //                         Constructors                                 //
    //***********************************/************************************   
    SerialTMC (PinName Uart_TX_pin, PinName Uart_RX_pin);
    //***********************************/************************************
    //                                Get Set                               //
    //***********************************/************************************
    //***********************************/************************************
    //                             Public Methods                           //
    //***********************************/************************************
    void beginSerial(uint32_t baudrate);
    void write(uint8_t slave_addr,uint8_t addr, uint32_t dataWrite);
    uint32_t read(uint8_t slave_addr,uint8_t addr);
    bool CRCerror = false;
  protected:
    //***********************************/************************************
    //                          Protected Methods                           //
    //***********************************/************************************
    BufferedSerial * SWSerial;
    int16_t serial_single_read();
    uint8_t serial_single_write(const uint8_t data);
    uint8_t calcCRC(uint8_t datagram[], uint8_t len);
    uint64_t ReadResquest(uint8_t datagram[], const uint8_t len, uint16_t timeout);
    static constexpr uint8_t TMC_READ = 0x00, TMC_WRITE = 0x80;
    static constexpr uint8_t TMC2209_SYNC = 0x05;
    static constexpr uint8_t replyDelay = 2;  //ms
    static constexpr uint8_t max_retries = 2;
    static constexpr uint8_t abort_window = 5; //5ms timeout

  

};




#endif