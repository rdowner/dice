/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */
#include        "defs.h"
#include        "dbug_protos.h"


#define EXECBASE (*(struct ExecBase **)4)
//#define THISPROC    ((struct Process *)(EXECBASE->ThisTask))
#define THISPROC    ((struct Process *)(SysBase->ThisTask))

// ************************************************************************

Prototype void          Newline(void);
Prototype void          PrintAddress(ULONG addr);
Prototype void          OffsetAddressBuf(ULONG addr, char *buf);

Prototype void          InitCommand(void);
Prototype void          InitModes(void);
Prototype void          RefreshCommand(int fullRefresh);
Prototype void          RefreshPrompt(BOOL fullRefresh);
Prototype void          RefreshWindow(int fullRefresh);
Prototype void          RefreshAllWindows(int fullRefresh);
Prototype void          SetDisplayMode(WORD, BOOL);

Prototype void          ReadPrefs(void);
Prototype void          WritePrefs(void);

Prototype void          abort(void);
Prototype BOOL          ParseArgToken(char *buf);
Prototype ULONG         OnOffToggle(char *arg, ULONG val, ULONG mask);
Prototype int           main(int ac, char *av[]);
Prototype LONG          CalcDisplayLines(void);
Prototype void          SetModeSave(WORD mode);
Prototype ULONG         ValidMemCheck(ULONG address);

Prototype __stkargs void kprintf(unsigned char *arg, ...);

// ***********************************************************************

Prototype UBYTE         LineBuf[128];
Prototype UBYTE         DirBuf[128];
Prototype WORD          ForceFullRefresh;
Prototype UBYTE         RexxReplyString[MAX_REXX_REPLY];
Prototype UBYTE         DefaultPubName[128];
Prototype char          lastCommand[128];
Prototype int           RStepFlag;
Prototype char          rexxhostname[16];

// ************************************************************************

DPREFS          dprefs = { 0,0,640,200 };
ULONG           ModeTopSave[MAX_MODES];
ULONG           ModeSubSave[MAX_MODES];
LONG            CommonAddrTable[MAX_MODES];
UBYTE           LineBuf[128];
UBYTE           DefaultPubName[128];
WORD            ForceFullRefresh;

char            targetName[128];
__aligned char  exeCommandName[128];    // cli_CommandName of debugged program
char            lastCommand[128];
char            commandLine[128];
UBYTE           RexxReplyString[MAX_REXX_REPLY];
UBYTE           DirBuf[128];
// UWORD                commandCol = 0, commandEnd = 0;
UWORD commandCol = 0;
UWORD commandEnd = 0;

int             RStepFlag = 0;

// ************************************************************************

void    Newline(void) {
        ScrEOL(); 
        ScrPlain(); 
        ScrPutNewline();
        ScrFlush();
}

void    PrintAddress(ULONG addr) {
        if (CurDisplay->ds_DisplayOffsets) {
                char    *s = NearestValue(addr);

                if (s) {
                        ULONG   val;
                        LookupSymbol(s, &val);
                        ScrPrintf("%s + $%04X\t", s, addr - val);
                        return;
                }
        }
        ScrPrintf("$%08X ", addr);
}

void OffsetAddressBuf(ULONG addr, char *buf)
{
        char    *s = NearestValue(addr);

        if (s) {
                ULONG   val;
                LookupSymbol(s, &val);
                sprintf(buf, "%s+%04x", s, addr - val);
        } else {
                sprintf(buf, "%08x", addr);
        }
}


void    RefreshPrompt(BOOL fullRefresh) {
        DEBUG   *debug = FindNearestDebug(CurDisplay->ds_WindowTop);
        char    *s = debug ? debug->sourceName : "NO SOURCE";
        int len = CurDisplay->ds_PromptStart+1;

        ScrPlain();     // put line back to normal
        ScrRowCol(CurDisplay->ds_ScrRows-2, len);
        CurDisplay->ds_PromptLen = ScrPrintf("(%s): ", s) + len;
}


// ************************************************************************

void    InitCommand(void) {
        if(stricmp(commandLine,"again"))strcpy(lastCommand, commandLine);
        strcpy(commandLine, "");
        commandCol = commandEnd = 0;
}

void    InitModes(void) {

        WORD    i;

        //  Initialize the topWindow save array for the various display
        //  modes to -1
        //
        //  Initialize the CommonAddrTable[] such that the dism/source/mixed
        //  modes share a common topWindow save address and the hex display
        //  modes share a common topWinodw save address

        for (i = 0; i < MAX_MODES; ++i) {
            ModeTopSave[i] = -1;
            ModeSubSave[i] = 0;
            CommonAddrTable[i] = i;
        }

        CommonAddrTable[DISPLAY_SOURCE] = DISPLAY_DISM;
        CommonAddrTable[DISPLAY_MIXED] = DISPLAY_DISM;
        CommonAddrTable[DISPLAY_WORDS] = DISPLAY_BYTES;
        CommonAddrTable[DISPLAY_LONGS] = DISPLAY_BYTES;

        ModeTopSave[DISPLAY_HUNKS] = 0;
        ModeTopSave[DISPLAY_BREAK] = 0;
        ModeTopSave[DISPLAY_SYMBOL] = 0;
        ModeTopSave[DISPLAY_HELP] = 0;
}

void    RefreshCommand(int fullRefresh) {
        DBugDisp *disp = CurDisplay;

        ScrRowCol(CurDisplay->ds_ScrRows-2, disp->ds_PromptLen);
        commandLine[commandEnd] = '\0';
        ScrPlain();
        ScrPuts(commandLine);
        ScrEOL();
        ScrRowCol(CurDisplay->ds_ScrRows-2, commandCol+disp->ds_PromptLen);
        if(fullRefresh)ScrFlush();
        if(fullRefresh == -1)drawdoublebox(); // redraw box if screen was cleared

}

// ************************************************************************

void    SetDisplayMode(WORD mode, BOOL setPrefered) {
        ULONG addr;
        SHORT   displayMode = CurDisplay->ds_DisplayMode;


        if (setPrefered) {
            CurDisplay->ds_PreferedMode = mode;
        }

        if (displayMode != mode) {
                ModeTopSave[CommonAddrTable[displayMode]] = CurDisplay->ds_WindowTop;
                ModeSubSave[CommonAddrTable[displayMode]] = CurDisplay->ds_WindowTopLine;
                CurDisplay->ds_DisplayMode = displayMode = mode;
                if ((addr = ModeTopSave[CommonAddrTable[mode]]) != (ULONG)-1)
                    CurDisplay->ds_WindowTop = addr;
                CurDisplay->ds_WindowTopLine = ModeSubSave[CommonAddrTable[mode]];
        }
        // if the register command hasn't been used, stay in auto mode
        if(!CurDisplay->ds_RegTouched) {
            if((mode == DISPLAY_MIXED) || (mode == DISPLAY_DISM))
                CurDisplay->ds_RegFlag = 1;
            else CurDisplay->ds_RegFlag = 0;
        }
}

void    SetModeSave(WORD mode) {
        ModeTopSave[CommonAddrTable[mode]] = CurDisplay->ds_WindowTop;
        ModeSubSave[CommonAddrTable[mode]] = CurDisplay->ds_WindowTopLine;
}

// ************************************************************************

void    RefreshAllWindows(int fullRefresh)
{
    DBugDisp *disp, *odisp = CurDisplay;

    RefreshWindow(fullRefresh); // do current display first
                                // (its the focus of attention)
    for (disp = (DBugDisp *)DisplayList.lh_Head; disp->ds_Node.ln_Succ; disp = (DBugDisp *)disp->ds_Node.ln_Succ) {
        if(disp != odisp) {     // do not do original window again
            CurDisplay = disp;
            RefreshWindow(fullRefresh);
        }
    }
    CurDisplay = odisp;
}

// ************************************************************************

void    RefreshWindow(int fullRefresh) {
        WORD            lines, count;
        DBugDisp        *disp = CurDisplay;
        int             clearStatus = fullRefresh;

        fullRefresh = (fullRefresh) ? 1 : 0;
        // check for force or mode change
        if ((disp->ds_LastRefreshMode != disp->ds_DisplayMode) || (disp->ds_LastRefreshTop != disp->ds_WindowTop) || ForceFullRefresh ) {
            fullRefresh = 1;
            ForceFullRefresh = 0;
        }

        disp->ds_LastRefreshMode = disp->ds_DisplayMode;
        disp->ds_LastRefreshTop = disp->ds_WindowTop;
        GetWindowSize();
        lines = disp->ds_ScrRows - 1;

        ScrCursoff();
        disp->ds_ScrTop = disp->ds_RegFlag;  // 0 for no registers, 1 otherwise

        if (clearStatus == -1)ScrClr();
        else if (fullRefresh)ScrScrollClr();

        ScrHome();
        count = RefreshRegisters(lines, (BOOL)fullRefresh);
        lines -= count;
        disp->ds_ScrTop += count;
        lines -= 3;     /* command line and two control lines   */

        if (lines > 0 && programState != STATE_EXITED) {
                count = RefreshWatchpoints(lines, (BOOL)fullRefresh);
                lines -= count;
                disp->ds_ScrTop += count;
        }
        if (lines > 0) {
                if (programState == STATE_EXITED && disp->ds_DisplayMode <= DISPLAY_MIXED) {
                        ScrPrintf("*** Program exited code %d", programD0);
                }
                else {
                        switch (disp->ds_DisplayMode) {
                                case DISPLAY_DISM:      lines -= RefreshDism(lines, (BOOL)fullRefresh); break;
                                case DISPLAY_SOURCE:    lines -= RefreshMixed(lines, (BOOL)fullRefresh, disp->ds_WindowTop, disp->ds_WindowTopLine, 0); break;
                                case DISPLAY_MIXED:     lines -= RefreshMixed(lines, (BOOL)fullRefresh, disp->ds_WindowTop, disp->ds_WindowTopLine, 1); break;
                                case DISPLAY_BYTES:     lines -= RefreshBytes(lines, (BOOL)fullRefresh, disp->ds_WindowTop); break;
                                case DISPLAY_WORDS:     lines -= RefreshWords(lines, (BOOL)fullRefresh, disp->ds_WindowTop); break;
                                case DISPLAY_LONGS:     lines -= RefreshLongs(lines, (BOOL)fullRefresh, disp->ds_WindowTop); break;
                                case DISPLAY_HUNKS:     lines -= RefreshHunks(lines, (BOOL)fullRefresh, disp->ds_WindowTop); break;
                                case DISPLAY_BREAK:     lines -= RefreshBreakpoints(lines, (BOOL)fullRefresh); break;
                                case DISPLAY_HELP:      lines -= RefreshHelp(lines, (BOOL)fullRefresh, disp->ds_WindowTop); break;
                                case DISPLAY_SYMBOL:    lines -= RefreshSymbols(lines, (BOOL)fullRefresh, disp->ds_WindowTop); break;
                                case DISPLAY_DOSBASE:
                                case DISPLAY_PROCESS:
                                case DISPLAY_INFO:
                                case DISPLAY_EXECBASE:
                                case DISPLAY_RESOURCES:
                                case DISPLAY_INTRS:
                                case DISPLAY_PORTS:
                                case DISPLAY_TASKS:
                                case DISPLAY_LIBS:
                                case DISPLAY_DEVICES:
                                case DISPLAY_MEMLIST:
                                case DISPLAY_REXXLIST:
                                case DISPLAY_SYMLIST:
                                                        lines -= RefreshList(lines, (BOOL)fullRefresh, disp->ds_WindowTop); break;
                                default:                break;
                        }
                }
        }
        while (lines > 0) {
                Newline();
                lines--;
        }

        RefreshFKeys(clearStatus);
        RefreshPrompt(clearStatus);
        RefreshCommand(clearStatus);

        ScrCurson();
        disp->ds_LastRefreshMode = disp->ds_DisplayMode;
        
}

/* utility routine to calculate the number of available lines in the display */
LONG    CalcDisplayLines(void)
{
    LONG lines = CurDisplay->ds_ScrRows - 1;
    WORD regcount = 0;
        GetWindowSize();
        ScrHome();
        // subtract register display, command line, and two control lines
        // to find how many lines we should look back
        // note:  no register display normally in source mode
        if(CurDisplay->ds_RegFlag)regcount = (lines > 5) ? 5 : lines;
        lines = CurDisplay->ds_ScrRows - 6 - regcount;
        return(lines);
}

// ************************************************************************

void    ReadPrefs(void) {
        int     fd, i;
        APTR wp = THISPROC->pr_WindowPtr;

        // turn off requesters
        THISPROC->pr_WindowPtr = (APTR)-1;
        if((fd = open("dcc_config:dd.config", O_READ)) <= 0) {
            fd = open("dcc:config/dd.config", O_READ);
        }
        for(i=0; i<MAXCOMMAND; i++)dprefs.alias[i]=0;   // clear entire alias table

        if (fd == -1) {
                dprefs.top = dprefs.left = 0;
                dprefs.width = 640; dprefs.height = 200;
                dprefs.DefaultOffset = 0;
                dprefs.DefaultMode = DISPLAY_SOURCE;
        }
        else {
                read(fd, &dprefs, sizeof(DPREFS));
                close(fd);
        }
        THISPROC->pr_WindowPtr = wp;
}

void    WritePrefs(void) {
        int     fd;
        APTR wp = THISPROC->pr_WindowPtr;

        // turn off requesters
        THISPROC->pr_WindowPtr = (APTR)-1;
        if((fd = open("dcc_config:dd.config", O_WRITE)) <= 0) {
            fd = open("dcc:config/dd.config", O_WRITE);
        }
        if (fd != -1) {
                dprefs.DefaultOffset = CurDisplay->ds_DisplayOffsets;
                dprefs.DefaultMode = CurDisplay->ds_DisplayMode;

                write(fd, &dprefs, sizeof(DPREFS));
                close(fd);
        }
        THISPROC->pr_WindowPtr = wp;
}


// ************************************************************************

void    abort(void) {
        CleanMem();
}

BOOL    ParseArgToken(char *buf) {
        char    *b = buf;

        while (argSize > 0 && *args == ' ') { 
            args++; 
            argSize--;
        }
        if (*args == '"') {
            args++;
            argSize--;
            while (argSize > 0 && *args != '"' && *args != '\n') {
                *buf++ = *args++;
                argSize--;
            }
            if (argSize > 0 && *args == '"') {
                args++;
                argSize--;
            }
        }
        else {
            while (argSize > 0 && *args != ' ' && *args != '\n') { 
                *buf++ = *args++;
                argSize--;
            }
        }
        *buf = '\0';
        return (b != buf);
}

char    *RexxHostName = NULL; //REXXPORTNAME;
char    rexxhostname[16];

int     main(int ac, char *av[]) {
        int i=1;
        atexit(abort);
        InitConsole();
        if (RexxSysBase == NULL) {
                puts("Unable to open rexxsyslib.library.");
                exit(20);
        }

        // port name kludge
        {
            int num = CreateDiceRexxPort(NULL,REXXPORTNAME);
            sprintf(rexxhostname,"%s.%02d",REXXPORTNAME,num);
        }

        if (!ParseArgToken(targetName)) {
                printf("Usage: DBug <options> program [program_args...]\n");
                printf("; Before using DBug, compile program with -d1 to add symbols\n");
                exit(20);
        }
        {
                WORD    i;
                char    *ps = &targetName[0], *pd = (char *)&exeCommandName[1];

                for (i=0; i<128; i++) exeCommandName[i] = 0;
                while(*pd++ = *ps++)exeCommandName[0]++;

        }
        InitModes();
        InitCommand();
        InitBreakpoints();
        memset(DirBuf,0,128);
        init_function_keys();
        if(((struct Library *)SysBase)->lib_Version >= 36) {
            GetCurrentDirName(DirBuf,128);
        }
        ReadPrefs();
        if (!DBugLoadSeg(targetName)) {
                printf("Can't loadseg %s\n", targetName);
                exit(20);
        }
        ScrOpen(TRUE, FALSE, NULL);
        SymbolCount = CountSymbols();
        ResetTarget();
        CmdRexx("ddinit.dd");   // check for init file
        EnterDebugger();
        CmdQuit("");
        return 0;
}

ULONG   OnOffToggle(char *arg, ULONG val, ULONG mask) {

        if (strnicmp(arg, "on", 2) == 0) {
                return(val | mask);
        } else if (strnicmp(arg, "off", 3) == 0) {
                return(val & ~mask);
        } else {
                return(val ^ mask);
        }
}


// This routine checks if it is being fed a legal address for debugger
// access to avoid annoying enforcer.
// All display refresh routines should use it, as well as special
// commands like SET
//
ULONG   ValidMemCheck(ULONG address) {

    // low memory check (enforcer) first
    // might need an 040 specific test if 040 enforcer protects more

    if(address < 0x1000)return 0;

    // this and should only be done on the 68000
    if(address & 1)address &= 0xFFFFFFFE;

        // add credit card && CDTV card && ZKick
        // if (OpenResource("card.resource")) 00600000 +00550002
        // if (FindResident("cdstrap")) 0x00E00000 + 00080000
        // if ((((ULONG)(SysBase->LibNode.lib_Node.ln_Name)) >> 16) == 0x20) 0x00200000,0x00080000

    if( TypeOfMem((UBYTE *)address) || 
        (address >= 0x00F00000 && address <= 0x00FFFFFF) ||
        (address >= 0x00BC0000 && address <= 0x00BEFFFF) ||
        (address >= 0x00D80000 && address <= 0x00DFFFFF))return address;
    return 0;
}
