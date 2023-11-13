/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */
#include        "defs.h"
#include        "dbug_protos.h"
#include        <lists.h>
#include        <lib/misc.h>

Prototype void  FreeDLIST(LIST *list);
Prototype BOOL  AddListNODE(LIST *list, UBYTE type, char *data);
Prototype BOOL  DownList(void);
Prototype BOOL  UpList(void);
Prototype BOOL  PageDownList(void);
Prototype BOOL  PageUpList(void);
Prototype WORD  RefreshList(WORD maxLines, BOOL fullRefresh, LONG top);
Prototype BOOL  SprintfDLIST(LIST *list, UBYTE type, char *fmt, ...);
Local unsigned int _swrite(char *buf, size_t n1, size_t n2, const char **sst);

Prototype void  ListDosBase(DBugDisp *dp);
Prototype void  ListProcess(DBugDisp *dp);

Prototype void  ListInfo(DBugDisp *dp);
Prototype void  ListExecBase(DBugDisp *dp);
Prototype void  BuildDLIST(LIST *dst, LIST *src, ULONG type);
Prototype void  ListResources(DBugDisp *dp);
Prototype void  ListIntrs(DBugDisp *dp);
Prototype void  ListPorts(DBugDisp *dp);
Prototype void  ListTasks(DBugDisp *dp);
Prototype void  ListLibs(DBugDisp *dp);
Prototype void  ListDevices(DBugDisp *dp);
Prototype void  ListMemList(DBugDisp *dp);
Prototype int   SizeDLIST(LIST *list);
Prototype BOOL  ListSymbols(DBugDisp *dp);

// ************************************************************************

int SizeDLIST(LIST *list) {
        DLIST   *dp;
        int i=0;

        for (dp = GetHead(&CurDisplay->ds_List); dp; 
                dp = GetSucc(&dp->node)) {
            ++i;
        }
        return i;
}

void    FreeDLIST(LIST *list) {
        LIST *lp;

        while (lp = RemHead(list)) Free(lp);
        NewList(list);
}


BOOL    AddListNODE(LIST *list, UBYTE type, char *name) {
        WORD    len = strlen(name);
        DLIST   *dp = (DLIST *)MallocPublic(sizeof(DLIST)+len+1);

        if (GetHead(list) == NULL)
            CurDisplay->ds_WindowTop = 0;

        if (dp) {
                dp->node.ln_Name = (char *)&dp->name[0];
                strcpy(dp->name, name);
                dp->node.ln_Type = type;
                AddTail(list, (NODE *)dp);
                return TRUE;
        }
        return FALSE;
}

// ************************************************************************

BOOL    DownList(void) {

        ++CurDisplay->ds_WindowTop;
        ++CurDisplay->ds_WindowBot;
        ScrScrollup();
        RefreshList(1, -1, CurDisplay->ds_WindowBot);
        return TRUE;
}

BOOL    UpList(void)
{
        if (CurDisplay->ds_WindowTop) {
            --CurDisplay->ds_WindowTop;
            --CurDisplay->ds_WindowBot;
            ScrScrolldown();
            RefreshList(1, -1, CurDisplay->ds_WindowTop);
            return TRUE;
        }
        return FALSE;
}

BOOL    PageUpList(void)
{
    int lines = CalcDisplayLines();

    if ((LONG)CurDisplay->ds_WindowTop > 0) {
        if(lines > 0) {
            CurDisplay->ds_WindowTop -= lines;
            if((LONG)CurDisplay->ds_WindowTop < 0)CurDisplay->ds_WindowTop = 0;
            CurDisplay->ds_WindowBot -= lines;
            if((LONG)CurDisplay->ds_WindowBot < 0)CurDisplay->ds_WindowBot = 0;
        }
        RefreshWindow(1);
    }
}

BOOL    PageDownList(void)
{
    int lines = CalcDisplayLines();

    if(lines > 0) {
            CurDisplay->ds_WindowTop += lines;
            CurDisplay->ds_WindowBot += lines;
    }
    RefreshWindow(1);
}

WORD    RefreshList(WORD maxLines, BOOL fullRefresh, LONG line) {
        DLIST   *dp;
        LONG trackLine = line;
        WORD count = 0;

        do_scroller();
        if (fullRefresh != -1) {
            if (dp = GetHead(&CurDisplay->ds_List)) {
                SetTitle(dp->name,NULL);
#if 0
                ScrPlain();
                ScrInverse();
                ScrPuts(dp->name);
                ScrPlain();
#endif
                if(CurDisplay->ds_RegFlag) {
                    ++count;
                    Newline();
                    if (--maxLines == 0)return count;
                }
            }
        }
        for (dp = GetHead(&CurDisplay->ds_List); dp && line; dp = GetSucc(&dp->node)) {
            --line;
        }
        while (dp && maxLines) {
            ScrPuts(dp->name);
            ScrPutNewline();
            dp = GetSucc(&dp->node);
            --maxLines;
            ++trackLine;
            ++count;
        }
        while (maxLines > 0) {
            ScrPutNewline();
            --maxLines;
            ++count;
        }
        if (fullRefresh > 0) {
            CurDisplay->ds_WindowBot = trackLine-1;
        }
        return (count);
}

// ************************************************************************

Local unsigned int _swrite(char *buf, size_t n1, size_t n2, const char **sst) {
        size_t n;

        if (n1 == 1)
                n = n2;
        else if (n2 == 1)
                n = n1;
            else
                n = n1 * n2;

        _slow_bcopy(buf, *sst, n);
        *sst += n;
        return(n2);
}

BOOL    SprintfDLIST(LIST *list, UBYTE type, char *fmt, ...) {
        char            buf[128];
        char            *ptr = &buf[0];
        va_list         va;
        WORD            n;

        va_start(va, fmt);
        n = (WORD)_pfmt(fmt, va, _swrite, &ptr);
        *ptr = 0;
        va_end(va);
        return AddListNODE(list, type, buf);
}

/************************************************************************/

void    ListDosBase(DBugDisp *dp) {
        LIST                            *lp = &dp->ds_List;
        struct DosLibrary               *d = DOSBase;
        struct RootNode                 *rn = d->dl_Root;

        FreeDLIST(lp);
        SetDisplayMode(DISPLAY_DOSBASE,0);
        SprintfDLIST(lp, DTYPE_DOSBASE, "struct DOSBase:");
        SprintfDLIST(lp, DTYPE_DOSBASE, " dl_Root = %08X ; Pointer to RootNode, described below", d->dl_Root);
        SprintfDLIST(lp, DTYPE_DOSBASE, " dl_GV   = %08X ; Pointer to BCPL global vector", d->dl_GV);
        SprintfDLIST(lp, DTYPE_DOSBASE, " dl_A2   = %08X ; Private register dump of DOS", d->dl_A2);
        SprintfDLIST(lp, DTYPE_DOSBASE, " dl_A5   = %08X", d->dl_A5);
        SprintfDLIST(lp, DTYPE_DOSBASE, " dl_A6   = %08X", d->dl_A6);

        SprintfDLIST(lp, DTYPE_DOSBASE, "");
        SprintfDLIST(lp, DTYPE_DOSBASE, "struct RootNode:");
        SprintfDLIST(lp, DTYPE_DOSBASE, " rn_TaskArray          = %08X ; [0] is max number of CLI's", BADDR(rn->rn_TaskArray));
        SprintfDLIST(lp, DTYPE_DOSBASE, "                                  ; [1] is APTR to APROCESS id of CLI 1");
        SprintfDLIST(lp, DTYPE_DOSBASE, "                                  ; [n] is APTR to APROCESS id of CLI n");
        SprintfDLIST(lp, DTYPE_DOSBASE, " rn_ConsoleSegment     = %08X ; SegList for the CLI", BADDR(rn->rn_ConsoleSegment));
#if 0
    struct  DateStamp rn_Time; ; Current time
#endif
        SprintfDLIST(lp, DTYPE_DOSBASE, " rn_RestartSeg         = %08X ; SegList for the disk validator APROCESS", rn->rn_RestartSeg);
        SprintfDLIST(lp, DTYPE_DOSBASE, " rn_Info               = %08X ; Pointer to the Info structure", BADDR(rn->rn_Info));
        SprintfDLIST(lp, DTYPE_DOSBASE, " rn_FileHandlerSegment = %08X ; segment for a file handler", BADDR(rn->rn_FileHandlerSegment));
#if 0
    struct MinList rn_CliList; ; new list of all CLI PROCESSes
                               ; the first cpl_Array is also rn_TaskArray
#endif
        SprintfDLIST(lp, DTYPE_DOSBASE, " rn_BootProc           = %08X ; private ptr to msgport of boot fs", rn->rn_BootProc);
        SprintfDLIST(lp, DTYPE_DOSBASE, " rn_ShellSegment       = %08X ; seglist for Shell (for NewShell)       ", BADDR(rn->rn_ShellSegment));
        SprintfDLIST(lp, DTYPE_DOSBASE, " rn_Flags              = %08X ; dos flags", rn->rn_Flags);
}

void    ListProcess(DBugDisp *dp) {
        APROCESS        *p = (APROCESS *)FindTask(NULL);
        LIST            *lp = &dp->ds_List;

        FreeDLIST(lp);
        SetDisplayMode(DISPLAY_PROCESS,0);
        SprintfDLIST(lp, DTYPE_PROCESS, "struct APROCESS @$%08X:", p);
        SprintfDLIST(lp, DTYPE_PROCESS, " pr_SegList        = %08X ; Array of seg lists for this APROCESS", BADDR(p->pr_SegList));
        SprintfDLIST(lp, DTYPE_PROCESS, " pr_StackSize      = %08X ; APROCESS stack size in bytes", p->pr_StackSize);
        SprintfDLIST(lp, DTYPE_PROCESS, " pr_GlobVec        = %08X ; Global vector for this APROCESS (BCPL)", p->pr_GlobVec);
        SprintfDLIST(lp, DTYPE_PROCESS, " pr_TaskNum        = %08X ; CLI task number, zero if not a CLI", p->pr_TaskNum);
        SprintfDLIST(lp, DTYPE_PROCESS, " pr_StackBase      = %08X ; High memory end of APROCESS stack", BADDR(p->pr_StackBase));
        SprintfDLIST(lp, DTYPE_PROCESS, " pr_Result2        = %08X ; Secondary result from last call", p->pr_Result2);
        SprintfDLIST(lp, DTYPE_PROCESS, " pr_CurrentDir     = %08X ; Lock on current directory", BADDR(p->pr_CurrentDir));
        SprintfDLIST(lp, DTYPE_PROCESS, " pr_CIS            = %08X ; Current CLI Input Stream", BADDR(p->pr_CIS));
        SprintfDLIST(lp, DTYPE_PROCESS, " pr_COS            = %08X ; Current CLI Output Stream", BADDR(p->pr_COS));
        SprintfDLIST(lp, DTYPE_PROCESS, " pr_ConsoleTask    = %08X ; Console handler APROCESS", p->pr_ConsoleTask);
        SprintfDLIST(lp, DTYPE_PROCESS, " pr_FileSystemTask = %08X ; File handler APROCESS for current drive", p->pr_FileSystemTask);
        SprintfDLIST(lp, DTYPE_PROCESS, " pr_CLI            = %08X ; pointer to CommandLineInterface", BADDR(p->pr_CLI));
        SprintfDLIST(lp, DTYPE_PROCESS, " pr_ReturnAddr     = %08X ; pointer to previous stack frame", p->pr_ReturnAddr);
        SprintfDLIST(lp, DTYPE_PROCESS, " pr_PktWait        = %08X ; To be called when awaiting msg", p->pr_PktWait);
        SprintfDLIST(lp, DTYPE_PROCESS, " pr_WindowPtr      = %08X ; Window for error printing", p->pr_WindowPtr);
        SprintfDLIST(lp, DTYPE_PROCESS, " pr_HomeDir        = %08X ; Home directory of program", BADDR(p->pr_HomeDir));
        SprintfDLIST(lp, DTYPE_PROCESS, " pr_Flags          = %08X ; Flags telling dos about APROCESS", p->pr_Flags);
        SprintfDLIST(lp, DTYPE_PROCESS, " pr_ExitCode       = %08X ; Code to call on exit of program or NULL", p->pr_ExitCode);
        SprintfDLIST(lp, DTYPE_PROCESS, " pr_ExitData       = %08X ; Passed as an argument to pr_ExitCode", p->pr_ExitData);
        SprintfDLIST(lp, DTYPE_PROCESS, " pr_Arguments      = %08X ; Arguments passed to APROCESS at start", p->pr_Arguments);
#if 0
    struct MinList pr_LocalVars; ; Local environment variables
#endif
        SprintfDLIST(lp, DTYPE_PROCESS, " pr_ShellPrivate   = %08X ; for the use of the current shell", p->pr_ShellPrivate);
        SprintfDLIST(lp, DTYPE_PROCESS, " pr_CES            = %08X ; Error stream - if NULL, use pr_COS", BADDR(p->pr_CES));
}

/************************************************************************/

/* EXEC */

void    ListInfo(DBugDisp *dp) {
        LIST            *lp = &dp->ds_List;
        struct Task     *t = SysBase->ThisTask;

        FreeDLIST(&dp->ds_List);
        SetDisplayMode(DISPLAY_INFO,0);
        SprintfDLIST(&dp->ds_List, DTYPE_INFO, "Task Structure");
        SprintfDLIST(&dp->ds_List, DTYPE_INFO, " tc_Flags      = %02X      ", (UBYTE)t->tc_Flags);
        SprintfDLIST(&dp->ds_List, DTYPE_INFO, " tc_State      = %02X      ", (UBYTE)t->tc_State);
        SprintfDLIST(&dp->ds_List, DTYPE_INFO, " tc_IDNestCnt  = %02X       ; intr disabled nesting", (UBYTE)t->tc_IDNestCnt);
        SprintfDLIST(&dp->ds_List, DTYPE_INFO, " tc_TDNestCnt  = %02X       ; task disabled nesting", (UBYTE)t->tc_TDNestCnt);
        SprintfDLIST(&dp->ds_List, DTYPE_INFO, " tc_SigAlloc   = %08X ; sigs allocated", t->tc_SigAlloc);
        SprintfDLIST(&dp->ds_List, DTYPE_INFO, " tc_SigWait    = %08X ; sigs we are waiting for", t->tc_SigWait);
        SprintfDLIST(&dp->ds_List, DTYPE_INFO, " tc_SigRecvd   = %08X ; sigs we have received", t->tc_SigRecvd);
        SprintfDLIST(&dp->ds_List, DTYPE_INFO, " tc_SigExcept  = %08X ; sigs we will take excepts for", t->tc_SigExcept);
        SprintfDLIST(&dp->ds_List, DTYPE_INFO, " tc_TrapAlloc  = %04X     ; traps allocated", t->tc_TrapAlloc);
        SprintfDLIST(&dp->ds_List, DTYPE_INFO, " tc_TrapAble   = %04X     ; traps enabled", t->tc_TrapAble);
        SprintfDLIST(&dp->ds_List, DTYPE_INFO, " tc_ExceptData = %08X ; points to except data", t->tc_ExceptData);
        SprintfDLIST(&dp->ds_List, DTYPE_INFO, " tc_ExceptCode = %08X ; points to except code", t->tc_ExceptCode);
        SprintfDLIST(&dp->ds_List, DTYPE_INFO, " tc_TrapData   = %08X ; points to trap data", t->tc_TrapData);
        SprintfDLIST(&dp->ds_List, DTYPE_INFO, " tc_TrapCode   = %08X ; points to trap code", t->tc_TrapCode);
        SprintfDLIST(&dp->ds_List, DTYPE_INFO, " tc_SPReg      = %08X ; stack pointer", t->tc_SPReg);
        SprintfDLIST(&dp->ds_List, DTYPE_INFO, " tc_SPLower    = %08X ; stack lower bound", t->tc_SPLower);
        SprintfDLIST(&dp->ds_List, DTYPE_INFO, " tc_SPUpper    = %08X ; stack upper bound + 2", t->tc_SPUpper);
        SprintfDLIST(&dp->ds_List, DTYPE_INFO, " tc_Switch     = %08X ; task losing CPU", t->tc_Switch);
        SprintfDLIST(&dp->ds_List, DTYPE_INFO, " tc_Launch     = %08X ; task getting CPU", t->tc_Launch);
        SprintfDLIST(&dp->ds_List, DTYPE_INFO, " tc_MemEntry   = %08X ; Allocated memory. Freed by RemTask()", t->tc_MemEntry);
        SprintfDLIST(&dp->ds_List, DTYPE_INFO, " tc_UserData   = %08X ; For use by the task; no restrictions!");

        SprintfDLIST(&dp->ds_List, DTYPE_INFO, "");
//      SprintfDLIST(&dp->ds_List, DTYPE_INFO, "Command Line: '%s'", currentContext->commandLine);
//      SprintfDLIST(&dp->ds_List, DTYPE_INFO, "Program is %d segments", currentContext->segCount);
}

/************************************************************************/

void    ListExecBase(DBugDisp *dp) {
        LIST            *lp = &dp->ds_List;
        struct ExecBase *s = SysBase;

        FreeDLIST(&dp->ds_List);
        SetDisplayMode(DISPLAY_EXECBASE,0);
        SprintfDLIST(&dp->ds_List, DTYPE_EXECBASE, "struct ExecBase:");
        SprintfDLIST(&dp->ds_List, DTYPE_EXECBASE, "SoftVer              =     %04X ; kickstart release number (obs.)", s->SoftVer);
        SprintfDLIST(&dp->ds_List, DTYPE_EXECBASE, "LowMemChkSum         =     %04X ; checksum of 68000 trap vectors", s->LowMemChkSum);
        SprintfDLIST(&dp->ds_List, DTYPE_EXECBASE, "ChkBase              = %08X ; system base pointer complement", s->ChkBase);
        SprintfDLIST(&dp->ds_List, DTYPE_EXECBASE, "ColdCapture          = %08X ; coldstart soft capture vector", s->ColdCapture);
        SprintfDLIST(&dp->ds_List, DTYPE_EXECBASE, "CoolCapture          = %08X ; coolstart soft capture vector", s->CoolCapture);
        SprintfDLIST(&dp->ds_List, DTYPE_EXECBASE, "WarmCapture          = %08X ; warmstart soft capture vector", s->WarmCapture);
        SprintfDLIST(&dp->ds_List, DTYPE_EXECBASE, "SysStkUpper          = %08X ; system stack base   (upper bound)", s->SysStkUpper);
        SprintfDLIST(&dp->ds_List, DTYPE_EXECBASE, "SysStkLower          = %08X ; top of system stack (lower bound)", s->SysStkLower);
        SprintfDLIST(&dp->ds_List, DTYPE_EXECBASE, "MaxLocMem            = %08X ; top of chip memory", s->MaxLocMem);
        SprintfDLIST(&dp->ds_List, DTYPE_EXECBASE, "DebugEntry           = %08X ; global debugger entry point", s->DebugEntry);
        SprintfDLIST(&dp->ds_List, DTYPE_EXECBASE, "DebugData            = %08X ; global debugger data segment", s->DebugData);
        SprintfDLIST(&dp->ds_List, DTYPE_EXECBASE, "AlertData            = %08X ; alert data segment", s->AlertData);
        SprintfDLIST(&dp->ds_List, DTYPE_EXECBASE, "MaxExtMem            = %08X ; top of extended mem, or null if none", s->MaxExtMem);
        SprintfDLIST(&dp->ds_List, DTYPE_EXECBASE, "ChkSum               =     %04X ; for all of the above (minus 2)", s->ChkSum);

#if 0
        struct  IntVector IntVects[16];
#endif

        SprintfDLIST(&dp->ds_List, DTYPE_EXECBASE, "ThisTask             = %08X ; pointer to current task (readable)", s->ThisTask);
        SprintfDLIST(&dp->ds_List, DTYPE_EXECBASE, "IdleCount            = %08X ; idle counter", s->IdleCount);
        SprintfDLIST(&dp->ds_List, DTYPE_EXECBASE, "DispCount            = %08X ; dispatch counter", s->DispCount);
        SprintfDLIST(&dp->ds_List, DTYPE_EXECBASE, "Quantum              =     %04X ; time slice quantum", s->Quantum);
        SprintfDLIST(&dp->ds_List, DTYPE_EXECBASE, "Elapsed              =     %04X ; current quantum ticks", s->Elapsed);
        SprintfDLIST(&dp->ds_List, DTYPE_EXECBASE, "SysFlags             =     %04X ; misc internal system flags", s->SysFlags);
        SprintfDLIST(&dp->ds_List, DTYPE_EXECBASE, "IDNestCnt            =       %02X ; interrupt disable nesting count", (UBYTE)s->IDNestCnt);
        SprintfDLIST(&dp->ds_List, DTYPE_EXECBASE, "TDNestCnt            =       %02X ; task disable nesting count", (UBYTE)s->TDNestCnt);

        SprintfDLIST(&dp->ds_List, DTYPE_EXECBASE, "AttnFlags            =     %04X ; special attention flags (readable)", s->AttnFlags);

        SprintfDLIST(&dp->ds_List, DTYPE_EXECBASE, "AttnResched          =     %04X ; rescheduling attention", s->AttnResched);
        SprintfDLIST(&dp->ds_List, DTYPE_EXECBASE, "ResModules           = %08X ; resident module array pointer", s->ResModules);
        SprintfDLIST(&dp->ds_List, DTYPE_EXECBASE, "TaskTrapCode         = %08X", s->TaskTrapCode);
        SprintfDLIST(&dp->ds_List, DTYPE_EXECBASE, "TaskExceptCode       = %08X", s->TaskExceptCode);
        SprintfDLIST(&dp->ds_List, DTYPE_EXECBASE, "TaskExitCode         = %08X", s->TaskExitCode);
        SprintfDLIST(&dp->ds_List, DTYPE_EXECBASE, "TaskSigAlloc         = %08X", s->TaskSigAlloc);
        SprintfDLIST(&dp->ds_List, DTYPE_EXECBASE, "TaskTrapAlloc        =     %04X ; System Lists (private!)", s->TaskTrapAlloc);

#if 0
        struct  List MemList;
        struct  List ResourceList;
        struct  List DeviceList;
        struct  List IntrList;
        struct  List LibList;
        struct  List PortList;
        struct  List TaskReady;
        struct  List TaskWait;
#endif

#if 0
        struct  SoftIntList SoftInts[5];
#endif

#if 0
        LONG    LastAlert[4];
#endif

        SprintfDLIST(&dp->ds_List, DTYPE_EXECBASE, "VBlankFrequency      =       %02X ; (readable)", (UBYTE)s->VBlankFrequency);
        SprintfDLIST(&dp->ds_List, DTYPE_EXECBASE, "PowerSupplyFrequency =       %02X ; (readable)", (UBYTE)s->PowerSupplyFrequency);

#if 0
        struct  List SemaphoreList;
#endif

        SprintfDLIST(&dp->ds_List, DTYPE_EXECBASE, "KickMemPtr           = %08X ; ptr to queue of mem lists", s->KickMemPtr);
        SprintfDLIST(&dp->ds_List, DTYPE_EXECBASE, "KickTagPtr           = %08X ; ptr to rom tag queue", s->KickTagPtr);
        SprintfDLIST(&dp->ds_List, DTYPE_EXECBASE, "KickCheckSum         = %08X ; checksum for mem and tags", s->KickCheckSum);
        SprintfDLIST(&dp->ds_List, DTYPE_EXECBASE, "ex_RamLibPrivate     = %08X", s->ex_RamLibPrivate);
        SprintfDLIST(&dp->ds_List, DTYPE_EXECBASE, "ex_EClockFrequency   = %08X ; (readable)", s->ex_EClockFrequency);
        SprintfDLIST(&dp->ds_List, DTYPE_EXECBASE, "ex_CacheControl      = %08X ; Private to CacheControl calls", s->ex_CacheControl);
        SprintfDLIST(&dp->ds_List, DTYPE_EXECBASE, "ex_TaskID            = %08X ; Next available task ID", s->ex_TaskID);

        SprintfDLIST(&dp->ds_List, DTYPE_EXECBASE, "ex_PuddleSize        = %08X", s->ex_PuddleSize);
        SprintfDLIST(&dp->ds_List, DTYPE_EXECBASE, "ex_PoolThreshold     = %08X", s->ex_PoolThreshold);
#if 0
        struct  MinList ex_PublicPool;
#endif

        SprintfDLIST(&dp->ds_List, DTYPE_EXECBASE, "ex_MMULock           = %08X ; private", s->ex_MMULock);

#if 0
        UBYTE   ex_Reserved[12];
#endif
}

void    BuildDLIST(LIST *dst, LIST *src, ULONG type) {
        struct Node     *n;
        char            *ps, *pd, buf[256];

        for (n=src->lh_Head; n->ln_Succ; n=n->ln_Succ) {
                if (n->ln_Name) {
                        ps = n->ln_Name; pd = &buf[0];
                        while (*ps) {
                                if (*ps == 0x0d || *ps == 0x0a) ps++; else *pd++ = *ps++;
                        }
                        *pd = '\0';
                }
                else
                        strcpy(buf, "<NO NAME>");
                SprintfDLIST(dst, type, "%08X %s", n, buf);
        }
}

/************************************************************************/


void    ListResources(DBugDisp *dp) {
        FreeDLIST(&dp->ds_List);
        SprintfDLIST(&dp->ds_List, DTYPE_RESOURCES, "RESOURCE LIST");
        BuildDLIST(&dp->ds_List, &SysBase->ResourceList, DTYPE_RESOURCES);
}

void    ListIntrs(DBugDisp *dp) {
        FreeDLIST(&dp->ds_List);
        SprintfDLIST(&dp->ds_List, DTYPE_INTRS, "INTERRUPT LIST");
        BuildDLIST(&dp->ds_List, &SysBase->IntrList, DTYPE_INTRS);
}

void    ListPorts(DBugDisp *dp) {
        struct List     *list = &SysBase->PortList;
        short           count;
        struct Node     *n, *n2;
        char            *ps, *pd, buf[256];
        struct MsgPort  *mp;
        static char     *actionTable[] = { "SIGNAL ", "SOFTINT", "IGNORE "};

        FreeDLIST(&dp->ds_List);
        SetDisplayMode(DISPLAY_PORTS,0);
        SprintfDLIST(&dp->ds_List, DTYPE_PORTS, "PORTS LIST");
        SprintfDLIST(&dp->ds_List, DTYPE_PORTS, "ADDRESS  ACTION  BIT# TASK     MSGS NAME");
        for (n=list->lh_Head; n->ln_Succ; n=n->ln_Succ) {
                mp = (struct MsgPort *)n;
                if (n->ln_Name) ps = n->ln_Name;
                else ps = "<NO NAME>";
                pd = &buf[0];
                while (*ps) {
                        if (*ps == 0x0d || *ps == 0x0a) ps++; else *pd++ = *ps++;
                }
                *pd = '\0';
                // count the number of messages waiting at the port
                for (count=0, n2 = mp->mp_MsgList.lh_Head; n2->ln_Succ; n2=n2->ln_Succ) count++;
                SprintfDLIST(&dp->ds_List, DTYPE_PORTS, "%08X %s  %2d  %08x %4d %s",
                        n,
                        actionTable[mp->mp_Flags&PF_ACTION],
                        (UBYTE)mp->mp_SigBit,
                        mp->mp_SigTask,
                        count,
                        buf
                );
        }
}

static char     *stateTable[] = {
        "TS_INVALID",
        "TS_ADDED  ",
        "TS_RUN    ",
        "TS_READY  ",
        "TS_WAIT   ",
        "TS_EXCEPT ",
        "TS_REMOVED",
};

void    ListTasks(DBugDisp *dp) {
        struct List     *list;
        short           i;
        struct Node     *n;
        char            *ps, *pd, buf[256];
        struct Task     *t;

        FreeDLIST(&dp->ds_List);
        SetDisplayMode(DISPLAY_TASKS,0);
        SprintfDLIST(&dp->ds_List, DTYPE_TASKS, "TASK LIST");
        SprintfDLIST(&dp->ds_List, DTYPE_TASKS, "  ADDRESS  LIST  STATE      SIGNALS NAME");
        for (i=0; i<2; i++) {
                list = i ? &SysBase->TaskWait : &SysBase->TaskReady;
                for (n=list->lh_Head; n->ln_Succ; n=n->ln_Succ) {
                        t = (struct Task *)n;
                        if (n->ln_Name) ps = n->ln_Name;
                        else ps = "<NO NAME>";
                        pd = &buf[0];
                        while (*ps) {
                                if (*ps == 0x0d || *ps == 0x0a) ps++; else *pd++ = *ps++;
                        }
                        *pd = '\0';
                        SprintfDLIST(&dp->ds_List, DTYPE_TASKS, "%c %08X %s %s %08X  %s",
                                (t == SysBase->ThisTask) ? 0x8d : ' ',
                                n,
                                i ? "WAIT " : "READY",
                                stateTable[t->tc_State],
                                t->tc_SigWait|t->tc_SigExcept,
                                buf
                        );
                }
        }
        n = (struct Node *)SysBase->ThisTask;
        t = (struct Task *)n;
        if (n->ln_Name) ps = n->ln_Name;
        else ps = "<NO NAME>";
        pd = &buf[0];
        while (*ps) {
                if (*ps == 0x0d || *ps == 0x0a) ps++; else *pd++ = *ps++;
        }
        *pd = '\0';
        SprintfDLIST(&dp->ds_List, DTYPE_TASKS, "%c %08X %s %s %08X  %s",
                (t == SysBase->ThisTask) ? 0x8d : ' ',
                n,
                i ? "WAIT " : "READY",
                stateTable[t->tc_State],
                t->tc_SigWait|t->tc_SigExcept,
                buf
        );
}

void    ListLibs(DBugDisp *dp) {
        struct List     *list = &SysBase->LibList;
        struct Node     *n;
        char            *ps, *pd, buf[256];
        struct Library  *lp;

        FreeDLIST(&dp->ds_List);
        SetDisplayMode(DISPLAY_LIBS,0);
        SprintfDLIST(&dp->ds_List, DTYPE_LIBS, "LIBRARY LIST");
        SprintfDLIST(&dp->ds_List, DTYPE_LIBS, "ADDRESS  OPENS VERSION NAME");
        for (n=list->lh_Head; n->ln_Succ; n=n->ln_Succ) {
                lp = (struct Library *)n;
                if (lp->lib_IdString) ps = lp->lib_IdString;
                else if (n->ln_Name) ps = n->ln_Name;
                else ps = "<NO NAME>";
                pd = &buf[0];
                while (*ps) {
                        if (*ps == 0x0d || *ps == 0x0a) ps++; else *pd++ = *ps++;
                }
                *pd = '\0';
                SprintfDLIST(&dp->ds_List, DTYPE_LIBS, "%08X %5d %3d.%-3d %s",
                        n,
                        lp->lib_OpenCnt,
                        lp->lib_Version, lp->lib_Revision,
                        buf
                );
        }
}

void    ListDevices(DBugDisp *dp) {
        struct List     *list = &SysBase->DeviceList;
        struct Node     *n;
        char            *ps, *pd, buf[256];
        struct Library  *lp;

        FreeDLIST(&dp->ds_List);
        SetDisplayMode(DISPLAY_DEVICES,0);
        SprintfDLIST(&dp->ds_List, DTYPE_DEVICES, "DEVICE LIST");
        SprintfDLIST(&dp->ds_List, DTYPE_DEVICES, "ADDRESS  OPENS VERSION NAME");
        for (n=list->lh_Head; n->ln_Succ; n=n->ln_Succ) {
                lp = (struct Library *)n;
                if (lp->lib_IdString) ps = lp->lib_IdString;
                else if (n->ln_Name) ps = n->ln_Name;
                else ps = "<NO NAME>";
                pd = &buf[0];
                while (*ps) {
                        if (*ps == 0x0d || *ps == 0x0a) ps++; else *pd++ = *ps++;
                }
                *pd = '\0';
                SprintfDLIST(&dp->ds_List, DTYPE_DEVICES, "%08X %5d %3d.%-3d %s",
                        n,
                        lp->lib_OpenCnt,
                        lp->lib_Version, lp->lib_Revision,
                        buf
                );
        }
}

void    ListMemList(DBugDisp *dp) {
        struct List             *list = &SysBase->MemList;
        struct Node             *n;
        char                    *ps, *pd, buf[256];
        struct MemHeader        *mh;
        struct MemChunk         *mc;
        ULONG                   largest, count;

        FreeDLIST(&dp->ds_List);
        dp->ds_DisplayMode = DISPLAY_MEMLIST;
        SetDisplayMode(DISPLAY_MEMLIST,0);
        SprintfDLIST(&dp->ds_List, DTYPE_MEMLIST, "MEMORY LIST");
        for (n=list->lh_Head; n->ln_Succ; n=n->ln_Succ) {
                if (n->ln_Name) {
                        ps = n->ln_Name; pd = &buf[0];
                        while (*ps) {
                                if (*ps == 0x0d || *ps == 0x0a) ps++; else *pd++ = *ps++;
                        }
                        *pd = '\0';
                }
                else
                        strcpy(buf, "<NO NAME>");
                mh = (struct MemHeader *)n;
                SprintfDLIST(&dp->ds_List, DTYPE_MEMLIST, "$%08X %08X-%08X %d %s",
                        n,
                        mh->mh_Lower,
                        mh->mh_Upper,
                        mh->mh_Free,
                        buf
                );
                strcpy(buf, "");
                if (mh->mh_Attributes & MEMF_PUBLIC) strcat(buf, "MEMF_PUBLIC ");
                if (mh->mh_Attributes & MEMF_CHIP) strcat(buf, "MEMF_CHIP ");
                if (mh->mh_Attributes & MEMF_FAST) strcat(buf, "MEMF_FAST ");
                if (mh->mh_Attributes & MEMF_LOCAL) strcat(buf, "MEMF_LOCAL ");
                if (mh->mh_Attributes & MEMF_24BITDMA) strcat(buf, "MEMF_24BITDMA ");
                SprintfDLIST(&dp->ds_List, DTYPE_MEMLIST, "  Satisfies %s", buf);

                largest = 0; count = 0;
                for (mc = mh->mh_First; mc; mc = mc->mc_Next) {
                        count++;
                        if (mc->mc_Bytes > largest) largest = mc->mc_Bytes;
                }
                SprintfDLIST(&dp->ds_List, DTYPE_MEMLIST, "  %d fragments, largest is %d bytes ", count, largest);
        }
}
#if 0
Local int my_comp(char **s1, char **s2);
int my_comp(char **s1, char **s2) {
      return(strcmp(*s1, *s2));
}
#endif

Local int my_comp(SYMLIST **arg1, SYMLIST **arg2);
int int my_comp(SYMLIST **arg1, SYMLIST **arg2)
{
ULONG *s1, *s2, len1, len2;

s1 = ((SYMLIST *)arg1)[0].symbolname;
s2 = ((SYMLIST *)arg2)[0].symbolname;

len1 = 4 * (*s1++);
len2 = 4 * (*s2++);

    if(len2 < len1)return -strncmp((char *)s2,(char *)s1,len2);
    return strncmp((char *)s1,(char *)s2,len1);
}

// Build a sorted Symbol List
BOOL    ListSymbols(DBugDisp *dp) {
        LIST            *lp = &dp->ds_List;
        int i, len, symcount = CountSymbols();
        SYMLIST *symbols;
        char buffer[128];
        ULONG *name;

    if((symbols= malloc(symcount * sizeof(SYMLIST)))) {
        FreeDLIST(lp);
        SetDisplayMode(DISPLAY_SYMLIST,0);
        CopySymbols(symbols);

        qsort(symbols, symcount, sizeof(SYMLIST), my_comp);
        SprintfDLIST(lp, DTYPE_SYMLIST, "SORTED SYMBOL LIST");
        for (i = 0; i < symcount; ++i) {
            name = symbols[i].symbolname;
            len = *name++;
            memset(buffer,0,128);
            strncpy(buffer,(char *)name,4*len);
            SprintfDLIST(lp, DTYPE_SYMLIST, "%08X %-16.16s", symbols[i].address, buffer);
        }
        free(symbols);
    }
}
