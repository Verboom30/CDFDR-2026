#include "SerialTMC.hpp"
//***********************************/************************************
//                         Constructors                                 //
//***********************************/************************************
SerialTMC::SerialTMC(PinName Uart_TX_pin, PinName Uart_RX_pin)
{
  SWSerial = new BufferedSerial(Uart_TX_pin, Uart_RX_pin);
}
//***********************************/************************************
//                                Get Set                               //
//***********************************/************************************
//***********************************/************************************
//                             Public Methods                           //
//***********************************/************************************
void SerialTMC::beginSerial(uint32_t baudrate) {
if (SWSerial != nullptr)
      {
          //SWSerial->begin(baudSrate);
          //SWSerial->end();
          SWSerial->set_baud(baudrate);
          SWSerial->set_format(
                      /* bits */ 8,
                      /* parity */ BufferedSerial::None,
                      /* stop bit */ 1
                  );
          SWSerial->set_blocking(false);  //set to non-blocking read
      }
}

void SerialTMC::write(uint8_t slave_addr, uint8_t addr, uint32_t dataWrite) {
  uint16_t bytesWritten = 0;
  uint8_t len = 7;
  addr |= TMC_WRITE;
  uint8_t datagram[] = {TMC2209_SYNC, slave_addr, addr, (uint8_t)(dataWrite>>24), (uint8_t)(dataWrite>>16), (uint8_t)(dataWrite>>8), (uint8_t)(dataWrite>>0), 0x00};
  datagram[len] = calcCRC(datagram, len);
  for(uint8_t i=0; i<=len; i++) {
    bytesWritten += serial_single_write(datagram[i]);
  }
  //wait_us(replyDelay*1000);
}

uint32_t SerialTMC::read(uint8_t slave_addr,uint8_t addr){
  constexpr uint8_t len = 3;
  addr |= TMC_READ;
  uint8_t datagram[] = {TMC2209_SYNC, slave_addr, addr, 0x00};
  datagram[len] = calcCRC(datagram, len);
  uint64_t out = 0x00000000UL;
  for (uint8_t i = 0; i < max_retries; i++) {
    out = ReadResquest(datagram, len, abort_window);
    CRCerror = false;
    uint8_t out_datagram[] = {
            static_cast<uint8_t>(out>>56),
            static_cast<uint8_t>(out>>48),
            static_cast<uint8_t>(out>>40),
            static_cast<uint8_t>(out>>32),
            static_cast<uint8_t>(out>>24),
            static_cast<uint8_t>(out>>16),
            static_cast<uint8_t>(out>> 8),
            static_cast<uint8_t>(out>> 0)
        };
        uint8_t crc = calcCRC(out_datagram, 7);
        if ((crc != static_cast<uint8_t>(out)) || crc == 0 ) {
            CRCerror = true;
            out = 0;
        } else {
            break;
        }
  }
  return out>>8;
}
//***********************************/************************************
//                          Protected Methods                           //
//***********************************/************************************
int16_t SerialTMC::serial_single_read(){
  int16_t out = 0;
  int16_t count = 0;
  if (&SWSerial != nullptr) {
     count = SWSerial->read(&out, 1); // read one character
  } 
  if (count >= 1) {
    //printf("<%02X|",out);
    return out;
  } else {
    return -1;
  }
}

uint8_t SerialTMC::serial_single_write(const uint8_t data){
  int out = 0;
  if (SWSerial != nullptr) {
    return SWSerial->write(&data,1);
  }
  return out;
}

uint8_t SerialTMC::calcCRC(uint8_t datagram[], uint8_t len) {
    uint8_t crc = 0;
    for (uint8_t i = 0; i < len; i++) {
        uint8_t currentByte = datagram[i];
        for (uint8_t j = 0; j < 8; j++) {
            if ((crc >> 7) ^ (currentByte & 0x01)) {
                crc = (crc << 1) ^ 0x07;
            } else {
                crc = (crc << 1);
            }
            crc &= 0xff;
            currentByte = currentByte >> 1;
        }
    }
    return crc;
}

uint64_t SerialTMC::ReadResquest(uint8_t datagram[], const uint8_t len, uint16_t timeout){
  SWSerial->sync(); //Fulsh buffers
  for(int i=0; i<=len; i++) serial_single_write(datagram[i]);
  wait_us(replyDelay*1000);

  uint32_t sync_target = (static_cast<uint32_t>(datagram[0])<<16) | 0xFF00 | datagram[2];
  uint32_t sync = 0;
  uint64_t start_ms, now;
  start_ms = Kernel::get_ms_count();
  do {
    now = Kernel::get_ms_count();
    if (now - start_ms > timeout) return 0;
    int16_t res = serial_single_read();
    if (res < 0) continue;
    sync <<= 8;
    sync |= res & 0xFF;
    sync &= 0xFFFFFF;
  } while (sync != sync_target);

  uint64_t out = sync;
  start_ms = Kernel::get_ms_count();
  for(uint8_t i=0; i<5;) {
    now = Kernel::get_ms_count();
    if (now - start_ms > timeout) return 0;

    int16_t res = serial_single_read();
    if (res < 0) continue;

    out <<= 8;
    out |= res & 0xFF;
    i++;
  }
  SWSerial->sync();
  return out;
}