/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */
/*
 *  _MAIN.C
 *
 *  Note that to use this _main we must be a process.  The programmer
 *  can overide our _main with his own if he wishes a task entry.
 *
 *  Note that we open '*' with modes 1005 so if the filesystem is our stderr
 *  we don't create a file called '*'.
 */
#include	<localdefs.h>

extern int	wbmain(void *);
extern int	main(int, char **);
extern void	_waitwbmsg(void);

extern WBMSG	*_WBMsg;
extern EBASE	*SysBase;

char		*args;
ULONG		argSize;
TASK		*thisTask;
APROCESS	*thisProcess;
CLI		*thisCli;
APTR		debugStack, debugStackTop, systemTrapHandler;
BPTR		commandNameSave;


__stkargs void _main(short len, char *arg) {
	APROCESS *proc = (APROCESS *)FindTask(NULL);

#ifdef NOTDEF	/* added by matt to debug the debugger */
	arg = "hello a b c\r\n";
	len = 13;
#endif

	thisProcess = proc; thisTask = (TASK *)proc;
	systemTrapHandler = thisTask->tc_TrapCode;
	debugStack = thisTask->tc_SPLower; debugStackTop = thisTask->tc_SPUpper;
	thisCli = BTOC(proc->pr_CLI, CLI);
	commandNameSave = thisCli->cli_CommandName;
	args = arg; argSize = len;

	if (proc->pr_Task.tc_Node.ln_Type == NT_PROCESS) {
		/*DOSBase = OpenLibrary("dos.library", 0);*/

		if (proc->pr_CIS) {
			_IoStaticFD[0].fd_Fh = proc->pr_CIS;
			_IoStaticFD[0].fd_Flags = O_RDWR | O_NOCLOSE | O_ISOPEN;
		}

		if (proc->pr_COS) {
			_IoStaticFD[1].fd_Fh = proc->pr_COS;
			_IoStaticFD[1].fd_Flags = O_RDWR | O_NOCLOSE | O_ISOPEN;
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

	_finitdesc(stdin,  0, __SIF_READ  | __SIF_NOFREE);
	_finitdesc(stdout, 1, __SIF_WRITE | __SIF_NOFREE);
	_finitdesc(stderr, 2, __SIF_WRITE | __SIF_NOFREE);

	if (proc->pr_Task.tc_Node.ln_Type == NT_PROCESS) {
		CLI *cli;

		if (cli = BTOC(proc->pr_CLI, CLI)) {
			char **av;
			int ac;
			unsigned char *copy = malloc(len+1);
			unsigned char *cname = (unsigned char *)((long)cli->cli_CommandName << 2);

			_slow_bcopy(arg, copy, len);
			copy[len] = 0;
			ac = _parseargs1(copy, len);
			av = malloc((ac + 2) << 2);
			_parseargs2(copy, av+1, ac);

			++ac;	/* include arg0 */

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
			CurrentDir(_WBMsg->sm_ArgList->wa_Lock);
			exit(wbmain(_WBMsg));
		}
	}
	exit(-1);
	_waitwbmsg();	/*  dummy, brings in alib/wbmain.a	*/
}

