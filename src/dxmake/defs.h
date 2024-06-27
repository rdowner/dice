
/*
 *  DEFS.H
 */

#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <assert.h>

#ifdef unix
#include <unistd.h>
#include <pwd.h>
#endif

#include "lists.h"

#ifdef sun
#include <strings.h>
#define USE_PUTENV      1
#else
#define USE_PUTENV      0
#endif

typedef struct Node Node;
typedef struct List List;

typedef unsigned char ubyte;
typedef unsigned short uword;

#define EXIT_CONTINUE   0

#define Prototype extern

#define FATAL   0
#define WARN    1
#define DEBUG   2

#define PBUFSIZE 256

#define NT_RESOLVED     0x01

#define ED_WAIT         1
#define ED_BACKGROUND   2

#define USE_DEBUG  1
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
 */

typedef struct DepNode {
    Node    dn_Node;
    List    dn_DepCmdList;      /*  list of lists   */
/*    time_t  dn_Time;*/
    short   dn_Symbolic;
    short   dn_Flags;
    int     dn_Result;
} DepNode;

#define DNF_VIRTUAL     0x0001  /* virtual lhs - has no command list */

#define DN_FAILED               -1
#define DN_CHANGED              0       
#define DN_NOCHANGE_TOUCH       1
#define DN_NOCHANGE             2


typedef struct DepRef  {
    Node    rn_Node;
    DepNode *rn_Dep;
 /*   time_t  rn_Time;*/
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

typedef struct IfNode {
    struct IfNode *if_Next;
    int         if_Value;
} IfNode;

#include "tokens.h"
#include "dmake-protos.h"

