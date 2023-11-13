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
Prototype List  CacheList[HSIZE];
Prototype List  SuffixList;
Prototype short DDebug;

SignalSemaphore SemLock;
List    CacheList[HSIZE];
List    SuffixList;
short   DDebug = 0;

void
InitC(void)
{
    short i;
    List *list;

    Forbid();
    CacheMax     = (AvailMem(MEMF_CHIP) + AvailMem(MEMF_FAST)) / 4;
    if (CacheMax < 32768)
        CacheMax = 32768;
    CacheMaxFile = CacheMax >> 2;
    Permit();

    NewList(&SuffixList);
    for (i = 0, list = CacheList; i < HSIZE; ++i, ++list)
        NewList(list);
    InitSemaphore(&SemLock);
}

void
UnInitC(void)
{
    short i;

    {
        List *list;
        CacheNode *cnode;

        for (i = 0, list = CacheList; i < HSIZE; ++i, ++list) {
            while (cnode = GetHead(list))
                DiceCacheClose(cnode);
        }
    }
    {
        Node *node;

        while (node = RemHead(&SuffixList)) {
            FreeMem(node, sizeof(Node) + strlen(node->ln_Name) + 1);
        }
    }
}

