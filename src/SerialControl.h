#ifndef SERIALCONTROL_H_
#define SERIALCONTROL_H_

#define CMDP_VERMAJ  2
#define CMDP_VERMIN  2

#ifndef STDIO
#	include <variant.h>
#	if defined(_VARIANT_MACCHINA_M2_)
#		define  STDIO  SerialUSB
#	else
#		define  STDIO  Serial
#	endif
#endif

typedef
	struct {
		const char*  cmd;
		int          (*func)(int argc,  char* argv[]);
		int          argMin;
		int          argMax;
		const char*  help;
	}
cmd_t;

#define  cmd_endList  {NULL, NULL, 0,0, NULL}

#define  ISWHITE(c)  ((c == ' ') || (c == '\t'))

bool   cmdSetup   (const cmd_t* list,  int max) ;
int    cmdExec    (const char* cmd) ;
void   cmdClear   ( ) ;
void   cmdSerial  ( ) ;
bool   cmdWaiting ( ) ;
char*  cmdGet     ( ) ;

#endif // SERIALCONTROL_H_
