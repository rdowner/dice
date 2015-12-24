
/*
 *  CreateNewProcTagList
 */

#include <exec/types.h>
#include <dos/dosextens.h>
#include <utility/tagitem.h>
#include <clib/dos_protos.h>

#ifndef HYPER
#define HYPER
#endif

typedef struct TagItem	 TagItem;
typedef struct Process	 Process;

struct Process *
HYPER ## CreateNewProcTagList( struct TagItem *tags )
{

    return CreateNewProc(tags);
}

