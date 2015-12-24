/***********************************************************************
*
*	Exec Support Function -- Extended IO Request
*
***********************************************************************/

#include "exec/types.h"
#include "exec/io.h"
#include "exec/memory.h"

#include "clib/exec_protos.h"

// extern APTR AllocMem();

/*
******* amiga.lib/CreateExtIO ************************************************
*
*   NAME
*	CreateExtIO -- create an IORequest structure
*
*   SYNOPSIS
*	ioReq = CreateExtIO(port,ioSize);
*
*	struct IORequest *CreateExtIO(struct MsgPort *, ULONG);
*
*   FUNCTION
*	Allocates memory for and initializes a new IO request block
*	of a user-specified number of bytes. The number of bytes
*	MUST be the size of a legal IORequest (or extended IORequest)
*	or very nasty things will happen.
*
*   INPUTS
*	port - an already initialized message port to be used for this IO
*	       request's reply port. If this is NULL this function fails.
*	ioSize - the size of the IO request to be created.
*
*   RESULT
*	ioReq - a new IO Request block, or NULL if there was not enough memory
*
*   EXAMPLE
*	if (ioReq = CreateExtIO(CreatePort(NULL,0),sizeof(struct IOExtTD)))
*
*   SEE ALSO
*	DeleteExtIO(), CreatePort(), exec.library/CreateMsgPort()
*
**********************************************************************
*/

struct IORequest *CreateExtIO(port,ioSize)
struct MsgPort *port;
ULONG ioSize;
{
struct IORequest *ioReq;

    if (port)
    {
        if (ioReq = AllocMem(ioSize,MEMF_CLEAR|MEMF_PUBLIC))
        {
            /* Mark request as inactive (NT_REPLYMSG) */
            ioReq->io_Message.mn_Node.ln_Type = NT_REPLYMSG;
            ioReq->io_Message.mn_Length       = ioSize;
            ioReq->io_Message.mn_ReplyPort    = port;

            return(ioReq);
        }
    }

    return(NULL);
}

/*
******* amiga.lib/DeleteExtIO ************************************************
*
*   NAME
*	DeleteExtIO - return memory allocated for extended IO request
*
*   SYNOPSIS
*	DeleteExtIO(ioReq);
*
*	VOID DeleteExtIO(struct IORequest *);
*
*   FUNCTION
*	Frees up an IO request as allocated by CreateExtIO().
*
*   INPUTS
*	ioReq - the IORequest block to be freed, or NULL.
*
*   SEE ALSO
*	CreateExtIO()
*
******************************************************************************
*/

void DeleteExtIO(ioReq)
struct IORequest *ioReq;
{
    if (ioReq)
    {
        /* try to make it hard to reuse the request by accident */
        ioReq->io_Message.mn_Node.ln_Succ = (void *) -1;
        ioReq->io_Device = (struct Device *) -1;

        FreeMem(ioReq,ioReq->io_Message.mn_Length);
    }
}
