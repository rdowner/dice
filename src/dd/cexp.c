/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */
/*
 *  CEXP.C
 */
#include        "defs.h"
#include        "dbug_protos.h"


Prototype long          ParseExp(char *, short *, long);

Prototype void          PushOp(short, short, short);
Prototype int           TopOfOpStack(void);
Prototype int           SecondOffOpStack(void);
Prototype void          PushAtom(long, short);

Local int               CombineOp(void);
Local int               GetAtomStack(short *, long *);
Local int               ParseCharConst(char *, long, long, long *);
Local int               ParseInt(char *, long, long, long *);
Local int               HexDig(char);
Local int               OctDig(char);
Local short             SymbolChar(short);
Local long              ExtSymbol(unsigned char *, long, long);


#define MAXATOM     128
#define MAXOPER     128

#define CGTEQ   256
#define CGTGT   257
#define CLTEQ   258
#define CLTLT   259
#define CNOTEQ  260
#define CEQEQ   261
#define CSYM    262
#define CUMI    263
#define CUIND   264

#define CDEFINED    280
#define CANDAND     281
#define COROR       282

#define QBIN    4           /*  binary operator, else unary */
#define LR      1           /*  left to right, else         */
#define RL      2           /*  right to left               */
#define BLR     (LR|QBIN)
#define BRL     (RL|QBIN)
#define XX      0

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

long ParseExp(char *buf, short *pundef, long max)
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

        if (unary) {    /*  int, sym, !exp, ~exp (exp), *exp  */
            switch(c) {
            case '*':   /*  indirection */
                PushOp(CUIND, 140, RL);
                break;
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
                    i = ParseInt(buf, i - 1, max, &v);
                    PushAtom(v, 0);
                    unary = 0;
                    break;
                }

                if (SymbolChar(c)) {
                        short   ni;
                        long    v;
                        long    *rptr;

                        --i;
                        ni = ExtSymbol(buf, i, max);    // extract symbol
                        if (i + 2 == ni && (rptr = RegisterAddress(buf + i))) {
                                PushAtom(*rptr, 0);
                                unary = 0;
                                i = ni;
                                break;
                        } 
                        else if (LookupSymLen(buf + i, ni-i, &v)) {
                                PushAtom(v, 0);
                                unary = 0;
                                i = ni;
                                break;
                        }

                        else if (LookupSymLen(addscore(buf + i), ni-i+1, &v)) {
                                PushAtom(v, 0);
                                unary = 0;
                                i = ni;
                                break;
                        }

                        else if (LookupSymLen(addat(buf + i), ni-i+1, &v)) {
                                PushAtom(v, 0);
                                unary = 0;
                                i = ni;
                                break;
                        }

                        i++;
                }

#ifdef MYKE_REMOVED_THIS
                /*
                 *  NOTE, handles 'defined' by pushing unary op instead
                 *        of atom.
                 */

                if (SymbolChar(c)) {    /*  SymbolChar() includes '0'-'9'   */
                    short ni;

                    --i;
                    ni = ExtSymbol(buf, i, max);    /*  extract symbol */
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

                            v = ParseExp(sym->Text, &xundef, sym->TextLen);
                            BaseOperIdx = baseOperIdx;
                            BaseAtomIdx = baseAtomIdx;
                            PushAtom(v, xundef);
                        } else {
                            PushAtom(0, 1);
                        }
                        unary = 0;
                    }
                    i = ni;
                    break;
                }
#endif
                if (c == '\'') {
                    long v;
                    i = ParseCharConst(buf, i - 1, max, &v);
                    PushAtom(v, 0);
                    unary = 0;
                    break;
                }
syntax:
                ScrStatus(("*** syntax error"));
syntax2:
                *pundef = 1;
                AtomIdx = baseAtomIdx;
                OperIdx = baseOperIdx;
                return(0);
            }
        }
        else {  /*  + - * / % & | ^ <= << < == != > >> >=   */
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
                    ScrStatus(("*** Too many close parens"));
                    goto syntax2;
                }
                --parens;
                while ((c = TopOfOpStack()) && c != '(')
                    CombineOp();
                if (c == 0)goto syntax;
                CombineOp();
                break;
            default:
                goto syntax;
            }
        }
    }
    while (TopOfOpStack() >= 0 && CombineOp() >= 0)
        ;

    if (TopOfOpStack() >= 0)goto syntax;

    {
        long v;

        if (GetAtomStack(pundef, &v) < 0)goto syntax;
        if (GetAtomStack(NULL, NULL) >= 0)goto syntax;  // shouldn't be anything left
            
        AtomIdx = baseAtomIdx;
        OperIdx = baseOperIdx;
        /*ScrStatus(("*** RESULT %d undef=%d", v, *pundef));*/
        return(v);
    }
}

void PushOp(short token, short pri, short type)
{
    if (pri) {
        while (OperIdx > BaseOperIdx) {
            Oper *op = OperStack + OperIdx - 1;
            if (pri > op->Pri)
                break;
            if (pri == op->Pri && (type & RL))
                break;
            if (CombineOp() < 0) {
                ScrStatus(("*** Syntax error"));
                break;
            }
        }
    }
    if (OperIdx == MAXOPER) {
        ScrStatus(("*** Expression too complex"));
        return;
    }
    {
        Oper *op = OperStack + OperIdx++;

        op->Pri = pri;
        op->Token = token;
        op->Type = type;
    }
}

int TopOfOpStack(void)
{
    if (OperIdx <= BaseOperIdx)return(-1);
    return((int)OperStack[OperIdx-1].Token);
}

int SecondOffOpStack(void)
{
    if (OperIdx - 1 <= BaseOperIdx)return(-1);
    return((int)OperStack[OperIdx-2].Token);
}


void PushAtom(long val, short isundef)
{
    Atom *atom;

    if (AtomIdx == MAXATOM) {
        ScrStatus(("*** Expression too complex"));
        return;
    }
    atom = AtomStack + AtomIdx++;
    atom->Value = val;
    atom->Undef = isundef;
}

static int CombineOp(void)
{
    Oper *op;
    Atom *a1, *a2;
    Atom ar;

    if (OperIdx <= BaseOperIdx)return(-1);
    op = OperStack + --OperIdx;

    if (AtomIdx <= BaseAtomIdx)return(-1);
    a1 = AtomStack + --AtomIdx;

    if (op->Type & QBIN) {
        if (AtomIdx <= BaseAtomIdx)return(-1);
        a2 = AtomStack + --AtomIdx;
    }

    if (op->Type & QBIN) {
        ar.Undef = a1->Undef | a2->Undef;

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
            if (a1->Value)ar.Value = a2->Value / a1->Value;
            else ar.Undef = 1;
            break;
        case '%':
            if (a1->Value)ar.Value = a2->Value % a1->Value;
            else ar.Undef = 1;
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
            ScrStatus(("*** fatal error parsing exp"));
            break;
        }
    } 
    else {
        ar.Undef = a1->Undef;

        switch(op->Token) {
        case '!':
            ar.Value = !a1->Value;
            break;
        case '~':
            ar.Value = ~a1->Value;
            break;
        case CUIND:                 /*  indirect through memory */
            if (a1->Value & 1) {
                ar.Undef = 1;
            } else {
                ar.Value = *(long *)a1->Value;
            }
            break;
        case CUMI:                  /*  negate  */
            ar.Value = -a1->Value;
            break;
        case '(':
            ar.Value = a1->Value;
            break;
        case CDEFINED:
            ar.Undef = 0;
            if (a1->Undef)
                ar.Value = 0;
            else
                ar.Value = 1;
            break;
        default:
            ScrStatus(("*** fatal error parsing exp"));
            break;
        }
    }
    AtomStack[AtomIdx++] = ar;
    /*ScrStatus(("*** op %c %03x result %d (%d)", op->Token, op->Token, ar.Value, ar.Undef));*/
}

static int GetAtomStack(short *pundef, long *pv)
{
    Atom *at;

    if (AtomIdx <= BaseAtomIdx) {
        if (pundef)*pundef = 1;
        return(-1);
    }
    at = AtomStack + --AtomIdx;
    if (pundef)*pundef = at->Undef;
    if (pv)*pv = at->Value;
    return(0);
}

static int ParseCharConst(char *buf, long i, long max, long *pv)
{
    unsigned char c;
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
    if (i < max) ++i;   /*      skip closing single quote   */
        
    else ScrStatus(("*** unterminated char const"));
    return(i);
}

/*
 *  0xhex
 *  0octal
 *  1-9decimal
 */

static int ParseInt(char *buf, long i, long max, long *pv)
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
        } 
        else {                                       /*  oct */
            short n;
            while (i < max && (n = OctDig(buf[i])) >= 0) {
                v = (v << 3) + n;
                ++i;
            }
        }
    }
    else {
        while (i < max && (c = buf[i]) >= '0' && c <= '9') { /*  dec */
            v = v * 10 + c - '0';
            ++i;
        }
    }
    while ((c = buf[i]) == 'L' || c == 'l' || c == 'U' || c == 'u')++i;

    // if a . follows the number, its a line number;  so find the
    // address associated with it
    if((buf[i] == '.') && (CurDisplay->ds_DisplayMode <= DISPLAY_LONGS)) {
        ULONG address, line ,info[2];

        i++;
        address = CurDisplay->ds_WindowTop;
        line = 0;
        CurrentMixedLine(&address,&line,info);

        while(info[1] < v) {
            NextMixedLine(&address,&line,info);
        }
        v = address;
    }
    *pv = v;
    return(i);
}

static int HexDig(char c)
{
    if (c >= '0' && c <= '9')return (c - '0');
    if (c >= 'a' && c <= 'f')c = c + ('A' - 'a');
    if (c >= 'A' && c <= 'F')return (c + (10 - 'A'));
    return(-1);
}

static int OctDig(char c)
{
    if (c >= '0' && c <= '7')return (c - '0');
    return(-1);
}


/*
 *  Random support
 */

short SymbolChar(short c)
{
    if (c >= '0' && c <= '9')return(1);
    if (c >= 'a' && c <= 'z')return(1);
    if (c >= 'A' && c <= 'Z')return(1);
    if (c == '_' || c == '@')return(1);
    return(0);
}

long ExtSymbol(unsigned char *base, long i, long max)
{
    long b = i;

    while (i < max && SymbolChar(base[i]))++i;
    return(i);
}



#ifdef MYKE_REMOVED_THIS

typedef struct Sym {
    char    *Name;
    char    *Text;
    long    TextLen;
} Sym;

Local Sym *FindSymbol(char *, short);
Sym *FindSymbol(char *name, short nameLen)
{
    static Sym SymAry[] = {
        {   "a", "b+3"  , 3   },
        {   "b", "4"    , 1   }
    };
    Sym *sym;

    for (sym = SymAry; sym < SymAry + (sizeof(SymAry)/sizeof(SymAry[0])); ++sym) {
        if (strlen(sym->Name) == nameLen && strnicmp(name, sym->Name, nameLen) == 0)
            return(sym);
    }
    return(NULL);
}
#endif