
/*
 *  UNIX/VFORK.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/ports.h>
#include <exec/tasks.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/dostags.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <ioctl.h>
#include <time.h>
#include <errno.h>
#include <setjmp.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <lib/misc.h>
#include <lib/unix.h>

#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/alib_protos.h>
#include <lists.h>

#ifndef UnixToAmigaPath
#define UnixToAmigaPath(path)   path
#endif

typedef struct CommandLineInterface CLI;
typedef struct FileLock FileLock;
typedef struct MsgPort  MsgPort;
typedef struct Node     Node;
typedef struct List     List;
typedef struct Process  Process;
typedef struct Task     Task;
typedef struct Segment  Segment;

typedef struct LockList {
    BPTR    NextPath;
    BPTR    PathLock;
} LockList;

typedef struct ProcNode {
    Node    pn_Node;
    long    pn_Pid;
    long    pn_ExitCode;
    long    pn_Status;      /*  0 == running, 1 == exited   */
    Task    *pn_Task;
    Task    *pn_Parent;
    Segment *pn_Seg;        /*  resident segment    */
} ProcNode;

#define SIGBREAKF_ALL   (SIGBREAKF_CTRL_C|SIGBREAKF_CTRL_D|SIGBREAKF_CTRL_E|SIGBREAKF_CTRL_F)

extern int __InUnixFork;

__near jmp_buf _amiga_vfork_jmp;
static int  vfork_fda[3];   /*  stdin , stdout, stderr  */
static int  vfork_fdc[3];   /*  closable                */
static int  vfork_init;
static int  vfork_pid;      /*  pid's for subprocs      */
static List ProcList;

typedef void *void_ptr;

void _amiga_vfork_returnc(long, ProcNode *);
void amiga_vfork_exit(void);

extern void _amiga_vfork_return(void);

void
amiga_vfork_exit()
{
    union wait status;

    amiga_vfork_sigall(SIGBREAKF_ALL);

    while (wait(&status) != -1)
        ;
}

/*
 *  This routine is called (out of context) when a child process exits.
 *  It must set the status & exit code then signal the parent
 */

void
_amiga_vfork_returnc(rc, pn)
long rc;
ProcNode *pn;
{
    ProcNode *scan;
    Process *proc = (Process *)pn->pn_Task;

    /*
     *  close stdin, stdout, stderr here to synchronize with parent
     *  ?? stderr flag ??
     */

    if (proc->pr_Flags & PRF_CLOSEINPUT) {
        Close(proc->pr_CIS);
        proc->pr_CIS = NULL;
        proc->pr_Flags &= ~PRF_CLOSEINPUT;
    }
    if (proc->pr_Flags & PRF_CLOSEOUTPUT) {
        Close(proc->pr_COS);
        proc->pr_COS = NULL;
        proc->pr_Flags &= ~PRF_CLOSEOUTPUT;
    }

    /*
     *  find job node and signal/set status
     */

    Forbid();
    for (scan = GetHead(&ProcList); scan; scan = GetSucc(&scan->pn_Node)) {
        /*
        fhprintf(((Process *)scan->pn_Parent)->pr_COS, "SCAN: %08lx: %08lx %08lx\n", scan, pn, rc);
        */
        if (pn == scan)
            break;
    }
    if (scan) {
        {
            Segment *seg;
            if (seg = pn->pn_Seg) {
                if (seg->seg_UC > 0)
                    --seg->seg_UC;
            }
        }
        scan->pn_ExitCode = rc;
        scan->pn_Status = 1;
        Signal(scan->pn_Parent, SIGF_SINGLE);
    }
    /*
    fhprintf(((Process *)scan->pn_Parent)->pr_COS, "EXIT: %s %08lx %08lx %d\n", FindTask(NULL)->tc_Node.ln_Name, pn, scan, ((Process *)FindTask(NULL))->pr_Result2);
    */
    /* Permit();    leave in Forbid state */
}

/*
 *  called from assembly tag
 */

int
_amiga_vforkc()
{
    if (vfork_init == 0) {
        vfork_init = 1;
        NewList(&ProcList);
        atexit(amiga_vfork_exit);
    }
    ++vfork_pid;

    __InUnixFork = 1;
    vfork_fda[0] = -1;
    vfork_fda[1] = -1;
    vfork_fda[2] = -1;
    vfork_fdc[0] = 0;
    vfork_fdc[1] = 0;
    vfork_fdc[2] = 0;
    return(0);              /*  child   */
}

int
amiga_execlp(path, arg0, ...)
char *path;
char *arg0;
{
    return(amiga_execvp(path, &arg0));
}

int
amiga_execvp(path, args)
char *path;
char **args;
{
    {
        Segment *seg;

        Forbid();
        if ((seg = FindSegment(FilePart(path), NULL, 0)) || (seg = FindSegment(FilePart(path), NULL, 1))) {
            if (seg->seg_UC >= 0)
                ++seg->seg_UC;
            Permit();
            return(amiga_execseg(args, seg, seg->seg_Seg));
        }
        Permit();
    }
    {
        BPTR segList;

        if (segList = LoadSeg(UnixToAmigaPath(path)))
            return(amiga_execseg(args, NULL, segList));
    }
    if (strchr(UnixToAmigaPath(path), ':') == NULL) {
        CLI *cli = (CLI *)BADDR(((Process *)FindTask(NULL))->pr_CLI);
        BPTR segList = NULL;

        if (cli) {
            LockList *ll;

            for (ll = (LockList *)BADDR(cli->cli_CommandDir); segList == NULL && ll; ll = (LockList *)BADDR(ll->NextPath)) {
                if (ll->PathLock) {
                    BPTR lock = CurrentDir(ll->PathLock);
                    segList = LoadSeg(UnixToAmigaPath(path));
                    CurrentDir(lock);
                }
            }
        }
        return(amiga_execseg(args, NULL, segList));
    }
}

int
amiga_execl(path, arg0, ...)
char *path;
char *arg0;
{
    return(amiga_execv(path, &arg0));
}

int
amiga_execv(path, args)
char *path;
char **args;
{
    {
        Segment *seg;

        Forbid();
        if ((seg = FindSegment(FilePart(path), NULL, 0)) || (seg = FindSegment(FilePart(path), NULL, 1))) {
            if (seg->seg_UC >= 0)
                ++seg->seg_UC;
            Permit();
            return(amiga_execseg(args, seg, seg->seg_Seg));
        }
        Permit();
    }
    {
        BPTR segList;

        if (segList = LoadSeg(UnixToAmigaPath(path)))
            return(amiga_execseg(args, NULL, segList));
    }
    return(amiga_execseg(args, NULL, NULL));
}

int
amiga_execseg(args, seg, segList)
char **args;
struct Segment *seg;
long segList;
{
    int r = -1;
    int freeSegList = (seg) ? 0 : 1;
    ProcNode *pn;

    if (__InUnixFork == 0) {
        if (freeSegList)
            UnLoadSeg(segList);
        errno = ENOEXEC;
        return(-1);
    }

    pn = malloc(sizeof(ProcNode));
    if (pn == NULL) return(-1);

    clrmem(pn, sizeof(ProcNode));

    pn->pn_Pid = vfork_pid;
    pn->pn_Parent = FindTask(NULL);
    AddTail(&ProcList, &pn->pn_Node);

    /*
     *  Startup the process
     */

    if (segList) {
        char *argStr;
        {
            short i;
            short len;
            for (i = 1, len = 0; args[i]; ++i)
                len += strlen(args[i]) + 1;
            argStr = malloc(len + 3);
            if (argStr)
            {
               for (i = 1, len = 0; args[i]; ++i)
                   len += sprintf(argStr + len, "%s ", args[i]);
               if (len)
                   --len;
               argStr[len++] = '\n';   /*  so BCPL programs work right */
               argStr[len] = 0;
            }
            else argStr = NULL;
        }

        pn->pn_Node.ln_Name = (char *)rega4();
        pn->pn_Seg = seg;

        pn->pn_Task = (Task *)CreateNewProcTags(
            NP_Seglist, segList,
            NP_FreeSeglist, freeSegList,
            NP_Input, (vfork_fda[0] >= 0) ? __getfh(vfork_fda[0])->fd_Fh : Input(),
            NP_CloseInput, vfork_fdc[0],
            NP_Output, (vfork_fda[1] >= 0) ? __getfh(vfork_fda[1])->fd_Fh : Output(),
            NP_CloseOutput, vfork_fdc[1],
            NP_Error, (vfork_fda[2] >= 0) ? __getfh(vfork_fda[2])->fd_Fh : Output(),
            NP_CloseError, vfork_fdc[2],
            NP_StackSize, ((struct CommandLineInterface *)BADDR(((struct Process *)pn->pn_Parent)->pr_CLI))->cli_DefaultStack * 4,
            NP_CommandName, args[0],
            NP_Arguments, argStr,
            NP_Cli, 1,
            NP_ExitCode, _amiga_vfork_return,
            NP_ExitData, pn,
            TAG_END
        );

        free(argStr);

        if (pn->pn_Task) {
            r = pn->pn_Pid;
        } else {
            if (freeSegList)
                UnLoadSeg(segList);
            segList = NULL;
        }
    }

    /*
     *  if success, invalidate descriptors that we passed to the child
     *  (child process will close them)
     */

    __InUnixFork = 0;

    if (r > 0) {
        _IOFDS *d;
        short i;

        for (i = 0; i < 3; ++i) {
            if ((vfork_fda[i] >= 0) && (d = __getfh(vfork_fda[i]))) {
                d->fd_Flags |= O_NOCLOSE;
                close(vfork_fda[i]);
            }
        }
    } else {
        pn->pn_Status = 1;
        pn->pn_ExitCode = -1;
        r = pn->pn_Pid;
        errno = ENOEXEC;
    }

    longjmp(_amiga_vfork_jmp, r);
}

/*
 *  amiga_dup2() is used to flag descriptors for stdin/stdout in the exec
 */

int
amiga_dup2(fdi, fdo)
int fdi;
int fdo;
{
    _IOFDS *d;

    if (fdo >= 0 && fdo < 3) {
        vfork_fda[fdo] = fdi;

        /*
         *  determine whether the created process will be allowed to
         *  close the handle.  Can't if it's marked NOCLOSE and can't
         *  if we are passing the same descriptor a second or third
         *  time.
         */

        if ((d = __getfh(fdi)) && (d->fd_Flags & O_NOCLOSE) == 0)
            vfork_fdc[fdo] = 1;
        if (vfork_fda[(fdo+1)%3] == fdi)
            vfork_fdc[fdo] = 0;
        if (vfork_fda[(fdo+2)%3] == fdi)
            vfork_fdc[fdo] = 0;
    }
    return(fdo);
}

int
wait(status)
union wait *status;
{
    return(wait3(status, 0, NULL));
}

int
wait3(status, flags, rus)
union wait *status;
int flags;
struct rusage *rus;
{
    ProcNode *pn = NULL;
    int r = -1;

    clrmem(status, sizeof(union wait));

    for (;;) {
        for (pn = GetHead(&ProcList); pn; pn = GetSucc(&pn->pn_Node)) {
            if (pn->pn_Status) {
                flags |= WNOHANG;
                break;
            }
        }

        if ((flags & WNOHANG) || GetHead(&ProcList) == NULL)
            break;

        {
            long mask;

            mask = Wait(SIGF_SINGLE | SIGBREAKF_ALL);
            if (mask & SIGBREAKF_ALL) {
                amiga_vfork_sigall(mask & SIGBREAKF_ALL);
            }
        }
    }

    if (pn) {
        Remove(&pn->pn_Node);
        status->w_retcode = pn->pn_ExitCode;
        status->u.wu_flags |= WF_EXITED;
        r = pn->pn_Pid;
        free(pn);
    } else {
        errno = 0;
    }
    return(r);
}

int
sigcheckchld()
{
    ProcNode *pn;

    for (pn = GetHead(&ProcList); pn; pn = GetSucc(&pn->pn_Node)) {
        if (pn->pn_Status)
            return(1);
    }
    return(0);
}

int
kill(pid, signo)
int pid;
int signo;
{
    ProcNode *pn;
    int r = -1;

    Forbid();
    for (pn = GetHead(&ProcList); pn; pn = GetSucc(&pn->pn_Node)) {
        if (pn->pn_Pid == pid && pn->pn_Status == 0) {
            Signal(pn->pn_Task, SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_D);
            r = 0;
        }
    }
    Permit();
    return(r);
}


void
amiga_vfork_sigall(mask)
long mask;
{
    ProcNode *pn;

    Forbid();
    for (pn = GetHead(&ProcList); pn; pn = GetSucc(&pn->pn_Node)) {
        if (pn->pn_Status == 0)
            Signal(pn->pn_Task, mask & SIGBREAKF_ALL);
    }
    Permit();
}

