
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
#include <lists.h>
#include <lib/version.h>
#else
#include <suplib/lists.h>
#include <suplib/memory.h>
#include <include/lib/version.h>
#define __aligned
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

#ifdef AMIGA
#include <clib/alib_protos.h>
#include <clib/exec_protos.h>
#include <exec/libraries.h>
extern struct Library *SysBase;
#define Running2_04() (SysBase->lib_Version >= 37)
#endif

typedef struct Node Node;
typedef struct List List;

typedef unsigned char ubyte;
typedef unsigned short uword;

#ifdef unix
#define EXIT_CONTINUE   0
#else
#define EXIT_CONTINUE   5
#endif

#define Prototype extern

#define FATAL   0
#define WARN    1
#define DEBUG   2

#define PBUFSIZE 256

#define NT_RESOLVED     0x01

#if USE_DEBUG
#define dbprintf(x)  { if (DDebug) printf x;}
#define db3printf(x) { if (DDebug >= 3) printf x;}
#define db4printf(x) { if (DDebug >= 4) printf x;}
#else
#define dbprintf(x)
#define db3printf(x)
#define db4printf(x)
#endif
/*
 *  A DepNode collects an entire left hand side symbol
 *  A DepCmdList collects one of possibly several groups for a DepNode
 *  A DepRef specifies a single dependency within a group
 *
 */

typedef struct DepNode {
    Node    dn_Node;
    List    dn_DepCmdList;      /*  list of lists   */
    time_t  dn_Time;
    short   dn_Symbolic;
    short   dn_Reserved;
} DepNode;

typedef struct DepRef  {
    Node    rn_Node;
    DepNode *rn_Dep;
} DepRef;

typedef struct DepCmdList {
    Node    dc_Node;            /*  greater link node   */
    List    dc_RhsList;         /*  right hand side(s)  */
    List    *dc_CmdList;         /*  command buf list    */
} DepCmdList;

#define NT_CMDEOL   0x01

typedef struct CmdNode {
    Node    cn_Node;
    long    cn_Idx;
    long    cn_Max;
    long    cn_RIndex;
} CmdNode;

typedef struct Var {
    Node    var_Node;
    List    var_CmdList;
} Var;

#include "tokens.h"
#include "dmake-protos.h"

