#ifndef CMDPROCESSLANG_H_
#define CMDPROCESSLANG_H_

// Give each language a number
#define LANG_UK   1

// Pick a language
#define LANGUAGE LANG_EN

#if LANGUAGE == LANG_EN
	char*  cmdHelp      = (char*)F("HELP");  // Must be in CAPITALS
#	define CMD_HELPLEN  4;                   // Length of HELP command
	
	char*  txtSetup     = (char*)F("cmdSetup() not called or failed!");
	char*  txtUnknown   = (char*)F("Unknown command: ");
	char*  txtCommands  = (char*)F("Commands");
	char*  txtListCmds  = (char*)F("List available commands");
	char*  txtCmdHelp   = (char*)F(" <command> : Command specific help");
	char*  txtTooLong   = (char*)F("Command too long");
	char*  txtLibName   = (char*)F("SerialCommand");
	char*  txtSetupOK   = (char*)F("Ready...");
	char*  txtSetupFail = (char*)F("setup failed!");
#else
#	errror Language not defined
#endif

#endif // CMDPROCESSLANG_H_
