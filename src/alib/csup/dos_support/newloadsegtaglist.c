
/*
 *  NewLoadSegTagList
 */

#include <exec/types.h>
#include <dos/dosextens.h>
#include <utility/tagitem.h>
#include <clib/dos_protos.h>

typedef struct TagItem   TagItem;

#ifndef HYPER
#define HYPER
#endif

HYPER ## NewLoadSegTagList( STRPTR file, struct TagItem *tags )
{
    return NewLoadSeg(file, tags);
}

