/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  LOADABS.C
 *
 *  loadabs exefile -o outfile -A addr
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef AMIGA
#include <lib/version.h>
#else
#include <include/lib/version.h>
#endif

#define HUNK_BSS        0x3EB
#define HUNK_CODE       0x3E9
#define HUNK_DATA       0x3EA
#define HUNK_RELOC32    0x3EC
#define HUNK_SYMBOLS    0x3F0
#define HUNK_END        0x3F2
#define HUNK_HEAD       0x3F3

IDENT("loadabs",".3");
DCOPYRIGHT;

int32_t NumHunks;
int32_t FirstHunk;
int32_t LastHunk;

short   DDebug;

typedef unsigned char ubyte;

typedef struct {
    int32_t    Type;
    int32_t    Len;
    int32_t    Pc;
    char    *Data;
} Hunk;

Hunk    *Hunks;

int32_t fgetl(FILE *);
void    LoadHeaderInfo(FILE *);
void    ScanHunks(FILE *, int32_t);
void    RelocHunks(FILE *);
void    DumpHunks(FILE *);

main(ac, av)
char *av[];
{
    short i;
    char *inFile = NULL;
    char *outFile= NULL;
    FILE *fi;
    FILE *fo;
    int32_t scanPc;

    if (ac == 1) {
        puts(Ident);
        puts(DCopyright);
        puts("LoadAbs exefile -o outfile -A begaddr -d[#]");
        exit(1);
    }

    for (i = 1; i < ac; ++i) {
        char *ptr = av[i];

        if (*ptr != '-') {
            if (inFile) {
                puts("input file specified twice!");
                exit(1);
            }
            inFile = ptr;
            continue;
        }
        ptr += 2;

        switch(ptr[-1]) {
        case 'o':
            if (outFile) {
                puts("output file specified twice!");
                exit(1);
            }
            outFile = (*ptr) ? ptr : av[++i];
            break;
        case 'A':
            scanPc = strtol((*ptr) ? ptr : av[++i], NULL, 0);
            break;
        case 'd':
            if (*ptr == 0)
                DDebug = 1;
            else
                DDebug = atoi(ptr);
            break;
        default:
            printf("Bad option: %s\n", av[i]);
            exit(1);
        }
    }
    if (i > ac) {
        puts("expected argument");
        exit(1);
    }
    if (inFile == NULL) {
        puts("no input executable specified");
        exit(1);
    }
    if (outFile == NULL) {
        puts("no output image file specified");
        exit(1);
    }
    if ((fi = fopen(inFile, "r")) == NULL) {
        puts("unable to open input file");
        exit(1);
    }
    if (fgetl(fi) != HUNK_HEAD) {
        puts("Not an executable");
        exit(1);
    }

    /*
     *  skip name
     */
    {
        int n = fgetl(fi);
        while (n--)
            fgetl(fi);
    }

    /*
     *  header info
     */

    NumHunks    = fgetl(fi);
    FirstHunk   = fgetl(fi);
    LastHunk    = fgetl(fi);

    Hunks = malloc(NumHunks * sizeof(Hunk));
    if (Hunks == NULL) {
        puts("malloc failed");
        exit(1);
    }
    LoadHeaderInfo(fi);
    {
        int32_t pos = ftell(fi);
        if (DDebug)
            puts("scan");

        ScanHunks(fi, scanPc);

        fseek(fi, pos, 0);
        if (DDebug)
            puts("reloc");

        RelocHunks(fi);
    }
    fclose(fi);

    if ((fo = fopen(outFile, "w")) == NULL) {
        puts("unable to create image file");
        exit(1);
    }
    if (DDebug)
        puts("dump");
    DumpHunks(fo);
    fclose(fo);
    return(0);
}

void
LoadHeaderInfo(fi)
FILE *fi;
{
    int n;
    Hunk *h;

    for (n = 0, h = Hunks; n < NumHunks; ++n, ++h) {
        h->Len = fgetl(fi) * 4;
        h->Data= malloc(h->Len + 1);    /*  +1 so can malloc 0 wo/error    */
        if (h->Data == NULL) {
            printf("malloc failed hunk %d (%d)\n", n, h->Len);
            exit(1);
        }
        clrmem(h->Data, h->Len);
    }
}

void
ScanHunks(fi, pc)
FILE *fi;
int32_t pc;
{
    int n;
    Hunk *h;

    n = 0;
    h = Hunks;

    while (n < NumHunks) {
        int32_t len;
        int32_t t = fgetl(fi);

        if (DDebug)
            printf("Header %08lx\n", t);

        switch(t) {
        case HUNK_BSS:
            h->Type = t;
            fgetl(fi);      /*  skip length */
            break;
        case HUNK_CODE:
        case HUNK_DATA:
            h->Type = t;
            len = fgetl(fi) * 4;
            if (len > h->Len || len < 0) {
                printf("Hunk Error: Text len %d/%d\n", len, h->Len);
                exit(1);
            }
            if (fread(h->Data, 1, len, fi) != len) {
                puts("Unexpected EOF");
                exit(1);
            }
            break;
        case HUNK_RELOC32:
            while (len = fgetl(fi) * 4) {
                if (DDebug)
                    printf("len = %d\n", len);
                fgetl(fi);      /*  skip hunk#  */
                fseek(fi, len, 1);
            }
            break;
        case HUNK_SYMBOLS:
            /*
             *  ignore
             */
            while (len = fgetl(fi)) {
                ubyte type = len >> 24;

                len &= 0x00FFFFFF;
                fseek(fi, len * 4, 1);

                switch(type) {
                case 0:         /*  SYMB    */
                case 1:         /*  DEF     */
                case 2:         /*  ABS     */
                case 3:         /*  RES     */
                    fgetl(fi);
                    break;
                case 130:       /*  COMMON  */
                    puts("COMMON symbol not supported");
                    fgetl(fi);  /*  skip common size */
                    /* fall through */
                case 129:       /*  REF32   */
                case 131:       /*  REF16   */
                case 132:       /*  REF8    */
                case 134:       /*  REF16D  */
                                /*  skip relocation info */
                    fseek(fi, fgetl(fi) * 4 + 4, 1);
                    break;
                default:
                    printf("Symbol type %d unknown\n", type);
                    exit(1);
                }
            }
            break;
        case HUNK_END:
            ++n;
            ++h;
            break;
        default:
            printf("Hunk type %08lx unknown\n", h->Type);
            exit(1);
        }
    }

    /*
     *  setup PC start locations for hunks
     */

    for (n = 0, h = Hunks; n < NumHunks; ++n, ++h) {
        h->Pc = pc;
        pc += h->Len;
    }
}

void
RelocHunks(fi)
FILE *fi;
{
    int n;
    int dhno;
    Hunk *h;
    Hunk *dh;

    n = 0;
    h = Hunks;

    while (n < NumHunks) {
        int32_t len;
        int32_t t;

        t = fgetl(fi);
        switch(t) {
        case HUNK_BSS:
            fgetl(fi);  /*  skip length */
            break;
        case HUNK_CODE:
        case HUNK_DATA:
            len = fgetl(fi) * 4;
            fseek(fi, len, 1);
            break;
        case HUNK_RELOC32:
            while (len = fgetl(fi)) {
                dhno = fgetl(fi);       /*  hunk to relocate to */
                if (dhno < FirstHunk || dhno >= NumHunks + FirstHunk) {
                    printf("hunk# in reloc32 oob: %d/%d\n", dhno, NumHunks);
                    exit(1);
                }
                dh = Hunks + dhno - FirstHunk;
                while (len) {           /*  offsets to relocate */
                    int32_t off = fgetl(fi);

                    if (off < 0 || off > h->Len - 4) {
                        printf("Offset out of bounds: %d/%d\n", off, h->Len);
                        exit(1);
                    }
                    *(int32_t *)(h->Data + off) += dh->Pc;
                    if (DDebug)
                        printf("Reloc in %d to %d offset %d add %04x\n", n, dhno, off, dh->Pc);
                    --len;
                }
            }
            break;
        case HUNK_SYMBOLS:
            /*
             *  ignore
             */
            while (len = fgetl(fi)) {
                ubyte type = len >> 24;

                len &= 0x00FFFFFF;
                fseek(fi, len * 4, 1);

                switch(type) {
                case 0:         /*  SYMB    */
                case 1:         /*  DEF     */
                case 2:         /*  ABS     */
                case 3:         /*  RES     */
                    fgetl(fi);
                    break;
                case 130:       /*  COMMON  */
                    puts("COMMON symbol not supported");
                    fgetl(fi);  /*  skip common size */
                    /* fall through */
                case 129:       /*  REF32   */
                case 131:       /*  REF16   */
                case 132:       /*  REF8    */
                case 134:       /*  REF16D  */
                                /*  skip relocation info */
                    fseek(fi, fgetl(fi) * 4 + 4, 1);
                    break;
                default:
                    printf("Symbol type %d unknown\n", type);
                    exit(1);
                }
            }
            break;
        case HUNK_END:
            ++n;
            ++h;
            break;
        default:
            printf("Hunk type %08lx unknown\n", h->Type);
            exit(1);
        }
    }
}

/*
 *  Dump the stuff, in order
 */

void
DumpHunks(fo)
FILE *fo;
{
    short n;
    Hunk *h;

    for (n = 0, h = Hunks; n < NumHunks; ++n, ++h) {
        fwrite(h->Data, 1, h->Len, fo);
    }
}


int32_t
fgetl(fi)
FILE *fi;
{
    int32_t v;

    v = getc(fi) << 24;
    v |= getc(fi) << 16;
    v |= getc(fi) << 8;
    v |= getc(fi);

    if (feof(fi)) {
        puts("unexpected EOF");
        exit(1);
    }
    if (DDebug)
        printf("get %08x\n", v);
    return(v);
}

