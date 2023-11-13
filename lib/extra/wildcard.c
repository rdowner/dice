
/*
 *  WILDCARD.C      AmigaDos wildcard comparator
 *
 *                  WARNING:  Uses *Lots* of stack
 *
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#define _EXTRA_WILDCARD_C
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <lib/misc.h>

#define SetWildStack    _SetWildStack
#define CompWild    _CompWild
#define ParseWild   _ParseWild
#define FreeWild    _FreeWild

/*
 *  Example:    #(a|b|c)xx  consts of:
 *
 *      masternode(#), wn_Next = xx
 *                     wn_Sub  = (a|b|c)
 *
 *      (a|b|c) wn_Next ->  a, wn_Next = NULL, wn_Sib = b
 *                          b, wn_Next = NULL, wn_Sib = c
 *                          c, wn_Next = NULL, wn_Sib = NULL
 *
 *  Etc...
 *
 *  The most complexity occurs when dealing with '#' ... this could
 *  probably be made more efficient.  Basically, we run through all
 *  possible repeats from N to none at all.  For example, if the pattern
 *  is #?x  then we try ????x ???x ??x ?x, and x.  The number of repeats
 *  we try is currently STUPIDLY set to the length of the name remaining
 *  since that is the worst case.
 *
 *  Note that this algorithm correctly handles extremely complex patterns
 *  like:   #(a|b#?|c).o
 */

typedef struct WildNode {
    struct WildNode *wn_Next;   /*  next wild node (append to current) */
    struct WildNode *wn_Sib;    /*  sibling wild node (a|b|c|d)        */
    struct WildNode *wn_Sub;    /*  sub wildnode #(a|b|c)              */
    struct WildNode *wn_Cont;   /*  used at Compare time               */
    const char *wn_Pat;         /*  patterns not containing | or #     */
    short wn_PatLen;            /*  length of pattern       */
    short wn_Type;              /*  '|', '#', or 0          */
    short wn_Repeat;            /*  # of repeats (for '#')  */
} WildNode;

void SetWildStack(long);                       /*  global  */
WildNode *ParseWild(const char *, short);
int CompWild(const char *, WildNode *, WildNode *);
void FreeWild(WildNode *);

WildNode *ParseSubWild(const char *, short);    /*  local   */
int extinpat(const char *, int);
int extonepat(const char *, int);


/*
 *  Parse the structure & run
 */

static char *BotStack;

WildNode *
ParseWild(pat, patlen)
const char *pat;
short patlen;
{
    short i;
    WildNode *wn = NULL;
    WildNode **wnext = &wn;

    if (patlen == 0)
        return(NULL);

    while (i = extinpat(pat, patlen)) {
        *wnext = ParseSubWild(pat, i);

        wnext = &(*wnext)->wn_Sib;

        pat += i;
        patlen -= i;
        if (patlen && *pat == '|') {
            ++pat;
            --patlen;
        }
    }
    return(wn);
}

static WildNode *
ParseSubWild(pat, patlen)
const char *pat;
short patlen;
{
    WildNode *wn;

    short i;

    if (patlen == 0)
        return(NULL);

    wn = calloc(sizeof(WildNode), 1);

    for (i = 0; i < patlen; ++i) {
        if (pat[i] == '#' || pat[i] == '(')
            break;
        if (pat[i] == '\'')
            ++i;
    }

    if (i) {
        wn->wn_Pat = pat;
        wn->wn_PatLen = i;
        wn->wn_Next = ParseSubWild(pat + i, patlen - i);
    } else if (*pat == '#') {
        ++pat;
        --patlen;
        i = extonepat(pat, patlen);
        wn->wn_Sub  = ParseWild(pat, i);
        wn->wn_Type = '#';
        wn->wn_Next = ParseSubWild(pat + i, patlen - i);
    } else if (*pat == '(') {
        i = extonepat(pat, patlen);
        wn->wn_Sub = ParseWild(pat + 1, i - 2);
        wn->wn_Type = 0;
        wn->wn_Next = ParseSubWild(pat + i, patlen - i);
    }
    return(wn);
}

void
FreeWild(wn)
WildNode *wn;
{
    if (wn) {
        if (wn->wn_Sib)
            FreeWild(wn->wn_Sib);
        if (wn->wn_Next)
            FreeWild(wn->wn_Next);
        if (wn->wn_Sub)
            FreeWild(wn->wn_Sub);
        free(wn);
    }
}

void
SetWildStack(n)
long n;
{
    BotStack = (char *)&n - n;
}

/*
 *  CompWild().     Pretty straight forward except for '#'.
 *
 *  The 'cont' field is a linked string of nodes that specify patterns
 *  that must be successfully compared after the current pattern, 'wn',
 *  compares successfully.  When we are dealing with repeats of a
 *  pattern, such as #(a|b).o, then the 'cont' field is used to specify
 *  that (a|b) should be repeated wn_Repeat times.
 *
 *  A further complication occurs with: #(a|b#?).o, where we have a
 *  nested repeat loop.  In this case as 'cont' is scanned ONLY the
 *  top repeat loop is decremented by the 'cont' code.  The inner loop
 *  will be decremented by the '#' code.  Thus, when we skip to the
 *  next 'cont' we must skip any nodes with non-zero wn_Repeat
 *
 *  I am not entirely certain that I've done it properly, the code is
 *  complex.
 */

CompWild(name, wn, cont)
const char *name;
WildNode *wn;
WildNode *cont;
{
    {
        char x;

        if (&x < BotStack) {
            errno = ESTACK;
            return(-1);
        }
    }
top:
    if (wn == NULL) {
        if (cont) {
            if (cont->wn_Repeat) {
                if (--cont->wn_Repeat) {
                    wn = cont->wn_Sub;
                    cont = cont;
                } else {
                    wn = cont->wn_Sub;
                    cont = cont->wn_Cont;
                    while (cont && cont->wn_Repeat)
                        cont = cont->wn_Cont;
                }
                goto top;
            } else {
                wn = cont;
                cont = cont->wn_Cont;
                while (cont && cont->wn_Repeat)
                    cont = cont->wn_Cont;
                goto top;
                /*return(CompWild(name, cont, cont->wn_Cont));*/
            }
        }
        if (*name)
            return(-1);
        return(0);
    }
    /*printf("foo %08lx (%d)%.*s\n", wn, wn->wn_PatLen, wn->wn_PatLen, wn->wn_Pat);*/
    if (wn->wn_PatLen) {
        const char *pat = wn->wn_Pat;
        const char *oname = name;
        short patlen;

        for (patlen = wn->wn_PatLen; patlen > 0; --patlen, ++pat) {
            switch(*pat) {
            case '%':
                break;
            case '?':
                if (*name == 0)
                    patlen = 0;     /*  fail    */
                ++name;
                break;
            case '\'':
                ++pat;
                --patlen;
                /* fall through */
            default:
                {
                    char c1 = *name;
                    char c2 = *pat;
                    if (c1 != c2) {
                        if (c1 >= 'a' && c1 <= 'z')
                            c1 += 'A' - 'a';
                        if (c2 >= 'a' && c2 <= 'z')
                            c2 += 'A' - 'a';
                        if (c1 != c2)
                            patlen = 0;     /*  fail    */
                    }
                }
                ++name;
                break;
            }
        }
        if (patlen < 0 || CompWild(name, wn->wn_Next, cont) < 0) {
            if (wn->wn_Sib)
                return(CompWild(oname, wn->wn_Sib, cont));
            return(-1);
        }
        return(0);
    }
    {
        WildNode *next = wn->wn_Next;

        if (next) {
            next->wn_Cont = cont;
            cont = next;
        }
        if (wn->wn_Type == '#') {
            short i = strlen(name);

            while (i > 0) {
                wn->wn_Cont = cont;
                wn->wn_Repeat = i;
                if (CompWild(name, NULL, wn) >= 0) {
                    wn->wn_Repeat = 0;
                    return(0);
                }
                --i;
            }
            wn->wn_Repeat = 0;
            if (CompWild(name, NULL, cont) < 0) {
                if (wn->wn_Sib) {
                    wn = wn->wn_Sib;
                    goto top;
                }
                return(-1);
            }
            return(0);
        } else {
            if (CompWild(name, wn->wn_Sub, cont) < 0) {
                if (wn->wn_Sib == NULL)
                    return(-1);
                wn = wn->wn_Sib;
                goto top;
                /*return(CompWild(name, wn->wn_Sib, cont));*/
            }
            return(0);
        }
    }
}


static int
extonepat(pat, patlen)
const char *pat;
int patlen;
{
    if (patlen <= 0) {
        printf("PATLEN BAD %d\n", patlen);
        return(0);
    }
    switch(*pat) {
    case '?':
    case '%':
        return(1);
    case '#':
        return(1 + extonepat(pat + 1, patlen - 1));
    case '|':
        puts("extone, unexpected '|'");
        return(0);
    case '(':
        {
            short pcnt = 0;
            short i = 0;

            while (patlen) {
                ++i;
                if (*pat == '(')
                    ++pcnt;
                if (*pat == ')' && --pcnt == 0)
                    return(i);
                ++pat;
                --patlen;
            }
        }
        puts("unexpected EOF looking for ')'");
        return(0);
    case '\'':
        if (patlen < 2) {
            puts("extone, unexpected EOF at '");
            return(0);
        }
        return(2);
    default:
        return(1);
    }
    /* not reached */
}


static int
extinpat(pat, patlen)
const char *pat;
int patlen;
{
    int i;
    short paren = 0;

    for (i = 0; i < patlen; ++i) {
        if (pat[i] == '(')
            ++paren;
        if (pat[i] == ')' && --paren < 0)
            return(i);
        if (pat[i] == '|' && paren == 0)
            return(i);
    }
    return(i);
}


