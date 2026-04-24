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

#define STARTMATCH 2s
#define ENDMATCH 15s

//*****************************************************************************
//                                 UART_TMC
//*****************************************************************************
Uart_TMC TMCSerial(TMC_UART_TX, TMC_UART_RX, R_SENSE);
BufferedSerial pc(USBTX, USBRX, 230400);

//*****************************************************************************
//                                   MOVE
//*****************************************************************************
volatile bool StopMove = false;
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
Thread sensor_thread(osPriorityNormal, 8192);

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

volatile bool end_match   = false;
volatile bool start_match = false;
bool LatchBau = false;

Thread game_thread(osPriorityNormal, 8192);
Thread endMatch_thread(osPriorityNormal, 512);
Thread Servo_thread(osPriorityNormal, 512);

//*****************************************************************************
//                             SENSOR THREAD
//*****************************************************************************
void checkSensors()
{
    //printf("Thread sensor start\r\n");

    i2c.frequency(400000);

    ThisThread::sleep_for(10ms);
    xshut = 1;
    ThisThread::sleep_for(50ms);

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

    //printf("Sensor OK\r\n");

    while (true)
    {
        if (FsmState == GAME)
        {
            uint16_t distance = g_tof->readRangeSingleMillimeters();
            if (g_tof->timeoutOccurred() || distance == 65535)
            {
                //printf("Read error\r\n");
            }
            else
            {
                //printf("Distance = %u mm\r\n", distance);
                StopMove = (distance < 200) ? 1 : 0;
                LedR = StopMove;
                LedG = StopMove;
                LedB = StopMove;
            }
        }
        ThisThread::sleep_for(50ms);
    }
}

//*****************************************************************************
//                              END MATCH THREAD
//*****************************************************************************
void endMatchProcess()
{
    ThisThread::sleep_for(STARTMATCH);
    start_match = true;
    //printf("Debut du match !\r\n");
    ThisThread::sleep_for(ENDMATCH - STARTMATCH);
    end_match = true;
    //printf("Fin du match !\r\n");
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
//                                DRIVE
//*****************************************************************************
void taskDrive()
{


RobotDiff.setPosition(375, 1775, 180, Couleur_Team);
RobotDiff.Robotgoto(380, 1196, 0, Couleur_Team, NORMALSPEED);  // Step 1
RobotDiff.Robotgoto(776, 1055, 0, Couleur_Team, NORMALSPEED);  // Step 2
RobotDiff.Robotgoto(470, 769, 0, Couleur_Team, NORMALSPEED);  // Step 3
RobotDiff.Robotgoto(909, 519, 0, Couleur_Team, NORMALSPEED);  // Step 4
RobotDiff.Robotgoto(1061, 1004, 0, Couleur_Team, NORMALSPEED);  // Step 5
RobotDiff.Robotgoto(793, 1244, 0, Couleur_Team, NORMALSPEED);  // Step 6
RobotDiff.Robotgoto(508, 1603, 0, Couleur_Team, NORMALSPEED);  // Step 7
RobotDiff.Robotgoto(375, 1775, 0, Couleur_Team, NORMALSPEED);  // Step 8 - POI startYellow






}

//*****************************************************************************
//                          ACTION SERVO FIN DE MATCH
//*****************************************************************************
// Exécute la séquence servo de fin de match.
// Vérifie le BAU avant chaque étape — si BAU actif, arrêt immédiat.
void servoFinDeMatch()
{
    while (1)
    {
        Servo1.pulsewidth_us(theta2pluse(140));
        ThisThread::sleep_for(400ms);
        Servo1.pulsewidth_us(theta2pluse(15));
        ThisThread::sleep_for(800ms);
    }
}

//*****************************************************************************
//                                MAIN FSM
//*****************************************************************************
void main_thread(void)
{
    FsmState = IDLE;

    Couleur_Team = !SW_team;
    if (Couleur_Team == false)
    {
        // bleu
        offset_posX  = 0;
        offset_Alpha = 1;
        LedR = 0;
        LedG = 0;
        LedB = 1;
    }
    else
    {
        // jaune
        offset_posX  = 3000;
        offset_Alpha = -1;
        LedR = 1;
        LedG = 1;
        LedB = 0;
    }

    while (1)
    {
        if (Fin_de_match) FsmState = END;

        switch (FsmState)
        {
        case IDLE:
            if (SW_Tirette == 1 && FsmState != END)
            {
                endMatch_thread.start(endMatchProcess);
                FsmState = WAIT_START;
            }
            break;

        case WAIT_START:
            if (start_match == true) FsmState = GAME;
            break;

        case GAME:
            taskDrive();
            FsmState = END;
            break;

        case END:
            break;
        }
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
    Servo1.period_ms(20);
    Servo1.pulsewidth_us(theta2pluse(30));

    TMCSerial.setup_all_stepper();

    static VL53L0X_mbed tof(i2c);
    g_tof = &tof;

    sensor_thread.start(mbed::callback(checkSensors));
    game_thread.start(main_thread);

    while (1)
    {
        if(SW_bau != 1 ) LatchBau = true;
        if ((end_match || LatchBau == true) && (Fin_de_match == false))
        {
            Fin_de_match = true;
            game_thread.terminate();
            sensor_thread.terminate();
            RobotDiff.stop();
            LedR = 1;
            LedG = 0;
            LedB = 0;
        }

         if (Fin_de_match){
            if(LatchBau == false) Servo_thread.start(servoFinDeMatch);
            else 
            {
                Servo_thread.terminate();
                Servo1.pulsewidth_us(theta2pluse(15));
                
                //Servo1 = 0.0;
            }
         }
    }
}
