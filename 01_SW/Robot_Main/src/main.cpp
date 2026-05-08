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

//#define DEBUG
//#define LIDAR

Stepper stepA(STEP_A, DIR_A);
Stepper stepB(STEP_B, DIR_B);
Stepper stepC(STEP_C, DIR_C);

DigitalOut En_drive_N(EN_DRIVE_N);

bool stopLidar = false;
bool Fin_de_match = false;
bool Couleur_Team = 0; // false bleu, true jaune

Holonome robot(&stepA, &stepB, &stepC, &stopLidar);

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
LidarAnalyzer LidaRayzer(LidarLD19, &robot, &led_lidar);

LCD lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7, LCD16x2);

volatile bool end_match = false;

Timeout endMatch;

Thread lcd_thread;
Thread game_thread;
Thread lidarAnalyzer_thread;

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

void printPosition()
{
    printf("%f;%f;%f\r\n",
           robot.getPositionX(),
           robot.getPositionY(),
           robot.getTheta());
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

void thread_lidar()
{
    while (true)
    {
        LidaRayzer.update();

#ifdef LIDAR
        stopLidar = LidaRayzer.isObstacleDetected();
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

void Home_Servo()
{
    servoCard2.setServoAngle(pince_top, 35);

    servoCard2.setServoAngle(pince_g, 50);
    servoCard2.setServoAngle(pince_d, 50);

    servoCard2.setServoAngle(pince1, 90);
    servoCard2.setServoAngle(pince2, 90);
    servoCard2.setServoAngle(pince3, 90);
    servoCard2.setServoAngle(pince4, 90);

    ThisThread::sleep_for(500ms);

    servoCard2.setServoAngle(bras, 7);
}

void ColorDetectTask(uint8_t arm_id)
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
        if (colorResults[i].color == TCS34007Mux::COLOR_YELLOW)
        {
            servo->setServoAngle(i, 50);
        }
        else
        {
            servo->setServoAngle(i, 100);
        }
    }
}
void ColorDetectTask2(uint8_t arm_id){

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

    servo->setServoAngle(pince_top, 15);
    ThisThread::sleep_for(250ms);
    servo->setServoAngle(bras, 130);
    ThisThread::sleep_for(1000ms);
    Home_Servo();
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

void safeStopAll(const char* msg)
{
    Fin_de_match = true;
    robot.stop();
    En_drive_N = 1;
    lcdStatus(msg);
}
void CheckBauFalse()
{
    while (true)
    {
        if(SW_bau.read() == 0){
            return;
        }else{
            lcdStatus("Attention !", "BAU");
        }
        ThisThread::sleep_for(500ms);
    }
}


void waitTeamValidation()
{
    while (true)
    {
        Couleur_Team = !SW_team.read();

        lcdStatus("Choix equipe",
                  Couleur_Team ? "JAUNE" : "BLEU");

        if (SW_init.read() == 0)
        {
            ThisThread::sleep_for(50ms);

            if (SW_init.read() == 0)
            {
                lcdStatus("Equipe validee",
                          Couleur_Team ? "JAUNE" : "BLEU");

                printf("Equipe validee : %s\r\n",
                       Couleur_Team ? "JAUNE" : "BLEU");

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


void Prise_Caise()
{
    servoCard2.setServoAngle(bras, 70);


    servoCard2.setServoAngle(pince_top, 90);
    servoCard2.setServoAngle(pince_g,   80); 
    servoCard2.setServoAngle(pince_d,   80); 
    servoCard2.setServoAngle(pince1,    60); 
    servoCard2.setServoAngle(pince2,    60); 
    servoCard2.setServoAngle(pince3,    60);
    servoCard2.setServoAngle(pince4,    60);
    ThisThread::sleep_for(500ms);
    servoCard2.setServoAngle(bras, 130);
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
    servoCard2.setServoAngle(bras, 140);
    servoCard2.setServoAngle(pince_top, 20);
    ThisThread::sleep_for(250ms);
    servoCard2.setServoAngle(pince_g,   35); 
    servoCard2.setServoAngle(pince_d,   35); 
    ThisThread::sleep_for(250ms);
    servoCard2.setServoAngle(bras, 7);
    ThisThread::sleep_for(1000ms);
    servoCard2.setServoAngle(pince_g,   50); 
    servoCard2.setServoAngle(pince_d,   50); 
    ThisThread::sleep_for(250ms);
}

void main_thread()
{
    FsmState = IDLE;

    lcdStatus("Wait Init",
              Couleur_Team ? "Team Yellow" : "Team Blue");

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
                Prise_Caise();
                ThisThread::sleep_for(1000ms);
                ColorDetectTask(2);
                ThisThread::sleep_for(1000ms);
                ColorDetectTask2(2);
                ThisThread::sleep_for(1000ms);
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

    En_drive_N = 1;

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

    servoCard1.begin();
    servoCard2.begin();
    servoCard3.begin();
    CheckBauFalse();
    Home_Servo();
    waitTeamValidation();

    initSensor();

    printf("\r\nSystem ready\r\n");

    lcd.cls();
    game_thread.start(main_thread);

    // lidarAnalyzer_thread.start(thread_lidar);

    while (true)
    {
        if (!Fin_de_match)
        {
            En_drive_N = SW_Drive.read();

            if (end_match)
            {
                safeStopAll("END TIMEOUT !");
            }
            else if (SW_bau.read() == 1)
            {
                safeStopAll("ARRET URGENCE !");
            }
        }

        ThisThread::sleep_for(20ms);
    }
}