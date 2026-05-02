#include "ColorSensor.hpp"

TCS34007Mux::TCS34007Mux(I2C& i2c, uint8_t mux_addr_7bit)
    : _i2c(i2c),
      _muxAddr(mux_addr_7bit << 1)
{
    for (int i = 0; i < 4; i++)
    {
        _baselineOk[i] = false;
        _yellowOk[i] = false;
    }
}

bool TCS34007Mux::begin()
{
    bool ok = true;

    for (uint8_t ch = 0; ch < 4; ch++)
    {
        printf("Init capteur %d : ", ch);

        if (!selectChannel(ch))
        {
            printf("MUX FAIL\r\n");
            ok = false;
            continue;
        }

        ThisThread::sleep_for(20ms);

        if (initSensor())
            printf("OK\r\n");
        else
        {
            printf("FAIL\r\n");
            ok = false;
        }
    }

    return ok;
}

bool TCS34007Mux::calibrateBaseline()
{
    printf("\r\n=== CALIBRATION BASELINE (rien devant) ===\r\n");
    ThisThread::sleep_for(1500ms);

    for (uint8_t ch = 0; ch < 4; ch++)
    {
        selectChannel(ch);
        ThisThread::sleep_for(20ms);

        uint32_t sum = 0;

        for (int i = 0; i < 10; i++)
        {
            TcsColor c;
            readColorRaw(c);
            sum += c.clear;
            ThisThread::sleep_for(50ms);
        }

        _baseline[ch].clear = sum / 10;
        _baselineOk[ch] = true;

        printf("Baseline ch%d = %u\r\n", ch, _baseline[ch].clear);
    }

    return true;
}

bool TCS34007Mux::calibrateYellow()
{
    printf("\r\n=== CALIBRATION JAUNE ===\r\n");
    ThisThread::sleep_for(2000ms);

    for (uint8_t ch = 0; ch < 4; ch++)
    {
        printf("Capteur %d (mettre jaune proche)\r\n", ch);

        selectChannel(ch);
        ThisThread::sleep_for(50ms);

        uint32_t sumC=0,sumR=0,sumG=0,sumB=0;

        for (int i = 0; i < 10; i++)
        {
            TcsColor c;
            readColorRaw(c);

            sumC += c.clear;
            sumR += c.red;
            sumG += c.green;
            sumB += c.blue;

            ThisThread::sleep_for(50ms);
        }

        float C = sumC / 10.0f;

        _yellowR[ch] = (sumR / 10.0f) / C;
        _yellowG[ch] = (sumG / 10.0f) / C;
        _yellowB[ch] = (sumB / 10.0f) / C;

        _yellowOk[ch] = true;

        printf("Jaune ch%d : r=%.2f g=%.2f b=%.2f\r\n",
               ch,_yellowR[ch],_yellowG[ch],_yellowB[ch]);
    }

    return true;
}

bool TCS34007Mux::readSensor(uint8_t channel, TcsColor& color)
{
    if (channel > 3)
        return false;

    if (!selectChannel(channel))
        return false;

    ThisThread::sleep_for(10ms);

    return readColorRaw(color);
}

TCS34007Mux::ColorDetected
TCS34007Mux::detectColor(uint8_t ch, const TcsColor& c)
{
    if (ch > 3 || !_baselineOk[ch] || c.clear == 0)
        return COLOR_NONE;

    float r = (float)c.red   / (float)c.clear;
    float g = (float)c.green / (float)c.clear;
    float b = (float)c.blue  / (float)c.clear;

    uint16_t base = _baseline[ch].clear;

    bool closeBlue =
        c.clear > base + 45 &&
        c.clear > base * 1.12f;

    bool closeYellow =
    c.clear > base + 80 &&
    c.clear > base * 1.25f;

    // BLEU
    if (closeBlue &&
        r < 0.34f &&
        b > 0.34f &&
        b > r * 1.35f &&
        b > g * 1.05f)
    {
        return COLOR_BLUE;
    }

    // Si calibration jaune faite, on utilise celle du capteur.
    // Sinon on utilise la moyenne par défaut des 4 capteurs.
    float yellowR = _yellowOk[ch] ? _yellowR[ch] : DEFAULT_YELLOW_R;
    float yellowG = _yellowOk[ch] ? _yellowG[ch] : DEFAULT_YELLOW_G;
    float yellowB = _yellowOk[ch] ? _yellowB[ch] : DEFAULT_YELLOW_B;

    float dr = fabsf(r - yellowR);
    float dg = fabsf(g - yellowG);
    float db = fabsf(b - yellowB);

    if (closeYellow &&
        dr < YELLOW_TOLERANCE &&
        dg < YELLOW_TOLERANCE &&
        db < YELLOW_TOLERANCE &&
        b < 0.25f)
    {
        return COLOR_YELLOW;
    }

    return COLOR_NONE;
}
TCS34007Mux::ColorDetected
TCS34007Mux::readAndDetect(uint8_t ch, TcsColor& c)
{
    if (!readSensor(ch,c)) return COLOR_NONE;
    return detectColor(ch,c);
}

const char* TCS34007Mux::colorToString(ColorDetected c)
{
    if (c==COLOR_BLUE) return "BLEU";
    if (c==COLOR_YELLOW) return "JAUNE";
    return "RIEN";
}

// -------- I2C bas niveau --------

bool TCS34007Mux::selectChannel(uint8_t ch)
{
    char cmd = 0x04 | ch;
    return _i2c.write(_muxAddr, &cmd, 1) == 0;
}

bool TCS34007Mux::initSensor()
{
    uint8_t id;
    if (!readReg8(REG_ID,id)) return false;
    if (id != 0x93) return false;

    writeReg(REG_ATIME,0xEB);
    writeReg(REG_ENABLE,ENABLE_PON);
    ThisThread::sleep_for(5ms);
    writeReg(REG_ENABLE,ENABLE_PON|ENABLE_AEN);
    ThisThread::sleep_for(50ms);

    return true;
}

bool TCS34007Mux::writeReg(uint8_t r,uint8_t v)
{
    char d[2]={r,v};
    return _i2c.write(TCS_ADDR,d,2)==0;
}

bool TCS34007Mux::readReg8(uint8_t r,uint8_t& v)
{
    char rr=r, vv;
    if (_i2c.write(TCS_ADDR,&rr,1,true)) return false;
    if (_i2c.read(TCS_ADDR,&vv,1)) return false;
    v=vv;
    return true;
}

bool TCS34007Mux::readReg16(uint8_t r,uint16_t& v)
{
    char rr=r,d[2];
    if (_i2c.write(TCS_ADDR,&rr,1,true)) return false;
    if (_i2c.read(TCS_ADDR,d,2)) return false;
    v = d[0] | (d[1]<<8);
    return true;
}

bool TCS34007Mux::disable()
{
    char control = 0x00; // désactive tous les canaux du mux
    return _i2c.write(_muxAddr, &control, 1) == 0;
}

bool TCS34007Mux::readColorRaw(TcsColor& c)
{
    return readReg16(REG_CDATAL,c.clear) &&
           readReg16(REG_RDATAL,c.red) &&
           readReg16(REG_GDATAL,c.green) &&
           readReg16(REG_BDATAL,c.blue);
}