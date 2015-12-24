

/*
 *  RequestFH.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 *
 *  error = RequestFH(fh, msg, how)
 *
 *  This sends a specialized DOS packet to support non-blocking IO /
 *  return-msg on data-ready or write space available.
 */

#define SysBase_DECLARED

#include <exec/types.h>
#include <exec/execbase.h>
#include <exec/memory.h>
#include <libraries/dos.h>
#include <libraries/dosextens.h>
#include <string.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>

#define ACTION_REQUEST	5000

#define BTOC(bptr)  ((void *)((long)bptr << 2))
#define CTOB(cptr)  ((BPTR)((long)cptr >> 2))

typedef void *Pointer;

typedef struct StandardPacket StdPacket;
typedef struct Process	      Process;
typedef struct Message	      Message;
typedef struct FileHandle     FileHandle;

extern struct ExecBase *SysBase;

int
RequestFH(bfh, msg, req)
BPTR bfh;
Message *msg;
long req;
{
    __aligned StdPacket sp;
    FileHandle *fh = BTOC(bfh);

    sp.sp_Msg.mn_Node.ln_Name = (char *)&(sp.sp_Pkt);
    sp.sp_Pkt.dp_Link = &sp.sp_Msg;
    sp.sp_Pkt.dp_Port = &((Process *)SysBase->ThisTask)->pr_MsgPort;
    sp.sp_Pkt.dp_Type = ACTION_REQUEST;
    sp.sp_Pkt.dp_Arg1 = fh->fh_Arg1;
    sp.sp_Pkt.dp_Arg2 = (long)msg;
    sp.sp_Pkt.dp_Arg3 = req;
    sp.sp_Pkt.dp_Arg4 = 0;	/*  reserved for future use */
    PutMsg(fh->fh_Type, &sp.sp_Msg);
    WaitPort(&((Process *)SysBase->ThisTask)->pr_MsgPort);
    GetMsg(&((Process *)SysBase->ThisTask)->pr_MsgPort);
    return(sp.sp_Pkt.dp_Res1);
}

