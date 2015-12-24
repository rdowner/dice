/***********************************************************************
*
*	Exec Support Functions -- Standard IO Requests
*
***********************************************************************/

#include "exec/types.h"
#include "exec/ports.h"
#include "exec/io.h"

extern struct IORequest *CreateExtIO(struct MsgPort *, ULONG);
extern VOID DeleteExtIO(struct IORequest *);

/*
******* amiga.lib/CreateStdIO ************************************************
*
*   NAME
*	CreateStdIO -- create an IOStdReq structure
*
*   SYNOPSIS
*	ioReq = CreateStdIO(port);
*
*	struct IOStdReq *CreateStdIO(struct MsgPort *)
*
*   FUNCTION
*	Allocates memory for and initializes a new IOStdReq structure.
*
*   INPUTS
*	port - an already initialized message port to be used for this IO
*	       request's reply port. If this is NULL this function fails.
*
*   RESULT
*	ioReq - a new IOStdReq structure, or NULL if there was not enough
*		memory
*
*   SEE ALSO
*	DeleteStdIO(), CreateExtIO(), exec.library/CreateIORequest()
*
******************************************************************************
*/

struct IOStdReq *
CreateStdIO( ioReplyPort )
    struct MsgPort *ioReplyPort;
{
    return( (struct IOStdReq *)
	CreateExtIO( ioReplyPort, sizeof( struct IOStdReq ) ) );
}


/*
******* amiga.lib/DeleteStdIO ************************************************
*
*   NAME
*	DeleteStdIO - return memory allocated for IOStdReq
*
*   SYNOPSIS
*	DeleteStdIO(ioReq);
*
*	VOID DeleteStdIO(struct IOStdReq *);
*
*   FUNCTION
*	Frees up an IOStdReq as allocated by CreateStdIO().
*
*   INPUTS
*	ioReq - the IORequest block to be freed, or NULL.
*
*   SEE ALSO
*	CreateStdIO(), DeleteExtIO(), exec.library/CreateIORequest()
*
******************************************************************************
*/

DeleteStdIO( ioStdReq )
    struct IOStdReq *ioStdReq;
{
    DeleteExtIO( (struct IORequest *)ioStdReq );
    return 0;
}
