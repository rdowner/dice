
/*
 *  _MAIN.C
 *
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 *
 *
 *  Note that to use this _main we must be a process.  The programmer
 *  can overide our _main with his own if he wishes a task entry.
 *
 *  Note that we open '*' with modes 1005 so if the filesystem is our stderr
 *  we don't create a file called '*'.
 */

#define SysBase_DECLARED

#include <exec/types.h>
#include <exec/execbase.h>
#include <libraries/dos.h>
#include <libraries/dosextens.h>
#include <workbench/startup.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <lib/bcpl.h>
#include <lib/misc.h>

extern int wbmain(void *);
extern int main(int, char **);
extern void _waitwbmsg(void);

typedef struct CommandLineInterface CLI;
typedef struct Process Process;
typedef struct ExecBase ExecBase;

extern void *_WBMsg;
extern ExecBase *SysBase;

__stkargs void
_main(len, arg)
short len;
char *arg;
{
    Process *proc = (Process *)FindTask(NULL);

    if (proc->pr_Task.tc_Node.ln_Type == NT_PROCESS) {
	/*DOSBase = OpenLibrary("dos.library", 0);*/

	if (proc->pr_CIS) {
	    _IoStaticFD[0].fd_Fh    =	proc->pr_CIS;
	    _IoStaticFD[0].fd_Flags =	O_RDWR | O_NOCLOSE | O_ISOPEN;
	}

	if (proc->pr_COS) {
	    _IoStaticFD[1].fd_Fh    =	proc->pr_COS;
	    _IoStaticFD[1].fd_Flags =	O_RDWR | O_NOCLOSE | O_ISOPEN;
	}

	if (proc->pr_ConsoleTask && proc->pr_CLI) {
	    CLI *cli = BTOC(proc->pr_CLI, CLI);

	    if (cli->cli_Background == 0) {
		if (_IoStaticFD[2].fd_Fh = Open("*", 1005))
		    _IoStaticFD[2].fd_Flags = O_RDWR | O_ISOPEN;
	    }
	    if (_IoStaticFD[2].fd_Fh == NULL && _IoStaticFD[1].fd_Fh) {
		_IoStaticFD[2].fd_Fh = _IoStaticFD[1].fd_Fh;
		_IoStaticFD[2].fd_Flags = O_RDWR | O_NOCLOSE | O_ISOPEN;
	    }
	}
    }

    if (_IoStaticFD[0].fd_Flags & O_ISOPEN)
	_finitdesc(stdin,  0, __SIF_READ);
    if (_IoStaticFD[1].fd_Flags & O_ISOPEN)
	_finitdesc(stdout, 1, __SIF_WRITE);
    if (_IoStaticFD[2].fd_Flags & O_ISOPEN)
	_finitdesc(stderr, 2, __SIF_WRITE);

    if (proc->pr_Task.tc_Node.ln_Type == NT_PROCESS) {
	CLI *cli;

	if (cli = BTOC(proc->pr_CLI, CLI)) {
	    char **av;
	    int ac;
	    unsigned char *copy = malloc(len+1);
	    unsigned char *cname = (unsigned char *)((long)cli->cli_CommandName << 2);

            if (copy == NULL || cname == NULL) exit(-1);  /* Make sure we have the memory */
	    _slow_bcopy(arg, copy, len);
	    copy[len] = 0;
	    ac = _parseargs1(copy, len);
	    av = malloc((ac + 2) << 2);
	    if (av == NULL) exit(-1);  /* Make sure we have the memory */
	    _parseargs2(copy, av+1, ac);

	    ++ac;   /*	include arg0	*/

	    if (cname) {
		len = *cname;

		arg = malloc(len+1);
		_slow_bcopy(cname + 1, arg, len);
		arg[len] = 0;
	    }
	    av[0] = arg;
	    av[ac] = 0;
	    exit(main(ac, av));
	} else {
	    CurrentDir(((struct WBStartup *)_WBMsg)->sm_ArgList->wa_Lock);
	    exit(wbmain(_WBMsg));
	}
    }
    exit(-1);
    _waitwbmsg();   /*	dummy, brings in alib/wbmain.a	    */
}

