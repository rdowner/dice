/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  FMS.C
 *
 *  File Disk Device (fmsdisk.device)
 *
 *  Simulates a trackdisk by using a large file to hold the 'blocks'.
 */

#include "defs.h"

Prototype DevCall NDev *DevInit(register __a0 APTR);
Prototype DevCall NDev *DevOpen(register __d0 long, register __a1 IOB *, register __d1 long);
Prototype APTR DevExpunge(void);
Prototype APTR DevClose(register __a1 IOB *);
Prototype APTR DevReserved(void);
Prototype void DevBeginIO(register __a1 IOB *iob);
Prototype void DevAbortIO(register __a1 IOB *iob);
Prototype void SynchroMsg(UWORD, NDUnit *);
Prototype __geta4 void CoProc(void);
Prototype void ExtendSize(NDUnit *, long);
Prototype void FlushCache(void);
Prototype void GetUnitName(int, char *);

extern struct Library *SysBase;
struct DosLibrary *DOSBase      = NULL;

NDev *DevBase;
APTR DevSegment;
PORT *FProc;
PORT FPort;

#ifdef DEBUG
long DbFH;
#endif

NDUnit *CacheUnit;
long    CacheLen;
char    CacheBuf[32768];    /*  32K buffer  */

/*
 *  Init, segment in A0 (registered args), arg must be declared as a pointer
 */

DevCall
NDev *
DevInit(register __a0 APTR seg)
{
    static func_ptr DevVectors[] = {
        (func_ptr)DevOpen,
        (func_ptr)DevClose,
        (func_ptr)DevExpunge,
        (func_ptr)DevReserved,
        (func_ptr)DevBeginIO,
        (func_ptr)DevAbortIO,
        (func_ptr)-1
    };
    NDev *db;

    DevBase = db = (NDev *)MakeLibrary((long **)DevVectors,NULL,NULL,sizeof(NDev),NULL);
    db->Lib.lib_Node.ln_Type = NT_DEVICE;
    db->Lib.lib_Node.ln_Name = DeviceName;
    db->Lib.lib_Flags = LIBF_CHANGED|LIBF_SUMUSED;
    db->Lib.lib_Version = 1;
    db->Lib.lib_IdString= (APTR)IdString;

    DevSegment = seg;
    AddDevice((DEV *)db);
    return(db);
}

/*
 *  open device
 *
 *      unitnum:    D0
 *      iob:        A1
 *      flags:      D1
 *
 *  have dummy pointer before iob so iob gets assigned A1 instead of A0
 *  for registered args interface.
 */

DevCall
NDev *
DevOpen(register __d0 long unitnum,
        register __a1 IOB *iob,
        register __d1 long flags
       )
{
    NDev *nd = DevBase;
    NDUnit *unit = &nd->Unit[unitnum];

    if (++nd->Lib.lib_OpenCnt == 1) {
        FProc = CreateProc("FMS-Dummy", 0, (long)DUMmySeg >> 2, 8192);
        FPort.mp_SigBit = SIGBREAKB_CTRL_D;     /*  port init */
        FPort.mp_SigTask= FProc->mp_SigTask;
        FPort.mp_Flags = PA_SIGNAL;
        NewList(&FPort.mp_MsgList);
    }

    if (++unit->OpenCnt == 1)
        SynchroMsg(CMD_OPENUNIT, unit);

    nd->Lib.lib_Flags &= ~LIBF_DELEXP;
    iob->io_Unit = (struct Unit *)unit;
    iob->io_Error = 0;
    return(nd);
}

/*
 *  expunge device, no arguments
 */

APTR
DevExpunge(void)
{
    NDev *nd = DevBase;
    APTR dseg = DevSegment;

    if (dseg == NULL)
        Alert(24);
    if (nd->Lib.lib_OpenCnt) {
        nd->Lib.lib_Flags |= LIBF_DELEXP;
        return(NULL);
    }
    Remove((NODE *)nd);
    FreeMem((char *)nd - nd->Lib.lib_NegSize, nd->Lib.lib_NegSize + nd->Lib.lib_PosSize);
    ADevExpunge();
    return(dseg);
}

/*
 *  device close.  Dummy pointer before iob so iob gets assigned A1
 *
 *      iob:    A1
 */

DevCall
APTR
DevClose(register __a1 IOB *iob)
{
    NDev *nd = DevBase;

    {
        NDUnit *unit = (NDUnit *)iob->io_Unit;
        if (unit->OpenCnt && --unit->OpenCnt == 0)
            SynchroMsg(CMD_CLOSEUNIT, unit);
    }

    if (nd->Lib.lib_OpenCnt && --nd->Lib.lib_OpenCnt)
        return(NULL);
    if (FProc) {
        SynchroMsg(CMD_KILLPROC, NULL);
        FProc = NULL;
    }
    if (nd->Lib.lib_Flags & LIBF_DELEXP)
        return(DevExpunge());
    /*
     *  close down resources
     */
    return(NULL);
}

DevCall
APTR
DevReserved(void)
{
    return((APTR)0);
}

DevCall
void
DevBeginIO(register __a1 IOB *iob)
{
    /*NDev *nd = DevBase;*/

    iob->io_Error = 0;
    iob->io_Actual = 0;

    switch(iob->io_Command & ~TDF_EXTCOM) {
    case CMD_INVALID:
        iob->io_Error = IOERR_NOCMD;
        break;
    case CMD_RESET:
        break;
    case CMD_READ:
        PutMsg(&FPort, &iob->io_Message);
        iob->io_Flags &= ~IOF_QUICK;    /*  not quick */
        iob = NULL;
        break;
    case CMD_WRITE:
        PutMsg(&FPort, &iob->io_Message);
        iob->io_Flags &= ~IOF_QUICK;    /*  not quick */
        iob = NULL;
        break;
    case CMD_UPDATE:
        PutMsg(&FPort, &iob->io_Message);
        iob->io_Flags &= ~IOF_QUICK;    /*  not quick */
        iob = NULL;
        break;
    case CMD_CLEAR:
        break;
    case CMD_STOP:
        break;
    case CMD_START:
        break;
    case CMD_FLUSH:
        break;
    case TD_MOTOR:          /*  motor,  no action   */
    case TD_SEEK:           /*  seek,   no action   */
        break;
    case TD_FORMAT:         /*  format              */
        PutMsg(&FPort, &iob->io_Message);
        iob->io_Flags &= ~IOF_QUICK;    /*  not quick */
        iob = NULL;
        break;
    case TD_REMOVE:         /*  not supported       */
        iob->io_Error = IOERR_NOCMD;
        break;
    case TD_CHANGENUM:      /*  change count never changes  */
        iob->io_Actual = 1;
        break;
    case TD_CHANGESTATE:    /*  0=disk in drive     */
        iob->io_Actual = 0;
        break;
    case TD_PROTSTATUS:     /*  io_Actual -> 0 (rw) */
        iob->io_Actual = 0;
        break;
    case TD_RAWREAD:        /*  not supported       */
    case TD_RAWWRITE:
        iob->io_Error = IOERR_NOCMD;
        break;
    case TD_GETDRIVETYPE:   /*  drive type?         */
        iob->io_Actual = 0;
        break;
    case TD_GETNUMTRACKS:
        iob->io_Actual = 0; /*  # of tracks?        */
        break;
    case TD_ADDCHANGEINT:   /*  action never taken  */
    case TD_REMCHANGEINT:
        break;
    default:
        iob->io_Error = IOERR_NOCMD;
        break;
    }
    if (iob) {
        if ((iob->io_Flags & IOF_QUICK) == 0)
            ReplyMsg((MSG *)iob);
    }
}

DevCall
void
DevAbortIO(register __a1 IOB *iob)
{

}


/*
 *  Server communications.  If we run out of memory, well, we retry
 *  until memory is available.
 */

void
SynchroMsg(cmd, unit)
UWORD cmd;
NDUnit *unit;
{
    IOB Iob;

    do {
        Iob.io_Message.mn_ReplyPort = CreatePort(NULL, 0);
        Iob.io_Command = cmd;
        Iob.io_Unit = (struct Unit *)unit;
    } while (Iob.io_Message.mn_ReplyPort == NULL);

    PutMsg(&FPort, &Iob.io_Message);
    WaitPort(Iob.io_Message.mn_ReplyPort);
    DeletePort(Iob.io_Message.mn_ReplyPort);
}

/*
 *      SERVER SIDE (IS A PROCESS)
 *
 *      File name is:
 */

__geta4 void
CoProc(void)
{
    IOB *iob;
    NDUnit *unit;
    char buf[128];
    char notdone = 1;

    Wait(SIGBREAKF_CTRL_D);     /*  wait for port init  */

    while (notdone) {
        WaitPort(&FPort);
        while (iob = (IOB *)GetMsg(&FPort)) {
            unit = (NDUnit *)iob->io_Unit;

#ifdef DEBUG
            if (DbFH) {
                sprintf(buf, "Cmd %08lx/%04x @ %08lx Buf %08lx %04x\n",
                    unit, iob->io_Command, iob->io_Offset, iob->io_Data, iob->io_Length
                );
                Write(DbFH, buf, strlen(buf));
            }
#endif
            /*
             *  cache (increase OFS throughput)
             */

            if (CacheLen && (iob->io_Command & ~TDF_EXTCOM) != CMD_WRITE)
                FlushCache();

            switch(iob->io_Command & ~TDF_EXTCOM) {
            case CMD_OPENUNIT:
                GetUnitName(unit - &DevBase->Unit[0], buf);
                unit->Fh = Open(buf, 1005);
                if (unit->Fh == NULL) {
                    unit->Fh = Open(buf, 1006);
                    unit->Extended = 1;
                }
#ifdef DEBUG
                if (DbFH) {
                    Write(DbFH, "OPEN ", 5);
                    Write(DbFH, buf, strlen(buf));
                    Write(DbFH, "\n", 1);
                }
#endif
                if (unit->Fh) {
                    Seek(unit->Fh, 0L, 1);
                    unit->Size = Seek(unit->Fh, 0L, -1);
                }
                unit->Pos = -1;
                break;
            case CMD_CLOSEUNIT:
                if (unit->Fh) {
                    Close(unit->Fh);
                    unit->Fh = NULL;
                }
                break;
            case CMD_KILLPROC:
                notdone = 0;
                break;
            case CMD_READ:
                if (unit->Fh == NULL)
                    break;
                if (iob->io_Offset + iob->io_Length > unit->Size)
                    ExtendSize(unit, iob->io_Offset + iob->io_Length);
                if (unit->Pos != iob->io_Offset)
                    Seek(unit->Fh, iob->io_Offset, -1);
                iob->io_Actual = Read(unit->Fh, (char *)iob->io_Data, iob->io_Length);
                if (iob->io_Actual == iob->io_Length)
                    unit->Pos = iob->io_Offset + iob->io_Actual;
                else
                    unit->Pos = -1;
                break;
            case CMD_WRITE:
                /*
                 *  This causes file to be closed/reopened after
                 *  formatting.
                 */
                if (unit->Extended && unit->Fh) {
                    FlushCache();
                    Close(unit->Fh);
                    GetUnitName(unit - &DevBase->Unit[0], buf);
                    unit->Fh = Open(buf, 1005);
                    unit->Extended = 0;
                }
                /* fall through */
            case TD_FORMAT:
                if (unit->Fh == NULL)
                    break;

                if (iob->io_Offset > unit->Size) {
                    FlushCache();
                    ExtendSize(unit, iob->io_Offset);
                }
                if (CacheUnit != unit) {
                    FlushCache();
                    CacheUnit = unit;
                }
                if (unit->Pos != iob->io_Offset) {

                    /*
                     *  Handle case where a CMD_WRITE modifies a previously
                     *  cached write (occurs with OFS *a lot*)
                     */

                    if (CacheLen) {
                        long ci = CacheLen - (unit->Pos - iob->io_Offset);
                        if (ci >= 0 && ci + iob->io_Length <= CacheLen) {
#ifdef DEBUG
                            if (DbFH)
                                Write(DbFH, "XBACK\n", 5);
#endif
                            iob->io_Actual = iob->io_Length;
                            movmem(iob->io_Data, CacheBuf + ci, iob->io_Length);
                            break;
                        }
                    }
                    FlushCache();
                    Seek(unit->Fh, iob->io_Offset, -1);
                }

                if (CacheLen + iob->io_Length > sizeof(CacheBuf))
                    FlushCache();
                if (CacheLen + iob->io_Length <= sizeof(CacheBuf)) {
                    iob->io_Actual = iob->io_Length;
                    unit->Pos = iob->io_Offset + iob->io_Actual;
                    movmem(iob->io_Data, CacheBuf + CacheLen, iob->io_Actual);
                    CacheLen += iob->io_Actual;
                } else {
                    if (CacheLen)
                        FlushCache();
                    iob->io_Actual = Write(unit->Fh, (char *)iob->io_Data, iob->io_Length);
                    if (iob->io_Actual == iob->io_Length)
                        unit->Pos = iob->io_Offset + iob->io_Actual;
                    else
                        unit->Pos = -1;
                }
                break;
            default:
                break;
            }

            if (notdone == 0)       /*  forbid before falling through */
                Forbid();           /*  and esp before replying       */
            ReplyMsg(&iob->io_Message);
        }
#ifdef DEBUG
        if (DbFH == NULL)
            DbFH = Open("con:0/0/320/100/Debug", 1006);
#endif
    }
#ifdef DEBUG
    if (DbFH)
        Close(DbFH);
#endif
    /* fall through to exit */
}

void
GetUnitName(int unitnum, char *buf)
{
BPTR envfile;
LONG envlen;
BOOL gotit = FALSE;

/* New - see if ENVIRONMENT variable FMSUnit<n> exists (ex. FMSUnit2)
 * containing full path to file.
 */
sprintf(buf,"ENV:FMSUnit%d",unitnum);
if(envfile = Open(buf,MODE_OLDFILE))
    {
    envlen = Read(envfile,buf,128-2);
    if(envlen >= 0)
        {
        gotit = TRUE;
        buf[envlen] = '\0';
        if(buf[envlen-1]=='\n') buf[envlen-1] = '\n';
        }
    Close(envfile);
    }
/* Fallback to old FMS:Unit<n> name */
if(!gotit)
    {
    sprintf(buf, "FMS:Unit%d", unitnum);
    }
}

/*
 *  Extend the file size in 4K chunks
 */

void
ExtendSize(unit, offset)
NDUnit *unit;
long offset;
{
    long pos;
    char *buf = AllocMem(EXT_CHUNK, MEMF_CLEAR|MEMF_PUBLIC);

    if (buf) {
        if (unit->Extended == 0)
            unit->Extended = 1;
        Seek(unit->Fh, 0L, 1);
        pos = Seek(unit->Fh, 0L, 0);
        while (pos < offset) {
            if (Write(unit->Fh, buf, EXT_CHUNK) != EXT_CHUNK)
                break;
            pos += EXT_CHUNK;
        }
        FreeMem(buf, EXT_CHUNK);
        unit->Pos = -1;     /*  unknown */
    }
}

/*
 *  flush sequential write cache
 */

void
FlushCache()
{
    NDUnit *unit = CacheUnit;

#ifdef DEBUG
    if (DbFH)
        Write(DbFH, "FLUSH\n", 6);
#endif

    if (CacheLen) {
        Write(unit->Fh, CacheBuf, CacheLen);
        CacheLen = 0;
    }
}
