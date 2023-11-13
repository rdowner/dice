
/*
 *  HUNKS.C
 *
 *  HUNKS executable
 *
 * (c)Copyright 1992 Obvious Implementations Corp, All Rights Reserved
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lib/version.h>

#ifdef _DCC
IDENT("hunks",".3");
DCOPYRIGHT;
#else
#define __aligned
#endif

typedef unsigned char ubyte;
typedef unsigned short uword;
typedef unsigned long ulong;

void DumpHunks(char *);
ulong fgetl(FILE *);
void fgetname(FILE *, char *, long);
void zfseek(FILE *, long);

short AllOpt;

main(ac, av)
int ac;
char *av[];
{
    short i;

#ifndef unix
    expand_args(ac, av, &ac, &av);
#endif

    if (ac == 1) {
#ifdef _DCC
        printf("%s\n%s\n", Ident, DCopyright);
#endif
        puts("hunks [-all] executeable/object_module");
        exit(0);
    }
    for (i = 1; i < ac; ++i) {
        char *ptr = av[i];

        if (*ptr != '-') {
            printf("\n** FILE %s **\n\n", ptr);
            DumpHunks(ptr);
            continue;
        }
        if (stricmp(ptr, "-all") == 0)
            AllOpt = 1;
    }
    return(0);
}

void
DumpHunks(fileName)
char *fileName;
{
    FILE *fi;
    __aligned char buf[256];

    if (fi = fopen(fileName, "r")) {
        ulong type;
        ulong flags;
        ulong skip;
        ulong n;

        while (type = fgetl(fi)) {
            skip = 0;

            flags = type & 0xFFFF0000;
            type  = type & 0x0000FFFF;

            switch(type) {
            case 0x3F3:             /*  HUNK_HEADER */
                {
                    long hno;

                    n = fgetl(fi);
                    fgetname(fi, buf, n);
                    skip = fgetl(fi);
                    printf("%08lx HUNK_HEADER\tname=%s\tsize=%d ", 0x3F3, buf, skip);
                    printf("first=%d ", hno = fgetl(fi));
                    printf("last=%d\n", fgetl(fi));

                    while (skip) {
                        --skip;
                        n = fgetl(fi);
                        printf("    %08lx HUNK %d SIZE %d", n, hno, (n & 0x00FFFFFF) * 4);
                        switch(n & 0xC0000000) {
                        case 0x00000000:
                            break;
                        case 0x40000000:
                            printf(" MEMF_CHIP");
                            break;
                        case 0x80000000:
                            printf(" MEMF_FAST");
                            break;
                        case 0xC0000000:
                            printf(" MEMF_CHIP|MEMF_FAST");
                            break;
                        }
                        puts("");
                        ++hno;
                    }
                }
                break;
            case 0x3E9:             /*  HUNK_CODE   */
                skip = fgetl(fi);
                printf("%08lx HUNK_CODE\tsize=%d\n", type, skip * 4);
                break;
            case 0x3EA:             /*  HUNK_DATA       */
                skip = fgetl(fi);
                printf("%08lx HUNK_DATA\tsize=%d\n", type, skip * 4);
                break;
                break;
            case 0x3EB:             /*  HUNK_BSS        */
                skip = fgetl(fi);
                printf("%08lx HUNK_BSS\tsize=%d\n", type, skip * 4);
                skip = 0;
                break;
            case 0x3EC:             /*  HUNK_RELOC32    */
                printf("%08lx HUNK_RELOC-ABSOLUTE32\n", type);
            case 0x3ED:             /*  HUNK_RELOC16-PC */
                if (type == 0x3ED)
                    printf("%08lx HUNK_RELOC-PCREL16\n", type);
            case 0x3EE:             /*  HUNK_RELOC8     */
                if (type == 0x3EE)
                    printf("%08lx HUNK_RELOC-PCREL8\n", type);
            case 0x3F8:             /*  HUNK_RELOC16-D  (special) */
                if (type == 0x3F8)
                    printf("%08lx HUNK_RELOC-DATAREL16\n", type);
                while (skip = fgetl(fi)) {
                    n = fgetl(fi);
                    printf("    %d relocations to hunk %d\n", skip, n);
                    if (AllOpt) {
                        while (skip) {
                            --skip;
                            n = fgetl(fi);
                            printf("\t@0x%08lx\n", n);
                        }
                    } else {
                        zfseek(fi, skip);
                    }
                }
                break;
            case 0x3EF:             /*  HUNK_EXT        */
                printf("%08lx HUNK_EXT\n", type);
            case 0x3F0:             /*  HUNK_SYMBOL     */
                if (type == 0x3F0)
                    printf("%08lx HUNK_SYMBOL\n", type);
                while (skip = fgetl(fi)) {
                    ubyte symType = skip >> 24;

                    skip &= 0x00FFFFFF;
                    fgetname(fi, buf, skip);
                    printf("    0x%02x:%-15s", symType, buf);

                    switch(symType) {
                    case 0:         /*  SYMB    */
                        printf("\tSYMBOL");
                    case 1:         /*  DEF     */
                        if (symType == 1)
                            printf("\tXDEF");
                    case 2:         /*  ABS     */
                        if (symType == 2)
                            printf("\tABS-SYM");
                    case 3:         /*  RES     */
                        if (symType == 3)
                            printf("\tRES-SYM");
                        n = fgetl(fi);
                        printf("\tval=0x%08lx", n);
                        break;
                    case 130:       /*  COMMON (def)*/
                        n = fgetl(fi);
                        printf("\tCOMMON size=0x%04lx", n);
                        break;
                    case 135:       /*  REF32-pc    */
                        printf("\tPCREL-32");
                    case 129:       /*  REF32       */
                        if (symType == 129)
                            printf("\tABSOLUTE-32");
                    case 131:       /*  REF16-pc    */
                        if (symType == 131)
                            printf("\tPCREL-16");
                    case 132:       /*  REF8        */
                        if (symType == 132)
                            printf("\tPCREL-8");
                    case 134:       /*  REF16D      */
                        if (symType == 134)
                            printf("\tDATAREL-16");
                        n = fgetl(fi);
                        printf("\t%d relocs", n);
                        zfseek(fi, n);
                        break;
                    default:
                        printf("\tUNKNOWN TYPE FIELD");
                    }
                    puts("");
                }
                break;
            case 0x3F1:             /*  HUNK_DEBUG          */
                skip = fgetl(fi);
                printf("%08lx HUNK_DEBUG\t%d bytes\n", type, skip * 4);
                break;
            case 0x3E8:
                n = fgetl(fi);
                fgetname(fi, buf, n);
                printf("%08lx HUNK_NAME\tname=%s\n", type, buf);
                break;
            case 0x3E7:
                n = fgetl(fi);
                fgetname(fi, buf, n);
                printf("%08lx HUNK_UNIT\tname=%s\n", type, buf);
                break;
            case 0x3F2:
                printf("%08lx HUNK_END\n\n", type);
                break;
            default:
                printf("%08lx UNKNOWN HUNK TYPE\n", type);
                break;
            }
            if (skip)
                zfseek(fi, skip);
        }
    }
}

ulong
fgetl(fi)
FILE *fi;
{
    ulong n;
    short c;

    if ((c = getc(fi)) != EOF) {
        n = c << 24;
        if ((c = getc(fi)) != EOF) {
            n |= c << 16;
            if ((c = getc(fi)) != EOF) {
                n |= c << 8;
                if ((c = getc(fi)) != EOF) {
                    n |= c;
                    return(n);
                }
            }
        }
    }
    return(0);
}

void
fgetname(fi, buf, len)
FILE *fi;
char *buf;
long len;
{
    long n;

    for (n = 0; n < len && n < 255/4; ++n) {
        *((long *)buf + n) = fgetl(fi);
    }
    *((long *)buf + n) = 0;
}

void
zfseek(fi, numLongs)
FILE *fi;
long numLongs;
{
    if (numLongs < 32) {
        while (numLongs) {
            --numLongs;
            fgetl(fi);
        }
    } else {
        fseek(fi, numLongs * 4, 1);
    }
}

