
/*
 *  DEFS.H
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/file.h>
#include <include/lib/version.h>

#ifdef AMIGA

#include <exec/types.h>
#include <exec/lists.h>
#include <exec/memory.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#ifndef abs
#define abs
#endif
#include <clib/alib_protos.h>

#else

#include <suplib/lists.h>
#include <suplib/memory.h>
#include <suplib/string.h>
#include <unistd.h>

#endif

#include "error.h"

#ifdef DEBUG
#define dbprintf(pri,x) if (DDebug > pri) printf x
#else
#define dbprintf(pri,x)
#endif

#define Prototype   extern
#define Local

#undef NULL
#define NULL ((void *)0L)

#define ALIGN(x)    (((x) + 7) & ~3)

#ifndef O_BINARY
#define O_BINARY    0
#endif

#define Assert(exp) if (!(exp)) _Assert(__FILE__, __LINE__);

#ifdef SANITY_CHECK
#define SanityCheck(n)  _SanityCheck(n)
#else
#define SanityCheck(n)
#endif

typedef unsigned char   ubyte;
typedef unsigned short  uword;

typedef struct List     List;
typedef struct Node     Node;
typedef struct MinNode  MinNode;

#define NT_BSS      1
#define NT_CODE     2
#define NT_DATA     3

#define NT_FTOBJ    5
#define NT_FTLIB    6

#define SCAN_RELOC_SYMCK    0
#define SCAN_RELOC_SYMIN    1
#define SCAN_RELOC_CNT      2
#define SCAN_RELOC_RUN      3
#define SCAN_RELOC_JUMP     4
#define SCAN_COMMON_RESOLVE 5

#define RESERVED_PCJMP_TYPE 100

#define HSIZE   1024            /*  symbol hash table. WARNING, SEE HASH.A   */
#define HMASK   (HSIZE-1)

#define HF_DATABSS      0x0001  /*  BSS appended onto DATA hunk, use AddSize    */
#define HF_SMALLDATA    0x0002  /*  BSS or DATA hunk associated w/ small-data   */

#define HUNKIDF_FLAG    0x10000 /*  flag far data/bss hunks not part of unfragged coag */

typedef struct DBInfo {
    int32_t    di_HunkId;
    int32_t    di_Size;     /*  longwords in hunk not including this field */
    int32_t    di_Base;     /*  base offset, 0 in object module            */
    int32_t    di_LINE;     /*  'LINE'                                     */
    int32_t    di_NameSize;    /*       longwords of name                          */
    int32_t    di_Ary[2];           /*  [N] name, then debug info                  */
} DBInfo;

typedef struct Hunk {
    Node    Node;           /*  node, in ln_Type, ln_Name. linked list by name */
    uword   HunkNo;         /*  hunk in module              */
    struct HunkListNode *HL;/*  base of list                */
    struct HunkListNode *HX;/*  secondary relocation        */
    struct Module *Module;  /*  associated module / NULL    */
    uint32_t   HunkId;      /*  hunk-id, including upper bits */
    uint32_t   Offset;      /*  offset in final output      */
    uint32_t   TotalBytes;     /*       extended size includes jump tab (not rounded) */
    uint32_t   Bytes;       /*  size of hunk data           */
    uint32_t   ExtDefs;     /*  # of exported definitions   */
    uint32_t   *Data;       /*  CODE, DATA                  */
    uint32_t   *JmpData;            /*  more data... for jmp table  */
    uint32_t   *Reloc32;            /*  other object file stuff     */
    uint32_t   *Reloc16;            /*  PC   relative               */
    uint32_t   *Reloc16D;           /*  DATA relative               */
    uint32_t   *Reloc8;
    uint32_t   *Ext;
    uint32_t   *Sym;
    DBInfo  *DbInfo;        /*  debug info                  */
    List    SymList;        /*  linked list of syms (only if SymOpt)    */
    uint32_t   SeekDebug;           /*  seek in output file of debug hunk   */
    uword   Flags;          /*  special flags               */
} Hunk;

typedef struct Module {
    Node    Node;           /*  link node               */
    uword   Reserved;
    int32_t    NumHunks;            /*  # of hunks in module    */
    Hunk    **Hunks;        /*  hunk list               */
    struct FileNode *FNode; /*  file node (library)     */
    char    *ModBeg;        /*  raw data location       */
    char    *ModEnd;
} Module;

typedef struct Sym {
    MinNode Node;           /*  based at hunk (only if SymOpt)  */
    struct Sym *HNext;      /*  hash link                   */
    Hunk    *Hunk;          /*  hunk defined in / can be NULL if type 2 */
    int32_t    Value;       /*  value of defined symbol     */
    int32_t    Refs;        /*  references                  */
    char    *SymName;       /*  name of symbol              */
    short   SymLen;         /*  length of symbol            */
    ubyte   Type;
    ubyte   Flags;
} Sym;

#define SYMF_SYMLIST    0x01

typedef struct FileNode {
    Node    Node;
    uword   Reserved;
    uint32_t   *Data;       /*  ptr to data     */
    uint32_t   Bytes;       /*  bytes in file   */
    uint32_t   *DPtr;       /*  used while scanning */
} FileNode;

typedef struct HunkListNode {
    Node    Node;           /*  list of combined hunks  */
    uword   Reserved;
    uint32_t   HunkId;      /*  includes upper bits     */
    uint32_t   FinalHunkNo;    /*       final assigned hunk     */
    uint32_t   FinalSize;           /*  almost final size of hunk       */
    uint32_t   AddSize;     /*  additional size BSS -r opt adj  */
    uint32_t   FinalExtDefs;   /*       total # of exported definitions */
    uint32_t   SeekSym;     /*  seek in output file of symbol hunk  */
    List    HunkList;       /*  list of modules w/ same type [& name]   */
    uint32_t   **ExtReloc32;   /*       final relocation info for hunk  */
    uint32_t   *CntReloc32;    /*       # of relocations for each hunk  */
    uint32_t   *CpyReloc32;    /*       moving index during final copy  */
} HunkListNode;

#include "dlink-protos.h"

#ifdef AMIGA

extern void *GetHead(struct List *);
extern void *GetSucc(struct Node *);
extern void *GetTail(struct List *);
extern void *GetPred(struct Node *);

#endif

