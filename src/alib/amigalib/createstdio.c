
/*
 *  CreateStdIO.C   (1.3/2.0) (for 2.0 only apps use CreateIORequest())
 */

#include <exec/types.h>
#include <exec/ports.h>
#include <exec/memory.h>
#include <exec/io.h>
#ifdef INCLUDE_VERSION      /*  2.0 */
#include <clib/exec_protos.h>
#include <clib/alib_protos.h>
#else
extern void *CreateExtIO(void *, long);
extern void DeleteExtIO(void *);
#endif

typedef struct MsgPort  MsgPort;
typedef struct IORequest IORequest;
typedef struct IOStdReq  IOStdReq;

#ifndef HYPER
#define HYPER
#endif

IOStdReq *
HYPER ## CreateStdIO(replyPort)
MsgPort *replyPort;
{
    return((IOStdReq *)CreateExtIO(replyPort, sizeof(IOStdReq)));
}

void
DeleteStdIO(io)
IOStdReq *io;
{
    DeleteExtIO((IORequest *)io);
}

