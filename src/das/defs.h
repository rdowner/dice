
/*
 * DAS/DEFS.H
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <fcntl.h>
#include <sys/file.h>
#include <include/lib/version.h>	/* DICE specific file */

#define FPCOPROC    1
#define MMUCOPROC   1

#ifndef AMIGA

#include <suplib/string.h>
#include <suplib/memory.h>
#include <unistd.h>

#define __stkargs

#endif

#ifdef DEBUG
#define dbprintf(pri,x) if (DDebug > pri) printf x
#else
#define dbprintf(pri,x)
#endif

#define Prototype   extern
#define Local

typedef unsigned char	ubyte;
typedef unsigned short	uword;

#include "ops.h"
#include "lab.h"
#include "reg.h"
#include "exp.h"
#include "sect.h"

#define AllocStructure(type)	zalloc(sizeof(type))

#define ZALLOCSIZ   4096

typedef struct EffAddr {
    Label   *Label1;
    Label   *Label2;
    char    ISize;	    /*	ISF_* from ops.h	*/
    char    Mode1;	    /*	AB_ mode(s)		*/
    char    Mode2;	    /*	alternate valid AB_ mode*/
    char    Reg1;	    /*	register spec		*/
    int32_t Offset1;	    /*	Integral Offset 	*/
    int32_t Offset2;
    uword   ExtWord;	    /*	index / regmask 	*/
} EffAddr;

#define MF_A5USED	0x01
#define MF_CALLMADE	0x02
#define MF_FORCELINK	0x04

struct FileNode;

typedef struct MachCtx {
    Label   *Label;	    /*	label or 0	    */
    Label   *m_XLabel;	    /*	used by directives  */
    Sect    *Sect;	    /*	section I'm in      */
    OpCod   *OpCode;	    /*	opcode or 0	    */
    int32_t    Bytes;	    /*	used by DS and DC, etc..    */
    char    *m_Operands;
    EffAddr Oper1;
    EffAddr Oper2;
    union {		    		/* directive specific	*/
	EffAddr *EABase;    		/* DC directive		*/
	char	*SaveStr;   		/* DC directive		*/
	uword	AlignValue; 		/* align directive	*/
	struct FileNode *FileNode;	/* include directive	*/
	int32_t    DebugLine;  		/* Line number for debug */
    } d;
    char    OpSize;	    /*	bwl 0,1,2,4	    */
    int32_t    LineNo;
    int32_t    m_Addr;
} MachCtx;

#define m_EABase    d.EABase
#define m_SaveStr   d.SaveStr
#define m_AlignValue d.AlignValue
#define m_DebugLine  d.DebugLine
#define m_FileNode   d.FileNode

typedef struct DebugNode {
    int32_t    db_Line;
    int32_t    db_Offset;
} DebugNode;

typedef struct FileNode {
    struct FileNode *fn_Parent;
    char	*fn_FileName;
    char	*fn_AsBuf;
    int32_t	fn_AsLen;
    int32_t	fn_MLines;
    int32_t	fn_LineNo;
    MachCtx	**fn_MBase;
} FileNode;

typedef struct IncNode {
    struct IncNode *in_Next;
    char	*in_Path;
} IncNode;

#include "error.h"
#include "das-protos.h"

