/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  DOS.C
 */

#include "defs.h"

Prototype void ReturnPacket(DosPacket *packet);
Prototype FileLock *MakeDosFileLock(GEntry *gentry, long mode);
Prototype void FreeDosFileLock(FileLock *flock);
Prototype void *DosAlloc(long bytes);
Prototype void DosFree(void *ptr);
Prototype long RoutePacket(DosPacket *packet);
Prototype GEntry *BLockToGEntry(BPTR lock);
Prototype MsgPort *BLockToGPort(BPTR lock);
Prototype BPTR BLockToGLock(BPTR lock);
Prototype ubyte *BNameToPtr(BPTR bname, short *plen);
Prototype long DoSyncPacket(MsgPort *, long, long, long, long, long);

Prototype BPTR LockPacket(BPTR lock, ubyte *name, short nameLen);
Prototype BPTR DupLockPacket(BPTR lock);
Prototype BPTR CreateDirPacket(BPTR lock, ubyte *name, short nameLen);
Prototype BPTR LockPacketPort(MsgPort *port, ubyte *name, short nameLen);
Prototype void UnLockPacket(BPTR lock);
Prototype BPTR OpenPacket(BPTR lock, ubyte *name, short nameLen, long mode);
Prototype void ClosePacket(BPTR fhb);
Prototype void SetCommentPacket(BPTR, ubyte *,short, ubyte *,short, GEntry *);
Prototype void ExaminePacket(BPTR lock, FileInfoBlock *fib);
Prototype long ReadPacket(BPTR fhb, ubyte *buf, long bytes);
Prototype long WritePacket(BPTR fhb, ubyte *buf, long bytes);

Prototype void makebstr(ubyte *name, short nameLen, ubyte *buf);

void
ReturnPacket(DosPacket *packet)
{
    Message *mess;
    MsgPort *replyPort;

    replyPort		     = packet->dp_Port;
    mess		     = packet->dp_Link;
    packet->dp_Port	     = PktPort;
    mess->mn_Node.ln_Name    = (char *)packet;
    PutMsg(replyPort, mess);
}

FileLock *
MakeDosFileLock(GEntry *gentry, long mode)
{
    FileLock *flock;

    if (flock = DosAlloc(sizeof(FileLock))) {
	flock->fl_Key = (long)gentry;
	flock->fl_Access = mode;
	flock->fl_Task = PktPort;
	flock->fl_Volume = CTOB(Dl);
    }
    return(flock);
}

void
FreeDosFileLock(FileLock *flock)
{
    DosFree(flock);
}

void *
DosAlloc(long bytes)
{
    long *pl;

    bytes += 4;

    if (pl = AllocMem(bytes, MEMF_PUBLIC|MEMF_CLEAR)) {
	*pl = bytes;
    }
    return((void *)(pl + 1));
}

void
DosFree(void *ptr)
{
    long *pl = (long *)ptr - 1;
    FreeMem(pl, *pl);
}

/*
 * Route the specified packet to the underlying filesystem after making
 * appropriate modifications (convert our locks to their locks, etc...),
 * and wait for a response.
 *
 * Returns a negative error code if an error occurs, else returns Res1
 */

long 
RoutePacket(DosPacket *packet)
{
    long r;

    switch(packet->dp_Type) {
    case ACTION_EXAMINE_OBJECT:		/* dp_Arg1 = lock	*/
    case ACTION_EXAMINE_NEXT:
    case ACTION_DELETE_OBJECT:
    case ACTION_CREATE_DIR:
	r = DoSyncPacket(
	    BLockToGPort(packet->dp_Arg1),
	    packet->dp_Type,
	    BLockToGLock(packet->dp_Arg1),
	    packet->dp_Arg2,
	    packet->dp_Arg3,
	    packet->dp_Arg4
	);
	break;
    case ACTION_SET_DATE:		/* dp_arg2 = lock	*/
    case ACTION_SET_PROTECT:
    case ACTION_SET_COMMENT:
	r = DoSyncPacket(
	    BLockToGPort(packet->dp_Arg2),
	    packet->dp_Type,
	    packet->dp_Arg1,
	    BLockToGLock(packet->dp_Arg2),
	    packet->dp_Arg3,
	    packet->dp_Arg4
	);
	break;
    case ACTION_RENAME_OBJECT:		/* dp_Arg1, Arg3 = lock	*/
	r = DoSyncPacket(
	    BLockToGPort(packet->dp_Arg1),
	    packet->dp_Type,
	    BLockToGLock(packet->dp_Arg1),
	    packet->dp_Arg2,
	    BLockToGLock(packet->dp_Arg3),
	    packet->dp_Arg4
	);
	break;
    case ACTION_CHANGE_MODE:		/* dp_Arg2 GHandle	*/
	/* no action */
	r = 0;
	break;
    }
    return(r);
}

ubyte *
BNameToPtr(BPTR bname, short *plen)
{
    ubyte *ptr = BTOC(bname);

    *plen = *ptr;
    return(ptr + 1);
}

GEntry *
BLockToGEntry(BPTR lock)
{
    FileLock *flock;
    GEntry *gentry;

    if (flock = BTOC(lock)) {
	gentry = (GEntry *)flock->fl_Key;
    } else {
	gentry = &GRoot;
    }
    return(gentry);
}

MsgPort *
BLockToGPort(BPTR lock)
{
    FileLock *flock = BTOC(BLockToGEntry(lock)->ge_Lock);
    return(flock->fl_Task);
}

BPTR
BLockToGLock(BPTR lock)
{
    return(BLockToGEntry(lock)->ge_Lock);
}

/*
 * Synchronous Packet
 */

long
DoSyncPacket(MsgPort *dport, long action, long a1, long a2, long a3, long a4)
{
    __aligned StdPacket std;

    clrmem(&std, sizeof(std));

    std.sp_Msg.mn_Node.ln_Name = (char *)&std.sp_Pkt;
    std.sp_Pkt.dp_Link = &std.sp_Msg;
    std.sp_Pkt.dp_Port = AuxPort;
    std.sp_Pkt.dp_Type = action;
    std.sp_Pkt.dp_Arg1 = a1;
    std.sp_Pkt.dp_Arg2 = a2;
    std.sp_Pkt.dp_Arg3 = a3;
    std.sp_Pkt.dp_Arg4 = a4;

    dbprintf(("DoSynchPacket to %08lx type %08lx (%d) args %lx %lx %lx %lx\n",
	dport, std.sp_Pkt.dp_Type, std.sp_Pkt.dp_Type,
	std.sp_Pkt.dp_Arg1,
	std.sp_Pkt.dp_Arg2,
	std.sp_Pkt.dp_Arg3,
	std.sp_Pkt.dp_Arg4
    ));

    PutMsg(dport, &std.sp_Msg);
    WaitPort(AuxPort);
    GetMsg(AuxPort);

    dbprintf(("Res1 %08lx Res2 %08lx\n", 
	std.sp_Pkt.dp_Res1, 
	std.sp_Pkt.dp_Res2
    ));

    if (std.sp_Pkt.dp_Res1 == DOS_FALSE)
	return(-std.sp_Pkt.dp_Res2);
    /*
    if (std.sp_Pkt.dp_Res2 > 0)
	return(-std.sp_Pkt.dp_Res2);
    */
    return(std.sp_Pkt.dp_Res1);
}

/*
 * Manual Packets
 */

BPTR 
LockPacket(BPTR lock, ubyte *name, short nameLen)
{
    FileLock *flock = BTOC(lock);
    long r;
    __aligned char buf[128];

    makebstr(name, nameLen, buf);
    if ((long)buf & 3)
	dbprintf(("BUF NOT ALIGNED\n"));

    r = DoSyncPacket(
	flock->fl_Task, ACTION_LOCATE_OBJECT, 
	lock, CTOB(buf), SHARED_LOCK, 
	0
    );
    if (r <= 0)
	r = 0;
    return(r);
}

BPTR
DupLockPacket(BPTR lock)
{
    FileLock *flock = BTOC(lock);
    long r;

    r = DoSyncPacket(flock->fl_Task, ACTION_COPY_DIR, lock, 0, 0, 0);
    if (r <= 0)
	r = 0;
    return(r);
}

BPTR 
CreateDirPacket(BPTR lock, ubyte *name, short nameLen)
{
    FileLock *flock = BTOC(lock);
    long r;
    __aligned char buf[128];

    makebstr(name, nameLen, buf);

    r = DoSyncPacket(
	flock->fl_Task, ACTION_CREATE_DIR, 
	lock, CTOB(buf), 0, 0
    );
    if (r <= 0)
	r = 0;
    return(r);
}

BPTR 
LockPacketPort(MsgPort *port, ubyte *name, short nameLen)
{
    long r;
    __aligned char buf[128];

    makebstr(name, nameLen, buf);

    r = DoSyncPacket(
	port, ACTION_LOCATE_OBJECT, 
	0, CTOB(buf), SHARED_LOCK, 
	0
    );
    if (r <= 0)
	r = 0;
    return(r);
}

void 
SetCommentPacket(BPTR lck,ubyte *np,short nl,ubyte *cp,short cl,GEntry *gentry)
{
    FileLock *flock = BTOC(lck);
    __aligned char buf[128];
    __aligned char cmm[128];

    makebstr(np, nl, buf);

    if (gentry && (gentry->ge_Flags & GEF_COMPRESSED))
	cmm[0] = sprintf(cmm + 1, "##%08lx##%.*s", gentry->ge_Bytes, cl, cp);
    else
	cmm[0] = sprintf(cmm + 1, "%.*s", cl, cp);

    DoSyncPacket(flock->fl_Task, ACTION_SET_COMMENT,
	0, lck, CTOB(buf), CTOB(cmm)
    );
}

void 
UnLockPacket(BPTR lock)
{
    FileLock *flock = BTOC(lock);

    DoSyncPacket(flock->fl_Task, ACTION_FREE_LOCK, lock, 0, 0, 0);
}

BPTR 
OpenPacket(BPTR lock, ubyte *name, short nameLen, long mode)
{
    FileHandle *fh = DosAlloc(sizeof(FileHandle));
    FileLock *flock = BTOC(lock);
    __aligned char buf[128];
    long r;

    dbprintf(("open: %d '%.*s'\n", mode, nameLen, name));

    makebstr(name, nameLen, buf);

    fh->fh_Port = (MsgPort *)DOS_FALSE;
    fh->fh_Type = flock->fl_Task;
    r = DoSyncPacket(flock->fl_Task, mode, CTOB(fh), lock, CTOB(buf), 0);
    if (r != DOS_TRUE) {
	DosFree(fh);
	return(0);
    } else {
	dbprintf(("open success fh=%08lx arg1=%08lx\n", fh, fh->fh_Arg1));
	return(CTOB(fh));
    }
}

void 
ClosePacket(BPTR fhb)
{
    FileHandle *fh = BTOC(fhb);

    DoSyncPacket(fh->fh_Type, ACTION_END, fh->fh_Arg1, 0, 0, 0);
    DosFree(fh);
}

void 
ExaminePacket(BPTR lock, FileInfoBlock *fib)
{
    FileLock *flock = BTOC(lock);

    DoSyncPacket(flock->fl_Task, ACTION_EXAMINE_OBJECT, lock, CTOB(fib), 0, 0);
}

long 
ReadPacket(BPTR fhb, ubyte *buf, long bytes)
{
    FileHandle *fh = BTOC(fhb);
    long r;

    if (fh->fh_Type == 0)
	return(0);
    r = DoSyncPacket(fh->fh_Type, ACTION_READ, fh->fh_Arg1,(long)buf,bytes,0);
    return(r);
}

long 
WritePacket(BPTR fhb, ubyte *buf, long bytes)
{
    FileHandle *fh = BTOC(fhb);
    long r;

    r = DoSyncPacket(fh->fh_Type, ACTION_WRITE,fh->fh_Arg1,(long)buf,bytes,0);
    return(r);
}

void
makebstr(ubyte *name, short nameLen, ubyte *buf)
{
    *buf = nameLen;
    ++buf;
    movmem(name, buf, nameLen);
    buf[nameLen] = 0;		// necessary for 1.3
}

