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
 *  HUNKS.C
 *
 *  HUNKS executable
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef AMIGA
#include <lib/version.h>
#else
#include <include/lib/version.h>
#include <suplib/all.h>
#ifdef __aligned
#undef __aligned
#endif
#define __aligned
#endif

IDENT("hunks",".3");
DCOPYRIGHT;

typedef unsigned char ubyte;
typedef unsigned short uword;

void DumpHunks(char *);
uint32_t fgetl(FILE *);
void fgetname(FILE *, char *, int32_t);
void zfseek(FILE *, int32_t);

short AllOpt;

int
main(int ac, char **av)
{
    short i;

#ifndef unix
    expand_args(ac, av, &ac, &av);
#endif

    if (ac == 1) {
        printf("%s\n%s\n", Ident, DCopyright);
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

    if ((fi = fopen(fileName, "r")) != NULL) {
        uint32_t type;
        uint32_t flags;
        uint32_t skip;
        uint32_t n;

        while ((type = fgetl(fi)) != 0) {
            skip = 0;

            flags = type & 0xFFFF0000;
            type  = type & 0x0000FFFF;

            switch(type) {
            case 0x3F3:             /*  HUNK_HEADER */
                {
                    int32_t hno;

                    n = fgetl(fi);
                    fgetname(fi, buf, n);
                    skip = fgetl(fi);
                    printf("%08x HUNK_HEADER\tname=%s\tsize=%ld ", 0x3F3, buf, skip);
                    printf("first=%ld ", (hno = fgetl(fi)));
                    printf("last=%ld\n", fgetl(fi));

                    while (skip) {
                        --skip;
                        n = fgetl(fi);
                        printf("    %08lx HUNK %ld SIZE %ld", n, hno, (n & 0x00FFFFFF) * 4);
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
                printf("%08lx HUNK_CODE\tsize=%ld\n", type, skip * 4);
                break;
            case 0x3EA:             /*  HUNK_DATA       */
                skip = fgetl(fi);
                printf("%08lx HUNK_DATA\tsize=%ld\n", type, skip * 4);
                break;
                break;
            case 0x3EB:             /*  HUNK_BSS        */
                skip = fgetl(fi);
                printf("%08lx HUNK_BSS\tsize=%ld\n", type, skip * 4);
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
                while ((skip = fgetl(fi)) != 0) {
                    n = fgetl(fi);
                    printf("    %ld relocations to hunk %ld\n", skip, n);
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
                while ((skip = fgetl(fi)) != 0) {
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
                        printf("\t%ld relocs", n);
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
                printf("%08lx HUNK_DEBUG\t%ld bytes\n", type, skip * 4);
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

uint32_t
fgetl(fi)
FILE *fi;
{
    uint32_t n;
    short c;

    if ((c = getc(fi)) != EOF) {
        n = c << 24;
        if ((c = getc(fi)) != EOF) {
            n |= c << 16;
            if ((c = getc(fi)) != EOF) {
                n |= c << 8;
                if ((c = getc(fi)) != EOF) {
                    n |= c;
                    return(FromMsbOrder(n));
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
int32_t len;
{
    int32_t n;

    for (n = 0; n < len && n < 255/4; ++n) {
        *((int32_t *)buf + n) = ToMsbOrder(fgetl(fi));
    }
    *((int32_t *)buf + n) = ToMsbOrder(0);
}

void
zfseek(fi, numLongs)
FILE *fi;
int32_t numLongs;
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

