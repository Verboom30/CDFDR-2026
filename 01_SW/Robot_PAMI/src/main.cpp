#include "mbed.h"
#include <inttypes.h>
#include <cmath>

#include "pinout.hpp"
#include "main_pck.hpp"
#include "Stepper.hpp"
#include "UART_TMC.hpp"
#include "Differentiel.hpp"
#include "VL53L0X_mbed.hpp"

//#define DEBUG

#define NORMALSPEED 1.0f
#define SLOWSPEED   0.4f

//*****************************************************************************
//                                 UART_TMC
//*****************************************************************************
Uart_TMC TMCSerial(TMC_UART_TX, TMC_UART_RX, R_SENSE);
BufferedSerial pc(USBTX, USBRX, 230400);

//*****************************************************************************
//                                   MOVE
//*****************************************************************************
bool StopMove      = false;
bool Fin_de_match  = false;
bool Couleur_Team  = false;   // false = bleu / true = jaune

Stepper StepperG(STEP_G, DIR_G, false);
Stepper StepperD(STEP_D, DIR_D, true);

differentiel RobotDiff(&StepperG, &StepperD, &StopMove);

//*****************************************************************************
//                                  SWITCH
//*****************************************************************************
DigitalIn SW_team(SW_TEAM);
DigitalIn SW_bau(SW_BAU);
DigitalIn SW_Tirette(TIRETTE);

//*****************************************************************************
//                                  SERVO
//*****************************************************************************
PwmOut Servo1(SERVO_1);
PwmOut Servo2(SERVO_2);

//*****************************************************************************
//                                   LED
//*****************************************************************************
DigitalOut LedR(LED_R);
DigitalOut LedG(LED_G);
DigitalOut LedB(LED_B);

//*****************************************************************************
//                              LASER SENSOR
//*****************************************************************************
Thread sensor_thread;

I2C i2c(VL53L0_I2C_SDA, VL53L0_I2C_SCL);
DigitalOut xshut(XSHUT_1);

VL53L0X_mbed* g_tof = nullptr;

//*****************************************************************************
//                                VARIABLES
//*****************************************************************************
int state         = 0;
int score         = 0;
int offset_posX   = 0;
int offset_Alpha  = 1;

volatile bool end_match = false;

Timeout endMatch;
Thread game_thread;

//*****************************************************************************
//                             SENSOR THREAD
//*****************************************************************************
void checkSensors()
{
    printf("Thread sensor start\r\n");

    i2c.frequency(400000);

    ThisThread::sleep_for(10ms);
    xshut = 1;
    ThisThread::sleep_for(50ms);

    // while (g_tof == nullptr)
    //     ThisThread::sleep_for(10ms);

    g_tof->setTimeout(200);

    printf("Init sensor...\r\n");
    ThisThread::sleep_for(100ms);

    if (!g_tof->init())
    {
        printf("VL53L0X init failed\r\n");

        while (true)
        {
            ThisThread::sleep_for(500ms);
        }
    }

    printf("Sensor OK\r\n");

    while (true)
    {
        uint16_t distance = g_tof->readRangeSingleMillimeters();

        if (g_tof->timeoutOccurred() || distance == 65535)
        {
            printf("Read error\r\n");
        }
        else
        {
            printf("Distance = %u mm\r\n", distance);
            StopMove = (distance < 150) ? 1 : 0;
            LedR = StopMove;
        }

        ThisThread::sleep_for(100ms);
    }
}

//*****************************************************************************
//                              END MATCH
//*****************************************************************************
void endMatchProcess()
{
    end_match = true;
}

//*****************************************************************************
//                            UTILS FUNCTIONS
//*****************************************************************************
float theta2pluse(int theta)
{
    return 500.0f + (100.0f / 9.0f) * float(theta);
}

void printPosition()
{
    // printf pour processing
    printf("%f;%f;%f\r\n",
           RobotDiff.getPositionX(),
           RobotDiff.getPositionY(),
           RobotDiff.getAlpha());
}

//*****************************************************************************
//                      THREAD AFFICHAGE POSITION
//*****************************************************************************
void routineAffichage()
{
    while (true)
    {
        printPosition();
        ThisThread::sleep_for(25ms);
    }
}

//*****************************************************************************
//                                MAIN FSM
//*****************************************************************************
void main_thread(void)
{
    FsmState = IDLE;

    while (1)
    {
        switch (FsmState)
        {
            case IDLE:
                RobotDiff.setPosition(0, 0, 0, Couleur_Team);

                RobotDiff.Robotgoto(0,    1000,   90, Couleur_Team, NORMALSPEED);
                RobotDiff.Robotgoto(1000, 1000,  180, Couleur_Team, NORMALSPEED);
                RobotDiff.Robotgoto(1000, 0,     -90, Couleur_Team, NORMALSPEED);
                RobotDiff.Robotgoto(0,    0,       0, Couleur_Team, NORMALSPEED);

                FsmState = END;
                break;

            case START_UP:
                break;

            case CAL:
                break;

            case WAIT_MATCH:
                break;

            case GAME:
                FsmState = END;
                break;

            case END:
                break;
        }

        ThisThread::sleep_for(10ms);
    }
}

//*****************************************************************************
//                                   MAIN
//*****************************************************************************
int main()
{
#ifdef DEBUG
    Thread threadAffichage;
    threadAffichage.start(routineAffichage);
#endif

    printf("Nucleo ON\r\n");

    SW_team.mode(PullUp);
    SW_bau.mode(PullUp);
    SW_Tirette.mode(PullUp);

    LedR = 0;
    LedG = 0;
    LedB = 0;

    TMCSerial.setup_all_stepper();

    static VL53L0X_mbed tof(i2c);   // adresse par défaut 0x29 (7 bits)
    g_tof = &tof;

    sensor_thread.start(checkSensors);
    game_thread.start(main_thread);

    while (1)
    {
        if ((end_match || SW_bau == 1) && (Fin_de_match == false))
        {
            Fin_de_match = true;

            // game_thread.terminate();
            // sensor_thread.terminate();
        }

        ThisThread::sleep_for(1s);
    }
}