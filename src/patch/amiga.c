/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  AMIGA replacement routines
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <libraries/dos.h>
#include <libraries/dosextens.h>

/*
 *  Uses PT: for 'patch temp'
 */

static long tfid;

char *
mktemp(buf)
char *buf;
{
    char *base = buf;
    buf += strlen(buf);
    sprintf(buf, "%06x%d", FindTask(NULL) >> 8, tfid++);
    return(base);
}

int
chmod(name, mode)
char *name;
int mode;
{
    return(0);
}

char *gettmpenv(const char *);
char *getenv(const char *);

char *
gettmpenv(id)
const char *id;
{
    static char *buf;
    static char *res = NULL;
    long fh;
    long len;

    buf = malloc(strlen(id) + 8);
    sprintf(buf, "ENV:%s", id);
    fh = Open(buf, 1005);
    free(buf);
    if (fh) {
	Seek(fh, 0L, 1);
	len = Seek(fh, 0L, -1);
	if (len < 0) {
	    Close(fh);
	    return(NULL);
	}
	if (res)
	    free(res);
	res = malloc(len + 1);
	Read(fh, res, len);
	Close(fh);
	res[len] = 0;
	return(res);
    }
    return(NULL);
}

char *
getenv(id)
const char *id;
{
    char *res = gettmpenv(id);
    char *perm = NULL;

    if (res)
	perm = strdup(res);
    return(perm);
}

int
fstat(fd, stat)
int fd;
struct stat *stat;
{
    long pos;

    setmem(stat, sizeof(struct stat), 0);
    pos = lseek(fd, 0L, 1);
    stat->st_size = lseek(fd, 0L, 2);
    stat->st_mode = S_IFREG;
    lseek(fd, pos, 0);
    return(0);
}

int
stat(name, stat)
char *name;
struct stat *stat;
{
    BPTR lock = Lock(name, SHARED_LOCK);
    __aligned struct FileInfoBlock fib;

    setmem(stat, sizeof(struct stat), 0);
    if (lock == NULL)
	return(-1);
    Examine(lock, &fib);
    UnLock(lock);
    stat->st_size = fib.fib_Size;
    stat->st_ino = (long)((struct FileLock *)BADDR(lock))->fl_Key;
    stat->st_dev = (long)((struct FileLock *)BADDR(lock))->fl_Task;
    stat->st_mode = (fib.fib_DirEntryType > 0) ? S_IFDIR : S_IFREG;
    return(0);
}

link(a, b)
char *a, *b;
{
    return(-1);
}

static char pnam[L_tmpnam];
static char Cmd[256];

FILE *
popen(cmd, modes)
char *cmd;
char *modes;
{
    tmpnam(pnam);
    {
	char *ptr;
	if (ptr = strrchr(cmd, '/')) {
	    cmd = ptr + 1;
	}
    }
    if (strnicmp(cmd, "ed", 2) == 0)
	sprintf(Cmd, "u%s < %s", cmd, pnam);
    else
	sprintf(Cmd, "%s < %s", cmd, pnam);
    return(fopen(pnam, modes));
}

pclose(fi)
FILE *fi;
{
    fclose(fi);
    Execute(Cmd, NULL, NULL);
    return(0);
}

