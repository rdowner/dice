
/*
 *      FILE.C  - push/pop files, include directive support.
 *
 *      $Filename: main.c $
 *      $Author: dillon $
 *      $Revision: 1.1.1.1 $
 *      $Date: 1998/10/10 06:23:22 $
 *      $Log: main.c,v $
 *      Revision 1.1.1.1  1998/10/10 06:23:22  dillon
 */

#include "defs.h"
#include "DAS_rev.h"

Prototype FileNode *OpenFile(const char *fileName);
Prototype void AddInclude(const char *fileName);
Prototype void PushFileNode(FileNode *fn);
Prototype void PopFileNode(void);
Prototype char *ParseFileName(const char *str);

void Filter(FileNode *fn);
FILE *FOpenInclude(const char *fileName, const char *modes);

FileNode *
OpenFile(const char *fileName)
{
    FileNode *fn = zalloc(sizeof(FileNode) + strlen(fileName) + 1);

    fn->fn_FileName = (char *)(fn + 1);
    strcpy(fn->fn_FileName, fileName);

    {
        FILE *fi = FOpenInclude(fileName, "r");
        if (fi == NULL)
            cerror(EFATAL_CANT_OPEN_FILE, fileName);
        fseek(fi, 0L, 2);
        fn->fn_AsLen = ftell(fi);
        if (fn->fn_AsLen <= 0)
            cerror(EFATAL_CANT_SEEK_INPUT);
        fn->fn_AsBuf = malloc(fn->fn_AsLen + 2);
        if (fn->fn_AsBuf == NULL)
            NoMemory();
        fseek(fi, 0L, 0);
        if (fread(fn->fn_AsBuf, 1, fn->fn_AsLen, fi) != fn->fn_AsLen)
            cerror(EFATAL_READ_ERROR);
        fn->fn_AsBuf[fn->fn_AsLen++] = '\n';  /* must end in newline */
        fn->fn_AsBuf[fn->fn_AsLen] = 0;
        fclose(fi);
    }
    /*
     *  Allocate a machine context structure for each line
     */
    {
        int32_t i;
        int32_t lines = 0;

        for (i = 0; fn->fn_AsBuf[i]; ++i) {
            if (fn->fn_AsBuf[i] == '\n')
                ++lines;
        }
        fn->fn_MBase = malloc((lines + 3) * sizeof(MachCtx *));
        if (fn->fn_MBase == NULL)
            NoMemory();
    }

    Filter(fn);
    return(fn);
}

void
PushFileNode(FileNode *fn)
{
    fn->fn_Parent = CurFile;
    CurFile = fn;
}

void
PopFileNode(void)
{
    if (CurFile == NULL) {
        cerror(EFATAL_INTERNAL_ERROR, "");
    }
    CurFile = CurFile->fn_Parent;
}

void
Filter(FileNode *fn)
{
    char *ptr = fn->fn_AsBuf;

    while (*ptr) {
        if (*ptr == '\"') {
            for (++ptr; *ptr != '\n' && *ptr != '\"'; ++ptr)
                ;   
            ++ptr;
        } else if (*ptr == '\'') {
            for (++ptr; *ptr != '\n' && *ptr != '\''; ++ptr)
                ;
            ++ptr;
        } else if (*ptr == ';') {
            while (*ptr != '\n')
                *ptr++ = ' ';
            ++ptr;
        } else {
            ++ptr;
        }
    }   
}

char *
ParseFileName(const char *str)
{
    int s;
    char *r = NULL;

    for (s = 0; str[s]; ++s) {
        if (str[s] == '\"') {
            ++s;
            break;
        }
    }
    if (str[s]) {
        int e;
        for (e = s; str[e] != '\"'; ++e)
            ;
        if (str[e] == '\"') {
            r = malloc(e - s + 1);
            memcpy(r, str + s, e - s);
            r[e - s] = 0;
        }
    }
    if (r == NULL)
        cerror(EFATAL_SYNTAX, str);
    return(r);
}

IncNode *InBase;
IncNode **InTail = &InBase;

void 
AddInclude(const char *fileName)
{
    IncNode *in = zalloc(sizeof(IncNode) + strlen(fileName) + 1);
    in->in_Path = (char *)(in + 1);
    strcpy(in->in_Path, fileName);
    *InTail = in;
    InTail = &in->in_Next;
}

FILE *
FOpenInclude(const char *fileName, const char *modes)
{
    char path[1024];
    FILE *fi = NULL;
    IncNode *in;

    /*
     * Try include file relative to source file
     */

    if (CurFile) {
        char *ptr;

        strcpy(path, CurFile->fn_FileName);
        if ((ptr = strrchr(path, '/')) != NULL) {
            strcpy(ptr + 1, fileName);
            fi = fopen(path, modes);
        }
    }

    /*
     * Try include file straight
     */
    if (fi == NULL)
        fi = fopen(fileName, modes);

    for (in = InBase; in && fi == NULL; in = in->in_Next) {
        sprintf(path, "%s/%s", in->in_Path, fileName);
        fi = fopen(path, modes);
    }
    return(fi);
}

