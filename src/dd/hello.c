/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include        <stddef.h>
#include        <stdlib.h>
#include        <stdio.h>
#include        <fcntl.h>
#include        <exec/execbase.h>
#include        <dos/dosextens.h>
#include        <clib/dos_protos.h>
#include        <clib/exec_protos.h>

extern void     testfunc(void);

typedef struct CommandLineInterface     CLI;
typedef struct Process                  PROCESS;


extern struct ExecBase                  *SysBase;

PROCESS *p;
CLI     *cli;

int    main(int ac, char *av[]) 
{
        short   i;
//      char s[256];
        char    *ps;


        testfunc();
// ******************************************************************************
//        p = (PROCESS *)SysBase->ThisTask;
        p = (struct Process *)FindTask(NULL);
/* this is a comment of
   multi line nature */
        cli = BADDR(p->pr_CLI);
        ps = BADDR(cli->cli_CommandName);
        printf("hello, world\n");
        printf("Command Line = '%s'\n", ps);
        for (i=0; i<ac; i++)
        {
            printf("arg %d = '%s'\n", i, av[i]);
            printf("args printed\n");
        }
        exit(0);
}

Abort() {
        exit(0);
}
