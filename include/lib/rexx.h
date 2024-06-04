
/*
 * $VER: lib/rexx.h 1.0 (17.4.93)
 *
 * (c)Copyright 1992 Obvious Implementations Corp, All Rights Reserved
 *
 *  DICE REXX SUPPORT
 */

#ifndef LIB_REXX_H
#define LIB_REXX_H

#include <stdint.h>

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif
#ifndef EXEC_PORTS_H
#include <exec/ports.h>
#endif

int32_t PlaceRexxCommand(struct MsgPort *, char *, char **, int32_t *);
int32_t PlaceRexxCommandDirect(struct MsgPort *, char *, char *, char **, int32_t *);
void ProcessRexxCommands(struct MsgPort *);
short CreateGlobalDiceRexxPort(struct MsgPort *, char *);
short CreateDiceRexxPort(struct MsgPort *, char *);
void DeleteDiceRexxPort(struct MsgPort *);
int GetDiceRexxPortSlot(struct MsgPort *, char **);

extern struct Library *RexxSysBase; /*  NULL if DICE was unable to open lib*/

/*
 *  Supplied by you, the user.  You must supply a DoRexxCommand() function.
 *  Note that DICE will reply the rexxmsg for you, you need only process
 *  it.
 *
 *  You must declare 'char *RexxHostName = "APPLICATION";'.  DICE will create
 *  an appropriate public message port with the name "APPLICATION.xx" (DICE
 *  finds a free slot).  If you supply 'char *RexxHostName = NULL;' then
 *  DICE will not create a message port by default.
 *
 *  You can create AREXX ports with CreateDiceRexxPort() and
 *  CreateGlobalDiceRexxPort().  Passing NULL as the name to
 *  CreateDiceRexxPort() creates a private message port.
 *  CreateDiceRexxPort() returns the port slot number (for non-global public
 *  message ports) when applicable, -1 on error.
 */

int32_t DoRexxCommand(void *rexxmsg, struct MsgPort *port, char *arg0, char **resstr);

extern char *RexxHostName;
extern short RexxSigBit;

#endif
