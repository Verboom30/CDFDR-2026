#include "mbed.h"

I2C i2c(PB_9, PB_8); // SDA, SCL

#define PCA9685_ADDR_7BIT  0x40
#define PCA9685_ADDR       (PCA9685_ADDR_7BIT << 1)

#define MODE1              0x00
#define MODE2              0x01
#define PRESCALE           0xFE
#define LED0_ON_L          0x06

#define SERVO_FREQ_HZ      50
#define SERVO_COUNT        4

// === CONFIG SERVOS ===

// true = symétrique (miroir)
bool servo_mirrored[SERVO_COUNT] = {
    false,  // servo 0
    true,   // servo 1
    false,  // servo 2
    true    // servo 3
};

// offset fin (réglage mécanique)
float servo_offset[SERVO_COUNT] = {
    0.0f,   // servo 0
    -5.0f,   // servo 1
    0.0f,   // servo 2
    0.0f    // servo 3
};

// =====================

bool write_reg(uint8_t reg, uint8_t value)
{
    char data[2] = { (char)reg, (char)value };
    return i2c.write(PCA9685_ADDR, data, 2) == 0;
}

bool read_reg(uint8_t reg, uint8_t &value)
{
    char r = reg;
    char v = 0;

    if (i2c.write(PCA9685_ADDR, &r, 1, true) != 0) return false;
    if (i2c.read(PCA9685_ADDR, &v, 1) != 0) return false;

    value = (uint8_t)v;
    return true;
}

void pca9685_set_pwm_freq(float freq_hz)
{
    float prescaleval = 25000000.0f / 4096.0f / freq_hz - 1.0f;
    uint8_t prescale = (uint8_t)(prescaleval + 0.5f);

    uint8_t oldmode = 0;
    read_reg(MODE1, oldmode);

    write_reg(MODE1, (oldmode & 0x7F) | 0x10);
    write_reg(PRESCALE, prescale);
    write_reg(MODE1, oldmode);

    ThisThread::sleep_for(5ms);

    write_reg(MODE1, oldmode | 0xA1);
}

void pca9685_set_pwm(uint8_t channel, uint16_t on, uint16_t off)
{
    uint8_t reg = LED0_ON_L + 4 * channel;

    char data[5];
    data[0] = reg;
    data[1] = on & 0xFF;
    data[2] = on >> 8;
    data[3] = off & 0xFF;
    data[4] = off >> 8;

    i2c.write(PCA9685_ADDR, data, 5);
}

void servo_write_us(uint8_t channel, uint16_t pulse_us)
{
    uint16_t ticks = (pulse_us * 4096UL) / 20000UL;
    pca9685_set_pwm(channel, 0, ticks);
}

// === TRANSFORMATION LOGIQUE → PHYSIQUE ===
float apply_servo_config(uint8_t ch, float angle)
{
    if (ch >= SERVO_COUNT)
        return angle;

    // 1. Clamp entrée
    if (angle < 0.0f) angle = 0.0f;
    if (angle > 180.0f) angle = 180.0f;

    // 2. Offset appliqué AVANT inversion
    float corrected = angle + servo_offset[ch];

    // 3. Inversion si nécessaire
    float out;
    if (servo_mirrored[ch]) {
        out = 90.0f - corrected;
    } else {
        out = corrected;
    }

    // 4. Clamp final
    if (out < 0.0f) out = 0.0f;
    if (out > 180.0f) out = 180.0f;

    return out;
}

void servo_angle(uint8_t channel, float angle_deg)
{
    if (channel >= SERVO_COUNT)
        return;

    float final_angle = apply_servo_config(channel, angle_deg);

    uint16_t pulse = 500 + (uint16_t)((final_angle / 180.0f) * 2000.0f);
    servo_write_us(channel, pulse);
}

bool pca9685_init()
{
    write_reg(MODE1, 0x00);
    write_reg(MODE2, 0x04);

    ThisThread::sleep_for(10ms);

    pca9685_set_pwm_freq(SERVO_FREQ_HZ);

    return true;
}

int main()
{
    i2c.frequency(100000);

    printf("\r\nTest PCA9685 - 4 servos clean\r\n");

    uint8_t mode1 = 0;
    if (!read_reg(MODE1, mode1)) {
        printf("PCA9685 non detecte\r\n");
        while (true) {
            ThisThread::sleep_for(1s);
        }
    }

    printf("PCA9685 OK\r\n");

    pca9685_init();

    while (true)
    {
        
        servo_angle(0, 90);
        servo_angle(1, 90);
        servo_angle(2, 90);
        servo_angle(3, 90);
        ThisThread::sleep_for(1000ms);

        servo_angle(0, 55);
        servo_angle(1, 55);
        servo_angle(2, 55);
        servo_angle(3, 55);
        ThisThread::sleep_for(1000ms);
    }
}