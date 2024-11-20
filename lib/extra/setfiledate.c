

/*
 *  SETFILEDATE.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 *
 *  BOOL = SetFileDate(filename, date)
 *
 *  Note, this call works under 1.3 and is equivalent to the 2.0
 *  dos.library call.  We cannot enable dos pragmas here because they
 *  will interfere with this routine's register conventions.
 */

#include <exec/types.h>
#include <exec/memory.h>
#include <libraries/dos.h>
#include <libraries/dosextens.h>
#include <string.h>
#include <clib/exec_protos.h>
#undef __DICE_INLINE    /*  problem w/ inline SetFileDate() */
#include <clib/dos_protos.h>

#define BTOC(bptr)  ((void *)((long)bptr << 2))
#define CTOB(cptr)  ((BPTR)((long)cptr >> 2))

typedef void *Pointer;

#ifndef ACTION_SET_DATE
#define ACTION_SET_DATE 34
#endif

typedef struct StandardPacket STDPKT;
typedef struct Process        PROC;
typedef struct DateStamp      DATESTAMP;
typedef struct FileLock       LOCK;
typedef struct Message        MSG;

LONG SetFileDate( CONST_STRPTR file, CONST struct DateStamp *date )
{
    STDPKT *packet;
    char   *buf;
    PROC   *proc;
    long   result;
    BPTR   lock;
    BPTR   flock = Lock(file, SHARED_LOCK);
    short  i;
    char *ptr = file;

    {
        if (flock == NULL)
            return(NULL);
        lock = ParentDir(flock);
        UnLock(flock);
        if (!lock)
            return(NULL);
        for (i = strlen(ptr) - 1; i >= 0; --i) {
            if (ptr[i] == '/' || ptr[i] == ':')
                break;
        }
        file += i + 1;
    }
    proc   = (PROC *)FindTask(NULL);
    packet = (STDPKT   *)AllocMem(sizeof(STDPKT), MEMF_CLEAR|MEMF_PUBLIC);
    buf = AllocMem(strlen(file)+2, MEMF_PUBLIC);
    strcpy(buf+1,file);
    buf[0] = strlen(file);

    packet->sp_Msg.mn_Node.ln_Name = (char *)&(packet->sp_Pkt);
    packet->sp_Pkt.dp_Link = &packet->sp_Msg;
    packet->sp_Pkt.dp_Port = &proc->pr_MsgPort;
    packet->sp_Pkt.dp_Type = ACTION_SET_DATE;
    packet->sp_Pkt.dp_Arg1 = NULL;
    packet->sp_Pkt.dp_Arg2 = (long)lock;        /*  lock on parent dir of file  */
    packet->sp_Pkt.dp_Arg3 = (long)CTOB(buf);   /*  BPTR to BSTR of file name   */
    packet->sp_Pkt.dp_Arg4 = (long)date;        /*  APTR to datestamp structure */
    PutMsg(((LOCK *)BTOC(lock))->fl_Task, (MSG *)packet);
    WaitPort(&proc->pr_MsgPort);
    GetMsg(&proc->pr_MsgPort);
    result = packet->sp_Pkt.dp_Res1;
    FreeMem(packet, sizeof(STDPKT));
    FreeMem(buf, strlen(file)+2);
    UnLock(lock);
    return(result);
}

