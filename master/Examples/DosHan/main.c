
/*
 *  MAIN.C
 *
 *  MAIN FOR DOS HANDLER TEST PROGRAM FOR DICE
 */

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/ports.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/filehandler.h>

#include <clib/exec_protos.h>
#include <clib/alib_protos.h>
#include <lists.h>

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#define DOS_TRUE    -1
#define DOS_FALSE   0

#define BTOC(bptr)  ((void *)((long)(bptr) << 2))
#define CTOB(cptr)  ((BPTR)(((long)cptr) >> 2))

typedef struct DosPacket    DosPacket;
typedef struct FileHandle   FileHandle;
typedef struct DeviceNode   DeviceNode;
typedef struct Process	    Process;
typedef struct Node	    Node;
typedef struct List	    List;
typedef struct MsgPort	    MsgPort;
typedef struct Message	    Message;

typedef struct XHandle {
    Node    xh_Node;
    long    xh_XHLen;
    long    xh_Refs;
    long    xh_Flags;
    List    xh_List;
} XHandle;

#define XHF_EOF     0x0001

typedef struct XNode {
    Node    xn_Node;
    char    *xn_Buf;
    long    xn_Offset;
    long    xn_Length;
} XNode;

void returnpacket(DosPacket *);
void Initialize(void);
void UnInitialize(void);


List	HanList;
DeviceNode *DevNode;
MsgPort    *PktPort;

/*
 *  Note that we use the _main entry point.  Also notice that we do not
 *  need to open any libraries.. they are openned for us via DICE's
 *  unique auto-library-open ability.
 */

void
_main()
{
    DosPacket  *packet;

    {
	Process *proc = (struct Process *)FindTask(NULL);
	PktPort = &proc->pr_MsgPort;
    }
    NewList(&HanList);
    Initialize();

    /*
     *	Main Loop
     */

    for (;;) {
	{
	    Message *msg;

	    while ((msg = GetMsg(PktPort)) == NULL)
		Wait(1 << PktPort->mp_SigBit);
	    packet = (DosPacket *)msg->mn_Node.ln_Name;
	}

	/*
	 *  default return value
	 */

	packet->dp_Res1 = DOS_TRUE;
	packet->dp_Res2 = 0;

	/*
	 *  switch on packet
	 */

	switch(packet->dp_Type) {
	case ACTION_DIE:	    /*	??? */
	    break;
	case ACTION_FINDUPDATE:     /*	FileHandle,Lock,Name	    Bool	*/
	case ACTION_FINDINPUT:	    /*	FileHandle,Lock,Name	    Bool	*/
	case ACTION_FINDOUTPUT:     /*	FileHandle,Lock,Name	    Bool	*/
	    {
		FileHandle *fh = BTOC(packet->dp_Arg1);
		XHandle *xh;

		{
		    unsigned char *base = BTOC(packet->dp_Arg3);
		    int len = *base;
		    char buf[128];

		    if (len >= sizeof(buf))
			len = sizeof(buf) - 1;

		    strncpy(buf, base + 1, len);
		    buf[len] = 0;

		    if ((xh = (XHandle *)FindName(&HanList, buf)) == NULL) {
			xh = AllocMem(sizeof(XHandle) + len + 1, MEMF_PUBLIC | MEMF_CLEAR);
			xh->xh_XHLen = sizeof(XHandle) + len + 1;
			xh->xh_Node.ln_Name = (char *)(xh + 1);
			movmem(buf, xh->xh_Node.ln_Name, len + 1);
			NewList(&xh->xh_List);
			AddTail(&HanList, &xh->xh_Node);
		    }
		    ++xh->xh_Refs;
		}
		fh->fh_Arg1 = (ULONG)xh;
		fh->fh_Port = (MsgPort *)DOS_TRUE;
	    }
	    break;
	case ACTION_READ:	    /*	FHArg1,CPTRBuffer,Length    ActLength	*/
	    /*
	     *	reading is straightforward except for handling EOF ... we
	     *	must guarentee a return value of 0 (no bytes left) before
	     *	beginning to return EOFs (-1's).  If we return a negative
	     *	number right off programs like COPY will assume a failure
	     *	(if TEST: is the source) and delete the destination file
	     */

	    {
		XHandle *xh = (XHandle *)packet->dp_Arg1;
		long bytes;

		packet->dp_Res1 = 0;
		while ((bytes = packet->dp_Arg3 - packet->dp_Res1) > 0) {
		    XNode *xn = (XNode *)RemHead(&xh->xh_List);

		    if (xn == NULL)
			break;

		    if (bytes > xn->xn_Length - xn->xn_Offset)
			bytes = xn->xn_Length - xn->xn_Offset;
		    movmem(xn->xn_Buf + xn->xn_Offset, (char *)packet->dp_Arg2 + packet->dp_Res1, bytes);

		    xn->xn_Offset += bytes;
		    packet->dp_Res1 += bytes;
		    if (xn->xn_Offset == xn->xn_Length) {
			FreeMem(xn->xn_Buf, xn->xn_Length);
			FreeMem(xn, sizeof(XNode));
		    } else {
			AddHead(&xh->xh_List, &xn->xn_Node);
		    }
		}
		if (packet->dp_Res1 == 0 && GetHead(&xh->xh_List) == NULL) {
		    if (xh->xh_Flags & XHF_EOF)
			packet->dp_Res1 = -1;	/*  EOF */
		    xh->xh_Flags |= XHF_EOF;
		}
	    }
	    break;
	case ACTION_WRITE:	    /*	FHArg1,CPTRBuffer,Length    ActLength	*/
	    {
		XHandle *xh = (XHandle *)packet->dp_Arg1;
		XNode *xn;
		long bytes = packet->dp_Arg3;

		packet->dp_Res1 = -1;
		if (xn = AllocMem(sizeof(XNode), MEMF_PUBLIC|MEMF_CLEAR)) {
		    if (xn->xn_Buf = AllocMem(bytes, MEMF_PUBLIC)) {
			movmem((char *)packet->dp_Arg2, xn->xn_Buf, bytes);
			xn->xn_Length = bytes;
			packet->dp_Res1 = bytes;
			AddTail(&xh->xh_List, &xn->xn_Node);
			xh->xh_Flags &= ~XHF_EOF;
		    } else {
			FreeMem(xn, sizeof(XNode));
			packet->dp_Res2 = ERROR_NO_FREE_STORE;
		    }
		} else {
		    packet->dp_Res2 = ERROR_NO_FREE_STORE;
		}
	    }
	    break;
	case ACTION_END:	    /*	FHArg1			    Bool:TRUE	*/
	    {
		XHandle *xh = (XHandle *)packet->dp_Arg1;

		if (--xh->xh_Refs == 0 && GetHead(&xh->xh_List) == NULL) {
		    Remove(&xh->xh_Node);
		    FreeMem(xh, xh->xh_XHLen);
		}
		break;
	    }
	    break;
	default:
	    packet->dp_Res2 = ERROR_ACTION_NOT_KNOWN;
	    break;
	}
	if (packet) {
	    if (packet->dp_Res2)
		packet->dp_Res1 = DOS_FALSE;
	    returnpacket(packet);
	}
    }

    /*
     *	no reached
     */

    UnInitialize();
    _exit(0);
}


/*
 *  PACKET ROUTINES.	Dos Packets are in a rather strange format as you
 *  can see by this and how the PACKET structure is extracted in the
 *  GetMsg() of the main routine.
 */

void
returnpacket(packet)
DosPacket *packet;
{
    Message *mess;
    MsgPort *replyPort;

    replyPort		     = packet->dp_Port;
    mess		     = packet->dp_Link;
    packet->dp_Port	     = PktPort;
    mess->mn_Node.ln_Name    = (char *)packet;
    PutMsg(replyPort, mess);
}

/*
 *  During initialization DOS sends us a packet and sets our dn_SegList
 *  pointer.  If we set our dn_Task pointer than every Open's go to the
 *  same handler (this one).  If we set dn_Task to NULL, every Open()
 *  will create a NEW instance of this process via the seglist, meaning
 *  our process must be reentrant (i.e. -r option).
 *
 *  note: dn_Task points to the MESSAGE PORT portion of the process
 *  (or your own custom message port).
 *
 *  If we clear the SegList then we also force DOS to reload our process
 *  from disk, but we also need some way of then UnLoadSeg()ing it ourselves,
 *  which we CANNOT do from this process since it rips our code out from
 *  under us.
 */

void
Initialize()
{
    DeviceNode *dn;
    Process *proc = (Process *)FindTask(NULL);
    DosPacket *packet;

    /*
     *	Handle initial message.
     */

    {
	Message *msg;

	WaitPort(PktPort);
	msg = GetMsg(PktPort);
	packet = (DosPacket *)msg->mn_Node.ln_Name;
    }

    {
	DevNode = dn = BTOC(packet->dp_Arg3);

	dn->dn_Task = PktPort;
    }
    packet->dp_Res1 = DOS_TRUE;
    packet->dp_Res2 = 0;
    returnpacket(packet);
}

void
UnInitialize(void)
{
    {
	DeviceNode *dn = DevNode;

	dn->dn_Task = NULL;
	/* dn->dn_SegList = NULL; */
    }
}

