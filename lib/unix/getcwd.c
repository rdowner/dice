
/*
 *  GETCWD.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <exec/types.h>
#include <libraries/dos.h>
#include <libraries/dosextens.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct FileInfoBlock FIB;
typedef struct Process Process;

char *
getcwd(path, max)
char *path;
int max;
{
    BPTR lock;
    FIB *fib;
    int len = 0;    /*	path length	*/
    int elen;	    /*	element length	*/
    short alloced = 0;

    {
	Process *proc = (Process *)FindTask(NULL);
	if (proc->pr_Task.tc_Node.ln_Type == NT_TASK)
	    return(NULL);
	lock = DupLock(proc->pr_CurrentDir);
	if (lock == NULL)
	    return(NULL);
    }
    if (path == NULL) {
	path = malloc(max);
	if (path == NULL)
	   max = 0;    /* Don't let the overwrite non-existent memory */
	alloced = 1;
    }

    if (fib = malloc(sizeof(FIB))) {
	while (lock && Examine(lock, fib)) {
	    BPTR newlock;

	    elen = strlen(fib->fib_FileName);
	    if (len + elen + 2 > max)
		break;

	    newlock = ParentDir(lock);

	    if (len) {
		if (newlock)
		    strins(path, "/");
		else
		    strins(path, ":");
		strins(path, fib->fib_FileName);
		++len;
	    } else {
		strcpy(path, fib->fib_FileName);
		if (newlock == NULL)
		    strcat(path, ":");
	    }
	    len += elen;

	    UnLock(lock);
	    lock = newlock;
	}
	free(fib);
    }

    if (lock) {
	UnLock(lock);
	if (alloced)
	    free(path);
	path = NULL;
    }
    return(path);
}


