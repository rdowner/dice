/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  FINAL.C
 *
 *  Generate final output
 */

#include "defs.h"

#define HEAD    0x48454144
#define DBGV    0x44424756

Prototype void  GenerateFinalExecutable(FILE *, List *);
Prototype short ForceSym(Sym *);

void
GenerateFinalExecutable(fo, list)
FILE *fo;
List *list;
{
    HunkListNode *hl;
    Hunk *hunk;
    int i;
    int32_t nhunk_debug = 0;   /*       debug hunks */
    int32_t nhunk_symd0 = 0;   /*       symbol hunks with no associated debug hunks */
    int32_t nhunk_symd1 = 0;   /*       symbol hunks with associated debug hunks    */
    int32_t headdb_seek = 0;

    putl(fo, 0x3F3);        /*  hunk_hdr    */
    putl(fo, 0);            /*  no name     */
    putl(fo, NumExtHunks);  /*  table size  */
    putl(fo, 0);            /*  first hunk  */
    putl(fo, NumExtHunks-1);/*  last hunk   */

    for (hl = GetHead(list); hl; hl = GetSucc(&hl->Node)) {
        int32_t mask = (hl->HunkId & 0xFFFF0000) & ~HUNKIDF_FLAG;

        if (ChipOpt) {
            mask &= ~0x80000000;
            mask |= 0x40000000;
        }
        if (ResOpt)
            putl(fo, mask | (hl->FinalSize >> 2));
        else
            putl(fo, mask | ((hl->FinalSize + hl->AddSize) >> 2));
    }

    /*
     *  If debugging is enabled (-d), include the master debug hunk
     *  required by CPR
     */

    if (DebugOpt) {
        int32_t lw = 8 + 3;    /*       8 lw of hdr, 3 lw of sub hdr    */
        int32_t nsymd0 = 0;
        int32_t nsymd1 = 0;
        int32_t max_array = 0;

        for (hl = GetHead(list); hl; hl = GetSucc(&hl->Node)) {
            short havesyms = 0;
            short havedebug = 0;

            for (hunk = GetHead(&hl->HunkList); hunk; hunk = GetSucc(&hunk->Node)) {
                if (hunk->DbInfo) {
                    ++lw;   /*  number of debug hunks   */
                    ++nhunk_debug;
                    havedebug = 1;
                }
            }
            {
                Sym *sym;

                for (hunk = GetHead(&hl->HunkList); hunk; hunk = GetSucc(&hunk->Node)) {
                    for (sym = GetHead(&hunk->SymList); sym; sym = GetSucc((Node *)&sym->Node)) {
                        if (sym->Type == 1) {
                            if (havesyms == 0) {
                                if (havedebug)
                                    ++nhunk_symd1;
                                else
                                    ++nhunk_symd0;
                                ++lw;
                                havesyms = 1;
                            }
                            if (havedebug)
                                ++nsymd1;
                            else
                                ++nsymd0;
                        }
                    }
                }
            }
        }

        if ((max_array = nhunk_debug) < nhunk_symd0)
            max_array = nhunk_symd0;
        if (max_array < nhunk_symd1)
            max_array = nhunk_symd1;

        putl(fo, 0x3F1);
        putl(fo, lw-2);
        putl(fo, max_array);
        putl(fo, HEAD);
        putl(fo, DBGV);
        putl(fo, 0x30310000);
        putl(fo, nsymd0);
        putl(fo, nsymd1);
        headdb_seek = ftell(fo);
        for (lw = lw - 8; lw; --lw)
            putl(fo, 0);
    }

    /*
     *  MAIN BODY
     */

    for (hl = GetHead(list); hl; hl = GetSucc(&hl->Node)) {
        int32_t mask = (hl->HunkId & 0xFFFF0000) & ~HUNKIDF_FLAG;

        if (ChipOpt) {
            mask &= ~0x80000000;
            mask |= 0x40000000;
        }

        /*
         *  output HUNK_CODE, DATA, or BSS
         */

        switch(hl->Node.ln_Type) {
        case NT_BSS:
            putl(fo, 0x3EB | mask);
            break;
        case NT_CODE:
            putl(fo, 0x3E9 | mask);
            break;
        case NT_DATA:
            putl(fo, 0x3EA | mask);
            break;
        }

        putl(fo, hl->FinalSize >> 2);

        /*
         *  if not BSS output the code/data
         */

        if (hl->Node.ln_Type == NT_DATA || hl->Node.ln_Type == NT_CODE) {
            int32_t n = 0;

            for (hunk = GetHead(&hl->HunkList); hunk; hunk = GetSucc(&hunk->Node)) {
                if (hunk->Node.ln_Type == NT_BSS)
                    continue;
                fwrite((char *)hunk->Data, 1, hunk->Bytes, fo);

                /*
                 *  Generate jump table
                 */

                if (hunk->TotalBytes != hunk->Bytes) {
                    dbprintf(0, ("WRITE %d bytes of jump table\n", hunk->TotalBytes - hunk->Bytes));
                    fwrite((char *)hunk->JmpData, 1, (hunk->TotalBytes - hunk->Bytes + 3) & ~3, fo);
                }
                n += (hunk->TotalBytes + 3) & ~3;
            }
            if (n != hl->FinalSize)
                cerror(EFATAL_FINAL_SIZE_MISMATCH, n, hl->FinalSize);
        }

        /*
         *  If there is any relocation information output that
         */

        for (i = 0; i < NumExtHunks; ++i) {
            if (hl->CntReloc32[i])
                break;
        }
        if (i < NumExtHunks) {          /*  at least one    */
            HunkListNode *hl2 = GetHead(list);

            if (PIOpt)
                cerror(EERROR_RELOC32_ILLEGAL_PI_NH);

            putl(fo, 0x3EC);            /*  HUNK_RELOC32    */
            for (i = 0; i < NumExtHunks; ++i) {
                int32_t n;

                if ((n = hl->CntReloc32[i]) != 0) {
                    if (ResOpt && !AbsWordOpt && hl->Node.ln_Type != NT_CODE && hl2->Node.ln_Type != NT_CODE)
                        cerror(EERROR_RELOC32_DATA_DATA_RES);
                    if (hl->CpyReloc32[i] != n)
                        cerror(EFATAL_RELOC_ARRAY_MISMATCH, hl->CpyReloc32[i], n);

                    putl(fo, n);        /*  n relocs to     */
                    putl(fo, i);        /*  hunk # i        */
                    fwrite((char *)hl->ExtReloc32[i], 4, n, fo);
                    if (PIOpt)
                        printf(" 32 bit reloc hunk %d:$%x to hunk %d\n",
                               hl->FinalHunkNo, hl->ExtReloc32[i][0], i);

#ifdef DEBUG
                    if (DDebug > 5) {
                        int32_t j;
                        for (j = 0; j < n; ++j)
                            printf(" -- reloc h %d offset %08x\n",
                                   i, hl->ExtReloc32[i][j]);
                    }
#endif
                }
                hl2 = GetSucc(&hl2->Node);
            }
            putl(fo, 0);
        }

        /*
         *  If symbols are enabled dump them
         */

        {
            Sym *sym;

            for (hunk = GetHead(&hl->HunkList); hunk; hunk = GetSucc(&hunk->Node)) {
                for (sym = GetHead(&hunk->SymList); sym; sym = GetSucc((Node *)&sym->Node)) {
                    if (sym->Type == 1 && (SymOpt || (sym->SymLen == 5 && ForceSym(sym))))
                        break;
                }
                if (sym)
                    break;
            }
            if (hunk) {
                hl->SeekSym = ftell(fo);
                putl(fo, 0x3F0);    /*  HUNK_SYMBOL */
                for (hunk = GetHead(&hl->HunkList); hunk; hunk = GetSucc(&hunk->Node)) {
                    for (sym = GetHead(&hunk->SymList); sym; sym = GetSucc((Node *)&sym->Node)) {
                        if (sym->Type == 1 && (SymOpt || (sym->SymLen == 5 && ForceSym(sym)))) {
                            int32_t sl = (sym->SymLen + 3) >> 2;        /*  # lws   */

                            putl(fo, sl);
                            fwrite(sym->SymName, 4, sl, fo);

                            putl(fo, sym->Value + hunk->Offset);
                        }
                    }
                }
                putl(fo, 0);
            }
        }

        /*
         *  If debugging is enabled (-d), include that
         */

        if (DebugOpt) {
            for (hunk = GetHead(&hl->HunkList); hunk; hunk = GetSucc(&hunk->Node)) {
                DBInfo *dbinfo;

                if ((dbinfo = hunk->DbInfo) == NULL)
                    continue;
                dbinfo->di_Base = ToMsbOrder(hunk->Offset);
                hunk->SeekDebug = ftell(fo);
                fwrite(dbinfo, 4, dbinfo->di_Size + 2, fo);
            }
        }

        /*
         *  HUNK_END
         */

        putl(fo, 0x3F2);
    }

    /*
     *  DEBUG MASTER HEADER (CPR SUPPORT)
     */

    if (DebugOpt) {
        fseek(fo, headdb_seek, 0);

        putl(fo, nhunk_debug);
        for (hl = GetHead(list); hl; hl = GetSucc(&hl->Node)) {
            for (hunk = GetHead(&hl->HunkList); hunk; hunk = GetSucc(&hunk->Node)) {
                if (hunk->DbInfo)
                    putl(fo, (hl->FinalHunkNo << 24) | hunk->SeekDebug);
            }
        }

        putl(fo, nhunk_symd0);
        if (SymOpt) {
            for (hl = GetHead(list); hl; hl = GetSucc(&hl->Node)) {
                short havedebug = 0;
                for (hunk = GetHead(&hl->HunkList); hunk; hunk = GetSucc(&hunk->Node)) {
                    if (hunk->DbInfo) {
                        havedebug = 1;
                        break;
                    }
                }
                if (hl->SeekSym && havedebug == 0) {
                    putl(fo, (hl->FinalHunkNo << 24) | hl->SeekSym);
                }
            }
        }

        putl(fo, nhunk_symd1);

        if (SymOpt) {
            for (hl = GetHead(list); hl; hl = GetSucc(&hl->Node)) {
                short havedebug = 0;
                for (hunk = GetHead(&hl->HunkList); hunk; hunk = GetSucc(&hunk->Node)) {
                    if (hunk->DbInfo) {
                        havedebug = 1;
                        break;
                    }
                }
                if (hl->SeekSym && havedebug == 1) {
                    putl(fo, (hl->FinalHunkNo << 24) | hl->SeekSym);
                }
            }
        }

        fseek(fo, 0L, 2);
    }
}

/*
 *  Force _main, @main, _exit, @exit
 *
 */

short
ForceSym(sym)
Sym *sym;
{
    /*if (sym->SymLen == 5)*/ {
        if (strncmp(sym->SymName + 1, "main", 4) == 0)
            return(1);
        if (strncmp(sym->SymName + 1, "exit", 4) == 0)
            return(1);
    }
    return(0);
}

