/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  SUBS.C
 */

#include "defs.h"
#ifdef AMIGA
#include <exec/execbase.h>
#endif

Prototype int ExtArgsEnv(short, char ***, char *);
Prototype int ExtArgsFile(short, char ***, char *);
Prototype int ExtArgsBuf(short, char ***, char *, int32_t);

Prototype char *skipspace(char *);
Prototype char *skipnspace(char *);
Prototype void CreateObjPath(char *);

#ifdef AMIGA
extern struct ExecBase *SysBase;
#endif

/*
 *  ExtArgsEnv()        DCCOPTS
 */

#ifdef AMIGA

int
ExtArgsEnv(short ac, char ***pav, char *envname)
{
    int32_t len = -1;
    char *str;
    int nac = 0;

#ifndef LATTICE
    if (SysBase->LibNode.lib_Version < 36) {
#else
    {
#endif
        char buf[64];

        sprintf(buf, "ENV:%s", envname);
        return(ExtArgsFile(ac, pav, buf));
    }
#ifndef LATTICE
      else {
        str = malloc(1024);
        len = GetVar(envname, str, 1024, 0);
        if (len > 0)
            str = strcpy(malloc(len + 1), str);
        else
            free(str);
        return(ExtArgsBuf(ac, pav, str, len));
    }
#endif
}

#else

int
ExtArgsEnv(short ac, char ***pav, char *envname)
{
    char *str;
    short len;

    if ((str = getenv(envname)) != NULL) {
        len = strlen(str);
        str = strcpy(malloc(len + 1), str);
        return(ExtArgsBuf(ac, pav, str, len));
    }
    return(ac);
}

#endif

int
ExtArgsFile(short ac, char ***pav, char *file)
{
    int fd;
    int32_t len = -1;
    char *str = NULL;

    if ((fd = open(file, O_RDONLY)) >= 0) {
        if ((len = lseek(fd, 0L, 2)) > 0) {
            str = malloc(len + 1);

            lseek(fd, 0L, 0);
            read(fd, str, len);
            str[len] = 0;
        }
        close(fd);
    }
    return(ExtArgsBuf(ac, pav, str, len));
}

int
ExtArgsBuf(short ac, char ***pav, char *str, int32_t len)
{
    char *ptr;
    int nac = 0;
    char **nav;

    if (len < 0)
        return(ac);

    /*
     *  parse
     */

    ptr = skipspace(str);
    while (*ptr) {
        ++nac;
        ptr = skipnspace(ptr);
        ptr = skipspace(ptr);
    }
    nav = malloc((ac + nac + 1) * sizeof(char *));
    movmem(*pav, nav, ac * sizeof(char *));
    nac = ac;
    ptr = skipspace(str);
    while (*ptr) {
        nav[nac] = ptr;
        ptr = skipnspace(ptr);
        if (*ptr)
            *ptr++ = 0;
        ptr = skipspace(ptr);
        ++nac;
    }
    nav[nac] = NULL;
    ac = nac;
    *pav = nav;
    return(ac);
}

char *
skipspace(ptr)
char *ptr;
{
    while (*ptr == '\n' || *ptr == ' ' || *ptr == 9)
        ++ptr;
    return(ptr);
}

char *
skipnspace(ptr)
char *ptr;
{
    while (*ptr != '\n' && *ptr != ' ' && *ptr != 9 && *ptr)
        ++ptr;
    return(ptr);
}

/*
 *  check for path existance
 */

void
CreateObjPath(file)
char *file;
{
    short i;
    short j;
#ifdef AMIGA
    BPTR lock;
#else
    struct stat statBuf;
#endif
    char tmp[128];

    for (i = strlen(file); i >= 0 && file[i] != '/' && file[i] != ':'; --i);

    if (i <= 0)
        return;
    strncpy(tmp, file, i);
    tmp[i] = 0;

    /*
     *  valid directory
     */

#ifdef AMIGA
    if (lock = Lock(tmp, SHARED_LOCK)) {
        UnLock(lock);
        return;
    }
#else
    if (stat(tmp, &statBuf) == 0)
        return;
#endif

    /*
     *  invalid, attempt to create directory path.
     */

    for (j = 0; j <= i; ++j) {
        if (file[j] == '/') {
            strncpy(tmp, file, j);
            tmp[j] = 0;
#ifdef AMIGA
            if (mkdir(tmp) < 0 && errno != EEXIST)
                break;
#else
            if (mkdir(tmp, 0666) < 0 && errno != EEXIST)
                break;
#endif
        }
    }
}

