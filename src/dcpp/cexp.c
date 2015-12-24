/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  CEXP.C
 *
 */

#include "defs.h"

Prototype long ParseIfExp(char *, short *, long, short);
Prototype void PushOp(short, short, short);
Prototype int TopOfOpStack(void);
Prototype int SecondOffOpStack(void);
Prototype void PushAtom(long, short);

Local int CombineOp(void);
Local int GetAtomStack(short *, long *);
Local int ParseCharConst(char *, long, long, long *);
Local int ParseInt(char *, long, long, long *);
Local int HexDig(char);
Local int OctDig(char);

#define MAXATOM     128
#define MAXOPER     128

#define CGTEQ	256
#define CGTGT	257
#define CLTEQ	258
#define CLTLT	259
#define CNOTEQ	260
#define CEQEQ	261
#define CSYM	262
#define CUMI	263

#define CDEFINED    264
#define CANDAND     265
#define COROR	    266

#define QBIN	4	    /*	binary operator, else unary */
#define LR	1	    /*	left to right, else	    */
#define RL	2	    /*	right to left		    */
#define BLR	(LR|QBIN)
#define BRL	(RL|QBIN)
#define XX	0

typedef struct Atom {
    long    Value;
    short   Undef;
    short   Reserved;
} Atom;

typedef struct Oper {
    short   Token;
    short   Pri;
    short   Type;
    short   Reserved;
} Oper;

static Atom AtomStack[MAXATOM];
static Oper OperStack[MAXOPER];
static short AtomIdx;
static short OperIdx;
static short BaseAtomIdx;
static short BaseOperIdx;

long
ParseIfExp(char *buf, short *pundef, long max, short subsym)
{
    short unary = 1;
    short i = 0;
    short parens = 0;
    short baseOperIdx;
    short baseAtomIdx;

    baseOperIdx = BaseOperIdx = OperIdx;
    baseAtomIdx = BaseAtomIdx = AtomIdx;

    while (i < max) {
	short c = buf[i];

	++i;
	if (c == ' ' || c == 9 || c == '\n')
	    continue;

	if (unary) {	/*  int, sym, !exp, ~exp (exp)	*/
	    switch(c) {
	    case '!':
	    case '~':
		PushOp(c, 140, RL);
		break;
	    case '-':
		PushOp(CUMI, 140, RL);
		break;
	    case '(':
		++parens;
		PushOp(c, 0, XX);
		break;
	    default:
		if (c >= '0' && c <= '9') {
		    long v;
		    /*
		    printf("%d(%c) ", i - 1, buf[i-1]);
		    */
		    i = ParseInt(buf, i - 1, max, &v);
		    /*
		    printf("%d INT %d\n", i, v);
		    */
		    PushAtom(v, 0);
		    unary = 0;

		    break;
		}
		/*
		 *  NOTE, handles 'defined' by pushing unary op instead
		 *	  of atom.
		 */
		if (SymbolChar[c]) {	/*  SymbolChar[] includes '0'-'9'   */
		    short ni;

		    --i;
		    ni = ExtSymbol(buf, i, max);
		    if (ni - i == 7 && strncmp(buf + i, "defined", 7) == 0) {
			PushOp(CDEFINED, 140, RL);
		    } else if (TopOfOpStack() == CDEFINED || SecondOffOpStack() == CDEFINED) {
			Sym *sym = FindSymbol(buf + i, ni - i);
			if (sym)
			    PushAtom(1, 0);
			else
			    PushAtom(0, 1);
			unary = 0;
		    } else {
			Sym *sym = FindSymbol(buf + i, ni - i);

			if (sym) {
			    short xundef;
			    long v;
			    if (subsym == 0) {
				v = 0;
				xundef = 0;
			    } else {
			    	/* We need to actually parse the symbol and then  */
			    	/* evaluate any arguments.  This code closely     */
			    	/* mimics the code found in HandleSymbol, but it  */
			    	/* has the additional requirement of not actually */
			    	/* sending anything out to the output file.       */
				short oldType = sym->Type;
				short creType = 0;

				{
				    long it = ni;
				    /* The -max is so that we can flag to the */
				    /* routine to not output anything         */
				    if (PrepareSymbolArgs(sym, buf, &it, -max) == NULL)
					goto syntax;
				    ni = it;
				}
				/*
				 * If pushing a macro argument for replace we must
				 * enable the macro it came from.  This allows
				 * MAX(MAX(1,2),3) while disallowing loop conditions,
				 * i.e.    #define BAR(x)  BAR(1,x)
				*/

				if (sym->Type & SF_MACROARG) {
				    if (sym->Creator == NULL)
					cerror(EFATAL_SOFTWARE_ERROR_CREATOR);
				    creType = sym->Creator->Type;
				    sym->Creator->Type &= ~SF_RECURSE;
				}

				v = ParseIfExp(sym->Text, &xundef, sym->TextLen, 1);

				if (sym->Type & SF_MACROARG)
				    sym->Creator->Type = creType;

				if (sym->NumArgs >= 0)
				{	/*  undef symbol args symbols	    */
				    short j;
				    for (j = 0; j < sym->NumArgs; ++j)
				    {
					if (UndefSymbol(sym->Args[j], sym->ArgsLen[j]) == 0)
					    cerror(EFATAL_SOFTWARE_ERROR_MACRO,
					    	   sym->SymName, sym->Args[j]);
				    }
				}
				sym->Type = oldType;

				BaseOperIdx = baseOperIdx;
				BaseAtomIdx = baseAtomIdx;
			    }
			    PushAtom(v, xundef);
			} else {
			    PushAtom(0, 1);
			}
			unary = 0;
		    }
		    i = ni;
		    break;
		}
		if (c == '\'') {
		    long v;
		    i = ParseCharConst(buf, i - 1, max, &v);
		    PushAtom(v, 0);
		    unary = 0;
		    break;
		}
syntax:
		cerror(EERROR_SYNTAX_ERROR);
syntax2:
		*pundef = 1;
		AtomIdx = baseAtomIdx;
		OperIdx = baseOperIdx;
		return(0);
	    }
	} else {	/*  + - * / % & | ^ <= << < == != > >> >=   */
	    unary = 1;

	    switch(c) {
	    case '+':
	    case '-':
		PushOp(c, 120, BLR);
		break;
	    case '*':
	    case '/':
	    case '%':
		PushOp(c, 130, BLR);
		break;
	    case '&':
		if (i < max && buf[i] == '&') {
		    PushOp(CANDAND, 50, BLR);
		    ++i;
		} else {
		    PushOp(c, 60, BLR);
		}
		break;
	    case '^':
		PushOp(c, 70, BLR);
		break;
	    case '|':
		if (i < max && buf[i] == '|') {
		    ++i;
		    PushOp(COROR, 40, BLR);
		} else {
		    PushOp(c, 80, BLR);
		}
		break;
	    case '<':
		if (i < max) {
		    short d = buf[i];
		    if (d == '<') {
			PushOp(CLTLT, 110, BLR);
			++i;
			break;
		    }
		    if (d == '=') {
			PushOp(CLTEQ, 100, BLR);
			++i;
			break;
		    }
		}
		PushOp(c, 100, BLR);
		break;
	    case '=':
		if (i >= max || buf[i] != '=')
		    goto syntax;
		PushOp(CEQEQ, 90, BLR);
		++i;
		break;
	    case '!':
		if (i >= max || buf[i] != '=')
		    goto syntax;
		PushOp(CNOTEQ, 90, BLR);
		++i;
		break;
	    case '>':
		if (i < max) {
		    short d = buf[i];
		    if (d == '>') {
			PushOp(CGTGT, 110, BLR);
			++i;
			break;
		    }
		    if (d == '=') {
			PushOp(CGTEQ, 100, BLR);
			++i;
			break;
		    }
		}
		PushOp(c, 100, BLR);
		break;
	    case ')':
		unary = 0;
		if (parens == 0) {
		    cerror(EERROR_TOO_MANY_CLOSE_PARENS);
		    goto syntax2;
		}
		--parens;
		while ((c = TopOfOpStack()) && c != '(')
		    CombineOp();
		if (c == 0)
		    goto syntax;
		CombineOp();
		break;
	    default:
		goto syntax;
	    }
	}
    }
    while (TopOfOpStack() >= 0 && CombineOp() >= 0);

    if (TopOfOpStack() >= 0)
	goto syntax;

    {
	long v;

	if (GetAtomStack(pundef, &v) < 0)
	    goto syntax;
	if (GetAtomStack(NULL, NULL) >= 0)  /*	shouldn't be anything left */
	    goto syntax;
	AtomIdx = baseAtomIdx;
	OperIdx = baseOperIdx;
	dbprintf(("RESULT %ld %d\n", v, *pundef));
	return(v);
    }
}

void
PushOp(short token, short pri, short type)
{
    if (pri) {
	while (OperIdx > BaseOperIdx) {
	    Oper *op = OperStack + OperIdx - 1;
	    if (pri > op->Pri)
		break;
	    if (pri == op->Pri && (type & RL))
		break;
	    if (CombineOp() < 0) {
		cerror(EERROR_SYNTAX_ERROR);
		break;
	    }
	}
    }
    if (OperIdx == MAXOPER) {
	cerror(EERROR_EXPRESSION_TOO_COMPLEX);
	return;
    }
    {
	Oper *op = OperStack + OperIdx++;

	op->Pri = pri;
	op->Token = token;
	op->Type = type;
    }
}

int
TopOfOpStack()
{
    if (OperIdx <= BaseOperIdx)
	return(-1);
    return((int)OperStack[OperIdx-1].Token);
}

int
SecondOffOpStack()
{
    if (OperIdx - 1 <= BaseOperIdx)
	return(-1);
    return((int)OperStack[OperIdx-2].Token);
}


void
PushAtom(long val, short isundef)
{
    Atom *atom;

    if (AtomIdx == MAXATOM) {
	cerror(EERROR_EXPRESSION_TOO_COMPLEX);
	return;
    }
    atom = AtomStack + AtomIdx++;
    atom->Value = val;
    atom->Undef = isundef;
}

int
CombineOp()
{
    Oper *op;
    Atom *a1, *a2 = NULL;
    Atom ar;

    if (OperIdx <= BaseOperIdx)
	return(-1);
    op = OperStack + --OperIdx;

    if (AtomIdx <= BaseAtomIdx)
	return(-1);
    a1 = AtomStack + --AtomIdx;

    if (op->Type & QBIN) {
	if (AtomIdx <= BaseAtomIdx)
	    return(-1);
	a2 = AtomStack + --AtomIdx;
    }

    if (op->Type & QBIN) {
	/*
	 * arguments treated as 0 if undefined.  Result is defined
	 */
#if 0
	ar.Undef = a1->Undef | a2->Undef;
#endif
	ar.Undef = 0;

	switch(op->Token) {
	case '+':
	    ar.Value = a2->Value + a1->Value;
	    break;
	case '-':
	    ar.Value = a2->Value - a1->Value;
	    break;
	case '*':
	    ar.Value = a2->Value * a1->Value;
	    break;
	case '/':
	    if (a1->Value)
		ar.Value = a2->Value / a1->Value;
	    else
		ar.Undef = 1;
	    break;
	case '%':
	    if (a1->Value)
		ar.Value = a2->Value % a1->Value;
	    else
		ar.Undef = 1;
	    break;
	case '&':
	    ar.Value = a2->Value & a1->Value;
	    break;
	case '|':
	    ar.Value = a2->Value | a1->Value;
	    break;
	case '^':
	    ar.Value = a2->Value ^ a1->Value;
	    break;
	case '<':
	    ar.Value = a2->Value < a1->Value;
	    break;
	case CLTLT:
	    ar.Value = a2->Value << a1->Value;
	    break;
	case CLTEQ:
	    ar.Value = a2->Value <= a1->Value;
	    break;
	case CEQEQ:
	    ar.Value = a2->Value == a1->Value;
	    break;
	case CNOTEQ:
	    ar.Value = a2->Value != a1->Value;
	    break;
	case '>':
	    ar.Value = a2->Value > a1->Value;
	    break;
	case CGTGT:
	    ar.Value = a2->Value >> a1->Value;
	    break;
	case CGTEQ:
	    ar.Value = a2->Value >= a1->Value;
	    break;
	case CANDAND:
	    if (a2->Value && a1->Value)
		ar.Value = 1;
	    else
		ar.Value = 0;
	    break;
	case COROR:
	    if (a2->Undef == 0 && a2->Value) {
		ar.Value = 1;
		ar.Undef = 0;
	    } else if (a1->Undef == 0 && a1->Value) {
		ar.Value = 1;
		ar.Undef = 0;
	    } else {
		ar.Value = 0;
	    }
	    break;
	default:
	    cerror(EFATAL_ERROR_PARSING_EXP);
	    break;
	}
    } else {
	/*
	 * arguments treated as 0 if undefined.  Result is defined
	 */
#if 0
	ar.Undef = a1->Undef;
#endif
	ar.Undef = 0;

	switch(op->Token) {
	case '!':
	    ar.Value = !a1->Value;
	    break;
	case '~':
	    ar.Value = ~a1->Value;
	    break;
	case CUMI:
	    ar.Value = -a1->Value;
	    break;
	case '(':
	    ar.Value = a1->Value;
	    ar.Undef = a1->Undef;
	    break;
	case CDEFINED:
	    ar.Undef = 0;
	    if (a1->Undef)
		ar.Value = 0;
	    else
		ar.Value = 1;
	    break;
	default:
	    cerror(EFATAL_ERROR_PARSING_EXP);
	    break;
	}
    }
    AtomStack[AtomIdx++] = ar;
    dbprintf(("op %c %03x result %ld (%d)\n", op->Token, op->Token, ar.Value, ar.Undef));
    return(0);
}

int
GetAtomStack(pundef, pv)
short *pundef;
long *pv;
{
    Atom *at;

    if (AtomIdx <= BaseAtomIdx) {
	if (pundef)
	    *pundef = 1;
	return(-1);
    }
    at = AtomStack + --AtomIdx;
    if (pundef)
	*pundef = at->Undef;
    if (pv)
	*pv = at->Value;
    return(0);
}

int
ParseCharConst(buf, i, max, pv)
char *buf;
long i;
long max;
long *pv;
{
    ubyte c;
    long v = 0;

    if (buf[i] != '\'')
	return(0);
    ++i;
    while (i < max && (c = buf[i]) != '\'') {
	++i;
	if (c == '\\' && i < max) {
	    c = buf[i++];
	    switch(c) {
	    case 'a':
		c = 7;
		break;
	    case 'b':
		c = 8;
		break;
	    case 'f':
		c = 'f'&0x1F;
		break;
	    case 'n':
		c = 10;
		break;
	    case 'r':
		c = 13;
		break;
	    case 't':
		c = 9;
		break;
	    case 'v':
		c = 11;
		break;
	    case 'x':
		{
		    short n;
		    short r = 0;
		    short cnt = 2;

		    while (cnt && (n = HexDig(c)) > 0) {
			r = (r << 4) + n;
			c = buf[i++];
			--cnt;
		    }
		    if (cnt)
			--i;
		    c = r;
		}
		break;
	    case '0':
	    case '1':
	    case '2':
	    case '3':
	    case '4':
	    case '5':
	    case '6':
	    case '7':
	    case '8':
	    case '9':
		{
		    short cnt;
		    short r;
		    short n;

		    for (r = cnt = 0; cnt < 3 && (n = OctDig(c)) >= 0; ++cnt) {
			r = (r << 3) + n;
			c = buf[i++];
		    }
		    --i;
		    c = r;
		}
		break;
	    default:
		c = buf[i];
		break;
	    }
	}
	v = (v << 8) | c;
    }
    *pv = v;
    if (i < max)    /*	skip closing single quote   */
	++i;
    else
	cerror(EERROR_UNTERMINATED_CHARCONST);
    return(i);
}

/*
 *  0xhex
 *  0octal
 *  1-9decimal
 */

int
ParseInt(buf, i, max, pv)
char *buf;
long i;
long max;
long *pv;
{
    char c;
    long v = 0;

    if (i < max && buf[i] == '0') {
	++i;
	if (i < max && (buf[i] == 'x' || buf[i] == 'X')) {   /*  hex */
	    short n;
	    ++i;
	    while (i < max && (n = HexDig(buf[i])) >= 0) {
		v = (v << 4) + n;
		++i;
	    }
	} else {					     /*  oct */
	    short n;
	    while (i < max && (n = OctDig(buf[i])) >= 0) {
		v = (v << 3) + n;
		++i;
	    }
	}
    } else {
	while (i < max && (c = buf[i]) >= '0' && c <= '9') { /*  dec */
	    v = v * 10 + c - '0';
	    ++i;
	}
    }
    while ((c = buf[i]) == 'L' || c == 'l' || c == 'U' || c == 'u')
	++i;
    *pv = v;
    return(i);
}

int
HexDig(char c)
{
    if (c >= '0' && c <= '9')
	return (c - '0');
    if (c >= 'a' && c <= 'f')
	c = c + ('A' - 'a');
    if (c >= 'A' && c <= 'F')
	return (c + (10 - 'A'));
    return(-1);
}

int
OctDig(char c)
{
    if (c >= '0' && c <= '7')
	return (c - '0');
    return(-1);
}

