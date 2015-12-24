/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  DEFINE.C
 *
 */

#include "defs.h"
#ifdef AMIGA
#include "/dc1/tokens.h"
#else
#include "../dc1/tokens.h"
#endif

Prototype void InitDefines(char *);
Prototype void ModifySymbolText(Sym *, short);
Prototype void do_undef(char *, int, int32_t *);
Prototype void do_define(char *, int, int32_t *);
Prototype int32_t PreliminaryReplace(char *, int32_t, char **, short *, short, char **);
Prototype Include * PrepareSymbolArgs(Sym *, char *, int32_t *, int32_t);
Prototype int32_t HandleSymbol(Sym *, char *, int32_t, int32_t);

#define  X_LINE 	    (SF_SPECIAL|0x100)
#define  X_DATE 	    (SF_SPECIAL|0x200)
#define  X_FILE 	    (SF_SPECIAL|0x300)
#define  X_BASE_FILE	    (SF_SPECIAL|0x400)
#define  X_TIME 	    (SF_SPECIAL|0x500)

static char SymBaseFileBuf[256];
static char SymFileBuf[256];
static char SymLineBuf[16];
static char SymDateBuf[64];
static char SymTimeBuf[64];

void
InitDefines(master)
char *master;
{
    time_t t;
    char *ct;
    struct tm *tp;

    time(&t);
    tp = localtime(&t);
    ct = ctime(&t);

    sprintf(SymBaseFileBuf, "\"%s\"", master);
    sprintf(SymDateBuf, "\"%.7s%.4s\"", ct + 4, ct + 20);
    sprintf(SymDateBuf + 32, "\"%d.%d.%d\"", tp->tm_mday, tp->tm_mon + 1, tp->tm_year % 100);
    sprintf(SymTimeBuf, "\"%.8s\"", ct + 11);

    if (StripOpt == 0) {
	DefineSimpleSymbol("__LINE__",      SymLineBuf, X_LINE);
	DefineSimpleSymbol("__DATE__",      SymDateBuf, 0);
	DefineSimpleSymbol("__TIME__",      SymTimeBuf, 0);
	DefineSimpleSymbol("__FILE__",      SymFileBuf, X_FILE);
	DefineSimpleSymbol("__BASE_FILE__", SymBaseFileBuf, 0);

	DefineSimpleSymbol("__COMMODORE_DATE__", SymDateBuf + 32, 0);

	if (UndefSyms == 0) {
	    DefineSimpleSymbol("__STDC__",  "1", 0);
	    DefineSimpleSymbol("mc68000",   "1", 0);
	    DefineSimpleSymbol("_DCC",      "1", 0);
	    DefineSimpleSymbol("AMIGA",     "1", 0);
	}
	if (FFPDef)
	    DefineSimpleSymbol("_FFP_FLOAT","1", 0);
	else
	    DefineSimpleSymbol("_SP_FLOAT", "1", 0);
    }
}

void
ModifySymbolText(Sym *sym, short type)
{
    Include *inc = GetNominalInclude(0);

    switch(type) {
    case X_FILE:
	if (inc->FileName)
	    sprintf(SymFileBuf, "\"%s\"", inc->FileName);
	else
	    sprintf(SymFileBuf, "\"<unknown>\"");
	sym->TextLen = strlen(sym->Text);
	break;
    case X_LINE:
	sprintf(SymLineBuf, "%d", inc->LineNo);
	sym->TextLen = strlen(SymLineBuf);
	break;
    }
}

void
do_undef(buf, max, pu)
char *buf;
int max;
int32_t *pu;   /*	unused	*/
{
    int32_t n = ExtSymbol(buf, 0, max);

    if (n)
	UndefSymbol(buf, n);
    else
	cerror(EERROR_UNDEF_NO_SYMBOL);
}

void
do_define(buf, max, pu)
char *buf;
int max;
int32_t *pu;   /*	unused	*/
{
    int i;
    int b;
    int symLen;
    int numArgs = -1;
    char *repBuf;
    int32_t repSize;
    short alloc = 0;
    static char *Args[MAX_ARGS];
    static short Lens[MAX_ARGS];

    symLen = ExtSymbol(buf, 0, max);	/*  extract symbol  */
    b = symLen;

    if (b < max && buf[b] == '(') {
	numArgs = 0;

	++b;
	while (b < max && buf[b] != ')') {
	    while (b < max && WhiteSpace[(ubyte)buf[b]])   /*	skip ws     */
		++b;
	    i = ExtSymbol(buf, b, max); 	    /*	symbol name */
	    if (i == b) {
		cerror(EERROR_ILLEGAL_NULL_SYMBOL);
		return;
	    }
	    if (numArgs == MAX_ARGS)
		cerror(EFATAL_MAX_MACRO_ARGS, MAX_ARGS);
	    Args[numArgs] = buf + b;
	    Lens[numArgs] = i - b;
	    ++numArgs;
	    dbprintf(("Macro Argument: %.*s\n", i -b, buf + b));

	    b = i;
	    while (b < max && WhiteSpace[(ubyte)buf[b]])
		++b;
	    if (b >= max || (buf[b] != ')' && buf[b] != ',')) {
		cerror(EERROR_EXPECTED_COMMA);
		return;
	    }
	    if (b < max && buf[b] == ',')
		++b;
	}
	if (b >= max || buf[b] != ')') {
	    cerror(EERROR_EXPECTED_CLOSE_PAREN);
	    return;
	}
	++b;
    }
    while (b < max && WhiteSpace[(ubyte)buf[b]])	/*  skip white space to macro body  */
	++b;

    /*
     *	Lens and Args are modified, a new buffer and buffer size is returned.
     *
     *	If a simple macro (no arguments), just tell DefineSymbol() to allocate
     *	a copy of the text buffer (which can go away since we free file buffers)
     */

    if (numArgs > 0) {
	repSize = PreliminaryReplace(buf + b, max - b, Args, Lens, numArgs, &repBuf);
    } else {
	repSize = max - b;
	repBuf	= buf + b;
	alloc = 1;
    }

    /*
     *	Define the symbol
     */

    DefineSymbol(buf, symLen, 0, numArgs, Args, Lens, repBuf, 1, alloc, repSize);
}

/*
 *  Do argument replace and create identifiers for the macro arguments that are
 *  not normal symbols.  "+%08lx"
 */

#define TS_SIZE 9

int32_t
PreliminaryReplace(
    char *src,
    int32_t max,
    char **args,
    short *lens,
    short n,
    char **pdst
) {
    int32_t i;
    int32_t di;
    int32_t bytes;
    short j;
    char *dst;
    static int32_t XId;	/*  replacement id. */
    static char Tmp[MAX_ARGS][16];

    for (j = 0; j < n; ++j)
	sprintf(Tmp[j], "\001%08x", XId + j);  /*  TS_SIZE */

    /*
     *	pass 1 : find symbols to replace and figure out the size of the new
     *		 buffer.
     */

    bytes = max;
    for (i = 0; i < max; ) {
	ubyte c = (ubyte)src[i];

	if ((c < '0' || c > '9') && SymbolChar[c]) {
	    int32_t l = ExtSymbol(src, i, max);
	    short len = l - i;

	    for (j = 0; j < n; ++j) {
		if (lens[j] == len && strncmp(args[j], src + i, len) == 0) {
		    bytes -= len;
		    bytes += TS_SIZE;
		    dbprintf(("MATCH %d\n", len));
		    break;
		}
	    }
	    i = l;
	} else if (c == '\"') {
	    i = SkipString(src, i + 1, max);
	} else if (c == '\'') {
	    i = SkipSingleSpec(src, i + 1, max);
	} else {
	    ++i;
	}
    }

    /*
     *	pass 2, do replacement
     */

    dbprintf(("replace buffer: %ld/%ld\n", max, bytes));

    if (bytes < 16)
	dst = zalloc(bytes+1);	    /*	efficiency only     */
    else
	dst = malloc(bytes+1);	    /*	+1 necessary? don't think so... */
    if (dst == NULL)
	ErrorNoMemory();

    for (i = di = 0; i < max && di < bytes;) {
	ubyte c = (ubyte)src[i];

	if ((c < '0' || c > '9') && SymbolChar[c]) {
	    int32_t l = ExtSymbol(src, i, max);
	    short len = l - i;

	    for (j = 0; j < n; ++j) {
		if (lens[j] == len && strncmp(args[j], src + i, len) == 0) {
		    dbprintf(("MATCH %d\n", len));
		    break;
		}
	    }
	    if (j == n) {   /*	not a replace symbol	*/
		movmem(src + i, dst + di, len);
		i += len;
		di += len;
	    } else {	    /*	is a replace symbol	*/
		movmem(Tmp[j], dst + di, TS_SIZE);
		i += len;
		di += TS_SIZE;
		dbprintf(("match len %d (%d/%d)\n", len, j, n));
	    }
	} else if (c == '\"') {
	    int32_t nn = i;
	    i = SkipString(src, i + 1, max);
	    movmem(src + nn, dst + di, i - nn);
	    di += i - nn;
	} else if (c == '\'') {
	    int32_t nn = i;
	    i = SkipSingleSpec(src, i + 1, max);
	    movmem(src + nn, dst + di, i - nn);
	    di += i - nn;
	} else {
	    dst[di++] = c;
	    ++i;
	}
    }
    if (i != max || di != bytes)
	cerror(EFATAL_SOFT_ERROR_REPLACE, i, max, di, bytes);

    XId += n;

    for (j = 0; j < n; ++j) {
	lens[j] = TS_SIZE;
	args[j] = Tmp[j];
    }

    *pdst = dst;
    return(bytes);
}

Include *PrepareSymbolArgs(sym, base, ip, max)
Sym *sym;
char *base;
int32_t *ip;
int32_t max;  /* Set it negative to avoid doing any dumps */
{
    short oldType = sym->Type;
    Include *push = PushBase;
    int32_t i  = *ip;
    /* int32_t oi = i;	**  position in original pushbase (not used)   */
    int32_t xi = i;	/*  position in followed pushbase   */
    short candump = 1;

    if (max < 0)
    {
       max = -max;
       candump = 1;
    }

    if ((sym->Type & SF_SPECIAL) == 0) {
	if (sym->Type & SF_RECURSE) {
	    cerror(EERROR_RECURSIVE_MACRO, sym->SymName);
	    push->Index = i;
	    return(NULL);
	}
	sym->Type |= SF_RECURSE;
    }

    if (sym->NumArgs >= 0) {	/*  overload symbol args symbols    */
	short j;
				/*  look for '('    */
	for (;;) {
	    while (i < max && WhiteSpace[(ubyte)base[i]])
		++i;
	    if (i >= max || base[i] != '\n')
		break;
	    ++PushBase->LineNo;
	    ForceLineSpec = 1;
	    ++i;
	}
	while (i >= max) {
	    push->Index = i;
	    if (push->IsFile)
		cerror(EWARN_MACRO_CROSSES_INCLUDE);
	    if ((push = push->Next) == NULL) {
		cerror(EERROR_UNEXPECTED_EOF_MACRO, sym->SymName);
		sym->Type = oldType;
		push->Index = i;
		return(NULL);
	    }
	    xi = i = push->Index;
	    max = push->MaxIndex;
	    base = push->Base;
	    dbprintf(("SKIPBACK %ld/%ld %08lx\n", i, max, (uint32_t)base));
	}

	/*
	 *  If no open-paren found then not a macro!
	 *
	 *  #define fubar(a,b)	hi!
	 *  ... 	int fubar;
	 *
	 */

	if (base[i] != '(') {
	    sym->Type = oldType;
	    if (push)
		push->Index = i;
	    if (candump)
	    {
		Dump(sym->SymName, 0, sym->SymLen);
		Dump(base, xi, i);
	    }
	    if (push)
		push->Index = i;
	    *ip = PushBase->Index;
	    return(NULL);
	}
	++i;

	/*
	 *  scan macro arguments.  Macro arguments are valid only at the
	 *  very next level, determined by setting
	 */

	for (j = 0; j < sym->NumArgs; ++j) {
	    short parens = 0;	/*  paren level 	*/
	    int32_t b;		/*  base of argument	*/

	    while (i < max && WhiteSpace[(ubyte)base[i]])
		++i;
	    b = i;

	    while (i < max) {
		short c = base[i];

		if (c == ',' && parens == 0)
		    break;
		if (c == '(') {
		    ++parens;
		    ++i;
		} else if (c == ')') {
		    if (parens == 0)
			break;
		    --parens;
		    ++i;
		} else if (c == '\"') {
		    i = SkipString(base, i + 1, max);
		} else if (c == '\'') {
		    i = SkipSingleSpec(base, i + 1, max);
		} else if (c == '\n') {
		    if (push)
			++push->LineNo;
		    ForceLineSpec = 1;
		    ++i;
		} else if (c == '/' && base[i+1] == '*') {
		    i = SkipComment(base, i + 2, max);
		} else {
		    ++i;
		}
	    }
	    dbprintf(("break-out %ld %c\n", i, base[i]));

	    {
		Sym *asym;
		asym = DefineSymbol(
			    sym->Args[j],
			    sym->ArgsLen[j],
			    SF_MACROARG,
			    -1,
			    NULL, NULL, base + b, 0, 0, i - b
			);
		asym->Creator = sym;
	    }

	    while (push && i >= max) {
		if (push->IsFile)
		    cerror(EWARN_MACRO_CROSSES_INCLUDE);
		push->Index = i;
		if ((push = push->Next) == NULL)
		    break;
		i = push->Index;
		max = push->MaxIndex;
		base = push->Base;
	    }

	    if (i < max && base[i] == ',') {
		++i;
		continue;
	    }

	    while (push && i >= max) {
		push->Index = i;
		if ((push = push->Next) == NULL)
		    break;
		i = push->Index;
		max = push->MaxIndex;
		base = push->Base;
	    }

	    if (i >= max || base[i] != ')') {
		cerror(EERROR_EXPECTED_COMMA_CLOSE);
		break;
	    }
	}
	if (j != sym->NumArgs) {
	    cerror(EERROR_NOT_ENOUGH_ARGS_MACRO, j, sym->NumArgs);
	    while (--j >= 0)
		UndefSymbol(sym->Args[j], sym->ArgsLen[j]);
	    sym->Type = oldType;
	    if (push)
		push->Index = i;
	    return(NULL);
	}
	if (base[i] != ')') {
	    cerror(EERROR_EXPECTED_CLOSE_PAREN_ARG, sym->SymName);
	} else {
	    ++i;
	}
    }
    if (push)
	push->Index = i;
    *ip = i;
    return(push);
}

/*
 *  Handle special symbol.  Replacement stuff has been stripped of comments.
 *  Must rescan to handle other macros and such.
 *
 *	symbol		-> replace-stuff    sym->NumArgs < 0
 *	symbol(args)	-> replace-macro    sym->NumArgs >= 0
 *
 *  Must understand parenthesis...
 *
 *  NOTE:   In scanning a macro if we reach EOF and expect an open paren we
 *	    may go up a cpp level to continue the scan.
 *
 *  #define dec(var) ((var)--)
 *  #define apply(func) (func(Glob))
 *  apply(dec);
 *
 *	    This can get tricky.  Basically, we pull the rug from under
 *	    previous levels of CPP().  Thus, CPP() must be very careful in
 *	    this regard.
 *
 *  #define GAG(a,b)	(a,b)
 *  GAG(GAG(1,2),3)
 *
 *	    Theoretically the macro arguments are supposed to be evaulated
 *	    first to allow this type of nesting.  But, we evaluate bottom
 *	    up!  To make this work right when a lower level macro's args
 *	    are evaulated the higher level macro names must be RE-ENABLED!
 *
 *
 *  This routine must return in index relative to the original PushBase
 *  regardless of any data it pulls from higher level pushbase's
 */

int32_t
HandleSymbol(sym, base, i, max)
Sym *sym;
char *base;
int32_t i;
int32_t max;
{
    Include *push;
    short oldType = sym->Type;
    short creType = 0;
    short save_fls;

    {
       int32_t it = i;
       push = PrepareSymbolArgs(sym, base, &it, max);
       if (push == NULL) return(it);
    }

    /*
     *	If pushing a macro argument for replace we must enable the macro
     *	it came from.  This allows MAX(MAX(1,2),3) while disallowing loop
     *	conditions, i.e.    #define BAR(x)  BAR(1,x)
     */

    if (sym->Type & SF_MACROARG)
    {
	if (sym->Creator == NULL)
	    cerror(EFATAL_SOFTWARE_ERROR_CREATOR);
	creType = sym->Creator->Type;
	sym->Creator->Type &= ~SF_RECURSE;
    }

    if (sym->Type & SF_STRINGIZE) {
	++GlobalStringize;
	fputc('\"', Fo);
    }

    save_fls = ForceLineSpec;
    ForceLineSpec = 0;

    PushBase->Index = cpp(PushBase->Index, PushBase->Level + 1,
			  sym->SymName, NULL, sym->Text, sym->TextLen);

    ForceLineSpec = save_fls;

    i = push->Index;

    if (sym->Type & SF_STRINGIZE) {
	--GlobalStringize;
	fputc('\"', Fo);
    }

    if (sym->Type & SF_MACROARG)
	sym->Creator->Type = creType;

    dbprintf(("(macro-return)"));

    if (sym->NumArgs >= 0) {	/*  undef symbol args symbols	    */
	short j;
	for (j = 0; j < sym->NumArgs; ++j) {
	    if (UndefSymbol(sym->Args[j], sym->ArgsLen[j]) == 0)
		cerror(EFATAL_SOFTWARE_ERROR_MACRO, sym->SymName, sym->Args[j]);
	}
    }
    sym->Type = oldType;
    push->Index = i;
    return(PushBase->Index);
}
