
/*
 *  CreateExtIO.C   (1.3/2.0) (for 2.0 only apps use CreateIORequest())
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 *
 */

#include <exec/types.h>
#include <exec/ports.h>
#include <exec/memory.h>
#include <exec/io.h>
#ifdef INCLUDE_VERSION      /*  2.0 */
#include <clib/exec_protos.h>
#include <clib/alib_protos.h>
#else
extern void *AllocMem(long, long);
extern void FreeMem(void *, long);
#endif

#ifndef HYPER
#define HYPER
#endif

typedef struct MsgPort  MsgPort;
typedef struct IORequest IORequest;
typedef struct IOStdReq  IOStdReq;

IORequest *
HYPER ## CreateExtIO(replyPort, size)
MsgPort *replyPort;
long size;
{
    IORequest *io = NULL;

    if (replyPort) {
        if (io = AllocMem(size, MEMF_PUBLIC | MEMF_CLEAR)) {
            io->io_Message.mn_ReplyPort = replyPort;
            io->io_Message.mn_Length = size;
            io->io_Message.mn_Node.ln_Type = NT_REPLYMSG;
        }
    }
    return(io);
}

void
DeleteExtIO(io)
IORequest *io;
{
    if (io) {
        long bad = -1;
        io->io_Message.mn_Node.ln_Succ = (void *)bad;
        io->io_Device = (void *)bad;
        FreeMem(io, io->io_Message.mn_Length);
    }
}

