
/*
 *  FDTOPRAGMA.C
 *
 * (c)Copyright 1992 Obvious Implementations Corp, All Rights Reserved
 *
 *  FDTOPRAGMA fdfile [-o inline_header_file]
 *  FDTOPRAGMA fddir  [-o inlinedir]
 *
 *  Generates an #include file for inline library calls given an FD file
 *  and CLIB-style prototype file.  The generated header file consists of
 *  entries as follows:
 *
 *  void func1(__A6 void *,__D0 int, __D1 int);
 *  #pragma FubarBase func1 2e 0102
 *  ...
 */

#ifndef unix
#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <clib/alib_protos.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/alib_protos.h>
#include <lib/profile.h>
#include <lib/version.h>
#include <lists.h>
#include <time.h>
#else
#include <../suplib/all.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <dirent.h>


#ifdef _DCC
IDENT("fdtopragma", ".9");
DCOPYRIGHT;
#endif

typedef unsigned char  ubyte;
typedef unsigned short uword;
typedef unsigned long  ulong;
typedef struct List     List;
typedef struct Node     Node;

typedef struct FDNode {
    Node    fn_Node;
    short   fn_Args;
    long    fn_Offset;      /*  library offset      */
    char    fn_Regs[32];    /*  transfer registers  */
} FDNode;

void    help(short);
void    exiterr(const char *, ...);
void    ScanFD(FILE *);
char    *ParseArg(char *, char **);
void    GenerateOutput(FILE *, char *);
void    GenerateFunction(char *, long);
void    GeneratePragmas(char *, char *);

List    FDList;         /*  list of FD files   */

char    *BaseVar;
char    *BaseVarPtr;

char    Buf[256];

int _DiceCacheEnable = 1;

main(ac, av)
short ac;
char **av;
{
    short i;
    struct stat s;
    char *fdName = NULL;
    char *outName = NULL;

    for (i = 1; i < ac; ++i) {
        char *ptr = av[i];

        if (*ptr != '-') {
            if (fdName == NULL)fdName = ptr;
            else help(1);
            continue;
        }
        ptr += 2;
        switch(ptr[-1]) {
        case 'o':
            outName = (*ptr) ? ptr : av[++i];
            break;
        default:
            help(0);
            break;
        }
    }
    if (fdName == NULL)
        help(0);

    /*
     *  HACK, handle case where fdName is a directory
     */
    if (stat(fdName, &s) == 0 && (s.st_mode & S_IFDIR)) {
        DIR *dir;
        struct dirent *den;

        if (dir = opendir(fdName)) {
            while ((den = readdir(dir)) != NULL) {
                char *ptr;

                if (ptr = strstr(den->d_name, "_lib.fd")) {
                    static char FdPath[256];
                    static char OutPath[256];

                    sprintf(FdPath, "%s%s", fdName, den->d_name);
                    sprintf(OutPath, "%s%.*s_pragmas.h", outName ? outName : "",
                                     ptr - den->d_name, den->d_name);
                    GeneratePragmas(FdPath, OutPath);
                } else {
                    fprintf(stderr, "%s: expected '_lib.fd' trailer\n");
                }
            }
            closedir(dir);
        }
    }
    else {
        GeneratePragmas(fdName, outName);
    }
    return(0);
}

void
GeneratePragmas(fdFile, outFile)
char *fdFile;
char *outFile;
{
    NewList(&FDList);
    BaseVarPtr = BaseVar = NULL;

    if(outFile)printf("FD=%s OUT=%s\n", fdFile, outFile);
    else printf("FD=%s\n", fdFile);

    /*
     *  Generate FD specifications from FD file
     */

    {
        FILE *fi;
        if ((fi = fopen(fdFile, "r")) == NULL)
            exiterr("Unable to open .FD file: %s", fdFile);
        ScanFD(fi);
        fclose(fi);
    }

    /*
     *  Generate output
     */

    if (outFile) {
        FILE *fo;

        if (fo = fopen(outFile, "w")) {
            GenerateOutput(fo, outFile);
            fclose(fo);
        } else {
            exiterr("Can't create %s\n", outFile);
        }
    } else {
        GenerateOutput(stdout, outFile);
    }
}

void
exiterr(const char *ctl, ...)
{
    va_list va;

    va_start(va, ctl);
    vfprintf(stderr, ctl, va);
    va_end(va);
    fprintf(stderr, "\n");
    exit(5);
}

void
help(short code)
{
#ifdef _DCC
    puts(Ident);
#endif
    puts("FDTOPRAGMA fdfile [-o outfile]");
    puts("FDTOPRAGMA fddir/ [-o outdir/]");
    puts("  Generates header files for inline library calls");
    exit(code);
}

void
ScanFD(fi)
FILE *fi;
{
    long bias = -1;
    short end = 0;
    short public = 1;

    char *key;

    while (fgets(Buf, sizeof(Buf), fi)) {
        if (Buf[0] == '\n' || Buf[0] == '*')
            continue;
        if (strncmp(Buf, "##", 2) != 0) {
            if (bias < 0 || BaseVar == NULL) {
                if (bias < 0) {
                    bias = 30;
                    printf("Error, No ##bias before function: %s\n", Buf);
                }
                if (BaseVar == NULL) {
                    BaseVarPtr = BaseVar = strdup("UnknownBase");
                    printf("Error, No ##base before function: %s\n", Buf);
                }
            }
            if (public)
                GenerateFunction(Buf, bias);
            bias += 6;
            continue;
        }
        if ((key = strtok(Buf + 2, " \t\n")) == NULL) {
            printf("\tError, Illegal null directive\n");
            continue;
        }
        if (stricmp(key, "base") == 0) {
            if (key = strtok(NULL, " \t\n")) {
                if (BaseVar)
                    free(BaseVar);
                BaseVarPtr = BaseVar = strdup(key);
                if (BaseVar[0] == '_')
                    ++BaseVarPtr;
            } else {
                printf("\tError, Illegal ##base directive\n");
            }
            continue;
        }
        if (stricmp(key, "bias") == 0) {
            if (key = strtok(NULL, " \t\n")) {
                char *dummy;

                bias = strtol(key, &dummy, 0);
                if (bias <= 0)
                    printf("\tError, Illegal ##bias: %d\n", bias);
            } else {
                printf("\tError, Illegal ##bias directive\n");
            }
            continue;
        }
        if (stricmp(key, "public") == 0) {
            public = 1;
            continue;
        }
        if (stricmp(key, "private") == 0) {
            public = 0;
            continue;
        }
        if (stricmp(key, "end") == 0) {
            end = 1;
            break;
        }
        printf("\tError, Unrecognized directive: %s\n", key);
    }
    if (bias < 0)
        puts("\tUnexpected EOF, no ##bias");
    if (BaseVar == NULL)
        puts("\tUnexpected EOF, no ##base");
    if (end == 0)
        puts("\tUnexpected EOF, no ##end directive");
}

/*
 *  funcname(var,var,var)(reg,reg,reg)      (or reg/reg)
 */

void
GenerateFunction(buf, bias)
char *buf;
long bias;
{
    FDNode *fd = malloc(sizeof(FDNode));
    char *scanPtr = buf;

    clrmem(fd, sizeof(FDNode));

    fd->fn_Offset = bias;

    {
        short noArgs = 0;

        while (*scanPtr && *scanPtr != '\t' && *scanPtr != ' ' && *scanPtr != '(')
            ++scanPtr;
        if (*scanPtr == ' ' || *scanPtr == '\t') {
            while (*scanPtr && *scanPtr != '(')
                *scanPtr++ = 0;
        }
        if (*scanPtr == '(') {
            *scanPtr++ = 0;
            if (*scanPtr == ')')
                noArgs = 1;
        }
        while (*scanPtr && *scanPtr != ')')   /*  skip text args  */
            ++scanPtr;
        while (*scanPtr && *scanPtr != '(')
            ++scanPtr;
        if (noArgs == 0 && *scanPtr == 0) {
            printf("\tError in line: %s\n", buf);
            return;
        }
    }
    fd->fn_Node.ln_Name = strdup(buf);

    /*
     *  get register description
     */

    if (*scanPtr)
        ++scanPtr;

    for (fd->fn_Args = 0; *scanPtr && *scanPtr != '\n' && *scanPtr != ')'; ++fd->fn_Args) {
        switch(*scanPtr) {
        case 'd':
        case 'D':
            fd->fn_Regs[fd->fn_Args] = *++scanPtr - '0';
            ++scanPtr;
            break;
        case 'a':
        case 'A':
            fd->fn_Regs[fd->fn_Args] = *++scanPtr - '0' + 8;
            ++scanPtr;
            break;
        default:
            printf("\tError in register spec: %s\n", scanPtr);
            return;
        }
        if (*scanPtr == ',' || *scanPtr == '/')
            ++scanPtr;
    }
    if (fd->fn_Args == 0 && *scanPtr != ')') {
        printf("\tError in register spec: %s\n", scanPtr);
        return;
    }
    AddTail(&FDList, &fd->fn_Node);
}


char *
ParseArg(ptr, ppt)
char *ptr;
char **ppt;
{
    char *base = ptr;

    if (ptr) {
        while (*ptr && *ptr != '(')
            ++ptr;
        if (*ptr == '(') {
            for (base = ++ptr; *ptr && *ptr != ')'; ++ptr)
                ;
            if (*ptr == ')') {
                *ptr++ = 0;
                *ppt = strdup(base);
            } else {
                ptr = NULL;
            }
        } else {
            ptr = NULL;
        }
    }
    return(ptr);
}

void
GenerateOutput(fo, outFile)
FILE *fo;
char *outFile;
{
    FDNode *fd;

    /* ** $ VER: proto/diskfont.h 1.0 (17.4.93) **        */
    /* #ifndef DISKFONT_PROTO_H                          */
    /* #define DISKFONT_PROTO_H 1                        */
    /* #include <pragmas/config.h>                       */
    /* #include <exec/types.h>                           */
    /* #include <clib/diskfont_protos.h>                 */
    /* #ifdef __SUPPORTS_PRAGMAS__                       */
    /* extern struct Library *DiskfontBase;              */
    /* #include <pragmas/diskfont_pragmas.h>             */
    /* #endif                                            */
    /* #endif                                            */

    {
        time_t t;
        struct tm *tp;

        time(&t);
        tp = localtime(&t);

        fprintf(fo, "/* %cVER: %s 1.0 (%d.%d.%d) */\n",
                   '$',outFile ? outFile : "",
                   tp->tm_mday, tp->tm_mon, tp->tm_year);
    }

    fprintf(fo, "#ifndef %s_PRAGMA_H\n", BaseVarPtr);
    fprintf(fo, "#define %s_PRAGMA_H\n\n", BaseVarPtr);

    /*
     *  #define's to generate library calls
     */

    for (fd = GetHead(&FDList); fd; fd = GetSucc(&fd->fn_Node))
    {
        short i;

        fprintf(fo, "#pragma libcall %s %s %lx ", BaseVarPtr,
                    fd->fn_Node.ln_Name, fd->fn_Offset);

        for (i = fd->fn_Args - 1; i >= 0; --i)
            fprintf(fo, "%x", fd->fn_Regs[i]);
        fprintf(fo, "0%x\n", fd->fn_Args & 15);
    }
    fprintf(fo, "\n#endif\n");
}

