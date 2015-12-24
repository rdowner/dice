/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  KTS.C
 *
 *  Additional DME commands written by Kevin T. Seghetti fixed up and
 *  incorporated by Matt Dillon 17 April 1988.
 */

#include "defs.h"
#include <intuition/intuitionbase.h>

Prototype void PMAdd (void);
Prototype void PMRem (void);
Prototype void PMKill (struct _ED *);
Prototype int do_pushmark (void);
Prototype void do_popmark (void);
Prototype void do_swapmark (void);
Prototype void do_purgemark (void);
Prototype void do_ping (void);
Prototype void do_pong (void);
Prototype void do_undo (void);

#define BLOCKDEPTH  5
#define PINGDEPTH   10

static long BSstack[BLOCKDEPTH];
static long BEstack[BLOCKDEPTH];
static ED   *Bp[BLOCKDEPTH];
static int  CurrDepth = 0;

static long PingLine[PINGDEPTH];
static long PingCol[PINGDEPTH];
static ED   *PingWin[PINGDEPTH];

void
PMAdd()
{
}

void
PMRem()
{
}

void
PMKill(ep)
ED *ep;
{
    short i, j;

    for (i = 0; i < PINGDEPTH; ++i) {       /*  remove ping-pong marks  */
	if (PingWin[i] == ep)
	    PingWin[i] = NULL;
    }
    for (i = j = 0; i < CurrDepth; ++i) {   /*  remove block marks      */
	Bp[j] = Bp[i];
	if (Bp[i] != ep)
	    ++j;
    }
    CurrDepth = j;
}

do_pushmark()
{
    text_sync();
    if (blockok()) {
	if (CurrDepth == BLOCKDEPTH) {
	    title("pushmark: stack limit reached");
	    return(-1);
	}
	BSstack[CurrDepth] = BSline;
	BEstack[CurrDepth] = BEline;
	Bp[CurrDepth] = BEp;

	++CurrDepth;
	text_redrawblock(0);
    }
    return(0);
}

void
do_popmark()
{
    text_sync();

    if (!CurrDepth) {           /*  no error message on purpose */
	text_redrawblock(0);    /*  remove any existing block   */
	return;
    }
    text_redrawblock(0);
    --CurrDepth;
    BSline = BSstack[CurrDepth];
    BEline = BEstack[CurrDepth];
    BEp = Bp[CurrDepth];
    if (BEp == NULL || BEline >= BEp->Lines) {
	BEp = NULL;
	BSline = BEline = -1;
    } else
	text_redrawblock(1);
}

void
do_swapmark()
{
    short i;
    long *ptmp;
    long tmp;

    if (do_pushmark() < 0)
	return;
    i = CurrDepth - 2;
    if (i >= 0) {
	ptmp = PingLine + i;
	tmp = ptmp[0]; ptmp[0] = ptmp[1]; ptmp[1] = tmp;
	ptmp = PingCol + i;
	tmp = ptmp[0]; ptmp[0] = ptmp[1]; ptmp[1] = tmp;
	ptmp = (long *)PingWin + i;
	tmp = ptmp[0]; ptmp[0] = ptmp[1]; ptmp[1] = tmp;
    }
    do_popmark();
}

void
do_purgemark()
{
    CurrDepth = 0;
}

void
do_ping()
{
    uword num = atoi(av[1]);

    if (num >= PINGDEPTH) {
	title("ping: out of range");
	return;
    }
    PingLine[num]= Ep->Line;
    PingCol[num] = Ep->Column;
    PingWin[num] = Ep;
    title("Line marked");
}

void
do_pong()
{
    uword num = atoi(av[1]);
    extern IBASE *IntuitionBase;

    text_sync();
    if (num < 0 || num >= PINGDEPTH || !PingWin[num]) {
	title("pong: range error or nothing marked");
	return;
    }
    text_cursor(1);
    text_switch(PingWin[num]->Win);
    text_cursor(0);

    if (IntuitionBase->ActiveWindow != Ep->Win) {
	WindowToFront(Ep->Win);
	ActivateWindow(Ep->Win);
    }
    if ((Ep->Line = PingLine[num]) >= Ep->Lines) {
	PingLine[num] = Ep->Line = Ep->Lines - 1;
    }
    Ep->Column = PingCol[num];
    text_load();
    text_sync();
}

void
do_undo()
{
    text_load();
    text_redisplaycurrline();
}

