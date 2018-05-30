#include <Arduino.h>

//----------------------------------------------------------------------------------------------------------------------
// Includes
//
#include "SerialControl.h"
#include "SerialControlLang.h"

#include <string.h>
#include <stdlib.h>

//----------------------------------------------------------------------------------------------------------------------
// Command control variable
//
bool          isSetup = false;

char*         cmd;       // Somewhere to hold the command
unsigned int  cmdLen;    // The (current) length of the command
unsigned int  cmdMax;    // Maximum length of a command
bool          cmdReady;  // Command is ready to be processed

const cmd_t*  cmdList;   // The command list

int           argMax;    // Maximum nuber of arguments for a command
char*         argList;   // The argument list
char**        argPtr;    // Argument pointers (to within argList)

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
bool  cmdSetup (const cmd_t* list,  int max)
{
	const cmd_t*  cmdp;

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
	STDIO.print(txtLibName);
	STDIO.print(F(" v"));
	STDIO.print(CMDP_VERMAJ, DEC);
	STDIO.print(F("."));
	STDIO.print(CMDP_VERMIN, DEC);
	STDIO.print(F(" "));
	STDIO.println( (isSetup) ? txtSetupOK : txtSetupFail );

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
int  cmdExec (const char* input)
{
	int           i;
	int           argCnt;
	char*         cp;
	char**        ap;
	const cmd_t*  cmdp;
	bool          quote;

	if (!isSetup)  return -1 ;

	if (input) {
		if (strlen(input) > cmdMax) {
			STDIO.print(txtTooLong);
			STDIO.print(F(": \""));
			STDIO.print(input);
			STDIO.println(F("\""));
			return -1;
		}
		memcpy(argList, input, cmdMax + 1);  // Copy the   new    command to argument list
	} else {
		memcpy(argList, cmd,   cmdMax + 1);  // Copy the previous command to argument list
	}

	// Empty all the argument pointers
	for (i = 0;  i < argMax + 1 + 1;  i++)  argPtr[i] = NULL ;

	// Break command up in to arguments
	cp = argList;                                      // Start processing the argument list
	while (ISWHITE(*cp))  cp++ ;                       // Skip prefixing whitesapce
	argCnt = 0;                                        // Start with 0 arguments
	for (ap = argPtr;  *cp;  ap++) {                   // Traverse input
		if (*cp == '"') {                              // If first char is a quote
			cp++;                                      //   Skip past it
			*ap = cp;                                  //   Collect <this> argument from AFTER the quote
			while (*cp && (*cp != '"'))  cp++ ;        //   This argument ends with another quote (or EOL)
		} else {                                       // else (no quote)
			*ap = cp;                                  //   Collect <this> argument from the beginning
			while (*cp && !ISWHITE(*cp))  cp++ ;       //   This argument ends with a whitespace (or EOL)
		}                                              //
		argCnt++;                                      //   Count the argument
		if (*cp != '\0') {                             //   If not end of input
			*cp = '\0';                                //     Terminate argument
			cp++;                                      //     Keep searching
		}                                              //
		if (argCnt == argMax)  break ;                 //   Argument array full?  Stop!
		while (ISWHITE(*cp))  cp++ ;                   //   Skip interim whitespace
	}

	// Uppercase the command
	for (cp = argPtr[0]; *cp;  cp++)  *cp = toupper(*cp) ;

	// No command given
	if (argCnt == 0)  {       // No arguments found
		argCnt = 2;           //   Infer "HELP HELP"
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
			STDIO.print(txtUnknown);
			STDIO.print(F("\""));
			STDIO.print(argPtr[0]);
			STDIO.println(F("\""));
			// Request command list
			argPtr[0] = cmdHelp;
			argPtr[1] = NULL;
		}
	}

	if (strcmp(argPtr[0], cmdHelp) == 0) {                          // Request for HELP?
		if (argPtr[1] == NULL) {                                    // Just "HELP"
			STDIO.print(txtCommands);                               // List commands
			STDIO.print(F(" {"));                                   // List commands
			STDIO.print(cmdHelp);                                   // "HELP"
			for (cmdp = cmdList;  cmdp->cmd;  cmdp++) {             //   Traverse command array
				STDIO.print(F(", "));                               //     List command
				STDIO.print(cmdp->cmd);                             //     ...
			}                                                       //
			STDIO.println(F("}"));                                  //   End of list
		} else {                                                    // "HELP xxx"
			for (cp = argPtr[1]; *cp;  cp++)  *cp = toupper(*cp) ;  // Uppercase the command name
			if (strcmp(argPtr[1], cmdHelp) == 0) {                  //   "HELP HELP"?
				STDIO.print(cmdHelp);                               //     Give help overview
				STDIO.print(F(" : "));                              //     ...
				STDIO.println(txtListCmds);                         //     ...
				STDIO.print(cmdHelp);                               //     ...
				STDIO.println(txtCmdHelp);                          //     ...
			} else {                                                //   "HELP <command>"
				for (cmdp = cmdList;  cmdp->cmd;  cmdp++) {         //     Traverse command array
					if (strcmp(argPtr[1], cmdp->cmd) == 0) {        //     Found <command>?
						STDIO.println(cmdp->help);                  //       Show specific help
						break;                                      //       done.
					}                                               //
				}                                                   //
				if (!cmdp->cmd) {                                   //   <command> not found
					STDIO.print(txtUnknown);
					STDIO.print(F("\""));
					STDIO.print(argPtr[1]);
					STDIO.println(F("\""));
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
	while (STDIO.available()) {
		int  chr = STDIO.read();    // Read in a keystroke

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

