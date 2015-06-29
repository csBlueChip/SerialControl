#include <Arduino.h>

//----------------------------------------------------------------------------------------------------------------------
// Includes
//
#include "SerialControl.h"
#include "inc/SerilaControlLang.h"

#include <string.h>
#include <stdlib.h>

//----------------------------------------------------------------------------------------------------------------------
// Command control variable
//
bool    isSetup = false;

char*   cmd;       // Somewhere to hold the command
int     cmdLen;    // The (current) length of the command
int     cmdMax;    // Maximum length of a command
bool    cmdReady;  // Command is ready to be processed

cmd_t*  cmdList;   // The command list

int     argMax;    // Maximum nuber of arguments for a command        
char*   argList;   // The argument list
char**  argPtr;    // Argument pointers (to within argList)

//+=====================================================================================================================
// Is the command ready to be processed
//
char*  cmdGet ( )
{
	return cmd;
}

//+=====================================================================================================================
// Is the command ready to be processed
//
bool  cmdWaiting ( )
{
	return cmdReady;
}

//+=====================================================================================================================
// Setup the command system
//
bool  cmdSetup (cmd_t* list,  int max)
{
	cmd_t*  cmdp;

	cmdMax = max;                    
	cmd    = (char*)malloc(cmdMax + 1);  // Allow for NUL terminator

	// Traverse the command list and discover the value for "most arguments"
	argMax  = 1;     // The command itself is the first (0th) argument
	cmdList = list;
	for (cmdp = cmdList;  cmdp->cmd;  cmdp++)
		// 1+ for argv[0]==<command> ... +1 for overflow parameters
		if (1 + cmdp->argMax + 1 > argMax)  argMax = 1 + cmdp->argMax + 1;  
		
	argList = (char*)malloc(cmdMax);
	argPtr  = (char**)malloc(sizeof(*argPtr) * (argMax + 1));  // +1 for NULL terminator

	// Ensure all memory was allocated successfully
	isSetup = cmd && argList && argPtr;

	// Initialise the input buffer
	cmdClear();

	// Announce the system
	Serial.print(txtLibName);
	Serial.print(" v");
	Serial.print(CMDP_VERMIN, DEC);
	Serial.print(".");
	Serial.print(CMDP_VERMAJ, DEC);
	Serial.print(" ");
	Serial.println( (isSetup) ? txtSetupOK : txtSetupFail );

	return isSetup;
}

//+=====================================================================================================================
// Clear the command string
//
void  cmdClear ( )
{
	if (!isSetup)  return ;

	memset(cmd, 0, cmdMax + 1);  // Set all bytes of the command string to 0 ('NUL')
	cmdLen   = 0;                // Set the command length to 0
	cmdReady = false;            // No command is ready for processing
}

//+=====================================================================================================================
// Execute the specified command
//
int  cmdExec (char* input)
{
	int     i;
	int     argCnt;
	char*   cp;
	char**  ap;
	cmd_t*  cmdp;
	char*   exec;

	if (!isSetup)  return -1 ;

	if (input) {
		if (strlen(input) > cmdMax) {
			Serial.print(txtTooLong);
			Serial.print(": \"");
			Serial.print(input);
			Serial.println("\"");
			return -1;
		}
		exec = input;
	} else {
		exec = cmd;
	}
	
	memcpy(argList, exec, cmdMax + 1);                          // Copy the command to argument list
	for (i = 0;  i < argMax + 1 + 1;  i++)  argPtr[i] = NULL ;  // Empty all the argument pointers                                  
	
	// Break command up in to arguments
	cp = argList;                                      // Start processing the argument list
	while (*cp == ' ')  cp++ ;                         // Skip prefixing whitesapce
	argCnt = 0;                                        // Start with 0 arguments
	for (ap = argPtr;  *cp;  ap++) {                   // Traverse input
		*ap = cp;                                      //   Collect <this> argument
		argCnt++;                                      //   ...
		while ((*cp != ' ') && (*cp != '\0'))  cp++ ;  //   Skip past this argument
		if (*cp != '\0') {                             //   If not end of input
			*cp = '\0';                                //     Terminate argument
			cp++;                                      //     Keep searching
		}                                              //
		if (argCnt == argMax)  break ;                 //   Argument array full?  Stop!
		while (*cp == ' ')  cp++ ;                     //   Skip interim whitespace
	}	

	// Uppercase the command
	for (cp = argPtr[0]; *cp;  cp++)  *cp = toupper(*cp) ;
	
	// No command given
	if (argCnt == 0)  {       // No arguments found
		argCnt = 2;           //   Imply "HELP HELP"
		argPtr[0] = cmdHelp;  //   ...
		argPtr[1] = cmdHelp;  //   ...
	}

	// Try to execute command
	if (strcmp(argPtr[0], cmdHelp) != 0) {            // Not "HELP"
		for (cmdp = cmdList;  cmdp->cmd;  cmdp++) {   // Traverse command array
			if (strcmp(argPtr[0], cmdp->cmd) == 0) {  // Command found?
      
				// Valid number of arguments for this command?  Execute command!
				if ((cmdp->argMin <= argCnt - 1) && (cmdp->argMax >= argCnt - 1)) {
					return cmdp->func(argCnt, argPtr);  // Return function exit code
          
				// Wrong number	of arguments for this command?  Imply request for HELP <cmd>
				} else {
					argPtr[1] = argPtr[0];
					argPtr[0] = cmdHelp;
					break;
				}
			}
		}
		// Didn't find the requested command
		if (!cmdp->cmd) {
			// Announce failure
			Serial.print(txtUnknown);
			Serial.print("\"");
			Serial.print(argPtr[0]);
			Serial.println("\"");
			// Request command list
			argPtr[0] = cmdHelp;
			argPtr[1] = NULL;
		}
	}

	if (strcmp(argPtr[0], cmdHelp) == 0) {                          // Request for HELP?
		if (argPtr[1] == NULL) {                                    // Just "HELP"
			Serial.print(txtCommands);                              // List commands
			Serial.print(" {");                                     // List commands
			Serial.print(cmdHelp);                                  // "HELP"
			for (cmdp = cmdList;  cmdp->cmd;  cmdp++) {             //   Traverse command array
				Serial.print(", ");                                 //     List command
				Serial.print(cmdp->cmd);                            //     ...
			}                                                       // 
			Serial.println("}");                                    //   End of list
		} else {                                                    // "HELP xxx"
			for (cp = argPtr[1]; *cp;  cp++)  *cp = toupper(*cp) ;  // Uppercase the command name
			if (strcmp(argPtr[1], cmdHelp) == 0) {                  //   "HELP HELP"?
				Serial.print(cmdHelp);                              //     Give help overview
				Serial.print(" : ");                                //     ...
				Serial.println(txtListCmds);                        //     ...
				Serial.print(cmdHelp);                              //     ...
				Serial.println(txtCmdHelp);                         //     ...
			} else {                                                //   "HELP <command>"
				for (cmdp = cmdList;  cmdp->cmd;  cmdp++) {         //     Traverse command array
					if (strcmp(argPtr[1], cmdp->cmd) == 0) {        //     Found <command>?
						Serial.println(cmdp->help);                 //       Show specific help
						break;                                      //       done.
					}                                               // 
				}                                                   // 
				if (!cmdp->cmd) {                                   //   <command> not found
					Serial.print(txtUnknown);
					Serial.print("\"");
					Serial.print(argPtr[1]);
					Serial.println("\"");
				}                                                   
			}
		}
	}
	return -1;
}

//+=====================================================================================================================
// Collect input from serial port
//
void  cmdSerial ( )
{
	if (!isSetup)  return ;
	
	// For as long as there are keystrokes to collect
	while (Serial.available()) {
		int  chr = Serial.read();  // Read in a keystroke

		// If the key is "Newline", command is ready for processing
		if (chr == '\n') {
			cmdReady = true;

		// Key is not a "Newline"
		} else {  
			if (cmdLen < cmdMax) {  // If there is room to add this keystroke
				cmd[cmdLen] = chr;  //   Add the keystroke
				cmdLen++;           //   Increase the length by 1
			}
		}
	}
}

