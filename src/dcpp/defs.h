
/*
 *  DEFS.H
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/file.h>
#include <include/lib/version.h>	/* DICE specific include */

#ifdef AMIGA

#include <lib/unix.h>

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <clib/exec_protos.h>

#else

#define __stkargs
#include <suplib/lists.h>
#include <suplib/memory.h>
#include <suplib/stdlib.h>
#include <suplib/string.h>
#include <unistd.h>

#endif

#ifdef DEBUG
#define dbprintf(fubar) if (DDebug) printf fubar
#else
#define dbprintf(fubar)
#endif

#define Prototype   extern
#define Local
#define arysize(ary)	(sizeof(ary)/sizeof((ary)[0]))

typedef unsigned char	ubyte;
typedef unsigned short	uword;
#ifndef linux
typedef unsigned long	ulong;
#endif

typedef struct Node Node;
typedef struct MinNode MinNode;
typedef struct List List;
typedef struct MinList MinList;

typedef struct Include {
    struct Include *Next;
    char    *FileName;
    long    LineNo;
    long    Level;

    long    Index;	    /*	set for prev cpp by push    */
    long    MaxIndex;	    /*	set by cpp() before scan    */
    char    *Base;	    /*	set by cpp() before scan    */

    char    IsFile;	    /*	vs macro replace	    */
    char    Reserved1;
    char    Reserved2;
    char    Reserved3;
} Include;

typedef struct PreCompNode {
    struct PreCompNode	*pn_Next;
    char    *pn_HeadName;
    char    *pn_OutName;
} PreCompNode;

typedef struct PreCompHdr {
    ulong   pc_Magic;
    long    pc_CppSize;     /*	preprocessed dump file	*/
    long    pc_SymSize;     /*	symbol table size	*/
    char    pc_Version[32];
} PreCompHdr;

#define PCH_MAGIC   (('d'<<24)|('I'<<16)|('c'<<8)|'E')

/*
 *  Symbol Type
 */

#define SF_RECURSE	0x01
#define SF_STRINGIZE	0x02
#define SF_SPECIAL	0x04
#define SF_MACROARG	0x08	/*  macro-arg, temp enable higher-up macro IDs	*/
#define SF_LITERAL	0x10

typedef struct Sym {
    struct Sym *Next;	/*  next in hash		*/
    struct Sym *Creator;/*  SF_MACROARG 		*/
    char    *SymName;
    short   SymLen;
    short   Type;	/*  type of symbol		*/
    short   NumArgs;	/*  if a macro, else -1 	*/
    char    **Args;	/*  names of args for replace	*/
    short   *ArgsLen;	/*  lengths of args for replace */
    char    *Text;	/*  contents of symbol		*/
    long    TextLen;
    long    SymGroup;	/*  for precompiled headers	*/
    short   Hv; 	/*  hash value			*/
} Sym;

#define HSIZE	1024	/*  symbol hash table		*/
#define HMASK	(HSIZE-1)

#define MAX_IF_LEVEL	    256
#define MAX_INCLUDE_LEVEL   32
#define MAX_ARGS	    256

#define ZA_SIZE 8192

#include "error.h"
#include "dcpp-protos.h"

