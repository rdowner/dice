/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  FUNCS.C
 */

#include "defs.h"

/*
 *  library interface
 */

Prototype LibCall void LockTestLib(void);
Prototype LibCall void UnLockTestLib(void);
Prototype LibCall void PostString(__A0 const char *);
Prototype LibCall long GetString(__A0 char *, __D0 long);

/*
 *  library local
 */

LibCall void
LockTestLib(void)
{
    ObtainSemaphore(&SemLock);
}

LibCall void
UnLockTestLib(void)
{
    ReleaseSemaphore(&SemLock);
}

LibCall void
PostString(name)
__A0 const char *name;
{
    Node *node;

    ObtainSemaphore(&SemLock);
    node = AllocMem(sizeof(Node) + strlen(name) + 1, MEMF_PUBLIC);
    node->ln_Name = (char *)(node + 1);
    strcpy(node->ln_Name, name);
    AddTail(&StrList, node);
    ReleaseSemaphore(&SemLock);
}

/*
 *  returns actual length of returned string regardless of buffer size,
 *  but only copies max chars to the buffer (including \0 which may
 *  cut off part of the string if the string is too large to fit)
 */

LibCall long
GetString(buf, max)
__A0 char *buf;
__D0 long max;
{
    Node *node;
    long len;

    ObtainSemaphore(&SemLock);
    if (node = RemHead(&StrList)) {
	len = strlen(node->ln_Name);
	strncpy(buf, node->ln_Name, max);
	if (len >= max)
	    buf[max-1] = 0;
	FreeMem(node, sizeof(Node) + len + 1);
    } else {
	len = -1;
	if (max > 0)
	    buf[0] = 0;
    }
    ReleaseSemaphore(&SemLock);
    return (len);
}

