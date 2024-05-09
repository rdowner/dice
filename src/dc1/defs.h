
/*
 *  DC1/DEFS.H
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 *
 */

typedef unsigned char   ubyte;
typedef unsigned short  uword;

#include <stdarg.h>                     /* standard includes    */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <include/lib/version.h>
#include "sym.h"                        /* local includes       */
#include "tokens.h"
#include "machine.h"
#include "stmt.h"
#include "var.h"
#include "types.h"
#include "error.h"

/*
 *  Subtle slow down for freeware release
 */

#ifdef _DCC
#if !defined(REGISTERED) && !defined(COMMERCIAL)
#undef putc
#define putc(c,fi) (((fi)->sd_WLeft <= 0) ? fputc((c),(fi)) : \
         (((--(fi)->sd_WLeft, ((*(fi)->sd_WPtr++ = (c))) == '\n')) ? (fflush(fi), '\n') : ((fi)->sd_WPtr)[-1]))
#endif
#endif

typedef struct LexFileNode {
    struct LexFileNode *lf_Next;
    FILE    *lf_Fi;         /*  file handle, keeps cache valid                */
    char    *lf_Buf;        /*  buffer, offset negative!! invalid < BaseIndex */
    char    *lf_FileName;
    int32_t    lf_BaseIndex;
    int32_t    lf_Index;            /*  current index   */
    int32_t    lf_Size;     /*  size of file    */
} LexFileNode;

#define MINIMAXPROCS    5


#define Prototype   extern
#define Local       static

#ifndef AMIGA

#define __stkargs
#include <suplib/string.h>
#include <suplib/memory.h>

#endif

#ifdef DEBUG
#define dbprintf(x) if (DDebug) printf x
#else
#define dbprintf(x)
#endif

#define CHUNKSIZE   8192

#include "dc1-protos.h"

#define AllocStructure(type)  ((type *)zalloc(sizeof(type)))
#define AllocTmpStructure(type)  ((type *)talloc(sizeof(type)))
#define arysize(ary)    (sizeof(ary)/sizeof((ary)[0]))

/*
 *  Call for gen*.c
 */

#define CallCenter() ((*exp->ex_Func)(&exp))
#define CallLeft()   ((*exp->ex_ExpL->ex_Func)(&exp->ex_ExpL))
#define CallRight()  ((*exp->ex_ExpR->ex_Func)(&exp->ex_ExpR))
#define EnsureReturnStorageLeft()   { if ((exp->ex_ExpR->ex_Flags & EF_CALL) || ((exp->ex_Flags & EF_ASSEQ) && (exp->ex_Flags & EF_CALL))) UnscratchStorage(exp->ex_ExpL); }
#define EnsureReturnStorageRight()  { if (exp->ex_ExpL->ex_Flags & EF_CALL) UnscratchStorage(exp->ex_ExpR); }

#define SOUTSIDE    1   /*  outside procedures              */
#define SARG        2   /*  declaring procedural arguments  */
#define SINSIDE     3   /*  inside procedure                */

#ifdef __BASE_FILE__
#ifdef AMIGA
#define Assert(cond)    if (!(cond)) _Assert(_SrcFile, __LINE__)
static const char *_SrcFile = __BASE_FILE__;
#else
#define Assert(cond)    if (!(cond)) _Assert(__BASE_FILE__, __LINE__)
#endif
#else
#define Assert(cond)    if (!(cond)) _Assert("uknown.c", __LINE__)
#endif

#define GetToken()  ((LexDispatch[(ubyte)LFBase->lf_Buf[LFBase->lf_Index]])())

