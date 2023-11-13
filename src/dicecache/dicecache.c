/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  DICECACHE.C
 *
 *  DICECACHE [ON][OFF][MAXFILE][MAXSIZE][ADD][REM]
 *
 */
#ifdef AMIGA
#include <lib/version.h>

#ifdef _DCC
IDENT("DiceCache",".57");
DCOPYRIGHT;
#endif
#endif

#include <stdio.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <clib/exec_protos.h>
#include <clib/dicecache_protos.h>

char Buf[1024];

extern void *DiceCacheBase;

void FlushMemory(void);

main(ac, av)
char *av[];
{
    short i;
    short flushit = 0;
    long la[6];

    puts("DICECACHE V1.00");

    if (DiceCacheBase == NULL) {
        DiceCacheBase = OpenLibrary("dicecache.library", 0);
        if (DiceCacheBase == NULL) {
            puts("Unable to open dicecache.library");
            exit(1);
        }
    }
    DiceCacheGet(la, 6);

    for (i = 1; i < ac; ++i) {
        char *ptr = av[i];

        if (stricmp(ptr, "?") == 0) {
            puts("DICECACHE [ON][OFF][FLUSH [n]][MAXFILE n][MAXSIZE n][ADD suffix][REM suffix][TEST]");
        } else if (stricmp(ptr, "ON") == 0) {
            DiceCacheEnable();
        } else if (stricmp(ptr, "OFF") == 0) {
            DiceCacheDisable();
            flushit = 1;
        } else if (stricmp(ptr, "FLUSH") == 0) {
            long bytes;

            if (av[i+1] == NULL || (bytes = strtol(av[i+1], NULL, 0)) == 0)
                bytes = 0x1FFFFFFF;
            else
                ++i;
            DiceCacheFlush(bytes);
        } else if (stricmp(ptr, "MAXFILE") == 0) {
            la[3] = strtol(av[++i], NULL, 0);
            DiceCacheSet(la, 4);
        } else if (stricmp(ptr, "MAXSIZE") == 0) {
            la[2] = strtol(av[++i], NULL, 0);
            DiceCacheSet(la, 4);
        } else if (stricmp(ptr, "ADD") == 0) {
            DiceCacheAddSuffix(av[++i]);
        } else if (stricmp(ptr, "REM") == 0) {
            DiceCacheRemSuffix(av[++i]);
        } else if (stricmp(ptr, "TEST") == 0) {
            long size;
            void *dc;
            char *buf;

            if (dc = DiceCacheOpen(ptr, "r", &size)) {
                printf("Open %s size %d\n", ptr, size);
                if (buf = DiceCacheSeek(dc, 0, &size)) {
                    printf("BUFFER %08lx size %d\n\n", buf, size);
                    write(1, buf, (size > 256) ? 256 : size);
                    puts("");
                } else {
                    printf("Seek failed\n");
                }
                DiceCacheClose(dc);
            } else {
                printf("DiceCacheOpen() failed\n");
            }
        } else {
            printf("Bad option: %s\n", ptr);
        }
    }

    DiceCacheGet(la, 6);
    DiceCacheGetSuffixes(Buf, sizeof(Buf));

    if (flushit) {
        DiceCacheFlush(0x1FFFFFFF);
        CloseLibrary(DiceCacheBase);
        DiceCacheBase = NULL;
        FlushMemory();
    } else {
        printf("Cached %d/%d/%d filemax=%d hit=%d/%d (%2d%%) suffixes=%s\n",
            la[0],
            la[1],
            la[2],
            la[3],
            la[4],
            la[5],
            la[4] * 100 / la[5],
            Buf
        );
    }
    return(0);
}

void
FlushMemory()
{
    void *ptr;

    Forbid();
    if (ptr = AllocMem(0x03FFFFFF, MEMF_PUBLIC)) {
        FlushMemory();
        FreeMem(ptr, 0x03FFFFFF);
    }
    Permit();
}

