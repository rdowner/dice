/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  SUBS.C
 *
 *  Subroutines.
 */

#include "defs.h"


Prototype void makemygadget (struct Gadget *);
Prototype int firstns (char *);
Prototype int lastns (char *);
Prototype int wordlen (char *);
Prototype int getpathto (BPTR, char *, char *);
Prototype void *allocb (int);
Prototype void *allocl (int);
Prototype void bmovl (void *, void *, long);
Prototype int detab (char *, char *, int);
Prototype int xefgets (FILE *, char *, int);
Prototype int ncstrcmp (unsigned char *, unsigned char *);
Prototype struct _ED *finded (char *, int);
Prototype void mountrequest(int);
Prototype int DeadKeyConvert(struct IntuiMessage *, UBYTE *, int, struct KeyMap *);
Prototype FONT * GetFont(char *, short);
Prototype char *GetDEnv(char *);
Prototype void SetDEnv(char *, char *);

Prototype extern __stkargs int arpreq (char *, char *, char *, WIN *);

Prototype extern void *clrmem(void *, size_t);


typedef struct FileInfoBlock FIB;
typedef struct Process	     PROC;


/*
 *  Create DME's text icon.
 */

void
makemygadget(gad)
struct Gadget *gad;
{
    static unsigned long ga[] = {
	0xFFFFFFFF,	/* 32 pixels across */
	0x80FDCBFD,
	0xFFFDDFFD,
	0x80000001,
	0x80DFDDDF,
	0x80000001,
	0xBC0EF00B,
	0x80000001,
	0xBFC00CDD,
	0x80000001,
	0xA00DF00F,
	0x80000001,
	0x80000001,

	0x80000001,
	0x80FDCBFD,
	0xFFFDDFFD,
	0x80000001,
	0x80DFDDDF,
	0x80000001,
	0xBC0EF00B,
	0x80000001,
	0xBFC00CDD,
	0x80000001,
	0xA00DF00F,
	0x80000001,
	0xFFFFFFFF
    };
    static struct Image image = {
	0, 0, 20, 16, 2, (unsigned short *)ga, 3, 0, NULL
    };
    clrmem(gad, sizeof(struct Gadget));
    gad->Width = 20;
    gad->Height = 17;
    gad->Flags	= GADGIMAGE|GADGHCOMP;
    gad->GadgetType   = BOOLGADGET;
    gad->Activation = RELVERIFY|GADGIMMEDIATE;
    gad->GadgetRender = (APTR)&image;
}

/*
 * return index of first non space.  Returns 0 if no spaces found.
 */

firstns(str)
char *str;
{
    short i;

    for (i = 0; str[i] && str[i] == ' '; ++i);
    if (str[i] == 0)
	i = 0;
    return((int)i);
}

/*
 *  Return index of last non-space, 0 if no spaces.
 */

lastns(str)
char *str;
{
    short i;

    for (i = strlen(str) - 1; i > 0 && str[i] == ' '; --i);
    if (i < 0)
	i = 0;
    return((int)i);
}

/*
 *  Return length of word under cursor
 */

wordlen(str)
char *str;
{
    short i;

    for (i = 0; *str && *str != ' '; ++i, ++str);
    return((int)i);
}

/*
 *  Backtracks the program lock, 0 on failure, 1 on success.
 */

getpathto(lock, arg0, buf)
BPTR lock;
char *arg0;
char *buf;
{
    FIB *fib;
    BPTR parLock = NULL;
    int r = 0;
    short lastDir = 0;

    buf[0] = 0;
    if (fib = malloc(sizeof(FIB))) {
	r = 1;
	while (lock) {
	    fib->fib_FileName[0] = 0;
	    if (Examine(lock, fib) == 0)
		r = 0;
	    if (parLock) {
		parLock = ParentDir(lock);
		UnLock(lock);
	    } else {
		parLock = ParentDir(lock);
		if (fib->fib_DirEntryType > 0)
		    lastDir = 1;
	    }
	    if (parLock == NULL)
		strins(buf, ":");
	    strins(buf, fib->fib_FileName);
	    lock = parLock;
	}
	free(fib);
	if (lastDir == 1) {
	    short len;
	    if ((len = strlen(buf)) && buf[len-1] != ':')
		strcat(buf, "/");
	    strcat(buf, arg0);
	}
    }
    return(r);
}

/*
 *  Allocation routines and other shortcuts
 */

void *
allocb(bytes)
{
    return(AllocMem(bytes, MEMF_CLEAR|MEMF_PUBLIC));
}

void *
allocl(lwords)
{
    return(AllocMem(lwords<<2, MEMF_CLEAR|MEMF_PUBLIC));
}

void
bmovl(s,d,n)
void *s, *d;
long n;
{
    movmem(s, d, n << 2);
}

/*
 *  Remove tabs in a buffer
 */

detab(ibuf, obuf, maxlen)
char *ibuf, *obuf;
{
    short i, j;

    maxlen -= 2;
    for (i = j = 0; ibuf[i] && j < maxlen; ++i) {
	if (ibuf[i] == 9) {
	    do {
		obuf[j++] = ' ';
	    } while ((j & 7) && j < maxlen);
	} else {
	    obuf[j++] = ibuf[i];
	}
    }
    if (j && obuf[j-1] == '\n')
	--j;
    while (j && obuf[j-1] == ' ')
	--j;
    obuf[j] = 0;
    return((int)j);
}

xefgets(fi, buf, max)
FILE *fi;
char *buf;
int max;
{
    char ebuf[256];

    if (fgets(ebuf, max, fi))
	return(detab(ebuf, buf, max));
    return(-1);
}

ncstrcmp(s1, s2)
ubyte *s1, *s2;
{
    ubyte c1, c2;

    for (;;) {
	c1 = *s1;
	c2 = *s2;
	if (c1 >= 'A' && c1 <= 'Z') c1 |= 0x20;
	if (c2 >= 'A' && c2 <= 'Z') c2 |= 0x20;
	if (c1 != c2)
	    break;
	if ((c1|c2) == 0)
	    return(0);
	++s1;
	++s2;
    }
    if (c1 < c2)
	return(-1);
    if (c1 > c2)
	return(1);
}

ED *
finded(str, doff)
char *str;
{
    ED *ed;

    for (ed = (ED *)DBase.mlh_Head; ed->Node.mln_Succ; ed = (ED *)ed->Node.mln_Succ) {
	if (strlen(ed->Name) >= doff && ncstrcmp(str, ed->Name+doff) == 0)
	    return(ed);
    }
    return(NULL);
}

void
mountrequest(bool)
int bool;
{
    static APTR original_pr_WindowPtr = NULL;
    register PROC *proc;

    proc = (PROC *)FindTask(0);
    if (!bool && proc->pr_WindowPtr != (APTR)-1) {
	original_pr_WindowPtr = proc->pr_WindowPtr;
	proc->pr_WindowPtr = (APTR)-1;
    }
    if (bool && proc->pr_WindowPtr == (APTR)-1)
	proc->pr_WindowPtr = original_pr_WindowPtr;
}

char *
GetDEnv(ename)
char *ename;
{
    char *str;

    if (str = getenv(ename)) {
	str = strdup(str);
    }
    return(str);
}

#ifdef NOTDEF /* old code */

    long envLock = Lock("env:", SHARED_LOCK);
    char *str = NULL;

    if (envLock) {
	long oldLock = CurrentDir(envLock);
	FILE *fi = fopen(ename, "r");
	long siz;
	if (fi) {
	    fseek(fi, 0L, 2);
	    siz = ftell(fi);
	    fseek(fi, 0L, 0);
	    if (siz > 0 && (str = malloc(siz + 1))) {
		fread(str, siz, 1, fi);
		str[siz] = 0;
	    }
	    fclose(fi);
	}
	UnLock(CurrentDir(oldLock));
    }
    return(str);
}

#endif

void
SetDEnv(ename, econt)
char *ename;
char *econt;
{
    long envLock = Lock("env:", SHARED_LOCK);

    if (envLock) {
	long oldLock = CurrentDir(envLock);
	FILE *fi = fopen(ename, "w");

	if (fi) {
	    fwrite(econt, strlen(econt), 1, fi);
	    fclose(fi);
	}
	UnLock(CurrentDir(oldLock));
    }
}


/*
 *  GETFONT()
 *
 *  This function properly searches resident and disk fonts for the
 *  font.
 */

struct Library *DiskfontBase;

FONT *
GetFont(name, size)
char *name;
short size;
{
    FONT *font1;
    TA Ta;
    short libwasopen = (DiskfontBase != (void *)NULL);

    Ta.ta_Name	= (UBYTE *)name;
    Ta.ta_YSize = size;
    Ta.ta_Style = 0;
    Ta.ta_Flags = 0;

    font1 = OpenFont(&Ta);
    if (font1 == NULL || font1->tf_YSize != Ta.ta_YSize) {
	FONT *font2;

	if (libwasopen || (DiskfontBase = OpenLibrary("diskfont.library", 0))) {
	    if (font2 = OpenDiskFont(&Ta)) {
		if (font1)
		    CloseFont(font1);
		font1 = font2;
	    }
	    if (libwasopen == 0)
		CloseLibrary(DiskfontBase);
	}
    }
    return(font1);
}

/*
 *  DEAD.C
 */

int
DeadKeyConvert(msg,buf,bufsize,keymap)
struct IntuiMessage *msg;
UBYTE *buf;
int bufsize;
struct KeyMap *keymap;
{
    static struct InputEvent ievent = { NULL, IECLASS_RAWKEY };
    if (msg->Class != RAWKEY)
	return(-2);
    ievent.ie_Code = msg->Code;
    ievent.ie_Qualifier = msg->Qualifier;
    ievent.ie_position.ie_addr = *((APTR *)msg->IAddress);
    return(RawKeyConvert(&ievent,(char *)buf,bufsize,keymap));
}

void *
clrmem(ptr, bytes)
void *ptr;
size_t bytes;
{
    setmem(ptr, bytes, 0);
    return(ptr);
}

