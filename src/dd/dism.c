/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */
#include        "defs.h"
#include        "dbug_protos.h"

// ************************************************************************

Prototype BOOL          DownDism(void);
Prototype BOOL          UpDism(void);
Prototype BOOL          PageDownDism(void);
Prototype BOOL          PageUpDism(void);
Prototype WORD          RefreshDism(short maxLines, BOOL fullRefresh);
Prototype ULONG         PreviousInstruction(ULONG addr);

// ************************************************************************

BOOL    DownDism(void) {
        ULONG   val;

        val = Disassemble(CurDisplay->ds_WindowTop, CurDisplay->ds_WindowTop, LineBuf);
        CurDisplay->ds_WindowTop += val;

        if (CurDisplay->ds_DisplayMode != CurDisplay->ds_LastRefreshMode) {
                RefreshWindow(1);
                return TRUE;
        }
        CurDisplay->ds_LastRefreshTop = CurDisplay->ds_WindowTop;
        ScrScrollup();

        CurDisplay->ds_WindowBot += Disassemble(CurDisplay->ds_WindowBot, CurDisplay->ds_WindowBot, LineBuf);
        Disassemble(CurDisplay->ds_WindowBot, CurDisplay->ds_WindowBot, LineBuf);   // actual next inst

        if (CurDisplay->ds_WindowBot == programPC) ScrHighlight();
        if (IsBreakpoint(CurDisplay->ds_WindowBot)) ScrUnderline();
        PrintAddress(CurDisplay->ds_WindowBot);
        if (!CurDisplay->ds_DisplayOffsets) {
                char    *s;

                s = LookupValue(CurDisplay->ds_WindowBot);
                if (!s) s = "";
                ScrPrintf("%-16.16s %s", s, LineBuf);
        }
        else ScrPuts(LineBuf);
        ScrEOL();
        ScrPlain();
        RefreshWindow(FALSE);
        return TRUE;
}

BOOL    UpDism(void) {

        if (CurDisplay->ds_DisplayMode != CurDisplay->ds_LastRefreshMode) {
                RefreshWindow(TRUE);
                return TRUE;
        }

        CurDisplay->ds_LastRefreshTop = CurDisplay->ds_WindowTop = PreviousInstruction(CurDisplay->ds_WindowTop);

        ScrScrolldown();

        if (CurDisplay->ds_WindowTop == programPC) ScrHighlight();
        if (IsBreakpoint(CurDisplay->ds_WindowTop)) ScrUnderline();
        PrintAddress(CurDisplay->ds_WindowTop);
        if (!CurDisplay->ds_DisplayOffsets) {
                char    *s;

                s = LookupValue(CurDisplay->ds_WindowTop);
                if (!s) s = "";
                ScrPrintf("%-16.16s %s", s, LineBuf);
        }
        else ScrPuts(LineBuf);
        ScrEOL();
        ScrPlain();
        RefreshWindow(FALSE);
        return TRUE;
}


BOOL    PageUpDism(void) {
        ULONG address = CurDisplay->ds_WindowTop;
        int i,  lines = CalcDisplayLines();

        if (CurDisplay->ds_DisplayMode != CurDisplay->ds_LastRefreshMode) {
                RefreshWindow(TRUE);
                return TRUE;
        }

        if(lines > 0) {
            for(i=0; i<lines; i++)address = PreviousInstruction(address);
            CurDisplay->ds_WindowTop = address;
            CurDisplay->ds_WindowTopLine = 0;
            RefreshWindow(1);
        }
        return TRUE;
}

BOOL    PageDownDism(void) {
        if (CurDisplay->ds_DisplayMode != CurDisplay->ds_LastRefreshMode) {
                RefreshWindow(TRUE);
                return TRUE;
        }
        CurDisplay->ds_WindowTop = CurDisplay->ds_WindowBot;
        CurDisplay->ds_WindowTopLine = 0;
        RefreshWindow(1);
        return TRUE;
}

WORD    RefreshDism(short maxLines, BOOL fullRefresh) {
        ULONG   address = CurDisplay->ds_WindowTop, size;
        WORD    count = 0;
        char    *s;
        BOOL    plotFlag;

        

        do_scroller();
//      if (fullRefresh) {
            if (fullRefresh)SetTitle(NULL,NULL);
#if 0
            ScrPlain();
            ScrInverse();
            ScrPuts("DISM MODE");
#endif
            // add a seperation between register display and source

            if(CurDisplay->ds_RegFlag) {
                ++count;
                Newline();
                if (--maxLines == 0)return count;
            }
//      }


        while (maxLines > 0) {
                plotFlag = fullRefresh;
                CurDisplay->ds_WindowBot = address;
                size = Disassemble(address, address, LineBuf);
                ScrPlain();

                plotFlag = TRUE; 

                if (address == programPC) { 
                        plotFlag = TRUE; 
                        ScrHighlight(); 
                }
                if (IsBreakpoint(address)) { 
                        plotFlag = TRUE; 
                        ScrUnderline(); 
                }
                if (address == lastPC) { 
                        plotFlag = TRUE; 
                        ScrPlain(); 
                }
                if (plotFlag) {
                        PrintAddress(address);
                        if (!CurDisplay->ds_DisplayOffsets) {
                                s = LookupValue(address);
                                if (!s) s = "";
                                ScrPrintf("%-16.16s %s", s, LineBuf);
                        }
                        else ScrPuts(LineBuf);
                        count++;
                        ScrPutNewline(); 
                        maxLines--;
                }
                else {
                        count++;
                        ScrPutNewline();
                        maxLines--;
                }
                address += size;
        }
        return count;
}

/*  Attempt to find previous instruction. This is done by
 *  searching forward from the nearest label address or
 *  20 words back, whichever is closer.
 */

ULONG   PreviousInstruction(ULONG addr) {
        ULONG val;
        ULONG siz;
        char *s;

        if (s = NearestValue(addr-2)) {
                LookupSymbol(s, &val);
                if ((long)(addr - val) > 40 || (long)(addr - val) <= 0)
                    val = addr - 40;
        }
        else val = addr - 40;

        siz = 0;
        while (val + siz < addr) {
            val = val + siz;
            siz = Disassemble(val, val, LineBuf);
            if ((long)siz <= 0)siz = 2;         //  if siz is weird
        }
        return(val);
}

