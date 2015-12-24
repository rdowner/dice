/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */
#include	"defs.h"
#include	"dbug_protos.h"
#include	<ctype.h>
#include	"dd_rev.h"


// ************************************************************************

Prototype void		InitConsole(void);
Prototype LONG		ScrMainBodyRange(void *ptop, void *pbot);
Prototype LONG		ScrColumns(WORD overhead, WORD itemWidth, WORD limit);
Prototype void		ScrFlush(void);
Prototype void		ScrPutNewline(void);
Prototype void		ScrPutChar(UBYTE c);
Prototype void		ScrWrite(char *s, ULONG len);
Prototype void		ScrPuts(char *s);
Prototype void		ScrPutsCtl(char *s);
Local unsigned int	_swrite(char *buf, size_t n1, size_t n2, const char **sst);
Prototype WORD		ScrPrintf(const char *fmt, ...);
Prototype void		ScrStatus(const char *fmt, ...);
Local void		QueueRead(void);
Local void		HandleConsoleInput(struct DBugDisp *disp, WORD c);
Prototype void		ScrHome(void);
Prototype void		ScrCursoff(void);
Prototype void		ScrCurson(void);
Prototype void		ScrClr(void);
Prototype void		ScrRowCol(UWORD row, UWORD col);
Prototype void		ScrEOL(void);
Prototype void		ScrHighlight(void);
Prototype void		ScrDull(void);
Prototype void		ScrPlain(void);
Prototype void		ScrInverse(void);
Prototype void		ScrItalics(void);
Prototype void		ScrUnderline(void);
Prototype void		ScrScrollup(void);
Prototype void		ScrScrolldown(void);
__autoexit Local void	ScrClose(void);
Prototype DBugDisp 	*ScrOpen(BOOL first, BOOL refresh, char *pubname);
Prototype void		ScrScrollClr(void);
Prototype void		GetWindowSize(void);
Prototype void		EnterDebugger(void);

Prototype void		CloseDisplay(struct DBugDisp *);
Prototype void		RequestCloseDisplay(struct DBugDisp *, int );
Prototype BOOL		CheckCloseDisplay(void);
Prototype void		ProcessDataAtCoord(DBugDisp *disp, WORD x, WORD y);
Prototype void 		SetTitle(char *title, DBugDisp *disp);

Prototype void 		draw_fkey_boxes(void);
Prototype void		drawdoublebox(void);
Local 	  void 		drawbox(int xcol,int yrow, int xlen, int ylen);
Local     void		do_putc(DBugDisp *disp, UBYTE c);

// ************************************************************************

Prototype struct DBugDisp  *CurDisplay;
Prototype LIST	DisplayList;

// ************************************************************************

#define CONIDCMP	NULL
#define IFLAGS 		IDCMP_NEWSIZE|IDCMP_MOUSEBUTTONS|IDCMP_CLOSEWINDOW|IDCMP_REFRESHWINDOW|IDCMP_MENUPICK|IDCMP_ACTIVEWINDOW|IDCMP_INACTIVEWINDOW
#define IFLAGS_FULL     IFLAGS|IDCMP_MOUSEMOVE|IDCMP_GADGETUP|IDCMP_GADGETDOWN|IDCMP_INTUITICKS

extern struct Menu      *DebugMenu;

extern struct Gadget ColorGadget;
extern struct Gadget DownGadget;
extern struct Gadget UpGadget;
extern struct Image ColorPropImage;
extern struct PropInfo ColorPropInfo;

DBugDisp    *CurDisplay;    //	currently active display/window
LIST	    DisplayList;    //	list of open windows
MPORT	    DBugPort;	    //	Master debug port for all console IO
WORD	    CheckClose;
char	    ValidClipChar[256];
static int  count = 0;	
#if 0
static int  ignoreclick = 0;
#endif
static int  PageFlag = 0;
static int  Ticked = 0;

USHORT	oldrow = 0xFFFF;	/* last mouse click position */
USHORT  oldcol = 0xFFFF;	

static ULONG seconds=0;
static ULONG micros=0;

// ************************************************************************

void	InitConsole(void) {

	WORD i;


	NewList(&DisplayList);

	DBugPort.mp_Flags  = PA_SIGNAL;
	DBugPort.mp_SigBit = SIGB_SINGLE;
	DBugPort.mp_SigTask = FindTask(NULL);
	NewList(&DBugPort.mp_MsgList);

	SetSignal(0L,SIGF_SINGLE);

	for (i = '0'; i <= '9'; ++i)ValidClipChar[i] = 1;
	for (i = 'a'; i <= 'z'; ++i)ValidClipChar[i] = 1;
	for (i = 'A'; i <= 'Z'; ++i)ValidClipChar[i] = 1;
	ValidClipChar['+'] = 1;
	ValidClipChar['_'] = 1;
	ValidClipChar['@'] = 1;
	ValidClipChar['$'] = 1;
	ValidClipChar['.'] = 1;

	ValidClipChar['-'] = 1;
}



// ************************************************************************

void	ScrFlush(void) {
	DBugDisp *disp;

	if ((disp = CurDisplay) && disp->ds_COutIndex) {
		disp->ds_CWriteReq.io_Command = CMD_WRITE;
		disp->ds_CWriteReq.io_Data = (APTR)disp->ds_COutBuf;
		disp->ds_CWriteReq.io_Length = disp->ds_COutIndex;
		DoIO((IOREQ *)&disp->ds_CWriteReq);
		disp->ds_COutIndex = 0;
	}
}

void	ScrPutChar(UBYTE c) {
	DBugDisp *disp;

	if (disp = CurDisplay) {
		disp->ds_COutBuf[disp->ds_COutIndex++] = c;
		if (disp->ds_COutIndex == sizeof(disp->ds_COutBuf))ScrFlush();
		if (c == '\t') {
		    int i = 8 - (disp->ds_ScrColNo & 7);
		    while(i--)do_putc(disp,' ');
		}
		else do_putc(disp,c);
	}
}

void do_putc(DBugDisp *disp,UBYTE c)
{
    if (disp->ds_ScrColNo < disp->ds_ScrCols && disp->ds_ScrRowNo < disp->ds_ScrRows && disp->ds_ScrAry)
        disp->ds_ScrAry[disp->ds_ScrColNo++ + disp->ds_ScrCols * disp->ds_ScrRowNo] = c;
}


void	ScrPutCharCtl(UBYTE c) {
	DBugDisp *disp;

	if (disp = CurDisplay) {
		disp->ds_COutBuf[disp->ds_COutIndex++] = c;
		if (disp->ds_COutIndex == sizeof(disp->ds_COutBuf))
			ScrFlush();
	}
}


void	ScrWrite(char *s, ULONG len) {

	while (len > 0) {
		ScrPutChar(*s++);
		--len;
	}
}

void	ScrPuts(char *s) {

	while (*s)ScrPutChar(*s++);
}

void	ScrPutsCtl(char *s) {

	while (*s)ScrPutCharCtl(*s++);

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

WORD	ScrPrintf(const char *fmt, ...) {
	char		buf[128];
	char		*ptr = &buf[0];
	va_list 	va;
	WORD		n;

	va_start(va, fmt);
	n = (WORD)_pfmt(fmt, va, _swrite, &ptr);
	*ptr = 0;
	ScrPuts(buf);

	va_end(va);
	n = strlen(buf);
	return n;
}

void	ScrStatus(const char *fmt, ...) {
	static char	buf[128];
	char		*ptr = &buf[0];
	va_list 	va;

	ScrPlain();
	ScrDull();

	va_start(va, fmt);
	_pfmt(fmt, va, _swrite, &ptr);
	*ptr = 0;

	ScrRowCol(CurDisplay->ds_ScrRows-2, 1);

	ScrPuts(" < Status: ");

	ScrInverse();
	ScrPuts(buf);
	ScrPlain(); 
	ScrDull();

	ScrPuts(" > ");
//	ScrPlain();

	CurDisplay->ds_PromptStart = strlen(buf)+14;
	RefreshPrompt(TRUE);
	strcpy(RexxReplyString,buf);	// for Arexx users
	va_end(va);
}

// ************************************************************************

Local void	QueueRead(void) {
	DBugDisp *disp;

	if (disp = CurDisplay) {
		if (disp->ds_CReadIP == 0) {
			disp->ds_CReadReq.io_Command = CMD_READ;
			disp->ds_CReadReq.io_Data = (APTR)&disp->ds_CInChar;
			disp->ds_CReadReq.io_Length = 1;
			SendIO((IOREQ *)&disp->ds_CReadReq);
			disp->ds_CReadIP = 1;
		}
	}
}


// ************************************************************************

void	ScrPutNewline(void) {
	DBugDisp *disp;

	ScrPutCharCtl('\n');

	if (disp = CurDisplay) {
		disp->ds_ScrColNo = 0;
		++disp->ds_ScrRowNo;
	}
}

void	ScrHome(void) {
	DBugDisp *disp;

	ScrPutsCtl("\x9b" "0;0\x48");
	if (disp = CurDisplay) {
		disp->ds_ScrColNo = 0;
		disp->ds_ScrRowNo = 0;
	}
}

void	ScrCursoff(void) {
	ScrPutsCtl("\x9b" "0 \x70");
}

void	ScrCurson(void) {
	ScrPutsCtl("\x9b" " \x70");
	ScrFlush();
}

void	ScrClr(void) {
	DBugDisp *disp;

	ScrPutCharCtl(12);
	if (disp = CurDisplay) {
		disp->ds_ScrColNo = 0;
		disp->ds_ScrRowNo = 0;
		if (disp->ds_ScrAry)
			clrmem(disp->ds_ScrAry, disp->ds_ScrRows * disp->ds_ScrCols);
	}
}

void	ScrRowCol(UWORD row, UWORD col) {
	DBugDisp *disp;
	char buf[32];

	sprintf(buf, "\x9b" "%d;%d\x48", row, col);
	ScrPutsCtl(buf);

	if (disp = CurDisplay) {
		disp->ds_ScrColNo = col - 1;
		disp->ds_ScrRowNo = row - 1;
	}
}


void	ScrEOL(void) {
	DBugDisp *disp;

	ScrPutsCtl("\x9b" "\x4b");
	if ((disp = CurDisplay) && disp->ds_ScrAry) {
		if (disp->ds_ScrRowNo < disp->ds_ScrRows && disp->ds_ScrColNo < disp->ds_ScrCols)
			clrmem(disp->ds_ScrAry + disp->ds_ScrRowNo * disp->ds_ScrCols + disp->ds_ScrColNo, disp->ds_ScrCols - disp->ds_ScrColNo);
	}
}

void	ScrHighlight(void) {
	if (IntuitionBase->ActiveScreen->BitMap.Depth == 1)
		ScrPutsCtl("\x9b" "7\x6d");
	else ScrPutsCtl("\x9b" "32\x6d");
}

void	ScrDull(void) {
	if (IntuitionBase->ActiveScreen->BitMap.Depth == 1)
		ScrPutsCtl("\x9b" "3\x6d");
	else	ScrPutsCtl("\x9b" "33\x6d");
}

void	ScrPlain(void) {
	ScrPutsCtl("\x9b" "0\x6d");
}

void	ScrInverse(void) {
	ScrPutsCtl("\x9b" "7\x6d");
}

void	ScrItalics(void) {
	ScrPutsCtl("\x9b" "3\x6d");
}

void	ScrUnderline(void) {
	ScrPutsCtl("\x9b" "4\x6d");
}

// scroll only scrollable window part up one line
// i.e. like a normal line feed

void	ScrScrollup(void) {
	DBugDisp *disp;
	WINDOW	 *win;

	if ((disp = CurDisplay) && (win = disp->ds_Win)) {
		RPORT	 *rp = win->RPort;
		UWORD	top = win->BorderTop + disp->ds_ScrTop * rp->TxHeight;
		UWORD	bottom = top + ScrMainBodyRange(NULL, NULL) * rp->TxHeight;
		ULONG	topln;
		ULONG	botln;

		ScrollRaster(rp, 0, rp->TxHeight, win->BorderLeft, top, CurDisplay->ds_ScrCols * rp->TxWidth + 3, bottom - 1);
		ScrMainBodyRange(&topln, &botln);
		--topln;
		if (disp->ds_ScrAry && botln < disp->ds_ScrRows) {
			//kprintf("top = %d bot = %d\n", topln, botln);
			movmem(disp->ds_ScrAry + (UWORD)(topln + 1) * disp->ds_ScrCols,
				disp->ds_ScrAry + (UWORD)topln * disp->ds_ScrCols,
				(UWORD)(botln - topln - 1) * disp->ds_ScrCols
			);
			clrmem(disp->ds_ScrAry + (UWORD)(botln - 1) * disp->ds_ScrCols, disp->ds_ScrCols);
		}
		ScrRowCol(botln, 1);
	}
}

// scroll only scrollable window part down one line

void	ScrScrolldown(void) {
	DBugDisp *disp;
	WINDOW	 *win;

	if ((disp = CurDisplay) && (win = disp->ds_Win)) {
		RPORT	 *rp = win->RPort;
		UWORD	top = win->BorderTop + disp->ds_ScrTop * rp->TxHeight;
		UWORD	bottom = top + ScrMainBodyRange(NULL,NULL) * rp->TxHeight;
		ULONG	topln;
		ULONG	botln;

		ScrollRaster(rp, 0, -rp->TxHeight, win->BorderLeft, top, CurDisplay->ds_ScrCols * rp->TxWidth + 3, bottom - 1);
		ScrMainBodyRange(&topln, &botln);
		ScrRowCol(topln, 1);
		--topln;
		if (disp->ds_ScrAry && botln < disp->ds_ScrRows) {
			movmem(disp->ds_ScrAry + (UWORD)topln * disp->ds_ScrCols,
				disp->ds_ScrAry + (UWORD)(topln + 1) * disp->ds_ScrCols,
				(UWORD)(botln - topln - 1) * disp->ds_ScrCols
			);
			clrmem(disp->ds_ScrAry + (UWORD)topln * disp->ds_ScrCols, disp->ds_ScrCols);
		}

	}
}

// Clear the Scrollable area of the screen

void	ScrScrollClr(void) {
	DBugDisp *disp;
	WINDOW	 *win;

	if ((disp = CurDisplay) && (win = disp->ds_Win)) {
		RPORT	 *rp = win->RPort;
		UWORD	top = win->BorderTop + disp->ds_ScrTop * rp->TxHeight;
		UWORD	bottom = top + ScrMainBodyRange(NULL, NULL) * rp->TxHeight;
		ULONG	topln;
		ULONG	botln;

		SetAPen(rp,0);
		RectFill(rp, win->BorderLeft, top, CurDisplay->ds_ScrCols * rp->TxWidth + 3, bottom - 1);
		ScrMainBodyRange(&topln, &botln);
		if (disp->ds_ScrAry && (topln < botln) && (topln < disp->ds_ScrRows)) {
			clrmem(disp->ds_ScrAry + (UWORD)(topln) * disp->ds_ScrCols, 
				(botln - topln) * disp->ds_ScrCols);
		}
		ScrRowCol(botln, 1);
	}
}

//  Return the main body range in console coordinates (row, col start
//  at 1)

LONG	ScrMainBodyRange(void *ptop, void *pbot) {

	if (ptop) {
	    //  top line, scroll area
	    *(long *)ptop = CurDisplay->ds_ScrTop + 1;
	}
	if (pbot)
	    *(long *)pbot = CurDisplay->ds_ScrRows - 4;    //  bottom line, scroll area
	return (CurDisplay->ds_ScrRows - CurDisplay->ds_ScrTop - 4);	  //  # lines in scroll area
}

LONG	ScrColumns(WORD overhead, WORD itemWidth, WORD limit) {
	LONG cols = (CurDisplay->ds_ScrCols - overhead) / itemWidth;
	LONG n;

	if (cols < 0)
	    cols = 0;
	if (cols > limit)		/*  no greater then 16	*/
	    cols = limit;

	for (n = 1; n <= cols; n <<= 1)  /*  power of 2 boundry  */
	    ;
	return (n >> 1);
}

// ************************************************************************

__autoexit Local void	ScrClose(void) {
	DBugDisp *disp;

	while ((disp = (DBugDisp *)DisplayList.lh_Head) != (DBugDisp *)&DisplayList.lh_Tail) {
	    if(!disp) {
		break;	
	    }
	    CloseDisplay(disp);
	}
       free_menus();
}

BOOL	CheckCloseDisplay(void) {
	DBugDisp	*disp;
	BOOL		retval = TRUE;
	BOOL		flag = TRUE;

    CheckClose = FALSE;


    while(flag) {
	flag = FALSE;
	for (disp = (DBugDisp *)DisplayList.lh_Head; disp->ds_Node.ln_Succ; disp = (DBugDisp *)disp->ds_Node.ln_Succ) {
	    if (!disp->ds_DoneFlag) {
		// can't exit, we still have a window open
		retval = FALSE;
	    }
	    else {
		CloseDisplay(disp);
		flag = TRUE;
		break;	// disp is illegal at the moment, start scan again
	    }
	}
    }
    return retval;
}


void	CloseDisplay(DBugDisp *disp) {

	if(!disp)return;
	Remove(&disp->ds_Node);

	if (disp->ds_CReadIP) {
		AbortIO((IOREQ *)&disp->ds_CReadReq);
		WaitIO((IOREQ *)&disp->ds_CReadReq);
		disp->ds_CReadIP = 0;
	}
	if (disp->ds_CWriteIP) {
		AbortIO((IOREQ *)&disp->ds_CWriteReq);
		WaitIO((IOREQ *)&disp->ds_CWriteReq);
		disp->ds_CWriteIP = 0;
	}
	if (disp->ds_CReadReq.io_Device) {
		CloseDevice((IOREQ *)&disp->ds_CReadReq);
		disp->ds_CReadReq.io_Device = NULL;
	}
	if (disp->ds_ScrAry) {
		Free(disp->ds_ScrAry);
		disp->ds_ScrAry = NULL;
	}
	if (disp->ds_Win) {
		//  XXX what about IDCMP that is hanging on to our port?
		//  when intuition reclaims the messages the port list
		//  will probably become invalid

		RequestCloseDisplay(disp,TRUE);
		ClearMenuStrip(disp->ds_Win);
		CloseWindow(disp->ds_Win);
		disp->ds_Win = NULL;
	}

	if(CurDisplay == disp) {
		CurDisplay = NULL;
	}

	FreeMem(disp, sizeof(DBugDisp));
}

char	version[] = VERSTAG " Copyright 1992, O.I.C., Redistribution and use under DICE-LICENSE.TXT\n";


static struct ExtNewWindow nw = {
	0,0,		/* window XY origin relative to TopLeft of screen */
	640,200,	/* window width and height */
	0,1,		/* detail and block pens   */
	0,		/* 0 IDCMP  */
	WINDOWSIZING|WINDOWDRAG|WINDOWDEPTH|WINDOWCLOSE|SMART_REFRESH|ACTIVATE,
	NULL,		/* first gadget in gadget list */
	NULL,		/* custom CHECKMARK imagery */
	NULL,		/* window title */
	NULL,		/* custom screen pointer */
	NULL,		/* custom bitmap */
	64,64,		/* minimum width and height */
	-1,-1,		/* maximum width and height */
	WBENCHSCREEN,	/* destination screen type */
	NULL
};

DBugDisp *ScrOpen(BOOL first, BOOL refresh, char *pubname) {
	DBugDisp *disp;
	DBugDisp *old = CurDisplay;
	BOOL ok = FALSE;

	if (disp = AllocMem(sizeof(DBugDisp), MEMF_PUBLIC|MEMF_CLEAR)) {
		NewList((LIST *)&disp->ds_List);
		nw.LeftEdge = dprefs.left;
		nw.TopEdge = dprefs.top;
		nw.Width = dprefs.width;
		nw.Height = dprefs.height;

		if (old) {
			disp->ds_DisplayOffsets = old->ds_DisplayOffsets;
			disp->ds_DisplayMode = old->ds_DisplayMode;
			disp->ds_PreferedMode = old->ds_PreferedMode;
			disp->ds_WindowTop    = old->ds_WindowTop;
			disp->ds_WindowTopLine= old->ds_WindowTopLine;
			disp->ds_WindowBot    = old->ds_WindowBot;
			disp->ds_WindowBotLine= old->ds_WindowBotLine;
		} 
		else {
			// first window, depend on preferences
			disp->ds_DisplayMode = dprefs.DefaultMode;
			disp->ds_PreferedMode = dprefs.DefaultMode;
			disp->ds_DisplayOffsets = dprefs.DefaultOffset;
			disp->ds_LastRefreshMode = DISPLAY_BYTES;
		}

		if(pubname && *pubname && (((struct Library *)SysBase)->lib_Version >= 36)) {
		    struct Screen *pub_screen;
		    struct TagItem wtags[3];

			nw.Flags |= WFLG_NW_EXTENDED;
			nw.Extension = wtags;
			wtags[0].ti_Tag = WA_PubScreenName;
			wtags[0].ti_Data = (ULONG)pubname;
			wtags[1].ti_Tag = WA_PubScreenFallBack;
			wtags[1].ti_Data = TRUE;
			wtags[2].ti_Tag = TAG_END;
			wtags[2].ti_Data = 0;

    			if(pub_screen = LockPubScreen(pubname)) {
			    disp->ds_Win = OpenWindow((struct NewWindow *)&nw);
			    UnlockPubScreen(pubname,pub_screen);
			}
		}
		else {
		    memset(DefaultPubName,0,128);	// clear the name
		    nw.Flags &= ~WFLG_NW_EXTENDED;	// clear the ext bit
		    disp->ds_Win = OpenWindow((struct NewWindow *)&nw);
		}

		if(disp->ds_Win) {
			PageFlag = 0;
			SetTitle(NULL,disp);	// set the window title
			disp->ds_Win->UserPort = &DBugPort;

			ModifyIDCMP(disp->ds_Win,IFLAGS);

			disp->ds_CReadReq.io_Message.mn_ReplyPort = &DBugPort;
			disp->ds_CReadReq.io_Data = (APTR)disp->ds_Win;
			disp->ds_CReadReq.io_Length = sizeof(WINDOW);
			if (OpenDevice("console.device", 0, (IOREQ *)&disp->ds_CReadReq, 0) == 0) {
				disp->ds_CWriteReq = disp->ds_CReadReq;

				CurDisplay = disp;
				QueueRead();
				ScrPutsCtl("\x9b" "\x3f\x37\x6c");
				ScrFlush();
				GetWindowSize();
				ok = TRUE;

				setscrollbar(0);	// add the scroll bar
				ModifyIDCMP(disp->ds_Win,IFLAGS_FULL);

				// start the menu subsystem
				init_default_menus();
				enable_menus();

				if (refresh)RefreshWindow(-1);

				//  can't set to new display here except
				//  for initial because we would be
				//  pulling it out from under the
				//  command interpreter

				if (first == 0)	CurDisplay = old;
			}
		}
	}
	if (disp) {
	    AddTail(&DisplayList, &disp->ds_Node);
	}
	if (ok == FALSE) {
	    if (disp)CloseDisplay(disp);
	    if (first) {
		exit(25);
	    }
	}
    return disp;
}

// ************************************************************************

void	GetWindowSize(void) {
	DBugDisp *disp;
	WINDOW	 *win;

	if ((disp = CurDisplay) && (win = disp->ds_Win)) {
		UWORD rows = (win->Height - win->BorderTop - win->BorderBottom) / win->RPort->TxHeight;
		UWORD cols = (win->Width - win->BorderLeft - win->BorderRight) / win->RPort->TxWidth;
		if (rows != disp->ds_ScrRows || cols != disp->ds_ScrCols) {
			disp->ds_ScrRows = rows;
			disp->ds_ScrCols = cols;
			if (disp->ds_ScrAry)
				Free(disp->ds_ScrAry);
			if (disp->ds_ScrAry = MallocPublic(rows * cols))
				clrmem(disp->ds_ScrAry, rows * cols);
		}
	}
}

// ************************************************************************

#if 0
 void DumpCharmap(void) {
	WORD	row;
	WORD	col;
	DBugDisp *disp = CurDisplay;

	fhprintf(Output(), "cols = %d rows = %d\n", disp->ds_ScrCols, disp->ds_ScrRows);
	for (row = 0; row < disp->ds_ScrRows; ++row) {
		for (col = 0; col < disp->ds_ScrCols; ++col) {
			BYTE c = disp->ds_ScrAry[row * disp->ds_ScrCols + col];
			if (c)
				fhprintf(Output(), "%c", c);
			else
				fhprintf(Output(), ".");
		}
		fhprintf(Output(), "\n");
	}
 }

#endif

__geta4 void	EnterDebugger(void) {
	struct	MenuItem *item;

    if (CurDisplay && CurDisplay->ds_DisplayMode > DISPLAY_MIXED)
	SetDisplayMode(CurDisplay->ds_PreferedMode, 0);

    RefreshWindow(-1);
    ScrStatus("Ready");
    RefreshCommand(1);
    ScrCurson();

    SetSignal(0L,SIGF_SINGLE);	// always clear the bit
    while (DisplayList.lh_Head != (NODE *)&DisplayList.lh_Tail) {
	ULONG	mask;
	MSG	*msg;

	if (CheckClose && CheckCloseDisplay()) {
	    break;
	}


//	mask = Wait( (1<<DBugPort.mp_SigBit) | (1<<RexxPort.mp_SigBit) );
	mask = Wait( (1<<DBugPort.mp_SigBit) | (1<<RexxSigBit) );
	if (mask & (1<<DBugPort.mp_SigBit)) {
 	    while (msg = GetMsg(&DBugPort)) {
		DBugDisp	*disp;

		//  If console IO message, handle console IO.
		//

		for (disp = (DBugDisp *)DisplayList.lh_Head; disp->ds_Node.ln_Succ; disp = (DBugDisp *)disp->ds_Node.ln_Succ) {
		    if (msg == (MSG *)&disp->ds_CReadReq) {
			disp->ds_CReadIP = 0;
			CurDisplay = disp;
			HandleConsoleInput(disp, disp->ds_CInChar);
			CurDisplay = disp;
			QueueRead();
			msg = NULL;
			break;
		    }
		}
		if (msg == NULL)continue;

		//  Not console IO, must be intuition
		//

		CurDisplay = NULL;
		for (disp = (DBugDisp *)DisplayList.lh_Head; disp->ds_Node.ln_Succ; disp = (DBugDisp *)disp->ds_Node.ln_Succ) {
		    if (((IMSG *)msg)->IDCMPWindow == disp->ds_Win) {
			CurDisplay = disp;
			break;
		    }
		}
		if (CurDisplay) {
		    switch (((IMSG*)msg)->Class) {
			case IDCMP_CLOSEWINDOW:
			RequestCloseDisplay(CurDisplay,FALSE);
			break;

		        case IDCMP_NEWSIZE:
			    GetWindowSize();
			    {
			        WINDOW	*window = CurDisplay->ds_Win;
			        dprefs.left = window->LeftEdge; dprefs.top = window->TopEdge;
			        dprefs.width = window->Width; dprefs.height = window->Height;
			    }
			    ScrClr();
			    setscrollbar(1);	// resize the scroll bar
			    RefreshWindow(-1);
			    ScrStatus("Window Resized");
			    RefreshCommand(1);
			    ScrFlush();
			    break;

			case IDCMP_MOUSEBUTTONS:
#if 0
//	multiple windows make ignoring the first click a pain
			    if(ignoreclick) {
				ignoreclick = 0;
				break;
			    }
#endif
			{

//			        ULONG cseconds,cmicros;
				USHORT newrow, newcol, nc, nr;
			    switch(((IMSG *)msg)->Code) {
				case SELECTUP:
				break;
				case SELECTDOWN:
//				    CurrentTime(&cseconds,&cmicros); /* what time is it ? */
				    newcol = (((IMSG *)msg)->MouseX);
				    nc = newcol >> 3;
				    newrow = (((IMSG *)msg)->MouseY);
				    nr = newrow >> 3;
					
				    // check for double click to set breakpoint
				    if((oldcol == nc) && (oldrow == nr)) {
					 // && DoubleClick(seconds,micros,cseconds,cmicros))
					 // people don't like doubleclick timing
					char *string="bp\r";
				    	while (*string) {
				            HandleConsoleInput(disp,*string++);
					}
					oldcol = oldrow = 0xFFFF;
				    }
				    else {
				    	oldcol = nc;
				    	oldrow =  nr;

				        //  Process click on data.  Console buffer is empty prefix with
				        //  current command first

				        ProcessDataAtCoord(disp, newcol, newrow);
				    }
				    // save times for next time
//				    seconds=cseconds;
//				    micros=cmicros;
				break;
				default:;
			    }
			    break;
			}

			case IDCMP_REFRESHWINDOW:
			    BeginRefresh(disp->ds_Win);
			    EndRefresh(disp->ds_Win, 1);
			    break;

			case IDCMP_INACTIVEWINDOW:
			    InActivateArrows(disp->ds_Win);
			    break;

			case IDCMP_ACTIVEWINDOW: {
#if 0
			    ignoreclick = 1;
#endif
			    PageFlag = 0;
			    ActivateArrows(disp->ds_Win);
			    RefreshCommand(1);
			    break;
			}
			
			case IDCMP_INTUITICKS:
			    if(PageFlag) {	// if gadget is held down
				Ticked = 1;
				ScrCursoff();
				if( PageFlag == ID_UP)DoCommand("UP");
				else DoCommand("DOWN");
				RefreshPrompt(TRUE);
				RefreshCommand(1);
				ScrCurson();
			    }
			    break;

			case IDCMP_GADGETUP: {
				unsigned int id;
				APTR address = ((IMSG *)msg)->IAddress;
				if( (id = ((struct Gadget *)address)->GadgetID) != ID_SCROLL) {
				    PageFlag = 0;
				    if(Ticked) {
					Ticked = 0;
					break;
				    }
				    ScrCursoff();
				    if( id == ID_UP)DoCommand("UP");
				    else DoCommand("DOWN");
				    RefreshPrompt(TRUE);
				    RefreshCommand(1);
				    ScrCurson();
				}
				break;
			}

			case IDCMP_MOUSEMOVE:
			    if(++count&1) {
				break; 	// ignore every other mouse
			    }		// else drop through to the scroll bar

			case IDCMP_GADGETDOWN: {
			    ULONG current, last;
				unsigned int id;
				APTR address = ((IMSG *)msg)->IAddress;

			    if( (id = ((struct Gadget *)address)->GadgetID) != ID_SCROLL) {
				PageFlag = (id == ID_UP) ? ID_UP : ID_DOWN;
				Ticked = 0;
				break;
			    }
			    switch (CurDisplay->ds_DisplayMode) {
				case DISPLAY_DISM:
				case DISPLAY_SOURCE:
				case DISPLAY_MIXED:
				case DISPLAY_BYTES:
				case DISPLAY_WORDS:
				case DISPLAY_LONGS:
				    last = (ScrollEnd - ScrollStart) >> 5;
				    break;

				case DISPLAY_HELP:
				    last = HelpSize();
				    break;

				case DISPLAY_SYMBOL:
				    last = SymbolCount;
				    break;

				case DISPLAY_HUNKS:
		    		    last = numHunks;
		    		    break;

				case DISPLAY_BREAK:
				    last = MAXBP+5;
				    break;

				    default:
		    		    last = SizeDLIST(&CurDisplay->ds_List);
			    }
			    current = FindScrollerTop(last, CurDisplay->ds_ScrRows, ColorPropInfo.VertPot);
			    // now go do it
			    switch(CurDisplay->ds_DisplayMode) {
				case DISPLAY_DISM:
				case DISPLAY_SOURCE:
				case DISPLAY_MIXED:
				case DISPLAY_BYTES:
				case DISPLAY_WORDS:
				case DISPLAY_LONGS:
				    if((ScrollStart + (current << 5)) != CurDisplay->ds_WindowTop) {
					CurDisplay->ds_WindowTop = ScrollStart + (current << 5);
					CurDisplay->ds_WindowTopLine = 0;
					RefreshWindow(1);
				    }
				    break;
				case DISPLAY_BREAK:
				    if(topBP != current) {
					topBP = current;
					RefreshWindow(1);
				    }
				    break;
				default:;
				    if(CurDisplay->ds_WindowTop != current) {
					CurDisplay->ds_WindowTop = current;
					CurDisplay->ds_WindowTopLine = 0;
					RefreshWindow(1);
				    }
			    }
			    break;
			}


			case IDCMP_MENUPICK: {
			    USHORT code = ((IMSG *)msg)->Code;
			    char *string;

			    if (!code || (code == MENUNULL))break;
				while (code && (code != MENUNULL)) { /* handle multiple selection	*/
				    item = ItemAddress(DebugMenu,(LONG) code);
				    string = MENU_USERDATA(item);
				    while (*string) {
				        HandleConsoleInput(disp,*string++);
				    }
				    HandleConsoleInput(disp,'\r');
				    code = item->NextSelect;
				}
			    break;
			}

			default:
//			    printf("IDCMP Class = %x Code = %x\n", ((IMSG *)msg)->Class, ((IMSG *)msg)->Code);
			break;
		    }
		}
		if(msg)ReplyMsg(msg);
	    }
	}
//	if (mask & (1<<RexxPort.mp_SigBit)) {
	if (mask & (1<<RexxSigBit)) {
	    ProcessRexxCommands(NULL);
	}
    }
}

void	HandleConsoleInput(DBugDisp *disp, WORD c) {
	switch(c) {
	case -1:
		break;
	case 3:
		RequestCloseDisplay(disp,TRUE);
		break;
	case 8:
		if (commandCol) {
			UWORD	col;

			if (commandEnd == commandCol) commandEnd--;
			commandCol--;
			for (col=commandCol; col<commandEnd; col++) {
				commandLine[col] = commandLine[col+1];
			}
			ScrCursoff(); 
			RefreshCommand(1); 
			ScrCurson();
		}
		break;
	case 13:
		commandLine[commandEnd] = '\0';
		ScrCursoff();
		commandEnd = 0; 	// OPEN command refresh case
		DoCommand(commandLine);
		ScrCurson();
		InitCommand();		// necessary to clear command line
		ScrCursoff();
		RefreshCommand(1);
		ScrCurson();
		break;
	case 18:
		ScrCursoff();
		RefreshWindow(FALSE);
		ScrCurson();
	case 24:
		InitCommand();
		ScrCursoff();
		RefreshCommand(1);
		ScrCurson();
		break;
	case 0x9b:
		disp->ds_CsiState = 1;
		break;
	default:
		switch (disp->ds_CsiState) {
		case 0:
			if (c >= ' ') {
				commandLine[commandCol] = c;
				if (commandCol == commandEnd) commandEnd++;
				commandCol++;
				ScrCursoff();
				RefreshCommand(1);
				ScrCurson();
			}
			break;
		case 1:
			ScrCursoff();

			switch (c) {
			case ' ': disp->ds_CsiState = 3; break;
			case '0': disp->ds_CsiState = 2; FunctionKey(0); break;
			case '1': disp->ds_CsiState = 2; FunctionKey(1); break;
			case '2': disp->ds_CsiState = 2; FunctionKey(2); break;
			case '3': disp->ds_CsiState = 2; FunctionKey(3); break;
			case '4': disp->ds_CsiState = 2; FunctionKey(4); break;
			case '5': disp->ds_CsiState = 2; FunctionKey(5); break;
			case '6': disp->ds_CsiState = 2; FunctionKey(6); break;
			case '7': disp->ds_CsiState = 2; FunctionKey(7); break;
			case '8': disp->ds_CsiState = 2; FunctionKey(8); break;
			case '9': disp->ds_CsiState = 2; FunctionKey(9); break;
			case '?': disp->ds_CsiState = 2; HelpKey(); break;
			case 'A': DoCommand("up"); break;
			case 'B': DoCommand("down"); break;
			case 'C': DoCommand("right"); break;
			case 'D': DoCommand("left"); break;
			case 'T': DoCommand("pageup"); break;
			case 'S': DoCommand("pagedown"); break;
			}

			if (disp->ds_CsiState == 1) disp->ds_CsiState = 0;
			RefreshPrompt(TRUE);
			RefreshCommand(1);
			ScrCurson();
			break;
		case 2:
			disp->ds_CsiState = 0;
			break;	// swallow ~
		case 3:
			ScrCursoff();
			switch (c) {
				case 'A': DoCommand("shift-left"); break;
				case '@': DoCommand("shift-right"); break;
			}
			disp->ds_CsiState = 0;
			ScrCurson();
			break;
		}
	}
}

//  We want to close a display, but since we are sharing an IDCMP port
//  we have to carefully detach it, process all remaining IDCMP messages,
//  then finally close the window.
//
//  Sequence:
//	(1) modify IDCMP to something that cannot occur to prevent further
//	    incoming messages
//	(2) detach our custom UserPort (DBugPort)
//	(3) set UserPort field to NULL
//	(4) modify IDCMP to 0

void	RequestCloseDisplay(DBugDisp *disp, int force) {
	struct DBugDisp *ndisp;
	int count = 0;
	struct IntuiText body = {0,1,JAM1,0,5,NULL,"Last DD window.  Really quit ?",NULL};
	struct IntuiText ok = {0,1,JAM1,0,0,NULL,"OK",NULL};
	struct IntuiText cancel = {0,1,JAM1,0,0,NULL,"CANCEL",NULL};

	if (disp && disp->ds_DoneFlag == FALSE) {
	    disp->ds_DoneFlag = TRUE;

	    // ask if really wants to quit if this is last window
	    if(!force) {
    		// scan through displays to see if this is the last window.
    		// if so, bring up a requester
    		for (ndisp = (DBugDisp *)DisplayList.lh_Head; ndisp->ds_Node.ln_Succ; ndisp = (DBugDisp *)ndisp->ds_Node.ln_Succ) {
	    	    if (!ndisp->ds_DoneFlag)count++;
        	}
        	if(count == 0) {	// all windows want to close, give warning
	    	    if(!AutoRequest(disp->ds_Win,&body,&ok,&cancel,NULL,NULL,300,80)) {
			disp->ds_DoneFlag = FALSE;	// cancel the quit
			return;
		    }
        	}
            }

		CheckClose = TRUE;
		if (disp->ds_Win) {
			Forbid();
			ModifyIDCMP(disp->ds_Win, IDCMP_REQCLEAR);
			disp->ds_Win->UserPort = NULL;
			ModifyIDCMP(disp->ds_Win, 0);
			Permit();
		}
	}
}

void	ProcessDataAtCoord(DBugDisp *disp, WORD x, WORD y) {
	WINDOW *win = disp->ds_Win;
	WORD row = (y - win->BorderTop) / win->RPort->TxHeight;
	WORD col = (x - win->BorderLeft) / win->RPort->TxWidth;
	UBYTE *ptr = &disp->ds_ScrAry[row * disp->ds_ScrCols + col];
	WORD len;
	WORD i;

	if (ValidClipChar[*ptr] == 0) {
		switch (*ptr) {
		case '*':
		case '-':
		case '/':
			HandleConsoleInput(disp, *ptr);
			break;
		case '<':
			HandleConsoleInput(disp, 'x'&0x1F);
			break;
		case '>':
			HandleConsoleInput(disp, 13);
			break;
		case ' ':
		case 0:
			//  mouse to the right of the command line
			//
			//  note: comparing row/col in 0+ format to
			//  screen rows/cols in 1+ format.

			if (row == disp->ds_ScrRows - 3 && col > disp->ds_PromptLen + 2) {
				HandleConsoleInput(disp, 13);
			}
			break;
		}
		return;
	}
	while (col >= 0 && ValidClipChar[*ptr]) {
		--col;
		--ptr;
	}
	++col;
	++ptr;
	for (len = 0; ValidClipChar[ptr[len]] && col + len < disp->ds_ScrCols; ++len)
		;
	{
		char *jp;
		int nflag = 0;

		if(*ptr == '-')nflag = 1;
		if (ptr[nflag] == '$' || (len == 8 && (strtol(ptr, &jp, 16), (jp - ptr == 8)))) {
		        if(nflag) {
			    HandleConsoleInput(disp, '-');
			    ++ptr;
			    --len;
			}

			HandleConsoleInput(disp, '0');
			HandleConsoleInput(disp, 'x');
			++ptr;
			--len;
		}
	}
	if (len == 2 && ptr[0] == '+' && ptr[1] == '+')--len;

	// handle those silly function keys

	if (len == 1 && ptr[0] >= '1' && ptr[0] <= '9' && ptr[1] == ':') {
		FunctionKey(ptr[0] - '1');
		return;
	}
	// and F10
	if(len == 2 && ptr[0] == '1' && ptr[1] == '0' && ptr[2] == ':') {
		FunctionKey(9);
		return;
	}

	for (i = 0; i < len; ++i)HandleConsoleInput(disp, ptr[i]);

	if((len == 2) && (ptr[2] == ':')) {	// treat it as a register clip
	    	HandleConsoleInput(disp, ':');
	}

	for(i=0; i<= 9; i++) {
	    if(!strncmp(ptr,fkeys[i],len)) {
		HandleConsoleInput(disp, '\r');
		return;
	    }
	}
	HandleConsoleInput(disp, ' ');
}

char	*ModeNames[10] = {
	"DISM MODE",
	"SOURCE MODE",
	"MIXED MODE",
	"BYTES (HEX)",
	"WORDS (HEX)",
	"LONGS (HEX)",
	"HUNKS",
	"SYMBOLS",
	"ONLINE HELP",
	"BREAKPOINTS"
};

/* The screen title is used to contain the debugger name and version,
 * its arexx port name, the current display mode, and
 * the program under debug with its arguments
 */

void SetTitle(char *title, DBugDisp *disp)
{
    int type;
    DBugDisp *ldisp = disp;

    if(!ldisp)ldisp=CurDisplay;
    type = ldisp->ds_DisplayMode;

    strcpy(ldisp->ds_windowTitle, " (" VERS " PORT: ");
    strcat(ldisp->ds_windowTitle,rexxhostname);
    strcat(ldisp->ds_windowTitle,") ");

    if((type < 10) && !title) {
	strcat(ldisp->ds_windowTitle,ModeNames[type]);
    }
    else {
	strcat(ldisp->ds_windowTitle,title);
    }
    strcat(ldisp->ds_windowTitle," ");

    strcat(ldisp->ds_windowTitle, targetName);
    if(argSize > 1)strncat(ldisp->ds_windowTitle, args, argSize-1);
    SetWindowTitles(ldisp->ds_Win,ldisp->ds_windowTitle,NULL);
}

void draw_fkey_boxes(void)
{
int i,xcol = 2,yrow = CurDisplay->ds_ScrRows, xlen;

    for(i=0; i < 10; i++) {
	xlen = strlen(fkeys[i]) + ((i == 9) ? 3 : 2);
	drawbox(xcol,yrow,xlen, -1);
	xcol += (xlen+1);
    }
}

// given the column, row, length, and height, draw a box

void drawbox(int xcol,int yrow, int xcollen, int yrowlen)
{
	WINDOW	 *win;
	RPORT	 *rp;
	int x,y,xlen,ylen, xmax, ymax;

    if(win = CurDisplay->ds_Win) {
	rp = win->RPort;

        y = win->BorderTop + ((yrow)  * rp->TxHeight);
	x = xcol * rp->TxWidth - 4;
	xlen = xcollen * rp->TxWidth + 1;
	ylen = rp->TxHeight + 3;
	
	if(x > (xmax = CurDisplay->ds_ScrCols * rp->TxWidth - 1))return; // out of range
	xlen = (x+xlen) <= xmax ? xlen : xmax-x;

	SetAPen(rp,2);	// white
	Move(rp,x,y);
	Draw(rp,x,y-ylen);
	Draw(rp,x+xlen,y-ylen);

	SetAPen(rp,1);	// black
	Draw(rp,x+xlen,y);
	if(y > (ymax = win->Height - 3))return; // out of range
	Draw(rp,x,y);
    }
}



void drawdoublebox(void)
{
	WINDOW	 *win;
	RPORT	 *rp;
	int x, y, xlen, ylen, i, j;

    if(win = CurDisplay->ds_Win) {
	rp = win->RPort;
	x = win->BorderLeft; // + rp->TxWidth - 6;
        y = win->BorderTop + (CurDisplay->ds_ScrRows-2) * rp->TxHeight + 2;
//	xlen = (CurDisplay->ds_ScrCols) * rp->TxWidth; // + 4;
	xlen = win->Width - win->BorderRight - win->BorderLeft - 2;
	ylen = rp->TxHeight + 6;

	for(i=2, j=0; i>0; i--, j++) {
	    SetAPen(rp,i);
//	    Move(rp,x+j,y+j);
//	    Draw(rp,x+j,y-ylen+j);
	    Move(rp,x+j,y-ylen+j);
	    Draw(rp,x+xlen+j,y-ylen+j);
//	    Draw(rp,x+xlen+j,y+j);
	    Move(rp,x+xlen+j,y+j);
	    Draw(rp,x+j,y+j);
	}
    }
}
