

#include "exec/types.h"
#include "exec/ports.h"
#include "exec/memory.h"

#include "clib/exec_protos.h"

// extern APTR AllocMem();
// extern UBYTE AllocSignal();
// extern struct Task *FindTask();

extern VOID NewList(struct List *);

/*
******* amiga.lib/CreatePort *************************************************
*
*   NAME
*	CreatePort - Allocate and initialize a new message port
*
*   SYNOPSIS
*	port = CreatePort(name,pri)
*
*	struct MsgPort *CreatePort(STRPTR,LONG);
*
*   FUNCTION
*	Allocates and initializes a new message port. The message list
*	of the new port will be prepared for use (via NewList).  A signal
*	bit will be allocated, and the port will be set to signal your
*	task when a message arrives (PA_SIGNAL).
*
*	You *must* use DeletePort() to delete ports created with
*	CreatePort()!
*
*   INPUTS
*	name - public name of the port, or NULL if the port is not named.
*	       The name string is not copied. Most ports do not need names,
*	       see notes below on this.
*	pri  - Priority used for insertion into the public port list,
*	       normally 0.
*
*   RESULT
*	port - a new MsgPort structure ready for use, or NULL if the port
*	       could not be created due to not enough memory or no available
*	       signal bit.
*
*   NOTE
*	In most cases, ports should not be named. Named ports are used for
*	rendez-vous between tasks. Everytime a named port needs to be located,
*	the list of all named ports must be traversed. The more named
*	ports there are, the longer this list traversal takes. Thus, unless
*	you really need to, do not name your ports, which will keep them off
*	of the named port list and improve system performance.
*
*   BUGS
*	With versions of amiga.lib prior to V37.14, this function would
*	not fail even though it couldn't allocate a signal bit. The port
*	would be returned with no signal allocated.
*
*   SEE ALSO
*	DeletePort(), exec.library/FindPort(), <exec/ports.h>,
*	exec.library/CreateMsgPort()
*
******************************************************************************
*/
struct MsgPort *CreatePort(name, pri)
    char *name;
    BYTE  pri;
{
LONG            sig;
struct MsgPort *port;

    sig = AllocSignal(-1);
    if (sig > 0)
    {
        if (port = AllocMem((ULONG)sizeof(struct MsgPort),MEMF_CLEAR|MEMF_PUBLIC))
        {
            port->mp_Node.ln_Name = name;
            port->mp_Node.ln_Pri  = pri;
            port->mp_Node.ln_Type = NT_MSGPORT;
            port->mp_Flags        = PA_SIGNAL;
            port->mp_SigBit       = sig;
            port->mp_SigTask      = FindTask(NULL);

            if (name)
                AddPort(port);
            else
                NewList(&port->mp_MsgList);

            return(port);
        }
        FreeSignal(sig);
    }

    return(NULL);
}


/*
******* amiga.lib/DeletePort *************************************************
*
*   NAME
*	DeletePort - free a message port created by CreatePort()
*
*   SYNOPSIS
*	DeletePort(port)
*
*	VOID DeletePort(struct MsgPort *);
*
*   FUNCTION
*	Frees a message port created by CreatePort. All messages that
*	may have been attached to this port must have already been
*	replied before this function is called.
*
*   INPUTS
*	port - message port to delete
*
*   SEE ALSO
*	CreatePort()
*
******************************************************************************
*/

VOID DeletePort(port)
    struct MsgPort *port;
{
    if (port->mp_Node.ln_Name)
	RemPort(port);

    port->mp_Node.ln_Type    = 0xff;
    port->mp_MsgList.lh_Head = (struct Node *) -1;

    FreeSignal(port->mp_SigBit);

    FreeMem(port,(ULONG)sizeof(struct MsgPort));
}
