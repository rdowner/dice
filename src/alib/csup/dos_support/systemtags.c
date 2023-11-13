
/*
 *  SystemTags
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
HYPER ## SystemTags(cmd, tag1, ...)
UBYTE *cmd;
unsigned long tag1;
{
    return(SystemTagList(cmd, (TagItem *)&tag1));
}

