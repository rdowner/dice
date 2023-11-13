/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */
#include        "defs.h"
#include        "dbug_protos.h"

// ************************************************************************

Prototype WORD          RefreshBreakpoints(WORD maxLines, BOOL fullRefresh);
Prototype BOOL          UpBreak(void);
Prototype BOOL          DownBreak(void);

Prototype void          InitBreakpoints(void);
Prototype void          InstallBreakpoints(void);
Prototype void          CheckBreakpoints(void);
Prototype BOOL          IsBreakpoint(ULONG address);
Prototype BOOL          SetBreakpoint(ULONG address, UWORD count, UWORD type);
Prototype BOOL          ClearBreakpoint(ULONG address);
Prototype void          SetTempBreakpoint(ULONG address);
Prototype void          SetAllBreakpoints(void);
Local     BOOL          setbp(short i, ULONG address, UWORD count, UWORD type);

// ************************************************************************

Prototype WORD topBP;

BP      bpTable[MAXBP];                 // 32 user breakpoints
BP      bpTemp;                         // temporary breakpoint
WORD    topBP = 0;                      // top breakpoint displayed from table

BOOL    UpBreak(void) {

        if(topBP > 0) {
            ScrScrolldown();
            topBP--;
            RefreshWindow(FALSE);
        }
        return(TRUE);
}

BOOL    DownBreak(void) {
        if(topBP < (MAXBP-1)) {
            ScrScrollup();
            topBP++;
            RefreshWindow(FALSE);
        }
        return(TRUE);
}

WORD    RefreshBreakpoints(WORD maxLines, BOOL fullRefresh) {
        WORD            i = MAXBP - topBP;
        static char     *stateTable[] = { "UNSET", "SET  ", "GROUP" };
        WORD            count = 0;

        do_scroller();
        if (fullRefresh != -1)SetTitle(NULL,NULL);
        SetTitle(NULL,NULL);
        ScrPlain(); 
        ScrInverse();
        ScrPuts("NUM STATE ADDRESS  SYMBOL NAME      SAVE COUNT");
//               xxx GROUP xxxxxxxx xxxxxxxxxxxxxxxx xxxx xxxxx
        ScrPlain(); 
        count++; 
        ScrPutNewline(); 
        maxLines--;

        while (topBP && i < maxLines) {
                topBP--;
                i = MAXBP - topBP;
        }
        for (i=topBP; i<MAXBP && maxLines > 0; i++, maxLines--) {
                char *s = LookupValue((ULONG)bpTable[i].address);
                if (!s) s = "";
                ScrPrintf("%3d %-5.5s %08x %-16.16s %04x %5d",
                        i,
                        stateTable[bpTable[i].state],
                        bpTable[i].address,
                        s,
                        bpTable[i].value,
                        bpTable[i].count
                );
                count++; 
                ScrPutNewline();
        }
        ScrPlain();
        return count;
}

void    InitBreakpoints(void) {
        short   i;

        for (i=0; i<MAXBP; i++) bpTable[i].state = BP_UNSET;
        bpTemp.state = BP_UNSET;
}

void    SetAllBreakpoints(void) {
        short   i;

        for (i=0; i<MAXBP; i++) {
                if ((bpTable[i].state == BP_UNSET) && (bpTable[i].address)) {
                        bpTable[i].state = BP_SET;
                        bpTable[i].count = 1;
                }
        }
}

void    InstallBreakpoints(void) {
        short   i;

        // better flush those caches, this is like self modifying code
        if(((struct Library *)SysBase)->lib_Version >= 36)CacheClearU();
        for (i=0; i<MAXBP; i++) {
                if (bpTable[i].state != BP_UNSET) *bpTable[i].address = 0x4afc; // ILLEGAL
        }
        if (bpTemp.state != BP_UNSET) *bpTemp.address = 0x4afc;
        if(((struct Library *)SysBase)->lib_Version >= 36)CacheClearU();
}

void    CheckBreakpoints(void) {
        short   i;

        for (i=0; i<MAXBP; i++) {
                if (bpTable[i].state != BP_UNSET) {
                        *bpTable[i].address = bpTable[i].value;
                        if (programPC == (ULONG)bpTable[i].address) {
                                programState = STATE_BREAKPOINT;
                                if (!(--bpTable[i].count)) {
                                    bpTable[i].state = BP_UNSET;
                                }
                        }
                        if (bpTable[i].state == BP_GROUP) {
                            bpTable[i].state = BP_UNSET;
                        }
                }
        }
        if (bpTemp.state != BP_UNSET) {
                *bpTemp.address = bpTemp.value;
                if (programPC == (ULONG)bpTemp.address) programState = STATE_STEPPEDOVER;
                bpTemp.state = BP_UNSET;
        }
        if (programPC == (ULONG)TargetExit) programState = STATE_EXITED;
}

BOOL    IsBreakpoint(ULONG address) {
        short   i;

        if(address = ValidMemCheck(address)) {  
            for (i=0; i<MAXBP; i++) {
                if (bpTable[i].state != BP_UNSET && address == (ULONG)bpTable[i].address) return TRUE;
            }
        }
        return FALSE;
}

BOOL    SetBreakpoint(ULONG address, UWORD count, UWORD type) {
short i;

    if(address = ValidMemCheck(address)) {
        for (i=0; i<MAXBP; i++) {
            // check for first completely empty slot first
            if ((bpTable[i].state == BP_UNSET) && !bpTable[i].address || ((ULONG)bpTable[i].address == address))
                return setbp(i, address, count, type);
        }
        for (i=0; i<MAXBP; i++) {       // try for the first unset breakpoint
            if (bpTable[i].state == BP_UNSET)return setbp(i, address, count, type);
        }
    }
    return FALSE;
}

BOOL    setbp(short i, ULONG address, UWORD count, UWORD type) {
        bpTable[i].state = type;
        bpTable[i].address = (UWORD *)address;
        bpTable[i].value = *bpTable[i].address;
        bpTable[i].count = count;
        return TRUE;
}

BOOL    ClearBreakpoint(ULONG address) {
        short   i;

        for (i=0; i<MAXBP; i++) {
                if (bpTable[i].state != BP_UNSET && address == (ULONG)bpTable[i].address) {
                        bpTable[i].state = BP_UNSET;
                        return TRUE;
                }
        }
        return FALSE;
}

void    SetTempBreakpoint(ULONG address) {
        bpTemp.state = BP_SET;
        bpTemp.address = (UWORD *)address;
        bpTemp.value = *bpTemp.address;
}


