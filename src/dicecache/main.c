/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  MAIN.C
 */

#include "defs.h"

#ifdef AMIGA
#include <lib/version.h>

#ifdef _DCC
IDENT("dicecache.library",".58");
DCOPYRIGHT;
#endif
#endif

Prototype LibCall CacheNode *DiceCacheOpen(__A0 const char *, __A1 const char *, __A2 long *);
Prototype LibCall void DiceCacheClose(__A0 CacheNode *);
Prototype LibCall void *DiceCacheSeek(__A0 CacheNode *, __D0 long, __A1 long *);
Prototype LibCall long DiceCacheGetSuffixes(__A0 char *, __D0 long);
Prototype LibCall long DiceCacheAddSuffix(__A0 const char *);
Prototype LibCall long DiceCacheRemSuffix(__A0 const char *);
Prototype LibCall void DiceCacheFlush(__D0 long);
Prototype LibCall void DiceCacheSet(__A0 long *, __D0 long);
Prototype LibCall void DiceCacheGet(__A0 long *, __D0 long);
Prototype LibCall void DiceCacheEnable(void);
Prototype LibCall void DiceCacheDisable(void);

Prototype char	  Buf[1024];
Prototype long	  CachedBytes;
Prototype long	  CachedBytesLocked;
Prototype long	  CacheMaxFile;
Prototype long	  CacheMax;
Prototype short   CacheEnabled;

char	Buf[1024];
long	CachedBytes;
long	CachedBytesLocked;
long	CacheMaxFile;
long	CacheMax;
long	CacheHits1;
long	CacheHits2;
long	CacheAttempts1;
long	CacheAttempts2;
short	CacheEnabled;

LibCall
CacheNode *
DiceCacheOpen(fileName, fileModes, psize)
__A0 const char *fileName;
__A1 const char *fileModes;
__A2 long *psize;
{
    BPTR lock;
    CacheNode *cn;

    /*
     *	Only for read access
     */

    *psize = 0;
    if (CacheEnabled == 0)
	return(NULL);

    dbprintf(("Cached %d/%d/%d\n", CachedBytesLocked, CachedBytes, CacheMax));

    if (strchr(fileModes, '+') || strchr(fileModes, 'a') || strchr(fileModes, 'w'))
	return(NULL);

    ObtainSemaphore(&SemLock);

    dbprintf(("open/lock\n"));

    /*
     *	Step 1, check for suffix match
     */

    {
	short len = strlen(fileName);
	Node *node;

	for (node = GetHead(&SuffixList); node; node = GetSucc(node)) {
	    if (node->ln_Pri < len && stricmp(node->ln_Name, fileName + len - node->ln_Pri) == 0)
		break;
	}
	if (node == NULL) {
	    ReleaseSemaphore(&SemLock);
	    *psize = PS_NO_MATCH;
	    return(NULL);
	}
    }

    /*
     *	Step 2, obtain lock on directory containing file and determine
     *	whether the file is cached.
     */

    {
	short len = MyPathPart(fileName) - fileName;
	strncpy(Buf, fileName, len);
	Buf[len] = 0;
    }

    if ((lock = Lock(Buf, SHARED_LOCK)) == NULL) {
	ReleaseSemaphore(&SemLock);
	*psize = PS_LOCK_FAILED;
	return(NULL);
    }

    *psize = PS_GENERAL_FAILURE;    /*	replaced w/ size if success */

    strcpy(Buf, MyFilePart(fileName));

    if (CacheAttempts1 == 100) {
	CacheAttempts2 = CacheAttempts1;
	CacheHits2 = CacheHits1;
	CacheAttempts1 = 0;
	CacheHits1 = 0;
    }
    ++CacheAttempts1;
    if (cn = FindCache(lock, Buf)) {
	++CacheHits1;

	UnLock(lock);
	if (cn->cn_Refs++ == 0)
	    CachedBytesLocked += cn->cn_Bytes;
	*psize = cn->cn_Bytes;
    } else {
	long siz;
	long fh;

	if (fh = Open(fileName, 1005)) {
	    Seek(fh, 0L, 1);
	    if ((siz = Seek(fh, 0L, -1)) > 0 && siz < CacheMaxFile) {
		ClearCache(siz);
		if (cn = AllocMem(sizeof(CacheNode) + strlen(Buf) + 1, MEMF_PUBLIC|MEMF_CLEAR)) {
		    cn->cn_Lock = lock;
		    cn->cn_Node.ln_Name = (char *)(cn + 1);
		    strcpy(cn->cn_Node.ln_Name, Buf);
		    AddCache(cn);

		    if (cn->cn_Data = AllocMem(siz + 4, MEMF_PUBLIC)) {
			*(long *)((char *)cn->cn_Data + siz) = 0;
			cn->cn_Bytes = siz;
			CachedBytes += siz;

			if (Read(fh, cn->cn_Data, siz) == siz) {
			    cn->cn_Flags |= CNF_VALID;
			    *psize = siz;
			}
		    }
		}
	    }
	    Close(fh);
	} else {
	    /*
	     *	If the file doesn't exist then it isn't really our fault.
	     */

	    ++CacheHits1;
	}
	if (cn) {
	    if ((cn->cn_Flags & CNF_VALID) == 0) {
		DiceCacheClose(cn);
		cn = NULL;
	    } else {
		++cn->cn_Refs;
		CachedBytesLocked += cn->cn_Bytes;
	    }
	} else {
	    UnLock(lock);
	}
    }

    ReleaseSemaphore(&SemLock);
    return(cn);
}

LibCall
void
DiceCacheClose(cn)
__A0 CacheNode *cn;
{
    ObtainSemaphore(&SemLock);
    if (--cn->cn_Refs == 0) {
	CachedBytesLocked -= cn->cn_Bytes;
	if (cn->cn_Flags & CNF_UNFIND)
	    --cn->cn_Refs;
    }
    if (cn->cn_Refs < 0) {
	Remove(&cn->cn_Node);
	if (cn->cn_Lock) {
	    UnLock(cn->cn_Lock);
	    cn->cn_Lock = 0;
	}
	if (cn->cn_Bytes) {
	    FreeMem(cn->cn_Data, cn->cn_Bytes + 4);
	    CachedBytes -= cn->cn_Bytes;
	}
	FreeMem(cn, sizeof(CacheNode) + strlen(cn->cn_Node.ln_Name) + 1);
    }
    ReleaseSemaphore(&SemLock);
}

LibCall
void *
DiceCacheSeek(cn, off, pbsize)
__A0 CacheNode *cn;
__D0 long off;
__A1 long *pbsize;
{
    if (off < 0 || off > cn->cn_Bytes) {
	*pbsize = 0;
	return(NULL);
    }
    *pbsize = cn->cn_Bytes - off;
    return((void *)(cn->cn_Data + off));
}

LibCall
long
DiceCacheGetSuffixes(buf, max)
__A0 char *buf;
__D0 long max;
{
    Node *node;
    long r = 0;
    short first = 0;
    char *ptr = buf;

    --max;

    ObtainSemaphore(&SemLock);
    for (node = GetHead(&SuffixList); node; node = GetSucc(node)) {
	short len = strlen(node->ln_Name);

	if (len >= max) {
	    r -= len + 1;
	} else {
	    if (first == 0) {
		*ptr++ = ' ';
		--max;
	    }
	    strcpy(ptr, node->ln_Name);
	    ptr += len;
	    max -= len;
	}
    }
    ReleaseSemaphore(&SemLock);
    if (r < 0)
	return(r);
    return(ptr - buf);
}

LibCall
long
DiceCacheAddSuffix(buf)
__A0 const char *buf;
{
    Node *node;
    long r;

    ObtainSemaphore(&SemLock);
    DiceCacheRemSuffix(buf);
    if (node = AllocMem(sizeof(Node) + strlen(buf) + 1, MEMF_PUBLIC|MEMF_CLEAR)) {
	node->ln_Name = (char *)(node + 1);
	node->ln_Pri = strlen(buf);
	strcpy(node->ln_Name, buf);
	AddHead(&SuffixList, node);
	r = 0;
    } else {
	r = -1;
    }
    ReleaseSemaphore(&SemLock);
    return(r);
}

LibCall
long
DiceCacheRemSuffix(buf)
__A0 const char *buf;
{
    Node *node;
    long r = -1;

    ObtainSemaphore(&SemLock);
    for (node = GetHead(&SuffixList); node; node = GetSucc(node)) {
	if (stricmp(buf, node->ln_Name) == 0) {
	    Remove(node);
	    FreeMem(node, sizeof(Node) + strlen(node->ln_Name) + 1);
	    r = 0;
	    break;
	}
    }
    ReleaseSemaphore(&SemLock);
    return(r);
}

LibCall
void
DiceCacheFlush(bytes)
__D0 long bytes;
{
    ObtainSemaphore(&SemLock);
    ClearCache(bytes);
    ReleaseSemaphore(&SemLock);
}

LibCall
void
DiceCacheSet(la, n)
__A0 long *la;
__D0 long n;
{
    short i;

    ObtainSemaphore(&SemLock);
    for (i = 0; i < n && i < 4; ++i, ++la) {
	switch(i) {
	case 0:
	    /*CachedBytesLocked = *la;*/
	    break;
	case 1:
	    /*CachedBytes = *la;*/
	    break;
	case 2:
	    CacheMax = *la;
	    break;
	case 3:
	    CacheMaxFile = *la;
	    break;
	}
    }
    ClearCache(CachedBytes - CacheMax);
    ReleaseSemaphore(&SemLock);
}

LibCall
void
DiceCacheGet(la, n)
__A0 long *la;
__D0 long n;
{
    short i;

    ObtainSemaphore(&SemLock);
    for (i = 0; i < n; ++i, ++la) {
	switch(i) {
	case 0:
	    *la = CachedBytesLocked;
	    break;
	case 1:
	    *la = CachedBytes;
	    break;
	case 2:
	    *la = CacheMax;
	    break;
	case 3:
	    *la = CacheMaxFile;
	    break;
	case 4:
	    *la = CacheHits2 + CacheHits1;
	    break;
	case 5:
	    *la = CacheAttempts2 + CacheAttempts1;
	    break;
	default:
	    *la = 0;
	    break;
	}
    }
    ReleaseSemaphore(&SemLock);
}

LibCall
void
DiceCacheEnable(void)
{
    ObtainSemaphore(&SemLock);
    if (CacheEnabled == 0) {
	CacheEnabled = 1;
	++LibBase->lib_OpenCnt;
    }
    ReleaseSemaphore(&SemLock);
}

LibCall
void
DiceCacheDisable(void)
{
    ObtainSemaphore(&SemLock);
    if (CacheEnabled) {
	CacheEnabled = 0;
	--LibBase->lib_OpenCnt;
    }
    ReleaseSemaphore(&SemLock);
}



