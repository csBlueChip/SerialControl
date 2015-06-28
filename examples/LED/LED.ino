#include "cmdProcess.h"

//------------------------------------------------------------------------------
// Local system definitions
//
#define  ledPin  3   // Pin number for LED (must be a PWM pin for "PULSE")

//------------------------------------------------------------------------------
// Command list
//
#define  cmdMax  15  // Size of command buffer

cmd_t  commands[] = { 
    {"ON",    ledOn ,   0,0, "Turn LED on"},
    {"OFF",   ledOff,   0,0, "Turn LED off"},
    {"DIM",   ledDim,   1,1, "DIM <brightness> : Dim LED. brightness=0..255"},
    {"PULSE", ledPulse, 0,2, "PULSE [<pulses> [<speed>]] : Pulse LED. pulses=nr of pulses, speed=0[fast]..1000[slow]"},
    cmd_endList 
};

//+=============================================================================
// Configure the Arduino
//
void  setup ( ) 
{
    pinMode(ledPin, OUTPUT);          // Set the LED control pin to OUTPUT
    Serial.begin(9600);               // Talk to the PC at 9600 baud
    if (!cmdSetup(commands, cmdMax))  // Try to setup the command system
        while (1)  delay(1000) ;      // If we failed, hang here!
    cmdExec("ON");                    // Issue a direct command
}

//+=============================================================================
//  The main monitoring loop
//
void  loop ( ) 
{
    if (cmdWaiting()) {            // Is there a command waiting?
        Serial.print(":");         //   Announce the command we received
        Serial.println(cmdGet());  //   ...
        cmdExec(NULL);             //   Execute the captured command
        cmdClear();                //   Prepare for the next serial command
    }
}

//+=============================================================================
//  This is called every time the loop() ends
//
void  serialEvent ( ) 
{
  cmdSerial();  // Use the serial command grabber
}

//+=============================================================================
// The function that performs the command "ON"
//
int  ledOn (int argc,  char* argv[])
{
    digitalWrite(ledPin, HIGH) ;
}

//+=============================================================================
// The function that performs the command "OFF"
//
int  ledOff (int argc,  char* argv[])
{
    digitalWrite(ledPin, LOW) ;
}

//+=============================================================================
// Set LED brightness
//
int  ledDim (int argc,  char* argv[])
{
    analogWrite(ledPin, atoi(argv[1]));    
}

//+=============================================================================
// Pulse LED 'N' times
// Pulsing uses PWM, so the LED must be connected to pins 3, 5, 6, 9, 10 or 11
// Because pins 0, 1, 2, 4, 7, 8, and 13 do not support PWM
//
int  ledPulse (int argc,  char* argv[])
{
    int  dim;
    int  step;
    int  cnt   = (argc >= 1+1) ? atoi(argv[1]) : 1;
    int  speed = (argc == 1+2) ? atoi(argv[2]) : 20;  // lower is faster
    
    Serial.print("Pulsing, please wait: ");
    for ( ;  cnt > 0;  cnt--) {
        Serial.print(cnt, DEC);
        Serial.print(".");
      
        step = 0;
        for (dim = 1;  dim <= 510;  step += 1, dim += step) {
            analogWrite(ledPin, dim/2);    
            delay(speed);
        } 
        Serial.print(".");
        for (dim = 510;  dim >= 1;  dim -= step, step -= 1) {
            analogWrite(ledPin, dim/2);    
            delay(speed);
        }
    }
    
    digitalWrite(ledPin, LOW);  // Finish with LED=off
    Serial.println("Done.");
}
