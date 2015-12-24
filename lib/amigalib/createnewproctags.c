
/*
 *  CreateNewProcTags
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 *
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

Process *
HYPER ## CreateNewProcTags(tag1, ...)
unsigned long tag1;
{
    return(CreateNewProc((TagItem *)&tag1));
}

