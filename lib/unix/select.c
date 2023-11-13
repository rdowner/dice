
/*
 *  UNIX/SELECT.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/ports.h>
#include <dos/dos.h>
#include <sys/types.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <lib/misc.h>
#include <lib/requestfh.h>
#include <lib/waitmsg.h>
#include <devices/timer.h>

#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/alib_protos.h>
#include <lists.h>

typedef struct MsgPort  MsgPort;
typedef struct Message  Message;
typedef struct Node     Node;
typedef struct List     List;
typedef struct timerequest Iot;

typedef struct ReqNode {
    Node    rn_Node;
    short   rn_Fd;
    long    rn_Fh;
    long    rn_Flags;
    Message rn_Message;
} ReqNode;

#define RNF_RFDS        0x0001
#define RNF_WFDS        0x0002
#define RNF_RETURNED    0x0100

static List FreeList = { (Node *)&FreeList.lh_Tail, NULL, (Node *)&FreeList.lh_Head };
static Iot      Iot0;
static MsgPort  IoSink;

void QueueMessage(List *, _IOFDS *, int, long, long);

void
_select_exit()
{
    if (Iot0.tr_node.io_Device) {
        CloseDevice((void *)&Iot0);
        Iot0.tr_node.io_Device = NULL;
    }
}

int
select(setSize, rfds, wfds, xfds, tv)
int setSize;
fd_set *rfds;
fd_set *wfds;
fd_set *xfds;
struct timeval *tv;
{
    List    actList;
    short   i, j;
    short   sigInt = 0;
    short   iotIp = 0;

    if (IoSink.mp_Flags == 0) {
        IoSink.mp_Node.ln_Type = NT_MSGPORT;
        IoSink.mp_Flags = PA_SIGNAL;
        IoSink.mp_SigBit= SIGB_SINGLE;
        IoSink.mp_SigTask = FindTask(NULL);
        NewList(&IoSink.mp_MsgList);

        Iot0.tr_node.io_Message.mn_ReplyPort = &IoSink;
        Iot0.tr_node.io_Command = TR_ADDREQUEST;
        OpenDevice("timer.device", UNIT_VBLANK, (void *)&Iot0, 0);
        atexit(_select_exit);
    }

    NewList(&actList);

    for (i = 0; i < setSize; i += sizeof(fd_mask) * 8) {
        fd_mask mask;
        _IOFDS *d;


        if (rfds && (mask = rfds->fds_bits[i])) {
            for (j = 0; j < sizeof(fd_mask) * 8; ++j) {
                if ((mask & (1 << j)) && (d = __getfh(i + j))) {
                    QueueMessage(&actList, d, i + j, FREQ_RPEND, RNF_RFDS);
                }
            }
        }
        if (wfds && (mask = wfds->fds_bits[i])) {
            for (j = 0; j < sizeof(fd_mask) * 8; ++j) {
                if ((mask & (1 << j)) && (d = __getfh(i + j))) {
                    QueueMessage(&actList, d, i + j, FREQ_WAVAIL, RNF_WFDS);
                }
            }
        }
    }

    /*
     *  nothing was queued!
     */

    if (GetHead(&actList) == NULL) {
#ifdef DEBUG
        puts("ACTLIST EMPTY");
#endif
        errno = 0;
        return(-1);
    }

    /*
     *  start timeout
     */

    if (tv && (tv->tv_secs || tv->tv_micro)) {
#ifdef DEBUG
        puts("START TIMEOUT");
#endif
        Iot0.tr_time.tv_secs = tv->tv_secs;
        Iot0.tr_time.tv_micro = tv->tv_micro;
        SendIO((void *)&Iot0);
        iotIp = 1;
    }

    /*
     *  wait for event(s)
     */

    i = 0;

    for (;;) {
        {
            ReqNode *rn;

            while (rn = (struct ReqNode *)GetMsg(&IoSink)) {
                if ((char *)rn == (char *)&Iot0) {
                    tv->tv_secs = 0;
                    tv->tv_micro = 0;
                    iotIp = 0;
#ifdef DEBUG
                    puts("TIMEOUT");
#endif
                    continue;
                }
#ifdef DEBUG
                puts("REQ RETURNED");
#endif
                rn = (ReqNode *)((char *)rn - offsetof(ReqNode, rn_Message));
                rn->rn_Flags |= RNF_RETURNED;
                ++i;
            }
        }
        /*
         *  if tv was not NULL and was 0, break out
         */
        if (tv && tv->tv_secs == 0 && tv->tv_micro == 0) {
#ifdef DEBUG
            puts("TIME ENDED");
#endif
            break;
        }
        if (i) {
#ifdef DEBUG
            puts("DESCRIPT FOUND");
#endif
            break;
        }
        {
            long mask = Wait(SIGF_SINGLE | SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_D);
            if (mask & (SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_D)) {
                sigInt = 1;
                break;
            }
        }
    }

    if (iotIp) {
        AbortIO((void *)&Iot0);
        WaitIO((void *)&Iot0);
    }

    /*
     *  abort remaining requests, clearing those that have not been returned
     *  (no event pending)
     */

    i = 0;

    {
        ReqNode *rn;

        while (rn = (struct ReqNode *)RemHead(&actList)) {
            if (rn->rn_Message.mn_Node.ln_Type == NT_MESSAGE) {
                if (rn->rn_Flags & RNF_RFDS)
                    FD_CLR(rn->rn_Fd, rfds);
                if (rn->rn_Flags & RNF_WFDS)
                    FD_CLR(rn->rn_Fd, wfds);
                RequestFH(rn->rn_Fh, &rn->rn_Message, FREQ_ABORT);
                WaitMsg(&rn->rn_Message);
            } else {
                if ((rn->rn_Flags & RNF_RETURNED) == 0)
                    WaitMsg(&rn->rn_Message);
                ++i;
            }
            AddTail(&FreeList, &rn->rn_Node);
        }
    }

    /*
     *  SIGINT?
     */

    if (sigInt) {
        raise(SIGINT);
        if (i == 0) {
            i = -1;
            errno = EINTR;
        }
    } else if (i == 0) {
        i = -1;
        errno = EWOULDBLOCK;
    }
    return(i);
}

void
QueueMessage(list, d, fd, how, flags)
List *list;
_IOFDS *d;
int fd;
long how;
long flags;
{
    ReqNode *rn;

    if ((rn = (struct ReqNode *)RemHead(&FreeList)) == NULL)
        rn = malloc(sizeof(ReqNode));
    if (rn) {
        rn->rn_Message.mn_ReplyPort = &IoSink;
        rn->rn_Flags = flags;
        rn->rn_Fh = d->fd_Fh;
        rn->rn_Fd = fd;

        if (RequestFH(d->fd_Fh, &rn->rn_Message, how)) {
#ifdef DEBUG
            printf("ADDED %d\n", fd);
#endif
            AddTail(list, &rn->rn_Node);
        } else {
#ifdef DEBUG
            printf("NOTSUPPORTED: FD %d\n", fd);
#endif
            AddTail(&FreeList, &rn->rn_Node);
        }
    }
}

