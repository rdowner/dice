/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  libmake.c
 *
 *  libmake  -n -oobjdir -l library file [-clean] [-pr] [-mr/R] [-proto] -Ddefine
 *              -prof
 *
 *  loads 'files' list and compiles out of date files accordingly, putting
 *  objects in an alternate directory if requested.
 *
 *  if -pr is specified it will be passed on to the compiler
 */

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <libraries/dos.h>
#define abs
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/alib_protos.h>
#include <lists.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lib/misc.h>
#include <lib/version.h>

#define EF_OOD      0x01
#define EF_COMP     0x02
#define EF_NOJOIN   0x04

typedef struct {
    char *SrcName;
    char *ObjName;
    int32_t Flags;
} NameNode;

typedef struct Node Node;
typedef struct List List;
typedef struct MinNode MinNode;
typedef struct MinList MinList;

typedef struct DateStamp DateStamp;

char Prefix[64];
char *CtlFile;
char *ObjDir;
char *DestLib;
NameNode *NList;
short   NumFiles;
short   DccExec;
short   DryRun;
short   Verbose;
short   Broke;
short   Clean;
short   PROpt;
short   ProtoOpt;
short   RegCallOpt;
short   LargeDataOpt;
short   LargeCodeOpt;
short   ProfOpt;
List    DefList;
List    ComFileList;
static char Buf[256];

int brk(void);
int main(int, char **);
void help(int);
int TimeCompareFiles(void);
int CompileFiles(void);
int Assemble(char *, char *);
int AssembleA68K(char *, char *);
int Compile(char *, char *);
int CompareTimes(DateStamp *, DateStamp *);
int ALibMake(void);
char *MungeFile(char *, char *);
char *GetIndex(char **);
int run_cmd(char *, int);
int GetFileTime(char *, DateStamp *);
Node *MakeNode(const char *);

#ifdef LATTICE
MinNode *GetHead(MinList *);
MinNode *GetSucc(MinNode *);
#endif

int
brk()
{
    Broke = 1;
    return(0);
}

main(ac, av)
int ac;
char *av[];
{
    char *nameBuf;

    onbreak(brk);


    NewList(&DefList);
    NewList(&ComFileList);

    /*
     *  calculate prefix
     */

#ifdef _DCC
    DccExec = 1;
#else
    DccExec = 0;
#endif

    {
        char *ptr = av[0];
        short len = strlen(ptr);

        while (len >= 0 && ptr[len] != '/' && ptr[len] != ':')
            --len;
        ptr = ptr + len + 1;    /*  file part   */

        for (len = 0; ptr[len] && ptr[len] != '_'; ++len);
        if (ptr[len] == '_')
            ++len;
        else
            len = 0;
        strncpy(Prefix, ptr, len);
        Prefix[len] = 0;
    }
    {
        short i;

        for (i = 1; i < ac; ++i) {
            char *ptr = av[i];
            if (*ptr != '-') {
                if (CtlFile == NULL) {
                    CtlFile = ptr;
                    continue;
                }
                AddTail(&ComFileList, MakeNode(ptr));
                continue;
            }
            ptr += 2;
            switch(ptr[-1]) {
            case 'v':
                Verbose = 1;
                break;
            case 'n':
                DryRun = 1;
                break;
            case 'D':
                if (*ptr == 0)
                    ptr = av[++i];
                AddTail(&DefList, MakeNode(ptr));
                break;
            case 'o':
                if (*ptr)
                    ObjDir = ptr;
                else
                    ObjDir = av[++i];
                break;
            case 'l':
                if (*ptr)
                    DestLib = ptr;
                else
                    DestLib = av[++i];
                break;
            case 'c':
                Clean = 1;
                DestLib = "";
                break;
            case 'p':
                if (strcmp(ptr, "rof") == 0)
                    ProfOpt = 1;
                else if (strcmp(ptr, "r") == 0)
                    PROpt = 1;
                else if (strcmp(ptr, "roto") == 0)
                    ProtoOpt = 1;
                else
                    help(1);
                break;
            case 'm':
                switch(*ptr) {
                case 'r':
                    RegCallOpt = 1;
                    break;
                case 'R':
                    RegCallOpt = 2;
                    if (ptr[1] == 'R')
                        RegCallOpt = 3;
                    break;
                case 'D':
                    LargeDataOpt = 1;
                    break;
                case 'C':
                    LargeCodeOpt = 1;
                    break;
                }
                break;
            default:
                help(1);
            }
        }
        if (i > ac)
            help(1);
    }
    if (ObjDir == NULL) {
        puts("must specify an output directory for objects (-o)");
        exit(1);
    }
    if (DestLib == NULL) {
        puts("must specify output library (-l)");
        exit(1);
    }
    {
        FILE *fi = fopen(CtlFile, "r");
        int32_t siz;

        if (fi == NULL) {
            perror("fopen");
            exit(1);
        }
        fseek(fi, 0L, 2);
        siz = ftell(fi);
        if (siz < 0) {
            perror("fseek");
            exit(1);
        }
        fseek(fi, 0L, 0);
        nameBuf = malloc(siz + 1);
        if (fread(nameBuf, siz, 1, fi) != 1) {
            perror("fread");
            exit(1);
        }
        fclose(fi);
        nameBuf[siz] = 0;
    }
    {
        char *namePtr;
        char *getIdx;
        int32_t files = 0;
        NameNode *nn;

        getIdx = nameBuf;

        for (namePtr = GetIndex(&getIdx); *namePtr; namePtr = GetIndex(&getIdx))
            ++files;
        {
            Node *node;
            for (node = GetHead(&ComFileList); node; node = GetSucc(node))
                ++files;
        }
        NList = malloc(sizeof(NameNode) * files);

        nn = NList;

        getIdx = nameBuf;
        NumFiles = 0;
        for (namePtr = GetIndex(&getIdx); *namePtr; namePtr = GetIndex(&getIdx)) {
            if (*namePtr == ';')
                continue;
            nn->Flags = 0;
            if (namePtr[0] == '*') {
                nn->Flags |= EF_NOJOIN;
                ++namePtr;
            }
            nn->SrcName = namePtr;
            nn->ObjName = MungeFile(nn->SrcName, ObjDir);
            ++nn;
            ++NumFiles;
        }
        {
            Node *node;
            for (node = GetHead(&ComFileList); node; node = GetSucc(node)) {
                nn->Flags = 0;
                if (node->ln_Name[0] == '*') {
                    nn->Flags |= EF_NOJOIN;
                    ++node->ln_Name;
                }
                nn->SrcName = node->ln_Name;
                nn->ObjName = MungeFile(node->ln_Name, ObjDir);
                ++nn;
                ++NumFiles;
            }
        }
    }

    if (Clean) {
        NameNode *nn;
        short i;

        for (i = 0, nn = NList; i < NumFiles; ++i, ++nn) {
            if (strcmp(nn->SrcName, nn->ObjName) != 0) {
                printf("%s\n", nn->ObjName);
                remove(nn->ObjName);
            }
        }
    } else {
        if (TimeCompareFiles() < 0) {
            if (CompileFiles() != 0)
                exit(10);
            if (ALibMake() < 0)
                exit(20);
        }
    }
    return(0);
}

void
help(n)
{
    fprintf(stderr, "libmake [-n] files -o objdir/ -l library [-clean]\n");
    exit(n);
}

int
TimeCompareFiles()
{
    NameNode *nn;
    short i;
    short r = 0;
    DateStamp d1;
    DateStamp d2;
    DateStamp d3;

    if (GetFileTime(DestLib, &d3) < 0)
        r = -1;

    for (i = 0, nn = NList; i < NumFiles; ++i, ++nn) {
        /*
         *  ignore files that are just being inserted
         */

        if (strcmp(nn->SrcName, nn->ObjName) == 0)
            continue;

        /*
         *  compare files that may need compilation, source against
         *  object and object against destination
         */

        if (GetFileTime(nn->SrcName, &d1)) {
            fprintf(stderr, "unable to find %s\n", nn->SrcName);
            continue;
        }
        if (GetFileTime(nn->ObjName, &d2) == 0) {
            if (CompareTimes(&d3, &d2) < 0)
                r = -1;
            if (CompareTimes(&d1, &d2) < 0)
                continue;
        }
        nn->Flags |= EF_OOD;
        r = -1;
    }
    return(r);
}

int
CompareTimes(d1, d2)
DateStamp *d1;
DateStamp *d2;
{
    if (d1->ds_Days < d2->ds_Days)
        return(-1);
    if (d1->ds_Days == d2->ds_Days) {
        if (d1->ds_Minute < d2->ds_Minute)
            return(-1);
        if (d1->ds_Minute == d2->ds_Minute) {
            if (d1->ds_Tick < d2->ds_Tick)
                return(-1);
            if (d1->ds_Tick == d2->ds_Tick)
                return(0);
        }
    }
    return(1);
}


CompileFiles()
{
    NameNode *nn;
    short i;
    int errs = 0;

    for (i = 0, nn = NList; i < NumFiles; ++i, ++nn) {
        if (nn->Flags & EF_OOD) {
            short j;
            char *ptr;

            if (strcmp(nn->SrcName, nn->ObjName) != 0) {
                for (j = strlen(nn->SrcName) - 1; j >= 0 && nn->SrcName[j] != '.'; --j);
                if (++j == 0)
                    ptr = "";
                else
                    ptr = nn->SrcName + j;

                if (stricmp(ptr, "a") == 0 || stricmp(ptr, "asm") == 0) {
                    errs += Assemble(nn->SrcName, nn->ObjName);
                } else if (stricmp(ptr, "a68") == 0) {
                    errs += AssembleA68K(nn->SrcName, nn->ObjName);
                } else {
                    errs += Compile(nn->SrcName, nn->ObjName);
                }
            }
        }
    }
    return(errs);
}

Assemble(src, obj)
char *src;
char *obj;
{
    sprintf(Buf, "%sdas -o%s %s", Prefix, obj, src);
    return(run_cmd(Buf, DccExec));
}

AssembleA68K(src, obj)
char *src;
char *obj;
{
    sprintf(Buf, "a68k -o%s %s -q", obj, src);
    return(run_cmd(Buf, 0));
}

/*
 *  COMPILE:
 *      -r  make residentable code
 *      -ms put const objects, including string constants, in the code segment
 */

Compile(src, obj)
char *src;
char *obj;
{
    char *verb = (Verbose) ? "-v" : "";
    char *propt= (PROpt) ? " -pr" : "";
    char *proto= (ProtoOpt) ? " -proto" : "";
    char *prof = (ProfOpt) ? " -prof" : "";
    char *rco;
    char *ldata= (LargeDataOpt) ? " -mD" : "";
    char *lcode= (LargeCodeOpt) ? " -mC" : "";

    switch(RegCallOpt) {
    case 0:
        rco = "";
        break;
    case 1:
        rco = " -mr";
        break;
    case 2:
        rco = " -mR";
        break;
    case 3:
        rco = " -mRR";
        break;
    }

    sprintf(Buf, "%sdcc -c %s -o %s %s -S -r -ms%s%s%s%s%s%s",
        Prefix, src, obj, verb, propt, rco, proto, prof, ldata, lcode
    );
    {
        Node *node;
        short len = strlen(Buf);
        for (node = GetHead(&DefList); node; node = GetSucc(node)) {
            len += sprintf(Buf + len, " -D%s", node->ln_Name);
        }
    }
    return(run_cmd(Buf, DccExec));
}

ALibMake()
{
    NameNode *nn;
    short i;
    int r = -1;
    FILE *fo;

    if (DryRun)
        return(0);

    printf("Creating alink 'join' type libray %s\n", DestLib);
    if (fo = fopen(DestLib, "w")) {
        r = 0;
        for (i = 0, nn = NList; i < NumFiles; ++i, ++nn) {
            FILE *fi;
            if (nn->Flags & EF_NOJOIN)
                continue;
            if (fi = fopen(nn->ObjName, "r")) {
                int c;
                while ((c = getc(fi)) != EOF)
                    putc(c, fo);
                fclose(fi);
            } else {
                printf("Couldn't read %s\n", nn->ObjName);
                r = -1;
            }
        }
        fclose(fo);
    } else {
        printf("Unable to create %s\n", DestLib);
    }
    return(r);
}



char *
MungeFile(name, prefix)
char *name;
char *prefix;
{
    char *str;
    char *postfix = ".o";
    short i;

    for (i = strlen(name) - 1; i >= 0 && name[i] != '.'; --i);
    if (i < 0)
        i = strlen(name);

    if (stricmp(name + i, ".o") == 0 || stricmp(name + i, ".lib") == 0) {
        str = malloc(strlen(name) + 1);
        strcpy(str, name);
    } else {
        str = malloc(strlen(prefix) + i + strlen(postfix) + 1);
        strcpy(str, prefix);
        sprintf(str + strlen(str), "%.*s%s", (int32_t)i, name, postfix);
    }
    return(str);
}

char *
GetIndex(ip)
char **ip;
{
    char *ptr = *ip;
    char *rp;

    while (*ptr == ' ' || *ptr == '\t' || *ptr == '\n')
        ++ptr;
    if (*ptr == 0)
        return(ptr);
    rp = ptr;
    while (*ptr && *ptr != '\n')
        ++ptr;
    *ptr = 0;
    *ip = ptr + 1;
    return(rp);
}

run_cmd(buf, dccExec)
char *buf;
{
    printf("%s\n", buf);
    if (Broke) {
        printf("^C\n");
        exit(1);
    }
    if (DryRun)
        return(0);

#ifdef _DCC
    if (dccExec) {
        short i;
        int32_t r;

        for (i = 0; buf[i] && buf[i] != ' '; ++i);
        if (buf[i] == ' ') {
            buf[i] = 0;
            for (++i; buf[i] == ' '; ++i);
        }
        r = exec_dcc(buf, buf + i);
        if (r) {
            printf("Exit code %d\n", r);
            exit(1);
        }
        return(r);
    }
#endif

    Execute(buf, NULL, Output());
    return(0);
}

GetFileTime(file, ds)
char *file;
DateStamp *ds;
{
    static struct FileInfoBlock *Fib;
    int32_t rv = -1;
    BPTR lock;

    if (Fib == NULL)
        Fib = malloc(sizeof(struct FileInfoBlock));
    if (lock = Lock(file, SHARED_LOCK)) {
        if (Examine(lock, Fib)) {
            *ds = Fib->fib_Date;
            rv = 0;
        }
        UnLock(lock);
    }
    return(rv);
}

Node *
MakeNode(str)
const char *str;
{
    Node *node = malloc(sizeof(Node) + strlen(str) + 1);
    node->ln_Name = (char *)(node + 1);
    strcpy(node->ln_Name, str);
    return(node);
}

#ifdef LATTICE

/*
 *  required since this program needs to be used by the bootstrap compile,
 *  before DICE's c.lib exists.
 */

MinNode *
GetHead(list)
MinList *list;
{
    MinNode *node;
    node = list->mlh_Head;
    if (node->mln_Succ)
        return(node);
    return(NULL);
}

MinNode *
GetSucc(node)
MinNode *node;
{
    node = node->mln_Succ;
    if (node->mln_Succ)
        return(node);
    return(NULL);
}

#endif
