/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  DUPDATE.C
 *
 *  Distribution Update.
 *
 *  DUPDATE dist-dir dest-dir   [FORCE][QUIET][NODEL] [DISTFILE distfilename]
 *  DUPDATE dist-file dest-dir  [FORCE][QUIET][NODEL] [DISTFILE distfilename]
 */

#include <exec/types.h>
#include <exec/lists.h>
#include <exec/memory.h>
#include <libraries/dos.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/alib_protos.h>
#include <lib/version.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct MinList  MLIST;
typedef struct List     LIST;
typedef struct Node     NODE;
typedef struct FileInfoBlock FIB;

MLIST   ScanList;       /*  scan list   */
char    Buf[256];

char    *DistFileName = ".DistFiles";
char    Force;
char    Quiet;
char    NoDel;
char    Broke;

#ifdef _DCC
IDENT("dupdate",".3");
DCOPYRIGHT;
#endif

typedef struct {
    NODE    Node;       /*  link node       */
    MLIST   List;       /*  if directory    */
    FIB     *Fib[2];    /*  file/dir info   */
} SNODE;

int brk(void);
int main(short, char **);
void Scan(int32_t, MLIST *, int, int);
void Update(int32_t, int32_t, MLIST *);
SNODE *SNodeIn(FIB *, MLIST *, int, int);
int CheckBroke(void);
int getyn(char *, char *);
int CopyFile(int32_t, int32_t, char *, char *, FIB *);
int DeleteDir(char *);
NODE *FindNode(LIST *, char *);

extern void *GetHead(void *);       /*   cr.lib funcs are registered */
extern void *GetSucc(void *);

int
brk()
{
    Broke = 1;
    return(0);
}

main(ac, av)
short ac;
char *av[];
{
    int32_t slock = 0;
    int32_t dlock = 0;
    char *sd = NULL;
    char *dd = NULL;

    onbreak(brk);
    NewList((LIST *)&ScanList);
    if (ac < 3) {
        puts("DUPDATE dist-dir dest-dir");
        exit(1);
    }
    {
        short i;
        for (i = 1; i < ac; ++i) {
            char *ptr = av[i];
            if (stricmp(ptr, "DISTFILE") == 0) {
                DistFileName = av[++i];
                continue;
            }
            if (stricmp(ptr, "FORCE") == 0) {
                Force = 1;
                continue;
            }
            if (stricmp(ptr, "QUIET") == 0) {
                Quiet = 1;
                continue;
            }
            if (stricmp(ptr, "NODEL") == 0) {
                NoDel = 1;
                continue;
            }
            if (sd == NULL) {
                sd = ptr;
                continue;
            }
            if (dd == NULL) {
                dd = ptr;
                continue;
            }
            puts("Too many arguments!");
            exit(1);
        }
        if (i > ac) {
            puts("Expected argument to option");
            exit(1);
        }
    }
#ifdef DEBUG
    puts("ScanA");
#endif
    slock = (int32_t)Lock(sd, ACCESS_READ);
    if (slock == NULL) {
        printf("can't open %s\n", sd);
        goto fail;
    }
#ifdef DEBUG
    puts("ScanB");
#endif
    dlock = (int32_t)Lock(dd, ACCESS_READ);
    if (dlock == NULL) {
        if (dlock = (int32_t)CreateDir(dd)) {
            UnLock(dlock);
            dlock = (int32_t)Lock(dd, ACCESS_READ);
        }
    }
#ifdef DEBUG
    puts("ScanC");
#endif
    if (dlock == NULL) {
        printf("can't open/create %s\n", dd);
        goto fail;
    }
#ifdef DEBUG
    puts("Scan1");
#endif
    Scan(slock, &ScanList, 0, 0);
#ifdef DEBUG
    puts("Scan2");
#endif
    Scan(dlock, &ScanList, 1, 0);
#ifdef DEBUG
    puts("Update");
#endif
    Update(slock, dlock, &ScanList);
fail:
#ifdef DEBUG
    puts("End");
#endif
    if (slock)
        UnLock(slock);
    if (dlock)
        UnLock(dlock);
    exit(1);
}

/*
 *  Scan.  Look for a .DistFiles file which contains a list of additional
 *  files and files to not include.
 */

void
Scan(lock, list, side, ignoreNoMatch)
int32_t lock;
MLIST *list;
int side;
int ignoreNoMatch;
{
    FIB *fib;
    SNODE *sn;
    FILE *fi;
    char *distfile;
    int32_t savlock;
    MLIST nolist;
    short onlyFlag = 0;

    NewList((LIST *)&nolist);

    if (CheckBroke())
        return;

#ifdef DEBUG
    printf("Examine %08lx\n", lock);
#endif

    fib = AllocMem(sizeof(FIB), MEMF_PUBLIC|MEMF_CLEAR);
    if (Examine(lock, fib) && fib->fib_DirEntryType < 0)    /*  file    */
        distfile = "";
    else
        distfile = DistFileName;

#ifdef DEBUG
    puts("CD1");
#endif
    savlock = (int32_t)CurrentDir(lock);
#ifdef DEBUG
    puts("CD2");
#endif
    if (side == 0 && (fi = fopen(distfile, "r"))) {
#ifdef DEBUG
        printf("open-success %s\n", distfile);
#endif
        while (fgets(Buf, sizeof(Buf), fi)) {
            short len = strlen(Buf);
            int32_t lock2;
#ifdef DEBUG
            printf("BUF: %s\n", Buf);
#endif

            if (CheckBroke())
                break;
            {
                char *ptr;
                if (strtok(Buf, " \t\n") == NULL)
                    continue;
                if (ptr = strtok(NULL, " \t\n")) {  /*  NO keyword */
                    if (stricmp(ptr, "NO") == 0) {
                        NODE *node = malloc(sizeof(NODE) + strlen(Buf) + 1);
                        node->ln_Name = (char *)(node + 1);
                        strcpy(node->ln_Name, Buf);
                        AddTail((LIST *)&nolist, node);
                        continue;
                    }
                    if (stricmp(ptr, "ONLY") == 0)
                        onlyFlag = 1;
                }
            }
            lock2 = (int32_t)Lock(Buf, ACCESS_READ);
            if (lock2 == NULL)
                continue;
            if (Examine(lock2, fib)) {
                if (fib->fib_DirEntryType < 0) {
                    sn = SNodeIn(fib, list, side, ignoreNoMatch);
                    strcpy(sn->Fib[side]->fib_FileName, Buf);
                } else {
                    if (sn = SNodeIn(fib, list, side, ignoreNoMatch)) {
                        strcpy(sn->Fib[side]->fib_FileName, Buf);
                        Scan(lock2, &sn->List, side, ignoreNoMatch);
                        /* Buf destroyed */
                    }
                }
            }
            UnLock(lock2);
        }
        fclose(fi);
    }
#ifdef DEBUG
    puts("Cont1");
#endif
    if (onlyFlag == 0 && Examine(lock, fib) && fib->fib_DirEntryType > 0) {
#ifdef DEBUG
        printf("Directory %s\n", fib->fib_FileName);
#endif
        while (ExNext(lock, fib)) {
            if (CheckBroke())
                break;
#ifdef DEBUG
            printf("Entry %s\n", fib->fib_FileName);
#endif
                                                    /*  ignore file */
            if (FindNode((LIST *)&nolist, fib->fib_FileName))
                continue;
            if (fib->fib_DirEntryType < 0) {        /*  plain file  */
                sn = SNodeIn(fib, list, side, ignoreNoMatch);
            } else if (fib->fib_DirEntryType > 0) { /*  directory   */
                if (sn = SNodeIn(fib, list, side, ignoreNoMatch)) {
                    int32_t lock2;
                    if (lock2 = (int32_t)Lock(fib->fib_FileName, ACCESS_READ)) {
                        Scan(lock2, &sn->List, side, ignoreNoMatch);
                        UnLock(lock2);
                    }
                }
            }
        }
    }
#ifdef DEBUG
    puts("scanend1");
#endif
    CheckBroke();
    FreeMem(fib, sizeof(FIB));
    CurrentDir(savlock);
#ifdef DEBUG
    puts("scanend2");
#endif
    {
        NODE *node;
        while (node = RemHead((LIST *)&nolist))
            free(node);
    }
#ifdef DEBUG
    puts("scanend3");
#endif
}

short tab = -4;

void
Update(slock, dlock, list)
int32_t slock, dlock;
MLIST *list;
{
    SNODE *sn;

    if (CheckBroke())
        return;
    tab += 4;

#ifdef DEBUG
    puts("update1");
#endif

    for (sn = GetHead(list); sn; sn = GetSucc(sn)) {

#ifdef DEBUG
        puts("xupdate1");
#endif
        if (CheckBroke())
            break;
#ifdef DEBUG
        puts("xupdate2");
#endif

        if (Quiet == 0) {
            short i;
            for (i = tab; i; --i)
                printf(" ");
            if (sn->Fib[0])
                printf("%-20s\t", sn->Node.ln_Name);
            else
                printf("%-20s\t", "-");

            if (sn->Fib[1])
                printf("%-20s\t", sn->Node.ln_Name);
            else
                printf("%-20s\t", "-");
        }

#ifdef DEBUG
        puts("xupdate3");
#endif

        if (sn->Fib[0] && sn->Fib[1]) {
            if (strncmp(sn->Fib[0]->fib_Comment, "NODIST", 6) == 0)
                sn->Fib[0] = NULL;
        }

#ifdef DEBUG
        puts("xupdate4");
#endif

        if (sn->Fib[0] && sn->Fib[1]) {
#ifdef DEBUG
            puts("xupdate5");
#endif

            if (sn->Fib[0]->fib_DirEntryType > 0) {
                int32_t savlock;
                int32_t locks;
                int32_t lockd;

                if (!Quiet)
                    puts("");
                savlock = (int32_t)CurrentDir(slock);
                locks = (int32_t)Lock(sn->Fib[0]->fib_FileName, ACCESS_READ);
                CurrentDir(dlock);
                lockd = (int32_t)Lock(sn->Fib[1]->fib_FileName, ACCESS_READ);
                CurrentDir(savlock);
                if (locks && lockd)
                    Update(locks, lockd, &sn->List);
                else
                    printf("software error %s\n", sn->Node.ln_Name);
                if (locks)
                    UnLock(locks);
                if (lockd)
                    UnLock(lockd);
            } else {                        /*  file    */
                if (sn->Fib[0]->fib_Date == sn->Fib[1]->fib_Date && sn->Fib[0]->fib_Size == sn->Fib[1]->fib_Size) {
                    if (!Quiet)
                        puts("(ok)");
                } else {
                    if (!Quiet)
                        puts("(update)");
                    CopyFile(slock, dlock, sn->Fib[0]->fib_FileName, sn->Fib[1]->fib_FileName, sn->Fib[0]);
                }
            }
        } else if (sn->Fib[0]) {            /*  file/dir in dist-dir only */
#ifdef DEBUG
            puts("xupdate6");
#endif
            if (strncmp(sn->Fib[0]->fib_Comment, "NODIST", 6) == 0) {
                if (!Quiet)
                    puts("(NODIST)");
                continue;
            }

            if (sn->Fib[0]->fib_DirEntryType < 0) {     /*  file    */
                if (!Quiet) {
                    if (Force)
                        puts("(newfile, copy)");
                    else
                        puts("");
                }
                if (Force || getyn("Copy File %s ? ", sn->Node.ln_Name)) {
                    CopyFile(slock, dlock, sn->Fib[0]->fib_FileName, sn->Node.ln_Name, sn->Fib[0]);
                }
            } else {
                if (!Quiet) {
                    if (Force)
                        puts("(newdir, copy)");
                    else
                        puts("");
                }
                if (Force || getyn("Copy Dir  %s ? ", sn->Node.ln_Name)) {
                    int32_t locks;
                    int32_t lockd;
                    int32_t savlock;

                    savlock = (int32_t)CurrentDir(slock);
                    locks = (int32_t)Lock(sn->Fib[0]->fib_FileName, ACCESS_READ);
                    CurrentDir(dlock);
                    if (lockd = (int32_t)CreateDir(sn->Node.ln_Name)) {
                        UnLock(lockd);
                        lockd = (int32_t)Lock(sn->Node.ln_Name, ACCESS_READ);
                    }
                    CurrentDir(savlock);
                    if (locks && lockd)
                        Update(locks, lockd, &sn->List);
                    if (locks)
                        UnLock(locks);
                    if (lockd)
                        UnLock(lockd);
                }
            }
        } else if (sn->Fib[1]) {            /*  file/dir in dest-dir only */
#ifdef DEBUG
            puts("xupdate7");
#endif
            if (strncmp(sn->Fib[1]->fib_Comment, "DISTKEEP", 8) == 0) {
                if (!Quiet)
                    puts("(KEEP)");
                continue;
            }
            if (NoDel) {
                if (!Quiet)
                    puts("(ignore)");
                continue;
            }
            if (!Quiet)
                puts("");

            if (sn->Fib[1]->fib_DirEntryType < 0) {     /*  file    */
                if (getyn("File %s not in source tree,\ndelete from dest? ", sn->Node.ln_Name)) {
                    int32_t savlock = (int32_t)CurrentDir(dlock);
                    if (DeleteFile(sn->Node.ln_Name) == 0) {
                        SetProtection(sn->Node.ln_Name, 0);
                        if (DeleteFile(sn->Node.ln_Name) == 0)
                            puts("(unable to delete)");
                    }
                    CurrentDir(savlock);
                }
            } else {
                if (getyn("Dir %s not in source tree,\ndelete from dest? ", sn->Node.ln_Name)) {
                    int32_t savlock = (int32_t)CurrentDir(dlock);
                    DeleteDir(sn->Node.ln_Name);
                    CurrentDir(savlock);
                }
            }
        }
#ifdef DEBUG
        puts("xupdate8");
#endif

    }
    tab -= 4;
}

SNODE *
SNodeIn(fib, list, side, ignm)
FIB *fib;
MLIST *list;
int side;
{
    SNODE *sn;

    for (sn = GetHead(list); sn; sn = GetSucc(sn)) {
        if (stricmp(fib->fib_FileName, sn->Node.ln_Name) == 0)
            break;
    }
    if (sn == NULL) {
        if (ignm)
            return(NULL);
        sn = malloc(sizeof(SNODE));
        setmem(sn, sizeof(SNODE), 0);
        NewList((LIST *)&sn->List);
        AddTail((LIST *)list, (NODE *)sn);
    }
    if (sn->Fib[side] == NULL) {
        sn->Fib[side] = malloc(sizeof(FIB));
        setmem(sn->Fib[side], sizeof(FIB), 0);
    }
    *sn->Fib[side] = *fib;
    if (sn->Node.ln_Name == NULL) {
        sn->Node.ln_Name = malloc(strlen(fib->fib_FileName)+1);
        strcpy(sn->Node.ln_Name, fib->fib_FileName);
    }
    return(sn);
}

CheckBroke()
{
    if (Broke == 0 && SetSignal(0,SIGBREAKF_CTRL_C|SIGBREAKF_CTRL_D) & (SIGBREAKF_CTRL_C|SIGBREAKF_CTRL_D)) {
        puts("ABORT");
        Broke = 1;
    }
    return(Broke);
}

getyn(s1, s2)
char *s1, *s2;
{
    char buf[256];

#ifdef DEBUG
    puts("gyn");
#endif
    for (;;) {
        printf(s1, s2);
        fflush(stdout);
        if (gets(buf) == NULL)
            return(0);
        if (buf[0] == 'y' || buf[0] == 'Y')
            return(1);
        if (buf[0] == 'n' || buf[0] == 'N')
            return(0);
        puts("");
    }
}

CopyFile(sdir, ddir, sname, dname, dfib)
int32_t sdir, ddir;
char *sname;
char *dname;
FIB *dfib;
{
    int32_t Fhs = 0;
    int32_t Fhd = 0;
    int32_t savlock;
    int32_t bufsiz;
    int32_t n;
    int32_t r;
    char *buf = NULL;
    short error = 0;

#ifdef DEBUG
    puts("xfile1");
    printf("xfile1 %08lx %08lx %s %s %08lx\n", sdir, ddir, sname, dname, dfib);
#endif
    savlock = (int32_t)CurrentDir(sdir);
#ifdef DEBUG
    puts("xfile2");
#endif
    Fhs = (int32_t)Open(sname, 1005);
    if (Fhs == NULL) {
        printf("unable to open %s for read\n", sname);
        error = 1;
        goto fail;
    }
#ifdef DEBUG
    puts("xfile3");
#endif
    CurrentDir(ddir);
#ifdef DEBUG
    puts("xfile4");
#endif
    Fhd = (int32_t)Open(dname, 1006);
#ifdef DEBUG
    puts("xfile5");
#endif
    if (Fhd == NULL) {
        SetProtection(dname, 0);
        DeleteFile(dname);
        Fhd = (int32_t)Open(dname, 1006);
        if (Fhd == NULL) {
            printf("Unable to open %s for write\n", dname);
            error = 1;
            goto fail;
        }
    }
#ifdef DEBUG
    puts("xfile6");
#endif
    for (bufsiz = 32768; bufsiz; bufsiz >>= 1) {
        if (buf = malloc(bufsiz))
            break;
    }
#ifdef DEBUG
    printf("bufsiz = %d\n", bufsiz);
#endif
    while ((n = Read(Fhs, buf, bufsiz)) > 0) {
        r = Write(Fhd, buf, n);
        if (r != n) {
            puts("write failed");
            error = 1;
            goto fail;
        }
    }
#ifdef DEBUG
    puts("xfile7");
#endif
fail:
    if (buf)
        free(buf);
    if (Fhs)
        Close(Fhs);
    if (Fhd)
        Close(Fhd);
#ifdef DEBUG
    puts("xfile8");
#endif
    if (error)
        SetProtection(dname, dfib->fib_Protection & ~(FIBF_READ|FIBF_WRITE));
    else
        SetProtection(dname, dfib->fib_Protection);
#ifdef DEBUG
    puts("xfile9");
#endif
    SetFileDate(dname, &dfib->fib_Date);
    if (dfib->fib_Comment[0])
        SetComment(dname, dfib->fib_Comment);
    CurrentDir(savlock);
#ifdef DEBUG
    puts("xfile10");
#endif

}

DeleteDir(name)
char *name;
{
    FIB *fib = malloc(sizeof(FIB));
    int32_t lock = (int32_t)Lock(name, ACCESS_READ);

    if (lock) {
        if (Examine(lock, fib) && fib->fib_DirEntryType > 0) {
            int32_t savdir = (int32_t)CurrentDir(lock);
            while (ExNext(lock, fib))
                DeleteDir(fib->fib_FileName);
            CurrentDir(savdir);
        }
        UnLock(lock);
        if (DeleteFile(name) == 0) {
            SetProtection(name, 0);
            DeleteFile(name);
        }
    }
    free(fib);
}

NODE *
FindNode(list, name)
LIST *list;
char *name;
{
    NODE *node;

    for (node = GetHead(list); node; node = GetSucc(node)) {
        if (stricmp(node->ln_Name, name) == 0)
            return(node);
    }
    return(NULL);
}

