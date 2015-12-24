
/*
 *  WAITMSG.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 *
 *  Wait for a message to be returned, suitable when another task is doing
 *  the ReplyMsg().  NOT suitable for when an interrupt is doing the
 *  ReplyMsg() (must use Disable()/Enable() pair instead of Forbid()/
 *  Permit() if used with interrupts)
 */

#include <exec/types.h>
#include <exec/ports.h>
#include <clib/exec_protos.h>
#include <stdio.h>

typedef struct Message Message;
typedef struct MsgPort MsgPort;

void WaitMsg(Message *);

void
WaitMsg(msg)
Message *msg;
{
    while (msg->mn_Node.ln_Type != NT_REPLYMSG)
	Wait(1 << msg->mn_ReplyPort->mp_SigBit);
    Forbid();
    Remove(&msg->mn_Node);
    Permit();
}

