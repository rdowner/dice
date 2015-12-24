/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  PLACE.C
 *
 *  PlaceRexxCommand()
 *
 *  Send an AREXX command according to arguments and RexxHostName
 */

#ifdef AMIGA

#include "rexx.h"

void ClearDiceRexxPort(MsgPort *);
void RexxReply(void *, long, char *);

static MsgPort RexxPort;		    /*	master port	*/
static char    *RexxPortName = "REXX";      /*  master port name    */

static short	GlobalFlag;
static short	MasterPortValid;

static List	RexxWIPList;
static List	RexxRIPList;
static List	RexxAuxPortList;
static short	RexxPortInitialized;

short	RexxSigBit;	/*  master signal   */

__autoinit
static void
__dice_rexx_init()
{
    /*
     *	initialize the list that tracks rexx requests sent
     */

    RexxWIPList.lh_Head = (Node *)&RexxWIPList.lh_Tail;
    RexxWIPList.lh_TailPred = (Node *)&RexxWIPList.lh_Head;

    /*
     *	initialize the list that tracks rexx requests received
     */

    RexxRIPList.lh_Head = (Node *)&RexxRIPList.lh_Tail;
    RexxRIPList.lh_TailPred = (Node *)&RexxRIPList.lh_Head;

    /*
     *	initialize the auxillary port list and add our master node to
     *	it.
     */

    RexxAuxPortList.lh_Head = (Node *)&RexxAuxPortList.lh_Tail;
    RexxAuxPortList.lh_TailPred = (Node *)&RexxAuxPortList.lh_Head;

    /*
     *	Get our master signal and create master port
     */

    RexxSigBit = AllocSignal(-1);

    if (RexxHostName)
	CreateDiceRexxPort(NULL, RexxHostName);

    RexxPortInitialized = 1;
}

/*
 *  The exit code can be called at any point... there might be multiple
 *  messages queued, in the queue, or in processing.  We must do the
 *  following items in order:
 *
 *	(1) prevent any further messages from being received
 *	(2) return any messages queued-in or in processing
 *	(3) wait for any outgoing messages to be returned
 */

__autoexit
static void
__dice_rexx_exit()
{
    if (RexxPortInitialized) {
	RexxPortNode *rp;

	/*
	 *  prevent further messages from being received
	 */

	for (rp = (RexxPortNode *)RexxAuxPortList.lh_Head;
	     rp->rp_Node.ln_Succ; rp = (RexxPortNode *)rp->rp_Node.ln_Succ) {
	    if (rp->rp_IsPublic) {
		RemPort(rp->rp_MsgPort);
		rp->rp_IsPublic = 0;
	    }
	}

	/*
	 *  return any messages in processing or queued-in
	 */

	for (rp = (RexxPortNode *)RexxAuxPortList.lh_Head;
	     rp->rp_Node.ln_Succ; rp = (RexxPortNode *)rp->rp_Node.ln_Succ)
	    ClearDiceRexxPort(rp->rp_MsgPort);

	/*
	 *  wait for replies from rexx commands we have sent.  Since we
	 *  have cleared all other messages, no lockout conditions can
	 *  occur at this point.
	 */

	{
	    RexxIPNode *rip;

	    while (rip = (RexxIPNode *)RemHead(&RexxWIPList)) {
		if (rip->rip_Returned == 0) {
		    while (((Message *)rip->rip_RexxMsg)->mn_Node.ln_Type != NT_REPLYMSG)
			Wait(1 << RexxSigBit);
		    Forbid();
		    Remove(&rip->rip_RexxMsg->rm_Node.mn_Node);
		    Permit();
		    rip->rip_Returned = 1;
		}
		ClearRexxMsg(rip->rip_RexxMsg, 1);
		DeleteRexxMsg(rip->rip_RexxMsg);
	    }
	}

	/*
	 *  Delete the ports.  Since we have cleared rp_IsPublic it will
	 *  not try to RemPort() them again.
	 */

	while ((rp = (RexxPortNode *)RexxAuxPortList.lh_Head) != (RexxPortNode *)&RexxAuxPortList.lh_Tail)
	    DeleteDiceRexxPort(rp->rp_MsgPort);

	/*
	 *  Free our master signal
	 */

	FreeSignal(RexxSigBit);
	RexxPortInitialized = 0;
    }
}

long
PlaceRexxCommandDirect(port, remoteName, arg, pres, pec)
MsgPort *port;
char *remoteName;
char *arg;
char **pres;
long *pec;
{
    char *rpn = RexxPortName;
    long r;

    RexxPortName = remoteName;
    r = PlaceRexxCommand(port, arg, pres, pec);
    RexxPortName = rpn;
    return(r);
}

long
PlaceRexxCommand(port, arg, pres, pec)
MsgPort *port;
char *arg;
char **pres;
long *pec;
{
    RexxIPNode rip;
    long rc = -2;

    if (port == NULL) {
	if (MasterPortValid == 0)
	    return(-1);
	port = &RexxPort;
    }

    if (pres)
	*pres = NULL;
    if (pec)
	*pec = 0;


    if (RexxSysBase) {
	rc = -1;

	if (rip.rip_RexxMsg = CreateRexxMsg(port, ((RexxHostName) ? RexxHostName : "UNKNOWN"), port->mp_Node.ln_Name)) {
	    MsgPort *arexxPort;

	    rip.rip_RexxMsg->rm_Node.mn_Node.ln_Name = "REXX";
	    rip.rip_RexxMsg->rm_Action = RXCOMM;
	    ARG0(rip.rip_RexxMsg) = CreateArgstring(arg,strlen(arg));

	    AddHead(&RexxWIPList, &rip.rip_Node);
	    rip.rip_Returned = 0;
	    rip.rip_RexxPort = port;

	    Forbid();
	    if (arexxPort = (MsgPort *)FindPort(RexxPortName)) {
		PutMsg(arexxPort, (Message *)rip.rip_RexxMsg);
		Permit();

		while (rip.rip_Returned == 0) {
		    RexxPortNode *rp;

		    /*
		     *	Since this is the only wait on this signal there
		     *	can be no lockout condition.
		     */

		    Wait(1 << RexxSigBit);
		    for (rp = (RexxPortNode *)RexxAuxPortList.lh_Head;
		         rp->rp_Node.ln_Succ; rp = (RexxPortNode *)(rp->rp_Node.ln_Succ))
			ProcessRexxCommands(rp->rp_MsgPort);
		}
	    } else {
		Permit();
	    }
	    Remove(&rip.rip_Node);

	    /*
	     *	Process result / return code here if rip.rip_Returned == 0
	     *	then we were not able to dispatch the message
	     */

	    if (rip.rip_Returned) {
		if ((rc = rip.rip_RexxMsg->rm_Result1) == 0) {
		    if (pres && rip.rip_RexxMsg->rm_Result2)
			*pres = strdup((char *)rip.rip_RexxMsg->rm_Result2);
		} else {
		    if (pec)
			*pec = (long)rip.rip_RexxMsg->rm_Result2;
		}
	    }
	    ClearRexxMsg(rip.rip_RexxMsg, 1);
	    DeleteRexxMsg(rip.rip_RexxMsg);
	}
    }
    return(rc);
}

void
ProcessRexxCommands(port)
MsgPort *port;
{
    RexxMsg *msg;

    if (port == NULL) {
	RexxPortNode *rp;

	for (rp = (RexxPortNode *)RexxAuxPortList.lh_Head;
	     rp->rp_Node.ln_Succ; rp = (RexxPortNode *)(rp->rp_Node.ln_Succ))
	    ProcessRexxCommands(rp->rp_MsgPort);
	return;
    }

    while (msg = (RexxMsg *)GetMsg(port)) {
	if ((msg->rm_Node.mn_Node.ln_Type == NT_MESSAGE) && IsRexxMsg(msg)) {
	    RexxIPNode rip;
	    char *str = NULL;
	    long rc;

	    rip.rip_RexxMsg = msg;
	    rip.rip_RexxPort = port;
	    AddTail(&RexxRIPList, &rip.rip_Node);
	    rc = DoRexxCommand(msg, port, ARG0(msg), &str);
	    Remove(&rip.rip_Node);
	    RexxReply(msg, rc, str);
	} else {
	    RexxIPNode *rip;

	    for (rip = (RexxIPNode *)RexxWIPList.lh_Head; rip->rip_Node.ln_Succ; rip = (RexxIPNode *)rip->rip_Node.ln_Succ) {
		if (msg == rip->rip_RexxMsg) {
		    rip->rip_Returned = 1;
		    break;
		}
	    }
	}
    }
}

/*
 *  Create a global DICE rexx port
 */

short
CreateGlobalDiceRexxPort(port, name)
MsgPort *port;
char *name;
{
    short r;

    GlobalFlag = 1;
    r = CreateDiceRexxPort(port, name);
    GlobalFlag = 0;

    return(r);
}

/*
 *  Create an application DICE rexx port
 */

short
CreateDiceRexxPort(port, name)
MsgPort *port;
char *name;
{
    RexxPortNode *rp;
    short pno;
    short isPublic;
    short allocSize;

    if (port == NULL) {
	port = &RexxPort;
	MasterPortValid = 1;
    }

    if (name == NULL) {
	name = "PRIVATE-REXX-PORT";
	isPublic = 0;
    } else {
	isPublic = 1;
    }
    allocSize = sizeof(RexxPortNode) + strlen(name) + 4;

    rp = AllocMem(allocSize, MEMF_PUBLIC|MEMF_CLEAR);

    rp->rp_Node.ln_Type = allocSize;
    rp->rp_MsgPort = port;
    rp->rp_IsPublic = isPublic;
    AddTail(&RexxAuxPortList, &rp->rp_Node);

    port->mp_Node.ln_Name = (char *)(rp + 1);

    port->mp_Node.ln_Type = NT_MSGPORT;
    port->mp_Node.ln_Pri = 1;
    port->mp_Flags = PA_SIGNAL;
    port->mp_SigBit = RexxSigBit;
    port->mp_SigTask = FindTask(NULL);
    port->mp_MsgList.lh_Head = (Node *)&port->mp_MsgList.lh_Tail;
    port->mp_MsgList.lh_TailPred = (Node *)&port->mp_MsgList.lh_Head;

    /*
     *	Give the port a real name according to AREXX semantics,
     *	portname.NN (01-99)
     */

    Forbid();

    if (GlobalFlag || isPublic == 0) {
	strcpy(port->mp_Node.ln_Name, name);
	if (isPublic && FindPort(port->mp_Node.ln_Name))
	    pno = -1;
	else
	    pno = 0;
    } else {
	for (pno = 1; pno < 100; ++pno) {
	    sprintf(port->mp_Node.ln_Name, "%s.%02d", name, pno);
	    if (FindPort(port->mp_Node.ln_Name) == NULL)
		break;
	}
	if (pno == 100)
	    pno = -1;
    }

    if (isPublic)
	AddPort(port);

    if (pno < 0)
	DeleteDiceRexxPort(port);
    else if (port == (MsgPort *)&RexxPort && RexxHostName == NULL)
	RexxHostName = port->mp_Node.ln_Name;

    Permit();

    return(pno);
}

/*
 *  Delete a DICE rexx port.  Note that there can be no outgoing messages
 *  in-progress for this port for this command to work properly.
 */

void
DeleteDiceRexxPort(port)
MsgPort *port;
{
    /*
     *	check if valid port, remove RexxPortNode if found
     */

    {
	RexxPortNode *rp;

	for (rp = (RexxPortNode *)RexxAuxPortList.lh_Head;
	     rp->rp_Node.ln_Succ; rp = (RexxPortNode *)rp->rp_Node.ln_Succ) {
	    if (rp->rp_MsgPort == port) {
		Remove(&rp->rp_Node);
		if (rp->rp_IsPublic)
		    RemPort(port);
		FreeMem(rp, rp->rp_Node.ln_Type);
		break;
	    }
	}
	if (rp == NULL)
	    return;
    }

    ClearDiceRexxPort(port);

    /*
     *	clear port info
     */

    clrmem(port, sizeof(MsgPort));

    if (port == (MsgPort *)&RexxPort)
	MasterPortValid = 0;
}

void
ClearDiceRexxPort(port)
MsgPort *port;
{
    /*
     *	kill any received messages that are in progress
     */

    {
	RexxIPNode *rip;
	RexxIPNode *rip_next;

	for (rip = (RexxIPNode *)RexxRIPList.lh_Head; rip->rip_Node.ln_Succ; rip = rip_next) {
	    rip_next = (RexxIPNode *)rip->rip_Node.ln_Succ;
	    if (rip->rip_RexxPort == port) {
		Remove(&rip->rip_Node);
		RexxReply(rip->rip_RexxMsg, 30, NULL);
	    }
	}
    }


    /*
     *	clear out any messages pending on the port
     */

    {
	RexxMsg *msg;

	while (msg = (RexxMsg *)GetMsg(port)) {
	    if ((msg->rm_Node.mn_Node.ln_Type == NT_MESSAGE) && IsRexxMsg(msg)) {
		RexxReply(msg, 30, NULL);
	    } else {
		RexxIPNode *rip;

		for (rip = (RexxIPNode *)RexxWIPList.lh_Head;
		     rip->rip_Node.ln_Succ; rip = (RexxIPNode *)rip->rip_Node.ln_Succ) {
		    if (msg == rip->rip_RexxMsg) {
			rip->rip_Returned = 1;
			break;
		    }
		}
	    }
	}
    }
}

void
RexxReply(vmsg, res1, str)
void *vmsg;
long res1;
char *str;
{
    RexxMsg *msg = vmsg;

    if (msg->rm_Result1 = res1) {
	msg->rm_Result2 = 0;
    } else {
	if (str)
	    msg->rm_Result2 = (long)CreateArgstring(str, strlen(str));
	else
	    msg->rm_Result2 = 0;
    }
    ReplyMsg((Message *)msg);
}

/*
 *  Obtain the fully qualified name of an application port (name.xx) and
 *  return an integer.	Returns -1 if the port does not contain a .xx
 *  extension.
 */

int
GetDiceRexxPortSlot(port, pname)
MsgPort *port;
char **pname;
{
    int slotNo = -1;
    char *ptr;

    if (port == NULL)
	port = &RexxPort;
    if (ptr = strrchr(port->mp_Node.ln_Name, '.')) {
	char *tmp;

	slotNo = strtol(ptr + 1, &tmp, 10);  /* must be base 10 re: '.01'... */
	if (tmp == ptr)
	    slotNo = -1;
    }
    if (pname)
	*pname = port->mp_Node.ln_Name;
    return(slotNo);
}

#endif

