/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  ERROR.C
 */

#include <fcntl.h>
#include "defs.h"
#ifndef AMIGA
#include <unistd.h>
#endif
#ifdef _DCC
#include <exec/tasks.h>     /*  for SetRequester() */
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <clib/exec_protos.h>
#endif

#ifndef O_BINARY
#define O_BINARY    0
#endif

Prototype void cerror(short, const char *, ...);
Prototype void vcerror(int32_t, short, const char *, va_list va, short);
Prototype void zerror(short, ...);
Prototype void yerror(int32_t, short, ...);
Prototype char *ObtainErrorString(short);
Prototype void ExitError(short);

void *SetRequester(void *new);

#ifdef AMIGA
char    *ErrorFileName1 = DCC_CONFIG "dice.errors";
char    *ErrorFileName2 = DCC "config/dice.errors";
#endif
char    *ErrorAry;
short   ErrorArySize;
char    ErrBuf[128];

void
zerror(short errorId, ...)
{
    int etype = errorId & 0x0FFF;
    char *ptr;
    va_list va;

    ptr = ObtainErrorString(etype);

    va_start(va, errorId);
    vcerror(((LFBase) ? LFBase->lf_Index : 0), errorId >> 12, ptr, va, etype);
    va_end(va);
}

void
yerror(int32_t lexIdx, short errorId, ...)
{
    int etype = errorId & 0x0FFF;
    char *ptr;
    va_list va;

    ptr = ObtainErrorString(etype);

    va_start(va, errorId);
    vcerror(lexIdx, errorId >> 12, ptr, va, etype);
    va_end(va);
}


void
vcerror(int32_t lexIdx, short etype, const char *buf, va_list va, short eno)
{
    static const char *TNames[] = { "?","Warning","Error", "Fatal", "SoftError" };
    int32_t lexIdxBeg;
    int32_t lexFileNameLen;
    int32_t lexLine;
    char *lexFile;
    int32_t errcol = 0;
    char ebuf[100];
    int32_t ebufoff = 0;

    lexLine=FindLexFileLine(lexIdx, &lexFile, &lexFileNameLen, &lexIdxBeg);

    /*
     * Use original lexer file to obtain line for printing since the
     * internal copy could be munged.
     *
     * We need to determine a couple of things:
     *  lexLine - Indicates the line number of the error (0 if no associated line)
     *  errcol  - The physical column where the error occurred (0 for no line)
     *  ebuf    - The null terminated buffer.  This buffer is at most 80 characters
     *            but will contain the character position that has the error.
     *  ebufoff - The logical start of the error buffer.  This will be 0 as int32_t as
     *            the error occurs in the first 80 columns.  Beyond that, this will
     *            jump by 10.
     * lexFile  - The name of the file containing the error
     * lexFileNameLen
     */

    ebuf[0] = 0;
    if (lexLine && ErrorInFileValid) {

        short c;
        int32_t i = lexIdxBeg;
        short pos = 0;

        while ((c = FindLexCharAt(i)) != EOF && c != '\n')
        {
            if (c == '\t')
            {
                short tab;
                tab = ((pos + ebufoff + 8) & ~7) - ebufoff;
                while (pos < tab)
                    ebuf[pos++] = ' ';
            }
            else
                ebuf[pos++] = c;

            if (i == lexIdx)
                errcol = pos + ebufoff;

            if (pos > 80)
            {
               if ((errcol - (pos + ebufoff)) > 10) break;
               bcopy(ebuf+10, ebuf, pos-10);
               pos -= 10;
               ebufoff += 10;
            }
            ++i;
        }
        ebuf[pos] = 0;
    }

    eprintf(1, "DC1: \"%.*s\" L:%d ", lexFileNameLen, lexFile, lexLine);

    if (ErrorOpt & 1)
        eprintf(1, "C:%d %c:%d ", errcol + 1, TNames[etype][0], eno);
    else
        eprintf(1, "%s:%d ", TNames[etype], eno);

    veprintf(1, buf, va);
    eprintf(1, "\n");

    if (lexLine && ErrorInFileValid && (ErrorOpt & 2))
    {
        short pos = errcol - ebufoff;

        /* We Need to account for the fact that the ^ will take up one space */
        if (pos)
           pos = pos - 1;

        eprintf(0, "%s\n%*.*s^\n", ebuf, pos, pos, "");
    }

    if (etype == ESOFT || etype == EFATAL) {
        ExitError(20);
    }
    if (etype == EWARN && ExitCode < 5)
        ExitCode = 5;
    if (etype != EWARN && ExitCode < 20)
        ExitCode = 20;
}

void
ExitError(short code)
{
    DumpStats();
    if (OutFileName) {
        fclose(stdout);
        remove(OutFileName);
    }
    if (ExitCode < code)
        ExitCode = code;
    exit(ExitCode);
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
        if (ErrorAry[i] == 'C' && ErrorAry[i+1] == '1' && strtol(ErrorAry + i + 3, &ptr, 10) == errNum)
            return(ptr + 1);
    }
    sprintf(ErrBuf, "(no entry in %s for error)", (UseFileName) ? UseFileName : "??");
    return(ErrBuf);
}

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

