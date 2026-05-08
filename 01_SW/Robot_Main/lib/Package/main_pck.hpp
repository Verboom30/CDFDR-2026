#ifndef __MAIN_PKG_H_
#define __MAIN_PKG_H_

#define RED     "\033[0;31m"
#define CYAN    "\033[0;36m"
#define WHITE   "\033[0;37m"
#define GREEN   "\033[0;32m"
#define YELLOW  "\033[0;33m"

#define R_SENSE 0.11f        // R-Sense in OHM. Match to your driver
#define RMSCURRENT 3000       // RMS current of Stepper Coil in mA
#define MSTEP   16
#define TOFF 5               // Enables driver in software - 3, 5
#define EN_SPREADCYCLE false // Toggle spreadCycle on TMC2208/2209/2224: default false, true: much faster!!!!
#define PWM_AUTOSCALE true   // Needed for stealthChop

#define pince1 0
#define pince2 1
#define pince3 2
#define pince4 3
#define pince_g 4
#define pince_top 5
#define pince_d 6
#define bras 7

enum {IDLE,START_UP,CAL,WAIT_MATCH,GAME,END}FsmState;  
#endif // __MAIN_PKG_H_