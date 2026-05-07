#include "ServoPCA9685.hpp"

ServoPCA9685::ServoPCA9685(I2C& i2c, uint8_t address_7bit)
    : _i2c(i2c),
      _addr(address_7bit << 1)
{
    for (uint8_t i = 0; i < SERVO_COUNT; i++)
    {
        _mirrored[i] = false;
        _offset[i] = 0.0f;
    }
}

bool ServoPCA9685::begin()
{
    uint8_t mode1 = 0;

    if (!readReg(MODE1, mode1)) {
        printf("PCA9685 0x%02X non detecte\r\n", _addr >> 1);
        return false;
    }

    printf("PCA9685 0x%02X OK\r\n", _addr >> 1);

    writeReg(MODE1, 0x00);
    writeReg(MODE2, 0x04);

    ThisThread::sleep_for(10ms);

    setPwmFreq(SERVO_FREQ_HZ);

    return true;
}

void ServoPCA9685::setMirrored(uint8_t channel, bool mirrored)
{
    if (channel >= SERVO_COUNT) return;
    _mirrored[channel] = mirrored;
}

void ServoPCA9685::setOffset(uint8_t channel, float offset_deg)
{
    if (channel >= SERVO_COUNT) return;
    _offset[channel] = offset_deg;
}

void ServoPCA9685::setServoAngle(uint8_t channel, float angle_deg)
{
    if (channel >= SERVO_COUNT) return;

    float final_angle = applyConfig(channel, angle_deg);

    uint16_t pulse = 500 + (uint16_t)((final_angle / 180.0f) * 2000.0f);
    writeUs(channel, pulse);
}

void ServoPCA9685::setAllAngle(float angle_deg)
{
    for (uint8_t ch = 0; ch < SERVO_COUNT; ch++) {
        setServoAngle(ch, angle_deg);
    }
}

bool ServoPCA9685::writeReg(uint8_t reg, uint8_t value)
{
    char data[2] = {
        static_cast<char>(reg),
        static_cast<char>(value)
    };

    return _i2c.write(_addr, data, 2) == 0;
}

bool ServoPCA9685::readReg(uint8_t reg, uint8_t& value)
{
    char r = static_cast<char>(reg);
    char v = 0;

    if (_i2c.write(_addr, &r, 1, true) != 0) return false;
    if (_i2c.read(_addr, &v, 1) != 0) return false;

    value = static_cast<uint8_t>(v);
    return true;
}

void ServoPCA9685::setPwmFreq(float freq_hz)
{
    float prescaleval = 25000000.0f / 4096.0f / freq_hz - 1.0f;
    uint8_t prescale = static_cast<uint8_t>(prescaleval + 0.5f);

    uint8_t oldmode = 0;
    readReg(MODE1, oldmode);

    writeReg(MODE1, (oldmode & 0x7F) | 0x10);
    writeReg(PRESCALE, prescale);
    writeReg(MODE1, oldmode);

    ThisThread::sleep_for(5ms);

    writeReg(MODE1, oldmode | 0xA1);
}

void ServoPCA9685::setPwm(uint8_t channel, uint16_t on, uint16_t off)
{
    if (channel >= 16) return;

    uint8_t reg = LED0_ON_L + 4 * channel;

    char data[5];
    data[0] = static_cast<char>(reg);
    data[1] = static_cast<char>(on & 0xFF);
    data[2] = static_cast<char>(on >> 8);
    data[3] = static_cast<char>(off & 0xFF);
    data[4] = static_cast<char>(off >> 8);

    _i2c.write(_addr, data, 5);
}

void ServoPCA9685::writeUs(uint8_t channel, uint16_t pulse_us)
{
    uint16_t ticks = (pulse_us * 4096UL) / 20000UL;
    setPwm(channel, 0, ticks);
}

float ServoPCA9685::applyConfig(uint8_t channel, float angle_deg)
{
    if (channel >= SERVO_COUNT)
        return angle_deg;

    if (angle_deg < 0.0f) angle_deg = 0.0f;
    if (angle_deg > 180.0f) angle_deg = 180.0f;

    float corrected = angle_deg + _offset[channel];

    float out;

    if (_mirrored[channel]) {
        out = 90.0f - corrected;
    } else {
        out = corrected;
    }

    if (out < 0.0f) out = 0.0f;
    if (out > 180.0f) out = 180.0f;

    return out;
}