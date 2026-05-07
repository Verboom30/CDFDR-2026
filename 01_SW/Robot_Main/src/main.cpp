#include "mbed.h"
#include "ServoPCA9685.hpp"

I2C i2c(PB_9, PB_8);

ServoPCA9685 servoCard0(i2c, 0x40);
ServoPCA9685 servoCard1(i2c, 0x41);
ServoPCA9685 servoCard2(i2c, 0x42);

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

void applyRobotPose(ServoPCA9685& card)
{
    card.setServoAngle(7, 25);

    card.setServoAngle(5, 30);

    card.setServoAngle(4, 50);
    card.setServoAngle(6, 50);

    card.setServoAngle(0, 90);
    card.setServoAngle(1, 90);
    card.setServoAngle(2, 90);
    card.setServoAngle(3, 90);
}


void Home_Servo()
{
    servoCard0.setServoAngle(5, 35);
    servoCard0.setServoAngle(4, 50); 
    servoCard0.setServoAngle(6, 50); 
    servoCard0.setServoAngle(0, 90); 
    servoCard0.setServoAngle(1, 90); 
    servoCard0.setServoAngle(2, 90);
    servoCard0.setServoAngle(3, 90);

    servoCard1.setServoAngle(5, 35);
    servoCard1.setServoAngle(4, 50); 
    servoCard1.setServoAngle(6, 50); 
    servoCard1.setServoAngle(0, 90); 
    servoCard1.setServoAngle(1, 90); 
    servoCard1.setServoAngle(2, 90);
    servoCard1.setServoAngle(3, 90);

    servoCard2.setServoAngle(5, 35);
    servoCard2.setServoAngle(4, 50); 
    servoCard2.setServoAngle(6, 50); 
    servoCard2.setServoAngle(0, 90); 
    servoCard2.setServoAngle(1, 90); 
    servoCard2.setServoAngle(2, 90);
    servoCard2.setServoAngle(3, 90);
    
    ThisThread::sleep_for(500ms);
    servoCard0.setServoAngle(7, 25);
    servoCard1.setServoAngle(7, 25);
    servoCard2.setServoAngle(7, 25);
}

int main()
{
    i2c.frequency(100000);

    printf("\r\nTest 3 cartes PCA9685\r\n");

    configureCard(servoCard0);
    configureCard(servoCard1);
    configureCard(servoCard2);

    servoCard0.begin();
    servoCard1.begin();
    servoCard2.begin();
    Home_Servo();
  

    while (true)
    {

        
    }
}