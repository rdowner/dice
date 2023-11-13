/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */
#include        "defs.h"
#include        "dbug_protos.h"

// ************************************************************************

Prototype BOOL          DownMixed(void);
Prototype BOOL          UpMixed(void);
Prototype BOOL          PageDownMixed(void);
Prototype BOOL          PageUpMixed(void);
Prototype BOOL          DownSource(void);
Prototype BOOL          UpSource(void);
Prototype BOOL          PageDownSource(void);
Prototype BOOL          PageUpSource(void);
Prototype WORD          RefreshMixed(WORD maxLines, BOOL fullRefresh, ULONG, ULONG, WORD);

Prototype WORD          PrevMixedLine(ULONG *addr, ULONG *line, LONG *info);
Prototype WORD          NextMixedLine(ULONG *addr, ULONG *line, LONG *info);
Prototype WORD          CurrentMixedLine(ULONG *addr, ULONG *line, LONG *info);


// ************************************************************************
//
//  Mixed mode utilizes both address and subline where as disassembly mode
//  utilizes only the address.  Source mode utilizes only the subline but
//  must track addresses when a line group is completed.

BOOL    UpMixed(void) {

        PrevMixedLine(&CurDisplay->ds_WindowTop, &CurDisplay->ds_WindowTopLine, NULL);
        PrevMixedLine(&CurDisplay->ds_WindowBot, &CurDisplay->ds_WindowBotLine, NULL);
        ScrScrolldown();
        RefreshMixed(1, -1, CurDisplay->ds_WindowTop, CurDisplay->ds_WindowTopLine, 1);

        return TRUE;
}

BOOL    DownMixed(void) {

        NextMixedLine(&CurDisplay->ds_WindowTop, &CurDisplay->ds_WindowTopLine, NULL);
        NextMixedLine(&CurDisplay->ds_WindowBot, &CurDisplay->ds_WindowBotLine, NULL);
        ScrScrollup();
        RefreshMixed(1, -1, CurDisplay->ds_WindowBot, CurDisplay->ds_WindowBotLine, 1);
        return TRUE;
}

BOOL    PageDownMixed(void) {
        CurDisplay->ds_WindowTop = CurDisplay->ds_WindowBot;
        CurDisplay->ds_WindowTopLine = CurDisplay->ds_WindowBotLine;

        RefreshWindow(1);
        return TRUE;
}

BOOL    PageUpMixed(void) {
        ULONG address = CurDisplay->ds_WindowTop;
        int i;
        int lines = CalcDisplayLines();

        if(lines > 0) {
            for(i=0; i<lines; i++)address = PreviousInstruction(address);
            CurDisplay->ds_WindowTop = address;
            CurDisplay->ds_WindowTopLine = 0;
            RefreshWindow(1);
        }
        return TRUE;
}

WORD    RefreshMixed(WORD maxLines, BOOL fullRefresh, ULONG addr, ULONG line, WORD mixedMode) {

        WORD    type;
        WORD    plotFlag;
        WORD    count = 0;
        LONG    size;
        long    info[2];    //  info[0] == lineBeg, info[1] == lineNo


        if((addr = ValidMemCheck((ULONG)addr)) == 0) {
            SetDisplayMode(DISPLAY_BYTES, 1);
            CurDisplay->ds_WindowTop = addr;
            return RefreshBytes(maxLines, -1, addr);
        }

        do_scroller();
        if (fullRefresh != -1) {
            SetTitle(NULL,NULL);
#if 0
            ScrPlain();
            ScrInverse();
            if (CurDisplay->ds_DisplayMode == DISPLAY_MIXED)ScrPuts("MIXED MODE");
            else ScrPuts("SOURCE MODE");
#endif
            // add a separation between register display and source
            if(CurDisplay->ds_RegFlag) {
                Newline();
                ++count;
                if (--maxLines == 0)return count;
            }
        }


        for (type = CurrentMixedLine(&addr, &line, info); maxLines; type = 0) {
                if (type == 0)type = NextMixedLine(&addr, &line, info);
                if ((mixedMode == 0) && (type == MIXTYPE_DISM)) {
                        // find the next source line
                    while((type = NextMixedLine(&addr, &line, info)) == MIXTYPE_DISM);
                }

                ScrPlain();
                switch(type) {
                    case MIXTYPE_NOSOURCE:      //  out of range of source, dism
                    case MIXTYPE_DISM:          //  disassembly
                        plotFlag = fullRefresh;
                        size = Disassemble(addr, addr, LineBuf);
                        if (addr == programPC) { 
                            plotFlag = TRUE; 
                            ScrHighlight(); 
                        }
                        else ScrPlain();

                        if (IsBreakpoint(addr)) { 
                            plotFlag = TRUE; 
                            ScrUnderline(); 
                        }
                        if (addr == lastPC) { 
                            plotFlag = TRUE; 
                            ScrPlain(); 
                        }
                        if (plotFlag) {
                            PrintAddress(addr);
                                if (!CurDisplay->ds_DisplayOffsets) {
                                        char *s = LookupValue(addr);
                                        if (!s) s = "";
                                        ScrPrintf("%-16.16s %s", s, LineBuf);
                                } 
                                else ScrPuts(LineBuf);
                        }
                        break;

                    case MIXTYPE_SOURCE:        { /*  info contains source line     */
                        long i;
                        char *lineStr = (char *)info[0];
                        for (i = 0; lineStr[i] && lineStr[i] != '\n'; ++i)
                                ;

                        if (addr == programPC)ScrHighlight();
                        else {
                            // if in source mode, display source lines in black;
                            // they are more important
                            if(CurDisplay->ds_DisplayMode == DISPLAY_SOURCE)ScrPlain();
                            else ScrDull();
                        }
                        if (IsBreakpoint(addr))ScrUnderline();

                        ScrPrintf("%6d. ", info[1]);  // PrintAddress(addr);
                        ScrWrite(lineStr, i);

                        break;
                    }
                    default:;
                }


                ScrPutNewline();
                ++count;
                --maxLines;
        }
        if (fullRefresh > 0) {
            CurDisplay->ds_WindowBot = addr;
            CurDisplay->ds_WindowBotLine = line;

        }

        return count;
}

// ************************************************************************
//
//  Source mode utilizes both address and subline like mixed mode, but
//  dism-only lines are ignored

BOOL    UpSource(void) {

        while (PrevMixedLine(&CurDisplay->ds_WindowTop, &CurDisplay->ds_WindowTopLine, NULL) == MIXTYPE_DISM)
            ;
        if (CurDisplay->ds_WindowBotLine != (ULONG)-1) {
            while (PrevMixedLine(&CurDisplay->ds_WindowBot, &CurDisplay->ds_WindowBotLine, NULL) == MIXTYPE_DISM)
                ;
        }

        ScrScrolldown();
        RefreshMixed(1, -1, CurDisplay->ds_WindowTop, CurDisplay->ds_WindowTopLine, 0);
        CurDisplay->ds_WindowBotLine = (ULONG)-1;      // no longer known

        return TRUE;
}

BOOL    DownSource(void) {

        while (NextMixedLine(&CurDisplay->ds_WindowTop, &CurDisplay->ds_WindowTopLine, NULL) == MIXTYPE_DISM)
                ;

        if (CurDisplay->ds_WindowBotLine == (ULONG)-1) {       // unknown
            WORD lines = ScrMainBodyRange(NULL,NULL)-1;

            CurDisplay->ds_WindowBot = CurDisplay->ds_WindowTop;
            CurDisplay->ds_WindowBotLine = CurDisplay->ds_WindowTopLine;
            // find the new bottom line by nexting through source lines
            while (lines > 0) {
                while (NextMixedLine(&CurDisplay->ds_WindowBot, &CurDisplay->ds_WindowBotLine, NULL) == MIXTYPE_DISM)
                ;
                --lines;
            }
        } else {
            while (NextMixedLine(&CurDisplay->ds_WindowBot, &CurDisplay->ds_WindowBotLine, NULL) == MIXTYPE_DISM)
                ;
        }
        ScrScrollup();
        RefreshMixed(1, -1, CurDisplay->ds_WindowBot, CurDisplay->ds_WindowBotLine, 0);

        return TRUE;
}


BOOL    PageDownSource(void) {

        CurDisplay->ds_WindowTop = CurDisplay->ds_WindowBot;
        CurDisplay->ds_WindowTopLine = CurDisplay->ds_WindowBotLine;

        RefreshWindow(1);
        return TRUE;
}

BOOL    PageUpSource(void) {
        ULONG address = CurDisplay->ds_WindowTop;
        int i;
        int lines = CalcDisplayLines();

        if(lines > 0) {
            for(i=0; i<lines; i++)address = PreviousInstruction(address);
            CurDisplay->ds_WindowTop = address;
            CurDisplay->ds_WindowTopLine = -1;
            RefreshWindow(1);
        }
        return TRUE;
}

/*
 *  MIXED MODE ROUTINES
 *
 *  The corner stone of this the mixed mode routines is CurrentMixedLine().
 *  It takes an address and a sub-line number (think of it as a sub address
 *  indexing through all source lines and the disassembly that apply to
 *  a single address).  line 0 through N-1 are source code lines while
 *  the last line, N, is the disassembly line.
 *
 *  This routine will modify *paddr / *pline if given a line that is beyond
 *  the range of the subgroup for the current address.  This routine returns
 *  MIXTYPE_SOURCE for source lines, MIXTYPE_DISM for disassembly lines,
 *  and MIXTYPE_NOSOURCE when we leave the range of our debugging info.
 */

WORD CurrentMixedLine(ULONG *paddr, ULONG *pline, LONG *info) {
        DEBUG   *debug;
        SOURCE  *source;
        ULONG   line;
        ULONG   addr;
        ULONG   addr_srcbeg = 0;
        ULONG   addr_srcend = 0;

    while(TRUE) {
        line = *pline;
        addr = *paddr;


        if ((debug = FindNearestDebug(addr)) && (source = debug->table)) {
            addr_srcbeg = source->address;
            addr_srcend = (debug->tableEnd - 1)->address;

            /*
             *  find source entry for this address then count lines
             */
            while (source != debug->tableEnd) {
                if (source->address == addr) {
                        ULONG grpLine;
                        ULONG baseLine = source->lineNumber;
                        SOURCE *base_source = source;
                        
                        if(source->address == addr_srcbeg) {
                            source->lineNumber = 1;     // fake it
                        }

                        baseLine = source->lineNumber;
                        base_source = source;

                        //  calculate number of source lines that apply to
                        //  this address

                        while ((source != debug->tableEnd) && (source->address == addr)) {
                                ++source;
                        }

                        grpLine = (source == debug->tableEnd) ? 0x7FFFFFFF : source->lineNumber - baseLine;
                        //  If out of range adjust the line number and
                        //  address and continue

                        if (line > grpLine) {
                                *pline = line - (grpLine + 1);
                                *paddr = addr + Disassemble(addr, addr, LineBuf);
                                continue;
                        }

                        //  If less then grpLine (0 to grpLine-1) this is
                        //  a source code line

                        if (line < grpLine) {
                                char *lptr = FindSourceLine(debug, base_source);
                                LONG n = line;
                                while (n && lptr) {
                                    if (lptr = strchr(lptr, '\n'))
                                        ++lptr;
                                    --n;
                                    ++baseLine;
                                }

                                //  Did we run into the end of the file?
                                //  (wouldn't need this if we had some way
                                //  of calculating grpLine for this case)
                                if (lptr == NULL) {
                                        if (n == 0)return(MIXTYPE_DISM);
                                        *pline = 0;
                                        *paddr = addr + Disassemble(addr, addr, LineBuf);
                                        continue;
                                }

                                //  info requested?
                                //

                                if (info) {
                                    if (lptr)info[0] = (long)lptr;
                                    else info[0] = (long)"";
                                    info[1] = baseLine;
                                }

                                return(MIXTYPE_SOURCE);
                        }
                        return(MIXTYPE_DISM);
                }
                ++source;
            }
        }
        if (line) {
            *paddr = addr + Disassemble(addr, addr, LineBuf);
            *pline = 0;
            continue;
        }
        if (addr_srcbeg == addr_srcend || addr < addr_srcbeg || addr > addr_srcend) {
            return(MIXTYPE_NOSOURCE);
        }
        return(MIXTYPE_DISM);
    }
}


WORD NextMixedLine(ULONG *addr, ULONG *line, LONG *info) {

        ++*line;
        return(CurrentMixedLine(addr, line, info));
}



WORD PrevMixedLine(ULONG *addr, ULONG *line, LONG *info) {

        if (*line) {
            --*line;
        } 
        else {
            ULONG newAddr;
            ULONG newLine;

            *line = 0;
            newAddr = *addr = PreviousInstruction(*addr);
            newLine = 0;

            while (*addr == newAddr) {
                *line = ++newLine;
                CurrentMixedLine(addr, line, NULL);
            }
            *addr = newAddr;
            *line = newLine - 1;
        }
        return(CurrentMixedLine(addr, line, info));
}