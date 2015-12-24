/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  MAIN.C
 */

#include "defs.h"

Prototype char *DevName;
Prototype short DDebug;

void myexit(void);

char DevName[64] = { 4, "arch" };
short DDebug;

const static char Ident[] = { "$VER: REGISTERED/COMMERCIAL FSOVL-HANDLER "
  VERSION ".01R (" __COMMODORE_DATE__ ") "
  "(c)Copyright 1992-93 Obvious Implementations Corp, Redistribution and use under DICE-LICENSE.TXT\n\r"
};

void
main(ac, av)
short ac;
char *av[];
{
    DosPacket  *packet;

    {
	short i;

	for (i = 1; i < ac; ++i) {
	    char *ptr = av[i];

	    if (*ptr != '-') {
		makebstr(ptr, strlen(ptr), DevName);
		continue;
	    }
	    ptr += 2;
	    switch(ptr[-1]) {
	    case 'd':
		DDebug = (*ptr) ? strtol(ptr, NULL, 0) : 1;
		break;
	    }
	}
    }
    atexit(myexit);
    dbprintf(("Device: %s:\n", DevName + 1));
    Initialize(DevName);
    InitCore();

    /*
     *	Main Loop
     */

    for (;;) {
    	long r;

	dbprintf(("LockRefs %d\n", LockRefs));
	{
	    Message *msg;
	    long sigs;

	    while ((msg = GetMsg(PktPort)) == NULL) {
		sigs = Wait(PktPortMask | SIGBREAKF_CTRL_D);
		if ((sigs & SIGBREAKF_CTRL_D) && LockRefs == 0)
		    exit(0);
	    }
	    packet = (DosPacket *)msg->mn_Node.ln_Name;
	}

	r = -ERROR_OBJECT_NOT_FOUND;
	packet->dp_Res1 = 0;
	packet->dp_Res2 = 0;

	dbprintf(("packet %08lx (%d) %08lx %08lx %08lx %08lx\n", 
	    packet->dp_Type, packet->dp_Type,
	    packet->dp_Arg1,
	    packet->dp_Arg2,
	    packet->dp_Arg3,
	    packet->dp_Arg4
	));

	switch(packet->dp_Type) {
	case ACTION_DIE:
	    break;
	case ACTION_FINDUPDATE:     /*	FileHandle,Lock,Name	    Bool*/
	case ACTION_FINDINPUT:	    /*	FileHandle,Lock,Name	    Bool*/
	case ACTION_FINDOUTPUT:     /*	FileHandle,Lock,Name	    Bool*/
	    {
		FileHandle *fh = BTOC(packet->dp_Arg1);
		GEntry *gentry = BLockToGEntry(packet->dp_Arg2);
		short nameLen;
		char *name = BNameToPtr(packet->dp_Arg3, &nameLen);
		GHandle *ghan = NULL;

		dbprintf(("open %.*s %d\n", nameLen, name, packet->dp_Type));

		if (gentry=MakeGEntry(gentry,name,nameLen,packet->dp_Type)) {
		    if (gentry->ge_Flags & GEF_DIRECTORY)
			r = -ERROR_OBJECT_WRONG_TYPE;
		    else
		    	ghan = MakeGHandle(gentry, packet->dp_Type);
		}
		if (ghan) {
		    fh->fh_Arg1 = ghan;
		    fh->fh_Port = (MsgPort *)DOS_TRUE;
		    r = DOS_TRUE;
		    dbprintf(("Success, file size %d\n",gentry->ge_Bytes));
		} else {
		    fh->fh_Port = (MsgPort *)DOS_FALSE;
		}
		if (gentry)
		    FreeGEntry(gentry, packet->dp_Type);
	    }
	    break;
	case ACTION_END:	    /*	FHArg1			  Bool:TRUE*/
	    {
		GHandle *ghan = (GHandle *)packet->dp_Arg1;

		FreeGHan(ghan);
		r = 0;
	    }
	    break;
	case ACTION_READ:	    /*	FHArg1,CPTRBuffer,Length    ActLength*/
	    {
		GHandle *ghan = (GHandle *)packet->dp_Arg1;
		GEntry *gentry = ghan->gh_GEntry;
		long n;

		n = ReadDataGEntry(
		    gentry, 
		    ghan->gh_Pos, 
		    (void *)packet->dp_Arg2,
		    packet->dp_Arg3
		);
		if (n > 0)
		    ghan->gh_Pos += n;
		r = n;
	    }
	    break;
	case ACTION_WRITE:	    /*	FHArg1,CPTRBuffer,Length    ActLength*/
	    {
		GHandle *ghan = (GHandle *)packet->dp_Arg1;
		GEntry *gentry = ghan->gh_GEntry;
		long n;

		n = WriteDataGEntry(
		    gentry,
		    ghan->gh_Pos,
		    (void *)packet->dp_Arg2,
		    packet->dp_Arg3
		);
		if (n > 0)
		    ghan->gh_Pos += n;
		r = n;
	    }
	    break;
	case ACTION_SEEK:    /*	FHArg1,Position,Mode	    OldPosition */
	    {
		GHandle *ghan = (GHandle *)packet->dp_Arg1;
		GEntry *gentry = ghan->gh_GEntry;
		long n = packet->dp_Arg2;

		switch(packet->dp_Arg3) {
		case -1:    // relative to beginning
		    break;
		case 0:	    // relative to current
		    n += ghan->gh_Pos;
		    break;
		case 1:	    // relative to end
		    n += gentry->ge_Bytes;
		    break;
		}
		if (n < 0 || n > gentry->ge_Bytes) {
		    r = -ERROR_SEEK_ERROR;
		} else {
		    r = ghan->gh_Pos;
		    ghan->gh_Pos = n;
		}
	    }
	    break;
	case ACTION_CREATE_DIR:     /*	Lock,Name		    Lock    */
	    {
		GEntry *gentry = BLockToGEntry(packet->dp_Arg1);
		short nameLen;
		char *name = BNameToPtr(packet->dp_Arg2, &nameLen);

		r = -ERROR_OBJECT_NOT_FOUND;
		if (gentry = MakeGEntry(gentry,name,nameLen,1006)) {
		    if (gentry->ge_Parent && gentry->ge_Parent != &GRoot) {
			r = CreateDirPacket(
			    gentry->ge_Parent->ge_Lock,
			    gentry->ge_Node.ln_Name + 1,
			    (ubyte)gentry->ge_Node.ln_Name[0]
			);
			if (r > 0)
			    UnLockPacket(r);
		    }
		    FreeGEntry(gentry, 1006);
		}
		if (r < 0)
		    break;
		packet->dp_Arg3 = EXCLUSIVE_LOCK;
	    }
	    /* fall through */
	case ACTION_LOCATE_OBJECT:  /*	Lock,Name,Mode		    Lock    */
	    {
		GEntry *gentry = BLockToGEntry(packet->dp_Arg1);
		short nameLen;
		char *name = BNameToPtr(packet->dp_Arg2, &nameLen);
		long mode = packet->dp_Arg3;

		if (gentry = MakeGEntry(gentry, name, nameLen, mode)) {
		    FileLock *flock = MakeDosFileLock(gentry, mode);
		    r = CTOB(flock);
		} else {
		    r = -ERROR_OBJECT_NOT_FOUND;
		}
	    }
	    break;
	case ACTION_RENAME_DISK:    /*	Name			    Bool    */
	    {
		r = 0;
	    }
	    break;
	case ACTION_FREE_LOCK:	    /*	Lock			    Bool    */
	    {
		GEntry *gentry = BLockToGEntry(packet->dp_Arg1);
		FileLock *flock = BTOC(packet->dp_Arg1);
		long mode = flock->fl_Access;

		FreeGEntry(gentry, mode);
		FreeDosFileLock(flock);
		r = 0;
	    }
	    break;
	case ACTION_COPY_DIR:	    /*	Lock			    Lock    */
	    {
		GEntry *gentry = BLockToGEntry(packet->dp_Arg1);
		FileLock *flock = MakeDosFileLock(gentry, 1005);
		MakeGEntry(gentry, "", 0, 1005);	// bump refs
		r = CTOB(flock);
	    }
	    break;
	case ACTION_PARENT:	    /*	Lock			    Lock    */
	    {
		GEntry *gentry = BLockToGEntry(packet->dp_Arg1);
		FileLock *flock;

		if (gentry->ge_Parent) {
		    flock = MakeDosFileLock(gentry->ge_Parent, 1005);
		    MakeGEntry(gentry->ge_Parent, "", 0, 1005);	// bump refs
		    r = CTOB(flock);
		} else {
		    r = 0;	// NO error code!
		}
	    }
	    break;
	case ACTION_EXAMINE_OBJECT: /*	Lock,Fib		    Bool    */
	    {
		GEntry *gentry = BLockToGEntry(packet->dp_Arg1);
		FileInfoBlock *fib = BTOC(packet->dp_Arg2);

		if (gentry == &GRoot) {
		    clrmem(fib, sizeof(FileInfoBlock));
		    strcpy(fib->fib_FileName, DevName);
		    fib->fib_DirEntryType = ST_ROOT;
		    r = DOS_TRUE;
		} else {
		    r = RoutePacket(packet);  // call underlying fs
		}
		if (r == DOS_TRUE)
		    FixFileInfo(fib);
	    }
	    break;
	case ACTION_IS_FILESYSTEM:  /*	always return TRUE		    */
	    r = DOS_TRUE;
	    break;
	case ACTION_EXAMINE_NEXT:   /*	Lock,Fib		    Bool    */
	    {
		GEntry *gentry = BLockToGEntry(packet->dp_Arg1);
		FileInfoBlock *fib = BTOC(packet->dp_Arg2);

		if (gentry == &GRoot)
		    r = -ERROR_NO_MORE_ENTRIES;
		else
		    r = RoutePacket(packet);  // call underlying fs
		if (r == DOS_TRUE)
		    FixFileInfo(fib);
	    }
	    break;
	case ACTION_DELETE_OBJECT:  /*	Lock,Name		    Bool    */
	    {
		GEntry *gentry = BLockToGEntry(packet->dp_Arg1);
		GEntry *gtmp;
		short nameLen;
		char *name = BNameToPtr(packet->dp_Arg2, &nameLen);

		if (gtmp = MakeGEntry(gentry, name, nameLen, 1005)) {
		    if (gtmp->ge_LCRefs != 1) {
		    	r = -ERROR_OBJECT_IN_USE;
		    } else {
			r = 0;
		    }
		    FreeGEntry(gtmp, 1005);

		    // XXX handle .Z extension

		    if (r == 0)
		    	r = RoutePacket(packet);  // call underlying fs
		} else {
		    r = -ERROR_OBJECT_NOT_FOUND;
		}
	    }
	    break;
	case ACTION_RENAME_OBJECT:  /*	SLock,SName,DLock,DName     Bool    */
	    /*
	     * Warning! does not handle case where source may be open
	     */

	    r = RoutePacket(packet);
	    break;
	case ACTION_MORE_CACHE:     /*	buffers 		    Bool    */
	    break;
	case ACTION_SET_PROTECT:    /*	-,Lock,Name,Mask	    Bool    */
	case ACTION_SET_DATE:	    /*	-,Lock,Name,cptr-datestamp  Bool    */
	    {
		GEntry *gentry = BLockToGEntry(packet->dp_Arg2);

		if (gentry != &GRoot)
		    r = RoutePacket(packet);
	    }
	    break;
	case ACTION_SET_COMMENT:    /*	-,Lock,Name,Comment	    Bool    */
	    {
		short len;
		char *name = BNameToPtr(packet->dp_Arg3, &len);
		short clen;
		char *cname = BNameToPtr(packet->dp_Arg4, &clen);
		GEntry *gentry = BLockToGEntry(packet->dp_Arg2);
		GEntry *gen = MakeGEntry(gentry, name, len, SHARED_LOCK);

		if (gentry != &GRoot) {
		    SetCommentPacket(
			gentry->ge_Lock,
			name,
			len,
			cname,
			clen,
			gen
		    );
		}
		if (gen)
		    FreeGEntry(gen, 1005);
		r = DOS_TRUE;
	    }
	    break;
	case ACTION_CHANGE_MODE:    /*	CHANGE_*type,fh/lock,newmode   Bool */
	    r = -ERROR_ACTION_NOT_KNOWN;

	    dbprintf(("Arg1 type %d\n", packet->dp_Arg1));
	    switch(packet->dp_Arg1) {
	    case CHANGE_FH:
		GHandle *ghan = (GHandle *)packet->dp_Arg2;

		r = RoutePacket(packet);
		if (r == 0) {
		    MakeGEntry(ghan->gh_GEntry, "", 0, packet->dp_Arg3);
		    FreeGEntry(ghan->gh_GEntry, ghan->gh_Mode);
		}
		break;
	    case CHANGE_LOCK:
		/* XXX */
		r = 0;
		break;
	    }
	    break;
	case ACTION_EXAMINE_ALL:    /*	lock,buf,len,ED_*type,ctl   contflag*/
	case ACTION_DISK_INFO:	    /*	InfoData		    Bool    */
	case ACTION_INFO:
	case ACTION_FLUSH:	    /*				    Bool    */
	case ACTION_DISK_TYPE:
	case ACTION_DISK_CHANGE:
	case ACTION_SET_FILE_SIZE:  /*	fh,offset,seekmode	    Bool    */


	case ACTION_WRITE_PROTECT:
	case ACTION_SAME_LOCK:	    /*	Lock1,Lock2		    LOCK_*  */
	case ACTION_FH_FROM_LOCK:   /*	lock,openmode  (deref lock) Fh	    */


	case ACTION_COPY_DIR_FH:    /*	fh			       lock */
	case ACTION_PARENT_FH:	    /*	fh			       lock */
	case ACTION_EVENT:
	case ACTION_INHIBIT:	    /*	Bool			    Bool    */

	case ACTION_READ_RETURN:
	case ACTION_WRITE_RETURN:
	case ACTION_FORMAT:
	case ACTION_MAKE_LINK:	    /*	slock,slinkname,dlock/name,mode  Bool*/
	    /*
	     *	mode is LINK_SOFT or LINK_HARD
	     */

	case ACTION_READ_LINK:	    /*	lock,cstr-name,pathbuf,bufsize	len */
	    /*
	     *	returned length is -1 on error, -2 if buffer too small
	     */

	case ACTION_LOCK_RECORD:    /*	fh,startpos,len,mode,to     Bool    */
	    /*
	     *	mode:  0=excl 1=excl/nbio 2=shared 3=shared/nbio
	     */


	case ACTION_FREE_RECORD:    /*	fh,startpos,len 	    Bool    */
	case ACTION_ADD_NOTIFY:
	case ACTION_REMOVE_NOTIFY:
	case ACTION_TIMER:
	case ACTION_SCREEN_MODE:    /*	bool(-1=raw 0=cook)	    OldState*/
	case ACTION_WAIT_CHAR:	    /*	timeout(ticks)		    Bool    */
	default:
	def:
	    r = -ERROR_ACTION_NOT_KNOWN;
	    break;
	}

	dbprintf(("return %08lx (%d)\n", r, r));

	if (packet) {
	    if (r < -1) {
		packet->dp_Res1 = DOS_FALSE;
		packet->dp_Res2 = -r;
	    } else {
		packet->dp_Res1 = r;
		packet->dp_Res2 = 0;
	    }
	    ReturnPacket(packet);
	}
    }
    /*
     *	not reached
     */
}

void
myexit()
{
    UnInitialize();
}

static const char Fubar[] = { "dee-ey-ey-el-el-oh-en" };
