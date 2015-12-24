/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  LEX.C
 */

/*
**      $Filename: lex.c $
**      $Author: dice $
**      $Revision: 30.326 $
**      $Date: 1995/12/24 06:09:42 $
**      $Log: lex.c,v $
 * Revision 30.326  1995/12/24  06:09:42  dice
 * .
 *
 * Revision 30.156  1995/01/11  05:04:48  dice
 * MINIDICE2 additions - check for input file too large (disabled, not
 * in MINIDICE release)
 *
 * Revision 30.5  1994/06/13  18:37:31  dice
 * .
 *
 * Revision 30.0  1994/06/10  18:04:52  dice
 * .
 *
 * Revision 1.5  1994/04/15  21:15:00  jtoebes
 * Correct dicecache compile problems.
 * Also fixed warning for hex constants in strings gobbling up characters.
 *
 * Revision 1.4  1993/11/14  17:22:10  jtoebes
 * Eliminate reference to lex.h (empty include file).
 *
 * Revision 1.3  1993/09/12  17:16:42  jtoebes
 * Fixed BUG00120 - \x does not allow a single hex character.
 * Changed the parsing so that it is more in line with ANSI.  \x will
 * parse as many characters as are hex.  also fixed the octal parsing code.
 *
 * Revision 1.2  1993/09/06  10:15:48  jtoebes
 * Fixed BUG01098 - Stack overflow with blank lines causing recursion.
 *
**/

#include "defs.h"
#ifdef _DCC
#include <lib/misc.h>	/*  get _parseargs1() & 2   */
#include <ioctl.h>
#include <clib/dicecache_protos.h>
#endif

Prototype void InitLex(void);

Prototype short LexSimpleToken(void);
Prototype short LexLineFeed(void);
Prototype short LexWhiteSpace(void);
Prototype short LexTLex(void);
Prototype short LexSymbol(void);
Prototype short LexInteger(void);
Prototype short LexDecimal(void);
Prototype short LexCharConst(void);
Prototype short LexString(void);
Prototype short LexToken(void);

/*Prototype short LexSymbolRef(short, int32_t);*/
Prototype short LexOctal(ubyte, int32_t);
Prototype short LexHex(ubyte, int32_t);
Prototype short LexFloating(int32_t, int32_t);

Prototype void PushLexFile(char *, short, int32_t, int32_t);
Prototype short PopLexFile(void);

Prototype int SpecialChar(int32_t *);
Prototype int32_t CharToNibble(short);
Prototype short SkipToken(short, short);
Prototype char *TokenToStr(short);
Prototype int32_t FindLexFileLine(int32_t, char **, int32_t *, int32_t *);
Prototype short FindLexCharAt(int32_t);

Prototype int32_t	  LexIntConst;
Prototype char	  *LexStrConst;   /*  also flt constant   */
Prototype int32_t	  LexStrLen;
Prototype Symbol  *LexSym;
Prototype void	  *LexData;
Prototype char	  LexHackColon;
Prototype char	  LexUnsigned;

Prototype char FileName[128];
Prototype ubyte SymbolSpace[256];
Prototype int32_t Depth;
Prototype short ErrorInFileValid;

Prototype   int32_t    LexCacheHits;
Prototype   int32_t    LexCacheMisses;
Prototype LexFileNode *LFBase;


Prototype short (*LexDispatch[256])(void);


LexFileNode *LFBase;

int32_t	LexIntConst;
char	*LexStrConst;	/*  also flt constant	*/
int32_t	LexStrLen;
Symbol	*LexSym;
void	*LexData;
char	LexHackColon;
char	LexUnsigned;

char FileName[128];
ubyte SymbolSpace[256];
int32_t Depth;

int32_t	LexCacheHits;
int32_t	LexCacheMisses;

#ifdef NOTDEF
xSymbol  *LexSymRefSym[256];
xshort	 LexSymRefIdx;
#endif

short	ErrorInFileValid;

#define TLF	 1
#define TWHITE	 2
#define TLEX	 3
#define TALPHA	 4
#define TZERO	 5
#define TNUM	 6
#define TCHARC	 7
#define TSTRC	 8
#define TTOKEN	 9
#define TALPHX  10

static const ubyte TokenType[256] = {
/*00*/	0,	0,	0,	0,	0,	0,	0,	0,
	0,	TWHITE, TLF,	0,	TWHITE, TWHITE, 0,	0,
/*10*/	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
/*20*/	TWHITE, 0,	TSTRC,	0,	0,	0,	0,	TCHARC,
	0,	0,	0,	0,	0,	0,	0,	0,
/*30*/	TZERO,	TNUM,	TNUM,	TNUM,	TNUM,	TNUM,	TNUM,	TNUM,
	TNUM,	TNUM,	0,	0,	0,	0,	0,	0,
/*40*/	0     , TALPHX, TALPHX, TALPHX, TALPHX, TALPHX, TALPHX, TALPHA,
	TALPHA, TALPHA, TALPHA, TALPHA, TALPHA, TALPHA, TALPHA, TALPHA,
/*50*/	TALPHA, TALPHA, TALPHA, TALPHA, TALPHA, TALPHA, TALPHA, TALPHA,
	TALPHA, TALPHA, TALPHA, 0,	0,	0,	0,	TALPHA,
/*60*/	0     , TALPHX, TALPHX, TALPHX, TALPHX, TALPHX, TALPHX, TALPHA,
	TALPHA, TALPHA, TALPHA, TALPHA, TALPHA, TALPHA, TALPHA, TALPHA,
/*70*/	TALPHA, TALPHA, TALPHA, TALPHA, TALPHA, TALPHA, TALPHA, TALPHA,
	TALPHA, TALPHA, TALPHA, 0,	0,	0,	0,	0,
/*80*/	TTOKEN, TTOKEN, TTOKEN, TTOKEN, TTOKEN, TTOKEN, TTOKEN, TTOKEN
};

/*short LexDispatch*/
short (*LexDispatch[256])(void);

short (*LexFuncTable[])(void) = {
    LexSimpleToken,
    LexLineFeed,
    LexWhiteSpace,
    LexTLex,
    LexSymbol,
    LexInteger,
    LexDecimal,
    LexCharConst,
    LexString,
    LexToken,
    LexSymbol
};

void
InitLex(void)
{
    short i;

    Assert(1);	/* filler to avoid gnu compiler warnings */

    {
	ubyte *ss = SymbolSpace;

	for (i = 'a'; i <= 'z'; ++i)
	    ss[i] = 1;
	for (i = 'A'; i <= 'Z'; ++i)
	    ss[i] = 1;
	for (i = '0'; i <= '9'; ++i)
	    ss[i] = 1;
	ss['_'] = 1;
    }
    for (i = 0; i < 256; ++i)
	LexDispatch[i] = LexFuncTable[(short)TokenType[i]];
}


/*
 *  Lexical file push/pop
 */

void
PushLexFile(char *name, short nameLen, int32_t begPos, int32_t bytes)
{
    LexFileNode *lf = zalloc(sizeof(LexFileNode) + nameLen + 1);

    if (bytes == -2) {
	lf->lf_FileName = "";
	lf->lf_Fi = (FILE *)name;
    } else {
	lf->lf_FileName = (char *)(lf + 1);
	movmem(name, lf->lf_FileName, nameLen);
	lf->lf_FileName[nameLen] = 0;

	if ((lf->lf_Fi = fopen(lf->lf_FileName, "r")) == NULL)
	    zerror(EFATAL_CANT_OPEN_FILE, lf->lf_FileName);
    }

    if (bytes < 0) {
	fseek(lf->lf_Fi, 0L, 2);
	if ((bytes = ftell(lf->lf_Fi)) < 0)
	    zerror(EFATAL_INPUT_NOT_SEEKABLE);
	fseek(lf->lf_Fi, begPos, 0);
    } else {
	fseek(lf->lf_Fi, begPos, 0);
    }
#ifdef MINIDICE2
    if (bytes > 65530) {
        zerror(EFATAL_INPUT_TOO_LARGE, bytes);
    }
#endif
#ifdef _DCC
    /*
     * HACK!  we should really add an IODOM_CACHE field and make ioctl()
     *        return it for cached files.
     */
    if (ioctl(fileno(lf->lf_Fi), IOC_DOMAIN, NULL) != IODOM_AMIGADOS) {
	/*
	 *  DICECACHE support - directly reference the cache.  Note that
	 *  we cannot close the file handle without invalidating our
	 *  cache entry and, since we reference pointers into the buffer,
	 *  that means we cannot close any handles until the program is
	 *  done.
	 */

        struct {
            void *cfh_Cn;    // dicecache.library descriptor
            int32_t  cfh_Size;
            int32_t  cfh_Pos;
        } *cfh = (void *)ioctl(fileno(lf->lf_Fi), IOC_GETDESC, NULL);

	lf->lf_Buf = DiceCacheSeek(cfh->cfh_Cn, begPos, &lf->lf_Size);
	lf->lf_Size = bytes;	/* XXX check against lf_Size	*/
	/*fprintf(stderr, "PUSH-CACHED %08lx %d\n", lf->lf_Buf, lf->lf_Size);*/
    } else
#endif
    {
	/*fprintf(stderr, "PUSH-UNCACHED %d\n", bytes);*/
	if ((lf->lf_Buf = malloc(bytes + 4)) != NULL) {
	    fread(lf->lf_Buf, bytes, 1, lf->lf_Fi);
	    clrmem(lf->lf_Buf + bytes, 4);
	    lf->lf_Size = bytes;
	} else {
	    NoMem();
	}
    }

    lf->lf_Next = LFBase;
    lf->lf_Index= 0;
    lf->lf_BaseIndex = 0;
    LFBase = lf;
}

short
PopLexFile(void)
{

    /*fprintf(stderr, "POPLEX %d\n", LFBase->lf_Index);*/

    if (LFBase) {
	LexFileNode *lf;

	if ((lf = LFBase->lf_Next) != NULL) {
	    LFBase = lf;
	    return(1);
	}
	/*
	 *  don't clear out last LFBase because lexical routines presume
	 *  one exists!
	 */
    }
    return(0);
}

short
LexToken(void)
{
    int32_t i = LFBase->lf_Index;
    char *ptr = LFBase->lf_Buf + i;

    LFBase->lf_Index = i + 2;

#ifdef NOTDEF
x    if (*ptr == TokCppRef_Byte) {
x	 return(LexSymbolRef(ptr[1], i + 2));
x    } else
#endif
    {
	return((ptr[0] << 12) | ptr[1]);
    }
}

short
LexLineFeed(void)
{
    int32_t i = LFBase->lf_Index;
    char *ptr = LFBase->lf_Buf + i;
    char *base;

    do {
	base = ptr;

        while (*ptr == '\n')    /*  optimize \n\n\n...  */
            ++ptr;
        while (*ptr == '#') {   /*  # line "file" depth */
            ++ptr;
            while (*ptr == ' ' || *ptr == 9)
                ++ptr;

            /*
             *  Ignore # line constructs, handle others
             */

            if (*ptr < '0' || *ptr > '9') {
                if (strncmp(ptr, "pragma", 6) == 0) {
                    ptr += 6;
                    while (*ptr == ' ' || *ptr == 9)
                        ++ptr;
#ifdef REGISTERED
                    if (strncmp(ptr, "libcall", 7) == 0) {
                        ProcessLibCallPragma(ptr + 7, 0);
                    } else if (strncmp(ptr, "syscall", 7) == 0) {
                        ProcessLibCallPragma(ptr + 7, 1);
                    } else
#endif
                    if (strncmp(ptr, "DCCOPTS", 7) == 0) {
                        /*
                         *  Process #pragma DCCOPTS.  Note that
                         *  associated memory is permanent since some options
                         *  refer directly to the parsed string.
                         */
#ifdef _DCC
                        char *p2 = ptr;
                        short len;
                        short ac;
                        char **av;

                        while (*p2 && *p2 != '\n')
                            ++p2;
                        len = p2 - ptr;
                        movmem(ptr, p2 = zalloc(len + 1), len);
                        if (ac = _parseargs1(p2 + 7, len - 7)) {
                            av = zalloc((ac + 2) * 4);
                            _parseargs2(p2 + 7, av + 1, ac);
                            ++ac;
                            ParseOpts(ac, av, 0);
                        }
#endif
                    }
                } else if (strncmp(ptr, "precomp", 7) == 0) {
                    int32_t begPos;
                    int32_t bytes;
                    char *name;
                    short nameLen;

                    for (; *ptr && *ptr != ' '; ++ptr)
                        ;
                    begPos = atoi(ptr);
                    for (ptr = ptr + 1; *ptr && *ptr != ' '; ++ptr)
                        ;
                    bytes = atoi(ptr);
                    while (*ptr && *ptr != '\"')
                        ++ptr;
                    name = ptr + 1;
                    for (ptr = ptr + 1; *ptr && *ptr != '\"'; ++ptr)
                        ;
                    nameLen = ptr - name;
                    while (*ptr && *ptr != '\n')
                        ++ptr;
                    LFBase->lf_Index = ptr - LFBase->lf_Buf + 1;
                    PushLexFile(name, nameLen, begPos, bytes);
                    return(GetToken());
                } else if (strncmp(ptr, "debug", 5) == 0) {
                    DDebug = atoi(ptr + 5);
                } else {
                    /* ignore... # line (parsed for error conditions only) */
                }
            }
            while (*ptr && *ptr != '\n')
                ++ptr;
            while (*ptr == '\n')
                ++ptr;
        }
        while (*ptr == ' ' || *ptr == '\t') /* optimization */
            ++ptr;
        LFBase->lf_Index = ptr - LFBase->lf_Buf;
    } while (base != ptr);
    return(GetToken());
}

short
LexWhiteSpace(void)
{
    const ubyte *tt = TokenType;
    const char *ptr = LFBase->lf_Buf + LFBase->lf_Index;

    while (tt[(ubyte)*ptr] == TWHITE)
	++ptr;
    LFBase->lf_Index = (char *)ptr - (char *)LFBase->lf_Buf;
    return(GetToken());
}

short
LexTLex(void)
{
    return(GetToken());
}


/*
 *  Lexical Symbol.  Symbol-init has already predefined most symbols
 */

short
LexSymbol(void)
{
    const char *base = LFBase->lf_Buf + LFBase->lf_Index;
    const char *ptr = base;

    {
	ubyte *ss = SymbolSpace;

	while (ss[(ubyte)*ptr])
	    ++ptr;
    }
    LexSym = MakeSymbol(base, ptr - base, TokId, NULL);
    LexData= LexSym->Data;

#ifdef NOTDEF
x    LexSymRefSym[LexSymRefIdx] = LexSym;
x    LexSymRefIdx = (LexSymRefIdx + 1) & 0xFF;
#endif

    {
	const ubyte *tt = TokenType;

	while (tt[(ubyte)*ptr] == TWHITE)
	    ++ptr;
    }
    LFBase->lf_Index = (char *)ptr - (char *)LFBase->lf_Buf;

    if (*ptr != ':') {
	LexHackColon = 0;
	return(LexSym->LexId);
    } else {
	LexHackColon = 1;

	switch(LexSym->LexId) {
	case TokVarId:
	case TokCase:
	case TokDefault:
	case TokEnumConst:
	    return(LexSym->LexId);
	default:
	    return(TokId);
	}
    }
}

#ifdef NOTDEF
x
x/*
x *  precompiled header support from DCPP, DCPP recognizes a previously
x *  defined symbol and gives a simple reference index with which we
x *  can look up the symbol instantly.
x */
x
xshort
xLexSymbolRef(relId, i)
xshort relId;
xlong i;
x{
x    relId = (LexSymRefIdx - relId) & 0xFF;
x    LexSym = LexSymRefSym[relId];
x
x#ifdef NOTDEF
x    LexSym = MakeSymbol(LexSymRefPtr[relId], LexSymRefLen[relId], TokId, NULL);
x#endif
x    LexData = LexSym->Data;
x
x    if (LexBuf[i] == ':') {
x	 LexHackColon = 1;
x
x	 switch(LexSym->LexId) {
x	 case TokVarId:
x	 case TokCase:
x	 case TokDefault:
x	 case TokEnumConst:
x	     return(LexSym->LexId);
x	 default:
x	     return(TokId);
x	 }
x    } else {
x	 LexHackColon = 0;
x	 return(LexSym->LexId);
x    }
x}
x
#endif

/*
 *  Hex or Octal
 */

short
LexInteger(void)
{
    int32_t i = LFBase->lf_Index + 1;
    ubyte c = LFBase->lf_Buf[i++];	    /*	second char */

    if (c == 'x' || c == 'X')
	return(LexHex(LFBase->lf_Buf[i], i + 1));
    return(LexOctal(c, i));
}

short
LexOctal(ubyte c, int32_t i)
{
    int32_t v = 0;
    int32_t b = i - 2;	/*  if flt  */
    char *lexBuf = LFBase->lf_Buf;

    LexUnsigned = 0;
    while (c >= '0' && c <= '7') {
	v = (v << 3) + (c - '0');
	c = lexBuf[i++];
    }
    if (v & 0x80000000)
	LexUnsigned = 1;

    while (c == 'L' || c == 'l' || c == 'U' || c == 'u') {
	if (c == 'u' || c == 'U')
	    LexUnsigned = 1;
	c = lexBuf[i++];
    }
    if (c == '.' || c == 'e' || c == 'E')
	return(LexFloating(b, i));

    LFBase->lf_Index = i - 1;
    LexIntConst = v;
    return(TokIntConst);
}

short
LexDecimal(void)
{
    char *lexBuf = LFBase->lf_Buf;
    int32_t i = LFBase->lf_Index;
    ubyte c = lexBuf[i++];
    int32_t v = 0;
    int32_t b = i - 1;	/*  if flt  */

    LexUnsigned = 0;
    while (c >= '0' && c <= '9') {
	v = (v * 10) + (c - '0');
	c = lexBuf[i++];
    }
    if (v & 0x80000000)
	LexUnsigned = 1;

    while (c == 'L' || c == 'l' || c == 'U' || c == 'u') {
	if (c == 'u' || c == 'U')
	    LexUnsigned = 1;
	c = lexBuf[i++];
    }
    if (c == '.' || c == 'e' || c == 'E')
	return(LexFloating(b, i - 1));
    LFBase->lf_Index = i - 1;
    LexIntConst = v;
    return(TokIntConst);
}

short
LexHex(ubyte c, int32_t i)
{
    int32_t v = 0;
    char *lexBuf = LFBase->lf_Buf;

    for (;;) {
	if (c >= '0' && c <= '9')
	    v = (v << 4) + (c - '0');
	else if (c >= 'a' && c <= 'f')
	    v = (v << 4) + (c - 'a' + 10);
	else if (c >= 'A' && c <= 'F')
	    v = (v << 4) + (c - 'A' + 10);
	else
	    break;
	c = lexBuf[i++];
    }
    while (c == 'L' || c == 'l' || c == 'U' || c == 'u')
	c = lexBuf[i++];
    LFBase->lf_Index = i - 1;
    LexIntConst = v;
    LexUnsigned = 1;
    return(TokIntConst);
}

short
LexFloating(b, i)
int32_t b, i;
{
    char *lexBuf = LFBase->lf_Buf;
    ubyte c = lexBuf[i++];

    LexStrConst = lexBuf + b;

    if (c == '.')
	c = lexBuf[i++];

    while (c >= '0' && c <= '9')
	c = lexBuf[i++];
    if (c == 'e' || c == 'E') {     /*  En E+n E-n  */
	/*
	 * used to set lexBuf[-1] to 'E' to fix bug in afp()
	 * but no longer since lexBuf may be direct from the cache!
	 */

	c = lexBuf[i++];
	if (c == '+' || c == '-')
	    c = lexBuf[i++];
	while (c >= '0' && c <= '9')
	    c = lexBuf[i++];
    }

    LexStrLen = i - b - 1;
    LexData = &DoubleType;

    switch(c) {
    case 'f':
    case 'F':
	LexData = &FloatType;
	++i;
	break;
    case 'l':
    case 'L':
	LexData = &LongDoubleType;
	++i;
	break;
    }
    LFBase->lf_Index = i - 1;

    return(TokFltConst);
}

short
LexCharConst(void)
{
    char *lexBuf = LFBase->lf_Buf;
    int32_t i = LFBase->lf_Index + 1;
    ubyte c = lexBuf[i++];
    short cnt = 0;
    int32_t v = 0;

    while (c != '\'') {
	if (c == 0)
	    zerror(EFATAL_UNEXPECTED_EOF);
	if (c == '\\') {
	    if (lexBuf[i] == '\n') {
		c = lexBuf[i+1];
		i += 2;
		continue;
	    }
	    c = SpecialChar(&i);
	}
	v = (v << 8) | c;
	++cnt;
	c = lexBuf[i++];
    }
    if (cnt > 4)
	zerror(EWARN_CHAR_CONST_TOO_LONG);
    if (cnt == 1)
	v = (char)v;
    LFBase->lf_Index = i;
    LexIntConst = v;
    LexUnsigned = 0;
    return(TokIntConst);
}

/*
 *  Extract a lexical string.  Since we may be referencing out of a
 *  file cache, and since we must terminate the string with \0, we
 *  therefore have to make a copy.
 */

short
LexString(void)
{
    int32_t i;
    int32_t n;
    ubyte c;
    ubyte nlisc = 0;
    ubyte pass = 0;
    char *ptr = NULL;
    char *lexBuf = LFBase->lf_Buf;

    for (pass = 0; pass < 2; ++pass) {
	i = LFBase->lf_Index;
	n = 0;

	for (;;) {
	    for (++i; (c = lexBuf[i]) != '\"'; ++i) {
		if (c == 0)
		    zerror(EFATAL_UNEXPECTED_EOF);
		if (c == '\n' && nlisc == 0) {
		    nlisc = 1;
		    zerror(EWARN_NEWLINE_IN_STRING_CONST);
		}
		if (c == '\\') {
		    ++i;
		    if (lexBuf[i] == '\n')
			continue;
		    c = SpecialChar(&i);
		    --i;
		}
		if (ptr)
		    ptr[n] = c;
		++n;
	    }

	    /*
	     *	Check for concactenated strings
	     */

	    for (c = lexBuf[++i]; TokenType[c] == TWHITE || TokenType[c] == TLF; c = lexBuf[++i])
		;
	    if (c != '\"')
		break;
	}
	if (pass == 0)
	    ptr = zalloc(n + 1);
	else
	    ptr[n] = 0;
	++n;
    }
    LFBase->lf_Index = i;
    LexStrConst = ptr;
    LexStrLen = n;
    return(TokStrConst);
}


#ifdef NOTDEF
xshort
xLexString(void)
x{
x    int32_t i = LexIdx + 1;
x    ubyte c;
x    ubyte nlisc = 0;
x    int32_t b = i;     /*  base of string frag */
x    int32_t j = i;     /*  actual write idx    */
x    int32_t appnl = 0;
x
x    for (;;) {
x	 c = LexBuf[i++];
x
x	 while (c != '\"') {
x	     if (c == 0)
x		 zerror(EFATAL_UNEXPECTED_EOF);
x	     if (c == '\n' && nlisc == 0) {
x		 nlisc = 1;
x		 zerror(EWARN_NEWLINE_IN_STRING_CONST);
x	     }
x	     if (c == '\\') {
x		 if (LexBuf[i] == '\n') {
x		     c = LexBuf[i+1];
x		     i += 2;
x		     ++appnl;
x		     continue;
x		 }
x		 c = SpecialChar(&i);
x	     }
x	     LexBuf[j++] = c;
x	     c = LexBuf[i++];
x	 }
x	 /*
x	  *  string LexBuf + b of length (j - b).  Check for concactenated strs
x	  *  as per ANSI.  Track file line numbering so we can recover it
x	  *  later.
x	  */
x
x	 c = LexBuf[i];
x	 while (TokenType[c] == TWHITE || TokenType[c] == TLF) {
x	     if (c == '\n')
x		 ++appnl;
x	     c = LexBuf[++i];
x	 }
x	 if (c != '\"')
x	     break;
x	 ++i;
x    }
x
x    /*
x     *  Terminating \0 is implicit to string constants, e.g. sizeof("abc").
x     *  Note that this also serves as a start-string indication for the
x     *  error routines (to avoid embedded newlines)
x     */
x
x    LexBuf[j++] = 0;
x    LexIdx = i;
x    LexStrConst = (char *)LexBuf + b;
x    LexStrLen	 = j - b;
x
x    /*
x     *  Clear out extra junk
x     */
x
x    while (j + appnl < i)
x	 LexBuf[j++] = ' ';
x
x    /*
x     *  Mark area as munged
x     */
x
x    MarkAreaMunged(b, j);
x
x    /*
x     *  Recover dummy newlines that were embedded in the string so line
x     *  number tracking doesn't screw up
x     */
x
x    while (appnl--)
x	 LexBuf[j++] = '\n';
x
x    return(TokStrConst);
x}
#endif

/*
 *  LexSimpleToken() returns simple tokens such as +, ++, etc..
 */

short
LexSimpleToken(void)
{
    char *ptr = LFBase->lf_Buf + LFBase->lf_Index + 1;
    short token;

    switch(ptr[-1]) {
    case '!':   /*  !, !=   */
	if (*ptr == '=') {
	    token = TokNotEq;
	    ++ptr;
	    break;
	}
	token = TokNot;
	break;
    case '%':   /*  %, %=   */
	if (*ptr == '=') {
	    token = TokPercentEq;
	    ++ptr;
	    break;
	}
	token = TokPercent;
	break;
    case '&':   /*  &, &&, &=   */
	if (*ptr == '&') {
	    token = TokAndAnd;
	    ++ptr;
	    break;
	}
	if (*ptr == '=') {
	    token = TokAndEq;
	    ++ptr;
	    break;
	}
	token = TokAnd;
	break;
    case '(':
	token = TokLParen;
	break;
    case ')':
	token = TokRParen;
	break;
    case '*':   /*  *, *=   */
	if (*ptr == '=') {
	    token = TokStarEq;
	    ++ptr;
	    break;
	}
	token = TokStar;
	break;
    case '+':   /*  +, ++, +=   */
	if (*ptr == '+') {
	    token = TokPlPl;
	    ++ptr;
	    break;
	}
	if (*ptr == '=') {
	    token = TokPlEq;
	    ++ptr;
	    break;
	}
	token = TokPl;
	break;
    case ',':
	token = TokComma;
	break;
    case '-':   /*  -, --, -=, ->   */
	switch(*ptr) {
	case '-':
	    token = TokMiMi;
	    ++ptr;
	    break;
	case '=':
	    token = TokMiEq;
	    ++ptr;
	    break;
	case '>':
	    token = TokStrInd;
	    ++ptr;
	    break;
	default:
	    token = TokMi;
	    break;
	}
	break;
    case '.':
	if (*ptr >= '0' && *ptr <= '9') {
	    int32_t i = (char *)ptr - (char *)LFBase->lf_Buf - 1;
	    return(LexFloating(i, i));
	}
	if (*ptr == '.' && ptr[1] == '.') {
	    token = TokDotDotDot;
	    ptr += 2;
	    break;
	}
	token = TokStrElm;
	break;
    case '/':   /*  /, /=   */
	if (*ptr == '=') {
	    token = TokDivEq;
	    ++ptr;
	    break;
	}
	token = TokDiv;
	break;
    case '<':   /*  <, <<, <<=, <=  */
	if (*ptr == '<') {
	    if (ptr[1] == '=') {
		token = TokLtLtEq;
		ptr += 2;
		break;
	    }
	    token = TokLtLt;
	    ++ptr;
	    break;
	}
	if (*ptr == '=') {
	    token = TokLtEq;
	    ++ptr;
	    break;
	}
	token = TokLt;
	break;
    case '=':
	if (*ptr == '=') {
	    token = TokEqEq;
	    ++ptr;
	    break;
	}
	token = TokEq;
	break;
    case '>':
	if (*ptr == '>') {
	    if (ptr[1] == '=') {
		token = TokGtGtEq;
		ptr += 2;
		break;
	    }
	    token = TokGtGt;
	    ++ptr;
	    break;
	}
	if (*ptr == '=') {
	    token = TokGtEq;
	    ++ptr;
	    break;
	}
	token = TokGt;
	break;
    case '?':
	token = TokQuestion;
	break;
    case ':':
	token = TokColon;
	break;
    case '[':
	token = TokLBracket;
	break;
    case ']':
	token = TokRBracket;
	break;
    case '^':
	if (*ptr == '=') {
	    token = TokCaratEq;
	    ++ptr;
	    break;
	}
	token = TokCarat;
	break;
    case '|':
	if (*ptr == '|') {
	    token = TokOrOr;
	    ++ptr;
	    break;
	}
	if (*ptr == '=') {
	    token = TokOrEq;
	    ++ptr;
	    break;
	}
	token = TokOr;
	break;
    case '~':
	token = TokTilde;
	break;
    case '{':
	token = TokLBrace;
	break;
    case '}':
	token = TokRBrace;
	break;
    case ';':
	token = TokSemi;
	break;
    case 0:	/*  force table lookup for critical code */
	if (PopLexFile()) {
	    return(GetToken());
	}
	/*
	 *  Do not update LexIdx
	 */
	return(0);
    case '#':   /*  hack to handle # directive on line 1   */
	if (LFBase->lf_Index == 0)
	    return(LexLineFeed());
	/* fall through */
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
    case 20:
    case 21:
    case 22:
    case 23:
    case 24:
    case 25:
    case 26:
    case 27:
    case 28:
    case 29:
    case 30:
    case 31:
    case 'a':
    case 'b':
    case 'c':
    case 'd':
    case 'e':
    case 'f':
    case 'g':
    case 'h':
    case 'i':
    case 'j':
    case 'k':
    case 'l':
    case 'm':
    case 'n':
    case 'o':
    case 'p':
    case 'q':
    case 'r':
    case 's':
    case 't':
    case 'u':
    case 'v':
    case 'w':
    case 'x':
    case 'y':
    case 'z':
    case 'A':
    case 'B':
    case 'C':
    case 'D':
    case 'E':
    case 'F':
    case 'G':
    case 'H':
    case 'I':
    case 'J':
    case 'K':
    case 'L':
    case 'M':
    case 'N':
    case 'O':
    case 'P':
    case 'Q':
    case 'R':
    case 'S':
    case 'T':
    case 'U':
    case 'V':
    case 'W':
    case 'X':
    case 'Y':
    case 'Z':
    default:
	zerror(EWARN_ILLEGAL_CHAR, ptr[-1], ptr[-1]);
	token = TokSemi;
	break;
    }
    LFBase->lf_Index = (char *)ptr - (char *)LFBase->lf_Buf;
    return(token);
}

/*
 *  MISC
 *
 */

int
SpecialChar(pi)
int32_t *pi;
{
    int32_t i = *pi;
    int c = LFBase->lf_Buf[i];
    int v = 0;

    *pi = ++i;

    switch(c) {
    case 10:	/*  a newline!	*/
	c = LFBase->lf_Buf[i];
	*pi = ++i;
	return(c);
	break;
    case 'n':
	return(10);
    case 'r':
	return(13);
    case 't':
	return(9);
    case 'f':
	return(12);
    case 'v':
	return(11);
    case 'a':
	return(7);
    case 'b':
	return(8);
    case 'x':
        for(;;)
        {
	    c = LFBase->lf_Buf[i];
	    switch(TokenType[c])
	    {
		case TALPHX:
		    c = (c & 7) + 9;	/* The & 7 makes the Upper and lower case */
					/* characters appear the same             */
		    break;
		case TNUM:
		case TZERO:
		    c -= '0';
		    break;
		default:
                    if ((i - *pi) > 2)
                    {
                       zerror(EWARN_HEX_CONST_TRUNCATED);
                    }
		    *pi = i;
		    return(v);
	    }
            i++;
            v <<= 4;
            v += c;
	}
    default:
        {
           int cnt = 0;

           while ((cnt < 3) && (c >= '0') && (c <= '7'))
           {
              v = (v << 3) | (c - '0');
              c = LFBase->lf_Buf[i];
              i++;
              cnt++;
           }
           if (cnt)
              i--;
           else
              v = c;
           *pi = i;
           return(v);
        }
    }
}

int32_t
CharToNibble(short c)
{
    if (c >= '0' && c <= '9')
	return(c - '0');
    return((c | 0x20) - 'a' + 10);
}

short
SkipToken(short t, short expect)
{
    if (t == expect)
	return(GetToken());
    zerror(EERROR_UNEXPECTED_TOKEN, TokenToStr(t), TokenToStr(expect));
    return(t);
}

char *
TokenToStr(short t)
{
    switch(t) {
    case TokComma:
	return(",");
    case TokSemi:
	return(";");
    case TokLParen:
	return("(");
    case TokRParen:
	return(")");
    case TokLBrace:
	return("{");
    case TokRBrace:
	return("}");
    case TokColon:
	return(":");
    case TokLBracket:
	return("[");
    case TokRBracket:
	return("]");
    default:
	return("<unknown>");
    }
}

/*
 *  Find the line number associated with a lexical offset
 */

int32_t
FindLexFileLine(lexIdx, plexFile, plexFileNameLen, plexIdxBeg)
int32_t lexIdx;
char **plexFile;
int32_t *plexFileNameLen;
int32_t *plexIdxBeg;
{
    int32_t i;
    char *ptr;
    int32_t lexLine = -1;

    static int32_t CacheLexIdx = -1;
    static int32_t CacheLexLine;
    static char *CacheLexFile;
    static int32_t CacheLexFileNameLen;

    printf("FINDLINE lexIdx %d %s\n", lexIdx, CacheLexFile);

    /*
     *	find start of line
     */

    *plexIdxBeg = lexIdx;
    *plexFile = "";

    if (LFBase == NULL)
	return(0);

    for (i = lexIdx, ptr = LFBase->lf_Buf + i; i > 0; --i) {
	if (ptr[-1] == '\n' /*&& !OffsetMunged(i-1)*/) {
	    *plexIdxBeg = i;
	    break;
	}
	--ptr;
    }
    /*printf(";i %d/%d:%c\n", i, LFBase->lf_Buf[i], LFBase->lf_Buf[i]);*/

    if (i == CacheLexIdx) {
	*plexFile	= CacheLexFile;
	*plexFileNameLen= CacheLexFileNameLen;
	*plexIdxBeg	= i;
	return(CacheLexLine);
    }

    /*
     *	reverse scan for '# <line> "<file>"'
     */

    while (i >= 0) {
	if (i == CacheLexIdx) {
	    *plexFile = CacheLexFile;
	    *plexFileNameLen = CacheLexFileNameLen;
	    lexLine += CacheLexLine + 1;
	    ++LexCacheHits;
	    break;
	}
	if (*ptr == '#' && (i == 0 || (ptr[-1] == '\n' /*&& !OffsetMunged(i)*/))) {
	    short j;
	    char c;

	    for (j = 1; (c = ptr[j]) == ' ' || c == '\t'; ++j)
		;
	    if ((c >= '0' && c <= '9') || c == '-') {
		lexLine += atoi(ptr + j);
		while ((c = ptr[j]) && c != '\"')
		    ++j;
		*plexFile = ptr + j + 1;
		*plexFileNameLen = strchr(*plexFile, '\"') - *plexFile;
		if (*plexFileNameLen < 0)
		    *plexFileNameLen = 0;
		CacheLexFile = *plexFile;
		CacheLexFileNameLen = *plexFileNameLen;
		++LexCacheMisses;
		break;

	    }
	}
	if (*ptr == '\n'/* && !OffsetMunged(i)*/)
	    ++lexLine;
	--i;
	--ptr;
    }
    CacheLexLine = lexLine;
    CacheLexIdx = *plexIdxBeg;

    return(lexLine);
}

short
FindLexCharAt(i)
int32_t i;
{
    if (LFBase) {
    	if (ftell(LFBase->lf_Fi) != i)
	    fseek(LFBase->lf_Fi, i, 0);
	return(getc(LFBase->lf_Fi));
    }
    return(EOF);
}

