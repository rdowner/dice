/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 * CMD1.C   (was TEXT1.C)
 */

#include "defs.h"

Prototype void escapecomlinemode (void);
Prototype void setpen (int);
Prototype void text_cursor (int);
Prototype int text_init (ED *, WIN *, struct NewWindow *);
Prototype int text_switch (struct Window *);
Prototype int text_sync (void);
Prototype int text_load (void);
Prototype int text_colno (void);
Prototype int text_lineno (void);
Prototype int text_lines (void);
Prototype int text_cols (void);
Prototype int text_imode (void);
Prototype int text_tabsize (void);
Prototype unsigned char *text_name (void);
Prototype void text_uninit (void);
Prototype void inversemode (int);
Prototype void text_position (int, int);
Prototype void displayblock (int);
Prototype void text_redrawblock (int);
Prototype void text_redisplaycurrline (void);
Prototype void text_redisplay (void);
Prototype void text_write (unsigned char *);
Prototype void text_displayseg (int, int);
Prototype void movetocursor (void);
Prototype void do_up (void);
Prototype void do_scrolldown (void);
Prototype void do_scrollup (void);
Prototype void do_down (void);
Prototype void do_page (void);
Prototype void do_downadd (void);
Prototype void do_left (void);
Prototype void do_right (void);
Prototype void do_col (void);
Prototype void do_tab (void);
Prototype void do_backtab (void);
Prototype void do_return (void);
Prototype void do_bs (void);
Prototype void do_recall (void);
Prototype void do_esc (void);
Prototype void do_del (void);
Prototype void do_top (void);
Prototype void do_bottom (void);
Prototype void do_firstcolumn (void);
Prototype void do_firstnb (void);
Prototype void do_lastcolumn (void);
Prototype void do_goto (void);
Prototype void do_screentop (void);
Prototype void do_screenbottom (void);
Prototype void do_findstr (void);
Prototype void do_findr (void);
Prototype void do_find (void);
Prototype void do_findmatch(void);
Prototype void do_window(void);
Prototype void search_operation (void);
Prototype int case_strncmp (char *, char *, int);


typedef struct Process PROC;

#define nomemory()  { memoryfail = 1; }

char RecallBuf[256];

void
setpen(line)
{
    ED *ep = Ep;
    RP *rp = ep->Win->RPort;

    short pen = (ep == BEp && line >= BSline && line <= BEline) ? ep->HGPen : ep->FGPen;
    if (Comlinemode)
	pen = ep->FGPen;
    if (pen != rp->FgPen)
	SetAPen(rp, pen);
}

text_init(oldep, win, nw)
ED *oldep;
WIN *win;
struct NewWindow *nw;
{
    ED *e;

    text_switch(NULL);
    e = (ED *)allocb(sizeof(ED));
    if (e == NULL)
	return(0);
    setmem(e, sizeof(ED), 0);
    e->Win = win;
    if (oldep) {
	e->dirlock = (long)DupLock((BPTR)oldep->dirlock);

	movmem(&oldep->BeginConfig, &e->BeginConfig, (char *)&e->EndConfig - (char *)&e->BeginConfig);

	if (oldep->Font) {
	    e->Font = oldep->Font;
	    ++e->Font->tf_Accessors;
	    if (win)
		SetFont(win->RPort, e->Font);
	}
	e->IWiny = oldep->IWiny + 16;
    } else {
	PROC *proc = (PROC *)FindTask(NULL);
	e->dirlock = (long)DupLock((BPTR)proc->pr_CurrentDir);

	e->Insertmode = 1;
	e->Tabstop = 4;
	e->WWCol = -1;
	e->Margin= 75;
	e->FGPen = 1;
	e->BGPen = 0;
	e->HGPen = 2;
	loadconfig(e);
    }
    e->Lines = 1;
    e->Maxlines = 32;
    e->List = (ubyte **)allocl(e->Maxlines);
    e->List[0] = allocb(1);
    e->List[0][0] = Current[0] = Clen = 0;
    AddHead((LIST *)&DBase, (NODE *)e);
    strcpy(e->Name, "unnamed");
    Ep = e;

    if (nw) {
	if (e->Winwidth && e->Winheight) {
	    nw->LeftEdge= e->Winx;
	    nw->TopEdge = e->Winy;
	    nw->Width	= e->Winwidth;
	    nw->Height	= e->Winheight;
	} else {
	    nw->LeftEdge= 0;
	    nw->TopEdge = 0;
	    nw->Width	= 640;
	    nw->Height	= 200;
	}
	nw->DetailPen = e->BGPen;
	nw->BlockPen  = e->FGPen;
    }

    return(1);
}

text_switch(win)
WIN *win;
{
    ED *e;

    if (win)
	text_sync();
    if (win) {
	for (e = (ED *)DBase.mlh_Head; e->Node.mln_Succ; e = (ED *)e->Node.mln_Succ) {
	    if (e->Win == win) {
		Ep = e;
		text_load();
		if (!Ep->iconmode) {
		    set_window_params();
		    window_title();
		}
		return(1);
	    }
	}
	return(0);
    }
}


text_sync()
{
    ED *ep = Ep;
    char redraw = 0;
    short len;
    ubyte *ptr;

    for (len = strlen(Current) - 1; len >= 0 && Current[len] == ' '; --len)
	Current[len] = '\0';
    Clen = len + 1;
    if (!Comlinemode) {
	if (strlen(ep->List[ep->Line]) != Clen) {
	    if (ptr = allocb(Clen+1)) {
		ep->Modified = 1;
		Overide = 0;
		FreeMem(ep->List[ep->Line], strlen(ep->List[ep->Line])+1);
		ep->List[ep->Line] = ptr;
	    } else {
		nomemory();
		strcpy(Current, ep->List[ep->Line]);
		Clen = strlen(Current);
	    }
	} else {
	    if (strcmp(ep->List[ep->Line], Current)) {
		ep->Modified = 1;
		Overide = 0;
	    }
	}
	strcpy(ep->List[ep->Line], Current);
    }
    if (Nsu == 0) {
	if (ep->Column - ep->Topcolumn >= Columns || ep->Column < ep->Topcolumn) {
	    redraw = 1;
	    ep->Topcolumn = ep->Column - (Columns>>1);
	    if (ep->Topcolumn < 0)
		ep->Topcolumn = 0;
	}
	if (ep->Line - ep->Topline >= Rows || ep->Line < ep->Topline) {
	    redraw = 1;
	    ep->Topline = ep->Line - (Rows>>1);
	    if (ep->Topline < 0)
		ep->Topline = 0;
	}
    }
    while (ep->Column > Clen)
	Current[Clen++] = ' ';
    Current[Clen] = '\0';
    if (redraw)
	text_redisplay();
    return((int)redraw);
}

text_load()
{
    if (Comlinemode)
	return(0);
    strcpy(Current, Ep->List[Ep->Line]);
    Clen = strlen(Current);
    while (Ep->Column > Clen)
	Current[Clen++] = ' ';
    Current[Clen] = '\0';
}

text_colno()
{
    return(Ep->Column);
}

text_lineno()
{
    return(Ep->Line+1);
}

text_lines()
{
    return(Ep->Lines);
}

text_cols()
{
    return((int)Clen);
}

text_imode()
{
    return((int)Ep->Insertmode);
}

text_tabsize()
{
    return((int)Ep->Tabstop);
}

ubyte *
text_name()
{
    return(Ep->Name);
}

void
text_uninit()
{
    ED *ep = Ep;

    PMKill(ep);
    freelist(ep->List, ep->Lines);
    FreeMem(ep->List, ep->Maxlines * sizeof(char *));

    if (BEp == ep) {
	BEp = NULL;
	BSline = BEline = -1;
    }
    Remove((NODE *)ep);
    if (ep->Font) {
	SetFont(ep->Win->RPort, ep->Win->WScreen->RastPort.Font);
	CloseFont(ep->Font);
    }
    UnLock((BPTR)ep->dirlock);
    FreeMem(ep, sizeof(ED));
    if (((ED *)DBase.mlh_Head)->Node.mln_Succ) {
	Ep = (ED *)DBase.mlh_Head;
	text_load();
    } else {
	Ep = NULL;
    }
}

void
inversemode(n)
{
    RP *rp = Ep->Win->RPort;

    if (n) {
	SetAPen(rp, ~Ep->BGPen);
	SetDrMd(rp, JAM2|INVERSVID);
    } else {
	setpen(Ep->Line);
	SetDrMd(rp, JAM2);
    }
}

void
text_cursor(n)
{
    ED *ep = Ep;
    RP *rp = ep->Win->RPort;

    movetocursor();
    inversemode(n);
    if (Current[ep->Column])
	Text(rp, Current+ep->Column, 1);
    else
	Text(rp, " ", 1);
    inversemode(0);
}

void
text_position(col, row)
{
    ED *ep = Ep;
    text_sync();
    if (col == 0)
	col = -1;
    ep->Column = ep->Topcolumn + col;
    if (ep->Column > 254)
	ep->Column = 254;
    if (ep->Column < 0)
	ep->Column = 0;
    ep->Line = ep->Topline + row;
    if (ep->Line >= ep->Lines)
	ep->Line = ep->Lines - 1;
    if (ep->Line < 0)
	ep->Line = 0;
    text_load();
    text_sync();
}

void
displayblock(on)
{
    long start = Ep->Topline;
    long lines = BEline - BSline + 1;

    if (start < BSline)
	start = BSline;
    if (!on) {
	BSline = BEline = -1;
	BEp = NULL;
    }
    if (Ep == BEp)
	text_displayseg(start - Ep->Topline, lines);
}

void
text_redrawblock(ok)
{
    WIN *savewin = NULL;

    if (BEp) {
	if (BEp != Ep) {
	    savewin = Ep->Win;
	    text_switch(BEp->Win);
	}
	if (BSline <= BEline && BSline >= 0 && BEline < Ep->Lines) {
	    if (!ok) {
		BEp = NULL;
		BSline = BEline = -1;
	    }
	    text_displayseg(0, Rows);
	}
	if (savewin)
	    text_switch(savewin);
    }
    if (!ok) {
	BEp = NULL;
	BSline = BEline = -1;
    }
}

void
text_displayseg(start, n)
{
    short i, c;
    ubyte *ptr;
    ED *ep = Ep;
    RP *rp = ep->Win->RPort;

    if (Nsu)
	return;
    for (i = start; i < start + n && i < Rows && ep->Topline + i < ep->Lines; ++i) {
	if (Comlinemode) {
	    if (ep->Topline + i != ep->Line)
		continue;
	    ptr = Current;
	    SetAPen(rp, ep->FGPen);
	} else {
	    ptr = ep->List[ep->Topline + i];
	    setpen(i+ep->Topline);
	}
	for (c = ep->Topcolumn; c && *ptr; ++ptr, --c);
	c = strlen(ptr);
	if (c) {
	    Move(rp, COLT(0), ROWT(i));
	    Text(rp, ptr, (c > Columns) ? Columns : c);
	}
    }
}

void
text_redisplay()
{
    ED *ep = Ep;
    RP *rp = ep->Win->RPort;

    if (Nsu)
	return;
    SetAPen(rp, ep->BGPen);
    if (Comlinemode)
	RectFill(rp, COL(0), ROW(Rows-1), Xbase + Xpixs - 1, Ybase + Ypixs - 1);
    else
	RectFill(rp, Xbase, Ybase, Xbase + Xpixs - 1, Ybase + Ypixs - 1);
    text_displayseg(0,Rows);
}

void
text_redisplaycurrline()
{
    ED *ep = Ep;
    RP *rp = ep->Win->RPort;

    int row = ep->Line - ep->Topline;

    if (Nsu)
	return;
    SetAPen(rp, ep->BGPen);
    RectFill(rp, COL(0), ROW(row), Xbase + Xpixs - 1, ROW(row+1)-1);
    text_displayseg(row, 1);
}

void
text_write(str)
ubyte *str;
{
    short len = strlen(str);
    short i;
    ED *ep = Ep;
    RP *rp = ep->Win->RPort;

    if (Clen + len >= 255) {
	text_sync();
	text_load();
    }
    if (ep->Insertmode == 0) {
	if (ep->Column + len >= 255)
	    goto fail;
	movmem(str, Current + ep->Column, len);
	if (ep->Column + len >= Clen)
	    Clen = ep->Column + len;
	Current[Clen] = 0;
    } else {
	if (Clen + len >= 255)
	    goto fail;
	movmem(Current + ep->Column, Current + ep->Column + len, Clen+1-ep->Column);
	movmem(str, Current + ep->Column, len);
	Clen += len;
	if (len < Columns - (ep->Column - ep->Topcolumn)) {
	    ScrollRaster(rp, -len * Xsize, 0 ,
		COL(ep->Column - ep->Topcolumn),
		ROW(ep->Line - ep->Topline),
		COL(Columns) - 1,
		ROW(ep->Line - ep->Topline + 1) - 1
	    );
	}
    }
    i = (ep->Column - ep->Topcolumn + len > Columns) ? Columns - ep->Column + ep->Topcolumn : len;
    setpen(ep->Line);
    Move(rp, COLT(ep->Column - ep->Topcolumn), ROWT(ep->Line - ep->Topline));
    Text(rp, str, i);
    ep->Column += len;
    if (ep->Column - ep->Topcolumn >= Columns)
	text_sync();
fail:
    if (Comlinemode == 0 && ep->Wordwrap)
	do_reformat(0);
}

void
do_up()
{
    ED *ep = Ep;
    RP *rp = ep->Win->RPort;

    if (ep->Line) {
	text_sync();
	--ep->Line;
	text_load();
	if (Ep->Line < Ep->Topline) {
	    if (Nsu == 0) {
		ScrollRaster(rp,0,-Ysize,COL(0),ROW(0),COL(Columns)-1,ROW(Rows)-1);
		--ep->Topline;
		text_displayseg(0, 1);
	    }
	}
    } else {
	Abortcommand = 1;
    }
}

void
do_scrolldown()
{
    ED *ep = Ep;
    RP *rp = ep->Win->RPort;

    if (ep->Topline + Rows < ep->Lines) {
	if (Nsu == 0) {
	    text_sync();
	    ScrollRaster(rp,0,Ysize,COL(0),ROW(0),COL(Columns)-1,ROW(Rows)-1);
	    ++ep->Topline;
	    ++ep->Line;
	    text_load();
	    text_displayseg(Rows-1, 1);
	}
    } else {
	Abortcommand = 1;
    }
}

void
do_scrollup()
{
    ED *ep = Ep;
    RP *rp = ep->Win->RPort;

    if (ep->Topline) {
	if (Nsu == 0) {
	    text_sync();
	    ScrollRaster(rp,0,-Ysize,COL(0),ROW(0),COL(Columns)-1,ROW(Rows)-1);
	    --ep->Topline;
	    --ep->Line;
	    text_load();
	    text_displayseg(0, 1);
	}
    } else {
	Abortcommand = 1;
    }
}

void
do_down()
{
    ED *ep = Ep;
    RP *rp = ep->Win->RPort;

    if (ep->Line + 1 < ep->Lines) {
	text_sync();
	++ep->Line;
	text_load();
	if (ep->Line - ep->Topline >= Rows) {
	    if (Nsu == 0) {
		ScrollRaster(rp,0,Ysize,COL(0),ROW(0),COL(Columns)-1,ROW(Rows)-1);
		++ep->Topline;
		text_displayseg(Rows-1, 1);
	    }
	}
    } else {
	Abortcommand = 1;
    }
}

/*
 *  PAGEUP
 *  PAGEDOWN
 *  PAGESET n	(n = 0 to 100 for percentage of #rows to scroll, minimum 1)
 *		can be > 100.
 */

void
do_page()
{
    int n, multiplier = 1;
    ED *ep = Ep;
    static short pctg = 80;

    switch(av[0][4]) {
    case 'u':
	multiplier = -1;
    case 'd':
	n = multiplier * Rows * pctg / 100;
	if (!n)
	    n = multiplier;
	if (n > 0 && ep->Topline >= ep->Lines - Rows)
	    return;
	text_sync();
	ep->Line += n;
	ep->Topline += n;
	if (ep->Line >= ep->Lines)
	    ep->Line = ep->Lines - 1;
	if (ep->Line < 0)
	    ep->Line = 0;
	if (ep->Topline >= ep->Lines)
	    ep->Topline = ep->Lines - Rows - 1;
	if (ep->Topline < 0)
	    ep->Topline = 0;
	text_load();
	if (!text_sync())
	    text_redisplay();
	break;
    case 's':
	pctg = atoi(av[1]);
	break;
    }
}

void
do_downadd()
{
    ED *ep = Ep;
    ubyte *ptr;

    if (ep->Line + 1 == ep->Lines) {
	ep->Modified = 1;
	if (makeroom(32) && (ptr = allocb(1))) {
	    ep->List[ep->Lines] = ptr;
	    *ptr = 0;
	    ++ep->Lines;
	} else {
	    nomemory();
	}
    }
    do_down();
}

void
do_left()
{
    ED *ep = Ep;

    if (ep->Column) {
	--ep->Column;
	if (ep->Column < ep->Topcolumn)
	    text_sync();
    } else {
	Abortcommand = 1;
    }
}

void
do_right()
{
    ED *ep = Ep;

    if (ep->Column != 254) {
	if (Current[ep->Column] == 0) {
	    Current[ep->Column] = ' ';
	    Current[ep->Column+1]= '\0';
	    ++Clen;
	}
	++ep->Column;
	if (ep->Column - ep->Topcolumn >= Columns)
	    text_sync();
    } else {
	Abortcommand = 1;
    }
}

void
do_tab()
{
    short n;
    ED *ep = Ep;

    for (n = ep->Tabstop-(ep->Column % ep->Tabstop); n > 0; --n)
	do_right();
}

void
do_backtab()
{
    short n;
    ED *ep = Ep;

    n = ep->Column % ep->Tabstop;
    if (!n)
	n = ep->Tabstop;
    for (; n > 0; --n)
	do_left();
}

void
do_return()
{
    ubyte buf[256];
    char *partial;

    if (Comlinemode) {
	strcpy(buf, Current);
	strcpy(RecallBuf, Current);
	partial = Partial;
	Partial = NULL;
	escapecomlinemode();
	if (partial) {
	    if (do_command(buf))
		do_command(partial);
	    free(partial);
	} else {
	    do_command(buf);
	}
    } else {
	Ep->Column = 0;
	text_sync();
	do_downadd();
    }
}

void
do_bs()
{
    ED *ep = Ep;
    RP *rp = ep->Win->RPort;

    if (ep->Column) {
	movmem(Current + ep->Column, Current + ep->Column - 1, Clen - ep->Column + 1);
	--ep->Column;
	--Clen;
	if (ep->Column < ep->Topcolumn) {
	    text_sync();
	} else {
	    ScrollRaster(rp, Xsize, 0,
		COL(ep->Column - ep->Topcolumn),
		ROW(ep->Line   - ep->Topline),
		COL(Columns)-1,
		ROW(ep->Line - ep->Topline + 1)-1
	    );
	    if (Clen >= ep->Topcolumn + Columns) {
		setpen(ep->Line);
		Move(rp, COLT(Columns-1), ROWT(ep->Line - ep->Topline));
		Text(rp, Current + ep->Topcolumn + Columns - 1, 1);
	    }
	}
	if (Comlinemode == 0 && ep->Wordwrap)
	    do_reformat(0);
    } else {
	Abortcommand = 1;
    }
}


/*
 * esc, escimm
 */

int Savetopline, Savecolumn, Savetopcolumn;

void
do_recall()
{
    av[0] = (ubyte *)"escimm";
    av[1] = (ubyte *)RecallBuf;
    do_esc();
}

void
do_esc()
{
    ED *ep = Ep;
    RP *rp = ep->Win->RPort;

    if (Comlinemode) {
	escapecomlinemode();
	return;
    }
    text_sync();
    if (av[0][3] == 'i')
	strcpy(Current, av[1]);
    else
	Current[0] = 0;
    Clen = strlen(Current);
    Comlinemode = 1;
    returnoveride(1);
    Savetopline = ep->Topline;
    Savecolumn	= ep->Column;
    Savetopcolumn = ep->Topcolumn;
    ep->Column	  = Clen;
    ep->Topcolumn = 0;
    ep->Topline   = ep->Line - Rows + 1;
    SetAPen(rp, ep->BGPen);
    RectFill(rp, COL(0), ROW(Rows-1), Xbase + Xpixs - 1, Ybase + Ypixs - 1);
    SetAPen(rp, ep->FGPen);
    Move(rp, COL(0), ROW(Rows-1) - 1);
    Draw(rp, Xbase + Xpixs - 1, ROW(Rows - 1) - 1);
    text_displayseg(Rows - 1, 1);
}

void
escapecomlinemode()
{
    ED *ep = Ep;
    RP *rp = ep->Win->RPort;

    if (Partial) {
	free(Partial);
	Partial = NULL;
    }
    if (Comlinemode) {
	strcpy(RecallBuf, Current);
	Comlinemode = 0;
	returnoveride(0);
	ep->Topline = Savetopline;
	ep->Column  = Savecolumn;
	ep->Topcolumn = Savetopcolumn;
	text_load();
	SetAPen(rp, ep->BGPen);
	RectFill(rp, COL(0), ROW(Rows-1)-1, Xbase + Xpixs - 1, Ybase + Ypixs - 1);
	SetAPen(rp, ep->FGPen);
	text_displayseg(Rows - 2, 2);
    }
}

void
do_del()
{
    ED *ep = Ep;
    RP *rp = ep->Win->RPort;

    if (Current[ep->Column]) {
	movmem(Current + ep->Column + 1, Current + ep->Column, Clen - ep->Column);
	--Clen;
	ScrollRaster(rp, Xsize, 0,
	    COL(ep->Column - ep->Topcolumn),
	    ROW(ep->Line - ep->Topline),
	    COL(Columns)-1,
	    ROW(ep->Line - ep->Topline + 1) - 1
	);
	if (Clen >= ep->Topcolumn + Columns) {
	    setpen(ep->Line);
	    Move(rp, COLT(Columns-1), ROWT(ep->Line-ep->Topline));
	    Text(rp, Current+ep->Topcolumn+Columns-1, 1);
	}
	if (Comlinemode == 0 && ep->Wordwrap)
	    do_reformat(0);
    }
}

void
do_top()
{
    text_sync();
    Ep->Line = 0;
    text_load();
    text_sync();
}

void
do_bottom()
{
    text_sync();
    Ep->Line = Ep->Lines - 1;
    text_load();
    text_sync();
}

void
do_firstcolumn()
{
    if (Ep->Column) {
	Ep->Column = 0;
	text_sync();
    }
}

void
do_firstnb()
{
    for (Ep->Column = 0; Current[Ep->Column] == ' '; ++Ep->Column);
    if (Current[Ep->Column] == 0)
	Ep->Column = 0;
    text_sync();
}

void
do_lastcolumn()
{
    short i;

    text_sync();
    i = (Comlinemode) ? Clen : strlen(Ep->List[Ep->Line]);
    if (i != Ep->Column) {
	Ep->Column = i;
	text_sync();
    }
}

/*
 * GOTO [+/-]N
 * GOTO BLOCK	start of block
 * GOTO START	start of block
 * GOTO END	end of block
 */

void
do_goto()
{
    long n, i;
    ubyte *ptr = av[1];

    i = 0;
    n = -1;

    switch(*ptr) {
    case 'b':
    case 's':
    case 'B':
    case 'S':
	n = -1;
	if (Ep == BEp)
	    n = BSline;
	break;
    case 'e':
    case 'E':
	n = -1;
	if (Ep == BEp)
	    n = BEline;
	break;
    case '+':
	i = 1;
    case '-':
	n = Ep->Line;
    default:
	n += atoi(ptr+i);
    }
    if (n >= Ep->Lines)
	n = Ep->Lines - 1;
    if (n < 0)
	n = 0;
    text_sync();
    Ep->Line = n;
    text_load();
    text_sync();
}

void
do_screentop()
{
    text_sync();
    Ep->Line = Ep->Topline;
    text_load();
    text_sync();
}

void
do_screenbottom()
{
    text_sync();
    Ep->Line = Ep->Topline + Rows - 1;
    if (Ep->Line < 0 || Ep->Line >= Ep->Lines)
	Ep->Line = Ep->Lines - 1;
    text_load();
    text_sync();
}

static ubyte Fstr[256];
static ubyte Rstr[256];
static short Srch_sign;
static char Doreplace;

/*
 * findstr, repstr
 */

void
do_findstr()
{
    if (av[0][0] == 'f')
	strcpy(Fstr, av[1]);
    else
	strcpy(Rstr, av[1]);
}

/*
 * findr, nextr, prevr
 */

void
do_findr()
{
    Doreplace = 1;
    Srch_sign = 1;
    switch(av[0][0]) {
    case 'f':
	strcpy(Fstr, av[1]);
	strcpy(Rstr, av[2]);
	break;
    case 'p':
	Srch_sign = -1;
	break;
    }
    search_operation();
}

/*
 * find, next, prev
 */

void
do_find()
{
    Doreplace = 0;
    Srch_sign = 1;
    switch(av[0][0]) {
    case 'f':
	strcpy(Fstr, av[1]);
	break;
    case 'p':
	Srch_sign = -1;
	break;
    }
    search_operation();
}


static char CaseIgnore;

void
search_operation()
{
    int flen = strlen(Fstr);
    int rlen = strlen(Rstr);
    char senabled = 0;
    ubyte *ptr;
    int i, col;
    ED *ep = Ep;

    CaseIgnore = ep->IgnoreCase;
    text_sync();
    if (!flen) {
	title("No find pattern");
	Abortcommand = 1;
	return;
    }

    col = ep->Column;
    if (col >= strlen(ep->List[ep->Line]))
	col = strlen(ep->List[ep->Line]);
    for (i = ep->Line;;) {
	ptr = ep->List[i];
	if (Srch_sign > 0) {
	    while (ptr[col]) {
		if (senabled && case_strncmp(Fstr,ptr+col,flen) == 0)
		    goto found;
		senabled = 1;
		++col;
	    }
	    senabled = 1;
	    if (++i >= ep->Lines)
		break;
	    col = 0;
	} else {
	    while (col >= 0) {
		if (senabled && case_strncmp(Fstr,ptr+col,flen) == 0)
		    goto found;
		senabled = 1;
		--col;
	    }
	    senabled = 1;
	    if (--i < 0)
		break;
	    col = strlen(ep->List[i]);
	}
    }
    title("Pattern Not Found");
    Abortcommand = 1;
    return;

found:
    ep->Line = i;
    ep->Column = col;

    text_load();
    if (Doreplace) {
	if (rlen > flen && rlen-flen+strlen(ptr) > 254) {
	    title("Replace: Line Too Long");
	    Abortcommand = 1;
	    return;
	}
	if (Clen-col-flen >= 0) {
	    movmem(Current+col+flen, Current+col+rlen, Clen-col-flen+1);
	    movmem(Rstr, Current+col, rlen);
	    Clen += rlen-flen;
	    ep->Column += rlen;
	}
	text_sync();
	text_redisplaycurrline();
    } else {
	text_sync();
    }
}

case_strncmp(s1, s2, len)
char *s1, *s2;
{
    if (CaseIgnore == 0)
	return(strncmp(s1, s2, len));
    for (; len; --len, ++s1, ++s2) {
	if ((*s1|0x20) != (*s2|0x20))
	    return(1);
	if (((*s1 >= 'a' && *s1 <= 'z') || (*s1 >= 'A' && *s1 <= 'Z')) &&
	    ((*s2 >= 'a' && *s2 <= 'z') || (*s2 >= 'A' && *s2 <= 'Z')))
	{
	    continue;
	}
	if (*s1 != *s2)
	    return(1);
    }
    return(0);
}


static ubyte brackets[][2] = {
    '(',    ')',
    '{',    '}',
    '[',    ']',
    '<',    '>',
    '`',    '\'',
    0
};

void
do_findmatch()
{
    ubyte c, c2;
    ubyte *lineptr;
    short direction = 0, i;
    long cnt = 0;
    long line, column;
    long endline, endcol, col, len;

    line = Ep->Line;
    column = Ep->Column;
    c = Current[column];

    for (i = 0; brackets[i][0]; i++) {
	if (brackets[i][0] == c) {	/* forward  */
	    c2 = brackets[i][1];
	    direction = 1;
	    endline = Ep->Lines - 1;
	    break;
	}
	if (brackets[i][1] == c) {	/* backward */
	    c2 = brackets[i][0];
	    direction = -1;
	    endline = 0;
	    break;
	}
    }

    if (direction == 0) {
	title("not matchable character");
	return;
    }

    for ( ; line != endline + direction; line += direction) {

	if (cnt == 0) {
	    lineptr = Current;	    /* current line (we're just starting) */
	    len = Clen;
	    col = column;
	} else {
	    lineptr = Ep->List[line];
	    len = strlen(lineptr);
	    col = (direction == 1) ? 0 : len - 1;
	}
	endcol = (direction == 1) ? len - 1 : 0;

	for ( ; col != endcol + direction; col += direction) {
	    if (lineptr[col] == c)
		cnt++;
	    else if (lineptr[col] == c2) {
		cnt--;
		if (cnt == 0) {     /* found match!!	      */
		    text_sync();    /* ok, now update buffers */
		    Ep->Line = line;
		    Ep->Column = col;
		    text_load();    /* and move to new place  */
		    text_sync();
		    return;
		}
	    }
	}
    }
    title("match not found");
}

/*
 *  Change to window by filepath, open new window and load new file
 *  if window cannot be found.
 */

void
do_window()
{
    ED *ed;
    char buf[128];

    if ((ed = finded(av[1], 0)) == NULL) {
	sprintf(buf, "newwindow newfile (%s)", av[1]);
	do_command(buf);
	ed = finded(av[1], 0);
    } else {
	WindowToFront(ed->Win);
	ActivateWindow(ed->Win);
    }
    if (ed == NULL) {
	title("unable to load file");
    } else {
	text_switch(ed->Win);
	if (Ep->iconmode)
	    uniconify();
	else
	    text_cursor(0);
    }
}

