/*
 * File:   mainXC16.c
 * Authors: WesleyF, bwhit05, cewell7, jdag2002
 *
 * Created on February 20, 2026, 10:01 AM
 */

//#include s
#include "xc.h"
#pragma config FNOSC = FRCDIV //sets to 8MHz clock
#pragma config OSCIOFNC = OFF //disables clock output on pin 8

//State Definitions
enum state {Testing, Line, Ball, Dump, Canyon, Dock, Laser, Done, Milestone5} state;
enum line {Left, Right, SwitchL, SwitchR, StopLine} line;
enum linePos {LeftL, RightL, Hidden} linePos;
enum lineVel {Ballanced, Turning} lineVel;
enum ball {Retrieve, Return} ball;
enum ballColor {White, Black} ballColor; 
enum canyon {Front, Front0, Exit} canyon;
enum canyonPos {For, Lef, Rig, Exi} canyonPos;
enum dock {Turn, Forward} dock;
enum laser {Sensor, Further} laser;
enum done {Sing, Stop} done;
enum milestone5 {Forward1, Turn90, Forward2, Turn180, Forward3, Finished} milestone5;

//variable/flag declarations
unsigned char hasBall = 1;
unsigned int leftVel = 0; //corresponds to OC2RS, pin 4. Lower is Faster. Min is 65535, max is 1000
unsigned int rightVel = 0; //corresponds to OC3RS, pin 5. Lower is Faster. Min is 65535, max is 1000
unsigned char leftDir = 0; //corresponds to A1, pin 3. 0 is forward
unsigned char rightDir = 0; //corresponds to B2, pin 6. 0 is forward
//unsigned char turning = 0;
unsigned int stepsL = 0; //steps by the left motor driver
unsigned int stepsR = 0; //steps by the right motor driver
//unsigned char fullTurn = 0;
unsigned int time = 0; //clock cycles
unsigned char canyon_done = 0;
//Interrupt functions
void __attribute__((interrupt, no_auto_psv)) _T1Interrupt(void){
    _T1IF = 0;
    time += 1;
}

//Left motor interrupt
void __attribute__((interrupt, no_auto_psv)) _OC2Interrupt(void){
    _OC2IF = 0;
    stepsL ++;
}

//Right motor interrupt
void __attribute__((interrupt, no_auto_psv)) _OC3Interrupt(void){
    _OC3IF = 0;
    stepsR ++;
}

// Configure ADC
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
void wait(int ticks);
void readline();
void turn90r();
void turn90l();
void turn45r();
void turn45l();
void turn180();
void forward(int steps);
void back(int steps);
void left();
void right();
void detectWalls();
void dump();

int main(void){
    //setup
    _RCDIV = 0b0; //set post scaler to 1
    
    {// setting default states
    state = Line;
    line = StopLine;
    linePos = Hidden;
    lineVel = Ballanced;
    ball = Retrieve;
    ballColor = Black;
    canyon = Front0;
    dock = Turn;
    laser = Sensor;
    done = Stop;
    milestone5 = Forward1;
    }
    
    {//pin declarations
    _TRISA0 = 1; //pin 2, photodiode laser (AN0)
    _TRISA1 = 1; //pin 3, photodiode ball (AN1)
    _TRISB0 = 0; //pin 4, left motor control (OC2)
    _TRISB1 = 0; //pin 5, right motor control (OC3)
    _TRISB2 = 1; //pin 6, right distance (AN4)
    _TRISA2 = 1; //pin 7, front distance (AN13)
    _TRISA3 = 1; //pin 8, left distance (AN14)
    //pin 9 is for programming 
    //pin 10 is for programming
    _TRISB7 = 0; //pin 11, laser enable (B7)
    _TRISB8 = 0; //pin 12, right motor direction (B8)
    _TRISB9 = 0; //pin 13, left motor direction (B9)
    _TRISA6 = 0; //pin 14, servos control (OC1)
    _TRISB12 = 1; //pin 15, QRD Left (AN12)
    _TRISB13 = 1; //pin 16, QRD Ball (AN11)
    _TRISB14 = 1; //pin 17, QRD Right (AN10/B14)
    _TRISB15 = 1; //pin 18, servo enable (B15)
    //pin 19 is ground
    //pin 20 is V+
    }

    {//PWM setup
    //pin 14, servos
    OC1CON1 = 0x0C06; //sets OCTSEL to Timer 5 (011) and OCM to PWM (110)
    OC1CON2 = 0x001F; //sets to pwm
    OC1RS = 1250; //sets duty cycle
    OC1R = 80; //sets clock start, between 32 and 156
    //pin 4, left motor
    OC2CON1 = 0x1C06; //sets clock and edge alignment
    OC2CON2 = 0x001F; //sets to pwm
    //pin 5, right motor
    OC3CON1 = 0x1C06; //sets clock and edge alignment
    OC3CON2 = 0x001F; //sets to pwm
    }

    //set up analog input pins
    config_ADC();

    {//timers setup
    T1CONbits.TON = 1; //Timer 1 on
    T1CONbits.TCS = 0; //internal clock
    T1CONbits.TCKPS = 0b10; //prescaler of 64
    PR1 = 3124; //period of 0.05 seconds (20 Hz)

    T2CONbits.TON = 1; //Timer 2 on
    T2CONbits.TCS = 0; //internal clock
    T2CONbits.TCKPS = 0b10; //prescaler of 64
    PR2 = 3124; //period of 0.05 seconds (20 Hz)

    T5CONbits.TON = 1; //Timer 3 on
    T5CONbits.TCS = 0; //internal clock
    T5CONbits.TCKPS = 0b10; //prescaler of 64
    PR5 = 1249; //period of 0.02 seconds (50 Hz)
    }

    {//interrupts setup
    _T1IP = 6; //timer 1 priority
    _T1IF = 0; //timer 1 flag
    _T1IE = 0; //timer 1 interrupt enable (disabled)
    _OC2IP = 4; //OC2 priority
    _OC2IF = 0; //OC2 flag
    _OC2IE = 1; //OC2 interrupt enable (enabled)
    _OC3IP = 4; //OC2 priority
    _OC3IF = 0; //OC2 flag
    _OC3IE = 0; //OC2 interrupt enable (enabled)
    }
    
    {//initial conditions
    leftVel = 16000;
    rightVel = 16000;
    leftDir = 0;
    rightDir = 0;
    stop();
    wait(20);//get rid of
    }

    while (1){//main loop
        switch(state){
            case Testing:{
                wait(100);
                dump();
                break;
            }
            case Line:{
                //detecting the line (or nothing)
                readline();
             
                //setting the motor directions and velocities
                switch(line){
                    case Left:{
                        left();
                        break;
                    }
                    case Right:{
                        right();
                        break;
                    }
                    case SwitchL:{
                        stop();
                        switch (linePos){
                            case LeftL:{
                                line = Left;
                                break;
                            }
                            case RightL:{
                                line = SwitchR;
                                break;
                            }
                            case Hidden:{
                                line = StopLine;
                                break;
                            }
                            default:{
                                linePos = Hidden;
                            }
                        }
                        break;
                    }
                    case SwitchR:{
                        stop();
                        switch (linePos){
                            case LeftL:{
                                line = SwitchR;
                                break;
                            }
                            case RightL:{
                                line = Right;
                                break;
                            }
                            case Hidden:{
                                line = StopLine;
                                break;
                            }
                            default:{
                                linePos = Hidden;
                            }
                        }
                        break;
                    }
                    case StopLine:{
                        stop();
                        switch (linePos){
                            case LeftL:{
                                line = SwitchL;
                                break;
                            }
                            case RightL:{
                                line = SwitchR;
                                break;
                            }
                            case Hidden:{
                                break;
                            }
                            default:{
                                linePos = Hidden;
                            }
                        }
                        break;
                    }
                    default:{
                        line = StopLine;
                    }
                }
                
                // if(stepsL >= 900){//Turning in to base
                //     state = Done;
                //     done = Stop;
                //     break;
                // }

                 if(ADC1BUF4 <= 1000){ // right sees wall, left sees wall, QRD sees no line
                     wait(20);
                     if(ADC1BUF14 <= 1000){
                        state = Canyon;
                     }
                     if(hasBall == 0){
                        state = Ball;
                     }
                     else{
                        state = Dump;
                     }
                 }
                
                break;
            }
            case Ball:{
                break;
            }
            case Dump:{
                if(ADC1BUF11 <= 2000){
                    forward(100);
                    wait(5);
                    turn90l();
                    wait(5);
                    forward(200);
                    dump();
                    back(200);
                    wait(5);
                    turn90r();
                    wait(100);
                }
                else{
                    forward(250);
                    wait(5);
                    turn90r();
                    wait(5);
                    forward(200);
                    dump();
                    back(200);
                    wait(5);
                    turn90l();
                    wait(100);
                }
                break;
            }
            case Canyon:{
                switch(canyon){
                    case Front:{
                        wait(20);
                        if(ADC1BUF13 >= 900){
                            canyon = Front0;
                            rightDir = 0;
                            leftDir = 0;
                        }else if(ADC1BUF4 <= 1500){
                            turn90l();
                        }else{
                            turn90r();
                        }
                        if(ADC1BUF12 <= 1000){
                            canyon = Exit;
                        }
                        break;
                    }
                    case Front0:{
                        leftVel = 8000;
                        rightVel = 8000;
                        if(ADC1BUF14 <= 700){
                            rightVel = 10000;
                        }
                        if(ADC1BUF4 <= 700){
                            leftVel = 10000;
                        }
                        if(ADC1BUF13 <= 900){
                            canyon = Front;
                            stop();
                        }else if(ADC1BUF12 <= 1000){
                            canyon = Exit;
                        }
                        else{
                            drive();
                        }
                        break;
                    }
                    case Exit:{
                        wait(20);
                        if(ADC1BUF14 <= 1400){
                            forward(300);
                            turn90r();
                            wait(40);
                        }else{
                            forward(100);
                            turn90l();
                            wait(40);
                        }
                        canyon_done = 0;
                        state = Line;
                        line = Left;
                        lineVel = Ballanced;
                        stepsL = 0;
                        break;
                    }
                    default:{
                        canyon = Front0;
                    }
                }
                break;
            }
            case Dock:{
                break;
            }
            case Laser:{
                break;
            }
            case Done:{
                switch(done){
                    case Sing:{
                        _OC2IE = 0;
                        _T1IE = 0;
                        sing();
                        break;
                    }
                    case Stop:{
                        _OC2IE = 0;
                        _T1IE = 0;
                        stop();
                        while(1){}
                        break;
                    }
                    default:{
                        done = Sing;
                    }
                }
                break;
            }
            case Milestone5:{
                switch(milestone5){
                    case(Forward1):{
                        if(time >= 1){
                            time = 0;
                            stepsL = 0;
                            TMR2 = 0;
                            _OC2IE = 1;
                            _T1IE = 0;
                            milestone5 = Turn90;
                            turn90l();
                        }
                        break;
                    }
                    case(Turn90):{
                        if(stepsL >= 550){
                            milestone5 = Forward2;
                            time = 0;
                            stepsL = 0;
                            TMR2 = 0;
                            _OC2IE = 0;
                            _T1IE = 1;
                            leftVel = 2000;
                            rightVel = 2000;
                            leftDir = 0;
                            rightDir = 0;
                            drive();
                        }
                        break;
                    }
                    case(Forward2):{
                        if(time>=1){
                            time = 0;
                            stepsL = 0;
                            TMR2 = 0;
                            _OC2IE = 1;
                            _T1IE = 0;
                            milestone5 = Turn180;
                            leftVel = 0;
                            rightVel = 2000;
                            leftDir = 0;
                            rightDir = 0;
                            drive();
                        }
                        break;
                    }
                    case(Turn180):{
                        if(stepsL >= 1100){
                            time = 0;
                            stepsL = 0;
                            TMR2 = 0;
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
                    }
                    case(Forward3):{
                        if(time>=1){
                            milestone5 = Finished;
                            time = 0;
                            stepsL = 0;
                            leftVel = 0;
                            rightVel = 0;
                            leftDir = 0;
                            rightDir = 0;
                            _T1IE = 0;
                            _OC2IE = 0;
                            stop();
                        }
                        break;
                    }
                    case(Finished):{
                        stop();
                        break;
                    }
                    default:{
                        milestone5 = Turn90;
                    }
                }
                break;
            }
            default:{
                state = Line;
            }
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
    //set directions
    _LATB9 = leftDir;
    _LATB8 = rightDir;

    //set OCxR
    OC2R = 50;
    OC3R = 50;

    //set PWM
    while(OC2RS != leftVel || OC3RS != rightVel){
        if(OC2RS < leftVel){
            OC2RS += 1000;
        } else if(OC2RS > leftVel){
            OC2RS -= 1000;
        }
        if(OC3RS < rightVel){
            OC3RS += 1000;
        } else if (OC3RS > rightVel){
            OC3RS -= 1000;
        }
        if(OC2RS - leftVel < 1000){
            OC2RS = leftVel;
        }
        if(OC3RS - rightVel < 1000){
            OC3RS = rightVel;
        }
    }
}

void stop(){
    OC2R = 65500;
    OC3R = 65500;
    OC2RS = 65500;
    OC3RS = 65500;
}

void wait(int ticks){
    stop();
    _OC2IE = 0;
    _T1IF = 0;
    time = 0;
    _T1IE = 1;
    TMR2 = 0;
    while (time < ticks){
    }
    _OC2IE = 1;
    _T1IE = 0;
}

void readline(){
    if(ADC1BUF12 <= 2000){
        linePos = LeftL;
    }else if(ADC1BUF12 >= 4000){
        linePos = Hidden;
    }else{
        linePos = RightL;
    }
}

void turn90r(){
    stop();
    _T1IE = 0;
    _OC2IF = 0;
    _OC2IE = 1;
    leftVel = 6000;
    rightVel = 6000;
    leftDir = 0;
    rightDir = 1;
    stepsL = 0;
    drive();
    while(stepsL < 282){}
    stop();
}

void turn90l(){
    stop();
    _T1IE = 0;
    _OC2IF = 0;
    _OC2IE = 1;
    leftVel = 6000;
    rightVel = 6000;
    leftDir = 1;
    rightDir = 0;
    stepsL = 0;
    drive();
    while(stepsL < 282){} //should be 259... testing 300
    stop();
}

void turn45r(){
    stop();
    _T1IE = 0;
    _OC2IF = 0;
    _OC2IE = 1;
    leftVel = 6000;
    rightVel = 6000;
    leftDir = 0;
    rightDir = 1;
    stepsL = 0;
    drive();
    while(stepsL < 131){}
    stop();
}

void turn45l(){
    stop();
    _T1IE = 0;
    _OC2IF = 0;
    _OC2IE = 1;
    leftVel = 6000;
    rightVel = 6000;
    leftDir = 1;
    rightDir = 0;
    stepsL = 0;
    drive();
    while(stepsL < 131){}
    stop();
}
void turn15l(){
    stop();
    _T1IE = 0;
    _OC2IF = 0;
    _OC2IE = 1;
    leftVel = 12000;
    rightVel = 6000;
    leftDir = 0;
    rightDir = 0;
    stepsL = 0;
    drive();
    while(stepsL < 50){}
    stop();
}

void turn180(){
    stop();
    _T1IE = 0;
    _OC2IF = 0;
    _OC2IE = 1;
    leftVel = 6000;
    rightVel = 6000;
    leftDir = 1;
    rightDir = 0;
    stepsL = 0;
    drive();
    while(stepsL < 535){}
    stop();
}

void forward(int steps){
    stop();
    _T1IE = 0;
    _OC2IF = 0;
    _OC2IE = 1;
    leftVel = 4000;
    rightVel = 4000;
    leftDir = 0;
    rightDir = 0;
    stepsL = 0;
    drive();
    while(stepsL < steps){}
    stop();
}

void back(int steps){
    stop();
    _T1IE = 0;
    _OC2IF = 0;
    _OC2IE = 1;
    leftVel = 4000;
    rightVel = 4000;
    leftDir = 1;
    rightDir = 1;
    stepsL = 0;
    drive();
    while(stepsL < steps){}
    stop();
}


void left(){
    switch (linePos){
        case LeftL:{
            switch (lineVel){
                case Ballanced:{
                    leftVel = 12000;
                    rightVel = 4000;
                    leftDir = 0;
                    rightDir = 0;
                    break;
                }
                case Turning:{
                    leftVel = 8000;
                    rightVel = 16000;
                    leftDir = 1;
                    rightDir = 0;
                    break;
                }
                default:{
                    lineVel = Ballanced;
                }
            }
            drive();
            break;
        }
        case RightL:{
            line = SwitchR;
            stop();
            break;
        }
        case Hidden:{
            line = StopLine;
            stop();
            break;
        }
        default:{
            linePos = Hidden;
        }
    }
}

void right(){
    switch (linePos){
        case LeftL:{
            line = SwitchL;
            stop();
            break;
        }
        case RightL:{
            switch (lineVel){
                case Ballanced:{
                    leftVel = 4000;
                    rightVel = 12000;
                    leftDir = 0;
                    rightDir = 0;
                    break;
                }
                case Turning:{
                    leftVel = 8000;
                    rightVel = 16000;
                    leftDir = 0;
                    rightDir = 0;
                    break;
                }
                default:{
                    lineVel = Ballanced;
                }
            }
            drive();
            break;
        }
        case Hidden:{
            line = StopLine;
            stop();
            break;
        }
        default:{
            linePos = Hidden;
        }
    }
}

void detectWalls(){
    if (ADC1BUF12 <= 2000){
        canyonPos = Exi;
    } else if (ADC1BUF13 >= 4000){
        canyonPos = For;
    } else if (ADC1BUF14 >= 4000){
        canyonPos = Lef;
    } else {
        canyonPos = Rig;
    } 
}

void dump(){
    OC1R = 40;
    wait(30);
    OC1R = 80;
}