/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */
#include	"defs.h"
#include	"dbug_protos.h"


Prototype void		RefreshFKeys(BOOL fullRefresh);
Prototype BOOL		FunctionKey(UWORD num);
Prototype BOOL		HelpKey(void);

Prototype BOOL 		CmdFKey(char *args);
Prototype BOOL		CmdRexx(char *args);
Prototype BOOL		CmdSavePrefs(char *args);
Prototype BOOL		CmdInfo(char *args);
Prototype BOOL		CmdExecBase(char *args);
Prototype BOOL		CmdResources(char *args);
Prototype BOOL		CmdIntrs(char *args);
Prototype BOOL		CmdPorts(char *args);
Prototype BOOL		CmdSymList(char *args);
Prototype BOOL		CmdTasks(char *args);
Prototype BOOL		CmdLibs(char *args);
Prototype BOOL		CmdDevices(char *args);
Prototype BOOL		CmdMemList(char *args);
Prototype BOOL		CmdDoBsBase(char *args);
Prototype BOOL		CmdProcess(char *args);
Prototype BOOL		CmdOffsets(char *args);
Prototype BOOL		CmdDown(char *args);
Prototype BOOL		CmdUp(char *args);
Prototype BOOL		CmdPageDown(char *args);
Prototype BOOL		CmdPageUp(char *args);
Prototype BOOL		CmdSource(char *args);
Prototype BOOL		CmdDism(char *args);
Prototype BOOL		CmdMixed(char *args);
Prototype BOOL		CmdBytes(char *args);
Prototype BOOL		CmdWords(char *args);
Prototype BOOL		CmdLongs(char *args);
Prototype BOOL		CmdBreakpoint(char *args);
Prototype BOOL		CmdBP(char *args);
Prototype BOOL		CmdClear(char *args);
Prototype BOOL		CmdHunks(char *args);
Prototype BOOL		CmdSymbol(char *args);
Prototype BOOL		CmdSet(char *args);
Prototype BOOL		RStepTarget(void);
Prototype BOOL		StepTarget(void);
Prototype BOOL		StepTargetRange(ULONG addressLow, ULONG addressHi);
Prototype BOOL		RStepTargetRange(ULONG addressLow, ULONG addressHi);
Prototype BOOL		CmdStep(char *args);
Prototype BOOL		CmdRStep(char *args);
Prototype BOOL		OverTarget(void);
Prototype BOOL		OverTargetRange(ULONG addressLow, ULONG addressHi);
Prototype BOOL		CmdOver(char *args);
Prototype BOOL		CmdReset(char *args);
Prototype BOOL		GoTarget(void);
Local void		CopyLastState(void);
Prototype BOOL		CmdGo(char *args);
Prototype BOOL		CmdEval(char *args);
Prototype BOOL		CmdOpen(char *args);
Prototype BOOL		CmdClose(char *args);
Prototype BOOL		CmdQuit(char *args);
Prototype BOOL		CmdHelp(char *args) ;
Prototype BOOL		CmdWatchBytes(char *args);
Prototype BOOL		CmdWatchWords(char *args);
Prototype BOOL		CmdWatchLongs(char *args);
Prototype BOOL		CmdWatchClear(char *args);
Prototype BOOL		CmdRefresh(char *args);
Prototype BOOL		CmdDosBase(char *args);
Prototype WORD		RefreshHelp(WORD maxLines, BOOL fullRefresh, LONG);
Prototype BOOL		DownHelp(void);
Prototype BOOL		UpHelp(void);
Prototype int		HelpSize(void);
Prototype char		*SkipBlanks(char *s);
Prototype BOOL		DoCommand(char *command);
Prototype BOOL		CmdAlias(char *args);
Prototype BOOL		CmdUnAlias(char *args);
Prototype BOOL		CmdRegs(char *args);
Prototype BOOL		CmdJump(char *args);
Prototype BOOL 		CmdBar(char *args);
Prototype BOOL 		CmdEnd(char *args);
Prototype BOOL 		CmdItem(char *args);
Prototype BOOL 		CmdTitle(char *args);
Prototype BOOL 		CmdMenus(char *args);
Prototype BOOL 		CmdSubItem(char *args);
Prototype BOOL 		CmdChangeWindow(char *args);
Prototype BOOL 		CmdAgain(char *args);
Local 	  void 		ShortStatus(int mode, char *command);
Prototype void 		init_function_keys(void);

// ************

typedef struct CMDDEF {
	char	*helpText;
	char	*command;
	UWORD	commandLen;
	BOOL	(*func)(char *args);
} CMDDEF;


CMDDEF	commandTable[] = {
	// NOTE: These must be in alphabetical order
	{ "again            - repeat last command",			"AGAIN", 5, CmdAgain },
	{ "alias            - replace command with arexx script",	"ALIAS", 5, CmdAlias },
	{ "bp {expr}        - set breakpoint at expression",            "BP",          2, CmdBP },
	{ "bp ALL           - set all breakpoints in table",            "BP",          2, CmdBP },
	{ "breakpoints      - display breakpoint table",                "BREAKPOINTS", 11, CmdBreakpoint },
	{ "bytes {expr}     - display bytes {at expression}",           "BYTES",        5, CmdBytes },
	{ "changewindow     - move/size current window",          	"CHANGEWINDOW",        12, CmdChangeWindow },
	{ "clear expr       - clear breakpoint at expression",          "CLEAR",        5, CmdClear },
	{ "clear ALL        - clear all breakpoints in table",          "CLEAR",        5, CmdClear },
	{ "close            - close a display",                         "CLOSE",        5, CmdClose },
	{ "devs             - display Exec Device List",                "DEVS",      4, CmdDevices },
	{ "dism {expr}      - change to dism mode {at expression}",     "DISM",         4, CmdDism },
	{ "dosbase          - intelligent display of DOSBase struct",   "DOSBASE",      7, CmdDosBase },
	{ "down             - move display down one line",              "DOWN",         4, CmdDown },
	{ "eval {expr}      - evaluate an expression",                  "EVAL",         4, CmdEval },
	{ "execbase         - display ExecBase (SysBase)",              "EXECBASE",     8, CmdExecBase },
	{ "fkey             - set function key definition",             "FKEY",     4, CmdFKey },
	{ "go {expr...}     - set {expr...} temp breakpoints & go",     "GO",           2, CmdGo  },
	{ "help             - online help",		                "HELP",         4, CmdHelp },
	{ "hunks            - change to hunks mode",                    "HUNKS",        5, CmdHunks },
	{ "info             - display ThisTask info",                   "INFO",         4, CmdInfo },
	{ "intrs            - display Exec Intrs List",                 "INTRS",        5, CmdIntrs },
	{ "jump {name}      - jump to public screen",                   "JUMP",        4, CmdJump },
	{ "libs             - display Exec Libs List",                  "LIBS",         4, CmdLibs },
	{ "longs {expr}     - display longs {at expression}",           "LONGS",        5, CmdLongs },
	{ "mbar slot        - add menu bar",	        		"MBAR",   4, CmdBar },
	{ "mend slot        - add menu end mark", 	   		"MEND", 	4, CmdEnd },
	{ "memlist          - display Exec Mem List",                   "MEMLIST",      6, CmdMemList },
	{ "mitem {...}      - add menu item",		       		"MITEM",   5, CmdItem },
	{ "mixed {expr}     - change to mixed mode {at expression}",    "MIXED",        5, CmdMixed },
	{ "menus            - activate a new set of user menus",        "MENUS", 5, CmdMenus },
	{ "msub {...}       - add a sub menu item",       		"MSUB",   4, CmdSubItem },
	{ "mtitle {...}     - add menu title",		       		"MTITLE",   6, CmdTitle },
	{ "offsets          - toggle display address/offsets",          "OFFSETS",      7, CmdOffsets },
	{ "open {type}      - open a new display",                      "OPEN",         4, CmdOpen },
	{ "over {expr}      - stepover one instruction or range",       "OVER",         4, CmdOver },
	{ "pagedown         - move display down one page",		"PAGEDOWN",         8, CmdPageDown },
	{ "pageup           - move display up one page",		"PAGEUP",         6, CmdPageUp },
	{ "rendlist         - end the arexx list",   		        "RENDLIST",         8, RXEndList },	
	{ "rgetbyte {expr}  - get bytes at {expr}",    			"RGETBYTE",         8, RXGetBytes },
	{ "rgetcom          - get command line input",			"RGETCOM",         7, RXGetCommand },
	{ "rgetdism {expr}  - get the dissembled line",              	"RGETDISM",         8, RXGetDismLine },
	{ "rgeteval {expr}  - evaluate the expression",              	"RGETEVAL",         8, RXGetEval },
	{ "rgetinfo {expr}  - get the program name and line number",  	"RGETINFO",         8, RXGetInfo },
	{ "rgetline {expr}  - return the source/mixed line",          	"RGETLINE",         8, RXGetLine },
	{ "rgetlong {expr}  - get longs at {expr}",    			"RGETLONG",         8, RXGetLongs },
	{ "rgetword {expr}  - get words at {expr}",    			"RGETWORD",         8, RXGetWords },
	{ "rgetpc           - return the program counter",            	"RGETPC",         6, RXGetPC },
	{ "rputlist         - add an item to the arexx list",         	"RPUTLIST",         8, RXPutList },	
	{ "rshowlist        - show the arexx list",   			"RSHOWLIST",         9, RXEndList },	
	{ "rstartlist       - start the arexx list",   			"RSTARTLIST",         10, RXStartList },	
	{ "rx rexx-script   - execute ARexx macro",                     "RX",         2, CmdRexx },
	{ "saveprefs        - save dd prefs to disk",                 	"SAVEPREFS",    9, CmdSavePrefs },
	{ "set {addr} {val} - set address to val",                 	"SET",    3, CmdSet },
	{ "ports            - display Exec Ports List",                 "PORTS",        5, CmdPorts },
	{ "process {expr}   - display process {at expression}",         "PROCESS",      7, CmdProcess },
	{ "resources        - display Exec Resource List",              "RESOURCES",    9, CmdResources },
	{ "quit             - _exit(20) & quit dd",                     "QUIT",         4, CmdQuit },
	{ "refresh          - refresh the window",                      "REFRESH",      7, CmdRefresh },
	{ "registers        - toggle the register display",             "REGISTERS",      9, CmdRegs },
	{ "reset            - _exit(20) & restart program",             "RESET",        5, CmdReset },
	{ "rstep            - toggle library call auto stepover flag",	"RSTEP",         5, CmdRStep },
	{ "source {expr}    - change to source mode {at expression}",   "SOURCE",       6, CmdSource },
	{ "step {expr}      - step program one instruction or range",   "STEP",         4, CmdStep },
	{ "symbols {expr}   - display symbol {at expr}",                "SYMBOLS",       7, CmdSymbol },
	{ "symlist          - display sorted symbol list",              "SYMLIST",       7, CmdSymList },
	{ "tasks            - display Exec Task Lists",                 "TASKS",        5, CmdTasks },
	{ "unalias          - restore command with arexx script",	"UNALIAS", 7, CmdUnAlias },
	{ "up               - move display up one line",                "UP",           2, CmdUp },
	{ "watchbyte {expr} - set/clear (toggle) byte watchpoints",     "WATCHBYTE",       9, CmdWatchBytes },
	{ "watchclear       - clear all watchpoints",     		"WATCHCLEAR",       10, CmdWatchClear },
    	{ "watchlong {expr} - set/clear (toggle) long watchpoints",     "WATCHLONG",       9, CmdWatchLongs },
	{ "watchword {expr} - set/clear (toggle) word watchpoints",     "WATCHWORD",       9, CmdWatchWords },
	{ "words {expr}     - display words {at expression}",           "WORDS",        5, CmdWords },
	{ "",                                                            NULL,           0, NULL }
};

char	*HelpTable[] = {
	"NOTES",
	"MOUSE CLICK:",
	"You can click the mouse on a label, register, or address",
	"To apply it to the command line.  The operators displayed",
	"in the command line are there for the express purpose of",
	"clicking on them.  Clicking on '<' cancels a line, Clicking",
	"on '>' executes it",
	"",
	"You can also click to the right of the command line to",
	"execute the current command.  Function key equivalents",
	"may be executed with a mouse click by clicking on the",
	"number (e.g. the '4' in '4:bytes')",
	"",
	"EXPRESSION:",
	"A C expression evaluator allowing parenthesis and most C",
	"operators, including longword indirection with *",
	"",
	"Note that C constants are used, so hex values must be",
	"preceeded by 0x, octal with 0, decimal by default.",
	""
};


// ************

char	*default_fkeys[10] = {
	"source",
	"mixed",
	"dism",
	"bytes",
	"symbols",
	"pc",
	"step",
	"over",
	"reset",
	"go"
};

Prototype char	fkeys[10][16];
char	fkeys[10][16];

void	RefreshFKeys(BOOL fullRefresh) {
	UWORD	i;

    if(fullRefresh == -1) {
	ScrPlain();
	ScrRowCol(CurDisplay->ds_ScrRows, 2);
	ScrDull();

	for (i=0; i<10; i++) {
	    ScrPrintf("%d:%s ", i+1, fkeys[i]);
	}
	ScrPlain();
	ScrEOL();

	ScrFlush();
	draw_fkey_boxes();
    }
}

void init_function_keys(void)
{
int i;
    
    memset((char *)fkeys,0,sizeof(fkeys));
    for(i=0; i<10; i++)strcpy(fkeys[i],default_fkeys[i]);
}




BOOL	FunctionKey(UWORD num) {
	if (num < 10) return DoCommand(fkeys[num]);
	return 0;
}

BOOL	HelpKey(void) {
	return DoCommand("help");
}


BOOL CmdFKey(char *args) {
char *ptr;
int n;

    if(ptr = strchr(args,' ')) {	// if there is a slot
	*ptr++ = NULL;
	if(((n = strtol(args,NULL,10)) > 0) && (n <= 10))  {
    	    if(*ptr) {	// and if there is a definition
		strncpy(fkeys[n-1],ptr,16);
		ScrStatus("Function key set");
		RefreshFKeys(-1);
		return TRUE;
	    }
	}
    }
    ScrStatus("*** Can't set function key %d",n);
    return FALSE;
}

// ************

// Handle Rexx Command.  This routine looks in current directory, then
// searches dcc:rexx/

BOOL	CmdRexx(char *arg) {
	long	rc = 1;
	char	*res;
	long	ec;
	char	buf[128];

	// don't add a . if they already have one
	if(!strchr(arg,'.'))sprintf(buf, "%s.dd", arg);
	else strcpy(buf, arg);

	rc = PlaceRexxCommand(NULL, buf, &res, &ec);
	if (res) {
	    ScrStatus(res);
	    free(res);
	}
	if (ec || rc) {
	    // try in dcc:rexx
	    sprintf(buf, "dcc:rexx/%s.dd", arg);
	    rc = PlaceRexxCommand(NULL, buf, &res, &ec);
	    if (res) {
	    	ScrStatus(res);
		free(res);
	    }
	    if (ec || rc)return FALSE;
	}
	return TRUE;
}

BOOL	CmdSavePrefs(char *args) {
	WINDOW	*window = CurDisplay->ds_Win;

	dprefs.left = window->LeftEdge; dprefs.top = window->TopEdge;
	dprefs.width = window->Width; dprefs.height = window->Height;
	WritePrefs();
	ScrStatus("Wrote Prefs");
	return TRUE;
}

BOOL	CmdInfo(char *args) {
	ListInfo(CurDisplay);
	/*CurDisplay->ds_ListTop = (DLIST *)CurDisplay->ds_List.lh_Head;*/
	RefreshWindow(TRUE);
	return TRUE;
}

BOOL	CmdExecBase(char *args) {
	ListExecBase(CurDisplay);
	/*CurDisplay->ds_ListTop = (DLIST *)CurDisplay->ds_List.lh_Head;*/
	RefreshWindow(TRUE);
	return TRUE;
}

BOOL	CmdResources(char *args) {
	ListResources(CurDisplay);
	/*CurDisplay->ds_ListTop = (DLIST *)CurDisplay->ds_List.lh_Head;*/
	RefreshWindow(TRUE);
	return TRUE;
}

BOOL	CmdIntrs(char *args) {
	ListIntrs(CurDisplay);
	/*CurDisplay->ds_ListTop = (DLIST *)CurDisplay->ds_List.lh_Head;*/
	RefreshWindow(TRUE);
	return TRUE;
}

BOOL	CmdPorts(char *args) {
	ListPorts(CurDisplay);
	/*CurDisplay->ds_ListTop = (DLIST *)CurDisplay->ds_List.lh_Head;*/
	RefreshWindow(TRUE);
	return TRUE;
}

BOOL	CmdSymList(char *args) {
	ListSymbols(CurDisplay);
	RefreshWindow(TRUE);
	return TRUE;
}

BOOL	CmdTasks(char *args) {
	ListTasks(CurDisplay);
	/*CurDisplay->ds_ListTop = (DLIST *)CurDisplay->ds_List.lh_Head;*/
	RefreshWindow(TRUE);
	return TRUE;
}

BOOL	CmdLibs(char *args) {
	ListLibs(CurDisplay);
	/*CurDisplay->ds_ListTop = (DLIST *)CurDisplay->ds_List.lh_Head;*/
	RefreshWindow(TRUE);
	return TRUE;
}

BOOL	CmdDevices(char *args) {
	ListDevices(CurDisplay);
	/*CurDisplay->ds_ListTop = (DLIST *)CurDisplay->ds_List.lh_Head;*/
	RefreshWindow(TRUE);
	return TRUE;
}

BOOL	CmdMemList(char *args) {
	ListMemList(CurDisplay);
	/*CurDisplay->ds_ListTop = (DLIST *)CurDisplay->ds_List.lh_Head;*/
	RefreshWindow(TRUE);
	return TRUE;
}


BOOL	CmdDosBase(char *args) {
	ListDosBase(CurDisplay);
	/*CurDisplay->ds_ListTop = (DLIST *)CurDisplay->ds_List.lh_Head;*/
	RefreshWindow(TRUE);
	return TRUE;
}

BOOL	CmdProcess(char *args) {
	ListProcess(CurDisplay);
	/*CurDisplay->ds_ListTop = (DLIST *)CurDisplay->ds_List.lh_Head;*/
	RefreshWindow(TRUE);
	return TRUE;
}

BOOL	CmdOffsets(char *args) {
	CurDisplay->ds_DisplayOffsets = !CurDisplay->ds_DisplayOffsets;
	RefreshWindow(TRUE);
	return TRUE;
}

BOOL	CmdDown(char *args) {
	switch (CurDisplay->ds_DisplayMode) {
		case DISPLAY_DISM:	return DownDism();
		case DISPLAY_SOURCE:	return DownSource();
		case DISPLAY_MIXED:	return DownMixed();
		case DISPLAY_BYTES:	return DownBytes();
		case DISPLAY_WORDS:	return DownWords();
		case DISPLAY_LONGS:	return DownLongs();
		case DISPLAY_HUNKS:	return DownHunks();
		case DISPLAY_BREAK:	return DownBreak();
		case DISPLAY_HELP:	return DownHelp();
		case DISPLAY_SYMBOL:	return DownSymbol();

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
		case DISPLAY_SYMLIST:
		case DISPLAY_REXXLIST:	return DownList();

		default:		break;
	}
	return TRUE;
}

BOOL	CmdPageDown(char *args) {
	switch (CurDisplay->ds_DisplayMode) {
		case DISPLAY_DISM:	return PageDownDism();
		case DISPLAY_SOURCE:	return PageDownSource();
		case DISPLAY_MIXED:	return PageDownMixed();
		case DISPLAY_BYTES:	return PageDownHex();
		case DISPLAY_WORDS:	return PageDownHex();
		case DISPLAY_LONGS:	return PageDownHex();
		case DISPLAY_HUNKS:	return DownHunks();
		case DISPLAY_BREAK:	return DownBreak();
		case DISPLAY_HELP:	return DownHelp();
		case DISPLAY_SYMBOL:	return PageDownSymbol();

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
		case DISPLAY_SYMLIST:
		case DISPLAY_REXXLIST:	return PageDownList();

		default:		break;
	}
	return TRUE;
}

BOOL	CmdPageUp(char *args) {
	switch (CurDisplay->ds_DisplayMode) {
		case DISPLAY_DISM:	return PageUpDism();
		case DISPLAY_SOURCE:	return PageUpSource();
		case DISPLAY_MIXED:	return PageUpMixed();
		case DISPLAY_BYTES:	return PageUpHex();
		case DISPLAY_WORDS:	return PageUpHex();
		case DISPLAY_LONGS:	return PageUpHex();
		case DISPLAY_HUNKS:	return UpHunks();
		case DISPLAY_BREAK:	return UpBreak();
		case DISPLAY_HELP:	return UpHelp();
		case DISPLAY_SYMBOL:	return PageUpSymbol();

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
		case DISPLAY_SYMLIST:
		case DISPLAY_REXXLIST:	return PageUpList();

		default:		break;
	}
	return TRUE;
}

BOOL	CmdUp(char *args) {
	switch (CurDisplay->ds_DisplayMode) {
		case DISPLAY_DISM:	return UpDism();
		case DISPLAY_SOURCE:	return UpSource();
		case DISPLAY_MIXED:	return UpMixed();
		case DISPLAY_BYTES:	return UpBytes();
		case DISPLAY_WORDS:	return UpWords();
		case DISPLAY_LONGS:	return UpLongs();
		case DISPLAY_HUNKS:	return UpHunks();
		case DISPLAY_BREAK:	return UpBreak();
		case DISPLAY_HELP:	return UpHelp();
		case DISPLAY_SYMBOL:	return UpSymbol();

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
		case DISPLAY_SYMLIST:
		case DISPLAY_REXXLIST:	return UpList();

		default:		break;
	}
	return TRUE;
}

BOOL	CmdSource(char *args) {
	ULONG	val;
	short	undef;

	SetDisplayMode(DISPLAY_SOURCE, 1);

	if (*args) {
		val = ParseExp(args, &undef, strlen(args));
		if (undef) return FALSE;
		CurDisplay->ds_WindowTop = val;
		CurDisplay->ds_WindowTopLine = 0;
	}

	RefreshWindow(TRUE);
	return TRUE;
}



BOOL	CmdDism(char *args) {
	ULONG	val;
	short	undef;

	SetDisplayMode(DISPLAY_DISM, 1);

	if (*args) {
		val = ParseExp(args, &undef, strlen(args));
		if (undef) return FALSE;
		CurDisplay->ds_WindowTop = val;
	}
	RefreshWindow(TRUE);
	return TRUE;
}


BOOL	CmdRegs(char *args) {

	CurDisplay->ds_RegFlag = (CurDisplay->ds_RegFlag) ? 0 : 1;
	CurDisplay->ds_RegTouched = TRUE;
	RefreshWindow(TRUE);
	return TRUE;
}

BOOL	CmdEval(char *args) {
	ULONG	val;
	short	undef;
	BOOL r = TRUE;

	if (*args) {
		val = ParseExp(args, &undef, strlen(args));
		if (undef) {
		    r = FALSE;
		    ScrStatus("Result undefined");
		} else {
		    char buf[128];

		    OffsetAddressBuf(val, buf);
		    ScrStatus("Result %d (0x%08lx) (%s)", val, val, buf);
		}
	}
	return r;
}

BOOL	CmdOpen(char *args) {
	DBugDisp *disp, *olddisp;

	disp = ScrOpen(FALSE, TRUE, DefaultPubName);
	if(*args && disp) {
	    olddisp = CurDisplay;
	    CurDisplay = disp;
	    DoCommand(args);
	    InitCommand();
	    ScrCursoff();
	    RefreshCommand(TRUE);
	    ScrCurson();
	    SetDisplayMode(CurDisplay->ds_DisplayMode, 1);  // make this the default for this window
	    CurDisplay = olddisp;
	}
	return TRUE;
}

BOOL	CmdClose(char *args) {

	RequestCloseDisplay(CurDisplay,TRUE);
	return TRUE;
}

BOOL	CmdMixed(char *args) {
	ULONG	val;
	short	undef;

	SetDisplayMode(DISPLAY_MIXED, 1);
	if (*args) {
		val = ParseExp(args, &undef, strlen(args));
		if (undef) return FALSE;
		CurDisplay->ds_WindowTop = val;
		CurDisplay->ds_WindowTopLine = 0;
	}
	RefreshWindow(TRUE);
	return TRUE;
}

BOOL	CmdBytes(char *args) {
	ULONG	val;
	short	undef;

	SetDisplayMode(DISPLAY_BYTES, 1);

	if (*args) {
		val = ParseExp(args, &undef, strlen(args));
		if (undef) return FALSE;
		CurDisplay->ds_WindowTop = val;
	}
	RefreshWindow(TRUE);
	return TRUE;
}

BOOL	CmdWords(char *args) {
	ULONG	val;
	short	undef;

	SetDisplayMode(DISPLAY_WORDS, 1);

	if (*args) {
		val = ParseExp(args, &undef, strlen(args));
		if (undef) return FALSE;
		CurDisplay->ds_WindowTop = val;
	}
	RefreshWindow(TRUE);
	return TRUE;
}

BOOL	CmdLongs(char *args) {
	ULONG	val;
	short	undef;

	SetDisplayMode(DISPLAY_LONGS, 1);

	if (*args) {
		val = ParseExp(args, &undef, strlen(args));
		if (undef) return FALSE;
		CurDisplay->ds_WindowTop = val;
	}
	RefreshWindow(TRUE);
	return TRUE;
}

BOOL	CmdBreakpoint(char *args) {

	SetDisplayMode(DISPLAY_BREAK, 0);
	RefreshWindow(TRUE);
	return TRUE;
}

BOOL	CmdBP(char *args) {
	ULONG	val;
	short	undef;

	if (*args) {
		if (!strnicmp(args, "ALL", 3)) {
			SetAllBreakpoints();
			ScrStatus("Breakpoint table set");
			RefreshAllWindows(TRUE);
			return TRUE;
		}
		val = ParseExp(args, &undef, strlen(args));
		if (undef || !val) val = CurDisplay->ds_WindowTop;

		// check if break point is already set
		// if so, clear it
		if(IsBreakpoint(val))return(CmdClear(args));

		if (SetBreakpoint(val, 1, BP_SET)) {
			RefreshAllWindows(TRUE);
			ScrStatus("BP set at 0x%08x", val);
			return TRUE;
		}
		else {
			ScrStatus("*** Can't set BP at 0x%08x", val);
			return FALSE;
		}
	}
	ScrStatus("*** Usage: bp {expr}");
	return FALSE;
}

BOOL	CmdClear(char *args) {
	ULONG	val;
	short	undef;

	if (*args) {
		if (!strnicmp(args, "ALL", 3)) {
			InitBreakpoints();
			ScrStatus("Breakpoint table cleared");
			RefreshAllWindows(TRUE);
			return TRUE;
		}
		val = ParseExp(args, &undef, strlen(args));
		if (undef) return FALSE;
		if (ClearBreakpoint(val)) {
			RefreshAllWindows(TRUE);
			ScrStatus("BP cleared at 0x%08x", val);
			return TRUE;
		}
		else {
			ScrStatus("*** Can't clear BP at 0x%08x", val);
			return FALSE;
		}
	}
	ScrStatus("*** Usage: clear {expr}");
	return FALSE;
}

BOOL	CmdHunks(char *args) {

	SetDisplayMode(DISPLAY_HUNKS, 0);
	RefreshWindow(TRUE);
	return TRUE;
}

BOOL	CmdSymbol(char *args) {
	ULONG	val;
	short	undef;

	SetDisplayMode(DISPLAY_SYMBOL, 0);

	if (*args) {
		val = ParseExp(args, &undef, strlen(args));
		if (undef) return FALSE;
		CurDisplay->ds_WindowTop = SymbolIndexOfAddr(val);     // pseudo index
	}
	RefreshWindow(TRUE);
	return TRUE;
}

static char *casts[3] = { "(BYTE)","(WORD)","(LONG)"};



BOOL	CmdSet(char *args) {
	UBYTE	*adb;
	UWORD   *adw;
	ULONG	*adl;
	ULONG	val=0, addr, address;
	short	undef = 1;
	int i, castsize = 2;
	char *ptr,*ptr1;

    if (*args) {
	ptr = args;
	ptr1 = strchr(args,' ');	// split into two strings
	if(ptr1) {
	    *ptr1++ = NULL;
	    for(i=0; i<3; i++) {  // check for cast operator
		if(!stricmp(casts[i],ptr)) {
		    castsize = i;
		    // adjust the pointers
		    ptr = ptr1;
		    if(!(ptr1 = strchr(ptr,' ')))return FALSE;
		    *ptr1++ = NULL;	// split the string
		    break;
		}
	    }
	    // check it its a register
            if((strlen(ptr) >= 3) && (ptr[2]==':')) {
		 ptr[2]=' ';
		 if((addr = (ULONG)RegisterAddress(ptr)))undef = 0;	// yes, its a register
	    }
	    if(undef)addr = ParseExp(ptr, &undef, strlen(ptr));  // must be an address
	    if(!undef) {
	        val = ParseExp(ptr1, &undef, strlen(ptr1));
	        if (address = ValidMemCheck((ULONG)addr)) {
	    	    switch (castsize) {
			case 0:
		    	    adb = (UBYTE *)address;
		    	    *adb = (BYTE)val;
		    	    break;
			case 1:
		    	    adw = (UWORD *)address;
		    	    *adw = (WORD)val;
		    	    break;
			case 2:
		    	    adl = (ULONG *)address;
		    	    *adl = (LONG)val;
		    	    break;

			default:
		    	    return FALSE;
		    }
		    ScrStatus("%08X set to %X",addr,val);
		    RefreshAllWindows(FALSE);
	    	    return TRUE;
		}
		ScrStatus("*** Can't set %08X to %X",addr,val);
		return FALSE;
	    }
	}
    }
    ScrStatus("*** Usage: set {expr} {value}");
    return FALSE;
}

BOOL CmdAgain(char *args) {
    return DoCommand(lastCommand);
}

BOOL CmdChangeWindow(char *args) {
char *ptr, *ptr1;
int size[4] = {-1, -1, -1, -1};
int i = -1;

    if(((struct Library *)SysBase)->lib_Version >= 36) {

    	ptr = args;
    	while ((++i < 4) && *ptr) {
	    size[i]=strtol(ptr,&ptr1,10);
	    ptr = ptr1;
    	}
    	if(i == 4) {
	    ChangeWindowBox(CurDisplay->ds_Win,size[0],size[1],size[2],size[3]);
	    ScrStatus("Window repositioned\n");
	    return TRUE;
    	}
        ScrStatus("*** Usage: ChangeWindow {left} {top} {width} {height}");
    }
    return FALSE;
}


BOOL	StepTargetRange(ULONG addressLow, ULONG addressHi) {

	// step within the address range given
	// (execute at least once, though)
	do {
	    CopyLastState();
	    programSR |= 0x8000;
	    EnterProgram();
	    programSR &= 0x7fff;
	} while((programPC >=  addressLow) && (programPC <= addressHi));

	if (CurDisplay->ds_DisplayMode > DISPLAY_MIXED)
		SetDisplayMode(CurDisplay->ds_PreferedMode, 0);

	if (programState == STATE_TRACE)programState = STATE_STEPPED;

	if ((programPC <= CurDisplay->ds_WindowTop) || (programPC >= CurDisplay->ds_WindowBot)) {
		CurDisplay->ds_WindowTop = programPC;
		ForceFullRefresh = 1;
	}
	return (programState == STATE_STEPPED);
}

BOOL	StepTarget(void) {

	CopyLastState();
	programSR |= 0x8000;
	EnterProgram();
	programSR &= 0x7fff;

	if (CurDisplay->ds_DisplayMode > DISPLAY_MIXED)
		SetDisplayMode(CurDisplay->ds_PreferedMode, 0);

	if (programState == STATE_TRACE)programState = STATE_STEPPED;
	if ((programPC <= CurDisplay->ds_WindowTop) || (programPC >= CurDisplay->ds_WindowBot)) {
		CurDisplay->ds_WindowTop = programPC;
		ForceFullRefresh = 1;
	}


	return (programState == STATE_STEPPED);
}

BOOL	CmdRStep(char *args)
{
    RStepFlag = !RStepFlag;
    if(!RStepFlag)ScrStatus("Library step over off");
    else ScrStatus("Library step over on");
    return TRUE;
}


// If an argument given, will do a range single step 

BOOL	CmdStep(char *args) {
	BOOL	result, fullRefresh = FALSE;
	ULONG endrange = 0;
	short undef;

	// process the range argument
	if(*args) {
		// stop before the specified instruction
		endrange = ParseExp(args, &undef, strlen(args)) - 2;
		if (undef) endrange = 0;
	}
	switch (CurDisplay->ds_PreferedMode) {
		case DISPLAY_SOURCE: {
		    ULONG address = programPC, line = 0;
		    WORD type, breakflag = 0;

		    if(CurrentMixedLine(&address,&line,NULL) == MIXTYPE_SOURCE) {
			// find end of address range for the target range
			while((type = NextMixedLine(&address,&line,NULL)) == MIXTYPE_SOURCE);

			while(type == MIXTYPE_DISM) {
			    endrange = address;
			    while(NextMixedLine(&address,&line,NULL) == MIXTYPE_DISM) {
				endrange = address;
			    }
			    if(endrange >= programPC) {
			        if(!RStepFlag)result = StepTargetRange(programPC,endrange);
				else result = RStepTargetRange(programPC,endrange);
				address = programPC;
				line = 0;
			    	if(CurrentMixedLine(&address,&line,NULL) == MIXTYPE_DISM)continue;
				breakflag = TRUE; // switch break
			        break;
			    }
			}
			if(breakflag)break;
		    }
#if 0
			//  This doesn't really work all that well
			//  So its gone

	DEBUG	*debug;
			while (1) {
				if (!OverTarget()) { result = FALSE; break; }
				debug = FindDebug(programPC);
				if (!debug)
					result = FALSE;
				else {
					if (FindSource(debug, programPC)) { result = TRUE; break; }
				}
			}
			break;
#endif

		}
		case DISPLAY_MIXED:
		case DISPLAY_DISM:
			if(endrange) {
			    if(!RStepFlag)result = StepTargetRange(programPC,endrange);
			    else result = RStepTargetRange(programPC,endrange);
			}
			else {
			    if(!RStepFlag)result = StepTarget();
			    else result = RStepTarget();
			}
			break;
	}
	RefreshAllWindows(FALSE);
	return result;
}

BOOL	OverTargetRange(ULONG addressLow, ULONG addressHi) {
	char	buf[128];
	ULONG	size, twp;
	UWORD	*pw;

    do {
	pw = (UWORD *)programPC;
	// over at a forward branch is treated as a regular StepTarget()
	// Check for Bcc instruction
	if (((*pw & 0xf000) == 0x6000) && ((*pw & 0xff00) != 0x6100)) {	
		if ((*pw & 0xff) == 0xff) {		// 32-bit displacement (020+ only)
		    ULONG *pl = (ULONG *)&pw[1];
		    if (*pl > 0) {
			StepTarget();
	        	continue;
		    }
		}
		else if ((*pw & 0xff) == 0x00) {	// 16-bit displacement
		    UWORD *lpw = (UWORD *)&pw[1];
		    if (*lpw > 0) {
			StepTarget();
	        	continue;
		    }
		}
		else {					// 8-bit displacement
		    if ((*pw & 0xff) < 0x80) {
			StepTarget();
	        	continue;
		    }
		}
	}
	size = Disassemble(programPC, programPC, buf);
	// over at RTS, or JMP is treated as a regular StepTarget()
	if (!strnicmp(buf, "RTS", 3) || !strnicmp(buf, "JMP", 3)) {
	    StepTarget();
	    continue;
	}

	// set temp breakpoint at instruction after current PC
        if(TypeOfMem((UBYTE *)((ULONG)programPC)+size)) {
	    SetTempBreakpoint(programPC+size);
	    if (!GoTarget()) return FALSE;
	}
	else {
	    twp = programPC+size;
    	    while(twp != programPC) {
		if(!StepTarget()) {
		    if(programState == STATE_EXITED)return FALSE;
		    break;
		}
	    }
	    if(programState == STATE_STEPPED)programState = STATE_STEPPEDOVER;
	}
	if (programPC < CurDisplay->ds_WindowTop || programPC > CurDisplay->ds_WindowBot) {
		CurDisplay->ds_WindowTop = programPC;
		ForceFullRefresh = 1;
	}
    } while ((programPC >=  addressLow) && (programPC <= addressHi));
    return (programState == STATE_STEPPEDOVER);
}

BOOL	OverTarget(void) {
	char	buf[128];
	ULONG	size, twp;
	UWORD	*pw = (UWORD *)programPC;
    // over at a forward branch is treated as a regular StepTarget()
    if (((*pw & 0xf000) == 0x6000) && ((*pw & 0xff00) != 0x6100)) {	// Bcc instruction
	if ((*pw & 0xff) == 0xff) {		// 32-bit displacement (020+ only)
	    ULONG	*pl = (ULONG *)&pw[1];
	    if (*pl > 0) return StepTarget();
	}
	else if ((*pw & 0xff) == 0x00) {	// 16-bit displacement
	    UWORD	*lpw = (UWORD *)&pw[1];
	    if (*lpw > 0) return StepTarget();
	}
	else {					// 8-bit displacement
	    if ((*pw & 0xff) < 0x80) {
		return StepTarget();
	    }
	}
    }

    size = Disassemble(programPC, programPC, buf);
    // over at RTS, or JMP is treated as a regular StepTarget()
    if (!strnicmp(buf, "RTS", 3) || !strnicmp(buf, "JMP", 3)) return StepTarget();

	// set temp breakpoint at instruction after current PC
        if(TypeOfMem((UBYTE *)((ULONG)programPC)+size)) {
	    SetTempBreakpoint(programPC+size);
	    if (!GoTarget()) return FALSE;
	}
	else {
	    twp = programPC+size;
    	    while(twp != programPC) {
		if(!StepTarget()) {
		    if(programState == STATE_EXITED)return FALSE;
		    break;
		}
	    }
	    if(programState == STATE_STEPPED)programState = STATE_STEPPEDOVER;
	}
	if ((programPC <= CurDisplay->ds_WindowTop) || (programPC >= CurDisplay->ds_WindowBot)) {
		CurDisplay->ds_WindowTop = programPC;
		ForceFullRefresh = 1;
	}

    return (programState == STATE_STEPPEDOVER);
}

// Rom Step Target
// Automatically steps over library calls
// otherwise single steps

BOOL	RStepTarget(void) {
	char	buf[128];
	ULONG	size, twp;

    size = Disassemble(programPC, programPC, buf);
    if (strnicmp(buf, "JSR", 3))return StepTarget();
    if(!strchr(buf,'-') || !strstr(buf,"(A6)"))return StepTarget();

    // well, its a JSR negative offset through A6, so lets skip over

    // set temp breakpoint at instruction after current PC
    if(TypeOfMem((UBYTE *)((ULONG)programPC)+size)) {
    	SetTempBreakpoint(programPC+size);
    	if (!GoTarget()) return FALSE;
    }
    else {
	// we're in rom;  setting the breakpoint will not work well
    	twp = programPC+size;
    	while(twp != programPC) {
	    if(!StepTarget()) {
	    	if(programState == STATE_EXITED)return FALSE;
		break;
	    }
	}
        if(programState == STATE_STEPPED)programState = STATE_STEPPEDOVER;
    }
    if (programPC <= CurDisplay->ds_WindowTop || programPC >= CurDisplay->ds_WindowBot) {
	CurDisplay->ds_WindowTop = programPC;
	ForceFullRefresh = 1;
    }

    return (programState == STATE_STEPPEDOVER);
}



BOOL	RStepTargetRange(ULONG addressLow, ULONG addressHi) {
	char	buf[128];
	ULONG	size, twp;

    do {
	size = Disassemble(programPC, programPC, buf);

    if (strnicmp(buf, "JSR", 3))return StepTarget();
    if(!strchr(buf,'-') || !strstr(buf,"(A6)"))return StepTarget();

    // well, its a JSR negative offset through A6, so lets skip over

	// set temp breakpoint at instruction after current PC
        if(TypeOfMem((UBYTE *)((ULONG)programPC)+size)) {
	    SetTempBreakpoint(programPC+size);
	    if (!GoTarget()) return FALSE;
	}
	else {
	    twp = programPC+size;
    	    while(twp != programPC) {
	    	if(!StepTarget()) {
	    	    if(programState == STATE_EXITED)return FALSE;
		    break;
		}
	    }
            if(programState == STATE_STEPPED)programState = STATE_STEPPEDOVER;
	}
	if (programPC < CurDisplay->ds_WindowTop || programPC > CurDisplay->ds_WindowBot) {
		CurDisplay->ds_WindowTop = programPC;
		ForceFullRefresh = 1;
	}
    } while ((programPC >=  addressLow) && (programPC <= addressHi));

    return (programState == STATE_STEPPEDOVER);
}

BOOL	CmdOver(char *args) {
	BOOL	result;
	ULONG  endrange = 0;
	short undef;

	// process the range argument
	if(*args) {
		// stop before the specified instruction
		endrange = ParseExp(args, &undef, strlen(args)) - 2;
		if (undef) endrange = 0;
	}

	switch (CurDisplay->ds_PreferedMode) {
		case DISPLAY_SOURCE: {
		    ULONG address = programPC, line = 0;
		    WORD type, breakflag = 0;

		    type = CurrentMixedLine(&address,&line,NULL);
		    if (type == 0)type = NextMixedLine(&address, &line, NULL);
		    if(type == MIXTYPE_SOURCE) {
			// find end of address range for the target range
			while((type = NextMixedLine(&address,&line,NULL)) == MIXTYPE_SOURCE);

			while(type == MIXTYPE_DISM) {
			    endrange = address;
			    while(NextMixedLine(&address,&line,NULL) == MIXTYPE_DISM)endrange = address;
			    if(endrange >= programPC) {
			        result = OverTargetRange(programPC,endrange);

				// find the next source line (if not on a source line)
				// and execute to it
				address = programPC;	// on a source line yet ?
				line = 0;
			    	type = CurrentMixedLine(&address,&line,NULL);
			    }
			}
		        if(type == MIXTYPE_SOURCE)break;
		    }
		    // drop through to mixed case
		    endrange = 0;
		}
		case DISPLAY_MIXED:
		case DISPLAY_DISM:
			if(endrange)result = OverTargetRange(programPC,endrange);
			else result = OverTarget();
			break;
	}
	RefreshAllWindows(FALSE);
	return result;
}

BOOL	CmdRefresh(char *args) {

	RefreshWindow(TRUE);
}


BOOL	CmdReset(char *args) {
	ULONG	i;

	if (programState != STATE_RESET && programState != STATE_EXITED &&
	    (LookupSymbol("@exit", &i) || LookupSymbol("_exit", &i))) {
		programPC = i;
		GoTarget();
		if (programState != STATE_EXITED) { 
			RefreshWindow(TRUE); 
			return FALSE; 
		}
	}
	ResetTarget();
	RefreshWindow(TRUE);
	ScrStatus("Target Reset");
	return TRUE;
}

BOOL	GoTarget(void) {

	if (IsBreakpoint(programPC)) {
		programSR |= 0x8000; 
		EnterDebugger(); 
		programSR &= 0x7fff;
		if (programState != STATE_TRACE) return FALSE;
	}
	InstallBreakpoints();

	CopyLastState();
	EnterProgram();
	CheckBreakpoints();
	
	if(CurDisplay) {
	    if (CurDisplay->ds_DisplayMode > DISPLAY_MIXED)
	        SetDisplayMode(CurDisplay->ds_PreferedMode, 0);

	    if (programPC < CurDisplay->ds_WindowTop || programPC > CurDisplay->ds_WindowBot)
		CurDisplay->ds_WindowTop = programPC;
	}
	return TRUE;
}

Local void	CopyLastState(void) {
	lastState = programState;
	lastSR = programSR;
	lastPC = programPC;
	lastD0 = programD0; lastD1 = programD1; lastD2 = programD2; lastD3 = programD3;
	lastD4 = programD4; lastD5 = programD5; lastD6 = programD6; lastD7 = programD7;
	lastA0 = programA0; lastA1 = programA1; lastA2 = programA2; lastA3 = programA3;
	lastA4 = programA4; lastA5 = programA5; lastA6 = programA6; lastA7 = programA7;
}

BOOL	CmdGo(char *args) {
	char	symbol[128];
	char	*pd;
	ULONG	addr;

	while (*args) {
		while (*args == ' ' || *args == '\t') args++;
		if (*args) {
			WORD	undef;

			pd = &symbol[0];
			while (*args && *args != ' ' && *args != '\t') *pd++ = *args++;
			*pd = '\0';
			addr = ParseExp(symbol, &undef, strlen(symbol));
			if (undef) return FALSE;
			if (!SetBreakpoint(addr, 1, BP_GROUP)) return FALSE;
		}
	}
	GoTarget();
//	RefreshWindow(FALSE);
	RefreshAllWindows(FALSE);	// just in case a breakpoint was hit.
					// probably should limit refresh to
					// source, asm, mixed, and bp wondows
	return TRUE;
}

BOOL	CmdQuit(char *args) {
	ULONG	i;

	if (programState != STATE_RESET && programState != STATE_EXITED) {
		if (LookupSymbol("@exit", &i)) {
			programD0 = RETURN_FAIL;
			programPC = i;
			GoTarget();
			if (programState != STATE_EXITED) { 
			    if(CurDisplay)RefreshWindow(TRUE); 
			    return FALSE; 
			}
		}
		else if (LookupSymbol("_exit", &i)) {
			ULONG	*pl = (ULONG *)programA7;
			--pl;
			*pl = RETURN_FAIL;
			programA7 = (ULONG)pl;
			programPC = i;
			GoTarget();
			if (programState != STATE_EXITED) { 
			    if(CurDisplay) {
				RefreshWindow(TRUE); 
			    }
			    return FALSE; 
			}
			// pick up return code and load into D0
			programD0 = *pl;
		}
	}
	printf("\n*** %s Exited, code %d\n", targetName, programD0);
	exit(0);
	return TRUE;
}

BOOL	CmdHelp(char *args) {

	if (*args == 0 && CurDisplay->ds_DisplayMode == DISPLAY_HELP) {
	    SetDisplayMode(CurDisplay->ds_PreferedMode, 0);
	} else {
		SetDisplayMode(DISPLAY_HELP, 0);
		if (CurDisplay->ds_WindowTop < 0)
		    CurDisplay->ds_WindowTop = 0;
		if (CurDisplay->ds_WindowTop > arysize(commandTable) + arysize(HelpTable))
		    CurDisplay->ds_WindowTop = arysize(commandTable) + arysize(HelpTable) - 1;
	}
	RefreshWindow(TRUE);
	return TRUE;
}

// ************

BOOL	CmdWatchClear(char *args) {

        InitWatchpoints();
	RefreshWindow(TRUE);
	return TRUE;
}


BOOL	CmdWatchBytes(char *args) {
	char	symbol[128];
	char	*pd;
	BOOL	fullRefresh = FALSE;

	while (*args) {
		while (*args == ' ' || *args == '\t') args++;
		if (*args) {
			pd = &symbol[0];
			while (*args && *args != ' ' && *args != '\t') *pd++ = *args++;
			*pd = '\0';
			if (IsWatchpoint(symbol)) {
				if (!ClearWatchpoint(symbol)) return FALSE;
				fullRefresh = TRUE;
			}
			else {
			    if (!SetWatchpoint(symbol, WP_BYTES)) return FALSE;
			}
			fullRefresh = TRUE;
		}
	}
	RefreshWindow(fullRefresh);
	return TRUE;
}

BOOL	CmdWatchWords(char *args) {
	char	symbol[128];
	char	*pd;
	BOOL	fullRefresh = FALSE;

	while (*args) {
		while (*args == ' ' || *args == '\t') args++;
		if (*args) {
			pd = &symbol[0];
			while (*args && *args != ' ' && *args != '\t') *pd++ = *args++;
			*pd = '\0';
			if (IsWatchpoint(symbol)) {
				if (!ClearWatchpoint(symbol)) return FALSE;
				fullRefresh = TRUE;
			}
			else if (!SetWatchpoint(symbol, WP_WORDS)) return FALSE;
			fullRefresh = TRUE;
		}
	}
	RefreshWindow(fullRefresh);
	return TRUE;
}

BOOL	CmdWatchLongs(char *args) {
	char	symbol[128];
	char	*pd;
	BOOL	fullRefresh = FALSE;

	while (*args) {
		while (*args == ' ' || *args == '\t') args++;
		if (*args) {
			pd = &symbol[0];
			while (*args && *args != ' ' && *args != '\t') *pd++ = *args++;
			*pd = '\0';
			if (IsWatchpoint(symbol)) {
				if (!ClearWatchpoint(symbol)) return FALSE;
				fullRefresh = TRUE;
			}
			else if (!SetWatchpoint(symbol, WP_LONGS)) return FALSE;
			fullRefresh = TRUE;
		}
	}
	RefreshWindow(fullRefresh);
	return TRUE;
}

// ************

BOOL	UpHelp(void) {
	if (CurDisplay->ds_WindowTop) {
		CurDisplay->ds_LastRefreshTop = --CurDisplay->ds_WindowTop;
		ScrScrolldown();
		RefreshHelp(1, 0, CurDisplay->ds_WindowTop);
	}
	return TRUE;
}

BOOL	DownHelp(void) {
	CurDisplay->ds_LastRefreshTop = ++CurDisplay->ds_WindowTop;
	ScrScrollup();
	RefreshHelp(1, 0, CurDisplay->ds_WindowTop + ScrMainBodyRange(NULL,NULL) - 1);
}


WORD	RefreshHelp(WORD maxLines, BOOL fullRefresh, LONG index) {
	WORD	count = 0;

	do_scroller();
	if (fullRefresh) {
		SetTitle(NULL,NULL);
#if 0
		ScrInverse();
		ScrPrintf("ON-LINE HELP");
#endif

	    if(CurDisplay->ds_RegFlag) {
		Newline();
		++count;
		if (--maxLines <= 0)return count;
	    }
	}
	ScrPlain();

	while (maxLines > 0 && index < arysize(commandTable) + arysize(HelpTable)) {
	    if (index >= 0) {
		if (index >= arysize(commandTable)) {
		    ScrPuts(HelpTable[index-arysize(commandTable)]);
		} 
		else {
		    ScrPuts(commandTable[index].helpText);
		}
		Newline();
		--maxLines;
		++count;
	    }
	    ++index;
	}
	return count;
}

int HelpSize(void)
{
return arysize(commandTable) + arysize(HelpTable) + 4;
}

BOOL	CmdAlias(char *args)
{
	short i;

	for (i=0; commandTable[i].command; i++) {
		if (!strnicmp(commandTable[i].command, args, commandTable[i].commandLen)) {
			dprefs.alias[i] = 1;
			return(TRUE);
		}
	}
	return(FALSE);
}

BOOL	CmdUnAlias(char *args)
{
	short i;

	for (i=0; commandTable[i].command; i++) {
		if (!strnicmp(commandTable[i].command, args, commandTable[i].commandLen)) {
			dprefs.alias[i] = 0;
			return(TRUE);
		}
	}
	return(FALSE);
}



BOOL CmdJump(char *args)
{
struct Screen *scr;
DBugDisp *disp, *olddisp;

    // jump only works in 2.0
    if(((struct Library *)SysBase)->lib_Version >= 36) {

        if(! *args) {		// we were not given a name to jump to
				// so lets go to next pub screen
	    if(!(scr=CurDisplay->ds_Win->WScreen))return FALSE;
	    if(!NextPubScreen(scr,DefaultPubName))return FALSE;
	}
    	else strcpy(DefaultPubName,args);
	RequestCloseDisplay(CurDisplay,TRUE);
	if(disp = ScrOpen(FALSE, TRUE, DefaultPubName)) {
	    olddisp = CurDisplay;
	    CurDisplay = disp;
	    InitCommand();
	    ScrCursoff();

	    ScrStatus("Jumped");
	    RefreshCommand(-1);
	
    	    ScrCurson();
	    CurDisplay = olddisp;
	    return TRUE;
	}
    return FALSE;
    }
}


BOOL CmdTitle(char *args)
{
char *ptr;
int n;

    if(ptr = strchr(args,' ')) {	// if there is a slot
	*ptr++ = NULL;
	if((n = FindSlot(args)) >= 0) {
    	    if(*ptr) {	// if there is a title given
	    	set_menu_item(n,NM_TITLE,ptr,"","");
		return TRUE;
	    }
	}
    }
    return FALSE;
}

BOOL CmdItem(char *args)
{
return ProcessMenuItem(args,NM_ITEM);
}

BOOL CmdSubItem(char *args)
{
return ProcessMenuItem(args,NM_SUB);
}

BOOL CmdBar(char *args)
{
int n;
 
    if((n = FindSlot(args)) >= 0)set_menu_item(n,NM_BAR,"","","");
    return(TRUE);
}

BOOL CmdEnd(char *args)
{
int n;

    if((n = strtol(args,NULL,10)) >= 0)set_menu_item(n,NM_END,"","","");
    return(TRUE);
}

BOOL CmdMenus(char *args)
{
    return enable_menus();
}

// ************

char	*SkipBlanks(char *s) { while (*s == ' ' || *s == '\t') s++; return s; }

BOOL	DoCommand(char *command) {
	ULONG	val;
	short	i, undef, j = 2;
	BOOL res;
	UBYTE *ptr;

	oldcol = 0xFFFE;	// cancel a possible double click
	if(! *command) {
	    ScrStatus("Ready");	// don't bother to search, they just hit a return
	    return TRUE;
	}
    ptr = command;
    while (j--) {
	for (i=0; commandTable[i].command; i++) {
		if (!strnicmp(commandTable[i].command, ptr, commandTable[i].commandLen)) {
		    if(dprefs.alias[i]) {
		        // try to run the correct Arexx script
		        res = CmdRexx(ptr);
		        ShortStatus(1,ptr);
		        return res;
		    }
		    ShortStatus(0,ptr);
		    if(j) ptr = &command[commandTable[i].commandLen];
		    else ptr = command;
		    return (*commandTable[i].func)(SkipBlanks(ptr));
		}
	}

	// not found in table
	if(j <= 0)break;	// second time through

	// not found in commandTable.  Maybe it's an "implied display" command (expression only)


	val = ParseExp(command, &undef, strlen(command));
	if (!undef) {
	    SetDisplayMode(CurDisplay->ds_PreferedMode, 0);
	    CurDisplay->ds_WindowTop = val;
	    ScrStatus("Result is %0X",val);
	    RefreshWindow(TRUE);
	    return TRUE;
	} 

	// allow the argument to be specified first (for convenience in entering commands)
	if (ptr = strrchr(command,' ')) {
	    *ptr++ = 0; 	// split the string
	}
	else break;
    }
    // hm, see if its an arexx command
    ShortStatus(2,command);	// reset the display
    if(CmdRexx(command)) return TRUE;
	
    // if its here, its not a command
    ScrStatus("*** Invalid Command: '%s'", command);
    return FALSE;
}


//    void *msg;		/*	RexxMsg structure if we need it     */
//    struct MsgPort *port;	/*	MsgPort structure if we need it     */
//    char *arg0;		/*	arg0				    */
//    char **pres;		/*	where to put our result if rc==0    */

long	DoRexxCommand(void *msg, MPORT *port, char *arg0, char **pres) {
	WORD	i;
	long res = 5;

        memset(RexxReplyString,0,MAX_REXX_REPLY);
  	ShortStatus(2,arg0);
        ScrCursoff();

	for (i=0; commandTable[i].command; i++) {
		if (!strnicmp(commandTable[i].command, arg0, commandTable[i].commandLen)) {
			if ((*commandTable[i].func)(SkipBlanks(&arg0[commandTable[i].commandLen])))
			    res = 0;
			else res = 10;
			*pres = RexxReplyString;
			break;
		}
	}
	RefreshCommand(TRUE);
	ScrCurson();
	return res;	// unrecognized command
}



// produce a shortened command status display
void ShortStatus(int mode, char *command)
{
char buf[128], *ptr , *s;

strcpy(buf,command);
if(ptr = strchr(buf,' '))*ptr=NULL;	// shorten to just the command

if(mode == 0)s = "Command";
else if(mode == 1) s = "Alias";
else s = "Rexx";

ScrStatus("%s '%s'",s,buf);
}


#if 0
Prototype void kprintline(char *string);
void kprintline(char *string)
{
kprintf("[");
while(*string && (*string != '\n'))kprintf("%lc",*string++);
kprintf("]\n");
}
#endif

#if 0
Prototype void kprintlen(char *string, int len);
void kprintlen(char *string, int len)
{
kprintf("[");
while(*string && (*string != '\n') && len--)kprintf("%lc",*string++);
kprintf("]\n");
}
#endif





