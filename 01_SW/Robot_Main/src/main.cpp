#include "mbed.h"
#include "Stepper.hpp"
#include "Holonome.hpp"
#include "pinout.hpp"
#include "ServoPCA9685.hpp"
#include "main_pck.hpp"
#include "ColorSensor.hpp"
//***********************************/************************************
//                                 MOVE                                 //
//***********************************/************************************
Stepper stepA(STEP_A,DIR_A);
Stepper stepB(STEP_B,DIR_B);
Stepper stepC(STEP_C,DIR_C);
DigitalOut En_drive_N(EN_DRIVE_N);
DigitalIn SW_Drive(SW_SPARE_1);
Holonome robot(&stepA, &stepB, &stepC, &stopLidar);
//***********************************/************************************
//                                 SERVO                                //
//***********************************/*************************************
I2C i2c(SDA, SCL);
ServoPCA9685 servoCard1(i2c, 0x40);
ServoPCA9685 servoCard2(i2c, 0x41);
ServoPCA9685 servoCard3(i2c, 0x42);
//***********************************/************************************
//                                 SENSOR                               //
//***********************************/************************************
TCS34007Mux sensor1(i2c, 0x70);
TCS34007Mux sensor2(i2c, 0x71);
TCS34007Mux sensor3(i2c, 0x72);
TCS34007Mux* cards[3] = {
    &sensor1,
    &sensor2,
    &sensor3
};
//***********************************/************************************
//                                 LIDAR                               //
//***********************************/************************************
DigitalOut Led(LIDAR_LED);

bool stopLidar = false;

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

void readsensor(){
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
}

void configureCard(ServoPCA9685& card)
{
    card.setMirrored(0, false);
    card.setMirrored(1, true);
    card.setMirrored(2, false);
    card.setMirrored(3, true);
    card.setMirrored(4, false);
    card.setMirrored(5, false);
    card.setMirrored(6, true);
    card.setMirrored(7, false);

    card.setOffset(0, 0.0f);
    card.setOffset(1, -5.0f);
    card.setOffset(2, 0.0f);
    card.setOffset(3, 0.0f);
    card.setOffset(4, 0.0f);
    card.setOffset(5, 0.0f);
    card.setOffset(6, 0.0f);
    card.setOffset(7, 0.0f);
}

void Home_Servo()
{
    servoCard2.setServoAngle(pince_top, 35);
    servoCard2.setServoAngle(pince_g,   50); 
    servoCard2.setServoAngle(pince_d,   50); 
    servoCard2.setServoAngle(pince1,    90); 
    servoCard2.setServoAngle(pince2,    90); 
    servoCard2.setServoAngle(pince3,    90);
    servoCard2.setServoAngle(pince4,    90);
    ThisThread::sleep_for(500ms);
    servoCard2.setServoAngle(bras, 18);
}

void Prise_Caise()
{
    servoCard2.setServoAngle(bras, 80);

    servoCard2.setServoAngle(pince_top, 90);
    servoCard2.setServoAngle(pince_g,   80); 
    servoCard2.setServoAngle(pince_d,   80); 
    servoCard2.setServoAngle(pince1,    60); 
    servoCard2.setServoAngle(pince2,    60); 
    servoCard2.setServoAngle(pince3,    60);
    servoCard2.setServoAngle(pince4,    60);
    ThisThread::sleep_for(500ms);
    servoCard2.setServoAngle(bras, 140);
    ThisThread::sleep_for(250ms);
    servoCard2.setServoAngle(pince1,    90); 
    servoCard2.setServoAngle(pince2,    90); 
    servoCard2.setServoAngle(pince3,    90);
    servoCard2.setServoAngle(pince4,    90);
    ThisThread::sleep_for(500ms);
   
    servoCard2.setServoAngle(pince_g,   40); 
    servoCard2.setServoAngle(pince_d,   40); 
    ThisThread::sleep_for(250ms);
    servoCard2.setServoAngle(pince_g,   50); 
    servoCard2.setServoAngle(pince_d,   50); 
    ThisThread::sleep_for(250ms);
    servoCard2.setServoAngle(bras, 150);
    servoCard2.setServoAngle(pince_top, 20);
    ThisThread::sleep_for(250ms);
    servoCard2.setServoAngle(pince_g,   35); 
    servoCard2.setServoAngle(pince_d,   35); 
    ThisThread::sleep_for(250ms);
    servoCard2.setServoAngle(bras, 18);
}


int main()
{
    En_drive_N = SW_Drive;
    SW_Drive.mode(PullUp);

    i2c.frequency(100000);
    initSensor();
    printf("\r\nTest 3 cartes PCA9685\r\n");
    configureCard(servoCard1);
    configureCard(servoCard2);
    configureCard(servoCard3);
    servoCard1.begin();
    servoCard2.begin();
    servoCard3.begin();
    Home_Servo();
    
    

    while (true) {
    
    }
}