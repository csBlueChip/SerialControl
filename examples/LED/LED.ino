//------------------------------------------------------------------------------
// Cross-platform compatability
//
#ifndef STDIO
#  include <variant.h>
#  if defined(_VARIANT_MACCHINA_M2_)
#    define  STDIO  SerialUSB
#  else
#    define  STDIO  Serial
#  endif
#endif

//------------------------------------------------------------------------------
// You need this to use the SerialControl library
//
#include "SerialControl.h"

//------------------------------------------------------------------------------
// Local system definitions
//
#define  ledPin  LED_BUILTIN  // Pin number for LED (must be a PWM pin for "PULSE")

//------------------------------------------------------------------------------
// You must add an entry here for each command you write
//
int  ledOn    (int argc,  char* argv[]) ;
int  ledOff   (int argc,  char* argv[]) ;
int  ledDim   (int argc,  char* argv[]) ;
int  ledPulse (int argc,  char* argv[]) ;
int  showArgs (int argc,  char* argv[]) ;

//------------------------------------------------------------------------------
// Command System
//
// Command format:
//   {"COMMAND", functionName, minArg, maxArg, "Help Text"}
//
//   "COMMAND"    .. Must be upper-case; must NOT contain whitespace (spaces/tabs);
//                   Must be enclosed in "quotes"
//   functionName .. The name of the function that will be called for this command
//                   It must have the function prototype:
//                      int functionName (int argc,  char* argv[])
//   minArg       .. The minimum number of arguments that are allowed
//   maxArg       .. The maximum number of arguments that are allowed
//   "Help Text"  .. What the Arduino will display if you say: HELP COMMAND
//
// "argc, argv" work exactly the same way as a normal C main() function.
// So lots of documentation is available on how to manipulate these parameters.
//

// This specifies the longest allowable command
// If you try to enter a longer command, your input will be truncated
#define  cmdMax  15

cmd_t  commands[] = { 
    {"ON",    ledOn ,   0,0, "Turn LED on"},
    {"OFF",   ledOff,   0,0, "Turn LED off"},
    {"DIM",   ledDim,   1,1, "DIM <brightness> : Dim LED. brightness=0..255"},
    {"PULSE", ledPulse, 0,2, "PULSE [<pulses> [<speed>]] : Pulse LED. pulses=nr of pulses, speed=0[fast]..1000[slow]"},
    {"ARGS",  showArgs, 0,9, "ARGS {up to nine arguments}"},
    cmd_endList  // This MUST always be the last entry in the command list
};

//+=============================================================================
// Configure the Arduino
//
void  setup ( ) 
{
    pinMode(ledPin, OUTPUT);          // Set the LED control pin to OUTPUT
  
    STDIO.begin(115200);              // Talk to the PC at 115200 baud
    while (!STDIO)  delay(10) ;       // Wait for the user to connect to the serial port
    
    if (!cmdSetup(commands, cmdMax))  // Try to setup the command system
        while (1)  delay(1000) ;      // If we failed, hang here!
        
    cmdExec("ON");                    // Issue a direct command
}

//+=============================================================================
// The main monitoring loop
//
void  loop ( ) 
{
    if (cmdWaiting()) {            // Is there a command waiting?
        STDIO.print(":");          //   Announce the command we received
        STDIO.println(cmdGet());   //   ...
        cmdExec(NULL);             //   Execute the captured command
        cmdClear();                //   Prepare for the next serial command
    }
    serialEvent();                 // You SHOULD never need this
}

//+=============================================================================
// This is [should be] called every time the loop() ends
// If things aren't going well, uncomment the call at the end of loop()
//
void  serialEvent ( ) 
{
    cmdSerial();  // Use the serial command grabber
}

//+=============================================================================
// Show the arguments as received (test & debug)
//
int  showArgs (int argc,  char* argv[])
{
    STDIO.print(F("Count: "));
    STDIO.println(argc, DEC);
    for (int i = 0;  i < argc;  i++) {
        STDIO.print(i, DEC);
        STDIO.print(F(" : |"));
        STDIO.print(argv[i]);
        STDIO.println(F("|"));
    }
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
    int  dim;         // Dimmer loop
    int  step;        // Brightness step
    int  cnt   = 1;   // Pulse count (default 1)
    int  speed = 20;  // Speed control (default 20); Lower is faster

    argc--;  // Ignore the 0th argument ("DIM")
    
    // If there is "at least one" argument, we assume it is a valid number
    // (as an ASCII string) and we convert it to an integer
    if (argc >= 1)  cnt = atoi(argv[1]);

    // If there are "two" arguments, we assume the second is a valid number
    // (as an ASCII string) and we convert it to an integer
    if (argc == 2)  speed = atoi(argv[2]);
    
    STDIO.print("Pulsing, please wait: ");

    for ( ;  cnt > 0;  cnt--) {
        STDIO.print(cnt, DEC);
        STDIO.print(".");
      
        // Fade in
        step = 0;
        for (dim = 1;  dim <= 510;  step += 1, dim += step) {
            analogWrite(ledPin, dim/2);    
            delay(speed);
        } 

        STDIO.print(".");

        // Fade out
        for (dim = 510;  dim >= 1;  dim -= step, step -= 1) {
            analogWrite(ledPin, dim/2);    
            delay(speed);
        }
    }

    // Finish with LED=off
    digitalWrite(ledPin, LOW);  
    STDIO.println("Done.");
}
