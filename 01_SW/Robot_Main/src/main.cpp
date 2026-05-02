#include "mbed.h"
#include "ColorSensor.hpp"

I2C i2c(PB_9, PB_8);

TCS34007Mux sensor1(i2c, 0x70);
TCS34007Mux sensor2(i2c, 0x71);
TCS34007Mux sensor3(i2c, 0x72);

TCS34007Mux* cards[3] = {
    &sensor1,
    &sensor2,
    &sensor3
};

void  disable_all_mux(){
    sensor1.disable();
    sensor2.disable();
    sensor3.disable();
    ThisThread::sleep_for(2ms);
}
void initSensor()
{
    printf("START Sensor\r\n");
    disable_all_mux();

    sensor1.begin();
    disable_all_mux();

    sensor2.begin();
    disable_all_mux();

    sensor3.begin();
    disable_all_mux();

    sensor1.calibrateBaseline();
    disable_all_mux();

    sensor2.calibrateBaseline();
    disable_all_mux();

    sensor3.calibrateBaseline();
    disable_all_mux();
}

int main()
{
    i2c.frequency(100000);
    initSensor();
    
    while (true)
    {
        for (uint8_t card = 0; card < 3; card++)
        {
            disable_all_mux();

            for (uint8_t ch = 0; ch < 4; ch++)
            {
                TCS34007Mux::TcsColor c;
                auto color = cards[card]->readAndDetect(ch, c);

                printf("Carte %d canal %d : C=%u R=%u G=%u B=%u -> %s\r\n",
                       card,
                       ch,
                       c.clear,
                       c.red,
                       c.green,
                       c.blue,
                       TCS34007Mux::colorToString(color));
            }
        }

        disable_all_mux();

        printf("----------------------------------------\r\n");
        ThisThread::sleep_for(1000ms);
    }
}