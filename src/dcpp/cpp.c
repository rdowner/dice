/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  CPP.C
 *
 */

#include "defs.h"

Prototype Include *PushBase;
Prototype char WhiteSpace[256];
Prototype char SpecialChar[256];
Prototype char SymbolChar[256];

Prototype void InitCpp(void);
Prototype int32_t cpp(int32_t, int, char *, FILE *, char *, int32_t);
Prototype int32_t SkipCommentLine(char *, int32_t, int32_t);
Prototype int32_t SkipComment(char *, int32_t, int32_t);
Prototype int32_t ExtSymbol(char *, int32_t, int32_t);
Prototype int32_t SkipString(char *, int32_t, int32_t);
Prototype int32_t SkipSingleSpec(char *, int32_t, int32_t);
Prototype void Dump(char *, int32_t, int32_t);
Prototype void DoLineSpec(void);

Include *PushBase;
char WhiteSpace[256];
char SpecialChar[256];
char SymbolChar[256];

void
InitCpp()
{
    short i;

    WhiteSpace[9] = 1;
    WhiteSpace[' '] = 1;
    WhiteSpace['\r'] = 1;
    WhiteSpace['l'&31] = 1;

    for (i = '0'; i <= '9'; ++i)
        SymbolChar[i] = 1;
    for (i = 'a'; i <= 'z'; ++i)
        SymbolChar[i] = 1;
    for (i = 'A'; i <= 'Z'; ++i)
        SymbolChar[i] = 1;
    SymbolChar['_'] = 1;
    SymbolChar[1] = 1;      /*  special for macro replace */

    SpecialChar['\''] = 1;
    SpecialChar['\"'] = 1;
    SpecialChar['/'] = 1;
    SpecialChar['?'] = 1;
}

int32_t
cpp(oldi, level, file, fi, xbuf, xbytes)
int32_t oldi;
int level;
char *file;
FILE *fi;
char *xbuf;
int32_t xbytes;
{
    int32_t len;
    int32_t i;
    int32_t w;
    short ifIndex = IfIndex;
    char *base;
    Include inc;

    if (level >= MAX_INCLUDE_LEVEL)
        cerror(EFATAL_MAX_INCLUDE, level);

    if (fi == NULL && xbuf == NULL) {
        ErrorOpenFailed(file, 0);
        return(oldi);
    }
    if (fi) {
        if (StripOpt == 0)
            fprintf(Fo, "# 1 \"%s\" %d\n", file, level);
        fseek(fi, 0L, 2);
        len = ftell(fi);
        if (len < 0) {
            cerror(EERROR_FILE_SIZE, file, len);
            fclose(fi);
            return(oldi);
        }
        base = malloc(len+1);
        if (base == NULL)
            ErrorNoMemory();
        fseek(fi, 0L, 0);
        { 
            int n;

            if ((n = fread(base, 1, len, fi)) != len) {
                cerror(EERROR_READ_ERROR, file);
                fclose(fi);
                return(oldi);
            }
        }
        fclose(fi);      /*  fi no longer valid */

        base[len] = 0;   /*  emergency terminator to prevent bad code from crashing us!  */

        /*
         *  Check for tri-graphs, modify accordingly
         */

        if (TriGraph) {
#ifdef NO_ASM
            char *ptr = base;
            char c;

            while ((c = *ptr) != 0) {
                ++ptr;
                if (c == '?' && *ptr == '?') {
                    if ((c = TriGraphConvert(ptr[1])) != 0) {
                        ptr[-1] = c;
                        len -= 2;
                        movmem(ptr + 2, ptr, len - (ptr - (char *)base));
                    }
                }
            }
#else
            len = HandleTriGraphs(base) - base;
#endif
        }
    } else {             /*  else macro  */
        base = xbuf;
        len = xbytes;
    }
    dbprintf(("CPP %s (%ld)\n", file, len));

    if (PushBase)
        PushBase->Index = oldi;

    inc.Next = PushBase;
    inc.FileName = file;
    inc.LineNo   = 1;
    inc.Level    = level;
    inc.IsFile   = (fi) ? 1 : 0;
    inc.MaxIndex = len;
    inc.Base     = base;
    PushBase = &inc;

    {
        short nl = 1;                       /*  newline just occured */
        short c;
        short stringtize = 0;

        for (i = w = 0; i < len; ) {
            c = base[i];

            /*
             *  White space.  If stripping, strip out completely except when
             *  not preceeded by a newline (leave one whitespace char), if
             *  not stripping then copy whitespace.
             */
            if (WhiteSpace[(ubyte)c]) {
                if (StripOpt) {
                    ++i;
                    if (nl == 0)
                        Dump(base, w, i);
                    while (i < len && WhiteSpace[(ubyte)base[i]])
                        ++i;
                    w = i;
                } else {
                    ++i;
                }
                continue;
            }
            if (c == '\n') {                /*  newline              */
                /*
                 *  If stripping, skip newlines after the first.
                 */

                if (StripOpt) {
                    ++i;
                    if (nl == 0) {
                        Dump(base, w, i);
                        if (w == i-1 || base[i-2] != '\\')
                            nl = 1;
                    }
                    w = i;
                    continue;
                }

                /*
                 *  If line continuation force resynchronization and set nl
                 *  to 0.  Else if line spec forced then dump to just before
                 *  the newline, execute a line spec.
                 */

                if (w < i && base[i-1] == '\\') {
                    ForceLineSpec = 1;
                    Dump(base, w, i - 1);
                    w = i + 1;
                    nl = 0;
                } else if (ForceLineSpec) {
                    Dump(base, w, i);
                    w = i;
                    DoLineSpec();
                    nl = 1;
                } else {
                    nl = 1;
                }
                ++i;
                ++inc.LineNo;
                continue;
            }

            /*
             *  Handle directive or token pasting
             */

            if (c == '#' && !StripOpt) {
                if (nl && fi) {
                    dbprintf(("directive\n"));
                    Dump(base, w, i);
                                            /*  execute directive    */
                    w = i = HandleDirective(base, i + 1, len);
                } else if (base[i+1] == '#') {
                    int32_t si = i + 2;
                    while (i >= w && WhiteSpace[(ubyte)base[--i]]);
                    ++i;
                    Dump(base, w, i);
                    i = si;
                    while (i < len && WhiteSpace[(ubyte)base[i]])
                        ++i;
                    w = i;
                } else if (fi == NULL) {
                    Dump(base, w, i);
                    stringtize = 1;
                    w = ++i;
                } else {
                    ++i;
                }
                continue;
            }

            /*
             *  Do not misinterpret a number as a symbol, especially parts
             *  of the number that contain characters such as 0x23 or
             *  0.234E+4  It will suffice to include '.' in our definition
             *  of a symbol for this loop
             *
             *  Note that embedded characters in numbers will not be handled
             *  by DC1 if symbolized for precompiled includes, but the
             *  processing of this is handled in PRECOMP.C, not here.
             */

            if (c >= '0' && c <= '9') {
                nl = 0;     /*  NOT a directive      */
                for (++i; i < len && (SymbolChar[(ubyte)(c = base[i])] || c=='.'); ++i)
                    ;
                continue;
            }

            /*
             *  symbol?
             */

            if (IfEnabled && SymbolChar[(ubyte)c]) {
                int32_t b = i;
                Sym *node;

                nl = 0;     /*  NOT a directive      */
                i = ExtSymbol(base, i, len);    /*  extract symbol       */
                if ((node = FindSymbol(base + b, i - b)) != NULL) {
                    if (node->Type && SF_MACROARG && stringtize)
                        node->Type |= SF_STRINGIZE;
                    Dump(base, w, b);
                    w = i = HandleSymbol(node, base, i, len);
                    node->Type &= ~SF_STRINGIZE;
                } else if (PreCompFlag) {
                    short t;

                    /*
                     *  Precomp optimization
                     */

                    if ((t = PreCompSymbol(base + b, i - b)) != 0) {
                        Dump(base, w, b);
                        w = i;
                        putc(((char *)&t)[0], Fo);
                        putc(((char *)&t)[1], Fo);
                    }
                }
                stringtize = 0;
                continue;
            }                               /*  delete comments      */
            if (stringtize)
                cerror(EERROR_STRINGTIZE);

            /*
             * Strip out embedded comments.  Leave nl flag intact if -strip
             * option was specified allowing us to strip whole sections,
             * including newlines.
             */

            if (SpecialChar[c]) {
                if (c == '/') {
                    if (base[i+1] == '*') {
                        if (StripOpt == 0 || nl == 0)
                            Dump(base, w, i);
                        w = i = SkipComment(base, i + 2, len);
                        if (nl == 0 || StripOpt == 0)
                            putc(' ', Fo);
                        continue;
                    }
                    if (base[i+1] == '/' && SlashSlashOpt) {
                        if (StripOpt == 0 || nl == 0)
                            Dump(base, w, i);
                        w = i = SkipCommentLine(base, i + 2, len);
                        continue;
                    }
                }
                nl = 0;
                if (c == '\'') {
                    i = SkipSingleSpec(base, i + 1, len);
                    continue;
                }
                if (c == '\"') {
                    i = SkipString(base, i + 1, len);
                    continue;
                }
            }
            nl = 0;
            ++i;
        }
        Dump(base, w, i);
        /* w = i */
    }

    if (xbuf == NULL)
        free(base);

    if (IfIndex != ifIndex)
        cerror(EWARN_IFS_LEFT_PENDING, IfIndex, ifIndex);

    PushBase = inc.Next;

    if (PushBase) {
        if (fi)
            DoLineSpec();
        return(PushBase->Index);
    }
    return(0);
}

int32_t
SkipCommentLine(base, i, max)
char *base;
int32_t i;
int32_t max;
{
    while (i < max && base[i] != '\n')
        ++i;
    if (i == max)
        cerror(EERROR_UNEXPECTED_EOF);
    return(i);
}

int32_t
SkipComment(base, i, max)
char *base;
int32_t i;
int32_t max;
{
    char c;

    while (i < max) {
        c = base[i];
        if (c == '/' && base[i+1] == '*')
            cerror(EWARN_NESTED_COMMENT);
        if (c == '*' && base[i+1] == '/')
            break;
        if (c == '\n') {
            ++PushBase->LineNo;
            if (StripOpt == 0)
                putc('\n', Fo);
        }
        ++i;
    }
    if (i == max) {
        cerror(EERROR_UNEXPECTED_EOF);
    } else {
        i += 2;
        if (i > max)
            cerror(EFATAL_SOFTERROR_177);
    }
    return(i);
}

int32_t
ExtSymbol(base, i, max)
char *base;
int32_t i;
int32_t max;
{
#ifdef DEBUG
    int32_t b = i;
#endif
    char *sc = SymbolChar;

    while (i < max && sc[(ubyte)base[i]])
        ++i;

    dbprintf(("ExtSymbol: %.*s\n", (int)(i - b), base + b));

    return(i);
}

#ifdef NOTDEF

/*
 *  This routine strips comments from a buffer
 */

void
StripComments(buf, bytes)
char *buf;
int32_t bytes;
{
    char c;
    int32_t i;
    int32_t b;

    for (i = 0; i < bytes; ++i) {
        if (buf[i] == '/' && i + 1 < bytes) {
            if (buf[i+1] == '*') {
                b = i;
                i += 2;
                while (i < bytes) {
                    if (buf[i] == '*' && buf[i+1] == '/' && i + 1 < bytes) {
                        i += 2;
                        break;
                    }
                    ++i;
                }
                setmem(buf + b, i - b, ' ');
            } else if (buf[i+1] == '/' && SlashSlashOpt) {
                b = i;
                while (i < bytes && buf[i] != '\n')
                    ++i;
                setmem(buf + b, i - b, ' ');
                --i;
            }
        }
    }
}

#endif

int32_t
SkipString(char *base, int32_t i, int32_t max)
{
    while (i < max && base[i] != '\"') {
        /*
         *  Embedded newline with no previous backslash
         */

        if (base[i] == '\n') {
            cerror(EERROR_UNTERMINATED_STRING);
            break;
        }

        /*
         *  Leave backslashes alone.  The specical case of a \ followed
         *  by a newline is handled by DC1
         */

        if (base[i] == '\\') {
            ++i;
        }
        ++i;
    }
    if (i < max)
        ++i;
    return(i);
}

int32_t
SkipSingleSpec(char *base, int32_t i, int32_t max)
{
    while (i < max && base[i] != '\'') {
        if (base[i] == '\n') {
            cerror(EERROR_UNTERMINATED_CHARCONST);
            break;
        }
        if (base[i] == '\\')
            ++i;
        ++i;
    }
    if (i < max)
        ++i;
    return(i);
}

void
Dump(base, w, i)
char *base;
int32_t w;
int32_t i;
{
    if (w < i) {
        if (IfEnabled) {
            if (GlobalStringize) {
                int32_t n;
                for (n = w; n < i; ++n) {
                    char c = base[n];
                    if (c < 32 || c == '\"' || c == '\'' /* || c == '\\'*/) {
                        fprintf(Fo, "\\%03o", (ubyte)c);
                    } else {
                        putc(c, Fo);
                    }
                }
            } else {
                fwrite(base + w, 1, i - w, Fo);
            }
        } else {
            while (w < i) {
                if (base[w] == '\n')
                    fputc('\n', Fo);
                ++w;
            }
        }
    }
}

void
DoLineSpec()
{
    fprintf(Fo, "\n# %d \"%s\" %d\n",
            PushBase->LineNo, PushBase->FileName, PushBase->Level);
    ForceLineSpec = 0;
}

