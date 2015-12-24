/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  CORE.C - core overlay routines
 */

#include "defs.h"

Prototype void InitCore(void);
Prototype GEntry *MakeGEntry(GEntry *groot, char *path, short len, long mode);
Prototype GHandle *MakeGHandle(GEntry *gentry, long mode);
Prototype void FreeGHan(GHandle *ghan);
Prototype void FreeGEntry(GEntry *gentry, long mode);
Prototype void FixFileInfo(FileInfoBlock *fib);
Prototype void SetGEntry(GEntry *gentry, FileInfoBlock *fib);
Prototype long WriteDataGEntry(GEntry *gentry,long pos, char *buf, long bytes);
Prototype long ReadDataGEntry(GEntry *gentry,long pos, char *buf, long bytes);

GEntry *AllocGEntry(GEntry *par, long lock, char *ptr, short len);
short NextSegment(char **pptr, short *plen, char *path, short len);

Prototype GEntry GRoot;
Prototype long LockRefs;

GEntry GRoot;
long   LockRefs;

void
InitCore()
{
    GRoot.ge_Node.ln_Name = DevName;
    GRoot.ge_LCRefs = 1;
    GRoot.ge_Flags |= GEF_DIRECTORY;
    NewList(&GRoot.ge_List);
}

/*
 *  Core of the system
 */

GEntry *
MakeGEntry(GEntry *groot, char *path, short len, long mode)
{
    char *sptr = path;
    short slen = 0;
    short softRootFlag = 0;
    GEntry *gsoftlink = NULL;

    ++groot->ge_LCRefs;
    ++LockRefs;
    while (groot && NextSegment(&sptr, &slen, path, len)) {
    	GEntry *gnew;

	// ignore device spec (total garbage because assignment specs
	// would also be there).  However, use the colon for softlinks

	if (sptr[slen-1] == ':' && softRootFlag == 0)
	    continue;

	if (groot == &GRoot || softRootFlag) {
	    /*
	     * Device specified, set to root if for us else look for the
	     * device and lock it's root.
	     */
	    short sslen = slen;

	    if (sptr[slen-1] == '/' || sptr[slen-1] == ':')
		--sslen;

	    dbprintf(("segment %.*s\n", sslen, sptr));

	    gnew = GetHead(&GRoot.ge_List);
	    while (gnew) {
		if ((ubyte)gnew->ge_Node.ln_Name[0] == sslen) {
		    if (strnicmp(gnew->ge_Node.ln_Name + 1, sptr, sslen) == 0)
			break;
		}
		gnew = GetSucc(&gnew->ge_Node);
	    }
	    if (gnew == NULL) {
	    	MsgPort *port;
		BPTR lock;

		if (port = FindDosDevice(sptr, sslen, &lock)) {
		    if (port == PktPort) {
		    	gnew = &GRoot;
		    } else {
			if (lock == 0)
		    	    lock = LockPacketPort(port, "", 0);
			if (lock) {
		    	    gnew = AllocGEntry(&GRoot, lock, sptr, sslen);
			    gnew->ge_Flags |= GEF_DIRECTORY;
			}
		    }
		}
	    }
	} else if (slen == 1 && sptr[0] == '/') {
	    /*
	     * Back one directory
	     */
	    if ((gnew = groot->ge_Parent) == NULL)
		gnew = &GRoot;
	} else {
	    /*
	     * Directory or file element
	     */
	    short sslen = (sptr[slen-1] == '/') ? slen - 1 : slen;

	    gnew = GetHead(&groot->ge_List);
	    while (gnew) {
		if ((ubyte)gnew->ge_Node.ln_Name[0] == sslen) {
		    if (strnicmp(gnew->ge_Node.ln_Name + 1, sptr, sslen) == 0)
			break;
		}
		gnew = GetSucc(&gnew->ge_Node);
	    }
	    if (gnew == NULL && groot != &GRoot) {
		long lock;

		/*
		 * the last element may not exist for modes 1006 so we 
		 * create an entry anyway with a lock field of 0.  If
		 * the open fails later on, the entry will be deleted
		 */

		if (sptr + slen == path + len && mode == 1006) {
		    gnew = AllocGEntry(groot, 0, sptr, sslen);
		} else if (lock = LockPacket(groot->ge_Lock, sptr, sslen)) {
		    __aligned FileInfoBlock fib;

		    fib.fib_Size = 0;
		    ExaminePacket(lock, &fib);
		    gnew = AllocGEntry(groot, lock, sptr, sslen);
		    if (gnew)
			SetGEntry(gnew, &fib);
		}
	    }
	}
	softRootFlag = 0;
	if (gnew) {
	    ++gnew->ge_LCRefs;
	    ++LockRefs;

	    // If we encountered a softlink the above reference is 
	    // left for gsoftlink and we follow the softlink relative
	    // to the previous directory instead of relative to the gnew
	    // lock.

	    if (gnew->ge_SoftLink) {
		if (gsoftlink)
		    FreeGEntry(gsoftlink, 1005);
		gsoftlink = gnew;
		path = gnew->ge_SoftLink;
		len  = strlen(gnew->ge_SoftLink);
		sptr = path;
		slen = 0;
		if (strchr(path, ':'))
		    softRootFlag = 1;

		gnew = groot;
	    	++groot->ge_LCRefs;
	    	++LockRefs;
	    }
	}
	FreeGEntry(groot, 1005);
	groot = gnew;
    }
    if (gsoftlink)
	FreeGEntry(gsoftlink, 1005);
    return(groot);
}

GEntry *
AllocGEntry(GEntry *gparent, long lock, char *sptr, short slen)
{
    GEntry *gentry;

    if (gentry = AllocMem(sizeof(GEntry) + slen + 1, MEMF_PUBLIC|MEMF_CLEAR)) {
	if (GetHead(&gparent->ge_List) == NULL) {
	    ++gparent->ge_LCRefs;;
	    ++LockRefs;
	}
	AddTail(&gparent->ge_List, &gentry->ge_Node);
	NewList(&gentry->ge_List);
	gentry->ge_Parent = gparent;
	gentry->ge_Lock = lock;
	gentry->ge_Node.ln_Name = (char *)(gentry + 1);
	gentry->ge_Node.ln_Name[0] = slen;
	movmem(sptr, gentry->ge_Node.ln_Name + 1, slen);
    }
    return(gentry);
}

GHandle *
MakeGHandle(GEntry *gentry, long mode)
{
    GHandle *ghan;
    BPTR fh;
    short error = 0;

    if (ghan = AllocMem(sizeof(GHandle), MEMF_PUBLIC|MEMF_CLEAR)) {
	/*
	 * Initialize handle
	 */

	ghan->gh_Mode = mode;
	ghan->gh_GEntry = gentry;

	/*
	 * Bump gentry FHRefs, if going from 0->1 decompress and load file
	 */

	if (gentry->ge_FHRefs++ == 0) {
	    ++gentry->ge_LCRefs;
	    ++LockRefs;
	    dbprintf(("Decomp: open\n"));

	    if (mode == 1006) {
		if (gentry->ge_Lock == 0) {
		    ubyte *name = gentry->ge_Node.ln_Name + 1;
		    short len = name[-1];
		    BPTR plock = gentry->ge_Parent->ge_Lock;

		    if (fh = OpenPacket(plock, name, len, 1006)) {
			ClosePacket(fh);
			gentry->ge_Lock = LockPacket(plock, name, len);
		    } else {
			error = 1;
		    }
		} else {
		    error = 1;
		}
	    } 
	}
    }
    if (error) {
	FreeGHan(ghan);
	ghan = NULL;
    }
    return(ghan);
}

long
ReadDataGEntry(GEntry *gentry, long pos, char *buf, long bytes)
{
    long n = 0;

    if (pos >= 0 && pos < gentry->ge_Bytes) {
	/*
	 * If not cached we must read the file, but only allocate a cache
	 * if the reader is reading less then the file size.  This optimizes
	 * whole-file reads in two ways: (1) no intermediate buffer is 
	 * allocated which saves memory and (2) we read/decompress directly
	 * to the reader's buffer.
	 */

	if ((gentry->ge_Flags & GEF_CACHED) == 0) {
	    BPTR fh;

	    if (fh = OpenPacket(gentry->ge_Lock, "", 0, 1005)) {
		if (pos == 0 && bytes >= gentry->ge_Bytes) {
		    dbprintf(("READ: DIRECT %d\n", bytes));
		    if (gentry->ge_Flags & GEF_COMPRESSED)
		        DeCompress(fh, buf, gentry->ge_Bytes);
		    else
			ReadPacket(fh, buf, gentry->ge_Bytes);
		    n = gentry->ge_Bytes;
		} else {
		    dbprintf(("READ: CACHELD %d\n", gentry->ge_Bytes));
		    gentry->ge_Buf = AllocMem(gentry->ge_Bytes+1,MEMF_PUBLIC);
		    if (gentry->ge_Buf) {
			if (gentry->ge_Flags & GEF_COMPRESSED)
			    DeCompress(fh, gentry->ge_Buf, gentry->ge_Bytes);
			else
			    ReadPacket(fh, gentry->ge_Buf, gentry->ge_Bytes);
			gentry->ge_Max = gentry->ge_Bytes + 1;
			gentry->ge_Flags |= GEF_CACHED;
		    }
		}
		ClosePacket(fh);
	    }
	}

	/*
	 * Copy data from cache
	 */

	if (gentry->ge_Flags & GEF_CACHED) {
	    n = gentry->ge_Bytes - pos;

	    if (n > bytes)
		n = bytes;
	    movmem(gentry->ge_Buf + pos, buf, n);
	    dbprintf(("READ: CACHED %d\n", bytes));
	}
    }
    return(n);
}

long
WriteDataGEntry(GEntry *gentry, long pos, char *buf, long bytes)
{
    long n = -ERROR_NO_FREE_STORE;

    /*
     * If file is not cached and not empty we have to cache what we have
     * first!
     */

    if ((gentry->ge_Flags & GEF_CACHED) == 0 && gentry->ge_Bytes) {
	BPTR fh;

	if (fh = OpenPacket(gentry->ge_Lock, "", 0, 1005)) {
	    gentry->ge_Buf = AllocMem(gentry->ge_Bytes,MEMF_PUBLIC);
	    if (gentry->ge_Buf) {
	        if (gentry->ge_Flags & GEF_COMPRESSED)
		    DeCompress(fh, gentry->ge_Buf, gentry->ge_Bytes);
		else
		    ReadPacket(fh, gentry->ge_Buf, gentry->ge_Bytes);
		gentry->ge_Max = gentry->ge_Bytes;
		gentry->ge_Flags |= GEF_CACHED;
	    }
	    ClosePacket(fh);
	}
    }

    /*
     *  Extend buffer if necessary
     *  Copy data in
     */

    /* printf("before pos %d bytes %d gentry %d/%d\n", pos, bytes, gentry->ge_Bytes, gentry->ge_Max); */

    {
	long cbytes = pos + bytes;

	if (cbytes > gentry->ge_Max) {
	    ubyte *xbuf;

	    if (xbuf = AllocMem(cbytes + (cbytes >> 1), MEMF_PUBLIC)) {
		movmem(gentry->ge_Buf, xbuf, gentry->ge_Bytes);
		if (gentry->ge_Max)
		    FreeMem(gentry->ge_Buf, gentry->ge_Max);
		gentry->ge_Buf = xbuf;
		gentry->ge_Max = cbytes + (cbytes >> 1);
	    }
	}
	if (cbytes <= gentry->ge_Max) {
	    movmem(buf, gentry->ge_Buf + pos, bytes);
	    n = bytes;
	    if (cbytes > gentry->ge_Bytes)
		gentry->ge_Bytes = cbytes;
	    gentry->ge_Flags |= GEF_DIRTY | GEF_CACHED;
	}
    }
    /* printf("after pos %d bytes %d gentry %d/%d\n", pos, bytes, gentry->ge_Bytes, gentry->ge_Max); */

    return(n);
}

void 
FreeGHan(GHandle *ghan)
{
    GEntry *gentry = ghan->gh_GEntry;

    /*
     *  Decrement gentry FHRefs.  If 0 and buffer is dirty, compress and
     *  write out buffer.  If buffer is not dirty simply free the memory.
     */

    if (--gentry->ge_FHRefs == 0)
	FreeGEntry(gentry, ghan->gh_Mode);
    ghan->gh_GEntry = NULL;
    FreeMem(ghan, sizeof(GHandle));
}

void 
FreeGEntry(GEntry *gentry, long mode)
{
    --LockRefs;
    if (--gentry->ge_LCRefs == 0) {
    	GEntry *gparent = gentry->ge_Parent;

	if (gentry->ge_Lock)
	    UnLockPacket(gentry->ge_Lock);
	gentry->ge_Lock = 0;
	Remove(&gentry->ge_Node);

	if (gentry->ge_Flags & GEF_DIRTY) {
	    BPTR fh;

	    fh = OpenPacket(
		gparent->ge_Lock, 
		gentry->ge_Node.ln_Name + 1, 
		(ubyte)gentry->ge_Node.ln_Name[0], 
		1006
	    );
	    if (fh) {
		if (Compress(fh, gentry->ge_Buf, gentry->ge_Bytes))
		    gentry->ge_Flags |= GEF_COMPRESSED;
		else
		    gentry->ge_Flags &= ~GEF_COMPRESSED;

		ClosePacket(fh);
	        SetCommentPacket(
		    gparent->ge_Lock,
		    gentry->ge_Node.ln_Name + 1,
		    (ubyte)gentry->ge_Node.ln_Name[0], 
		    "",
		    0,
		    gentry
	    	);
	    }
	    gentry->ge_Flags &= ~GEF_DIRTY;
	}

	if (gentry->ge_Buf)
	    FreeMem(gentry->ge_Buf, gentry->ge_Max);
	gentry->ge_Buf = NULL;
	gentry->ge_Max = 0;

    	if (gentry->ge_SoftLink) {
	    FreeMem(gentry->ge_SoftLink, strlen(gentry->ge_SoftLink) + 1);
	    gentry->ge_SoftLink = NULL;
        }
	FreeMem(gentry, sizeof(GEntry)+(ubyte)gentry->ge_Node.ln_Name[0]+1);
	if (GetHead(&gparent->ge_List) == NULL) {
    	    FreeGEntry(gparent, 1005);
	}
    }
}

/*
 * Use comment field to determine file size. Look for
 * ##%08lx##
 */

void 
FixFileInfo(FileInfoBlock *fib)
{
    ubyte len = (ubyte)fib->fib_Comment[0];

    if (len >= 12) {
	if (fib->fib_Comment[1] == '#' 
	&& fib->fib_Comment[2] == '#'
	&& fib->fib_Comment[11] == '#'
	&& fib->fib_Comment[12] == '#') {
	    /*
	     * Actual file size after decompression
	     */

	    fib->fib_Size = strtol(fib->fib_Comment + 3, NULL, 16);

	    /*
	     * Make comment prefix invisible
	     */
	    movmem(fib->fib_Comment + 13, fib->fib_Comment + 1, len - 12 + 1);
	    fib->fib_Comment[0] -= 12;
	}
    }
}

void
SetGEntry(GEntry *gentry, FileInfoBlock *fib)
{
    if (fib->fib_DirEntryType > 0)
	gentry->ge_Flags |= GEF_DIRECTORY;

    fib->fib_Comment[1+(ubyte)fib->fib_Comment[0]] = 0;

    if ((ubyte)fib->fib_Comment[0] >= 12) {
	if (fib->fib_Comment[1] == '#' 
	&& fib->fib_Comment[2] == '#'
	&& fib->fib_Comment[11] == '#'
	&& fib->fib_Comment[12] == '#') {
	    fib->fib_Size = strtol(fib->fib_Comment + 3, NULL, 16);
	    gentry->ge_Flags |= GEF_COMPRESSED;
	}
    }
    gentry->ge_Bytes = fib->fib_Size;

    if (gentry->ge_SoftLink) {
	FreeMem(gentry->ge_SoftLink, strlen(gentry->ge_SoftLink) + 1);
	gentry->ge_SoftLink = NULL;
    }
    {
	char *sl;

    	if (sl = strstr(fib->fib_Comment + 1, "SOFTLINK=")) {
	    sl += 9;
	    gentry->ge_SoftLink = AllocMem(strlen(sl) + 1, MEMF_PUBLIC);
	    strcpy(gentry->ge_SoftLink, sl);
	}
    }
}

short 
NextSegment(char **pptr, short *plen, char *path, short len)
{
    char *ptr;
    char *pathEnd = path + len;

    *pptr += *plen;
    for (ptr = *pptr; ptr < pathEnd; ++ptr) {
	if (*ptr == '/' || *ptr == ':') {
	    ++ptr;
	    break;
	}
    }
    *plen = ptr - *pptr;
    if (*plen == 0)
	return(0);
    return(1);
}

