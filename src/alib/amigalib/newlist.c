
/*
 *  NewList.C
 *
 */

#include <exec/types.h>
#include <exec/lists.h>
#ifdef INCLUDE_VERSION	    /*	2.0 */
#include <clib/alib_protos.h>
#endif

#ifndef HYPER
#define HYPER
#endif

void
HYPER ## NewList(list)
struct List *list;
{
    list->lh_Head     = (struct Node *)&list->lh_Tail;
    list->lh_Tail     = NULL;
    list->lh_TailPred = (struct Node *)&list->lh_Head;
}

