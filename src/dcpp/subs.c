/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  SUBS.C
 *
 */

#include "defs.h"
#ifdef AMIGA
#include <exec/tasks.h>     /*	for SetRequester() */
#include <dos/dos.h>
#include <dos/dosextens.h>
#endif

#ifndef O_BINARY
#define O_BINARY 0
#endif

Prototype void cerror(short, ...);
Prototype void *zalloc(int32_t);
Prototype void *AllocCopy(void *, int32_t);
Prototype Include *GetNominalInclude(int);
Prototype void ErrorNoMemory(void);
Prototype void ErrorOpenFailed(char *, short);
Prototype __stkargs int TriGraphConvert(short);
Prototype char *ObtainErrorString(short);
Prototype void ErrorExit(short);

Prototype void eprintf(const char *ctl, ...);
Prototype void veprintf(const char *ctl, va_list va);
Prototype void eputc(char c);

void *SetRequester(void *);

#ifdef AMIGA
char	*ErrorFileName1 = DCC_CONFIG "dice.errors";
char	*ErrorFileName2 = DCC "config/dice.errors";
#endif
char	*ErrorAry;
int32_t	ErrorArySize;
char	ErrBuf[128];

void
cerror(short errorId, ...)
{
    static const char *Ary[] = { "?Bad", "Error", "Warning", "Fatal" };
    Include *inc = GetNominalInclude(1);
    va_list va;

    if (inc && inc->FileName == NULL)
	inc->FileName = "<unknown-file>";

    eprintf("DCPP: \"%s\" L:%d C:0 %.*s:%d ",
	((inc) ? inc->FileName : ""),
	((inc) ? inc->LineNo : 0),
	((ErrorOpt == 2) ? 1 : (int)strlen(Ary[errorId >> 12])),
	Ary[errorId >> 12],
	errorId & 0x0FFF
    );
    va_start(va, errorId);
    veprintf(ObtainErrorString(errorId & 0x0FFF), va);
    va_end(va);
    eputc('\n');

    switch(errorId & EF_MASK) {
    case EF_WARN:
	if (ExitCode < 5)
	    ExitCode = 5;
	break;
    case EF_ERROR:
	if (ExitCode < 20)
	    ExitCode = 20;
	break;
    case EF_FATAL:
	ErrorExit(20);
	break;
    }
}

void
ErrorExit(short code)
{
    if (Fo)
	fclose(Fo);
    if (OutName)
	remove(OutName);
    if (ExitCode < code)
	ExitCode = code;
    exit(ExitCode);
}

void *
zalloc(bytes)
int32_t bytes;
{
    static char *Buf;
    static int32_t Bytes;
    void *ptr;

    bytes = (bytes + 3) & ~3;

    if (bytes > Bytes) {
	if (bytes > ZA_SIZE)
	    cerror(EFATAL_SOFTWARE_ERROR_ALLOCATE, bytes);
	Buf = malloc(ZA_SIZE);
	if (Buf == NULL)
	    ErrorNoMemory();
	Bytes = ZA_SIZE;
	setmem(Buf, Bytes, 0);
    }
    ptr = Buf;
    Buf += bytes;
    Bytes -= bytes;
    return(ptr);
}

void *
AllocCopy(buf, bytes)
void *buf;
int32_t bytes;
{
    void *ptr;

    if (bytes > 128) {		/*  efficiency	*/
	ptr = malloc(bytes);
	if (ptr == NULL)
	    ErrorNoMemory();
    } else {
	ptr = zalloc(bytes);
    }
    movmem(buf, ptr, bytes);
    return(ptr);
}

Include *
GetNominalInclude(int noerr)
{
    Include *inc = PushBase;

    while (inc && inc->IsFile == 0)
	inc = inc->Next;
    if (inc == NULL) {
	if (noerr)
	    return(NULL);
	cerror(EFATAL_NOMINAL_INCLUDE);
    }
    return(inc);
}

void
ErrorNoMemory()
{
    eprintf("NO MEMORY!\n");
    ErrorExit(25);
}

void
eprintf(const char *ctl, ...)
{
    va_list va;

    va_start(va, ctl);
    veprintf(ctl, va);
    va_end(va);
}

void
veprintf(const char *ctl, va_list va)
{
    va_list tmp_va;

    va_copy(tmp_va, va);
    vfprintf(stderr, ctl, tmp_va);
    if (ErrorFi) {
	va_copy(tmp_va, va);
        vfprintf(ErrorFi, ctl, tmp_va);
    }
}

void
eputc(char c)
{
    fputc(c, stderr);
    if (ErrorFi)
        fputc(c, ErrorFi);
}

void
ErrorOpenFailed(char *file, short len)
{
    if (len == 0)
	len = strlen(file);
    cerror(EERROR_CANT_OPEN_FILE, len, file);
}

__stkargs int
TriGraphConvert(short c)
{
    switch(c) {
    case '=':
	return('#');
    case '(':
	return('[');
    case '/':
	return('\\');
    case ')':
	return(']');
    case '\'':
	return('^');
    case '<':
	return('{');
    case '!':
	return('|');
    case '>':
	return('}');
    case '-':
	return('~');
    }
    return(0);
}

char *
ObtainErrorString(short errNum)
{
    short i;
    static char *UseFileName;

    if (ErrorAry == NULL) {
	int fd;
	short siz;
	void *save;

	save = SetRequester((void *)-1);
#ifdef AMIGA
	UseFileName = ErrorFileName1;
	fd = open(ErrorFileName1, O_RDONLY|O_BINARY);
#endif
	SetRequester(save);
#ifdef AMIGA
	if (fd < 0) {
	    if ((fd = open(ErrorFileName2, O_RDONLY|O_BINARY)) < 0) {
		sprintf(ErrBuf, "(can't open %s!)", ErrorFileName2);
		return(ErrBuf);
	    }
	    UseFileName = ErrorFileName2;
	}
#else
	UseFileName = LocatePath("DERRORS", "dice.errors");
	fd = open(UseFileName, O_RDONLY|O_BINARY);
	if (fd < 0) {
	    sprintf(ErrBuf, "(can't open %s!)", UseFileName);
	    return(ErrBuf);
	}
#endif
	siz = lseek(fd, 0L, 2);
	lseek(fd, 0L, 0);
	ErrorAry = malloc(siz + 1);
	read(fd, ErrorAry, siz);
	close(fd);
	{
	    char *ptr;
	    for (ptr = strchr(ErrorAry, '\n'); ptr; ptr = strchr(ptr + 1, '\n'))
		*ptr = 0;
	}
	ErrorAry[siz] = 0;
	ErrorArySize = siz;
    }
    for (i = 0; i < ErrorArySize; i += strlen(ErrorAry + i) + 1) {
	char *ptr;
	if (ErrorAry[i] == 'C' && ErrorAry[i+1] == 'P' && strtol(ErrorAry + i + 3, &ptr, 10) == errNum)
	    return(ptr + 1);
    }
    sprintf(ErrBuf, "(no entry in %s for error)", (UseFileName) ? UseFileName : "??");
    return(ErrBuf);
}

#ifdef LATTICE

int
cmpmem(ubyte *s1, ubyte *s2, int32_t n)
{
    while (n) {
	if (*s1 < *s2)
	    return(-1);
	if (*s1 > *s2)
	    return(1);
	--n;
	++s1;
	++s2;
    }
    return(0);
}

#endif

#ifdef AMIGA

void *
SetRequester(void *new)
{
    void *old;
    struct Process *proc = (struct Process *)FindTask(NULL);

    old = proc->pr_WindowPtr;
    proc->pr_WindowPtr = new;
    return(old);
}

#else

void *
SetRequester(void *new)
{
    return(NULL);
}

#endif

