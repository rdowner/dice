/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  REXX.C
 *
 *  (c) Copyright 1992 by Matthew Dillon, All Rights Reserved
 *
 *  original  ARexx port by Kim DeVaughn, 1987
 */

#include "defs.h"
#include <lib/rexx.h>

Prototype int foundcmd;
Prototype int CmdErr;

Prototype void init_arexx(void);
Prototype void do_rx (void);
Prototype void do_rx1 (void);
Prototype void do_rx2 (void);
Prototype void do_rxImplied (char *, char *);
Prototype int do_rexx (char *);

#if AREXX

int foundcmd;       /* control for implicit ARexx macro invocation   */
int CmdErr;         /* global command error flag for do_rexx()'s use */
PORT AuxRexxPort;   /* unique port name for DME-sourced commands     */

/*
 *  init_arexx() sets up an auxillary ARexx port which is unique.  We
 *  pass this port when generating outgoing ARexx commands so ARexx can
 *  find this particular instance of DME again.
 */

void
init_arexx()
{
    static char AuxName[15];
    if (RexxSysBase) {
        sprintf(AuxName, "DME.%08lx", FindTask(NULL));
        CreateDiceRexxPort(&AuxRexxPort, AuxName);
    }
}

/*
 *  Explicit ARexx invocation, no arguments
 */

void
do_rx()
{
    do_rexx(av[1]);
}

/*
 *  explicit invocation interface between do_command() and do_rexx
 *  for ARexx macros having ONE argument (i.e., for the "rx1" command)
 */

void
do_rx1()
{
    char macbuf[256];

    strcpy(macbuf, av[1]);
    strcat(macbuf, " ");
    strcat(macbuf, av[2]);
    do_rexx(macbuf);
}

/*
 *  explicit invocation interface between do_command() and do_rexx
 *  for ARexx macros having TWO arguments (i.e., for the "rx2" command)
 */

void
do_rx2()
{
    char macbuf[256];

    strcpy(macbuf, av[1]);
    strcat(macbuf, " ");
    strcat(macbuf, av[2]);
    strcat(macbuf, " ");
    strcat(macbuf, av[3]);
    do_rexx(macbuf);
}

/*
 *  implicit invocation interface between do_command() and do_rexx
 *  for ARexx macros implicitly called; arbitrary number of arguments
 */

void
do_rxImplied(cmd, args)
char *cmd;
char *args;
{
    char macbuf[256];

    strcpy(macbuf, cmd);
    strcat(macbuf, " ");
    strcat(macbuf, args);
    do_rexx(macbuf);
}

/*
 *  issue a command to ARexx ...
 */

int
do_rexx(macstr)
char *macstr;
{
    char *res;
    long rc = 1;

    if (RexxSysBase) {
        BPTR oldLock = CurrentDir(Ep->dirlock);
        long ec;
        rc = PlaceRexxCommand(&AuxRexxPort, macstr, &res, &ec);
        /*printf("rc = %d ec = %d res = %s\n", rc, ec, res ? res : "<null>");*/

        if (res) {
            av[1] = "result";
            av[2] = res;
            do_set();
            free(res);
        }
        if (rc) {
            if (ec)
                title("Command not found");
        }
        CurrentDir(oldLock);
    } else {
        title("couldn't open rexxsyslib.library on startup");
    }
    return(rc);
}

/*
 *  Process a received ARexx command
 */

int32_t
DoRexxCommand(msg, port, arg0, pres)
void *msg;
PORT *port;
char *arg0;
char **pres;
{
    long rc = 5;
    char *ptr;

    CmdErr = 0;
    foundcmd = 0;

    rc = do_command(ptr = strdup(arg0));
    free(ptr);

    if (foundcmd) {
        rc = (rc == 1) ? 0 : 5;
        if (CmdErr)
            rc = 10;
    } else {
        rc = do_rexx(arg0);
    }
    foundcmd = 1;   /* XXX */
    return(rc);
}

#endif

