
/*
 *  DEFS.H
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#ifdef AMIGA
#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#else
#include <sys/param.h>
#include <suplib/lists.h>
#include <suplib/memory.h>
#define __autoinit
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define Prototype extern

#ifdef DEBUG
#define dbprintf(pri,x) if (DDebug > pri) printf x
#else
#define dbprintf(pri,x)
#endif

#ifdef _DCC
#define cprintf printf
#define csprintf sprintf
#endif

#define MAXUNITS    512
#define SMAX_BUF    256
#define EWARN       1
#define EERROR      2
#define EFATAL      3
#define ESOFT       4

typedef unsigned char ubyte;
typedef unsigned short uword;
typedef struct MinNode MinNode;
typedef struct Node Node;
typedef struct List List;

#if defined(AMIGA) && !defined(CROSS_COMPILE)
#include "/das/ops.h"
#else
#include "../das/ops.h"
#endif

typedef struct ProgramUnit {
    Node        pu_Node;
    char        *pu_Name;

    int         pu_Offset;      /*  data/code offset    */
    unsigned int pu_Size;       /*  size in bytes       */
    unsigned int pu_Type;
} ProgramUnit;

typedef struct RelocInfo {
    MinNode     ri_Node;
    struct Symbol   *ri_Sym;
    short       ri_SrcHunk;
    short       ri_DstHunk;
    short       ri_RelocSize;
    short       ri_RelocFlags;
    int         ri_SrcOffset;
} RelocInfo;

typedef struct Symbol {
    MinNode     sm_Node;        /*  sorted by offset    */
    short       sm_DefHunk;
    unsigned int sm_Value;
    unsigned int sm_CommonSize;
    ubyte       sm_Type;
    char        sm_Name[4];
} Symbol;

#define RF_PCREL    0x0001
#define RF_A4REL    0x0002

extern short NumHunks;
/*extern int32_t StartDo;*/
/*extern int32_t StartPc;*/

#ifdef NOTDEF
extern void *GetSucc();
extern void *GetPred();
extern void *GetHead();
extern void *GetTail();
extern void *RemHead();
#endif

#include "dobj-protos.h"
