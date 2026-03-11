/*
 * File:   mainXC16.c
 * Author: WesleyF, 
 *
 * Created on February 20, 2026, 10:01 AM
 */

//#include s
#include "xc.h"
#pragma config FNOSC = FRCDIV //sets to 8MHz clock

//State Definitions
enum {Line, Ball, Dump, Canyon, Dock, Laser, Done, Milestone5} state;
enum {Left, Right, SwitchL, SwitchR, StopLine} line;
enum {LeftL, RightL, Hidden} linepos;
enum {Retrieve, Return} ball;
enum {White, Black} ballColor; 
enum {Left1, Right1, Exit} canyon;
enum {Turn, Forward} dock;
enum {Sensor, Further} laser;
enum {Sing, Stop} done;
enum {Forward1, Turn90, Forward2, Turn180, Forward3, Finished} milestone5;


//variable/flag declarations
unsigned char hasBall = 0;
unsigned int leftVel = 0; //corresponds to OC2RS, pin 4. Lower is Faster. Min is 65535, max is 1000
unsigned int rightVel = 0; //corresponds to OC3RS, pin 5. Lower is Faster. Min is 65535, max is 1000
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
}

void __attribute__((interrupt, no_auto_psv)) _OC2Interrupt(void){
    _OC2IF = 0;
    steps ++;
}

void config_ADC(void)
{
    _ADON = 0;    // Disable A/D module during configuration
    
    // AD1CON1
    _MODE12 = 1;  // 12-bit resolution
    _FORM = 0;    // unsigned integer output
    _SSRC = 7;    // auto convert
    _ASAM = 1;    // auto sample

    // AD1CON2
    _PVCFG = 0;   // use VDD as positive reference
    _NVCFG = 0;   // use VSS as negative reference
    _BUFREGEN = 1;// store results in buffer corresponding to channel number
    _CSCNA = 1;   // scanning mode
    _SMPI = 7;    // begin new sampling sequence after every sample
    _ALTS = 0;    // sample MUXA only

    // AD1CON3
    _ADRC = 0;    // use system clock
    _SAMC = 0;    // sample every A/D period
    _ADCS = 0x3F; // TAD = 64*TCY

    // AD1CSS -- Choose which channel/pin to scan
    _CSS0 = 1;
    _CSS1 = 1;
    _CSS4 = 1;
    _CSS13 = 1;
    _CSS14 = 1;
    _CSS10 = 1;
    _CSS11 = 1;
    _CSS12 = 1;


    _ADON = 1;    // enable module
}



//function declarations
void sing();
void drive();
void stop();
void wait();

int main(void){
    //setup
    // setting default states
    state = Line;
    line = StopLine;
    linepos = Hidden;
    ball = Retrieve;
    ballColor = Black;
    canyon = Left1;
    dock = Turn;
    laser = Sensor;
    done = Stop;
    milestone5 = Forward1;
    _RCDIV = 0b0; //set post scaler to 1

    _TRISA0 = 1; //pin 2, photodiode laser
    _TRISA1 = 1; //pin 3, photodiode ball
    _TRISB0 = 0; //pin 4, left motor control
    _TRISB1 = 0; //pin 5, right motor control
    _TRISB2 = 1; //pin 6, right distance
    _TRISA2 = 1; //pin 7, front distance
    _TRISA3 = 1; //pin 8, left distance

    _TRISB7 = 0; //pin 11, laser enable
    _TRISB8 = 0; //pin 12, right motor direction
    _TRISB9 = 0; //pin 13, left motor direction
    _TRISA6 = 0; //pin 14, servos control
    _TRISB12 = 1; //pin 15, QRD Left
    _TRISB13 = 1; //pin 16, QRD Ball
    _TRISB14 = 1; //pin 17, QRD Right
    _TRISB15 = 1; //pin 18, servo enable

    OC1CON1 = 0x1C06; //sets clock and edge alignment
    OC1CON2 = 0x001F; //sets to pwm
    OC2CON1 = 0x1C06; //sets clock and edge alignment
    OC2CON2 = 0x001F; //sets to pwm
    OC3CON1 = 0x1C06; //sets clock and edge alignment
    OC3CON2 = 0x001F; //sets to pwm

    config_ADC();


    T1CONbits.TON = 1; 
    T1CONbits.TCS = 0;
    T1CONbits.TCKPS = 0b10; //prescaler of 64
    PR1 = 31250; //period of 1 second

    _T1IP = 6;
    _T1IF = 0;
    _T1IE = 0;
    _OC2IP = 4;
    _OC2IF = 0;
    _OC2IE = 1;
    OC2RS = 2000;
    OC2R = 100;
    OC3RS = 2000;
    OC3R = 100;
    
    //Pin Definitions
    
    
    //initial conditions
    leftVel = 16000;
    rightVel = 16000;
    leftDir = 0;
    rightDir = 0;
    drive();

    //main loop
    while (1){
        switch(state){
            case Line:
                //detecting the line (or nothing)
                if(ADC1BUF12 <= 750){
                    linepos = LeftL;
                }else if(ADC1BUF12 >= 1000){
                    linepos = RightL;
                }else{
                    linepos = Hidden;
                }

                //setting the motor directions and velocities
                switch(line){
                    case Left:
                        switch (linepos){
                            case LeftL:
                                break;
                            case RightL:
                                line = SwitchR;
                                break;
                            case Hidden:
                                line = StopLine;
                                break;
                            default:
                                linepos = Hidden;
                        }
                        leftVel = 32000;
                        rightVel = 8000;
                        leftDir = 0;
                        rightDir = 0;
                        drive();
                        break;
                    case Right:
                        switch (linepos){
                            case LeftL:
                                line = SwitchL;
                                break;
                            case RightL:
                                break;
                            case Hidden:
                                line = StopLine;
                                break;
                            default:
                                linepos = Hidden;
                        }
                        leftVel = 8000;
                        rightVel = 32000;
                        leftDir = 0;
                        rightDir = 0;
                        drive();
                        break;
                    case SwitchL:
                        wait();
                        line = Left;
                        break;
                    case SwitchR:
                        wait();
                        line = Right;
                        break;
                    case StopLine:
                        stop();
                        switch (linepos){
                            case LeftL:
                                line = SwitchL;
                                break;
                            case RightL:
                                line = SwitchR;
                                break;
                            case Hidden:
                                break;
                            default:
                                linepos = Hidden;
                        }
                        break;
                    default:
                    line = StopLine;

                }
                
                if(steps >= 8000){
                    state = Done;
                    done = Stop;
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
    _LATB9 = leftDir;
    _LATB8 = rightDir;
    OC3RS = leftVel;
    OC2RS = rightVel;
    OC2R = 50;
    OC3R = 50;
}

void stop(){
    OC2R = leftVel;
    OC3R = rightVel;
    OC2RS = leftVel;
    OC3RS = rightVel;
}

void wait(){
    stop();
    _OC2IE = 0;
    _T1IF = 0;
    _T1IE = 1;
    time = 0;
    steps = 0;
    TMR1 = 0;
    while (0){
    }
}