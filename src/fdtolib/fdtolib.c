/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */
/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  FDTOLIB.C
 *
 *  FDTOLIB fdfile[s] [-h hdrfile] -o libname [-mr] [-mD] [-pprefix] -I incldir
 *                      -prof -mc -mC -auto libname -AUTO libname
 *
 *  Generates a normal model or registerized modem library interface given
 *  any set of FD files.
 *
 *  -ms, -r, and -pr are ignored
 */

#ifdef AMIGA
#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/alib_protos.h>
#include <lib/profile.h>
#include <lib/version.h>
#else
#include <include/lib/profile.h>
#include <include/lib/version.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef AMIGA
#include <suplib/all.h>
#endif

#ifndef L_tmpnam
#define L_tmpnam    64
#endif

#define RF_SCRATCH  0x0303
#define RB_BP       (8+6)
#define RF_BP       (1 << RB_BP)

IDENT("fdtolib", ".5");
DCOPYRIGHT;

typedef unsigned short uword;
typedef struct List     List;
typedef struct Node     Node;

typedef struct LVONode {
    Node    lv_Node;
    short   lv_Offset;
} LVONode;

typedef struct RSNode {
    Node    rn_Node;
    short   rn_Args;        /*  -1 specifies stack  */
    short   rn_Flag;        /*  equiv/fd found      */
    short   rn_Regs[32];    /*  transfer registers  */
} RSNode;

void    help(int);
void    ScanFD(FILE *, FILE *);
void    ScanRSTmp(FILE *);
void    GenerateFunction(FILE *, char *, char *, int);
int     AssembleFile(char *, char *);
void    JoinOutput(FILE *, char *);
char    *RegMaskToStr(uword, short *);
uword   RegsToMask(short *, short);
char    *RegToStr(short);
void    RegsCall(FILE *, char *, int32_t, RSNode *, short *);
void    StackCall(FILE *, char *, int32_t, short *, short);
short   PushMask(FILE *, uword);
short   PopMask(FILE *, uword);
void    AddLVOList(char *, int);
void    GenerateLVOList(FILE *);
void    GenerateAutoOpen(FILE *, char *);
void    DicePrefix(char *, char *, char *);

List    FDList;         /*  list of FD files   */
List    RSList;         /*  register spec list */
List    LVOList;
char    DccOptsBuf[1024];
char    *OutFile;
char    *HdrFile;
char    *FuncPrefix = "_";
char    Buf[256];
char    FuncName[256];
char    Prefix[64];
short   RegOpt;
short   Verbose;
short   Symbols = 1;
short   SmallData = 1;
short   SmallCode = 1;
short   ProfOpt;
short   AutoOpt;
char    *DataModel = "(A4)";
char    *CodeModel = "(PC)";
char    *SharedLibName;

int
main(int ac, char **av)
{
    int i;

    NewList(&FDList);
    NewList(&RSList);
    NewList(&LVOList);

    DicePrefix(Prefix, av[0], "");

#ifndef LATTICE
#ifndef unix
    expand_args(ac, av, &ac, &av);
#endif
#else
    setbuf(stdout, NULL);
#endif

    for (i = 1; i < ac; ++i) {
        char *ptr = av[i];

        if (*ptr != '-') {
            Node *node = malloc(sizeof(Node));
            node->ln_Name = ptr;
            AddTail(&FDList, node);
            continue;
        }
        ptr += 2;
        switch(ptr[-1]) {
        case 'a':
            AutoOpt = 1;
            SharedLibName = av[++i];
            break;
        case 'A':
            AutoOpt = 2;
            SharedLibName = av[++i];
            break;
        case 'p':
            if (strcmp(ptr, "rof") == 0)
                ProfOpt = 1;
            else if (strcmp(ptr, "r") == 0)     /*  ignore -pr  */
                ;
            else
                FuncPrefix = (*ptr) ? ptr : av[++i];
            break;
        case 'o':
            OutFile = (*ptr) ? ptr : av[++i];
            break;
        case 'h':
            HdrFile = (*ptr) ? ptr : av[++i];
            break;
        case 'm':
            if (*ptr == 'r' || *ptr == 'R')
                RegOpt = 1;
            else if (*ptr == 'd') {
                SmallData = 1;
                DataModel = "(A4)";
            } else if (*ptr == 'D') {
                SmallData = 0;
                DataModel = "";
            } else if (*ptr == 'c') {
                SmallCode = 1;
                CodeModel = "(pc)";
            } else if (*ptr == 'C') {
                SmallCode = 0;
                CodeModel = "";
            } else if (*ptr == 's') {
                /*  ignore  */
            }
            break;
        case 'r':
            /* ignore -r    */
            break;
        case 'v':
            ++Verbose;
            break;
        case 'I':
            sprintf(DccOptsBuf + strlen(DccOptsBuf), " -I%s", (*ptr) ? ptr : av[++i]);
            break;

        case 'n':
            Symbols = 0;
            break;

        default:
            help(1);
        }
    }
    if (OutFile == NULL || (RegOpt && HdrFile == NULL))
        help(ac != 1);

    if (RegOpt == 0 && HdrFile)
        puts("Warning: header file ignored (used only with -mr)");

    /*
     *  step 2, ask DCC to generate a register specification file
     */

    if (RegOpt) {
        FILE *fi;
        char rs_tmp[L_tmpnam];

        sprintf(Buf, "%sdcc -mRRX %s -a -o %s%s",
            Prefix,
            HdrFile,
            tmpnam(rs_tmp),
            DccOptsBuf
        );
        if(Symbols)strcat(Buf," -s -sym");
        puts(Buf);
#ifdef unix
        system(Buf);
#else
        Execute(Buf, NULL, NULL);
#endif

        if ((fi = fopen(rs_tmp, "r")) == NULL) {
            puts("Unable to generate register specification file");
            help(20);
        }
        ScanRSTmp(fi);
        fclose(fi);
        remove(rs_tmp);
    }

    /*
     *  step 3
     */


    {
        FILE *fi;
        FILE *fo = fopen(OutFile, "w");
        Node *node;

        if (fo == NULL) {
            printf("Error, Unable to create %s\n", OutFile);
            exit(20);
        }
        while ((node = RemHead(&FDList)) != NULL) {
            printf("generate %s", node->ln_Name);
            fi = fopen(node->ln_Name, "r");
            if (fi) {
                puts("");
                ScanFD(fi, fo);
                fclose(fi);
            } else {
                puts(" (open failed)");
            }
        }
    }
    {
#ifdef NOTDEF
        RSNode *rs;

        for (rs = RSList.lh_Head; rs->rn_Node.ln_Succ; rs = (RSNode *)rs->rn_Node.ln_Succ) {
            if (rs->rn_Flag == 0)
                printf("Warning, no FD entry found for: %s\n", rs->rn_Node.ln_Name);
        }
#endif
    }
    return(0);
}

void
help(int code)
{
    puts(Ident);
    puts(DCopyright);
    puts("FDTOLIB files/wildcard [-h hdrfile] -o libname [-mr] [-mD]");
    puts("  -mr :   generate registered library from header & fd file");
    puts("  -mD :   large data model (else small data model)");
    exit(code);
}

void
ScanFD(fi, fo)
FILE *fi;
FILE *fo;
{
    char *base = NULL;
    int32_t bias = -1;
    short end = 0;
    short public = 1;

    char *key;

    while (fgets(Buf, sizeof(Buf), fi)) {
        if (Buf[0] == '\n' || Buf[0] == '*')
            continue;
        if (strncmp(Buf, "##", 2) != 0) {
            if (bias < 0 || base == NULL) {
                printf("Error, No ##base/##bias before function: %s\n", Buf);
                continue;
            }
            if (public && AutoOpt != 2)
                GenerateFunction(fo, Buf, base, bias);
            bias += 6;
            continue;
        }
        if ((key = strtok(Buf + 2, " \t\n")) == NULL) {
            printf("\tError, Illegal null directive\n");
            continue;
        }
        if (stricmp(key, "base") == 0) {
            if ((key = strtok(NULL, " \t\n")) != NULL) {
                if (base)
                    free(base);
                base = strdup(key);
            } else {
                printf("\tError, Illegal ##base directive\n");
            }
            continue;
        }
        if (stricmp(key, "bias") == 0) {
            if ((key = strtok(NULL, " \t\n")) != NULL) {
                char *dummy;

                bias = strtol(key, &dummy, 0);
                if (bias <= 0)
                    printf("\tError, Illegal ##bias: %ld\n", bias);
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
    if (base == NULL)
        puts("\tUnexpected EOF, no ##base");
    if (end == 0)
        puts("\tUnexpected EOF, no ##end directive");
    if (AutoOpt != 2)
        GenerateLVOList(fo);
    if (AutoOpt)
        GenerateAutoOpen(fo, base);
}

/*
 *  funcname(var,var,var)(reg,reg,reg)      (or reg/reg)
 */

static short    FRegs[128];

void
GenerateFunction(fo, buf, base, bias)
FILE *fo;
char *buf;
char *base;
int bias;
{
    char tmpFile[L_tmpnam];
    char objFile[L_tmpnam+4];
    char *funcName;
    FILE *ft;
    short argCnt;
    short noArgs = 0;

    funcName = buf;
    while (*funcName && *funcName != '\t' && *funcName != ' ' && *funcName != '(')
        ++funcName;
    if (*funcName == ' ' || *funcName == '\t') {
        while (*funcName && *funcName != '(')
            *funcName++ = 0;
    }
    if (*funcName == '(') {
        *funcName++ = 0;
        if (*funcName == ')')
            noArgs = 1;
    }
    while (*funcName && *funcName != ')')   /*  skip text args  */
        ++funcName;
    while (*funcName && *funcName != '(')
        ++funcName;
    if (noArgs == 0 && *funcName == 0) {
        printf("\tError in line: %s\n", buf);
        return;
    }

    /*
     *  get register description
     */

    if (*funcName)
        ++funcName;
    for (argCnt = 0; *funcName && *funcName != '\n' && *funcName != ')'; ++argCnt) {
        switch(*funcName) {
        case 'd':
        case 'D':
            FRegs[argCnt] = *++funcName - '0';
            ++funcName;
            break;
        case 'a':
        case 'A':
            FRegs[argCnt] = *++funcName - '0' + 8;
            ++funcName;
            break;
        default:
            printf("\tError in register spec: %s\n", funcName);
            return;
        }
        if (*funcName == ',' || *funcName == '/')
            ++funcName;
    }
    if (noArgs == 0 && *funcName != ')') {
        printf("\tError in register spec: %s\n", funcName);
        return;
    }

    /*
     *  generate
     */

    funcName = strdup(buf);
    {
        RSNode *rs = NULL;

        if (Verbose)
            printf("    %-15s %d %d ", funcName, -bias, argCnt);

        if (RegOpt) {
            for (rs = (RSNode *)RSList.lh_Head; rs->rn_Node.ln_Succ; rs = (RSNode *)rs->rn_Node.ln_Succ) {
                if (strcmp(rs->rn_Node.ln_Name + 1, funcName) == 0) {
                    rs->rn_Flag = 1;
                    break;
                }
            }
            if (rs->rn_Node.ln_Succ == NULL) {  /* (list tail)  */
                if (Verbose)
                    puts("NO MATCH FOUND");
                return;
            }

            if (rs->rn_Args >= 0 && rs->rn_Args != argCnt) {
                printf("Error, argCnt mismatch %s (%d/%d)\n", rs->rn_Node.ln_Name, rs->rn_Args, argCnt);
                return;
            }
        }
        if (Verbose)
            fflush(stdout);

        tmpnam(tmpFile);
        sprintf(objFile, "%s.o", tmpFile);

        if (RegOpt)
            sprintf(FuncName, "%s", rs->rn_Node.ln_Name);
        else
            sprintf(FuncName, "%s%s", FuncPrefix, funcName);

        if ((ft = fopen(tmpFile, "w")) != NULL) {
            if (ProfOpt) {
                fprintf(ft, "\txref\t__ProfInit\n");
                fprintf(ft, "\txref\t__ProfExec\n");

                fprintf(ft, "\n\tsection autoinit1,code\n");
                fprintf(ft, "\tlea\tlp0%s,A0\n", DataModel);
                fprintf(ft, "\tjsr\t__ProfInit\n");
                fprintf(ft, "\n\tsection libdata,data\n");
                fprintf(ft, "\tds.l\t0\n");
                fprintf(ft, "lp0\n");
                fprintf(ft, "\tdc.l\t0\n");
                fprintf(ft, "\tdc.l\t0\n");
                fprintf(ft, "\tdc.l\t0\n");
                fprintf(ft, "\tdc.w\t%d\n", (40 + strlen(FuncName) + (1 + 3)) & ~3);
                fprintf(ft, "\tdc.w\t0\n");
                fprintf(ft, "\tdc.l\t0\n");
                fprintf(ft, "\tdc.l\t0\n");
                fprintf(ft, "\tdc.l\t0\n");
                fprintf(ft, "\tdc.l\t0\n");
                fprintf(ft, "\tdc.l\tlp1\n");
                fprintf(ft, "\tdc.l\tlp2\n");
                fprintf(ft, "\tdc.b\t\'%s\',0\n", FuncName);

                fprintf(ft, "\tds.l\t0\n");
            }
            fprintf(ft, "\n\tsection ,code\n\n");
            fprintf(ft, "\txref\t%s\n\n", base);

            fprintf(ft, "\txdef\t%s\n", FuncName);
            fprintf(ft, "%s:\n", FuncName);

            AddLVOList(funcName, -bias);

            if (ProfOpt) {
                fprintf(ft, "\tjsr\t__ProfExec%s\n", CodeModel);
                fprintf(ft, "lp1\n");
            }

            if (RegOpt == 0 || rs->rn_Args == -1) {     /*  STACK CALL */
                StackCall(ft, base, bias, FRegs, argCnt);
            } else {                                    /*  REG CALL   */
                RegsCall(ft, base, bias, rs, FRegs);
            }
            fputs("\tEND\n", ft);
            fclose(ft);

            /*
             *  assemble the temp file
             */

            if (Verbose)
                puts("");
            if (AssembleFile(tmpFile, objFile))
                JoinOutput(fo, objFile);
            remove(objFile);
            remove(tmpFile);
        } else {
            printf("\tError, Unable to create file: %s\n", tmpFile);
        }
    }
}

void
ScanRSTmp(fi)
FILE *fi;
{
    RSNode *rs;
    char *ptr;
    char *symPtr;

    while (fgets(Buf, sizeof(Buf), fi)) {
        if (strnicmp(Buf, "##regspec", 9) != 0)
            continue;

        for (ptr = Buf + 9; *ptr == ' ' || *ptr == '\t'; ++ptr);
        symPtr = ptr;
        while (*ptr && *ptr != '(')
            ++ptr;
        if (*ptr != '(') {
            printf("Error scanning RS file: %s\n", Buf);
            continue;
        }
        *ptr++ = 0;
        rs = malloc(sizeof(RSNode) + strlen(symPtr) + 1);
        rs->rn_Node.ln_Name = (char *)(rs + 1);
        rs->rn_Flag = 0;
        strcpy(rs->rn_Node.ln_Name, symPtr);

        if (*ptr == '*') {
            rs->rn_Args = -1;
            while (*ptr && *ptr != ')')
                ++ptr;
        } else {
            rs->rn_Args = 0;
            while (*ptr && *ptr != ')') {
                switch(*ptr) {
                case 'd':
                case 'D':
                    rs->rn_Regs[rs->rn_Args++] = ptr[1] - '0';
                    ptr += 2;
                    break;
                case 'a':
                case 'A':
                    rs->rn_Regs[rs->rn_Args++] = ptr[1] - '0' + 8;
                    ptr += 2;
                    break;
                default:
                    printf("Error, Illegal RS file register spec: %s\n", ptr);
                    ptr = "";
                    break;
                }
                if (*ptr == ',')
                    ++ptr;
            }
        }
        if (*ptr != ')') {
            printf("Error scanning RS file: %s\n", symPtr + strlen(symPtr) + 1);
            continue;
        }
        if (Verbose > 1)
            printf("RS-SCAN: %s (%d)\n", rs->rn_Node.ln_Name, rs->rn_Args);
        AddTail((struct List *)&RSList, &rs->rn_Node);
    }
}

int
AssembleFile(inFile, outFile)
char *inFile;
char *outFile;
{
    remove(outFile);

    if (Verbose > 2) {
        FILE *fi;

        if ((fi = fopen(inFile, "r")) != NULL) {
            while (fgets(Buf, sizeof(Buf), fi))
                fputs(Buf, stdout);
            fclose(fi);
        }
    }
/*    sprintf(Buf, "%sdas %s -o %s -nu -sym", */
    sprintf(Buf, "%sdas %s -o %s",
        Prefix,
        inFile,
        outFile
     );
     if (Verbose > 0)
         puts(Buf);
     if(Symbols)strcat(Buf," -s");
#ifdef unix
    system(Buf);
#else
    Execute(Buf, NULL, NULL);
#endif
    return(1);
}

void
JoinOutput(fo, file)
FILE *fo;
char *file;
{
    FILE *fi;
    short c;

    if ((fi = fopen(file, "r")) != NULL) {
        while ((c = getc(fi)) != EOF)
            putc(c, fo);
        fclose(fi);
    } else {
        printf("Error, Can't read %s\n", file);
    }
}

/*
 *  Generate assembly for a stack based call
 */

void
StackCall(FILE *ft, char *base, int32_t bias, short *regs, short args)
{
    uword mask = (RegsToMask(regs, args) | RF_BP) & ~RF_SCRATCH;
    char *ptr;
    short i;
    short j;
    short n;

    /*
     *  step 1, what regs need to be saved?
     */

    n = PushMask(ft, mask);

    /*
     *  step 2, load regs from stack
     */

    for (i = j = 0; i < args; i = j) {
        uword lmask = 1 << regs[i];
        short ln;
        int offset = n * 4 + i * 4 + 4;

        for (j = i + 1; j < args; ++j) {
            if (regs[j] < regs[j-1])
                break;
            lmask |= 1 << regs[j];
        }
        ptr = RegMaskToStr(lmask, &ln);
        if (ln > 1)
            fprintf(ft, "\tmovem.l\t%d(sp),%s\n", offset, ptr);
        else if (ln > 0)
            fprintf(ft, "\tmove.l\t%d(sp),%s\n", offset, ptr);
    }
    /*
     *  step 3, load library base register
     */

    fprintf(ft, "\tmove.l\t%s%s,A%d\n", base, DataModel, RB_BP - 8);

    /*
     *  step 4, make call & return
     */

    if (n) {
        fprintf(ft, "\tjsr\t-%ld(A%d)\n", bias, RB_BP - 8);
        PopMask(ft, mask);
        if (ProfOpt) {
            fprintf(ft, "\tjsr\t__ProfExec%s\n", CodeModel);
            fprintf(ft, "lp2\n");
        }
        fprintf(ft, "\tRTS\n");
    } else {
        if (ProfOpt) {
            fprintf(ft, "\tjsr\t-%ld(A%d)\n", bias, RB_BP - 8);
            fprintf(ft, "\tjsr\t__ProfExec%s\n", CodeModel);
            fprintf(ft, "lp2\n");
            fprintf(ft, "\tRTS\n");
        } else {
            fprintf(ft, "\tjmp\t-%ld(A%d)\n", bias, RB_BP - 8);
        }
    }
}

/*
 *  Generate assembly for a register based call
 */

void
RegsCall(ft, base, bias, rs, regs)
FILE *ft;
char *base;
int32_t bias;
RSNode *rs;
short *regs;
{
    uword mask = (RegsToMask(regs, rs->rn_Args) | RF_BP) & ~RF_SCRATCH;
    short i;
    short j;
    short n;

    /*
     *  step 1, what regs need to be saved?
     */

    n = PushMask(ft, mask);

    /*
     *  step 2, load regs from other regs.  If destination is 'in use',
     *  then use EXG instead (and track where the reg went to)
     *
     *  src:    rs->rn_Regs[i]
     *  dest:   regs[i]
     */

    for (i = 0; i < rs->rn_Args; ++i) {
        for (j = 0; j < rs->rn_Args; ++j) { /*  is dest in use? */
            if (i != j && regs[i] == rs->rn_Regs[j])
                break;
        }
        if (j == rs->rn_Args) {                 /*  not in use          */
            if (regs[i] != rs->rn_Regs[i]) {    /*  not in right plac   */
                fprintf(ft, "\tmove.l\t%s,%s\n", RegToStr(rs->rn_Regs[i]), RegToStr(regs[i]));
                rs->rn_Regs[i] = -1;
            }
        } else {
            fprintf(ft, "\texg.l\t%s,%s\n", RegToStr(rs->rn_Regs[i]), RegToStr(regs[i]));
            rs->rn_Regs[j] = rs->rn_Regs[i];
            rs->rn_Regs[i] = -1;
        }
    }

    /*
     *  step 3, load library base register
     */

    fprintf(ft, "\tmove.l\t%s%s,A%d\n", base, DataModel, RB_BP - 8);

    /*
     *  step 4, make call and return
     */

    if (n) {
        fprintf(ft, "\tjsr\t-%ld(A%d)\n", bias, RB_BP - 8);
        PopMask(ft, mask);

        if (ProfOpt) {
            fprintf(ft, "\tjsr\t__ProfExec%s\n", CodeModel);
            fprintf(ft, "lp2\n");
        }
        fprintf(ft, "\tRTS\n");
    } else {
        if (ProfOpt) {
            fprintf(ft, "\tjsr\t-%ld(A%d)\n", bias, RB_BP - 8);
            fprintf(ft, "\tjsr\t__ProfExec%s\n", CodeModel);
            fprintf(ft, "lp2\n");
            fprintf(ft, "\tRTS\n");
        } else {
            fprintf(ft, "\tjmp\t-%ld(A%d)\n", bias, RB_BP - 8);
        }
    }
}

char *
RegMaskToStr(uword mask, short *nr)
{
    static char buf[64];
    char *ptr = buf;
    short i;
    short l = -1;

    *nr = 0;

    for (i = 0; i < 8; ++i) {
        if (mask & (1 << i)) {
            ++*nr;
            if (l >= 0)
                *ptr++ = '/';
            ptr += sprintf(ptr, "D%d", i);
            l = i;
        }
    }
    for (i = 8; i < 16; ++i) {
        if (mask & (1 << i)) {
            ++*nr;
            if (l >= 0)
                *ptr++ = '/';
            ptr += sprintf(ptr, "A%d", i - 8);
            l = i;
        }
    }
    *ptr = 0;
    return(buf);
}

uword
RegsToMask(short *regs, short args)
{
    uword mask = 0;

    while (args > 0) {
        mask |= 1 << *regs++;
        --args;
    }
    return(mask);
}

char *
RegToStr(short rno)
{
    static char Buf[2][16];
    static short BNo;
    char *ptr = Buf[BNo];

    BNo = 1 - BNo;
    if (rno < 8)
        sprintf(ptr, "D%d", rno);
    else
        sprintf(ptr, "A%d", rno - 8);
    return(ptr);
}

short
PushMask(FILE *fo, uword mask)
{
    short n;
    char *ptr;

    ptr = RegMaskToStr(mask, &n);
    if (n > 1)
        fprintf(fo, "\tmovem.l\t%s,-(sp)\n", ptr);
    else if (n > 0)
        fprintf(fo, "\tmove.l\t%s,-(sp)\n", ptr);
    return(n);
}

short
PopMask(FILE *fo, uword mask)
{
    short n;
    char *ptr;

    ptr = RegMaskToStr(mask, &n);
    if (n > 1)
        fprintf(fo, "\tmovem.l\t(sp)+,%s\n", ptr);
    else if (n > 0)
        fprintf(fo, "\tmove.l\t(sp)+,%s\n", ptr);
    return(n);
}

void
AddLVOList(funcName, bias)
char *funcName;
int bias;
{
    LVONode *node;

    if ((node = malloc(sizeof(LVONode) + strlen(funcName) + 1)) != NULL) {
        AddTail(&LVOList, &node->lv_Node);
        node->lv_Node.ln_Name = (char *)(node + 1);
        strcpy(node->lv_Node.ln_Name, funcName);
        node->lv_Offset = bias;
    }
}

void
GenerateLVOList(fo)
FILE *fo;
{
    char tmpFile[L_tmpnam];
    char objFile[L_tmpnam + 4];
    LVONode *node;
    FILE *ft;

    tmpnam(tmpFile);
    sprintf(objFile, "%s.o", tmpFile);

    if ((node = (LVONode *)RemHead((struct List *)&LVOList)) != NULL) {
        if ((ft = fopen(tmpFile, "w")) != NULL) {
            fprintf(ft, "\n\tsection ,code\n\n");

            while (node) {
                if (Verbose)
                    printf("_LVO%-20s = %d\n", node->lv_Node.ln_Name, node->lv_Offset);
                fprintf(ft, "_LVO%s\tEQU\t%d\n", node->lv_Node.ln_Name, node->lv_Offset);
                fprintf(ft, "\txdef\t_LVO%s\n", node->lv_Node.ln_Name);
                free(node);
                node = (LVONode *)RemHead((struct List *)&LVOList);
            }
            fputs("\tEND\n", ft);
            fclose(ft);

            if (AssembleFile(tmpFile, objFile))
                JoinOutput(fo, objFile);
            remove(objFile);
            remove(tmpFile);
        }
    }
}

/*
 *  generate auto-open tag code for this library
 */

void
GenerateAutoOpen(fo, base)
FILE *fo;
char *base;
{
    char tmpFile[L_tmpnam];
    char objFile[L_tmpnam + 4];
    FILE *ft;

    tmpnam(tmpFile);
    sprintf(objFile, "%s.o", tmpFile);

    if ((ft = fopen(tmpFile, "w")) != NULL) {
        fprintf(ft, "\n\txdef\t%s\n", base);
        fprintf(ft, "\n\txref\t__AutoFail0\n\n");
        fprintf(ft, "_LVOOpenLibrary\tequ\t-552\n");
        fprintf(ft, "_LVOCloseLibrary\tequ\t-414\n\n");

        fprintf(ft, "\n\tsection autoinit0,code\n\n");

        fprintf(ft, "\tmoveq.l\t#0,D0\n");
        fprintf(ft, "\tlea\tlibname(pc),A1\n");
        fprintf(ft, "\tjsr\t_LVOOpenLibrary(A6)\n");
        fprintf(ft, "\tmove.l\tD0,%s%s\n", base, DataModel);
        fprintf(ft, "\tbeq\t__AutoFail0\n");
        fprintf(ft, "\tbra\topennext\n");
        fprintf(ft, "libname\tdc.b\t'%s',0\n", SharedLibName);
        fprintf(ft, "\tds.w\t0\n");
        fprintf(ft, "opennext\n\n");

        fprintf(ft, "\tsection autoexit0,code\n\n");
        fprintf(ft, "\tmove.l\t%s%s,D0\n", base, DataModel);
        fprintf(ft, "\tbeq\tclosenext\n");
        fprintf(ft, "\tmove.l\tD0,A1\n");
        fprintf(ft, "\tjsr\t_LVOCloseLibrary(A6)\n");
        fprintf(ft, "closenext\n\n");

        fprintf(ft, "\tsection libdata,data\n\n");
        fprintf(ft, "%s\tdc.l\t0\n", base);

        fputs("\tEND\n", ft);
        fclose(ft);

        if (AssembleFile(tmpFile, objFile))
            JoinOutput(fo, objFile);
        remove(objFile);
        remove(tmpFile);
    }
}


#ifdef LATTICE

char *
tmpnam(buf)
char *buf;
{
    static char Buf[L_tmpnam];
    static short Seq;

    if (buf == NULL)
        buf = Buf;
    sprintf(buf, "T:%06lx%d", (int32_t)FindTask(NULL) >> 4, Seq++);
    return(buf);
}

#endif

void
DicePrefix(char *buf, char *av0, char *app)
{
    char *ptr;
    short n = 0;

    for (ptr=av0+strlen(av0); ptr >= av0 && *ptr != '/' && *ptr != ':'; --ptr)
        ;
    ++ptr;
    if ((av0 = strchr(ptr, '_')) != NULL) {
        n = av0 - ptr + 1;
        strncpy(buf, ptr, n);
    }
    strcpy(buf + n, app);
}

