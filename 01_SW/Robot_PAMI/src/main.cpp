#include "mbed.h"

I2C i2c(PB_7, PB_6); // SDA, SCL

#define MUX_ADDR_7BIT       0x70
#define TCS_ADDR_7BIT       0x29

#define MUX_ADDR            (MUX_ADDR_7BIT << 1)
#define TCS_ADDR            (TCS_ADDR_7BIT << 1)

#define TCS_ENABLE          0x80
#define TCS_ATIME           0x81
#define TCS_ID              0x92
#define TCS_CDATAL          0x94
#define TCS_RDATAL          0x96
#define TCS_GDATAL          0x98
#define TCS_BDATAL          0x9A

#define TCS_PON             0x01
#define TCS_AEN             0x02

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

TcsColor baseline[4];
bool baseline_ok[4] = {false, false, false, false};

bool select_mux_channel(uint8_t channel)
{
    if (channel > 3) return false;

    char control = 0x04 | channel; // PI4MSD5V9544A / PCA9544A
    return i2c.write(MUX_ADDR, &control, 1) == 0;
}

bool write_reg(uint8_t reg, uint8_t value)
{
    char data[2];
    data[0] = reg;
    data[1] = value;

    return i2c.write(TCS_ADDR, data, 2) == 0;
}

bool read_reg8(uint8_t reg, uint8_t &value)
{
    char r = reg;
    char v = 0;

    if (i2c.write(TCS_ADDR, &r, 1, true) != 0) return false;
    if (i2c.read(TCS_ADDR, &v, 1) != 0) return false;

    value = static_cast<uint8_t>(v);
    return true;
}

bool read_reg16(uint8_t reg, uint16_t &value)
{
    char r = reg;
    char data[2];

    if (i2c.write(TCS_ADDR, &r, 1, true) != 0) return false;
    if (i2c.read(TCS_ADDR, data, 2) != 0) return false;

    value = static_cast<uint16_t>(data[0]) |
            (static_cast<uint16_t>(data[1]) << 8);

    return true;
}

bool init_tcs()
{
    uint8_t id = 0;

    if (!read_reg8(TCS_ID, id)) {
        return false;
    }

    if (id != 0x93) {
        printf("ID inattendu : 0x%02X\r\n", id);
        return false;
    }

    // Temps d'intégration environ 50 ms
    if (!write_reg(TCS_ATIME, 0xEB)) return false;

    // Power ON
    if (!write_reg(TCS_ENABLE, TCS_PON)) return false;
    ThisThread::sleep_for(5ms);

    // Power ON + ADC enable
    if (!write_reg(TCS_ENABLE, TCS_PON | TCS_AEN)) return false;
    ThisThread::sleep_for(60ms);

    return true;
}

bool read_color(TcsColor &c)
{
    if (!read_reg16(TCS_CDATAL, c.clear)) return false;
    if (!read_reg16(TCS_RDATAL, c.red)) return false;
    if (!read_reg16(TCS_GDATAL, c.green)) return false;
    if (!read_reg16(TCS_BDATAL, c.blue)) return false;

    return true;
}

bool calibrate_baseline()
{
    printf("\r\nCalibration baseline : ne rien mettre devant les capteurs\r\n");
    ThisThread::sleep_for(1500ms);

    for (uint8_t ch = 0; ch < 4; ch++)
    {
        if (!select_mux_channel(ch)) {
            printf("Canal %d : erreur mux calibration\r\n", ch);
            baseline_ok[ch] = false;
            continue;
        }

        ThisThread::sleep_for(20ms);

        uint32_t sum_c = 0;
        uint32_t sum_r = 0;
        uint32_t sum_g = 0;
        uint32_t sum_b = 0;

        bool ok = true;

        for (int i = 0; i < 10; i++)
        {
            TcsColor c;

            if (!read_color(c)) {
                ok = false;
                break;
            }

            sum_c += c.clear;
            sum_r += c.red;
            sum_g += c.green;
            sum_b += c.blue;

            ThisThread::sleep_for(60ms);
        }

        if (!ok) {
            printf("Canal %d : erreur lecture calibration\r\n", ch);
            baseline_ok[ch] = false;
            continue;
        }

        baseline[ch].clear = sum_c / 10;
        baseline[ch].red   = sum_r / 10;
        baseline[ch].green = sum_g / 10;
        baseline[ch].blue  = sum_b / 10;

        baseline_ok[ch] = true;

        printf("Baseline canal %d : C=%u R=%u G=%u B=%u\r\n",
               ch,
               baseline[ch].clear,
               baseline[ch].red,
               baseline[ch].green,
               baseline[ch].blue);
    }

    printf("Calibration terminee\r\n\r\n");
    return true;
}

ColorDetected detect_color(uint8_t ch, const TcsColor &c)
{
    if (ch > 3 || !baseline_ok[ch]) {
        return COLOR_NONE;
    }

    uint16_t c0 = baseline[ch].clear;

    float r = (float)c.red   / (float)c.clear;
    float g = (float)c.green / (float)c.clear;
    float b = (float)c.blue  / (float)c.clear;

    // proximité
    bool close_blue =
        c.clear > c0 + 45 &&
        c.clear > c0 * 1.12f;

    bool close_yellow =
        c.clear > c0 + 110 &&
        c.clear > c0 * 1.35f;

    //-------------------------------------------------
    // BLEU RAL 5017
    //-------------------------------------------------
    if (
        close_blue &&
        r < 0.34f &&          // peu de rouge
        b > 0.34f &&          // bleu élevé
        b > r * 1.35f &&
        b > g * 1.05f
    ) {
        return COLOR_BLUE;
    }

    //-------------------------------------------------
    // JAUNE RAL 1023
    //-------------------------------------------------
    if (
        close_yellow &&
        r > 0.34f &&
        g > 0.28f &&
        b < 0.24f &&
        r > b * 1.60f &&
        g > b * 1.35f
    ) {
        return COLOR_YELLOW;
    }

    return COLOR_NONE;
}

const char* color_to_string(ColorDetected color)
{
    switch (color) {
        case COLOR_BLUE:
            return "BLEU";

        case COLOR_YELLOW:
            return "JAUNE";

        default:
            return "RIEN";
    }
}

int main()
{
    i2c.frequency(100000);

    printf("\r\nInitialisation des 4 TCS34007 via mux\r\n");

    for (uint8_t ch = 0; ch < 4; ch++)
    {
        printf("Init capteur canal %d : ", ch);

        if (!select_mux_channel(ch)) {
            printf("erreur mux\r\n");
            continue;
        }

        ThisThread::sleep_for(20ms);

        if (init_tcs()) {
            printf("OK\r\n");
        } else {
            printf("ECHEC\r\n");
        }
    }

    calibrate_baseline();

    printf("Lecture couleurs\r\n");

    while (true)
    {
        for (uint8_t ch = 0; ch < 4; ch++)
        {
            if (!select_mux_channel(ch)) {
                printf("Canal %d : erreur mux\r\n", ch);
                continue;
            }

            ThisThread::sleep_for(10ms);

            TcsColor c;

            if (!read_color(c)) {
                printf("Canal %d : erreur lecture capteur\r\n", ch);
                continue;
            }

            ColorDetected color = detect_color(ch, c);

            printf("Canal %d : C=%5u R=%5u G=%5u B=%5u | Base C=%5u -> %s\r\n",
                   ch,
                   c.clear,
                   c.red,
                   c.green,
                   c.blue,
                   baseline[ch].clear,
                   color_to_string(color));
        }

        printf("----------------------------------------\r\n");
        ThisThread::sleep_for(500ms);
    }
}