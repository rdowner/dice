
/*
 *  $VER: lists.h 1.0 (17.4.93)
 *
 * (c)Copyright 1992 Obvious Implementations Corp, All Rights Reserved
 */

#ifndef LISTS_H
#define LISTS_H

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif
#ifndef EXEC_LISTS_H
#include <exec/lists.h>
#endif

extern void *GetHead(void *);
extern void *GetTail(void *);
extern void *GetSucc(void *);
extern void *GetPred(void *);

#endif

