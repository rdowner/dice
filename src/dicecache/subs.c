/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  SUBS.C
 */

#include "defs.h"

Prototype void xprintf(const char *, ...);
Prototype char *MyFilePart(const char *);
Prototype char *MyPathPart(const char *);
Prototype long MySameLock(BPTR, BPTR);

void
xprintf(ctl, ...)
const char *ctl;
{
    va_list va;

    va_start(va, ctl);
    vfhprintf(Output(), ctl ,va);
    va_end(va);
}

char *
MyFilePart(ptr)
const char *ptr;
{
    short i;

    for (i = strlen(ptr) - 1; i >= 0; --i) {
        if (ptr[i] == '/' || ptr[i] == ':')
            break;
    }
    return(ptr + i + 1);
}

char *
MyPathPart(ptr)
const char *ptr;
{
    const char *p = MyFilePart(ptr);

    if (p > ptr && p[-1] == '/')
        --p;
    return(p);
}

long
MySameLock(l1, l2)
BPTR l1;
BPTR l2;
{
    FileLock *lock1 = BTOC(l1);
    FileLock *lock2 = BTOC(l2);

    if (lock1->fl_Volume == lock2->fl_Volume) {
        if (lock1->fl_Key == lock2->fl_Key)
            return(LOCK_SAME);
        return(LOCK_SAME_HANDLER);
    }
    return(LOCK_DIFFERENT);
}

