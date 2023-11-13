
/*
 *  System
 */

#include <exec/types.h>
#include <dos/dosextens.h>
#include <utility/tagitem.h>
#include <clib/dos_protos.h>

#ifndef HYPER
#define HYPER
#endif

typedef struct TagItem   TagItem;

LONG
HYPER ## System(cmd, tags)
UBYTE *cmd;
TagItem *tags;
{
    return(SystemTagList(cmd, tags));
}

