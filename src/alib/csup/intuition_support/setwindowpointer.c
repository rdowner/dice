
/* Intuition tag call */

#include <exec/types.h>
#include <clib/intuition_protos.h>

#ifndef HYPER
#define HYPER
#endif

typedef struct TagItem   TagItem;

void 
HYPER ## SetWindowPointer( struct Window *win, unsigned long tag1, ... )
{
        SetWindowPointerA(win, (TagItem *)&tag1 );
}

