#ifndef CMDPROCESS_H_
#define CMDPROCESS_H_

typedef
	struct {
		char*  cmd;
		int    (*func)(int argc,  char* argv[]);
		int    argMin;
		int    argMax;
		char*  help;
	}
cmd_t;

#define  cmd_endList  {NULL, NULL, 0,0, NULL}

bool   cmdSetup   (cmd_t* list,  int max) ;
int    cmdExec    (char* cmd) ;
void   cmdClear   ( ) ;
void   cmdSerial  ( ) ;
bool   cmdWaiting ( ) ;
char*  cmdGet     ( ) ;

#endif // CMDPROCESS_H_
