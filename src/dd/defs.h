/*
 * Defs.h
 *
 * Synopsis:
 *	Master include file for DBug debugger.
 */
#include    <localdefs.h>


// * Macros

#define arysize(ary)		  (sizeof(ary)/sizeof((ary)[0]))

// * Data structures

typedef struct DLIST {
	NODE	node;
	char	name[2];
} DLIST;

// for the sorted symbol list display

typedef struct SYMLIST {
	ULONG *symbolname;
	ULONG address;
} SYMLIST;

// for menus
#define NM_BAR	4

enum DTYPES {
	DTYPE_DOSBASE,
	DTYPE_PROCESS,
	DTYPE_INFO,
	DTYPE_EXECBASE,
	DTYPE_RESOURCES,
	DTYPE_INTRS,
	DTYPE_PORTS,
	DTYPE_TASKS,
	DTYPE_LIBS,
	DTYPE_DEVICES,
	DTYPE_MEMLIST,
	DTYPE_REXXLIST,
	DTYPE_SYMLIST,
	MAX_DTYPE,
};

typedef struct DBugDisp {
	NODE	ds_Node;
	WINDOW	*ds_Win;		//  control window
	STDREQ	ds_CReadReq;		//  console read request
	STDREQ	ds_CWriteReq;		//  console write request

	UBYTE	ds_CReadIP;		//  read request in progress
	UBYTE	ds_CWriteIP;		//  write request in progress (not used)
	UBYTE	ds_OpenError;
	UBYTE	ds_CsiState;		//  CSI escape state machine

	UBYTE	ds_DoneFlag;
	UBYTE	ds_DisplayOffsets;
	UBYTE	ds_PromptLen;
	UBYTE	ds_PromptStart;

	ULONG	ds_Flags;


	UBYTE	ds_CInChar;		//  console buffering
	UBYTE	ds_COutBuf[255];
	WORD	ds_COutIndex;

	UWORD	ds_DisplayMode; 	//  display mode for window
	UWORD	ds_PreferedMode;	//  prefered mode for window

	UWORD	ds_ScrRows;		//  computed window boundries
	UWORD	ds_ScrCols;

	UWORD	ds_ScrTop;
	UWORD	ds_ScrColNo;		//  CHARMAP TRACKING / MOUSE BUT

	UWORD	ds_ScrRowNo;

	UBYTE	ds_RegFlag;		// register display off or on for window
	UBYTE	ds_RegTouched;		// flag for auto registers control

	UBYTE	*ds_ScrAry;

	//  ds_Window* modes are managed mainly by the up/down/refresh
	//  routines associated with a display mode and do not necessarily
	//  contain addresses

	ULONG	ds_WindowTop;		//  top address / index
	ULONG	ds_WindowTopLine;	//  top sub-address / index (mixed mode)
	ULONG	ds_WindowBot;		//  bottom address / index
	ULONG	ds_WindowBotLine;	//  bottom sub-addrss / index (mixed mode)

	ULONG	ds_LastRefreshMode;	//  used to determine non-linear
	ULONG	ds_LastRefreshTop;	//  changes and force a full refresh

	LIST	ds_List;		// list used for special list display mode
	char	ds_windowTitle[128];
} DBugDisp;

#define DF_STACK    0x0001


// there is an array of these for each -d1 debug LINE hunk
// the addresses are relocated at load time
typedef struct SOURCE {
	ULONG	lineNumber;			// lineNumber of source line
	ULONG	address;			// address of source line in memory
} SOURCE;

// For each code/data/bss hunk, there may be a linked list of these DEBUG structures.
// Each DEBUG structure contains source-level debugging information.
typedef struct DEBUG	{
	struct DEBUG	*link;			// ptr to next debug_struct in list
	struct SOURCE	*table; 		// ptr to line#/address pair table
	struct SOURCE	*tableEnd;		// ptr to end of line#/address pair table
	char		sourceName[128];	// source filename
	char		*source;		// ptr to source file in RAM
	ULONG		addrBegin;		// range debug hunk applies to
	ULONG		addrEnd;
} DEBUG;

// At load time, the HUNK_HEADER hunk determines how many code/data/bss hunks exist for the file.
// An array of that many (numHunks) HUNK structures is allocated.  Each HUNK contains a pointer
// to the actual file data and a pointer to the allocated memory to hold the hunk during runtime.
// When the program is reset, the hunk data is copied to the allocated memory and relocated.
typedef struct HUNK {
	ULONG	type;				// type of hunk (code/data + FAST/CHIP)
	ULONG	size;				// size of allocated hunk (may be larger than hunk_Hsize)
	ULONG	*memptr; 			// ptr to actual hunk start
	ULONG	*actual; 			// ptr to actual hunk data
	ULONG	hSize;				// size of hunk data, in longs
	APTR	hunk;				// ptr to hunk in file
	APTR	reloc32;			// ptr to reloc32 hunk in file
	APTR	symbols;			// ptr to symbol hunk in file
	DEBUG	*debug; 			// ptr to debug_struct list for this hunk
} HUNK;

// Breakpoint table format
typedef struct BP {
	UWORD	state;				// see defines below
	UWORD	count;				// # times breakpoint hit before halting...
	UWORD	value;				// save wd value from code at bp_Address
	UWORD	*address;			// address of breakpoint
} BP;

// There are 3 kinds of breakpoints...
#define BP_UNSET	0			// no breakpoint at all
#define BP_SET		1			// sticky breakpoint
#define BP_GROUP	2			// group breakpoint

#define MAXBP		32			// max # of breakpoints

// Watchpoint table format
typedef struct WP {
	ULONG		type;			// see defines below
	char		expression[128];	// expression evaluated at display time
} WP;

#define MAXWP		32			// max # of watchpoints

// There are 4 kinds of watchpoints...
#define WP_UNSET	0
#define WP_BYTES	1
#define WP_WORDS	2
#define WP_LONGS	3

#define MAXCOMMAND	96

// preferences
typedef struct DPREFS	{
	UWORD	top,left,width,height;
	UWORD DefaultMode;
	UBYTE DefaultOffset;
	alias[MAXCOMMAND];
} DPREFS;


#define REXXPORTNAME	"DD"
#define REXXEXT		"DD"
#define MAX_REXX_REPLY	256

// * Global variables

extern IBASE			*IntuitionBase;
extern GBASE			*GfxBase;
extern EBASE			*SysBase;
extern struct DosLibrary	*DOSBase;
extern struct Library		*GadToolsBase;

extern char *RexxHostName;

extern char	*args;
extern ULONG	argSize;
extern TASK	*thisTask;
extern APROCESS *thisProcess;
extern CLI	*thisCli;

extern char	targetName[];
extern ULONG	*exeFile;
extern ULONG	exeSize;
extern ULONG	numHunks, firstHunk, lastHunk;
extern HUNK	*hunkArray;


extern USHORT oldrow, oldcol;

	// value of programState
#define STATE_RUNNING			0
#define STATE_HALTED			1
#define STATE_BUS_ERROR 		2
#define STATE_ADDRESS_ERROR		3
#define STATE_ILLEGAL_INSTRUCTION	4
#define STATE_ZERO_DIVIDE		5
#define STATE_CHK			6
#define STATE_TRAPV			7
#define STATE_PRIVILEDGE_VIOLATION	8
#define STATE_TRACE			9
#define STATE_LINEA			10
#define STATE_LINEF			11
//				...	..
#define STATE_RESET			64
#define STATE_EXITED			65
#define STATE_STEPPED			66
#define STATE_STEPPEDOVER		67
#define STATE_BREAKPOINT		68

extern UBYTE	*programStack;
extern ULONG	programStackSize;

extern ULONG	programState;
extern UWORD	programSR;
extern ULONG	programPC;
extern ULONG	programD0;
extern ULONG	programD1;
extern ULONG	programD2;
extern ULONG	programD3;
extern ULONG	programD4;
extern ULONG	programD5;
extern ULONG	programD6;
extern ULONG	programD7;
extern ULONG	programA0;
extern ULONG	programA1;
extern ULONG	programA2;
extern ULONG	programA3;
extern ULONG	programA4;
extern ULONG	programA5;
extern ULONG	programA6;
extern ULONG	programA7;

extern ULONG	lastState;
extern UWORD	lastSR;
extern ULONG	lastPC;
extern ULONG	lastD0;
extern ULONG	lastD1;
extern ULONG	lastD2;
extern ULONG	lastD3;
extern ULONG	lastD4;
extern ULONG	lastD5;
extern ULONG	lastD6;
extern ULONG	lastD7;
extern ULONG	lastA0;
extern ULONG	lastA1;
extern ULONG	lastA2;
extern ULONG	lastA3;
extern ULONG	lastA4;
extern ULONG	lastA5;
extern ULONG	lastA6;
extern ULONG	lastA7;

enum DISPLAY_MODES {
	DISPLAY_DISM,
	DISPLAY_SOURCE,
	DISPLAY_MIXED,
	DISPLAY_BYTES,
	DISPLAY_WORDS,
	DISPLAY_LONGS,
	DISPLAY_HUNKS,
	DISPLAY_SYMBOL,
	DISPLAY_HELP,
	DISPLAY_BREAK,
	DISPLAY_DOSBASE,
	DISPLAY_PROCESS,
	DISPLAY_INFO,
	DISPLAY_EXECBASE,
	DISPLAY_RESOURCES,
	DISPLAY_INTRS,
	DISPLAY_PORTS,
	DISPLAY_TASKS,
	DISPLAY_LIBS,
	DISPLAY_DEVICES,
	DISPLAY_MEMLIST,
	DISPLAY_REXXLIST,
	DISPLAY_SYMLIST,
	MAX_MODES,
};

#define MIXTYPE_DISM	    1
#define MIXTYPE_SOURCE	    2
#define MIXTYPE_NOSOURCE    3

extern DPREFS	dprefs;

extern char	commandLine[];
extern UWORD	commandCol, commandEnd;

extern WP	wpTable[MAXWP]; 		// 32 user watchpoints

extern BP	bpTable[MAXBP]; 		// 32 user breakpoints
extern BP	bpTemp; 			// temporary breakpoint
extern WORD	topBP;				// top breakpoint displayed from table

extern DBugDisp *CurDisplay;			//

extern unsigned long  ScrollStart, ScrollEnd;
extern int SymbolCount;

// * Assembly Function Prototypes

Prototype __stkargs UBYTE	*MallocPublic(ULONG size);
Prototype __stkargs UBYTE	*MallocFast(ULONG size);
Prototype __stkargs UBYTE	*MallocChip(ULONG size);
Prototype __stkargs UBYTE	*MallocAny(ULONG size, ULONG type);
Prototype __stkargs void	Free(APTR ptr);
Prototype __stkargs void	CleanMem(void);
Prototype __stkargs ULONG	Disassemble(ULONG src, ULONG addr, char *buf);
Prototype __stkargs void	EnterProgram(void);
Prototype __stkargs void	TargetExit(void);

#define ID_SCROLL 0
#define ID_UP	1
#define ID_DOWN	2