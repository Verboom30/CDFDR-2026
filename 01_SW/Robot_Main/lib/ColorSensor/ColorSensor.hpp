#ifndef COLOR_SENSOR_HPP
#define COLOR_SENSOR_HPP

#include "mbed.h"

static constexpr float DEFAULT_YELLOW_R = 0.3625f;
static constexpr float DEFAULT_YELLOW_G = 0.4125f;
static constexpr float DEFAULT_YELLOW_B = 0.1825f;

static constexpr float YELLOW_TOLERANCE = 0.065f;

class TCS34007Mux
{
public:
    enum ColorDetected {
        COLOR_NONE,
        COLOR_BLUE,
        COLOR_YELLOW
    };

    struct TcsColor {
        uint16_t clear;
        uint16_t red;
        uint16_t green;
        uint16_t blue;
    };

    TCS34007Mux(I2C& i2c, uint8_t mux_addr_7bit);

    bool begin();
    bool calibrateBaseline();
    bool calibrateYellow();

    bool readSensor(uint8_t channel, TcsColor& color);
    ColorDetected detectColor(uint8_t channel, const TcsColor& color);
    ColorDetected readAndDetect(uint8_t channel, TcsColor& color);

    static const char* colorToString(ColorDetected color);
    bool disable();

private:
    I2C& _i2c;
    uint8_t _muxAddr;

    TcsColor _baseline[4];
    bool _baselineOk[4];

    float _yellowR[4];
    float _yellowG[4];
    float _yellowB[4];
    bool _yellowOk[4];

    // REGISTRES
    static constexpr uint8_t TCS_ADDR = (0x29 << 1);

    static constexpr uint8_t REG_ENABLE = 0x80;
    static constexpr uint8_t REG_ATIME  = 0x81;
    static constexpr uint8_t REG_ID     = 0x92;

    static constexpr uint8_t REG_CDATAL = 0x94;
    static constexpr uint8_t REG_RDATAL = 0x96;
    static constexpr uint8_t REG_GDATAL = 0x98;
    static constexpr uint8_t REG_BDATAL = 0x9A;

    static constexpr uint8_t ENABLE_PON = 0x01;
    static constexpr uint8_t ENABLE_AEN = 0x02;

private:
    bool selectChannel(uint8_t channel);
    bool initSensor();

    bool writeReg(uint8_t reg, uint8_t value);
    bool readReg8(uint8_t reg, uint8_t& value);
    bool readReg16(uint8_t reg, uint16_t& value);

    bool readColorRaw(TcsColor& c);
};

#endif