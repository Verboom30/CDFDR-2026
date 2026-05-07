#include "mbed.h"
#include "Stepper.hpp"
#include "Holonome.hpp"
#include "pinout.hpp"

Stepper stepA(STEP_A,DIR_A);
Stepper stepB(STEP_B,DIR_B);
Stepper stepC(STEP_C,DIR_C);
DigitalOut En_drive_N(EN_DRIVE_N);
DigitalOut Led(LIDAR_LED);

DigitalIn SW_Drive(SW_SPARE_1);
bool stopLidar = false;

Holonome robot(&stepA, &stepB, &stepC, &stopLidar);

int main()
{
    En_drive_N = SW_Drive;
    Led=! SW_Drive;
    SW_Drive.mode(PullUp);
    //robot.run();

    robot.setPosition(0, 0, 0);

    // déplacement holonome direct :
    
     robot.Robotmove(0, 1000, 0);      // +Y
     
    // robot.Robotmove(0, 0, 90);       // rotation

    // // aller à une position :
    // robot.Robotgoto(1000, 500, 0);

    while (true) {
        printf("X=%.1f Y=%.1f T=%.1f\r\n",
               robot.getPositionX(),
               robot.getPositionY(),
               robot.getTheta());

        ThisThread::sleep_for(500ms);
    }
}