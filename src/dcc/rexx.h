
/*
 *  [LIBSRCPATH]REXX/REXX.H (PRIVATE)
 *
 *  only lib/rexx.h is public
 */

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/ports.h>

#define __DICE_INLINE       /*  force inline calls for autoinit stuff */

#include <clib/exec_protos.h>
#include <clib/alib_protos.h>
#include <clib/dos_protos.h>
#include <clib/rexxsyslib_protos.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lib/rexx.h>

typedef struct MsgPort  MsgPort;
typedef struct Message  Message;
typedef struct List     List;
typedef struct Node     Node;
typedef struct RexxMsg  RexxMsg;

typedef struct RexxIPNode {
    Node    rip_Node;
    short   rip_Returned;
    RexxMsg *rip_RexxMsg;
    MsgPort *rip_RexxPort;
} RexxIPNode;

typedef struct RexxPortNode {
    Node    rp_Node;
    short   rp_IsPublic;
    MsgPort *rp_MsgPort;
} RexxPortNode;

