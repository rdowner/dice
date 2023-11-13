/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */
#include        "defs.h"
#include        "dbug_protos.h"


// ************************************************************************

Prototype WORD          RefreshHunks(WORD maxLines, BOOL fullRefresh, LONG index);
Prototype WORD          RefreshSymbols(WORD maxLines, BOOL fullRefresh, LONG index);
Prototype LONG          SymbolIndexOfAddr(ULONG val);
Prototype BOOL          DownHunks(void);
Prototype BOOL          UpHunks(void);
Prototype BOOL          DownSymbol(void);
Prototype BOOL          PageDownSymbol(void);
Prototype BOOL          UpSymbol(void);
Prototype BOOL          PageUpSymbol(void);

Local BOOL              HunkError(ULONG hunkType);
Prototype BOOL          DBugLoadSeg(char *filename);
Prototype void          AllocateStack(void);
Prototype void          ResetTarget(void);
Prototype DEBUG         *FindDebug(ULONG address);
Prototype DEBUG         *FindNearestDebug(ULONG address);
Prototype SOURCE        *FindSource(DEBUG *debug, ULONG address);
Prototype char          *FindSourceLine(DEBUG *debug, SOURCE *source);
Prototype __stkargs char *LookupValue(ULONG value);
Local     char          *LookupOffset(ULONG value,ULONG *symBuf);
Prototype ULONG         *NearestSymbol(ULONG value);
Prototype char          *NearestValue(ULONG value);
Local BOOL              CompareLStrings(ULONG *s1, ULONG *s2);
Prototype BOOL          LookupSymbol(char *symbol, ULONG *value);
Prototype BOOL          LookupSymLen(char *symbol, UWORD len, ULONG *value);
Prototype int           CountSymbols(void);
Prototype void          CopySymbols(SYMLIST *symlist);

Prototype char          *addscore(char *string);
Prototype char          *addat(char *string);

// ************************************************************************

UBYTE           *programStack = NULL;
UBYTE           *programStackTop = NULL;
ULONG           programStackSize = 32768;

// ************************************************************************

BOOL    DownHunks(void) {
        CurDisplay->ds_LastRefreshTop = ++CurDisplay->ds_WindowTop;
        ScrScrollup();
        RefreshHunks(1, 0, CurDisplay->ds_WindowTop + ScrMainBodyRange(NULL,NULL) - 1);
        return TRUE;
}


BOOL    UpHunks(void) {
        if (CurDisplay->ds_WindowTop) {
                CurDisplay->ds_LastRefreshTop = --CurDisplay->ds_WindowTop;
                ScrScrolldown();
                RefreshHunks(1, 0, CurDisplay->ds_WindowTop);
        }
        return TRUE;
}


WORD    RefreshHunks(WORD maxLines, BOOL fullRefresh, LONG index) {
        WORD    count = 0;
        HUNK    *thisHunk;
        DEBUG   *debug;
        ULONG   hunkNum;

        do_scroller();
        if (fullRefresh) {
                SetTitle(NULL,NULL);

                // add a separation line between hunks and register display
                if(CurDisplay->ds_RegFlag) {
                    ++count;
                    Newline();
                    if (--maxLines == 0)return count;
                }
                ScrInverse();
                ScrPuts("HUNK   TYPE     SIZE    ACTUAL   RELOCS   SYMBOL    LINE");
//                       xxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx
                count++;
                Newline();
                if (--maxLines == 0)return count;
        }
        ScrPlain();

        for (thisHunk = &hunkArray[0], hunkNum = 0; hunkNum < numHunks; hunkNum++, thisHunk++) {
                if (index <= 0) {
                        ScrPrintf("%4d %08X %08X %08X %08X %08X %08X",
                                hunkNum,
                                thisHunk->type,
                                thisHunk->size,
                                thisHunk->actual,
                                thisHunk->reloc32,
                                thisHunk->symbols,
                                thisHunk->debug
                        );
                        count++; ScrPutNewline(); maxLines--; if (!maxLines) return count;

                }
                --index;
                for (debug=thisHunk->debug; debug; debug = debug->link) {
                        if (index <= 0) {
                                ScrPrintf("     %08X %s", debug->table[0].address, debug->sourceName);
//                                         xxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx
                                count++; 
                                ScrPutNewline(); 
                                maxLines--; 
                                if (!maxLines) return count;
                        }
                        --index;
                }
        }
        return count;
}

BOOL    UpSymbol(void) {
        if (CurDisplay->ds_WindowTop) {
                CurDisplay->ds_LastRefreshTop = --CurDisplay->ds_WindowTop;
                ScrScrolldown();
                RefreshSymbols(1, 0, CurDisplay->ds_WindowTop);
        }
        return TRUE;
}

BOOL    DownSymbol(void) {
        CurDisplay->ds_LastRefreshTop = ++CurDisplay->ds_WindowTop;
        ScrScrollup();
        RefreshSymbols(1, 0, CurDisplay->ds_WindowTop + ScrMainBodyRange(NULL,NULL) - 1);
        return TRUE;
}

BOOL    PageDownSymbol(void) {
    int lines = CalcDisplayLines();

    if(lines > 0) {
        CurDisplay->ds_WindowTop += lines;
        RefreshWindow(1);
    }
    return TRUE;
}

BOOL    PageUpSymbol(void) {
    int lines = CalcDisplayLines();

    if (CurDisplay->ds_WindowTop) {
        if(lines > 0) {
                CurDisplay->ds_WindowTop -= lines;
                if((LONG)CurDisplay->ds_WindowTop < 0)CurDisplay->ds_WindowTop = 0;
                RefreshWindow(1);
        }
    }
    return TRUE;
}

WORD    RefreshSymbols(WORD maxLines, BOOL fullRefresh, LONG index) {
        ULONG   buf[32];
        WORD    count = 0;
        HUNK    *thisHunk;
        ULONG   hunkNum, *symPtr;

        ScrPlain();
        do_scroller();
        if (fullRefresh && maxLines) { 
                SetTitle(NULL,NULL);


                // add a separation line between hunks and register display
                if(CurDisplay->ds_RegFlag) {
                    ++count;
                    Newline();
                    if (--maxLines == 0)return count;
                }
#if 0
                ScrInverse();
                ScrPuts("ADDRESS  SYMBOL           VALUES");
//                       xxxxxxxx xxxxxxxxxxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx
                ScrPlain();
                count++;
                ScrPutNewline();
                if (--maxLines == 0)return TRUE;
#endif
        }


        for (thisHunk = &hunkArray[0], hunkNum = 0; hunkNum < numHunks; hunkNum++, thisHunk++) {
                if (!(symPtr = thisHunk->symbols))continue;
                while (*symPtr) {
                        if (index <= 0) {
                            ULONG   *name = &symPtr[1];
                            ULONG   i = symPtr[0] & 0x00FFFFFF;

                            for (i = 0; i < symPtr[0]; ++i)buf[i] = name[i];
                            buf[i] = 0;
                            if(IsBreakpoint(name[i]))ScrUnderline();
                            else ScrPlain();
                            ScrPrintf("%08X %-16.16s", name[i], buf);
                            --maxLines;
                            ++count;
                            ScrPutNewline();
                            if (maxLines == 0)return count;
                        }
                        --index;
                        symPtr = &symPtr[*symPtr + 2];
                }
        }
        return count;
}

/*
 *
 *
 */

LONG    SymbolIndexOfAddr(ULONG val) {
        HUNK    *thisHunk;
        LONG    index = 0;
        ULONG   hunkNum;
        ULONG   *sym;

        if ((sym = NearestSymbol(val)) == NULL)
                return 0;

        for (thisHunk = &hunkArray[0], hunkNum = 0; hunkNum < numHunks; hunkNum++, thisHunk++) {
                ULONG   *symPtr;

                if ((symPtr = thisHunk->symbols) == NULL)
                        continue;

                //  Scan symbols
                //  [namelen][name][value]

                while (*symPtr) {
                        if (symPtr == sym)
                            return(index);
                        symPtr = symPtr + (*symPtr + 2);
                        ++index;
                }
        }
        return 0;
}

// ************************************************************************

#define DPRINTF         FALSE

#if DPRINTF
#define dprintf kprintf
#else
#define dprintf //
#endif


ULONG   *exeFile;
ULONG   exeSize = 0;
ULONG   numHunks, firstHunk, lastHunk;
HUNK    *hunkArray;

Local BOOL      HunkError(ULONG hunkType) {
        printf("*** Hunk Error: $%08X\n", hunkType);
        return FALSE;
}

BOOL    DBugLoadSeg(char *filename) {
        int     fd;
        ULONG   i, hunkType, *nextHunk;
        HUNK    *thisHunk = NULL;
        ULONG   *exePtr;
        ULONG   *lasthunk = NULL;

        fd = open(filename, O_READ);
        if (fd == -1) return FALSE;
        exeSize = lseek(fd, 0, 2); lseek(fd, 0, 0);
        exeSize += 3; exeSize >>= 2;
        exeFile = (ULONG *)MallocPublic(exeSize*4+4);
        if (!exeFile) { close(fd); return FALSE; }
        read(fd, exeFile, exeSize*4);
        close(fd);
        exeFile[exeSize] = 0;

        exePtr = exeFile;
        if (*exePtr != HUNK_HEADER) { Free(exeFile); exeFile = 0; return FALSE; }
        while (1) {
                hunkType = *exePtr++;
dprintf("HUNK TYPE is %lx == ",hunkType);
                switch ( hunkType & 0x3fffffff ) {
                        case NULL:                      // 0
dprintf("NULL\n");
                                return TRUE;
                        case HUNK_UNIT:                 // 999
dprintf("HUNK_UNIT\n");
                                return HunkError(hunkType);
                        case HUNK_NAME:                 // 1000
dprintf("HUNK_NAME\n");
                                return HunkError(hunkType);
                        case HUNK_CODE:                 // 1001
#if     DPRINTF
dprintf("HUNK_CODE\n");
goto qwe;
#endif
                        case HUNK_DATA:                 // 1002
#if     DPRINTF
dprintf("HUNK_DATA\n");
goto qwe;
#endif
                        case HUNK_BSS:                  // 1003
#if     DPRINTF
dprintf("HUNK_BSS\n");
qwe:
#endif
                            if (!thisHunk) return FALSE;
                            thisHunk->type = hunkType;
                            thisHunk->reloc32 = NULL;
                            thisHunk->debug = NULL;
                            thisHunk->symbols = NULL;
                            thisHunk->hSize = *exePtr++;
                            thisHunk->hunk = exePtr;
                            if ((thisHunk->type & 0x3fffffff) != HUNK_BSS) {
                                // skip over code/data section
                                exePtr = &exePtr[thisHunk->hSize];
                            }
                            i = (thisHunk->size << 2) + 4 ; // size of hunk in bytes
                                                          // (inc. next hunk link)

                            // special new memory allocation defined in 2.04 AmigaDOS
                            if (((thisHunk->type & ((1<<31)|(1<<30))) == ((1<<31)|(1<<30)))) { // any
                                ULONG memtype = (*exePtr++) & 0x00FFFFFF;
                                thisHunk->memptr = (ULONG *)MallocAny(i,memtype);
                            }
                            else if (thisHunk->type & (1<<31)) {        // fast
                                thisHunk->memptr = (ULONG *)MallocFast(i);
                            }
                            else if (thisHunk->type & (1<<30)) {        // chip
                                thisHunk->memptr = (ULONG *)MallocChip(i);
                            }
                            else {                                      // public
                                thisHunk->memptr = (ULONG *)MallocPublic(i);
                            }
                            if (!thisHunk->memptr) {
                                printf("*** Can't allocate memory for hunk\n");
                                return FALSE;
                            }

                            // skip past the link to next hunk
                            thisHunk->actual = thisHunk->memptr + 1;
                                dprintf("Allocation at %lx\n",thisHunk->actual);
                            // check Scroller Range Settings
                            if((thisHunk->type & 0x3fffffff) == HUNK_CODE) {
                                if((ULONG)thisHunk->actual < ScrollStart)
                                    ScrollStart = (unsigned long)thisHunk->actual;
                                if((ULONG)(thisHunk->actual + i/4) > ScrollEnd)
                                    ScrollEnd = (unsigned long)thisHunk->actual + i/4;
                            }

                            // store the link to this hunk in the previous hunk
                            if(lasthunk) {
                                *lasthunk = MKBADDR((ULONG *)thisHunk->memptr);
                            }
                            lasthunk = thisHunk->memptr;


                            break;
                        case HUNK_RELOC32:              // 1004
dprintf("HUNK_RELOC32\n");
                                if (!thisHunk) return FALSE;
                                thisHunk->reloc32 = exePtr;
                                while (i = *exePtr++) exePtr = &exePtr[i+1];
                                break;
                        case HUNK_RELOC16:              // 1005
dprintf("HUNK_RELOC16\n");
                                return HunkError(hunkType);
                        case HUNK_RELOC8:               // 1006
dprintf("HUNK_RELOC8\n");
                                return HunkError(hunkType);
                        case HUNK_EXT:                  // 1007
dprintf("HUNK_EXT\n");
                                return HunkError(hunkType);
                        case HUNK_SYMBOL:               // 1008
dprintf("HUNK_SYMBOL\n");
                                if (!thisHunk) return FALSE;
                                thisHunk->symbols = exePtr;
                                while (i = *exePtr++) {
                                        exePtr = &exePtr[i];
                                        *exePtr += (ULONG)thisHunk->actual;
                                        exePtr++;
                                }
                                break;
                        case HUNK_DEBUG:                // 1009
dprintf("HUNK_DEBUG\n");
                                if (!thisHunk) return FALSE;
                                i = *exePtr++;
                                nextHunk = &exePtr[i];
                                i = (ULONG)thisHunk->actual;
                                i += *exePtr++;         // address that offsets in table are from
                                if (*exePtr++ == 'LINE') {
                                        DEBUG   *dbg = (DEBUG *)MallocPublic(sizeof(DEBUG));
                                        ULONG   base = i, *pl;

                                        if (!dbg) {
                                                printf("Can't allocate DEBUG structure\n");
                                                exit(20);
                                        }
                                        dbg->link = thisHunk->debug;
                                        thisHunk->debug = dbg;
                                        for (i=0; i<128; i++) dbg->sourceName[i] = '\0';
                                        i = *exePtr++;  // length of filename in longs
                                        pl = (ULONG *)&dbg->sourceName[0];
                                        while (i > 0) { *pl++ = *exePtr++; --i; }
                                        dbg->table = (SOURCE *)exePtr;
                                        dbg->tableEnd = (SOURCE *)nextHunk;
                                        // read in the source file
                                        dbg->source = NULL;
                                        {
                                                int fd = open(dbg->sourceName, O_READ);
                                                if (fd != -1) {
                                                        i = lseek(fd, 0, 2); lseek(fd, 0, 0);
                                                        dbg->source = (char *)MallocPublic(i+1);
                                                        if (dbg->source) {
                                                                read(fd, dbg->source, i);
                                                                dbg->source[i] = '\0';
                                                        }
                                                        close(fd);
                                                }
                                        }

                                        // relocate line#/offset pair table
                                        // determine bounds of debugger hunk
                                        //   XXX can miss code inbetween linked
                                        //       object modules, addresses are
                                        //       not right up against each other

                                        {
                                                SOURCE  *ps;
                                                for (ps = dbg->table; ps != dbg->tableEnd; ps++)
                                                        ps->address += base;

                                                if (ps != dbg->table) {
                                                        dbg->addrBegin = dbg->table->address;
                                                        dbg->addrEnd   = (ps-1)->address;
                                                }
                                        }
                                }
                                exePtr = nextHunk;
                                break;
                        case HUNK_END:                  // 1010
dprintf("HUNK_END\n");
                                thisHunk++;
                                break;
                        case HUNK_HEADER:               // 1011
dprintf("HUNK_HEADER\n");
                                while (i = *exePtr++) exePtr = &exePtr[i];      // skip libs
                                numHunks = *exePtr++;
                                firstHunk = *exePtr++;
                                lastHunk = *exePtr++;
                                hunkArray = (HUNK *)MallocPublic(sizeof(HUNK) * numHunks);
                                if (!hunkArray) {
                                        printf("*** Insufficient memory for HunkHeader table\n");
                                        exit(20);
                                }
                                // fetch hunk sizes from the header and store in hunkArray
                                for (i=0; i<numHunks; i++) hunkArray[i].size = *exePtr++;
                                thisHunk = hunkArray;
                                break;
//                      case HUNK_INVALID:              // 1012
//dprintf("case HUNK_INVALID\n");
//                              return HunkError(hunkType);
                        case HUNK_OVERLAY:              // 1013
dprintf("HUNK_OVERLAY\n");
                                return HunkError(hunkType);
                        case HUNK_BREAK:                // 1014
dprintf("HUNK_BREAK\n");
                                return HunkError(hunkType);
                        case HUNK_DREL32:               // 1015
dprintf("HUNK_DREL32\n");
                                return HunkError(hunkType);
                        case HUNK_DREL16:               // 1016
dprintf("HUNK_DREL16\n");
                                return HunkError(hunkType);
                        case HUNK_DREL8:                // 1017
dprintf("HUNK_DREL8\n");
                                return HunkError(hunkType);
                        case HUNK_LIB:                  // 1018
dprintf("HUNK_LIB\n");
                                return HunkError(hunkType);
                        case HUNK_INDEX:                // 1019
dprintf("HUNK_INDEX\n");
                                return HunkError(hunkType);
                default:;
dprintf("Unknown hunk type %lx\n",hunkType);
                        break;
                }
        }
}

void    AllocateStack(void) {
        if (!programStack) {
            // reset stack size to CLI stack size if CLU default
            // stack is larger than 32K
            programStackSize = MAX(thisCli->cli_DefaultStack * 4, programStackSize);
                programStack = (UBYTE *)MallocPublic(programStackSize);
                if(!programStack) {
                    printf("Can't Allocate Program Stack!\n");
                    exit(20);
                }
                programStackTop = &programStack[programStackSize];
        }
}

void    ResetTarget(void) {
        HUNK            *thisHunk = &hunkArray[0];
        ULONG           i;
        struct FileHandle *fh;
        char    *pd;

        AllocateStack();
        for (i=0; i<numHunks; i++, thisHunk++) {
            switch (thisHunk->type & 0x3fffffff) {
                // code and data hunks are copied from the file into allocated memory.
                // then the reloc32 hunk is applied to relocate the copied hunk.
                case HUNK_CODE:
                case HUNK_DATA: { // copy the hunk
                    ULONG       *ps = thisHunk->hunk, *pd = thisHunk->actual;
                    ULONG       i = thisHunk->hSize;
                    while (i > 0) { 
                        *pd++ = *ps++; 
                        --i;
                    }
                }
                { // relocations
                    UBYTE       *base = (UBYTE *)thisHunk->actual;
                    ULONG       offset, *src = thisHunk->reloc32;
                    ULONG       hunkNumber, numRelocs;

                    if (!src) break;    // no reloc32 hunk
                    numRelocs = *src++;
                    while (numRelocs) {
                        hunkNumber = *src++;
                        offset = (ULONG)hunkArray[hunkNumber].actual;
                        while (numRelocs) {
                            *(ULONG *)&base[*src++] += offset;
                            numRelocs--;
                        }
                        numRelocs = *src++;
                    }
                }
                break;
                case HUNK_BSS:
                    break;
                default:;
dprintf("UNKNOWN HUNK in RESET\n");
                break;
            }
        }
        // Load Input() with command line, if any
        // (A debugger has to do shell like things, occasionally
        if((fh = (struct FileHandle *)BADDR(Input()))) {
            Flush(Input());
            pd = (char *)BADDR(fh->fh_Buf);
            strcpy(pd,"");
            strncat(pd,args,argSize-1);
            strcat(pd,"\n");
            fh->fh_Pos = 0;
            fh->fh_End = strlen(pd);
        }

        // initialize registers
        lastState = programState = STATE_RESET;
        lastPC = programPC = (ULONG)hunkArray[0].actual;
        lastSR = programSR = 0;

        lastA0 = programA0 = (ULONG)args;
        lastD0 = programD0 = argSize;

        lastD1 = lastD2 = lastD3 = lastD4 = lastD5 = lastD6 = lastD7 =
                programD1 = programD2 = programD3 = programD4 = programD5 = programD6 = programD7 = 0;
        lastA1 = lastA2 = lastA3 = lastA4 = lastA5 = lastA6 =
                programA1 = programA2 = programA3 = programA4 = programA5 = programA6 = 0;
        programA7 = (ULONG)&programStack[programStackSize];

        // BCPL programs are going to want A1, A2, A5, and A6 to be
        // set up properly.  The easiest place to get this is from
        // our own stack
        {
            long *stacktop;
            long *clistack;


            // get pointer to top of our stack
            stacktop = ((struct Process *)(SysBase->ThisTask))->pr_ReturnAddr;  

            // get last value of CLI stack
            clistack   = (long *) *(++stacktop);

            lastA1 = programA1 =(ULONG)(*(++clistack));
            lastA2 = programA2 =(ULONG)(*(++clistack));
            lastA5 = programA5 =(ULONG)(*(++clistack));
            lastA6 = programA6 =(ULONG)(*(++clistack));


        { // push "catcher" address on program stack
                ULONG   *a7 = (ULONG *)programA7;
                --a7;   // put half the stack size (in longs) on stack
                *a7 = programStackSize/4;
                --a7;
                *a7 = (ULONG)TargetExit;
                lastA7 = programA7 = (ULONG)a7;
        }

        }

        if (LookupSymbol("@main", &i) || LookupSymbol("_main", &i)) {
                SetTempBreakpoint(i);
                GoTarget();
        }

        CurDisplay->ds_WindowTop = programPC;
        CurDisplay->ds_WindowTopLine = 0;

        // set the default save address tables
        SetModeSave(DISPLAY_BYTES);
        SetModeSave(DISPLAY_SOURCE);
}

DEBUG   *FindDebug(ULONG address) {
        DEBUG           *debug;
        SOURCE          *source;

        if (debug = FindNearestDebug(address)) {
                for (source = debug->table; source != debug->tableEnd; ++source) {
                        if (source->address == address)break;
                }
                debug = NULL;
        }
        return(debug);
}

#ifdef NOTDEF   // REMOVED

DEBUG   *FindDebug(ULONG address) {
        HUNK            *thisHunk;
        ULONG           hunkNum, hunkStart, hunkEnd;
        DEBUG           *debug;
        SOURCE          *source;

        for (thisHunk = &hunkArray[0], hunkNum = 0; hunkNum < numHunks; thisHunk++, hunkNum++) {
                hunkStart = (ULONG)thisHunk->actual; hunkEnd = hunkStart + (thisHunk->size<<2);
                if (address < hunkStart || address > hunkEnd) continue;
                debug = thisHunk->debug;
                while (debug) {
                        source = debug->table;
                        while (source != debug->tableEnd) {
                                if (source->address == address) return debug;
                                source++;
                        }
                        debug = debug->link;
                }
        }
        return NULL;
}

#endif

DEBUG   *FindNearestDebug(ULONG address) {
        HUNK            *thisHunk;
        ULONG           hunkNum, hunkStart, hunkEnd;
        DEBUG           *debug;

        for (thisHunk = &hunkArray[0], hunkNum = 0; hunkNum < numHunks; thisHunk++, hunkNum++) {
                hunkStart = (ULONG)thisHunk->actual; hunkEnd = hunkStart + (thisHunk->size<<2);
                if (address < hunkStart || address > hunkEnd) continue;

                for (debug = thisHunk->debug; debug; debug = debug->link) {
                        if (address >= debug->addrBegin && address <= debug->addrEnd)
                                return(debug);
                }
        }
        return NULL;
}


SOURCE  *FindSource(DEBUG *debug, ULONG address) {
        SOURCE  *source;

        source = debug->table;
        if (source) {
                while (source != debug->tableEnd) {
                        if (source->address == address) return source;
                        source++;
                        if (source != debug->tableEnd && source->address > address) {
                                --source;
                                return source;
                        }
                }
        }
        return NULL;
}

char    *FindSourceLine(DEBUG *debug, SOURCE *source) {
        char    *line = debug->source;
        ULONG   currentLine = 1;

    if(line) {
        while (currentLine < source->lineNumber) {
                while (*line && (*line != '\n')) {
                    *line++;
                }
                if (*line) line++;
                currentLine++;
        }
        if (*line) return line;
    }
    return NULL;
}

__stkargs char  *LookupValue(ULONG value) {
        HUNK            *thisHunk;
        ULONG           i, hunkNum, *symPtr, *name;
        ULONG           hunkStart, hunkEnd;
        ULONG           *ps, *pd;
        static ULONG    symBuf[64];

        for (thisHunk = &hunkArray[0], hunkNum = 0; hunkNum < numHunks; 
        thisHunk++, hunkNum++) {
                hunkStart = (ULONG)thisHunk->actual; hunkEnd = hunkStart + (thisHunk->size<<2);
                if (value < hunkStart || value > hunkEnd) continue;
                symPtr = thisHunk->symbols;
                if (!symPtr) return NULL;
                while (*symPtr) {
                        name = symPtr;
                        i = *symPtr++;
                        symPtr = &symPtr[i];
                        if (*symPtr++ == value) {
                                for (i=0; i<64; i++) symBuf[i] = 0;
                                pd = &symBuf[0];
                                i = *name++;
                                ps = name;
                                while (i > 0) { *pd++ = *ps++; --i; }
                                return (char *)&symBuf[0];
                        }
                }
                return LookupOffset(value,symBuf); // return NULL;
        }
        return LookupOffset(value,symBuf); // return NULL;
}

char    *LookupOffset(ULONG value,ULONG *symBuf) {
    char *s;
    ULONG val;

    if(CurDisplay->ds_DisplayOffsets) {
        if(s = NearestValue(value)) {
            LookupSymbol(s,&val);
            sprintf((char *)&symBuf[0],"%s + $%04X",s,value-val);
            return (char *)&symBuf[0];
        }
    }
    return NULL;
}

ULONG   *NearestSymbol(ULONG value) {
        HUNK            *thisHunk;
        ULONG           i, hunkNum, *symPtr, *name, diff = 0x7fffffff, *found;
        ULONG           hunkStart, hunkEnd;

        found = NULL;
        for (thisHunk = &hunkArray[0], hunkNum = 0; hunkNum < numHunks; thisHunk++, hunkNum++) {
                hunkStart = (ULONG)thisHunk->actual; hunkEnd = hunkStart + (thisHunk->size<<2);
                if (value < hunkStart || value > hunkEnd) continue;
                symPtr = thisHunk->symbols;
                if (!symPtr) return NULL;
                while (*symPtr) {
                        name = symPtr;
                        i = *symPtr++;
                        symPtr = &symPtr[i];
                        if (*symPtr == value) { found = name; break; }
                        if (*symPtr < value && value - *symPtr < diff) { found = name; diff = value - *symPtr; }
                        symPtr++;
                }
                return found;
        }
        return NULL;
}

char    *NearestValue(ULONG value) {
        ULONG   *found;
        ULONG   *ps, *pd;
        LONG    i;
        static ULONG    symBuf[64];

        if (found = NearestSymbol(value)) {
                for (i=0; i<64; i++) symBuf[i] = 0;
                pd = &symBuf[0];
                i = *found++;
                ps = found;
                while (i > 0) { *pd++ = *ps++; --i; }
                return (char *)&symBuf[0];
        }
        return(NULL);
}


#ifdef NOTDEF

//
//          REMOVED
//

char    *NearestValue(ULONG value) {
        HUNK            *thisHunk;
        ULONG           i, hunkNum, *symPtr, *name, diff = 0x7fffffff, *found;
        ULONG           hunkStart, hunkEnd;
        ULONG           *ps, *pd;
        static ULONG    symBuf[64];

        found = NULL;
        for (thisHunk = &hunkArray[0], hunkNum = 0; hunkNum < numHunks; thisHunk++, hunkNum++) {
                hunkStart = (ULONG)thisHunk->actual; hunkEnd = hunkStart + (thisHunk->size<<2);
                if (value < hunkStart || value > hunkEnd) continue;
                symPtr = thisHunk->symbols;
                if (!symPtr) return NULL;
                while (*symPtr) {
                        name = symPtr;
                        i = *symPtr++;
                        symPtr = &symPtr[i];
                        if (*symPtr == value) { found = name; break; }
                        if (*symPtr < value && value - *symPtr < diff) { found = name; diff = value - *symPtr; }
                        symPtr++;
                }
                if (found) {
                        for (i=0; i<64; i++) symBuf[i] = 0;
                        pd = &symBuf[0];
                        i = *found++;
                        ps = found;
                        while (i > 0) { *pd++ = *ps++; --i; }
                        return (char *)&symBuf[0];
                }
                return NULL;
        }
        return NULL;
}

#endif

Local BOOL      CompareLStrings(ULONG *s1, ULONG *s2) {
        ULONG len = *s1++;
        if (len != *s2++) return FALSE;
        while (len > 0) {
                if (*s1++ != *s2++) return FALSE;
                len--;
        }
        return TRUE;
}

int     CountSymbols(void) {
        HUNK            *thisHunk;
        ULONG           i, hunkNum, *symPtr;
        int symcount = 0;

        for(thisHunk= &hunkArray[0],hunkNum=0;hunkNum < numHunks;
                                                        thisHunk++,hunkNum++){
                symPtr = thisHunk->symbols;
                if (!symPtr) continue;
                while (*symPtr) {
                        i = *symPtr++;
                        symPtr = &symPtr[i];
                        symPtr++;
                        symcount++;
                }
        }
        return symcount;
}

BOOL    LookupSymbol(char *symbol, ULONG *value) {
        HUNK            *thisHunk;
        ULONG           i, hunkNum, *symPtr, *name;
        UBYTE           *ps, *pd;
        ULONG           symBuf[64];

        for (i=0; i<64; i++) {
            symBuf[i] = 0;
        }

        ps = (UBYTE *)symbol; 
        pd = (UBYTE *)&symBuf[1];

        i = 0;
        while (*pd++ = *ps++) i++;
        i += 3;
        i /= 4;
        symBuf[0] = i;


        for(thisHunk= &hunkArray[0],hunkNum=0;hunkNum < numHunks;
                                                        thisHunk++,hunkNum++){
                symPtr = thisHunk->symbols;
                if (!symPtr) continue;
                while (*symPtr) {
                        name = symPtr;
                        i = *symPtr++;
                        symPtr = &symPtr[i];
                        if (CompareLStrings(symBuf, name)) {
                                *value = *symPtr;
                                return TRUE;
                        }
                        symPtr++;
                }
        }
        return FALSE;
}


void    CopySymbols(SYMLIST *symlist) {
        HUNK            *thisHunk;
        ULONG           i, hunkNum, *symPtr, *name;
        int             j = 0;

        for(thisHunk= &hunkArray[0],hunkNum=0;hunkNum < numHunks; thisHunk++,hunkNum++){
                symPtr = thisHunk->symbols;
                if (!symPtr) continue;
                while (*symPtr) {
                        name = symPtr;
                        i = *symPtr++;
                        symPtr = &symPtr[i];
                        symlist[j].symbolname = name;
                        symlist[j].address = *symPtr;
                        j++;
                        symPtr++;
                }
        }
}

BOOL    LookupSymLen(char *symbol, UWORD len, ULONG *value) {
        HUNK            *thisHunk;
        ULONG           i, hunkNum, *symPtr, *name;
        UBYTE           *ps, *pd;
        ULONG           symBuf[64];

        ps = (UBYTE *)symbol; pd = (UBYTE *)&symBuf[1];
        for (i=0; i<64; i++) symBuf[i] = 0;
        for (i = 0; i<len; i++) *pd++ = *ps++;
        *pd = '\0';
        i += 3;
        i /= 4;
        symBuf[0] = i;

        for (thisHunk = &hunkArray[0], hunkNum = 0; hunkNum < numHunks; thisHunk++, hunkNum++) {
                symPtr = thisHunk->symbols;
                if (!symPtr) continue;
                while (*symPtr) {
                        name = symPtr;
                        i = *symPtr++;
                        symPtr = &symPtr[i];
                        if (CompareLStrings(symBuf, name)) {
                                *value = *symPtr;
                                return TRUE;
                        }

                        symPtr++;
                }
        }
        return FALSE;
}

char *addscore(char *string)
{
static char buffer[256];

strcpy(&buffer[1],string);
buffer[0]='_';

return buffer;
}


char *addat(char *string)
{
static char buffer[256];

strcpy(&buffer[1],string);
buffer[0]='@';

return buffer;
}