/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  LOCK.C
 */

#include "defs.h"

#ifdef NOTDEF
/*
 *  library interface
 */

Prototype LibCall void LockHDDB(void);
Prototype LibCall void UnLockHDDB(void);

/*
 *  library local
 */

LibCall void
LockHDDB(void)
{
    ObtainSemaphore(&SemLock);
}

LibCall void
UnLockHDDB(void)
{
    ReleaseSemaphore(&SemLock);
}

#endif
