

//Main tab / StellarisWheeStat5_3 sketch
// pins setup for spi module 0
// simultaneous pwm outputs on pins 2.1, 2.4 and 1.6 
//  
//  Oct 31, 2013; at 3.3 volt input, amp will not drive counter electrode,
//                moving amp intput to +5 volts gives wider useful vRead range

#include<Energia.h>           //need this for SPI to work
#include "wiring_analog.c"    // need this for pwm at high frequency, resolution
#include <altSPI.h>           // alternative SPI library must be in "libraries" folder in sketchbook

/////////////////////////
unsigned int level;              //gain value for digital potentiostat  
int vcc =3290;                   // full scale voltage in mV

int mVinit;                     // initial potential in mV +2000 to replace Vinit
int dInit;                      //  pwm difference variable based on mVinit 
int dInt;
int iGain;                      // current gain, 1 to 255, sets digital pot
int mVfnl;                      // final voltage in mV + 2000
int dFnl;                       //  pwm difference variable based on mV final
int nSteps;                     // number of steps in scan
int dSignal;                    // pwm input to signal pin
int dSig;                       // pwm input to signal pin
int dRef;                       // pwm input to ref pin
int delay1 = 1;                 // plating delay time (seconds)
int openCirc;                   //digital value for open circuit potential
boolean oCircRun = false;
int mVi;
int pwmClock = 40000; 
int mVres = 10;    // Voltage scale resolution in mV
// mVres not currently an output of the GUI, must change in Energia
int pwmRes;  // = vcc/mVres;
int halfRes;  // = pwmRes/2;

int pwm_step = 1;           // pwm increment for step--should probably be decreased.  JSS 9/10/13
unsigned int initial_ms = delay1*1000;   // convert initial delay ms to sec
// initial delay is pre-concentration time from serial inputint scanRate = 100;              // entered in mV/s
//int address = 19;                // for mcp42xxx digital pot, write to both potentiometer registers
int RAMP = 0; 
int CV = 1;           // run mode for cyclic voltammetry
int ASV = 2;          // run mode for anodic strippinng voltammetery
int logASV = 3;       // run mode for data logging ASV experiment
int diff_Pulse = 4;  
int mode;

int gain0;
int gain1;
int scanRate;
float stepTime;                  // = step_mV*1000/scanRate;       // mseconds per step
//unsigned int offSet;             //Offset in mV
int dOff;                        // digital value of offset         
int pgm = 0;               //stripping_volt tab
int runs = 1;                 // number of runs for data logging
unsigned int runDelay;                // min between runs for data logging
//unsigned int duty_cyc;         // duty cycle variable (0-1024) 
//unsigned int ref_cyc;         // duty cycle for reference input -inserted 9/10/13
//long initial_ms = 1000;           // initial delay time (mseconds)
#define signal_pin  PB_4            // Originaly on PF_1 (LED1), move to PB_4, J1 pin 7
#define ref_pin  PB_6            // Originaly PF_2 (LED2), plan move to PB_6, J2 pin 14 
#define offset_pin PB_0            // Originaly PF_3 (LED3), plan move to PB_0, J1 pin 3;   
#define pulse_pin  PA_6          // PA_6; J1, Pin 9  only digital read and write. 
#define Iread_pin  A11              //J1 pin 2, Analog read current, was A3  
#define Vread_pin  A8            //J1, pin 6, Analog read, was A4    
//spi clock on pin 1.5, mosi on pin 1.7

unsigned int inVolt = 0;                 // voltage read
int mVread;                            // can go to negative values
unsigned int Iread1 = 0;                 // hight pulse current read 
unsigned int Iread2 = 0;                 // low pulse curent read 
unsigned int vRead = 0;                 // voltage read
unsigned int iRead = 0;                 // high pulse current read 
unsigned int param =0;

/**********************************************/
void setup() {
  pinMode (signal_pin,OUTPUT);      // set pin 2.1 for output   
  pinMode (ref_pin,OUTPUT);         // new 9/10/13  
  pinMode (offset_pin,OUTPUT);     // new 9/10/13
  pinMode (pulse_pin,OUTPUT);      // set pulse pin as output
  pinMode (Vread_pin,INPUT);
  pinMode (Iread_pin,INPUT);

  pwmRes = vcc/mVres;        // number of data points in full voltage scale
  halfRes = pwmRes/2;        // half scale for setting reference voltage, offset
  PWMWrite(offset_pin,330,165,pwmClock);  // sets current offset to VCC/2
  PWMWrite(ref_pin,330,165,pwmClock);     // set reference voltage to VCC/2

  //  analogResolution(1023);         // divide the full duty cycle into 1024 steps
  //analogFrequency(5000);          // set the full duty cycle to 0.2 ms
  Serial.begin(9600);             // begin serial comm. at 9600 baud

}

void loop (){                          // stripping voltammetry
//  if (run == 0) {                // run this part of loop to setup run
  setupRun();
  nSteps = abs((dFnl - dInit)/pwm_step);   //number of steps is positive value
  setupDigiPot();                // two lines moved up


  // run = 1;  }  // end of if run == 0 loop

  // move this loop to Open_Circuit tab?
  if (oCircRun == false) {             // use existing open circuit value?
    openCirc = (dInit + dFnl)/2;
  }
  PWMWrite(offset_pin,pwmRes,dOff,pwmClock);  // sets current offset
  if (mode == RAMP || mode == CV) {
    ramp();  
    Serial.print(99999);  
    Serial.print('\t');
    Serial.println(99999);
  }
  if (mode == diff_Pulse) {
    diffPulse();
    Serial.print(99999);  
    Serial.print('\t');
    Serial.print(99999);
    Serial.print('\t');
    Serial.println(00000);
  }
  if (mode == ASV || mode == logASV) {
    digitalWrite(pulse_pin,LOW);              // set pulse pin to low
    PWMWrite(signal_pin,pwmRes,dFnl,pwmClock);  // electrode cleaning step
    delay(2000);
    diffPulse();  

    if (mode == logASV) {

      for (int q = 2; q<=runs; ++q) {

        Serial.print(55555);  
        Serial.print('\t');
        Serial.print(q);  
        Serial.print('\t');
        Serial.println(00000);
        delay (runDelay*1000);
        digitalWrite(pulse_pin,LOW);              // set pulse pin to low
        PWMWrite(offset_pin,pwmRes,dOff,pwmClock);
        PWMWrite(signal_pin,pwmRes,dFnl,pwmClock);  // set signal voltage to 512 + 1/2 Vfnl
        delay(2000);
        diffPulse();
      }
    }
    Serial.print(99999);  
    Serial.print('\t');
    Serial.print(99999);  
    Serial.print('\t');
    Serial.println(00000);
  }
}



