
/*
 *  RESIDENT.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 *
 */

#define SysBase_DECLARED
#define DOSBase_DECLARED

#include <exec/types.h>
#include <exec/execbase.h>
#include <exec/ports.h>
#include <exec/memory.h>
#include <libraries/dos.h>
#include <libraries/dosextens.h>
#include <lib/bcpl.h>
#include <lib/misc.h>
#include <clib/exec_protos.h>
#include <string.h>

typedef struct DosLibrary   DosLibrary;
typedef struct RootNode     RootNode;
typedef struct DosInfo      DosInfo;
typedef struct CommandLineInterface CLI;

typedef struct MsgPort      MsgPort;
typedef struct Message      Message;
typedef struct Task         Task;
typedef struct Process      Process;

typedef struct SegNode {
    BPTR    NextEntry;
    LONG    UseCount;
    BPTR    SegPtr;
    unsigned char    SegName[32];
} SegNode;

extern struct ExecBase *SysBase;
extern DosLibrary *DOSBase;

/*
 *  Search the resident list for the specified command returning its segment.
 *  case insensitive (for OS 1.3 only)
 */

long
_SearchResident(cmd)
char *cmd;
{
    SegNode *node;
    DosInfo *di = BTOC(((RootNode *)DOSBase->dl_Root)->rn_Info, DosInfo);
    short len = strlen(cmd);

    Forbid();
    for (node = BTOC(di->di_NetHand, SegNode); node; node = BTOC(node->NextEntry, SegNode)) {
        if (node->SegName[0] == len) {
            short i;
            for (i = 0; i < len; ++i) {
                if ((node->SegName[1+i] ^ cmd[i]) & ~0x20)
                    break;
            }
            if (i == len) {
                Permit();
                return((long)node->SegPtr);
            }
        }
    }
    Permit();
    return(0);
}

