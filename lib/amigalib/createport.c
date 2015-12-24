
/*
 *  CreatePort.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 *
 */

#include <exec/types.h>
#include <exec/ports.h>
#include <exec/memory.h>
#ifdef INCLUDE_VERSION	    /*	2.0 */
#include <clib/exec_protos.h>
#include <clib/alib_protos.h>
#else
extern void *AllocMem(long, long);
extern void FreeMem(void *, long);
extern void NewList(struct List *);
extern void AddPort(struct MsgPort *);
extern void RemPort(struct MsgPort *);
extern int AllocSignal(int);
extern void FreeSignal(int);
extern void *FindTask(char *);
#endif

#ifndef HYPER
#define HYPER
#endif

typedef struct MsgPort	MsgPort;

MsgPort *
HYPER ## CreatePort(name, pri)
UBYTE *name;
long pri;
{
    MsgPort *port;

    if (port = AllocMem(sizeof(MsgPort), MEMF_PUBLIC | MEMF_CLEAR)) {
	if ((char)(port->mp_SigBit = AllocSignal(-1)) >= 0) {
	    port->mp_Node.ln_Pri = pri;
	    port->mp_Node.ln_Type = NT_MSGPORT;
	    port->mp_Node.ln_Name = name;
	    port->mp_Flags   = PA_SIGNAL;
	    port->mp_SigTask = FindTask(NULL);
	    NewList(&port->mp_MsgList);
	    if (name)
		AddPort(port);
	} else {
	    FreeMem(port, sizeof(MsgPort));
	    port = NULL;
	}
    }
    return(port);
}

void
DeletePort(port)
MsgPort *port;
{
    if (port) {
	if (port->mp_Node.ln_Name)
	    RemPort(port);
	if ((port->mp_Flags & PF_ACTION) == PA_SIGNAL) {
	    FreeSignal(port->mp_SigBit);
	    port->mp_Flags = PA_IGNORE;
	}
	FreeMem(port, sizeof(MsgPort));
    }
}

