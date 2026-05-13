#include "mbed.h"
#include "Stepper.hpp"
#include "Holonome.hpp"
#include "pinout.hpp"
#include "lcd.hpp"
#include "ServoPCA9685.hpp"
#include "main_pck.hpp"
#include "ColorSensor.hpp"
#include "lidar.hpp"
#include "LidarAnalyzer.hpp"
#include "CommandAsserv.hpp"
#define NORMALSPEED 1.0f


//#define DEBUG
#define LIDAR


DigitalOut En_drive_N(EN_DRIVE_N);
DigitalOut En_servo_N(ENABLE_SERVO_N);

bool stopLidar = false;
bool Fin_de_match = false;
Team Couleur_Team = BLUE; 

I2C i2c(SDA, SCL);

ServoPCA9685 servoCard1(i2c, 0x40);
ServoPCA9685 servoCard2(i2c, 0x41);
ServoPCA9685 servoCard3(i2c, 0x42);

DigitalIn SW_init(SW_INIT);
DigitalIn SW_team(SW_TEAM);
DigitalIn SW_bau(SW_BAU);
DigitalIn SW_Tirette(TIRETTE);
DigitalIn SW_Drive(SW_SPARE_1);
DigitalIn SW_Stepper(SW_SPARE_2);

TCS34007Mux sensor1(i2c, 0x70);
TCS34007Mux sensor2(i2c, 0x71);
TCS34007Mux sensor3(i2c, 0x72);

TCS34007Mux::ColorResult colorResults[4];

DigitalOut led_lidar(LIDAR_LED);

Lidar* LidarLD19 = new Lidar(LIDAR_TX, LIDAR_RX, 230400);
BufferedSerial pc(USBTX, USBRX, 230400);
//CommandAsserv asserv(UART_TX, UART_RX, 115200);


LCD lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7, LCD16x2);

Stepper stepA(STEP_A, DIR_A, false);
Stepper stepB(STEP_B, DIR_B, false);
Stepper stepC(STEP_C, DIR_C, false);

Holonome Robot(&stepA, &stepB, &stepC, &stopLidar);
LidarAnalyzer LidaRayzer(LidarLD19, &Robot, &led_lidar);
volatile bool end_match = false;



Timeout endMatch;

Thread lcd_thread;
Thread game_thread;
Thread lidarAnalyzer_thread;
Thread drive_thread;
Thread broadcast_thread;

#ifdef DEBUG
Thread threadAffichage;
#endif

void lcdStatus(const char* line1, const char* line2 = "")
{
    lcd.cls();
    lcd.locate(0, 0);
    lcd.printf("%s", line1);
    lcd.locate(0, 1);
    lcd.printf("%s", line2);
}


void endMatchProcess()
{
    end_match = true;
}

// --- Debug position Robot / lidar ---
// void printPosition()
// {
//     CommandAsserv::BroadcastData data = asserv.getBroadcast();

//     /*
//      * Debug simple Processing
//      * Format :
//      * X ; Y ; Alpha
//      */
//     printf("%d;%d;%d\r\n",
//            data.x,
//            data.y,
//            data.alpha);

// }

void printPosition()
{
 
  //printf pour processing
//   printf("%f;%f;%f\r\n",
//         Robot.getPositionX(),
//         Robot.getPositionY(),
//         Robot.getTheta());
  
  printf("%d;%d;%d;%f;%f;%f;%f;%f;%f\r\n",
        LidaRayzer.isObstacleDetected(),
        int(Robot.getPositionX()),
        int(Robot.getPositionY()),
        Robot.getTheta(),
        LidaRayzer.getObstacleX(),
        LidaRayzer.getObstacleY(),
        Robot.getPosCibleX(),
        Robot.getPosCibleY(),
        LidaRayzer.getObstacleAngleCible());

  // printf("[Position] X = %.2f mm | Y = %.2f mm | Angle = %.2f°\n",
  //        Robot.getPositionX(),
  //        Robot.getPositionY(),
  //        Robot.getTheta());

  // printf("[Position] X = %.2f mm | Y = %.2f mm | Angle = %.2f° | posG = %d stp | posD = %d stp | StepG = %d stp | StepD = %d stp | SpeedG = %f stp/sec² | SpeedD = %f stp/sec² | DeltaG = %d | DeltaD = %d\n",
  //        Robot.getPositionX(),
  //        Robot.getPositionY(),
  //        Robot.getTheta(),
  //        Robot.getPosG(),
  //        Robot.getPosD(),
  //        Robot.getStepG(),
  //        Robot.getStepD(),
  //        Robot.getSpeedG(),
  //        Robot.getSpeedD(),
  //        Robot.getDeltaG(),
  //        Robot.getDeltaD()
  //       );
}

void routineAffichage()
{
    while (true)
    {
        printPosition();
        ThisThread::sleep_for(100ms);
    }
}

void print_lcd()
{
    while (true)
    {
        if (!end_match)
        {
            lcd.locate(0, 1);
            lcd.printf("Time :%d   ",
                       int(endMatch.remaining_time().count()) / 1000000);
        }

        ThisThread::sleep_for(200ms);
    }
}

// void thread_broadcast()
// {
//     while (true)
//     {
//         CommandAsserv::BroadcastData data = asserv.getBroadcast();

//         if (data.valid)
//         {
//             printf("ASSERV X=%d Y=%d A=%d VX=%d VY=%d VA=%d\r\n",
//                    data.x,
//                    data.y,
//                    data.alpha,
//                    data.vx,
//                    data.vy,
//                    data.valpha);
//         }

//         ThisThread::sleep_for(100ms);
//     }
// }

void thread_lidar()
{
    while (true)
    {
#ifdef LIDAR
        LidaRayzer.update();
        if(FsmState == GAME){
            stopLidar = LidaRayzer.isObstacleDetected();
        }else{
            stopLidar = 0;
        }
#endif
        ThisThread::sleep_for(5ms);
    }
}


void disable_all_mux()
{
    sensor1.disable();
    sensor2.disable();
    sensor3.disable();
    ThisThread::sleep_for(2ms);
}

void initSensor()
{
    printf("START Sensor\r\n");
    lcdStatus("START SENSOR");

    disable_all_mux();

    lcdStatus("Sensor1", "Init...");
    sensor1.begin();

    disable_all_mux();

    lcdStatus("Sensor2", "Init...");
    sensor2.begin();

    disable_all_mux();

    lcdStatus("Sensor3", "Init...");
    sensor3.begin();

    disable_all_mux();

    lcdStatus("Sensor1", "Calib...");
    sensor1.calibrateBaseline();

    disable_all_mux();

    lcdStatus("Sensor2", "Calib...");
    sensor2.calibrateBaseline();

    disable_all_mux();

    lcdStatus("Sensor3", "Calib...");
    sensor3.calibrateBaseline();

    disable_all_mux();
}

void ColorDect(TCS34007Mux& arms, TCS34007Mux::ColorResult results[4])
{
    disable_all_mux();
    ThisThread::sleep_for(5ms);
    for (uint8_t ch = 0; ch < 4; ch++)
    {
        results[ch].color = arms.readAndDetect(ch, results[ch].raw);

        printf("canal %d : C=%u R=%u G=%u B=%u -> %s\r\n",
               ch,
               results[ch].raw.clear,
               results[ch].raw.red,
               results[ch].raw.green,
               results[ch].raw.blue,
               TCS34007Mux::colorToString(results[ch].color));
    }
}

void Home_Servo(ServoPCA9685& servo)
{
    servo.setServoAngle(pince_top, 35);

    servo.setServoAngle(pince_g, 50);
    servo.setServoAngle(pince_d, 50);

    servo.setServoAngle(pince1, 90);
    servo.setServoAngle(pince2, 90);
    servo.setServoAngle(pince3, 90);
    servo.setServoAngle(pince4, 90);

    ThisThread::sleep_for(500ms);

    servo.setServoAngle(bras, 7);
}

void DeposeCaise1(uint8_t arm_id)
{
    TCS34007Mux* sensor = nullptr;
    ServoPCA9685* servo = nullptr;

    switch (arm_id)
    {
        case 1:
            sensor = &sensor1;
            servo  = &servoCard1;
            break;

        case 2:
            sensor = &sensor2;
            servo  = &servoCard2;
            break;

        case 3:
            sensor = &sensor3;
            servo  = &servoCard3;
            break;

        default:
            return;
    }

    ColorDect(*sensor, colorResults);

    for (size_t i = 0; i < 4; i++)
    {
        bool goodColor = false;

        if (Couleur_Team == BLUE) // équipe BLEU
        {
            goodColor = (colorResults[i].color == TCS34007Mux::COLOR_YELLOW);
        }
        else // équipe JAUNE
        {
            goodColor = (colorResults[i].color == TCS34007Mux::COLOR_BLUE);
        }

        if (goodColor)
        {
            servo->setServoAngle(i, 50);
        }
        else
        {
            servo->setServoAngle(i, 100);
        }
    }
}
void DeposeCaise2(uint8_t arm_id){

    ServoPCA9685* servo = nullptr;

    switch (arm_id)
    {
        case 1:
            servo  = &servoCard1;
            break;

        case 2:
            servo  = &servoCard2;
            break;
        case 3:
            servo  = &servoCard3;
            break;

        default:
            return;
    }

    servo->setServoAngle(pince_top, 15);
    ThisThread::sleep_for(250ms);
    servo->setServoAngle(bras, 130);
    ThisThread::sleep_for(500ms);
    servo->setServoAngle(pince_top, 35);
    servo->setServoAngle(pince_g, 50);
    servo->setServoAngle(pince_d, 50);
    servo->setServoAngle(pince1, 50);
    servo->setServoAngle(pince2, 50);
    servo->setServoAngle(pince3, 50);
    servo->setServoAngle(pince4, 50);
    ThisThread::sleep_for(500ms);
    servo->setServoAngle(bras, 7);
    ThisThread::sleep_for(500ms);
    servo->setServoAngle(pince1, 90);
    servo->setServoAngle(pince2, 90);
    servo->setServoAngle(pince3, 90);
    servo->setServoAngle(pince4, 90);
    
}


void Prise_Caise(uint8_t arm_id)
{

    ServoPCA9685* servo = nullptr;
    switch (arm_id)
    {
        case 1:
            servo  = &servoCard1;
            break;

        case 2:
            servo  = &servoCard2;
            break;
        case 3:
            servo  = &servoCard3;
            break;

        default:
            return;
    }

    servo->setServoAngle(bras, 70);
    servo->setServoAngle(pince_top, 90);
    servo->setServoAngle(pince_g,   80); 
    servo->setServoAngle(pince_d,   80); 
    servo->setServoAngle(pince1,    60); 
    servo->setServoAngle(pince2,    60); 
    servo->setServoAngle(pince3,    60);
    servo->setServoAngle(pince4,    60);
    ThisThread::sleep_for(500ms);
    servo->setServoAngle(bras, 170);
    ThisThread::sleep_for(750ms);
    servo->setServoAngle(pince1,    90); 
    servo->setServoAngle(pince2,    90); 
    servo->setServoAngle(pince3,    90);
    servo->setServoAngle(pince4,    90);
    ThisThread::sleep_for(500ms);
    servo->setServoAngle(pince_g,   40); 
    servo->setServoAngle(pince_d,   40); 
    ThisThread::sleep_for(250ms);
    servo->setServoAngle(pince_g,   50); 
    servo->setServoAngle(pince_d,   50); 
    ThisThread::sleep_for(250ms);
    servo->setServoAngle(bras, 180);
    servo->setServoAngle(pince_top, 15);
    ThisThread::sleep_for(500ms);
    servo->setServoAngle(pince_g,   35); 
    servo->setServoAngle(pince_d,   35); 
    ThisThread::sleep_for(500ms);
    servo->setServoAngle(bras, 7);
    ThisThread::sleep_for(500ms);
    servo->setServoAngle(pince_top, 20);
    servo->setServoAngle(pince_g,   50); 
    servo->setServoAngle(pince_d,   50); 
    ThisThread::sleep_for(250ms);
}
void configureCard(ServoPCA9685& servo)
{
    servo.setMirrored(pince1,    false);
    servo.setMirrored(pince2,    true);
    servo.setMirrored(pince3,    false);
    servo.setMirrored(pince4,    true);
    servo.setMirrored(pince_g,   false);
    servo.setMirrored(pince_top, false);
    servo.setMirrored(pince_d,   true);
    servo.setMirrored(bras,      false);

}

void configureCardOffsets(ServoPCA9685& servo,
                          float off0, float off1, float off2, float off3,
                          float off4, float off5, float off6, float off7)
{
    servo.setOffset(pince1,    off0);
    servo.setOffset(pince2,    off1);
    servo.setOffset(pince3,    off2);
    servo.setOffset(pince4,    off3);
    servo.setOffset(pince_g,   off4);
    servo.setOffset(pince_top, off5);
    servo.setOffset(pince_d,   off6);
    servo.setOffset(bras,      off7);
}

void safeStopAll(const char* msg)
{
    Fin_de_match = true;
    end_match = true;
    En_drive_N = 1;
    En_servo_N = 1;
    //asserv.stop();
    lcdStatus(msg);
}

void thread_drive_enable()
{
    while (true)
    {
        if(SW_bau.read() == 1)
        {    safeStopAll("ARRET URGENCE !");
           
        }else{
            En_drive_N = SW_Drive.read();
            En_servo_N = 0;
        }
        ThisThread::sleep_for(200ms);
    }
}

void waitTeamValidation()
{
    while (true)
    {
        Couleur_Team = (SW_team.read() == 1) ? BLUE : YELLOW;

        lcdStatus("Choix equipe",
                  (Couleur_Team == YELLOW) ? "JAUNE" : "BLEU");

        if (SW_init.read() == 0)
        {
            ThisThread::sleep_for(50ms);

            if (SW_init.read() == 0)
            {
                lcdStatus("Equipe validee",
                          (Couleur_Team == YELLOW) ? "JAUNE" : "BLEU");

                printf("Equipe validee : %s\r\n",
                       (Couleur_Team == YELLOW) ? "JAUNE" : "BLEU");

                while (SW_init.read() == 0)
                {
                    ThisThread::sleep_for(20ms);
                }

                ThisThread::sleep_for(500ms);
                return;
            }
        }

        ThisThread::sleep_for(100ms);
    }
}


void main_thread()
{
    FsmState = IDLE;

    lcdStatus("Wait Init",
              Couleur_Team == YELLOW ? "Team Yellow" : "Team Blue");

    while (true)
    {
        switch (FsmState)
        {
        case IDLE:
            if (SW_init != 1)
            {
                lcdStatus("Start Up !");
                FsmState = START_UP;
            }
            break;

        case START_UP: 
            // Strategy generated by StrategyEditor
// Total points: 5

                Robot.setPosition(375, 1775, 180, Couleur_Team);
                Robot.Robotgoto(375, 1775, -30, Couleur_Team, NORMALSPEED);  // Step 1 - POI startYellow
                Robot.Robotgoto(135, 1775, -30, Couleur_Team, NORMALSPEED);  // Step 1 - POI startYellow
                Robot.Robotgoto(135, 1850, -30, Couleur_Team, NORMALSPEED);  // Step 2
                Robot.Robotgoto(115, 1850, -30, Couleur_Team, NORMALSPEED);  // Step 3
                Robot.setPosition(125, 1826, -30, Couleur_Team);
                Robot.Robotgoto(375, 1775, -150, Couleur_Team, NORMALSPEED);  // Step 4 - POI startYellow
                Robot.setPosition(375, 1775, -150, Couleur_Team);
             
// Hacked even more
            FsmState = CAL;
            break;

        case CAL:
            lcdStatus("Wait Match !");
            FsmState = WAIT_MATCH;
            break;

        case WAIT_MATCH:
            if (SW_Tirette == 1)
            {
                endMatch.attach(endMatchProcess, 100s);
                lcdStatus("GAME!");

                lcd_thread.start(print_lcd);

                FsmState = GAME;
            }
            break;

        case GAME:
// Total points: 21
        Robot.Robotgoto(417, 1222, -150, Couleur_Team, NORMALSPEED);  // Step 5
        Robot.Robotgoto(350, 1215, -150, Couleur_Team, NORMALSPEED);  // Step 1
        Prise_Caise(2);
        Robot.Robotgoto(350, 1215, 0, Couleur_Team, NORMALSPEED);  // Step 2
        Robot.Robotgoto(1120, 1050, 0, Couleur_Team, NORMALSPEED);  // Step 3
        Robot.Robotgoto(1120, 900, 0, Couleur_Team, NORMALSPEED);  // Step 4 - POI stockYellow_04
        Prise_Caise(3);
        Robot.Robotgoto(1080, 700, -120, Couleur_Team, NORMALSPEED);  // Step 5
        Robot.Robotgoto(1080, 340, -120, Couleur_Team, NORMALSPEED);  // Step 6
        Prise_Caise(1);
        Robot.Robotgoto(680, 300, -120, Couleur_Team, NORMALSPEED);  // Step 7
        DeposeCaise1(1);
        Robot.Robotgoto(700, 500, -120, Couleur_Team, NORMALSPEED);  // Step 8
        DeposeCaise2(1);
        Robot.Robotgoto(700, 300, -120, Couleur_Team, NORMALSPEED);  // Step 9
        Robot.Robotgoto(320, 830, 90, Couleur_Team, NORMALSPEED);  // Step 10
        DeposeCaise1(3);
        Robot.Robotgoto(550, 830, 90, Couleur_Team, NORMALSPEED);  // Step 11
        DeposeCaise2(3);
        Robot.Robotgoto(370, 830, 90, Couleur_Team, NORMALSPEED);  // Step 12
        Robot.Robotgoto(660, 830, 30, Couleur_Team, NORMALSPEED);  // Step 13
        DeposeCaise1(2);
        Robot.Robotgoto(500, 830, 30, Couleur_Team, NORMALSPEED);  // Step 14
        DeposeCaise2(2);
        Robot.Robotgoto(580, 830, 30, Couleur_Team, NORMALSPEED);  // Step 15
        Robot.Robotgoto(352, 1090, -150, Couleur_Team, NORMALSPEED);  // Step 16
        Robot.Robotgoto(483, 1050, -150, Couleur_Team, NORMALSPEED);  // Step 16
        Robot.Robotgoto(987, 1380, -60, Couleur_Team, NORMALSPEED);  // Step 17
        Robot.Robotgoto(395, 1364, -60, Couleur_Team, NORMALSPEED);  // Step 18
        Robot.Robotgoto(300, 1820, -60, Couleur_Team, NORMALSPEED);  // Step 19 - POI startYellow



            

                FsmState = END;
            break;

        case END:
            break;
        }

        ThisThread::sleep_for(20ms);
    }
}

int main()
{
#ifdef DEBUG
    threadAffichage.start(routineAffichage);
#endif

    drive_thread.start(thread_drive_enable);

    SW_init.mode(PullUp);
    SW_team.mode(PullUp);
    SW_bau.mode(PullUp);
    SW_Tirette.mode(PullUp);
    SW_Drive.mode(PullUp);
    SW_Stepper.mode(PullUp);

    i2c.frequency(100000);

    configureCard(servoCard1);
    configureCard(servoCard2);
    configureCard(servoCard3);

    // offsets indépendants carte 1
    configureCardOffsets(servoCard1,
        0.0f, -5.0f, 0.0f, 0.0f,
        0.0f,  0.0f, 0.0f, 12.0f
    );

    // offsets indépendants carte 2
    configureCardOffsets(servoCard2,
        0.0f, -5.0f, 0.0f, 0.0f,
        0.0f,  0.0f, 0.0f, 0.0f
    );

    // offsets indépendants carte 3
    configureCardOffsets(servoCard3,
        0.0f, -5.0f, 0.0f, 0.0f,
        10.0f, -7.0f,  10.0f, 15.0f
    );

    servoCard1.begin();
    servoCard2.begin();
    servoCard3.begin();
    Home_Servo(servoCard1);
    Home_Servo(servoCard2);
    Home_Servo(servoCard3);
    waitTeamValidation();

    initSensor();

    printf("\r\nSystem ready\r\n");

    lcd.cls();
    game_thread.start(main_thread);

    //asserv.enableBroadcast();
    lidarAnalyzer_thread.start(thread_lidar);
    //broadcast_thread.start(thread_broadcast);

    while (true)
    {
        if (!Fin_de_match)
        {
            if (end_match)
            {
                safeStopAll("END TIMEOUT !");
            }
        }
        ThisThread::sleep_for(1s);
    }
}