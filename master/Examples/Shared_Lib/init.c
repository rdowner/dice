/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  INIT.C
 */

#include "defs.h"

Prototype void InitC(void);
Prototype void UnInitC(void);

Prototype SignalSemaphore SemLock;
Prototype List            StrList;

SignalSemaphore SemLock;
List            StrList;

void
InitC(void)
{
    NewList(&StrList);
    InitSemaphore(&SemLock);
}

void
UnInitC(void)
{
    Node *node;

    while (node = RemHead(&StrList))
        FreeMem(node, sizeof(Node) + strlen(node->ln_Name) + 1);
}

