#include "LinearActuator.hpp"
//***********************************/************************************
//                         Constructors                                 //
//***********************************/************************************
LinearActuator::LinearActuator(PinName step, PinName dir,PinName SW_up, PinName SW_down, bool reverve): _sw_up(SW_up), _sw_down(SW_down)
{
    StepperAct = new Stepper(step, dir);;
    _sw_up.mode(PullUp);
    _sw_down.mode(PullUp);
    if(reverve == true){
        _reverse =-1;
    }else{
        _reverse =1;
    }
    routine.start(callback(this, &LinearActuator::routine_Actuator));
    _step_move = 0;
    _Cmd = "";
   //InitLinearActuator();
   

}

//***********************************/************************************
//                                Get Set                               //
//***********************************/************************************



//***********************************/************************************
//                             Public Methods                           //
//***********************************/************************************
// bool LinearActuator::InitDir(void)
// {
//     do
//     {  
//         StepperAct->move(100);
//         while(!StepperAct->stopped());
//     } while (_sw_up !=0 and _sw_down !=0);
//     if(_sw_down !=1) reverse =-1;
//     else if (_sw_up !=1) reverse = 1;
//     return true;
// }

bool LinearActuator::goUp(void)
{
    _Cmd = "UP";
    return (_sw_up !=1) ? true : false;
}

bool LinearActuator::goDown(void)
{
    _Cmd = "DOWN";
    return (_sw_down !=1) ? true : false;
}

void LinearActuator::InitLinearActuator(void){
    StepperAct->setSpeed(SPEED_ACT);
    StepperAct->setAcceleration(SPEED_ACT/ACC_ACT);
    StepperAct->setDeceleration(SPEED_ACT/DEC_ACT);
    
}
void LinearActuator::move(int step){
    _step_move = step;
    _Cmd = "MOVE";

}

void LinearActuator::routine_Actuator(void)
{ 
   while (1)
   {
     if (_Cmd == "UP") {
        StepperAct->move(100000*_reverse);
        while (_sw_up !=0);
        StepperAct->stop();
        _Cmd = "";
     }else if (_Cmd == "DOWN")
     {
        StepperAct->move(-100000*_reverse);
        while (_sw_down !=0);
        StepperAct->stop();
        _Cmd = "";
     }else if (_Cmd == "MOVE")
     {
        StepperAct->move(_step_move*_reverse);
        while(!StepperAct->stopped());
        _Cmd = "";
     }
     
     
   }
   
}

