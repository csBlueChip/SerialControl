#ifndef SERIALCONTROLLANG_H_
#define SERIALCONTROLLANG_H_

// Give each language a number
#define LANG_UK   1

// Pick a language
#define LANGUAGE LANG_EN

#if LANGUAGE == LANG_EN
	char*  cmdHelp      = (char*)"HELP";  // Must be in CAPITALS
#	define CMD_HELPLEN  4;                // Length of HELP command
	
	char*  txtSetup     = (char*)"cmdSetup() not called or failed!";
	char*  txtUnknown   = (char*)"Unknown command: ";
	char*  txtCommands  = (char*)"Commands";
	char*  txtListCmds  = (char*)"List available commands";
	char*  txtCmdHelp   = (char*)" <command> : Command specific help";
	char*  txtTooLong   = (char*)"Command too long";
	char*  txtLibName   = (char*)"SerialCommand";
	char*  txtSetupOK   = (char*)"Ready...";
	char*  txtSetupFail = (char*)"setup failed!";
#else
#	errror Language not defined
#endif

#endif // SERIALCONTROLLANG_H_
