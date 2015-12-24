/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  DU <files/dirs>
 *
 */

#include <exec/types.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <clib/dos_protos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <lib/version.h>

#define push_jmp(jbuf,jptr) ((jptr = JmpBase), (JmpBase = jbuf), setjmp(jbuf))
#define forget_jmp(jptr)    { JmpBase = jptr; }

typedef struct FileInfoBlock	Fib;
typedef struct InfoData 	InfoData;

typedef struct Accum {
    struct Accum *Parent;
    char    *Name;
    long    Files;
    long    Dirs;
    long    Blocks;
} Accum;

char *AccToName(Accum *);
long GetBlockSize(char *);

int mybrk(void);
void ScanDir(Accum *, char *, int);

#ifdef _DCC
IDENT("du",".03");
DCOPYRIGHT;
#endif

long BlockSize;
jmp_buf *JmpBase;

main(ac, av)
short ac;
char *av[];
{
    static Accum ac0;
    short i;
    long bytes;
    jmp_buf jbuf;
    jmp_buf *jptr;

    onbreak(mybrk);
//    if (push_jmp(jbuf,jptr)) {
    jptr = JmpBase;
    JmpBase = &jbuf;
    if(setjmp(jbuf)) {
	puts("^C");
	exit(1);
    }

    if (ac == 1)
	av[ac++] = "";      /*  overwrites the NULL, which is OK    */

    bytes = 0;
    for (i = 1; i < ac; ++i) {
	long lastBlocks = ac0.Blocks;
	BlockSize = GetBlockSize(av[i]);
	ScanDir(&ac0, av[i], 0);
	bytes += (ac0.Blocks - lastBlocks) * BlockSize;
    }
    if (ac > 2)
	printf("%-30s %6ld Kb %6ld BLKS\n", "--TOTAL--", bytes / 1024, ac0.Blocks);
    return(0);
}

void
ScanDir(acp, name, level)
Accum *acp;
char *name;
int level;
{
    BPTR lock;
    jmp_buf jbuf;
    jmp_buf *jptr;

    if (lock = Lock(name, SHARED_LOCK)) {
	__aligned Fib fib;
	Accum acx;

//	if (push_jmp(jbuf,jptr)) {
	jptr = JmpBase;
    	JmpBase = &jbuf;
    	if(setjmp(jbuf)) {
	    UnLock(lock);
//	    forget_jmp(jptr);
	    JmpBase = jptr;
	    longjmp(*jptr, 1);
	}
	clrmem(&acx, sizeof(acx));
	acx.Parent = acp;
	acx.Name = name;

	memset((char *)&fib,0,sizeof(fib));
	if (Examine(lock, (struct FileInfoBlock *)&fib)) {
	    if (fib.fib_DirEntryType > 0) {    /*  dir	   */
		long old = CurrentDir(lock);

//		forget_jmp(jptr);
		JmpBase = jptr;

//		if (push_jmp(jbuf,jptr)) {
		jptr = JmpBase;
    		JmpBase = &jbuf;
    		if(setjmp(jbuf)) {
		    CurrentDir(old);
		    UnLock(lock);
//		    forget_jmp(jptr);
		    JmpBase = jptr;
		    longjmp(*jptr, 1);
		}

		++acx.Blocks;
		++acx.Dirs;
		while (ExNext(lock, (struct FileInfoBlock *)&fib)) {
		    if((fib.fib_DirEntryType != ST_LINKDIR)
		      && (fib.fib_DirEntryType != ST_LINKFILE)
		      && (fib.fib_DirEntryType != ST_SOFTLINK)) {
			ScanDir(&acx, fib.fib_FileName, level + 1);	
		      }
		      else {
			++acx.Files;	// show link as a file
			++acx.Blocks;
		      }
		}
		CurrentDir(old);
		Examine(lock, (struct FileInfoBlock *)&fib);
	    } else {				/*  link    */
		++acx.Files;
		acx.Blocks += 1 + fib.fib_NumBlocks + fib.fib_NumBlocks / 72;
	    }
	}
//	forget_jmp(jptr);
	JmpBase = jptr;
	UnLock(lock);
	acp->Blocks += acx.Blocks;
	acp->Dirs   += acx.Dirs;
	acp->Files  += acx.Files;
	if (fib.fib_DirEntryType > 0 && level < 2) {
	    printf("%*.*s%-*s %6ld Kb %6ld BLKS\n",
		level, level, "",
		30 /* - level */, AccToName(&acx),
		acx.Blocks / 2, acx.Blocks
	    );
	}
    }
}

char *
AccToName(acp)
Accum *acp;
{
    static char Buf[1024];
    char *ptr = Buf + sizeof(Buf);

    *--ptr = 0;
    while (acp && acp->Name) {
	int len = strlen(acp->Name);
	char c = acp->Name[len-1];

	if (*ptr && c && c != '/' && c != ':')
	    *--ptr = '/';
	ptr -= len;
	strncpy(ptr, acp->Name, len);
	acp = acp->Parent;
    }
    return(ptr);
}

int mybrk()
{
    longjmp(*JmpBase, 1);
}

long
GetBlockSize(dir)
char *dir;
{
    long lock;
    long lock2;
    long r = 0;
    InfoData id;

    if (lock = Lock(dir, SHARED_LOCK)) {
	while (lock2 = ParentDir(lock)) {
	    UnLock(lock);
	    lock = lock2;
	}
	if (Info(lock, &id))
	    r = id.id_BytesPerBlock;
	UnLock(lock);
    }
    return(r);
}

