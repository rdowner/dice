/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  CACHE.C
 */

#include "defs.h"

Prototype CacheNode *FindCache(BPTR, char *);
Prototype void AddCache(CacheNode *);
Prototype void ClearCache(long);

CacheNode *
FindCache(lock, buf)
BPTR lock;
char *buf;
{
    long hv = ((FileLock *)((long)lock << 2))->fl_Key;
    List *list;
    CacheNode *cn;
    __aligned FileInfoBlock fib;

    dbprintf(("HV %08lx %d (%s) ", hv, hv & HMASK, buf));

    list = CacheList + (hv & HMASK);
    for (cn = GetHead(list); cn; cn = GetSucc(&cn->cn_Node)) {
	dbprintf(("TEST1 %s\n", cn->cn_Node.ln_Name));
	if (cn->cn_Flags & CNF_UNFIND)
	    continue;
	dbprintf(("TEST2 %08lx %08lx %d\n",
	    hv,
	    ((FileLock *)((long)cn->cn_Lock << 2))->fl_Key,
	    MySameLock(lock, cn->cn_Lock)
	));

	if (hv == ((FileLock *)((long)cn->cn_Lock << 2))->fl_Key && MySameLock(lock, cn->cn_Lock) == LOCK_SAME) {
	    /*
	     *	In correct directory, check filename
	     */
	    if (stricmp(buf, cn->cn_Node.ln_Name) == 0)
		break;
	}
    }

    /*
     *	Ensure that the file is not out of date
     */

    if (cn) {
	BPTR lock2;

	dbprintf(("FOUND\n"));
	cn->cn_Flags |= CNF_UNFIND;

	lock = CurrentDir(lock);
	if (lock2 = Lock(buf, SHARED_LOCK)) {
	    if (Examine(lock2, &fib)) {
		dbprintf(("CHECKDATE %08lx %08lx",
		    fib.fib_Date.ds_Days + fib.fib_Date.ds_Minute + fib.fib_Date.ds_Tick,
		    cn->cn_Fib.fib_Date.ds_Days + cn->cn_Fib.fib_Date.ds_Minute + cn->cn_Fib.fib_Date.ds_Tick
		));

		if (fib.fib_Date == cn->cn_Fib.fib_Date) {
		    dbprintf(("OK\n"));
		    cn->cn_Flags &= ~CNF_UNFIND;
		} else {
		    dbprintf(("NOT OK\n"));
		}
	    }
	    UnLock(lock2);
	}
	lock = CurrentDir(lock);
	if (cn->cn_Flags & CNF_UNFIND) {
	    if (cn->cn_Refs == 0)
		DiceCacheClose(cn);
	    cn = NULL;
	}
    } else {
	dbprintf(("NOT FOUND\n"));
    }
    return(cn);
}

void
AddCache(cn)
CacheNode *cn;
{
    long hv = ((FileLock *)((long)cn->cn_Lock << 2))->fl_Key;
    List *list;
    BPTR lock;
    BPTR lock2;

    list = CacheList + (hv & HMASK);
    AddHead(list, &cn->cn_Node);

    lock = CurrentDir(cn->cn_Lock);
    if (lock2 = Lock(cn->cn_Node.ln_Name, SHARED_LOCK)) {
	Examine(lock2, &cn->cn_Fib);
	UnLock(lock2);
    }
    CurrentDir(lock);
}

/*
 *  ClearCache(need)
 *
 *  we need <need> bytes.  The cache currently contains CachedBytes,
 *  a maximum of CacheMax.  We need to remove P percent of our cache
 *
 *	clear = need - (CacheMax - CachedBytes)     bytes to clear
 *
 */

void
ClearCache(bytes)
long bytes;
{
    long clear = bytes - (CacheMax - (CachedBytes - CachedBytesLocked));
    long index = rand() % ((CachedBytes - CachedBytesLocked) - clear);
    short i;
    List  *list;
    CacheNode *cn;
    CacheNode *cn_next;

    if (clear <= 0)
	return;
    if (clear > CachedBytes - CachedBytesLocked)
	index = 0;

    for (i = 0, list = CacheList; i < HSIZE; ++i, ++list) {
	for (cn = GetHead(list); cn; cn = cn_next) {
	    cn_next = GetSucc(&cn->cn_Node);
	    if (cn->cn_Refs == 0) {
		if (index < cn->cn_Bytes) {
		    index -= cn->cn_Bytes;
		    DiceCacheClose(cn);
		} else {
		    index -= cn->cn_Bytes;
		}
	    }
	    if (index + clear <= 0)
		return;
	}
    }
}

