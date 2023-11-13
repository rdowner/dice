/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  DAS     Minimal assembler and peephole optimizer for DCC
 *
 *          -doesn't understand include or IF constructions
 *          -only understands decimal and $ hex
 *          -only understands +, -, and negate
 *
 *  DAS infile [-o outfile] -O[optlevel]
 */

/*
**      $Filename: main.c $
**      $Author: dillon $
**      $Revision: 1.2 $
**      $Date: 1999/02/08 22:51:42 $
**      $Log: main.c,v $
**      Revision 1.2  1999/02/08 22:51:42  dillon
**
**
**          Add INCLUDE directive.  INCLUDE "filename".  Filename is relative to
**          source file doing the INCLUDE.
**
**          Add -I option to das to specify include directories.
**
**      Revision 1.1.1.1  1998/10/10 06:23:22  dillon
**
**
**          Initial import of DICE into CVS repository.  Logs for the thousands of
**          changes prior to this import have essentially been lost   -Matt.
**
 * Revision 30.326  1995/12/24  06:12:49  dice
 * .
 *
 * Revision 30.5  1994/06/13  18:41:10  dice
 * .
 *
 * Revision 30.0  1994/06/10  18:07:44  dice
 * .
 *
 * Revision 1.2  1993/09/19  20:55:01  jtoebes
 * Fixed BUG06033 - DAS does not report original C source line numbers.
 * Changed format of error message to take into account a C file present.
 * Also respect the error formatting codes so that it is parsed by the editor.
 *
 * Revision 1.1  1993/09/19  19:56:59  jtoebes
 * Initial revision
 *
**/

#include "defs.h"
#include "DAS_rev.h"

static char *DCopyright =
"Copyright (c) 1992,1993,1994,1999 Obvious Implementations Corp., Redistribution & Use under DICE-LICENSE.TXT." VERSTAG;


Prototype FileNode *CurFile;
Prototype short   AddSym;
Prototype short   DDebug;
Prototype short   Optimize;
Prototype short   NoUnitName;
Prototype short   Verbose;
Prototype short   ErrorOpt;
Prototype FILE    *ErrorFi;
Prototype FILE    *Fo;
Prototype char    *FoName;
Prototype int32_t         ExitCode;
Prototype char    *SrcFileName;
Prototype char    *AsmFileName;

Prototype int   main(int, char **);
Prototype void  DebugPass(char);
Prototype void  *zalloc(int32_t);
Prototype void  help(void);
Prototype short CToSize(char);

#ifndef DEBUG
#define DebugPass(x)
#endif

FileNode *CurFile;
short   AddSym;
short   DDebug;
short   NoUnitName;
short   Optimize;
short   Verbose;
short   ErrorOpt;
FILE    *ErrorFi;
FILE    *Fo;        /*  used on error exit to close file  */
char    *FoName;    /*  used on error exit to delete file */
char    *AsmFileName;
char    *SrcFileName;
int32_t ExitCode;

int
main(int ac, char **av)
{
    char *inFile = NULL;
    char *outFile = NULL;
    short i;

    if (ac == 1)
        help();

    for (i = 1; i < ac; ++i) {
        char *ptr = av[i];
        if (*ptr != '-') {
            inFile = ptr;
            AsmFileName = ptr;
            if (SrcFileName == NULL)
                SrcFileName = ptr;
            continue;
        }
        ptr += 2;
        switch(ptr[-1]) {
        case 'n':
            switch(*ptr) {
            case 'u':
                NoUnitName = 1;
                break;
            }
            break;
        case 'v':
            Verbose = 1;
            break;
        case 'd':
            DDebug = 1;
            break;
        case 's':
            AddSym = 1;
            break;
        case 'I':
            if (*ptr == 0)
                ptr = av[++i];
            AddInclude(ptr);
            break;
        case 'O':
            Optimize = atoi((*ptr) ? ptr : av[++i]);
            break;
        case 'F':
            ErrorOpt = 1;
            if (*ptr == 'F')
                ErrorOpt = 2;
            else if ((*ptr >= '0') && (*ptr <= '9'))
                ErrorOpt = *ptr - '0';

            ptr = av[++i];
            ErrorFi = fopen(ptr, "a");
            break;
        case 'N':
            if (*ptr == 0)
                ptr = av[++i];
            SrcFileName = ptr;
            break;
        case 'o':
            outFile = (*ptr) ? ptr : av[++i];
            break;
        default:
            help();
        }
    }
    if (inFile == NULL) {
        cerror(EERROR_NO_INPUT_FILE);
        help();
    }
    if (outFile == NULL) {
        i = strlen(inFile);
        outFile = malloc(i + 5);
        strcpy(outFile, inFile);
        for (--i; i >= 0 && outFile[i] != '.'; --i);
        if (outFile[i] == '.')
            strcpy(outFile + i + 1, "o");
        else
            strcat(outFile, ".o");
    }

    PushFileNode(OpenFile(inFile));

    InitAlNumAry();
    InitOps();
    InitSect();

    DebugPass('a');
    PassA();            /*      Loads MachCtx & resolves directives     */
    DebugPass('b');
    (void)PassB(0);         /*  resolves addressing modes (expressions) */
    DebugPass('x');
    ResetSectAddrs();
    DebugPass('c');
    (void)PassC(0);         /*  optimize!   */
    ResetSectAddrs();
    DebugPass('g');
    (void)PassG(0);        /*  generate code                   */
    DebugPass('s');
    {
        FILE *fo = fopen(outFile, "w");
        if (fo == NULL)
            cerror(EFATAL_CANT_CREATE_FILE, outFile);
        Fo = fo;
        FoName = outFile;
        SectCreateObject(fo, inFile);
        fclose(fo);
        Fo = NULL;
    }
    if (ExitCode > 5 && FoName)
        remove(FoName);
    return(ExitCode);
}

#ifdef DEBUG

void
DebugPass(c)
char c;
{
    dbprintf(0, ("PASS %c\n", c));
}

#endif

void *
zalloc(bytes)
int32_t bytes;
{
    static char *Buf;
    static int32_t Bytes;

    if (bytes <= Bytes) {
        void *ptr;

        ptr = (void *)Buf;
        Buf += bytes;
        Bytes -= bytes;
        return(ptr);
    }
    if (bytes > 128) {
        void *ptr;

        ptr = malloc(bytes);
        if (ptr == NULL) {
            NoMemory();
        }
        setmem(ptr, bytes, 0);
        return(ptr);
    }
    Buf = malloc(ZALLOCSIZ);
    if (Buf == NULL) {
        NoMemory();
    }
    Bytes = ZALLOCSIZ;

    setmem(Buf, Bytes, 0);

    Buf += bytes;
    Bytes -= bytes;
    return(Buf - bytes);
}

void
help()
{
    printf("%s\n%s\n", VSTRING, DCopyright);
    cerror(EVERB_INTRO1);
    cerror(EVERB_INTRO2);
    exit(5);
}

short
CToSize(char c)
{
    switch(c|0x20) {
    case 'b':
    case 's':
        return(1);
    case 'w':
        return(2);
    case 'l':
        return(4);
    }
    cerror(EERROR_ILLEGAL_SIZE, c);
    return(0);
}

