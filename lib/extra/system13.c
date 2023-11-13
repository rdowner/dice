
/*
 *  SYSTEM13.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 *
 *  This routine will do the equivalent of system() for programs running
 *  under the 1.3 operating system.  system13() will work from either a
 *  CLI or workbench-run program and will properly return the exit code
 *  of the program.
 *
 *  Due to overhead, this call has not been integrated into system()
 *
 *  Under 1.3, NEWCLI, ENDCLI, and RUN must be in your path OR made
 *  resident.  Note that making these three programs resident will
 *  greatly increase system13()'s efficiency.  The dummy shell is left
 *  open for the duration of the program, so multiple system() calls
 *  will be efficient.
 *
 *      D0 - scratch
 *      D1 - scratch
 *
 *      A0 - scratch
 *      A1 - server is_Data pointer (scratch)
 *
 *      A5 - jump vector register (scratch)
 *      A6 - scratch
 *
 */

#define DOSBase_DECLARED

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/ports.h>
#include <exec/interrupts.h>
#include <exec/memory.h>
#include <exec/alerts.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/filehandler.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/alib_protos.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <lib/misc.h>

#define BTOC(bptr)  ((void *)((long)(bptr) << 2))
#define CTOB(cptr)  ((BPTR)(((long)cptr) >> 2))

#define DOS_TRUE    (-1)
#define DOS_FALSE   (0)

#ifdef DEBUG
#define dbprintf(x) fhprintf x
#else
#define dbprintf(x)
#endif

typedef struct DosPacket    DosPacket;
typedef struct FileHandle   FileHandle;
typedef struct DeviceNode   DeviceNode;
typedef struct Process      Process;
typedef struct MinNode      Node;
typedef struct List         List;
typedef struct Node         MaxNode;
typedef struct MsgPort      MsgPort;
typedef struct Message      Message;
typedef struct Interrupt    Interrupt;
typedef struct DosList      DosList;
typedef struct RootNode     RootNode;
typedef struct DosInfo      DosInfo;
typedef struct CommandLineInterface CLI;
typedef struct Task         Task;

typedef struct LockList {
    BPTR    NextPath;
    BPTR    PathLock;
} LockList;


void *DosAllocMem(long);
void DosFree(void *);
void MkDevice(char *, MsgPort *);
void DelDevice(void);
void ReturnPacket(DosPacket *);
void CopyWorkbenchPath(CLI *);
long StartCommand(char *);
long WaitCommand(void);
void WaitPktMask(void);

long _sys13SoftIntC(void);
long _sys13SoftIntA(__A1 void *);

extern struct DosLibrary *DOSBase;

static DosList *Dl;
static List     ReqList;        /*  pending read requests on dummy handle */
static MsgPort  PktPort;
static Task     *RemShellTask;
static char     DevName[16];
static char     DevBuf[128];
static long     DummyRefs;
static long     PktMask;
static long     ReturnCode;
static short    CmdStatus;      /*  0 none running, 1 in prog, -1 done  */
static short    ShellRunning;
static short    ConsoleTaken;
static short    TermFlag;
static long     BreakMask;
static Interrupt DevInt;

#ifdef DEBUG
static BPTR DBFh;
#endif

static __autoinit
void
sys13_startup()
{
    struct Process *proc = (struct Process *)FindTask(NULL);
    sprintf(DevName, "DICE%08lx", &TermFlag);

#ifdef DEBUG
    DBFh = Open("con:0/0/110/100/debug", 1005);
#endif

    dbprintf((DBFh, "PROCESS %08lx\n", proc));

    NewList(&ReqList);
    PktPort.mp_Node.ln_Name = (char *)&proc->pr_Task;
    PktPort.mp_Node.ln_Type = NT_MSGPORT;
    PktPort.mp_Flags = PA_SOFTINT;
    PktPort.mp_SigBit = AllocSignal(-1);
    PktPort.mp_SoftInt = &DevInt;

    NewList(&PktPort.mp_MsgList);

    DevInt.is_Data = (APTR)rega4();
    DevInt.is_Code = (void(*)())_sys13SoftIntA;
    DevInt.is_Node.ln_Type = NT_INTERRUPT;

    PktMask = 1 << PktPort.mp_SigBit;

    MkDevice(DevName, &PktPort);

    dbprintf((DBFh, "TEST1\n"));
}

static __autoexit
void
sys13_shutdown()
{
    struct Process *proc = (struct Process *)FindTask(NULL);

    if (ShellRunning) {
        TermFlag = 1;
        if (StartCommand("endcli") == 0)
            WaitCommand();
        while (DummyRefs)
            WaitPktMask();
    }
    DelDevice();
    if ((char)PktPort.mp_SigBit >= 0) {
        FreeSignal(PktPort.mp_SigBit);
        PktPort.mp_SigBit = -1;
    }
#ifdef DEBUG
    if (DBFh) {
        Close(DBFh);
        DBFh = 0;
    }
#endif
}

/*
 *  Are we having fun yet?  Now, we really roll...
 *
 *  (1) Open a dummy file handle (may already be done)
 *  (2) Execute("", fh, NULL) to get a remote shell (may already be done)
 *  (3) Supply command as a replied read request
 *  (4) wait until cli_Module becomes NULL (command complete)
 *  (5) Obtain return code
 *  (6) ENDCLI the shell (defered until autoexit)
 */

int
system13(buf)
const char *buf;
{
    long r = -1;

    if (StartCommand(buf) == 0)
        r = WaitCommand();
    return(r);
}

/*
 *  WARNING!!! We cannot make *any* DOS calls while a command is in
 *  progress due to the possibility of an exception interrupting us
 *  in the middle of such a call, then trying to do a DOS call itself.
 */

static long
StartCommand(buf)
char *buf;
{
    Process *proc = (Process *)PktPort.mp_Node.ln_Name;

    dbprintf((DBFh, "START1\n"));

    if (proc->pr_ConsoleTask == NULL) {
        proc->pr_ConsoleTask = &PktPort;
        ConsoleTaken = 1;
    }

    if (ShellRunning == 0) {
        /*
         *  can't do this in autoinit or _main will think we have a
         *  real console and attempt to open it!
         */

        TermFlag = 1;
        sprintf(DevBuf, "newshell <%s: >%s: %s: from nil:", DevName, DevName, DevName);
        Execute(DevBuf, 0, 0);
        /*System(DevBuf, NULL);*/
        if (IoErr()) {
            if (ConsoleTaken) {
                proc->pr_ConsoleTask = NULL;
                ConsoleTaken = 0;
            }
            TermFlag = 0;
            return(-1);
        }
        ShellRunning = 1;
        if (StartCommand("") == 0)  /*  synchronize */
            WaitCommand();
        TermFlag = 0;

        /*
         *  If running from the workbench we have no CLI and must copy
         *  the path from the workbench process's CLI
         */

        if ((((Task *)PktPort.mp_Node.ln_Name)->tc_Node.ln_Type != NT_PROCESS || ((Process *)PktPort.mp_Node.ln_Name)->pr_CLI == NULL) && RemShellTask) {
            if (RemShellTask->tc_Node.ln_Type == NT_PROCESS && ((Process *)RemShellTask)->pr_CLI)
                CopyWorkbenchPath(BTOC(((Process *)RemShellTask)->pr_CLI));
        }
    }

    dbprintf((DBFh, "START2\n"));

    /*
     *  Preset cli_ReturnCode in case command not found
     */

    if (RemShellTask) {
        CLI *cli = BTOC(((Process *)RemShellTask)->pr_CLI);
        if (cli)
            cli->cli_ReturnCode = 10;
    }

    /*
     *  Supply command line by replying to read requests.  Turn off
     *  signal exceptions to handle it synchronously
     */

    {
        Message *msg;
        DosPacket *pkt;
        short len = strlen(buf);
        short n;

        if (len == 0 || buf[len-1] != '\n')
            ++len;  /*  virtual newline */

        while (len) {
            for (;;) {
                Disable();
                msg = (Message *)RemHead(&ReqList);
                Enable();
                if (msg)
                    break;
                WaitPktMask();
            }
            pkt = (DosPacket *)msg->mn_Node.ln_Name;

            if (TermFlag == 0 && pkt->dp_Port->mp_SigTask != (Task *)PktPort.mp_Node.ln_Name) {
                if ((pkt->dp_Port->mp_Flags & PF_ACTION) == PA_SIGNAL && ((Task *)pkt->dp_Port->mp_SigTask)->tc_Node.ln_Type == NT_PROCESS)
                    RemShellTask = pkt->dp_Port->mp_SigTask;
            }

            /*
             *  Handle packet
             */

            if ((n = pkt->dp_Arg3) > len)
                n = len;
            movmem(buf, (void *)pkt->dp_Arg2, n);
            if (len == n)   /*  last char is a newline  */
                ((char *)pkt->dp_Arg2)[n-1] = '\n';
            len -= n;
            buf += n;
            pkt->dp_Res1 = n;
            CmdStatus = 1;      /*  in progress */
            ReturnPacket(pkt);
        }
    }
    return(0);
}

/*
 *  Wait for command completion - last handle closed or cli_Module == NULL
 *
 *  Reenable signal exceptions after completion
 */

static long
WaitCommand()
{
    Process *proc = (Process *)PktPort.mp_Node.ln_Name;

    dbprintf((DBFh, "WAIT1\n"));

    while (CmdStatus == 1)
        WaitPktMask();

    if (ConsoleTaken) {
        proc->pr_ConsoleTask = NULL;
        ConsoleTaken = 0;
    }
    if (BreakMask) {
        SetSignal(BreakMask, BreakMask);
        chkabort();
        BreakMask = 0;
    }
    return(ReturnCode);
}

/*
 *  Software Interrupt deals with handler functions
 */

long
_sys13SoftIntC()
{
    Message *msg;
    DosPacket *pkt;

    while (msg = GetMsg(&PktPort)) {
        pkt = (DosPacket *)msg->mn_Node.ln_Name;

        pkt->dp_Res2 = 0;


        switch(pkt->dp_Type) {
        case ACTION_READ:

            AddTail(&ReqList, &((Message *)pkt->dp_Link)->mn_Node);

            /*
             *  Check command competion -- shell's cli_Module set to
             *  NULL.  Also handle giving the shell a path
             */

            {
                Process *proc = pkt->dp_Port->mp_SigTask;

                if (proc->pr_Task.tc_Node.ln_Type == NT_PROCESS && proc->pr_CLI) {
                    CLI *cli = BTOC(proc->pr_CLI);

                    if (cli->cli_Module == NULL) {
                        if (CmdStatus == 1) {
                            CmdStatus = -1;
                            ReturnCode = cli->cli_ReturnCode;
                        }
                    }
                }
            }
            continue;
        case ACTION_WRITE:
            /*
             *  Copy any output to our standard out (if we have a
             *  standard out).  We have to skip the shell prompts
             *  and other garbage which is why the junk.  Note that
             *  we always force Res1 = Arg3 so the write 'succeeds'
             *
             *  we cannot write while running async (exceptions enabled)
             *  as a crash would occur if we try to make a dos call
             *  over a dos call in progress.
             */


            if (CmdStatus == 1 && TermFlag == 0) {
                Process *proc = (Process *)PktPort.mp_Node.ln_Name;
                CLI *remCli = BTOC(((Process *)pkt->dp_Port->mp_SigTask)->pr_CLI);

                if (remCli && remCli->cli_Module) {
                    if (proc->pr_Task.tc_Node.ln_Type == NT_PROCESS && proc->pr_COS) {
                        /*
                         *  forward the packet
                         */
                        PutMsg(((FileHandle *)BTOC(proc->pr_COS))->fh_Type, msg);
                        continue;
                    }
                }
            }
            pkt->dp_Res1 = pkt->dp_Arg3;
            break;
        case ACTION_FINDUPDATE:
        case ACTION_FINDINPUT:
        case ACTION_FINDOUTPUT:
            {
                FileHandle *fh = BTOC(pkt->dp_Arg1);
                fh->fh_Arg1 = pkt->dp_Arg1;
                fh->fh_Port = (MsgPort *)DOS_TRUE;
                pkt->dp_Res1 = DOS_TRUE;
            }
            ++DummyRefs;
            break;
        case ACTION_END:
            pkt->dp_Res1 = DOS_TRUE;
            if (--DummyRefs == 0) {     /*  shell shutdown */
                if (CmdStatus == 1)
                    CmdStatus = -1;
                ReturnCode = 0;
            }
            break;
        default:
            pkt->dp_Res2 = ERROR_ACTION_NOT_KNOWN;
            pkt->dp_Res1 = DOS_FALSE;
            break;
        }
        ReturnPacket(pkt);
    }
    Signal((Task *)PktPort.mp_Node.ln_Name, PktMask);
    return(0);
}

/*
 *  DEVICE CREATION AND DELETION
 */


void
MkDevice(devName, port)
char *devName;
MsgPort *port;
{
    DosList *dl;
    RootNode *root;
    DosInfo *info;

    Dl = dl = (struct DosList *)DosAllocMem(sizeof(struct DosList)+strlen(devName)+2);
    strcpy((char *)(dl+1) + 1, devName);
    *(char *)(dl + 1) = strlen(devName);
    dl->dol_Type = DLT_DEVICE;
    dl->dol_Task = port;
    dl->dol_Name = MKBADDR((char *)(dl+1));

    Forbid();
    root  = (struct RootNode *)DOSBase->dl_Root;
    info  = (struct DosInfo  *)BADDR(root->rn_Info);
    dl->dol_Next = info->di_DevInfo;
    info->di_DevInfo = MKBADDR(dl);
    Permit();
}

void
DelDevice()
{
    DosList *dl;
    DosInfo *info;
    RootNode *root;
    DosList *dls;
    BPTR    *bpp;

    if (dl = Dl) {
        Forbid();
        root  = (struct RootNode *)DOSBase->dl_Root;
        info  = (struct DosInfo  *)BADDR(root->rn_Info);

        for (bpp = &info->di_DevInfo; dls = BADDR(*bpp); bpp = &dls->dol_Next) {
            if (dls == dl)
                break;
        }
        if (dls == dl) {
            *bpp = dls->dol_Next;
        } else {
            Alert(0x07AAAAAA|AT_Recovery);
        }
        Permit();
        DosFree(dl);
        Dl = NULL;
    }
}

void *
DosAllocMem(bytes)
long bytes;
{
    long *ptr;

    bytes += 4;

    if (ptr = AllocMem(bytes, MEMF_PUBLIC | MEMF_CLEAR)) {
        *ptr++ = bytes;
        return((void *)ptr);
    }
    Alert(AG_NoMemory|AT_DeadEnd);
}

void
DosFree(vptr)
void *vptr;
{
    long *ptr = vptr;
    --ptr;
    FreeMem(ptr, *ptr);
}

void
ReturnPacket(packet)
DosPacket *packet;
{
    Message *mess;
    MsgPort *replyPort;

    replyPort                = packet->dp_Port;
    mess                     = packet->dp_Link;
    packet->dp_Port          = &PktPort;
    mess->mn_Node.ln_Name    = (char *)packet;
    PutMsg(replyPort, mess);
}

/*
 *  Copy the path from the workbench process's CLI into the specified CLI
 */

void
CopyWorkbenchPath(dcli)
CLI *dcli;
{
    CLI *scli;
    LockList *lls;
    Process *sproc;

    if ((sproc = (struct Process *)FindTask("Workbench")) && sproc->pr_Task.tc_Node.ln_Type == NT_PROCESS) {
        if (scli = BTOC(sproc->pr_CLI)) {
            for (lls = BTOC(scli->cli_CommandDir); lls; lls = BTOC(lls->NextPath)) {
                BPTR lock;
                LockList *ll;
                LockList **llast = (LockList **)&dcli->cli_CommandDir;

                if (lock = DupLock(lls->PathLock)) {
                    if (ll = ((LockList *)AllocMem(sizeof(LockList) + 4, MEMF_PUBLIC|MEMF_CLEAR) + 1)) {
                        ((long *)ll)[-1] = sizeof(LockList) + 4;
                        ll->NextPath = (BPTR)llast;
                        ll->PathLock = lock;
                        *llast = (LockList *)MKBADDR(ll);
                        llast = (LockList **)&ll->NextPath;
                    } else {
                        UnLock(lock);
                    }
                }
            }
        }
    }
}

void
WaitPktMask(void)
{
    long mask = 0;

    while ((mask & PktMask) == 0) {
        mask = Wait(PktMask | SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_D | SIGBREAKF_CTRL_E | SIGBREAKF_CTRL_F);
        if (mask & (SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_D | SIGBREAKF_CTRL_E | SIGBREAKF_CTRL_F)) {
            if (RemShellTask)
                Signal(RemShellTask, (mask & (SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_D | SIGBREAKF_CTRL_E | SIGBREAKF_CTRL_F)));
            else
                BreakMask |= mask & (SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_D | SIGBREAKF_CTRL_E | SIGBREAKF_CTRL_F);
        }
    }
}



