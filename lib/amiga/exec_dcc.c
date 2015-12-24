
/*
 *  SYSTEM_DCC.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 *
 *  LoadSeg() and run a dcc program as this process.  ONLY DCC programs may
 *  be run with this call.  The resident list is searched properly.
 *
 *  uses cli_DefaultStack if we are a process and the cli exists, else uses
 *  a 4K stack.   Also modifies cli_CommandName and cli_Module temporarily.
 */

#define SysBase_DECLARED

#include <exec/types.h>
#include <exec/ports.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include <libraries/dosextens.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <lib/bcpl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <lib/misc.h>


typedef struct CommandLineInterface CLI;

extern struct ExecBase	*SysBase;

exec_dcc(cmd, args)
char *cmd;
char *args;
{
    long r;
    long seg;
    long unload = 0;
    long stackSize;
    char *stack;
    CLI *cli = NULL;

    if ((seg = _SearchResident(cmd)) == NULL) {
	unload = 1;
	if ((seg = LoadSeg(cmd)) == NULL) {
	    long lock;
	    long oldLock;
	    if (lock = _SearchPath(cmd)) {
		oldLock = CurrentDir(lock);
		seg = LoadSeg("");
		UnLock(CurrentDir(oldLock));
	    }
	}
    }
    if (seg == NULL) {
	errno = ENOTFND;
	return(-1);
    }

    /*
     *	allocate stack, make call, deallocate stuff.  p.s. do not need the -4.
     */

    stackSize = 4096;
    if (SysBase->ThisTask->tc_Node.ln_Type != NT_TASK) {
	cli = BTOC(((struct Process *)SysBase->ThisTask)->pr_CLI, CLI);

	if (cli)
	    stackSize = cli->cli_DefaultStack * 4;
    }
    if (stack = AllocMem(stackSize, MEMF_PUBLIC)) {
	char *cmdName;
	BPTR oldModule;
	BPTR oldName;

	if (cli) {
            oldModule = cli->cli_Module;
            oldName   = cli->cli_CommandName;
            cli->cli_Module = seg;

	    cmdName = malloc(strlen(cmd) + 3);

	    if (cmdName)
	    {
                strcpy(cmdName + 1, cmd);
                strcat(cmdName, "\n");
                cmdName[0] = strlen(cmd);

                cli->cli_CommandName = CTOB(cmdName);
	    }
	}
	r = _ExecSeg(seg, args, strlen(args), stack + stackSize - 4);

	FreeMem(stack, stackSize);

	if (cli) {
	    cli->cli_Module = oldModule;
	    cli->cli_CommandName = oldName;
	    free(cmdName);
	}
    }
    if (unload)
	UnLoadSeg(seg);
    return(r);
}

