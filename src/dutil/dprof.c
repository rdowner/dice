/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  DPROF.C
 *
 *  DPROF <dproffile> [-call]
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>

#ifdef AMIGA
#include <lib/profile.h>
#include <lib/version.h>
#include <lib/muldiv.h>
#else
#include <include/lib/profile.h>
#include <include/lib/version.h>
#include <include/lib/muldiv.h>
#endif

ProfSym **ProfAry;
ProfSym *ProfList;
ProfSym *ProfData;

void DumpProfInfo(void);
void DumpProfInfoFrom(ProfSym *);
void DumpProfTree(int32_t, ProfSym *, short, int32_t);
void DumpCombineProfTree(ProfSym *);
void SortProfList(void);
void help(int);
char *NameOf(ProfSym *);
short NoLoop(ProfSym *, ProfSym *);

uint32_t TimeBase;
short MaxSymLen;
short CallTreeOpt;
char    FileName[256];

IDENT("dprof", ".1");
DCOPYRIGHT;

main(ac, av)
char *av[];
{
    int fd;
    short i;
    ProfHdr phdr;

    for (i = 1; i < ac; ++i) {
        char *ptr = av[i];
        if (*ptr != '-') {
            strcpy(FileName, ptr);
            continue;
        }
        ptr += 2;
        switch(ptr[-1]) {
        case 'c':
            CallTreeOpt = 1;
            break;
        default:
            help(0);
        }
    }
    if (FileName[0] == 0)
        help(0);

    strcat(FileName, ".dprof");
    if ((fd = open(FileName, O_RDONLY|O_BINARY)) < 0) {
        FileName[strlen(FileName) - 6] = 0;
        if ((fd = open(FileName, O_RDONLY|O_BINARY)) < 0) {
            printf("Unable to open %s\n", FileName);
            exit(10);
        }
    }
    if (read(fd, &phdr, sizeof(phdr)) != sizeof(phdr) || phdr.ph_Magic != PROF_MAGIC) {
        printf("%s not a dprof file\n", FileName);
        exit(20);
    }
    if ((ProfAry = malloc(phdr.ph_NumIds * sizeof(ProfSym *))) == NULL) {
        puts("no memory");
        exit(20);
    }
    TimeBase = phdr.ph_TimeBase;
    clrmem(ProfAry, phdr.ph_NumIds * sizeof(ProfSym *));
    {
        int32_t n;
        ProfSym *ps;

        n = lseek(fd, 0L, 2);

        if ((ProfData = malloc(n)) == NULL) {
            puts("no memory!");
            exit(20);
        }
        lseek(fd, 0L, 0);
        if (read(fd, ProfData, n) != n) {
            printf("read error\n");
            exit(20);
        }
        for (ps = (ProfSym *)((ProfHdr *)ProfData + 1); (char *)ps < ((char *)ProfData + n); ps = (ProfSym *)((char *)ps + ps->ps_Size)) {
            int32_t i = ps->ps_Id;

            ps->ps_TimeStamp = 0;
            ps->ps_AccumTime = 0;

            if (i < 0 || i >= phdr.ph_NumIds) {
                printf("illegal id: %d\n", i);
                exit(20);
            }
            ProfAry[i] = ps;
            if (ps->ps_FuncName[0]) {
                short len;

                ps->ps_Link = ProfList;
                ProfList = ps;

                if ((len = strlen(ps->ps_FuncName)) > MaxSymLen)
                    MaxSymLen = len;
            } else if (ProfList) {
                ps->ps_SibLink = ProfList->ps_SibLink;
                ProfList->ps_SibLink = ps;
            }
        }
        for (ps = (ProfSym *)((ProfHdr *)ProfData + 1); (char *)ps < ((char *)ProfData + n); ps = (ProfSym *)((char *)ps + ps->ps_Size)) {
            if (ps->ps_Parent)
                ps->ps_Parent = ProfAry[(int32_t)ps->ps_Parent];
        }
    }

    printf("\n%s\t\t%s\n\n", Ident, FileName);

    DumpProfInfo();
    return(0);
}

void
help(code)
{
    printf("%s\n%s\n", Ident, DCopyright);
    exit(code);
}

void
DumpProfInfo()
{
    ProfSym *ps;
    uint32_t grandTotal = 0;

    /*
     *  Id for quick name reference
     */

    for (ps = ProfList; ps; ps = ps->ps_Link) {
        ProfSym *ps2;
        for (ps2 = ps; ps2; ps2 = ps2->ps_SibLink)
            ps2->ps_Id = ps->ps_Id;
    }

    /*
     *  AccumTime   total time used by node & subroutines for all instances
     *              (only good in header node of proflist)
     *
     *  TotalTime   total time used by node & subroutines
     *
     *  TimeStamp   local time used by this node only
     */


    for (ps = ProfList; ps; ps = ps->ps_Link) {
        ProfSym *ps2;
        int32_t total = 0;
        int32_t calls = 0;

        if (ps->ps_Parent == NULL)
            grandTotal += ps->ps_TotalTime;

        for (ps2 = ps; ps2; ps2 = ps2->ps_SibLink) {
            if (NoLoop(ps, ps2))
                total += ps2->ps_TotalTime;

            if (ps2->ps_Parent)
                ps2->ps_Parent->ps_TimeStamp += ps2->ps_TotalTime;
        }
        ps->ps_AccumTime = total;
    }

    for (ps = ProfList; ps; ps = ps->ps_Link) {
        ProfSym *ps2;

        for (ps2 = ps; ps2; ps2 = ps2->ps_SibLink)
            ps2->ps_TimeStamp = ps2->ps_TotalTime - ps2->ps_TimeStamp;
    }

    printf("GrandTotal: %5ld.%02d mS\n",
        MulDivU(grandTotal, 1000, TimeBase), MulDivU(grandTotal, 100000, TimeBase) % 100,
    );
    printf("\n**** BY ROUTINE ****\n\n");

    /*
     *  sort by total time
     */

    SortProfList();

    /*
     *  display master statistics
     */

    for (ps = ProfList; ps; ps = ps->ps_Link) {
        ProfSym *ps2;
        int32_t total = 0;
        int32_t local = 0;
        int32_t calls = 0;

        for (ps2 = ps; ps2; ps2 = ps2->ps_SibLink) {
            if (NoLoop(ps, ps2))
                total += ps2->ps_TotalTime;
            local += ps2->ps_TimeStamp;
            calls += ps2->ps_NumCalls;
        }
        printf("%-*s calls=%-6d ", MaxSymLen, ps->ps_FuncName, calls);
        printf("total=%5ld.%02d mS (%3d.%02d%%) local=%5ld.%02d mS (%3d.%02d%%)\n",
            MulDivU(total, 1000, TimeBase), MulDivU(total, 100000, TimeBase) % 100,
            MulDivU(total, 100, grandTotal), MulDivU(total, 10000, grandTotal) % 100,

            MulDivU(local, 1000, TimeBase), MulDivU(local, 100000, TimeBase) % 100,
            MulDivU(local, 100, grandTotal), MulDivU(local, 10000, grandTotal) % 100
        );
    }

    printf("\n**** BY PARENT ****\n\n");

    for (ps = ProfList; ps; ps = ps->ps_Link) {
        if (ps->ps_SibLink)
            DumpProfInfoFrom(ps);
    }

    printf("\n**** COMBINED CALL TREE ****\n\n");

    for (ps = ProfList; ps; ps = ps->ps_Link)
        DumpCombineProfTree(ps);

    if (CallTreeOpt) {
        printf("\n**** CALL TREE ****\n\n");

        for (ps = ProfList; ps; ps = ps->ps_Link) {
            if (ps->ps_Parent == NULL)
                DumpProfTree(ps->ps_TotalTime, ps, 0, 32768);
        }
    }
}

void
DumpProfInfoFrom(ps)
ProfSym *ps;
{
    ProfSym *ps2;
    int32_t total = 0;
    int32_t calls = 0;

    for (ps2 = ps; ps2; ps2 = ps2->ps_SibLink) {
        if (NoLoop(ps, ps2))
            total += ps2->ps_TotalTime;
        calls += ps2->ps_NumCalls;
    }
    printf("%-*s calls=%-6d ", MaxSymLen, ps->ps_FuncName, calls);
    printf("total=%5ld.%02d mS\n",
        MulDivU(total, 1000, TimeBase), MulDivU(total, 100000, TimeBase) % 100
    );
    for (ps2 = ps; ps2; ps2 = ps2->ps_SibLink) {
        printf("    From %-*s calls=%-6d ", MaxSymLen, NameOf(ps2->ps_Parent), ps2->ps_NumCalls);
        printf("total=%5ld.%02d mS (%3d.%02d%%)\n",
            MulDivU(ps2->ps_TotalTime, 1000, TimeBase), MulDivU(ps2->ps_TotalTime, 100000, TimeBase) % 100,
            MulDivU(ps2->ps_TotalTime, 100, total), MulDivU(ps2->ps_TotalTime, 10000, total) % 100
        );
    }
}

void
DumpProfTree(total, ps, tab, limit)
int32_t total;
ProfSym *ps;
short tab;
int32_t limit;
{
    ProfSym *ps1;
    ProfSym *ps2;
    short cnt = 0;

    printf("%*.*s%-*s calls=%-5ld tot=%5ld.%02d (%3d.%02d)  loc=%5ld.%02d (%3d.%02d)",
        tab, tab, "",
        MaxSymLen, NameOf(ps),
        ps->ps_NumCalls,
        MulDivU(ps->ps_TotalTime, 1000, TimeBase), MulDivU(ps->ps_TotalTime, 100000, TimeBase) % 100,
        MulDivU(ps->ps_TotalTime, 100, total), MulDivU(ps->ps_TotalTime, 10000, total) % 100,
        MulDivU(ps->ps_TimeStamp, 1000, TimeBase), MulDivU(ps->ps_TimeStamp, 100000, TimeBase) % 100,
        MulDivU(ps->ps_TimeStamp, 100, total), MulDivU(ps->ps_TimeStamp, 10000, total) % 100
    );

    if (limit) {
        for (ps1 = ProfList; ps1; ps1 = ps1->ps_Link) {
            for (ps2 = ps1; ps2; ps2 = ps2->ps_SibLink) {
                if (ps2->ps_Parent == ps) {
                    if (cnt == 0)
                        puts(" {");
                    DumpProfTree(total, ps2, tab + 4, limit - 1);
                    ++cnt;
                }
            }
        }
    }
    if (cnt)
        printf("%*.*s}\n", tab, tab, "");
    else
        puts("");
}

void
DumpCombineProfTree(ps)
ProfSym *ps;
{
    ProfSym *ps2;
    ProfSym *ps3;
    int32_t total = 0;
    int32_t local = 0;
    int32_t calls = 0;

    for (ps2 = ps; ps2; ps2 = ps2->ps_SibLink) {
        if (NoLoop(ps, ps2))
            total += ps2->ps_TotalTime;
        calls += ps2->ps_NumCalls;
        local += ps2->ps_TimeStamp;
    }
    if (calls == 0)
        return;

    printf("%-*s calls=%-5ld tot=%5ld.%02d (%3d.%02d)  loc=%5ld.%02d (%3d.%02d)\n",
        MaxSymLen, NameOf(ps),
        calls,
        MulDivU(total, 1000, TimeBase), MulDivU(total, 100000, TimeBase) % 100,
        MulDivU(total, 100, total), MulDivU(total, 10000, total) % 100,
        MulDivU(local, 1000, TimeBase), MulDivU(local, 100000, TimeBase) % 100,
        MulDivU(local, 100, total), MulDivU(local, 10000, total) % 100
    );

    /*
     *  find all routines this routine explicitly calls
     */

    for (ps2 = ProfList; ps2; ps2 = ps2->ps_Link) {
        int32_t subTotal = 0;
        int32_t subLocal = 0;
        int32_t subCalls = 0;

        /*
         *  add totals for routines that we (ps) call.  For each ps2 element
         *  check to see if it is called by ps.
         */

        for (ps3 = ps2; ps3; ps3 = ps3->ps_SibLink) {
            if (ps3->ps_Parent && ps3->ps_Parent->ps_Id == ps->ps_Id) {
                subTotal += ps3->ps_TotalTime;
                subCalls += ps3->ps_NumCalls;
                subLocal += ps3->ps_TimeStamp;

                /*
                 *  Scan the baselist backwards looking for this ps3.
                 *  If found then subtract the base total entry from
                 *  the subTotal since (1) the base is already counted
                 *  and (2) its subroutines have been or will be counted
                 *
                 *                          B
                 *                           \
                 *                            Y     Y's total
                 *                             \
                 *                              B   should not include B
                 */

                {
                    ProfSym *ps4;
                    ProfSym *ps5;

                    for (ps4 = ps; ps4; ps4 = ps4->ps_SibLink) {
                        for (ps5 = ps4->ps_Parent; ps5; ps5 = ps5->ps_Parent) {
                            if (ps5->ps_Id == ps->ps_Id)
                                break;
                            if (ps5 == ps3) {
                                subTotal -= ps4->ps_TotalTime;
                                break;
                            }
                        }
                    }
                }
            }
        }

        if (subCalls) {
            if (ps2->ps_Id == ps->ps_Id) {
                printf("    %-*s calls=%-5ld\n",
                    MaxSymLen, "<SELF>",
                    subCalls
                );
            } else {
                printf("    %-*s calls=%-5ld tot=%5ld.%02d (%3d.%02d)  loc=%5ld.%02d (%3d.%02d)\n",
                    MaxSymLen, NameOf(ps2),
                    subCalls,
                    MulDivU(subTotal, 1000, TimeBase), MulDivU(subTotal, 100000, TimeBase) % 100,
                    MulDivU(subTotal, 100, total), MulDivU(subTotal, 10000, total) % 100,
                    MulDivU(subLocal, 1000, TimeBase), MulDivU(subLocal, 100000, TimeBase) % 100,
                    MulDivU(subLocal, 100, total), MulDivU(subLocal, 10000, total) % 100
                );
            }
        }
    }
}


char *
NameOf(psc)
ProfSym *psc;
{
    if (psc == NULL)
        return("<root>");
    return(ProfAry[psc->ps_Id]->ps_FuncName);
}

void
SortProfList()
{
    ProfSym *ps;
    ProfSym *psnext;
    ProfSym **pp;
    int32_t total = 0;

    for (ps = ProfList, ProfList = NULL; ps; ps = psnext) {
        psnext = ps->ps_Link;

        for (pp = &ProfList; *pp; pp = &(*pp)->ps_Link) {
            if (ps->ps_AccumTime > (*pp)->ps_AccumTime)
                break;
        }
        ps->ps_Link = *pp;
        *pp = ps;
    }
}

/*
 *  Ensure that no parent of node is in the baselist
 */

short
NoLoop(base, node)
ProfSym *base;
ProfSym *node;
{
    if (node) {
        while (node = node->ps_Parent) {
            if (node->ps_Id == base->ps_Id)
                return(0);
        }
    }
    return(1);
}

