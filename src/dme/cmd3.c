/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 * CMD3.C
 *
 *  SETFONT
 *  IGNORECASE
 *  SET
 *  SETENV
 *  UNSET
 *  UNSETENV
 *  CD
 *  SAVECONFIG
 *  FGPEN
 *  TPEN
 *  BGPEN
 *  TITLE
 *  JUSTIFY
 *  UNJUSTIFY
 *  MODIFIED
 *  UNDELINE
 */

#include "defs.h"

Prototype void do_setfont (void);
Prototype void do_ignorecase (void);
Prototype void do_cd (void);
Prototype void do_set (void);
Prototype void do_setenv (void);
Prototype void do_unset (void);
Prototype void do_unsetenv (void);
Prototype char *getvar (char *);
Prototype void loadconfig(ED *);

Prototype void do_saveconfig(void);
Prototype void do_fgpen(void);
Prototype void do_bgpen(void);
Prototype void do_hgpen(void);
Prototype void do_tpen(void);
Prototype void do_justify(void);
Prototype void do_title(void);
Prototype void do_undeline(void);
Prototype void do_unjustify(void);
Prototype void do_modified(void);
Prototype void do_sizewindow(void);
Prototype void do_space(void);


#define nomemory()  { memoryfail = 1; }

/*
 *  SETFONT font size
 */

void
do_setfont()
{
    FONT *font = (FONT *)GetFont(av[1], (short)atoi(av[2]));
    ED *ep = Ep;
    if (font) {
	text_sync();
	if (ep->Font)
	    CloseFont(ep->Font);
	ep->Font = font;
	SetFont(ep->Win->RPort, font);
	SetRast(ep->Win->RPort, 0);
	RefreshWindowFrame(ep->Win);
	set_window_params();
	text_redisplay();
    } else {
	title("Unable to find font");
    }
}

void
do_ignorecase()
{
    ED *ep = Ep;

    if (av[1][0]) {
	switch(av[1][1] & 0x1F) {
	case 'n'&0x1F:
	    ep->IgnoreCase = 1;
	    break;
	case 'f'&0x1F:
	    ep->IgnoreCase = 0;
	    break;
	case 'o'&0x1F:
	    ep->IgnoreCase = 1 - ep->IgnoreCase;
	    break;
	}
	if (ep->IgnoreCase)
	    title("Case InSensitive");
	else
	    title("Case Sensitive");
    }
}

/*
 *  av[1]
 */

void
do_cd()
{
    BPTR oldlock;
    BPTR lock;

    oldlock = CurrentDir((BPTR)Ep->dirlock);
    if (lock = Lock(av[1], SHARED_LOCK)) {
	UnLock(CurrentDir(oldlock));
	Ep->dirlock = (long)lock;
    } else {
	CurrentDir(oldlock);
	Abortcommand = 1;
	title("Unable to CD");
    }
}

/*
 *  VARIABLE SUPPORT!
 */

#define VARS	struct _VARS
VARS {
    MNODE   Node;
    char    *Name;
    char    *Str;
};

static MLIST SList = { (MNODE *)&SList.mlh_Tail, NULL, (MNODE *)&SList.mlh_Head };

void
do_set()
{
    VARS *v;
    void do_unset();

    do_unset();
    if (v = malloc(sizeof(VARS))) {
	if (v->Name = malloc(strlen(av[1])+1)) {
	    if (v->Str = malloc(strlen(av[2])+1)) {
		AddHead((LIST *)&SList, (NODE *)v);
		strcpy(v->Name, av[1]);
		strcpy(v->Str , av[2]);
		return;
	    }
	    free(v->Name);
	}
	free(v);
    }
    nomemory();
}

void
do_setenv()
{
    SetDEnv(av[1], av[2]);
}

void
do_unset()
{
    VARS *v;

    for (v = (VARS *)SList.mlh_Head; v->Node.mln_Succ; v = (VARS *)v->Node.mln_Succ) {
	if (strcmp(v->Name, av[1]) == 0) {
	    Remove((NODE *)v);
	    free(v->Name);
	    free(v->Str);
	    free(v);
	    break;
	}
    }
}

void
do_unsetenv()
{
    char *ptr = (char *)av[1];
    char *tmp = malloc(4+strlen(ptr)+1);

    if (tmp) {
	strcpy(tmp, "ENV:");
	strcat(tmp, ptr);
	mountrequest(0);
	DeleteFile(tmp);
	mountrequest(1);
	free(tmp);
    }
}

/*
 *  Search (1) internal list, (2) enviroment, (3) macros.  The variable
 *  is allocated with malloc().  NULL if not found.  ENV: need not exist.
 */

char *
getvar(find)
char *find;
{
    char *str = NULL;
    {
	VARS *v;

	for (v = (VARS *)SList.mlh_Head; v->Node.mln_Succ; v = (VARS *)v->Node.mln_Succ) {
	    if (strcmp(v->Name, find) == 0) {
		if (str = malloc(strlen(v->Str)+1)) {
		    strcpy(str, v->Str);
		    return(str);
		}
	    }
	}
    }

    mountrequest(0);
    str = (char *)GetDEnv(find);
    mountrequest(1);
    if (str)
	return(str);

    if ((str = keyspectomacro(find)) || (str = menutomacro(find))) {
	char *ptr = malloc(strlen(str)+1);
	if (ptr) {
	    strcpy(ptr, str);
	    return(ptr);
	}
    }
    return(NULL);
}

void
do_col()
{
    int col;

    {
	char *ptr = av[1];

	switch(*ptr) {
	case '+':
	    col = text_colno() + atoi(ptr + 1);
	    if (col > 254)
		col = 254;
	    break;
	case '-':
	    col = text_colno() + atoi(ptr);
	    if (col < 0)
		col = 0;
	    break;
	default:
	    col = atoi(ptr) - 1;
	    break;
	}
    }
    if (col > 254 || col < 0) {
	Abortcommand = 1;
	return;
    }
    while (Clen < col)
	Current[Clen++] = ' ';
    Current[Clen] = 0;
    Ep->Column = col;
    if (Ep->Column - Ep->Topcolumn >= Columns || Ep->Column < Ep->Topcolumn)
	text_sync();
}

void
do_saveconfig()
{
    ED *ep = Ep;
    FILE *fi;

    if (ep->iconmode == 0) {
	WIN *win = ep->Win;
	ep->Winx      = win->LeftEdge;
	ep->Winy      = win->TopEdge;
	ep->Winwidth  = win->Width;
	ep->Winheight = win->Height;
    }

    if (fi = fopen("s:dme.config", "w")) {
	fwrite(&ep->BeginConfig, (char *)&ep->EndConfig - (char *)&ep->BeginConfig, 1, fi);
	fclose(fi);
    }
}

void
loadconfig(ep)
ED *ep;
{
    FILE *fi;

    if (fi = fopen("s:dme.config", "r")) {
	fread(&ep->BeginConfig, (char *)&ep->EndConfig - (char *)&ep->BeginConfig, 1, fi);
	fclose(fi);
    }
}

void
do_fgpen()
{
    ED *ep = Ep;

    ep->FGPen = atoi(av[1]);
}

void
do_tpen()
{
    ED *ep = Ep;

    ep->TPen = atoi(av[1]);
}

void
do_bgpen()
{
    ED *ep = Ep;

    ep->BGPen = atoi(av[1]);
}

void
do_hgpen()
{
    ED *ep = Ep;

    ep->HGPen = atoi(av[1]);
}

/*
 *  Commands submitted by Markus Wenzel
 */

void
do_undeline()
{
   do_insline();
   text_load();
   strcpy(Current, Deline);
   text_sync();
   text_displayseg(Ep->Line - Ep->Topline, 1);
}


void
do_modified()
{
    register ED *ep = Ep;

    if (av[1][0]) {
	switch(av[1][1] & 0x1F) {
	case 'n' & 0x1F:
	    ep->Modified = 1;
	    break;
	case 'f' & 0x1F:
	    ep->Modified = 0;
	    break;
	case 'o' & 0x1F:
	    ep->Modified = ep->Modified ? 0 : 1;
	    break;
	}
    }
}


void
do_unjustify()
{
    short i, j, waswhite = FALSE;
    ubyte c;


    for (i = 0; Current[i] == ' '; i++);
    for (j = i; Current[i]; i++) {
	c = Current[j] = Current[i];
	if (c != ' ' || !waswhite)
	    j++;
	waswhite = (c == ' ');

    }
    Current[j] = 0;

    if (i != j) {
	text_sync();
	text_redisplaycurrline();
    }
}


void
do_justify()
{
    ED *ep = Ep;
    short firstnb, lastnb, i, n, fill, c, sp;
    short changed = FALSE;


    switch(av[1][0]) {
    case 'c':
	break;
    case 'f':
	firstnb = firstns(Current);
	lastnb = lastns(Current);
	if (firstnb < lastnb && ep->Margin < 255) {
	    n = 0;
	    i = firstnb;
	    while (i <= lastnb) {
		while ((c = Current[i]) && c != ' ')
		    i++;
		if (i <= lastnb) {
		    n++;
		    while (Current[i] == ' ')
			i++;
		}
	    }
	    fill = ep->Margin - lastnb - 1;
	    i = firstnb;
	    Current[lastnb + 1] = 0;
	    if (n > 0 && fill > 0)
		changed = TRUE;
	    while (n > 0 && fill > 0 && Current[i]) {
		while ((c = Current[i]) && c != ' ')
		    i++;
		sp = fill / n;
		movmem(&Current[i], &Current[i + sp], strlen(&Current[i]) + 1);
		memset(&Current[i], ' ', sp);
		while (Current[i] == ' ')
		    i++;
		fill -= sp;
		n--;
	    }
	}
	break;
    default:
	break;
    }

    if (changed) {
	text_sync();
	text_redisplaycurrline();
    }
}


void
do_title()
{
    static ubyte wtitle[256];

    strncpy(wtitle, av[1], 255);
    wtitle[255] = 0;
    title(wtitle);
}

void
do_space()
{
    ED *ep = Ep;
    int insmode = ep->Insertmode;

    ep->Insertmode = 1;
    text_write(" ");
    ep->Insertmode = insmode;
}

void						/* MMW 1.42 */
do_sizewindow()
{
    volatile WIN *win = Ep->Win;
    struct NewWindow nw;
    int mdx, mdy;

    GeometryToNW(av[1], &nw);

    if (nw.LeftEdge + nw.Width <= win->WScreen->Width &&
	nw.TopEdge + nw.Height <= win->WScreen->Height &&
	nw.Width >= win->MinWidth &&
	nw.Height >= win->MinHeight) {

	mdx = nw.LeftEdge - win->LeftEdge;
	mdy = nw.TopEdge - win->TopEdge;
	if (mdx > 0)
	    mdx = 0;
	if (mdy > 0)
	    mdy = 0;

	MoveWindow(win, mdx, mdy);
	SizeWindow(win, nw.Width - win->Width, nw.Height - win->Height);
	MoveWindow(win, nw.LeftEdge - win->LeftEdge, nw.TopEdge - win->TopEdge);
    }
}

