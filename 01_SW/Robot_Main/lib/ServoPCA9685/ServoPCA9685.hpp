#ifndef SERVO_PCA9685_HPP
#define SERVO_PCA9685_HPP

#include "mbed.h"

class ServoPCA9685
{
public:
    static constexpr uint8_t SERVO_COUNT = 8;

    ServoPCA9685(I2C& i2c, uint8_t address_7bit);

    bool begin();

    void setMirrored(uint8_t channel, bool mirrored);
    void setOffset(uint8_t channel, float offset_deg);

    void setServoAngle(uint8_t channel, float angle_deg);
    void setAllAngle(float angle_deg);

    void releaseServo(uint8_t channel);
    void releaseAllServos();
    

private:
    static constexpr uint8_t MODE1     = 0x00;
    static constexpr uint8_t MODE2     = 0x01;
    static constexpr uint8_t PRESCALE  = 0xFE;
    static constexpr uint8_t LED0_ON_L = 0x06;

    static constexpr float SERVO_FREQ_HZ = 50.0f;

private:
    I2C& _i2c;
    uint8_t _addr;

    bool _mirrored[SERVO_COUNT];
    float _offset[SERVO_COUNT];

private:
    bool writeReg(uint8_t reg, uint8_t value);
    bool readReg(uint8_t reg, uint8_t& value);

    void setPwmFreq(float freq_hz);
    void setPwm(uint8_t channel, uint16_t on, uint16_t off);

    void writeUs(uint8_t channel, uint16_t pulse_us);
    float applyConfig(uint8_t channel, float angle_deg);
};

#endif