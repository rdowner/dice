
/*
 *  EXPAND_ARGS.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <exec/types.h>
#include <libraries/dos.h>
#include <clib/dos_protos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lib/misc.h>

#define MAXPATHLEN  512

static char **MyAv;
static int   AcIdx;
static int   AcMax;

typedef struct FileInfoBlock FIB;

int ExpandElements(char *, short, char *, char *);
int AddArgument(char *);
int expand_args(int, const char **, int *, char ***);

int
expand_args(ac, av, nac, nav)
int   ac;
const char **av;
int   *nac;
char ***nav;
{
    char *tmp1 = malloc(MAXPATHLEN);        /*  max path len    */
    char *tmp2 = malloc(MAXPATHLEN);        /*  max path len    */
    char *arg;
    int error = 0;

    MyAv  = malloc(sizeof(char *) * 8);
    AcIdx = 1;
    AcMax = 8;
    *nac = 0;

    if (MyAv == NULL || tmp1 == NULL || tmp2 == NULL)
        return(-1);

    MyAv[0] = *av;

    while (error >= 0 && (arg = *++av)) {
        short i;
        char c;

        for (i = 0; c = arg[i]; ++i) {
            if (c == '#' || c == '?' || c == '|')
                break;
        }
        if (c == 0) {
            AddArgument(arg);
            continue;
        }

        tmp1[0] = 0;    /*  clear path buf  */


        error = ExpandElements(tmp1, 0, arg, tmp2);
    }
    if (error == 0) {
        MyAv[AcIdx] = NULL;
        *nac = AcIdx;
        *nav = MyAv;
    }
    return(error);
}

/*
 *  ExpandElements(tmp1, t1len, wild, tmp2)
 *      tmp1    pointer to constructed path so far (is the current directory)
 *      t1len   index to end of constructed path
 *      wild    wildcard argument path remainder
 *      tmp2    scratch buf
 */

static int
ExpandElements(tmp1, t1len, wild, tmp2)
char *tmp1;
short t1len;
char *wild;
char *tmp2;
{
    BPTR oldLock;
    BPTR lock;
    short argOk = 1;
    int error = 0;

    {
        lock = Lock("", SHARED_LOCK);

        if (lock == NULL)
            return(-1);
        oldLock = CurrentDir(lock);
    }

    while (*wild) {
        char c;
        short i;
        short fndwild = 0;

        for (i = 0; c = wild[i]; ++i) {
            switch(c) {
            case '/':
            case ':':
            case 0:
                break;
            case '#':
            case '?':
            case '(':
            case '|':
                fndwild = 1;
            default:
                continue;
            }
            break;
        }
        strncpy(tmp2, wild, i);
        tmp2[i] = 0;
        if (wild[i] == ':')
            strcat(tmp2, ":");
        if (i == 0 && wild[i] == '/')
            strcat(tmp2, "/");

        /*printf("fndw %d tmp1=%s tmp2=%s i = %d wild = %s\n", fndwild, tmp1, tmp2, i, wild);*/

        if (fndwild == 0) {
            lock = Lock(tmp2, SHARED_LOCK);
            if (lock == NULL) {         /*  path element failed */
                argOk = 0;
                break;
            }
            if (wild[i] == '/') {       /*  path element?       */
                FIB *fib;

                if (i)
                    strcat(tmp2, "/");
                if (fib = malloc(sizeof(FIB))) {
                    if (Examine(lock, fib) == 0 || fib->fib_DirEntryType < 0) {
                        argOk = 0;
                        free(fib);
                        UnLock(lock);
                        break;
                    }
                    free(fib);
                }
            }

            UnLock(CurrentDir(lock));   /*  success, append     */

            strcat(tmp1, tmp2);
            t1len = strlen(tmp1);

            wild += i;
            if (*wild == ':' || *wild == '/')
                ++wild;
        } else {
            FIB *fib;
            void *wildnode;
            short addslash = 0;

            _SetWildStack(2048);
            wild += i;
            if (wild[0] == '/') {
                ++wild;
                addslash = 1;
            }

            if (wildnode = _ParseWild(tmp2, strlen(tmp2))) {
                if (fib = malloc(sizeof(FIB))) {
                    if (Examine(lock, fib) && fib->fib_DirEntryType > 0) {
                        while (error >= 0 && ExNext(lock, fib)) {
                            long xlock;

                            /*printf("FIB: %s\n", fib->fib_FileName);*/
                            if (_CompWild(fib->fib_FileName, wildnode, NULL) >= 0) {
                                if (xlock = Lock(fib->fib_FileName, SHARED_LOCK)) {
                                    xlock = CurrentDir(xlock);
                                    strcpy(tmp1 + t1len, fib->fib_FileName);
                                    if (addslash)
                                        strcat(tmp1 + t1len, "/");
                                    /*printf("ADDSL %d, wild %s\n", addslash, wild);*/
                                    if (ExpandElements(tmp1, t1len + addslash + strlen(fib->fib_FileName), wild, tmp2 + strlen(tmp2) + 1) < 0)
                                        error = -1;
                                    UnLock(CurrentDir(xlock));
                                }
                            } else {
                                ;
                            }
                        }
                    }
                    free(fib);
                }
                _FreeWild(wildnode);
            }
            argOk = 0;
            break;
        }
    }

    /*
     *  'lock' variable could be illegal here
     */

    if (argOk) {
        if (AddArgument(tmp1) < 0)
            error = -1;
    }
    UnLock(CurrentDir(oldLock));
    return(error);
}

static int
AddArgument(arg)
char *arg;
{
    MyAv[AcIdx] = strdup(arg);
    if (++AcIdx == AcMax) {
        AcMax *= 2;
        MyAv = realloc(MyAv, sizeof(char *) * AcMax);
        if (MyAv == NULL)
            return(-1);
    }
    return(0);
}

