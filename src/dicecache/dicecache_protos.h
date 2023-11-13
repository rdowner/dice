
/*
 *  CLIB/DICECACHE_PROTOS.H
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#ifndef CLIB_DICECACHE_PROTOS_H
#define CLIB_DICECACHE_PROTOS_H 1

/*
 *  If DiceCacheOpen() returns NULL, *psize contains:
 */

#define PS_CACHE_DISABLED   0   /*  disabled or not read-only   */
#define PS_NO_MATCH         1
#define PS_LOCK_FAILED      2
#define PS_GENERAL_FAILURE  9

/*
 *
 */

#ifndef _DICECACHE_INTERNAL_SKIP

typedef void *CacheNode;

CacheNode *DiceCacheOpen(const char *, const char *, long *);
void DiceCacheClose(CacheNode *);
void *DiceCacheSeek(CacheNode *, long, long *);
long DiceCacheGetSuffixes(char *, long);
long DiceCacheAddSuffix(const char *);
long DiceCacheRemSuffix(const char *);
void DiceCacheFlush(long);
void DiceCacheSet(long *, long);
void DiceCacheGet(long *, long);
void DiceCacheEnable(void);
void DiceCacheDisable(void);

#endif
#endif

