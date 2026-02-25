/*
 * File:   mainXC16.c
 * Author: WesleyF, 
 *
 * Created on February 20, 2026, 10:01 AM
 */

//#include s
#include "xc.h"
#pragma config FNOSC = FRCDIV

//State Definitions
enum {Line, Ball, Dump, Canyon, Dock, Laser, Done, Milestone5} state;
enum {Left, Right} line;
enum {Retrieve, Return} ball;
enum {White, Black} ballColor; 
enum {Left1, Right1, Exit} canyon;
enum {Turn, Forward} dock;
enum {Sensor, Further} laser;
enum {Sing, Stop} done;
enum {Forward1, Turn90, Forward2, Turn180, Forward3, Finished} milestone5;

//variable/flag declarations
unsigned char hasBall = 0;
unsigned int leftVel = 0; //corresponds to OC2RS, pin 4. Lower is Faster
unsigned int rightVel = 0; //corresponds to OC1RS, pin 14. Lower is Faster
unsigned char leftDir = 0; //corresponds to A1, pin 3. 0 is forward
unsigned char rightDir = 0; //corresponds to B2, pin 6. 0 is forward
//unsigned char turning = 0;
unsigned int steps = 0;
//unsigned char fullTurn = 0;
unsigned int time = 0;


//Interrupt functions
void __attribute__((interrupt, no_auto_psv)) _T1Interrupt(void){
    _T1IF = 0;
    _T1IE = 0;
    _OC2IE = 1;
    time = 1;
//    if (steps >= 425){
//        if (turning == 1){
//            if (fullTurn = 1){
//                fullTurn = 0;
//            }else{
//                line = Left;
//                turning = 0;
//                _T1IF = 0;
//                fullTurn = 1;
//            }
//        }
//    }else{
//        line = Right;
//        turning = 1;
//        PR1 = 425;
//    }
}

void __attribute__((interrupt, no_auto_psv)) _OC2Interrupt(void){
    _OC2IF = 0;
    steps ++;
}



//function declarations
void sing();
void drive();
void stop();

int main(void){
    //setup
    _RCDIV = 0; //set postscailer to 1
    ANSA = 0; //set to digital logic
    ANSB = 0; //set to digital logic
    _TRISA1 = 0; //pin 3
    _TRISB2 = 0; //pin 6
    OC2CON1 = 0x1C06; //sets clock and edge alignment
    OC2CON2 = 0x001F; //sets to pwm
    OC1CON1 = 0x1C06; //sets clock and edge alignment
    OC1CON2 = 0x001F; //sets to pwm
    T1CONbits.TON = 1;
    T1CONbits.TCS = 0;
    T1CONbits.TCKPS = 0b11; //prescaler of 256
    PR1 = 7813; //period of 1 second
    _T1IP = 6;
    _T1IF = 0;
    _T1IE = 1;
    _OC2IP = 4;
    _OC2IF = 0;
    _OC2IE = 0;
    OC2RS = 2000;
    OC2R = 100;
    OC1RS = 2000;
    OC1R = 100;
    
    //Pin Definitions
    

    state = Milestone5;
    milestone5 = Forward1;
    
    //initial conditions
    leftVel = 2000;
    rightVel = 2000;
    leftDir = 0;
    rightDir = 0;
    drive();

    //main loop
    while (1){
        switch(state){
            case Line:
                switch(line){
                    case Left:
                        leftVel = 2000;
                        rightVel = 0;
                        leftDir = 0;
                        rightDir = 0;
                        drive();
                        break;
                    case Right:
                        leftVel = 0;
                        rightVel = 2000;
                        leftDir = 0;
                        rightDir = 0;
                        drive();
                        break;
                    default:
                        line = Right;
                }
                break;
            case Ball:
                break;
            case Dump:
                break;
            case Canyon:
                break;
            case Dock:
                break;
            case Laser:
                break;
            case Done:
                switch(done){
                    case Sing:
                        sing();
                        break;
                    case Stop:
                        stop();
                        break;
                    default:
                        done = Sing;
                }
                break;
            case Milestone5:
                switch(milestone5){
                    case(Forward1):
                        if(time == 1){
                            time = 0;
                            steps = 0;
                            TMR1 = 0;
                            milestone5 = Turn90;
                            leftVel = 0;
                            rightVel = 2000;
                            leftDir = 0;
                            rightDir = 0;
                            drive();
                        }
                        break;
                    case(Turn90):
                        if(steps >= 550){
                            milestone5 = Forward2;
                            time = 0;
                            steps = 0;
                            TMR1 = 0;
                            _OC2IE = 0;
                            _T1IE = 1;
                            leftVel = 2000;
                            rightVel = 2000;
                            leftDir = 0;
                            rightDir = 0;
                            drive();
                        }
                        break;
                    case(Forward2):
                        if(time>=1){
                            time = 0;
                            steps = 0;
                            TMR1 = 0;
                            milestone5 = Turn180;
                            leftVel = 0;
                            rightVel = 2000;
                            leftDir = 0;
                            rightDir = 0;
                            drive();
                        }
                        break;
                    case(Turn180):
                        if(steps>=1100){
                            time = 0;
                            steps = 0;
                            TMR1 = 0;
                            _OC2IE = 0;
                            _T1IF = 0;
                            _T1IE = 1;
                            milestone5 = Forward3;
                            leftVel = 2000;
                            rightVel = 2000;
                            leftDir = 0;
                            rightDir = 0;
                            drive();
                        }
                        break;
                    case(Forward3):
                        if(time==1){
                            milestone5 = Finished;
                            time = 0;
                            steps = 0;
                            leftVel = 0;
                            rightVel = 0;
                            leftDir = 0;
                            rightDir = 0;
                            time = 0;
                            steps = 0;
                            _T1IE = 0;
                            _OC2IE = 0;
                            stop();
                        }
                        break;
                    case(Finished):
                        stop();
                        break;
                    default:
                        milestone5 = Turn90;
                }
                break;
            default:
                state = Line;  
        }
    }
    return 0;
}

void sing(){
    leftVel = 100;
    rightVel = 100;
    leftDir ^= 1;
    rightDir = 1 ^ leftDir;
    drive();
    return;
}

void drive(){
    _LATB2 = leftDir;
    _LATA1 = rightDir;
    OC1RS = leftVel;
    OC2RS = rightVel;
    OC2R = 50;
    OC1R = 50;
}

void stop(){
    OC2R = leftVel;
    OC1R = rightVel;
}
                