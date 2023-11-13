/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  DIRECT.C
 *
 */

#include "defs.h"
#ifdef _DCC
#include <lib/misc.h>   /*  get _parseargs1() & 2   */
#endif

Prototype short IfEnabled;
Prototype short IfIndex;

Prototype void InitDirective(void);
Prototype int32_t HandleDirective(char *, int, int);
Prototype void do_if(char *, int, int32_t *);
Prototype void do_ifndef(char *, int, int32_t *);
Prototype void do_ifdef(char *, int, int32_t *);
Prototype void do_else(char *, int, int32_t *);
Prototype void do_elif(char *, int, int32_t *);
Prototype void do_endif(char *, int, int32_t *);
Prototype void do_pragma(char *, int, int32_t *);
Prototype void do_error(char *, int, int32_t *);
Prototype void do_line(char *, int, int32_t *);
Prototype void do_null(char *, int, int32_t *);
Prototype void do_passthru(char *, int, int32_t *);

short IfEnabled = 1;
short IfIndex;

static char IfAry[MAX_IF_LEVEL];
static char InElse[MAX_IF_LEVEL];
static char AutoEndif[MAX_IF_LEVEL];    /*  for #elif   */

typedef struct Direct {
    short   Len;
    short   IfFlag;
    void    (*Func)(char *, int, int32_t *);
    const char *Name;
} Direct;

Direct DirAry[] = {
    {   6,  0,  do_define,  "define"    },
    {   5,  1,  do_ifdef,   "ifdef"     },
    {   6,  1,  do_ifndef,  "ifndef"    },
    {   2,  1,  do_if,      "if"        },
    {   4,  1,  do_else,    "else"      },
    {   4,  1,  do_elif,    "elif"      },
    {   5,  1,  do_endif,   "endif"     },
    {   7,  0,  do_include, "include"   },
    {   6,  0,  do_pragma,  "pragma"    },
    {   5,  0,  do_error,   "error"     },
    {   5,  0,  do_undef,   "undef"     },
    {   4,  0,  do_line,    "line"      },
    {   1, -1,  do_passthru,"#"         },
    {   0,  0,  do_null,    ""          },
    {   -1 }
};

void
InitDirective()
{
    IfEnabled = 1;
    IfAry[0] = 1;
    InElse[0] = 0;
}

int32_t
HandleDirective(base, i, max)
char *base;
int i;
int max;
{
    int32_t b = i;
    int32_t e;
    int32_t idx;
    Direct *dir;

    /*
     *  Determine extent of directive and strip comments.  Comments
     *  are not subject to newline termination.  Strip out newlines as
     *  they can interfere with line numbering.
     */

    {
        int32_t n = i;
        int32_t x = i;

        while (n < max && base[n] != '\n') {
            if (base[n] == '\'') {
                int32_t n2 = SkipSingleSpec(base, n + 1, max);
                movmem(base + n, base + x, n2 - n);
                x += n2 - n;
                n = n2;
                continue;
            }
            if (base[n] == '\"') {
                int32_t n2 = SkipString(base, n + 1, max);
                movmem(base + n, base + x, n2 - n);
                x += n2 - n;
                n = n2;
                continue;
            }
            if (base[n] == '/') {
                if (base[n+1] == '*') {
                    n = SkipComment(base, n + 2, max);
                    base[x++] = ' ';
                    continue;
                }
                if (base[n+1] == '/' && SlashSlashOpt) {
                    n = SkipCommentLine(base, n + 2, max);
                    base[x++] = ' ';
                    continue;
                }
            }
            if (base[n] == '\\' && base[n+1] == '\n') {
                ++PushBase->LineNo;
                putc('\n', Fo);
                n += 2;
                continue;
            }
            base[x++] = base[n++];
        }
        while (x > i && (base[x-1] == ' ' || base[x-1] == '\t'))
            --x;
        max = x;
        ++PushBase->LineNo;
        e = n;
    }

    /*
     *  Skip white space, find the directive.  Allow ## directives through
     *  with no modifications (but #if/#endif will work around them).  If
     *  stripping an include, ignore the directive (simply spit it out)
     */

    while (i < max && (base[i] == ' ' || base[i] == 9))     /*  skip ws */
        ++i;
    b = i;
    while (i < max && SymbolChar[(ubyte)base[i]])           /*  directive */
        ++i;

    for (dir = DirAry; dir->Len >= 0; ++dir) {
        if (dir->Len == i - b && strncmp(dir->Name, base + b, i - b) == 0)
            break;
    }

    while (i < max && (base[i] == ' ' || base[i] == 9))     /*  more ws */
        ++i;

    b = i;                                                  /*  BASE    */

    if (dir->Len < 0) {
        if (IfEnabled)
            cerror(EERROR_UNKNOWN_DIRECTIVE);
        putc('\n', Fo);
        return(e+1);
    }
    if (dir->IfFlag <= 0 && IfEnabled == 0) {
        putc('\n', Fo);
        return(e+1);
    }
    if (dir->IfFlag >= 0)
        putc('\n', Fo);

    /*
     *  note that max is different from idx.  The stuff between max and idx
     *  is junk.
     */

    idx = e + 1;

    (*dir->Func)(base + b, max - b, &idx);

    return(idx);
}

void
do_if(buf, max, pu)
char *buf;
int max;
int32_t *pu;   /*       unused */
{
    short undef;
    int32_t v;

    if (IfIndex == MAX_IF_LEVEL)
        cerror(EFATAL_MAX_IFS, MAX_IF_LEVEL);
    if (IfEnabled == 0) {
        ++IfIndex;
        IfAry[IfIndex] = 0;
        InElse[IfIndex] = 0;
        AutoEndif[IfIndex] = 0;
        return;
    }
    v = ParseIfExp(buf, &undef, max, 1);
    if (undef) {
        /* REMOVED, ANSI COMPAT
        cerror(EERROR, "#if : undefined symbol");
        v = 1;
        */
    }
    if (v == 0)
        IfEnabled = 0;
    ++IfIndex;
    IfAry[IfIndex] = IfEnabled;
    InElse[IfIndex] = 0;
    AutoEndif[IfIndex] = 0;
}

void
do_ifndef(buf, max, pu)
char *buf;
int max;
int32_t *pu;   /*       unused */
{
    short undef;

    ParseIfExp(buf, &undef, max, 0);
    if (undef)
        do_if("1", 1, NULL);
    else
        do_if("0", 1, NULL);
}

void
do_ifdef(buf, max, pu)
char *buf;
int max;
int32_t *pu;   /*       unused */
{
    short undef;

    ParseIfExp(buf, &undef, max, 0);
    if (undef)
        do_if("0", 1, NULL);
    else
        do_if("1", 1, NULL);
}

void
do_else(buf, max, pu)
char *buf;
int max;
int32_t *pu;   /*       unused */
{
    if (InElse[IfIndex]) {
        cerror(EERROR_MULTIPLE_ELSE_FOR_IF);
        return;
    }
    InElse[IfIndex] = 1;
    if (IfIndex && IfAry[IfIndex-1] == 0)
        return;
    IfAry[IfIndex] = 1 - IfAry[IfIndex];
    IfEnabled = IfAry[IfIndex];
}

void
do_elif(buf, max, pu)
char *buf;
int max;
int32_t *pu;   /*       unused */
{
    do_else(buf, max, NULL);
    AutoEndif[IfIndex] = 1;
    do_if(buf, max, NULL);
}

void
do_endif(buf, max, pu)
char *buf;
int max;
int32_t *pu;   /*       unused */
{
    if (IfIndex == 0) {
        cerror(EERROR_ENDIF_WITHOUT_IF);
        return;
    }
    do {
        --IfIndex;
        IfEnabled = IfAry[IfIndex];
    } while (IfIndex && AutoEndif[IfIndex]);
}

/*
 *  #pragma
 *
 *  DCCOPTS - processed by DCPP and passed on
 *  <other> - passed on
 */

void
do_pragma(buf, max, pu)
char *buf;
int max;
int32_t *pu;   /*       unused */
{
    if (strncmp(buf, "DCCOPTS", 7) == 0) {
#ifdef _DCC
        /*
         *  Parse #pragma DCCOPTS lines.  Use DICE's argument parser
         *  for main().  I am following the standard argc/argv opts
         *  here with a dummy av[0] and av[n] = NULL to give ParseOpts()
         *  the same format as main() gives it.  This is really not
         *  necessary but what the hell.
         */

        char *copy;
        char **av;
        short ac;

        movmem(buf, copy = zalloc(max + 1), max);

        if (ac = _parseargs1(copy + 7, max - 7)) {
            av = zalloc((ac + 2) * 4);
            _parseargs2(copy + 7, av+1, ac);
            ++ac;
            ParseOpts(ac, av, 0);
        }
#endif
    }

    /*
     *  Pragma's are always passed on
     */

    Dump("#pragma ", 0, 8);
    Dump(buf, 0, max);
    Dump("\n", 0, 1);
}

void
do_null(buf, max, pu)
char *buf;
int max;
int32_t *pu;   /*       unused  */
{

}

void
do_error(buf, max, pu)
char *buf;
int max;
int32_t *pu;   /*       unused */
{
    cerror(EERROR_ERROR_DIRECTIVE, max, buf);
}

/*
 *  # line <lineno> "filename"
 */

void
do_line(buf, max, pu)
char *buf;
int max;
int32_t *pu;   /*       unused */
{
    short len = 0;
    Include *pb = PushBase;

    pb->LineNo = atoi(buf);
    while (max && *buf != '\"') {
        --max;
        ++buf;
    }
    if (*buf == '\"') {
        ++buf;
        --max;
        while (len < max && buf[len] != '\"')
            ++len;
        if (strlen(pb->FileName) != len || strncmp(pb->FileName, buf, len) != 0) {
            pb->FileName = malloc(len + 1);
            pb->FileName[len] = 0;
            strncpy(pb->FileName, buf, len);
        }
    }
    fprintf(Fo, "# %d \"%s\" %d\n", pb->LineNo, pb->FileName, pb->Level);
}

void
do_passthru(buf, max, pu)
char *buf;
int max;
int32_t *pu;   /*       unused */
{
    Dump("##", 0, 2);
    Dump(buf, 0, max);
    Dump("\n", 0, 1);
}

