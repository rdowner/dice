
/*
 *  RESIDENT.C
 *
 *  Create temporary 'resident' node that LoadSeg()s a program, runs it,
 *  and replies a message.  This node is uniquely named and can only be
 *  used in a non-reentrant fashion.
 *
 *  You create the node, then Execute() the node's name... simple!
 */

#include <exec/types.h>
#include <exec/execbase.h>
#include <exec/ports.h>
#include <exec/memory.h>
#include <libraries/dos.h>
#include <libraries/dosextens.h>

typedef struct DosLibrary   DosLibrary;
typedef struct RootNode     RootNode;
typedef struct DosInfo	    DosInfo;
typedef struct CommandLineInterface CLI;

typedef struct MsgPort	    MsgPort;
typedef struct Message	    Message;
typedef struct Task	    Task;
typedef struct Process	    Process;

typedef struct SegNode {
    BPTR    NextEntry;
    LONG    UseCount;
    BPTR    SegPtr;
    BPTR    SegName;
} SegNode;


#define BTOC(bptr, type)  ((type *)((long)(bptr) << 2))
#define CTOB(cptr)  ((BPTR)((long)(cptr) >> 2))

/*
 *  Create and return the node, return node name in auxillary argument
 *
 *  When you run a program using this node a message will be returned to
 *  the specified MsgPort when the program exits, containing the exit code.
 *
 *  If you intend to run a program in the background you must create a node
 *  for each program.
 */

extern struct ExecBase *SysBase;
extern DosLibrary *DOSBase;

main(ac, av)
char *av[];
{
    DosInfo *di = BTOC(((RootNode *)DOSBase->dl_Root)->rn_Info, DosInfo);
    SegNode *sn;

    printf("root: %08lx\n", DOSBase->dl_Root);
    printf("info: %08lx\n", di);
    printf("di %08lx di->di_NetHand %08lx\n", di, di->di_NetHand);

    for (sn = BTOC(di->di_NetHand, SegNode); sn; sn = BTOC(sn->NextEntry, SegNode)) {
	int namlen = *(unsigned char *)&sn->SegName;
	printf("sn %08lx name %-15.*s seg %08lx ref %ld\n",
	    sn,
	    namlen,
	    (char *)&sn->SegName + 1,
	    sn->SegPtr,
	    sn->UseCount
	);
	if (ac == 3 && namlen == strlen(av[1]) && strncmp(av[1], (char *)&sn->SegName + 1, namlen) == 0) {
	    sn->UseCount = atoi(av[2]);
	}
    }
}

