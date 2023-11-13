/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  RUN.C
 */

#include "defs.h"
#ifdef AMIGA
#include <dos/dosextens.h>
#include <dos/var.h>
#include <dos/dostags.h>
#else
#include <unistd.h>
#include <sys/wait.h>
#endif

typedef struct CommandLineInterface CLI;
typedef struct Process              Process;

Prototype long Execute_Command(char *, short);
Prototype void InitCommand(void);
Prototype long LoadSegLock(long, char *);

#ifdef AMIGA
BPTR SaveLock;
#endif
char RootPath[512];

extern struct Library *SysBase;

void
ICExit(void)
{
#ifdef AMIGA
    if (SaveLock) {
        UnLock(CurrentDir(SaveLock));
        SaveLock = NULL;
    }
#endif
}

void
InitCommand()
{
#ifdef AMIGA
    SaveLock = CurrentDir(DupLock(((Process *)FindTask(NULL))->pr_CurrentDir));
#endif
    getcwd(RootPath, sizeof(RootPath));
    atexit(ICExit);
}

/*
 *  cmd[-1] is valid space and, in fact, must be long word aligned!
 */

long
Execute_Command(char *cmd, short ignore)
{
    register char *ptr;

    for (ptr = cmd; *ptr && *ptr != ' ' && *ptr != '\t' && *ptr != '\n'; ++ptr)
        ;


    /*
     *  Internal MakeDir because AmigaDOS 2.04's MakeDir is unreliable
     *  with RunCommand() (it can crash)
     *
     *  Internal CD because we special case it
     */

#ifdef AMIGA
    if (ptr - cmd == 7 && strnicmp(cmd, "makedir", 7) == 0) {
        long lock;
        short err = 0;

        while (*ptr == ' ' || *ptr == '\t')
            ++ptr;
        if (lock = CreateDir(ptr))
            UnLock(lock);
        else {
            printf("Unable to makedir %s\n", ptr);
            err = 20;
        }
        return((ignore) ? 0 : err);
    } else
    if (ptr - cmd == 6 && strnicmp(cmd, "fwrite", 6) == 0) {
        char *t;
        BPTR fh;
        short err = 0;

        while (*ptr == ' ' || *ptr == '\t')
            ++ptr;
        for (t = ptr; *t && *t != ' ' && *t != '\t'; t++);
        if (*t) *t++ = '\0';
        if (fh = Open(ptr, MODE_NEWFILE)) {
            int len;
            len = strlen(t);
            for(ptr = t; *ptr; ptr++) if (*ptr == ' ') *ptr = '\n';
            t[len] = '\n';
            Write(fh, t, len+1);
            t[len] = '\0';
            Close(fh);
            err = 0;
        }
        else
        {
            printf("Unable to write %s\n", ptr);
            err = 20;
        }
       return((ignore) ? 0 : err);
    } else
#endif
    if (ptr - cmd == 2 && strncasecmp(cmd, "cd", 2) == 0) {
#ifdef AMIGA
        long lock;
#endif
        short err = 0;

        while (*ptr == ' ' || *ptr == '\t')
            ++ptr;
        {
            short len = strlen(ptr);    /*  XXX HACK HACK */
            if (len && ptr[len-1] == '\n')
                ptr[len-1] = 0;
        }
#ifdef AMIGA
        if (*ptr == 0)
            lock = DupLock(SaveLock);
        else
            lock = Lock(ptr, SHARED_LOCK);
        if (lock)
            UnLock(CurrentDir(lock));
        else {
            printf("Unable to cd %s\n", ptr);
            err = 20;
        }
#else
        if (*ptr == 0)
            err = chdir(RootPath);
        else
            err = chdir(ptr);
        if (err != 0) {
            err = 20;
            printf("Unable to cd %s\n", ptr);
        }
#endif
        return((ignore) ? 0 : err);
    }

    /*
     * run command cmd
     *
     */

#ifdef AMIGA
    {
        short i;
        short ci;
        short c;
        short err = 0;
        short useSystem = 0;
        Process *proc = (Process *)FindTask(NULL);
        char *cmdArgs;

        for (i = 0; cmd[i] && cmd[i] != ' ' && cmd[i] != 9 && cmd[i] != 10 && cmd[i] != 13; ++i)
            ;
        if (strpbrk(cmd + i, "<>|`"))
            useSystem = 1;
        else
            useSystem = 0;

        if (c = cmd[ci = i])
            ++ci;
        cmd[i] = 0;

        cmdArgs = malloc(strlen(cmd + ci) + 3);
        sprintf(cmdArgs, "%s\n\r", cmd + ci);
        fflush(stdout);

        /*
         *  NOTE: RunCommand() is unreliable if no pr_CLI exists,
         *  MUST use system13() in that case.
         */

#if INCLUDE_VERSION >= 36
        if (SysBase->lib_Version >= 36 && proc->pr_CLI) {
            long seg;
            long stack;
            long lock = 0;
            CLI *cli = (CLI *)BADDR(proc->pr_CLI);
            static char OldCmd[128];
            char dt[4];
            struct TagItem *tags[] = {
                NP_CopyVars, TRUE,
                TAG_END, NULL};

            if (cli) {
                GetProgramName(OldCmd, sizeof(OldCmd));
                SetProgramName(cmd);
                stack = cli->cli_DefaultStack * 4;
            } else {
                stack = 8192;
            }

            /*
             *  note: Running2_04() means V37 or greater
             */

            if (useSystem || (Running2_04() && GetVar(cmd, dt, 2, LV_ALIAS | GVF_LOCAL_ONLY) >= 0))
                goto dosys;

            if ((seg = FindSegment(cmd, 0L, 0)) || (seg = FindSegment(cmd, 0L, 1))) {
                dbprintf(("A cmd = '%s' stack = %d\n", cmdArgs, stack));
                err = RunCommand(((long *)seg)[2], stack, cmdArgs, strlen(cmdArgs) - 1);
            } else if ((lock = _SearchPath(cmd)) && (seg = LoadSegLock(lock, ""))) {
                dbprintf(("B\n"));
                err = RunCommand(seg, stack, cmdArgs, strlen(cmdArgs) - 1);
                UnLoadSeg(seg);
            } else if ((lock = Lock("dcc:bin", SHARED_LOCK)) && (seg = LoadSegLock(lock, cmd))) {
                dbprintf(("C %08x\n", seg));
                dbprintf(("CMD= %s", cmdArgs));
                err = RunCommand(seg, 8192, cmdArgs, strlen(cmdArgs) - 1);
                UnLoadSeg(seg);
            } else {
dosys:
                dbprintf(("D\n"));
                cmd[i] = c;
                /*err = system13(cmd);*/
                err = SystemTagList(cmd, tags);
            }
            if (cli)
                SetProgramName(OldCmd);
            if (lock)
                UnLock(lock);
        } else
#endif
        {
            dbprintf(("E\n"));
            cmd[i] = c;
            err = system13(cmd);
        }
        free(cmdArgs);
        if (err)
            printf("Exit code %d %s\n", err, (ignore) ? "(Ignored)":"");
        if (ignore)
            return(0);
        return(err);
    }
#else
    {
        int err;

        if ((err = vfork()) == 0) {
            execlp("/bin/sh", "/bin/sh", "-c", cmd, 0);
            exit(30);
        } else {
#ifdef NOTDEF
            union wait uwait;

            while (wait(&uwait) != err || WIFEXITED(uwait) == 0)
                ;
            err = uwait.w_retcode;
#endif
            int status;
            while (wait(&status) != err || WIFEXITED(status) == 0)
                ;
            err = WEXITSTATUS(status);
        }
        if (err)
            printf("Exit code %d %s\n", err, (ignore) ? "(Ignored)":"");
        if (ignore)
            return(0);
        return(err);
    }
#endif
}

#ifdef AMIGA

long
LoadSegLock(lock, cmd)
long lock;
char *cmd;
{
    long oldLock;
    long seg;

    oldLock = CurrentDir(lock);
    seg = LoadSeg(cmd);
    CurrentDir(oldLock);
    return(seg);
}


#endif
