#ifndef CMDPROCESSLANG_H_
#define CMDPROCESSLANG_H_

// Give each language a number
#define LANG_UK   1

// Pick a language
#define LANGUAGE LANG_EN

#if LANGUAGE == LANG_EN
	char*  cmdHelp      = "HELP";  // Must be in CAPITALS
#	define CMD_HELPLEN  4;         // Length of HELP command
	
	char*  txtSetup     = "cmdSetup() not called or failed!";
	char*  txtUnknown   = "Unknown command: ";
	char*  txtCommands  = "Commands";
	char*  txtListCmds  = "List available commands";
	char*  txtCmdHelp   = " <command> : Command specific help";
	char*  txtTooLong   = "Command too long";
	char*  txtSetupOK   = "Command system ready.";
	char*  txtSetupFail = "Command system failed.";
#else
#	errror Language not defined
#endif

#endif // CMDPROCESSLANG_H_
