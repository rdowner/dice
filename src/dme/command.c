/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 * COMMAND.C
 *
 * )c		     single character (typing)
 * 'c                single character (typing)
 * `string'          string of characters w/ embedded `' allowed!
 * (string)		same thing w/ embedded () allowed!
 * \c		     override
 *
 * name arg arg      command name. The arguments are interpreted as strings
 *		     for the command.
 *
 * $scanf	     macro insert scanf'd variable
 * $filename	     macro insert current file name
 *
 * Any string arguments not part of a command are considered to be typed
 * text.
 */

#include "defs.h"

Prototype void init_command (void);
Prototype int do_command (char *);
Prototype void do_null (void);
Prototype void do_source (void);
Prototype void do_quit (void);
Prototype void do_execute (void);
Prototype void do_repeat (void);
Prototype char *breakout (char **, char *, char **);


typedef struct Process PROC;

#define CF_COK	1   /*	Can be executed while in command line mode	*/
#define CF_PAR	2   /*	ESCIMM special flag.. save rest of command line */
		    /*	so it can be executed after user entry		*/

#define CF_ICO	4   /*	OK to execute if iconified, else uniconify first*/

#define BTOCP(val, type)    ((type)((long)val << 2))

typedef void (*FPTR) ARGS((long));

typedef struct {
    const char *name;	 /* command name       */
    ubyte args;
    ubyte flags;
    void (*func) ARGS((long));	/* function	      */
} COMM;

/*============================================================================*/

/*
 *  WLEFT/WRIGHT will check command line mode themselves, and thus can
 *  be marked flags=1 even though they can change the line number.
 *
 *  No more than 255 commands may exist unless you change the type of hindex[]
 *
 *  Command names MUST be sorted by their first character
 */

static unsigned char hindex[26];    /*	alpha hash into table	*/

	/*	  args flags	*/

static const COMM Comm[] = {
#ifndef NO_DO2
    "addpath",       1, CF_COK, (FPTR)do_addpath,
#endif
    "append",        1, CF_COK, (FPTR)do_append,
    "arpinsfile",    0,      0, (FPTR)do_arpinsfile,
    "arpload",       0,      0, (FPTR)do_arpload,
    "arpsave",       0,      0, (FPTR)do_arpsave,
    "aslfont",       0,      0, (FPTR)do_aslfont,
    "aslinsfile",    0,      0, (FPTR)do_aslinsfile,
    "aslload",       0,      0, (FPTR)do_aslload,
    "aslsave",       0,      0, (FPTR)do_aslsave,
    "back",          0, CF_COK, (FPTR)do_bs,
    "backtab",       0, CF_COK, (FPTR)do_backtab,
    "bappend",       1, CF_COK, (FPTR)do_bappend,
    "bcopy",         0,      0, (FPTR)do_bcopy,
    "bdelete",       0,      0, (FPTR)do_bdelete,
    "bgpen",         1,      0, (FPTR)do_bgpen,
    "block",         0,      0, (FPTR)do_block,    /* checks com name for mode */
    "bmove",         0,      0, (FPTR)do_bmove,
    "bottom",        0,      0, (FPTR)do_bottom,
    "bs",            0, CF_COK, (FPTR)do_bs,
    "bsave",         1, CF_COK, (FPTR)do_bsave,
    "bsource",       0,      0, (FPTR)do_bsource,
    "cd",            1, CF_COK, (FPTR)do_cd,
    "chfilename",    1,      0, (FPTR)do_chfilename,
    "col",           1, CF_COK, (FPTR)do_col,
#ifndef NO_DO_CTAGS
    "ctags",         0, CF_ICO, (FPTR)do_ctags,
#endif
    "del",           0, CF_COK, (FPTR)do_del,
    "deline",        0,      0, (FPTR)do_deline,
    "down",          0,      0, (FPTR)do_down,
    "downadd",       0,      0, (FPTR)do_downadd,
    "esc",           0, CF_COK, (FPTR)do_esc,
    "escimm",        1, CF_PAR, (FPTR)do_esc,
    "execute",       1, CF_ICO, (FPTR)do_execute,
    "fgpen",         1,      0, (FPTR)do_fgpen,
    "find",          1,      0, (FPTR)do_find,     /* checks com name for mode */
    "findmatch",     0, CF_COK, (FPTR)do_findmatch,
    "findr",         2,      0, (FPTR)do_findr,    /* checks com name for mode */
    "findstr",       1, CF_COK, (FPTR)do_findstr,  /* checks com name for mode */
    "first",         0, CF_COK, (FPTR)do_firstcolumn,
    "firstnb",       0, CF_COK, (FPTR)do_firstnb,
    "goto",          1,      0, (FPTR)do_goto,
    "hgpen",         1,      0, (FPTR)do_hgpen,
    "iconify",       0, CF_ICO, (FPTR)do_iconify,
    "iconsave",      1, CF_COK, (FPTR)do_iconsave,
    "if",            2, CF_COK, (FPTR)do_if,
    "ifelse",        3, CF_COK, (FPTR)do_if,
    "ignorecase",    1, CF_COK, (FPTR)do_ignorecase,
    "insertmode",    1, CF_COK, (FPTR)do_insertmode,
    "insfile",       1,      0, (FPTR)do_edit,
    "insline",       0,      0, (FPTR)do_insline,
    "join",          0,      0, (FPTR)do_join,
    "justify",       1,      0, (FPTR)do_justify,
    "last",          0, CF_COK, (FPTR)do_lastcolumn,
    "left",          0, CF_COK, (FPTR)do_left,
    /*
    "lrow",          1, CF_COK, (FPTR)do_lrow,
    */
    "map",           2, CF_COK, (FPTR)do_map,
    "margin",        1, CF_COK, (FPTR)do_margin,
    "menuon",        0,      0, (FPTR)do_menuon,
    "menuoff",       0,      0, (FPTR)do_menuoff,
    "menuadd",       3,      0, (FPTR)do_menuadd,
    "menudel",       2,      0, (FPTR)do_menudel,
    "menudelhdr",    1,      0, (FPTR)do_menudelhdr,
    "menuclear",     0,      0, (FPTR)do_menuclear,
    "modified",      1,      0, (FPTR)do_modified,
    "newfile",       1,      0, (FPTR)do_edit,     /* checks com name for mode */
    "newwindow",     0, CF_ICO, (FPTR)do_newwindow,
    "next",          0,      0, (FPTR)do_find,
    "nextr",         0,      0, (FPTR)do_findr,
    "null",          0, CF_COK, (FPTR)do_null,
    "openwindow",    1, CF_ICO, (FPTR)do_openwindow,
    "pagedown",      0,      0, (FPTR)do_page,
    "pageset",       1,      0, (FPTR)do_page,
    "pageup",        0,      0, (FPTR)do_page,
    "ping",          1, CF_ICO, (FPTR)do_ping,
    "pong",          1,      0, (FPTR)do_pong,
    "prev",          0,      0, (FPTR)do_find,
    "prevr",         0,      0, (FPTR)do_findr,
    "popmark",       0,      0, (FPTR)do_popmark,
    "purgemark",     0,      0, (FPTR)do_purgemark,
    "pushmark",      0,      0, (FPTR)do_pushmark,
    "quit",          0, CF_ICO, (FPTR)do_quit,
    "recall",        0, CF_COK, (FPTR)do_recall,
#ifndef NO_DO_REFS
    "ref",           0,      0, (FPTR)do_refs,
#endif
    "reformat",      0,      0, (FPTR)do_reformat,
    "remeol",        0, CF_COK, (FPTR)do_remeol,
#ifndef NO_DO2
    "rempath",       1, CF_COK, (FPTR)do_rempath,
#endif
    "repeat",        2, CF_ICO|CF_COK, (FPTR)do_repeat,
    "repstr",        1, CF_COK, (FPTR)do_findstr,
    "resettoggle",   1, CF_COK, (FPTR)do_toggle,
    "resize",        2,      0, (FPTR)do_resize,
    "return",        0, CF_COK, (FPTR)do_return,   /* special meaning in command line mode */
    "right",         0, CF_COK, (FPTR)do_right,
#if AREXX
    "rx",            1,      0, (FPTR)do_rx,       /* explicit ARexx macro invocation      */
    "rx1",           2,      0, (FPTR)do_rx1,      /* explicit, with 1 arg  to ARexx macro */
    "rx2",           3,      0, (FPTR)do_rx2,      /* explicit, with 2 args to ARexx macro */
#endif
    "saveas",        1, CF_ICO|CF_COK, (FPTR)do_saveas,
    "saveconfig",    0, CF_ICO|CF_COK, (FPTR)do_saveconfig,
    "savemap",       1, CF_ICO|CF_COK, (FPTR)do_savemap,  /* checks com name for mode */
    "saveold",       0, CF_ICO|CF_COK, (FPTR)do_save,
    "savesmap",      1, CF_ICO|CF_COK, (FPTR)do_savemap,
    "savetabs",      1, CF_ICO|CF_COK, (FPTR)do_savetabs,
    "scanf",         1, CF_COK, (FPTR)do_scanf,
    "screenbottom",  0,      0, (FPTR)do_screenbottom,
    "screentop",     0,      0, (FPTR)do_screentop,
    "scrollup",      0,      0, (FPTR)do_scrollup,
    "scrolldown",    0,      0, (FPTR)do_scrolldown,
    "set",           2, CF_ICO|CF_COK, (FPTR)do_set,
    "setenv",        2, CF_ICO|CF_COK, (FPTR)do_setenv,
    "setfont",       2,      0, (FPTR)do_setfont,
    "setparcol",     1, CF_COK, (FPTR)do_setparcol,
    "settoggle",     1, CF_COK, (FPTR)do_toggle,
    "source",        1, CF_COK, (FPTR)do_source,
    "split",         0,      0, (FPTR)do_split,
    "swapmark",      0,      0, (FPTR)do_swapmark,
    "tab",           0, CF_COK, (FPTR)do_tab,
    "tabstop",       1, CF_COK, (FPTR)do_tabstop,
    "title",         1,      0, (FPTR)do_title,
    "tlate",         1, CF_COK, (FPTR)do_tlate,
    "toggle",        1, CF_COK, (FPTR)do_toggle,
    "tomouse",       0,      0, (FPTR)do_tomouse,
    "top",           0,      0, (FPTR)do_top,
    "tpen",          1,      0, (FPTR)do_tpen,
    "unblock",       0,      0, (FPTR)do_block,
    "undeline",      0,      0, (FPTR)do_undeline,
    "undo",          0,      0, (FPTR)do_undo,
    "unjustify",     0,      0, (FPTR)do_unjustify,
    "unmap",         1, CF_ICO|CF_COK, (FPTR)do_unmap,
    /*
    "urow",          1, CF_COK, (FPTR)do_urow,
    */
    "unset",         1, CF_ICO|CF_COK, (FPTR)do_unset,
    "unsetenv",      1, CF_ICO|CF_COK, (FPTR)do_unsetenv,
    "up",            0,      0, (FPTR)do_up,
    "while",         2, CF_ICO|CF_COK, (FPTR)do_if,
    "window",        1,      0, (FPTR)do_window,
    "wleft",         0, CF_COK, (FPTR)do_wleft,
    "wordwrap",      1, CF_COK, (FPTR)do_wordwrap,
    "wright",        0, CF_COK, (FPTR)do_wright,
    NULL, 0, 0, NULL
};

void
init_command()
{
    short hi;
    COMM *comm;

    hi = sizeof(Comm)/sizeof(Comm[0]) - 2;
    comm = Comm + hi;

    while (hi >= 0) {
	hindex[comm->name[0] - 'a'] = hi;
	--hi;
	--comm;
    }
}

#define MAXIA	5

do_command(str)
char *str;
{
    char *arg;
    char *aux1, *aux2;
    char *repstr[MAXIA];
    char quoted;
    short repi = 0;
    short i, j;
    static int level;

    if (++level > 20) {
	title("Recursion Too Deep!");
	--level;
#if AREXX
	foundcmd = 1;	/* to prevent us from trying an ARexx macro */
#endif
	return(0);
    }
    while (arg = breakout(&str, &quoted, &aux1)) {
	if (quoted) {
	    if (Ep->iconmode)
		uniconify();
	    text_write(arg);
#if AREXX				/* MMW 1.45 */
	    foundcmd = 1;		/* MMW 1.45 */
#endif					/* MMW 1.45 */
	    goto loop;
	}
	for (i = 0; arg[i]; ++i) {
	    if (arg[i] >= 'A' && arg[i] <= 'Z')
		arg[i] += 'a' - 'A';
	}

	if (arg[0] >= 'a' && arg[0] <= 'z') {
	    COMM *comm = &Comm[hindex[arg[0]-'a']];
	    for (; comm->name && comm->name[0] == arg[0]; ++comm) {
		if (strcmp(arg, comm->name) == 0) {
#if AREXX
		    foundcmd = 1;
#endif
		    av[0] = (ubyte *)comm->name;
		    for (j = 1; j <= comm->args; ++j) {
			av[j] = (ubyte *)breakout(&str, &quoted, &aux2);
			if (aux2) {
			    if (repi == MAXIA) {
				free(aux2);
				title("Command too complex");
				goto fail;
			    } else {
				repstr[repi++] = aux2;
			    }
			}
			if (!av[j]) {
			    title("Bad argument");
			    goto fail;
			}
		    }
		    av[j] = NULL;   /* end of arglist */
		    if ((comm->flags & CF_COK) || !Comlinemode) {
			if (comm->flags & CF_PAR) {
			    if (Partial)
				free(Partial);
			    Partial = (char *)malloc(strlen(str)+1);
			    strcpy(Partial, str);
			    str += strlen(str);     /*	skip string */
			}
			if (Ep->iconmode && !(comm->flags & CF_ICO))
			    uniconify();
			(*comm->func)(-1);
		    }
		    if (Abortcommand)
			goto fail;
		    goto loop;
		}
	    }
	}

	/* Command not found, check for macro	*/

	{
	    char *s2;
	    int ret;
	    if ((s2 = keyspectomacro(arg)) || (s2 = menutomacro(arg))) {
		s2 = (char *)strcpy(malloc(strlen(s2)+1), s2);
		ret = do_command(s2);
		free(s2);
#if AREXX
		if (ret) {
		    foundcmd = 1;   /* dunno about this yet for ARexx macros */
		    goto loop;
		}
#else
		if (ret)
		    goto loop;
#endif
		goto fail;
	    }
	}

	/* Command still not found, check for public macro  */
	/* code to be added */

#if AREXX
	do_rxImplied(arg, str);
#else
	title("Unknown Command");
#endif
fail:
	--level;
	while (--repi >= 0)
	    free(repstr[repi]);
	if (aux1)
	    free(aux1);
	return(0);
loop:
	if (aux1)
	    free(aux1);
    }
    --level;
    while (--repi >= 0)
	free(repstr[repi]);
    return(1);
}

void
do_null()
{
}

void
do_source()
{
    char buf[256];
    FILE *fi;
    char *str;
    BPTR oldlock = CurrentDir(DupLock((BPTR)Ep->dirlock));

    if (fi = fopen(av[1], "r")) {
	while (fgets(buf, 256, fi)) {
	    if (buf[0] == '#')
		continue;
	    for (str = buf; *str; ++str) {
		if (*str == 9)
		    *str = ' ';
	    }
	    if (str > buf && str[-1] == '\n')
		str[-1] = 0;
	    do_command(buf);
	}
	fclose(fi);
    } else {
	if (av[0])
	    title("File not found");
    }
    UnLock(CurrentDir(oldlock));
}


void
do_quit()
{
    extern char Quitflag;

    Quitflag = 1;
}

void
do_execute()
{
    BPTR oldlock = CurrentDir((BPTR)Ep->dirlock);

    system13(av[1]);
    CurrentDir(oldlock);
}

/*
 * repeat X command
 *
 * Since repeat takes up 512+ stack, it should not be nested more than
 * twice.
 *
 * (if X is not a number it can be abbr. with 2 chars)
 *
 * X =	N     -number of repeats
 *	line  -current line # (lines begin at 1)
 *	lbot  -#lines to the bottom, inc. current
 *	cleft -column # (columns begin at 0)
 *		(thus is also chars to the left)
 *	cright-#chars to eol, including current char
 *	tr    -#char positions to get to next tab stop
 *	tl    -#char positions to get to next backtab stop
 */

#define SC(a,b) ((a)<<8|(b))

void
do_repeat()
{
    ubyte *ptr = av[1];
    unsigned long n;
    char buf1[256];
    char buf2[256];

    breakreset();
    strcpy(buf1, av[2]);
    switch((ptr[0]<<8)+ptr[1]) {
    case SC('l','i'):
	n = text_lineno();
	break;
    case SC('l','b'):
	n = text_lines() - text_lineno() + 1;
	break;
    case SC('c','l'):
	n = text_colno();
	break;
    case SC('c','r'):
	n = text_cols() - text_colno();
	break;
    case SC('t','r'):
	n = text_tabsize()-(text_colno() % text_tabsize());
	break;
    case SC('t','l'):
	n = text_colno() % text_tabsize();
	if (n == 0)
	    n = text_tabsize();
	break;
    default:
	n = atoi(av[1]);
	break;
    }
    while (n > 0) {
	strcpy(buf2, buf1);
	if (do_command(buf2) == 0 || breakcheck()) {
	    Abortcommand = 1;
	    break;
	}
	--n;
    }
}

/*
 *  BREAKOUT()
 *
 *  Break out the next argument.  The argument is space delimited and
 *  might be quoted with `' or (), or single quoted as 'c or )c
 *
 *  Also:	$var	    -variable insertion
 *		^c	    -control character
 */

char *
breakout(ptr, quoted, paux)
char **ptr;
char **paux;
char *quoted;
{
    char *str = *ptr;
    char *base;
    short count;
    char opc;
    char clc;
    char immode;
    char isaux;
    char buf[256];
    short di;

    {
	short z = 0;

	count = z;
	opc   = z;
	clc   = z;
	immode= z;
	isaux = z;
	di    = z;
    }

    *quoted = 0;
    *paux = NULL;
    while (*str == ' ')
	++str;
    if (!*str)
	return(NULL);

    *ptr = str;
    base = str;
    while (*str) {
	if (immode) {
	    if (di != sizeof(buf)-1)
		buf[di++] = *str;
	    ++str;
	    continue;
	}
	if (count == 0) {
	    if (*str == ' ')
		break;
	    if (*str == '\'' || *str == ')')
		clc = *str;
	    if (*str == '`') {
		opc = '`';
		clc = '\'';
	    }
	    if (*str == '(') {
		opc = '(';
		clc = ')';
	    }
	}
	if (*str == opc) {
	    ++count;
	    if (str == *ptr) {
		*quoted = 1;
		base = ++str;
		continue;
	    }
	}
	if (*str == clc) {
	    --count;
	    if (count == 0 && *quoted)	   /*  end of argument	   */
		break;
	    if (str == *ptr && count < 0) {
		immode = 1;
		*quoted = 1;
		base = ++str;
		continue;
	    }
	}

	/*
	 *  $varname $(varname) $`varname'.  I.E. three forms are allowed,
	 *  which allows one to insert the string almost anywhere.  The
	 *  first form names are limited to alpha-numerics, '-', and '_'.
	 */

	if (*str == '$') {
	    char *p2;
	    char *tmpptr;
	    char c, ce;
	    short len;

	    ce = 0;			    /*	first form  */
	    ++str;			    /*	skip $	    */
	    if (*str == '(') {              /*  second form */
		ce = ')';
		++str;
	    } else if (*str == '`') {       /*  third form  */
		ce = '\'';
		++str;
	    }
	    p2 = str;			   /*  start of varname    */
	    if (ce) {			    /*	until end char OR   */
		while (*p2 && *p2 != ce)
		    ++p2;
	    } else {			    /*	smart end-varname   */
		while ((*p2 >= 'a' && *p2 <= 'z') ||
			(*p2 >= 'A' && *p2 <= 'Z') ||
			(*p2 >= '0' && *p2 <= '9') ||
			*p2 == '-' || *p2 == '_' ) {
		    ++p2;
		}
	    }
	    len = p2 - str;		   /*  length of variable  */

	    c = *p2; *p2 = 0;		  /*  temp. terminate \0  */
	    if (strcmp(str, "scanf") == 0) {
		*p2 = c;
		isaux = 1;
		if (di + strlen(String) < sizeof(buf)-1) {
		    strcpy(buf + di, String);
		    di += strlen(buf + di);
		}
		str += len;		    /*	next string pos     */
		if (ce)
		    ++str;
		continue;
	    }
	    if (strcmp(str, "fpath") == 0) {
		short i;
		for (i = strlen(Ep->Name); i >= 0; --i) {
		    if (Ep->Name[i] == ':' || Ep->Name[i] == '/')
			break;
		}
		++i;
		*p2 = c;
		isaux = 1;
		if (di + i < sizeof(buf)-1) {
		    movmem(Ep->Name, buf + di, i);
		    di += i;
		    buf[di] = 0;
		}
		str += len;
		if (ce)
		    ++str;
		continue;
	    }
	    if (strcmp(str, "fname") == 0) {
		short i;
		short j;
		for (i = strlen(Ep->Name); i >= 0; --i) {
		    if (Ep->Name[i] == ':' || Ep->Name[i] == '/')
			break;
		}
		++i;
		j = strlen(Ep->Name + i);
		*p2 = c;
		isaux = 1;
		if (di + j < sizeof(buf)-1) {
		    movmem(Ep->Name + i, buf + di, j);
		    di += j;
		    buf[di] = 0;
		}
		str += len;
		if (ce)
		    ++str;
		continue;
	    }
	    if (strcmp(str, "filename") == 0) {
		*p2 = c;
		isaux = 1;
		if (di + strlen(Ep->Name) < sizeof(buf)-1) {
		    strcpy(buf + di, Ep->Name);
		    di += strlen(buf + di);
		}
		str += len;
		if (ce)
		    ++str;
		continue;
	    }
	    if (strcmp(str, "colno") == 0) {
		*p2 = c;
		isaux = 1;
		if (di < sizeof(buf)-8) {
		    sprintf(buf + di, "%ld", Ep->Column + 1);
		    di += strlen(buf + di);
		}
		str += len;
		if (ce)
		    ++str;
		continue;
	    }
	    if (strcmp(str, "lineno") == 0) {
		*p2 = c;
		isaux = 1;
		if (di < sizeof(buf)-8) {
		    sprintf(buf + di, "%ld", Ep->Line + 1);
		    di += strlen(buf + di);
		}
		str += len;
		if (ce)
		    ++str;
		continue;
	    }
	    if (strcmp(str, "margin") == 0) {                       /* MMW */
		*p2 = c;
		isaux = 1;
		if (di < sizeof(buf)-8) {
		    sprintf(buf + di, "%ld", Ep->Margin);
		    di += strlen(buf + di);
		}
		str += len;
		if (ce)
		    ++str;
		continue;
	    }
	    if (strcmp(str, "modified") == 0) {                       /* MMW */
		*p2 = c;
		isaux = 1;
		if (di < sizeof(buf)-8) {
		    sprintf(buf + di, "%ld", Ep->Modified ? 1 : 0);
		    di += strlen(buf + di);
		}
		str += len;
		if (ce)
		    ++str;
		continue;
	    }
	    if (strcmp(str, "currentline") == 0) {
		short i;

		*p2 = c;
		isaux = 1;
		if (di < sizeof(buf)-8) {
		    for (i = strlen(Current) - 1; i >= 0 && Current[i] == ' '; --i);
		    ++i;
		    if (i > 0)
			strncpy(buf + di, Current, i);
		    *(buf + di + i) = 0;
		    di += strlen(buf + di);
		}
		str += len;
		if (ce)
		    ++str;
		continue;
	    }




	    if (tmpptr = getvar(str)) {
		p2 = tmpptr;
		str[len] = c;
		isaux = 1;
		if (di + strlen(p2) < sizeof(buf)-1) {
		    strcpy(buf + di, p2);
		    di += strlen(buf + di);
		}
		str += len;
		if (ce)
		    ++str;
		free(p2);
		continue;
	    }
	    *p2 = c;
	    --str;
	    if (ce)
		--str;
	}
	if (*str == '^' && (str[1] & 0x1F)) {
	    ++str;
	    *str &= 0x1F;
	    isaux = 1;
	}
	if (*str == '\\' && str[1]) {
	    ++str;
	    isaux = 1;
	}
	buf[di++] = *str++;
    }
    buf[di++] = 0;
    if (isaux) {
	*paux = malloc(di);
	strcpy(*paux, buf);
	base = *paux;
    }
    if (*str) { 	    /*	space ended */
	*str = '\0';
	*ptr = str + 1;     /*	next arg    */
    } else {
	*ptr = str;	    /*	last arg    */
    }
    return(base);
}


