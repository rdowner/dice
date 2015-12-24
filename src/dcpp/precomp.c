/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  PRECOMP.C
 *
 *  handle precompiled includes
 *
 */

#include "defs.h"

#ifdef AMIGA
#include "/dc1/tokens.h"
#else
#include "../dc1/tokens.h"
#endif

#define THASHSIZE   256
#define THASHMASK   (THASHSIZE-1)

#define FASTPRECOMP 1

Prototype void InitPrecomp(void);
Prototype void LoadPrecompiledHeader(char *, PreCompHdr *, int);
Prototype short PreCompSymbol(char *, short);

/*Prototype void DumpPrecompiledHeader(PreCompNode *, FILE *, long, long);*/
Prototype void DumpPrecompiledPrefix(PreCompNode *);
Prototype void DumpPrecompiledPostfix(PreCompNode *, FILE *);


typedef struct TokenNode {
    struct TokenNode *tn_Next;
    char    *tn_Name;
    short   tn_Len;
    short   tn_Token;
} TokenNode;

#define MakeToken(name,token)	{ NULL, name, sizeof(name) - 1, (((token >> 4) & 0xFF00) | (ubyte)token | TOKF_PRIVATE) }

TokenNode TokenAry[] = {
    MakeToken("sizeof",     TokSizeof),
    MakeToken("break",      TokBreak),
    MakeToken("case",       TokCase),
    MakeToken("continue",   TokContinue),
    MakeToken("default",    TokDefault),
    MakeToken("do",         TokDo),
    MakeToken("else",       TokElse),
    MakeToken("for",        TokFor),
    MakeToken("goto",       TokGoto),
    MakeToken("if",         TokIf),
    MakeToken("return",     TokReturn),
    MakeToken("struct",     TokStruct),
    MakeToken("enum",       TokEnum),
    MakeToken("switch",     TokSwitch),
    MakeToken("union",      TokUnion),
    MakeToken("while",      TokWhile),
    MakeToken("typedef",    TokTypeDef)
};

TokenNode *TokenHash[THASHSIZE];

/*char CharType[256];*/

void
InitPrecomp()
{
    TokenNode *tn;
    TokenNode **tp;

    for (tn = TokenAry; tn < &TokenAry[arysize(TokenAry)]; ++tn) {
	tp = &TokenHash[hash(tn->tn_Name, tn->tn_Len) & THASHMASK];
	tn->tn_Next = *tp;
	*tp = tn;
    }
}

short
PreCompSymbol(char *ptr, short len)
{
    TokenNode *tn;

    for (tn = TokenHash[hash(ptr, len) & THASHMASK]; tn; tn = tn->tn_Next) {
	/*fprintf(stderr, "%d %d %.*s %.*s\n", len, tn->tn_Len, len, ptr, tn->tn_Len, tn->tn_Name);*/
	if (len == tn->tn_Len && cmpmem(tn->tn_Name, ptr, len) == 0) {
	    return(tn->tn_Token);
	}
    }
    return(0);
}

/*
 *  Load precompiled header from file descriptor / specified device.
 *  copy preprocessed data directly to Fo and incorporate symbol table
 *  additions.
 */

void
LoadPrecompiledHeader(fileName, pch, fd)
char *fileName;
PreCompHdr *pch;
int fd;
{
    /*
     *	copy preprocessor dump
     */

    if (pch->pc_CppSize) {
	long bufsiz = ((pch->pc_CppSize > 32768) ? 32768 : pch->pc_CppSize);
	char *buf;
#if !FASTPRECOMP
	long bytes = pch->pc_CppSize;
#endif

	if ((buf = malloc(bufsiz)) == NULL)
	    ErrorNoMemory();
#if FASTPRECOMP
	fprintf(Fo, "#precomp %ld %ld \"%s\"\n", (long)lseek(fd, 0, 1), pch->pc_CppSize, fileName);
	lseek(fd, pch->pc_CppSize, 1);
#else
	xxx;
	bytes -= 4;	/*  don't embed \0's    */
	while (bytes) {
	    long n;

	    n = read(fd, buf, ((bytes > bufsiz) ? bufsiz : bytes));
	    if (n < 0)
		cerror(EFATAL_READ_PRECOMP);
	    fwrite(buf, n, 1, Fo);
	    bytes -= n;
	}
	free(buf);
#endif
    }

    /*
     *	incorporate symbol set	pc_SymSize
     *
     *	scan symbols in reverse
     */

    if (pch->pc_SymSize) {
	char *symData;
	long i;
	long j;

	if ((symData = malloc(pch->pc_SymSize)) == NULL)
	    ErrorNoMemory();
	if (read(fd, symData, pch->pc_SymSize) != pch->pc_SymSize)
	    cerror(EFATAL_PARSE_PRECOMP);
	for (i = pch->pc_SymSize - 4; i > 0; i = j - 8) {
	    j = i - *(long *)(symData + i);
	    DefinePrecompSymbol((Sym *)(symData + j));
	}
    }
}

/*
 *  Dump prefix and postfix for preprocessed headers
 */

void
DumpPrecompiledPrefix(pcn)
PreCompNode *pcn;
{
    PreCompHdr pch;

    /*
     *	dummy header
     */

    pch.pc_Magic = 0;	    /*	bad value for now   */
    pch.pc_SymSize = 0;
    pch.pc_CppSize = 0;
    pch.pc_Version[0] = 0;
    fwrite(&pch, 1, sizeof(pch), Fo);
    fputc('\n', Fo);        /*  need an initial newline */
}

/*
 *  Dump Postfix for precompiled headers
 *
 *  Note that foOrig is our cpp output file, NOT the precompiled output
 *  file (which is still Fo)
 */

void
DumpPrecompiledPostfix(pcn, foOrig)
PreCompNode *pcn;
FILE *foOrig;
{
    PreCompHdr pch;
    long pos1;
    long pos2;

    fwrite("\0\0\0\0", 4, 1, Fo);   /* terminator for DC1   */
    pos1 = ftell(Fo);
    DumpPrecompSymbols(Fo);
    pos2 = ftell(Fo);

    /*
     *	fixup header
     */

    fseek(Fo, 0L, 0);
    pch.pc_Magic = PCH_MAGIC;
    pch.pc_SymSize = pos2 - pos1;
    pch.pc_CppSize = pos1 - sizeof(pch);
    strcpy(pch.pc_Version, VersionId);
    fwrite(&pch, sizeof(pch), 1, Fo);

#if FASTPRECOMP
    fprintf(foOrig, "#precomp %zd %ld \"%s\"\n",
	sizeof(pch),
	pch.pc_CppSize,
	pcn->pn_OutName
    );
#else
    {
	static char XBuf[2048];
	long n;
	long s;

	for (s = pch.pc_CppSize; s; s -= n) {
	    n = (s < sizeof(XBuf)) ? s : sizeof(XBuf);
	    if (fread(XBuf, 1, n, Fo) == n) {
		if (fwrite(XBuf, 1, n, foOrig) != n) {
		    exit(22);
		}
	    } else {
		exit(22);
	    }
	}
    }
#endif
}

/*
 *  Save preprocessed header from fo<begpos-endpos> to pcn->pn_OutName,
 *  also save symbol table
 */

#ifdef NOTDEF3

void
DumpPrecompiledHeader(pcn, fi, begpos, endpos)
PreCompNode *pcn;
FILE *fi;
long begpos;
long endpos;
{
    PreCompHdr pch;
    FILE *fo;
    long pos;
    static char *SymRefPtr[256];
    static long SymRefLen[256];
#ifdef NOTDEF
    short symRefIdx = 0;
#endif

    setmem(SymRefLen, sizeof(SymRefLen), 0);
    if ((fo = fopen(pcn->pn_OutName, "w+")) == NULL)
	cerror(EFATAL_CANT_CREATE_FILE, pcn->pn_OutName);
    if (fseek(fi, begpos, 0) < 0)
	cerror(EFATAL_SEEK_PRECOMP);

    /*
     *	dummy header
     */

    pch.pc_Magic = 0;	    /*	bad value for now   */
    pch.pc_SymSize = 0;
    pch.pc_CppSize = 0;
    pch.pc_Version[0] = 0;
    fwrite(&pch, 1, sizeof(pch), fo);

    /*
     *	copy preprocessor dump to precomp file, tokenize simple constructs
     */

    if (endpos != begpos) {
	char *buf;
	long bytes = endpos - begpos;
	long j = 0;
	short c;
	short wsFlag = 0;   /*	remove white space flag */

	if ((buf = malloc(bytes)) == NULL)
	    ErrorNoMemory();
	if (fread(buf, bytes, 1, fi) != 1)
	    cerror(EFATAL_READ_PRECOMP);

	pch.pc_CppSize = bytes;

	fwrite(buf, bytes, 1, fo);

#ifdef NOTDEF2	    /*	doesn't help enough */
	while (j < bytes) {
	    long i;

	    c = buf[j];

	    /*
	     *	Scan for symbols.  Ignore symbols that are part of numbers
	     *	because DC1 cannot parse that.	We must also ignore
	     *	quoted strings and character constants.
	     */

	    switch(CharType[c]) {
	    case -2:	 /* white space 	    */
		if (wsFlag) {
		    --pch.pc_CppSize;
		} else {
		    putc(c, fo);
		    wsFlag = 1;
		}
		++j;
		continue;
	    case -1:	 /* quote or single quote   */
		wsFlag = 0;
		for (i = j, ++j; j < bytes && buf[j] != buf[i]; ++j) {
		    if (buf[j] == '\\')
			++j;
		}
		++j;
		fwrite(buf + i, j - i, 1, fo);
		break;
	    case 0:	/*  not anything remarkable */
		wsFlag = 0;
		putc(c, fo);
		++j;
		break;
	    case 3:
		for (i = j, ++j; j < bytes && buf[j] != '\n'; ++j)
		    ;
		fwrite(buf + i, j - i, 1, fo);
		break;
	    case 1:	/*  number		    */
		/*
		 *  In parsing a number we skip potential symbols embedded
		 *  in the number because DC1 cannot handle this case.
		 */

		wsFlag = 0;
		for (i = j, ++j; j < bytes && (CharType[c = buf[j]] > 0 || c == '.'); ++j)
		    ;
		fwrite(buf + i, j - i, 1, fo);
		break;
	    case 2:	/*  symbol		    */
		/*
		 *  In parsing a symbol note that numerical types are valid
		 *  symbol characters after the first character, e.g. A23
		 */

		wsFlag = 0;
		{
		    TokenNode *tn;
		    char *ptr = buf + j;

		    for (i = j, ++j; j < bytes && CharType[buf[j]] > 0; ++j)
			;
		    i = j - i;	    /*	i == length of symbol	*/

		    for (tn = TokenHash[hash(ptr, i) & THASHMASK]; tn; tn = tn->tn_Next) {
			if (tn->tn_Len == i && cmpmem(ptr, tn->tn_Name, i) == 0)
			    break;
		    }
		    if (tn) {
			fwrite(&tn->tn_Token, sizeof(tn->tn_Token), 1, fo);
			pch.pc_CppSize -= i - sizeof(tn->tn_Token);
			wsFlag = 1;
		    } else {
#ifdef NOTDEF
			short idx;

			/*
			 *  check previously used symbols.  When placing
			 *  a reference use a simple 8 bit hash value but
			 *  leave room to allow DC1 to change it to a
			 *  real pointer.
			 */

			for (idx = (symRefIdx - 1) & 255; idx != symRefIdx; idx = (idx - 1) & 255) {
			    if (i == SymRefLen[idx] && cmpmem(SymRefPtr[idx], ptr, i) == 0)
				break;
			}
			idx = (symRefIdx - idx) & 255;

			if (idx > 0 && idx < 128) {
			    putc(TokCppRef1_Byte, fo);
			    putc(idx, fo);
			    pch.pc_CppSize -= i - sizeof(tn->tn_Token);
			    wsFlag = 1;
			} else
#endif
			{
#ifdef NOTDEF
			    SymRefPtr[symRefIdx] = ptr;
			    SymRefLen[symRefIdx] = i;
			    symRefIdx = (symRefIdx + 1) & 0xFF;
#endif
			    fwrite(ptr, i, 1, fo);
			    wsFlag = 0;
			}
		    }
		}
		break;
	    }
	}
#endif
	free(buf);
    }

    fseek(fo, 0L, 1);
    pos = ftell(fo);

    /*
     *	dump symbols
     */

    DumpPrecompSymbols(fo);

    /*
     *	fixup header
     */

    pch.pc_Magic = PCH_MAGIC;
    pch.pc_SymSize = ftell(fo) - pos;
    strcpy(pch.pc_Version, VersionId);
    fseek(fo, 0L, 0);
    fwrite(&pch, sizeof(pch), 1, fo);
    fclose(fo);
    fseek(fi, endpos, 0);
}

#endif
