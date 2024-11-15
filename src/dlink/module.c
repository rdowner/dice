/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  MODULE.C
 *
 */

#include "defs.h"

#define LINE    0x4C494E45

Prototype Module    *CreateModule(FileNode *);
Prototype int   CreateSymbolTable(Module *);
Prototype int   ScanLibForInclusion(Module *);
Prototype void  CreateHunkListNodes(Module *, List *);
Prototype void  FinalCombineHunkListNodes(List *);
Prototype void  FixInternalSymbols(List *);
Prototype void  PIOptCombineIntoCode(List *);

/*
 *  Given a file node and a fully loaded file create a Module structure for
 *  that node.  Set the Node->ln_Type to the file node's type.
 *
 *  This routine creates a *single* module .. that is, it stops if it reaches
 *  a second hunk_unit.
 */

Module *
CreateModule(fn)
FileNode *fn;
{
    Module *mod = zalloc(sizeof(Module));

    ++MemNumModules;
    mod->Node.ln_Type = fn->Node.ln_Type;
    mod->FNode = fn;

    /*
     *  Do a validity scan and incidently count the number of hunks you
     *  find.
     */

    mod->ModBeg = (char *)fn->DPtr;
    {
        uint32_t *scan = fn->DPtr;
        uint32_t *scanEnd = (uint32_t *)((char *)fn->Data + fn->Bytes);
        int32_t nl;

        /*
         *  look for hunk_unit
         */

        if ((uword)FromMsbOrder(*scan) != 0x3E7)
            cerror(EFATAL_OBJECT_PARSE, fn->Node.ln_Name, (char *)fn->DPtr - (char *)fn->Data);

        nl = FromMsbOrder(scan[1] * 4);
        mod->Node.ln_Name = zalloc(nl + 1);
        movmem(scan + 2, mod->Node.ln_Name, nl);
        mod->Node.ln_Name[nl] = 0;

        scan += FromMsbOrder(scan[1]) + 2;

        /*
         *  scan hunks for the module.
         */

        while (scan < scanEnd && (uword)FromMsbOrder(*scan) != 0x3E7) {
            ++mod->NumHunks;            /*  # hunks in module       */

            if ((uword)FromMsbOrder(*scan) == 0x3E8) {
                scan += FromMsbOrder(scan[1]) + 2;      /*  skip hunk_name          */
            }

            while (FromMsbOrder(*scan) != 0x3F2) { /*  until we get a hunk_end */
                uint32_t len;

                switch((uword)FromMsbOrder(*scan)) {
                case 0x3E9:             /*  HUNK_CODE       */
                case 0x3EA:             /*  HUNK_DATA       */
                    scan += FromMsbOrder(scan[1]) + 2;
                    break;
                case 0x3EB:             /*  HUNK_BSS        */
                    scan += 2;
                    break;
                case 0x3EC:             /*  HUNK_RELOC32    */
                case 0x3ED:             /*  HUNK_RELOC16-PC */
                case 0x3EE:             /*  HUNK_RELOC8     */
                case 0x3F7:             /*  HUNK_RELOC32-D  (special) */
                case 0x3F8:             /*  HUNK_RELOC16-D  (special) */
                case 0x3F9:             /*  HUNK_RELOC8-D   (special) */
                    ++scan;
                    while (FromMsbOrder(*scan))
                        scan += FromMsbOrder(*scan) + 2;
                    ++scan;
                    break;
                case 0x3EF:             /*  HUNK_EXT        */
                case 0x3F0:             /*  HUNK_SYMBOL     */
                    ++scan;
                    while ((len = FromMsbOrder(*scan)) != 0) {
                        ubyte type = len >> 24;

                        len &= 0x00FFFFFF;
                        scan += len + 1;

                        switch(type) {
                        case 0:         /*  SYMB    */
                        case 1:         /*  DEF     */
                        case 2:         /*  ABS     */
                        case 3:         /*  RES     */
                            ++scan;     /*  skip value  */
                            break;
                        case 130:       /*  COMMON (def)*/
                            /*fatal("COMMON symbol not supported, module %s", fn->Node.ln_Name);*/
                            ++scan;     /*  skip common size */
                            break;
                        case 135:       /*  REF32-pc    */
                        case 129:       /*  REF32       */
                        case 131:       /*  REF16-pc    */
                        case 132:       /*  REF8        */
                        case 134:       /*  REF16D      */
                                        /*  skip relocation info */
                            scan += FromMsbOrder(scan[0]) + 1;
                            break;
                        default:
                            cerror(EFATAL_UNKNOWN_SYMBOL_TYPE, type, fn->Node.ln_Name);
                        }
                    }
                    ++scan;     /*  skip 0 terminator */
                    break;
                case 0x3F1:             /*  HUNK_DEBUG          */
                    scan += FromMsbOrder(scan[1]) + 2;
                    break;
                default:
                    cerror(EFATAL_UNKNOWN_HUNK_TYPE, *scan, fn->Node.ln_Name);
                }
            }
            ++scan;     /*  skip hunk_end   */
        }
        if (scan > scanEnd)
            cerror(EFATAL_OBJECT_SIZE_MISMATCH, fn->Node.ln_Name);
    }

    /*
     *  Allocate some stuff
     */

    mod->Hunks = zalloc(sizeof(Hunk *) * mod->NumHunks);
    MemNumHunks += mod->NumHunks;

    /*
     *  Do a load scan
     */

    {
        uint32_t *scan = fn->DPtr;
        uint32_t *scanEnd = (uint32_t *)((char *)fn->Data + fn->Bytes);
        short hunkNo = 0;

        /*
         *  look for hunk_unit
         */

        if ((uword)FromMsbOrder(*scan) != 0x3E7)
            cerror(EFATAL_146);
        scan += FromMsbOrder(scan[1]) + 2;

        /*
         *  scan hunks for the module.
         */

        while (scan < scanEnd && (uword)FromMsbOrder(*scan) != 0x3E7) {
            Hunk *hunk = zalloc(sizeof(Hunk));
            ++MemNumHunks;

            if (hunkNo >= mod->NumHunks)
                cerror(EFATAL_TOO_MANY_HUNKS);

            mod->Hunks[hunkNo] = hunk;
            hunk->HunkNo = hunkNo;
            hunk->Module = mod;
            NewList(&hunk->SymList);    /*  only used if SymOpt     */
            ++hunkNo;

            /*
             *  hunk name.  If section name starts with 'far' or in
             *  dummy flag to upper bits of hunkid to force the hunk
             *  to remain outside normal data+bss small model combine.
             */

            if ((uword)FromMsbOrder(*scan) == 0x3E8) {
                int32_t nl = FromMsbOrder(scan[1]) * 4;
                hunk->Node.ln_Name = zalloc(nl + 1);
                movmem(scan + 2, hunk->Node.ln_Name, nl);
                hunk->Node.ln_Name[nl] = 0;
                if (strnicmp(hunk->Node.ln_Name, "far", 3) == 0)
                    hunk->HunkId |= HUNKIDF_FLAG;
                scan += FromMsbOrder(scan[1]) + 2;          /*  skip hunk_name          */
            } else {
                hunk->Node.ln_Name = "";
            }

            /*  until we get a hunk_end */

            while ((uword)FromMsbOrder(*scan) != 0x3F2) {  
                uint32_t len;

                switch((uword)FromMsbOrder(*scan)) {
                case 0x3E9:             /*  HUNK_CODE       */
                    hunk->HunkId |= FromMsbOrder(*scan);
                    hunk->Node.ln_Type = NT_CODE;
                    hunk->Data = scan + 2;
                    hunk->Bytes= FromMsbOrder(scan[1]) << 2;
                    scan += FromMsbOrder(scan[1]) + 2;
                    break;
                case 0x3EA:             /*  HUNK_DATA       */
                    hunk->HunkId |= FromMsbOrder(*scan);
                    hunk->Node.ln_Type = NT_DATA;
                    hunk->Data = scan + 2;
                    hunk->Bytes= FromMsbOrder(scan[1]) << 2;
                    scan += FromMsbOrder(scan[1]) + 2;
                    break;
                case 0x3EB:             /*  HUNK_BSS        */
                    hunk->HunkId |= FromMsbOrder(*scan);
                    hunk->Node.ln_Type = NT_BSS;
                    hunk->Data = NULL;
                    hunk->Bytes = FromMsbOrder(scan[1]) << 2;
                    scan += 2;
                    break;
                case 0x3F8:             /*  HUNK_RELOC16-D  */
                    hunk->Reloc16D = scan + 1;
                    /* fall through */
                case 0x3F7:             /*  HUNK_RELOC32-D  */
                    if (FromMsbOrder(*scan) == 0x3F7)
                        hunk->Reloc32D = scan + 1;
                    /* fall through */
                case 0x3F9:             /*  HUNK_RELOC8-D   */
                    if (FromMsbOrder(*scan) == 0x3F9)
                        hunk->Reloc8D = scan + 1;
                    /* fall through */
                case 0x3EC:             /*  HUNK_RELOC32    */
                    if (FromMsbOrder(*scan) == 0x3EC)
                        hunk->Reloc32 = scan + 1;
                    /* fall through */
                case 0x3ED:             /*  HUNK_RELOC16    */
                    if (FromMsbOrder(*scan) == 0x3ED)
                        hunk->Reloc16 = scan + 1;
                    /* fall through */
                case 0x3EE:             /*  HUNK_RELOC8     */
                    if (FromMsbOrder(*scan) == 0x3EE)
                        hunk->Reloc8 = scan + 1;
                    ++scan;
                    while (FromMsbOrder(*scan))
                        scan += FromMsbOrder(*scan) + 2;
                    ++scan;
                    break;
                case 0x3EF:             /*  HUNK_EXT        */
                    hunk->Ext = scan + 1;
                    /* fall through */
                case 0x3F0:             /*  HUNK_SYMBOL     */
                    if (FromMsbOrder(*scan) == 0x3F0)
                        hunk->Sym = scan + 1;
                    ++scan;
                    while ((len = FromMsbOrder(*scan)) != 0) {
                        ubyte type = len >> 24;

                        len &= 0x00FFFFFF;
                        scan += len + 1;

                        switch(type) {
                        case 0:         /*  SYMB    */
                        case 1:         /*  DEF     */
                        case 2:         /*  ABS     */
                        case 3:         /*  RES     */
                            if (type != 2)
                                ++hunk->ExtDefs;
                            ++scan;     /*  skip value  */
                            break;
                        case 130:       /*  COMMON  */
                            /*fatal("COMMON symbol not supported");*/
                            /*++hunk->ExtDefs;*/
                            ++scan;     /*  skip common size */
                            break;
                        case 135:       /*  REF32-pc    */
                        case 129:       /*  REF32       */
                        case 131:       /*  REF16-pc    */
                        case 132:       /*  REF8        */
                        case 134:       /*  REF16D      */
                                        /*  skip relocation info */
                            scan += FromMsbOrder(scan[0]) + 1;
                            break;
                        }
                    }
                    ++scan;     /*  skip 0 terminator */
                    break;
                case 0x3F1:             /*  HUNK_DEBUG      */
                    if (((DBInfo *)scan)->di_LINE == LINE)
                        hunk->DbInfo = (DBInfo *)scan;
                    scan += FromMsbOrder(scan[1]) + 2;
                    break;
                }
            }
            ++scan;     /*  skip hunk_end   */
            hunk->TotalBytes = hunk->Bytes;
            if (hunk->Node.ln_Type == 0)
                cerror(EERROR_NO_CODE_DATA_BSS, hunk->Node.ln_Name, hunk->Module->Node.ln_Name);
        }
        fn->DPtr = scan;
        if (hunkNo < mod->NumHunks)
            cerror(EFATAL_TOO_FEW_HUNKS);
        if (hunkNo > mod->NumHunks)
            cerror(EFATAL_TOO_MANY_HUNKS);
        mod->ModEnd = (char *)fn->DPtr;
    }

    return(mod);
}

/*
 *  run SCAN_RELOC_SYMIN on all external symbol tables for all hunks in
 *  this module.
 *
 *  returns number of newly undefined symbols that have been created
 *  (referencing symbols that are already undefined ... previously
 *  referenced and still not defined, do not count in the count)
 */

int
CreateSymbolTable(mod)
Module *mod;
{
    short i;
    int32_t r = 0;

    for (i = 0; i < mod->NumHunks; ++i)
        r += ScanHunkExt(mod->Hunks[i], SCAN_RELOC_SYMIN);
    return(r);
}

/*
 *  returns true if this module, part of a library, contains definitions
 *  used by previously loaded modules
 */

int
ScanLibForInclusion(mod)
Module *mod;
{
    short i;

    for (i = 0; i < mod->NumHunks; ++i) {
        if (ScanHunkExt(mod->Hunks[i], SCAN_RELOC_SYMCK))
            return(1);
    }
    return(0);
}

/*
 *  This is the great coagulator... hlist begins empty.  We allocate a
 *  master HunkListNode for each 'hunk' in the final output, adding hunks
 *  that appear in the module to the appropriate HunkListNode.
 *
 *  We can never combine hunks with different HunkId's (including the
 *  upper bits)
 *
 *  We must also keep sections in order.
 *
 *  Note that only hunks with the same HunkListNode may run PC relative
 *  references to each other.
 */

void
CreateHunkListNodes(mod, hlist)
Module *mod;
List *hlist;
{
    short i;

    for (i = 0; i < mod->NumHunks; ++i) {
        Hunk *hunk;
        Hunk *h;
        HunkListNode *hn;

        hunk = mod->Hunks[i];

        /*
         *  Search for the hunk type & name amoung HunkListNode type and names.
         *  If not found, append a new HunkListNode.   Give HunkListNode's
         *  priorities:
         *                  64  NT_CODE
         *                  32  NT_DATA
         *                  16  NT_BSS
         */

        for (hn = GetHead(hlist); hn; hn = GetSucc(&hn->Node)) {
            if (hn->HunkId != hunk->HunkId)
                continue;
#ifdef NOTDEF
                /*
                 *  if data or bss
                 */
                if ((uword)hn->HunkId == (uword)hunk->HunkId) {
                    if (hn->Node.ln_Type == NT_DATA || hn->Node.ln_Type == NT_BSS) {
                        if (ResOpt) {
                            cerror(EWARN_CANNOT_COAG_DATABSS);
                            ResOpt = 0;
                        }
                    }
                }
                continue;
            }
#endif
            /*
             *  Ignore section names for -r or -pi/-pr
             */

            if (PIOpt || ResOpt)
                break;

            /*
             *  Collect if the same section name
             */

            if (strcmp(hn->Node.ln_Name, hunk->Node.ln_Name) == 0)
                break;

            /*
             *  collection of differently named hunks only works if -frag
             *  is not specified AND the hunks are normal hunks (no upper
             *  bits set)
             *
             *  hunk names that begin with 'far' have a dummy upper bit set
             */

            if (FragOpt == 0 && (hunk->HunkId & 0xFFFF0000) == 0)
                break;
        }
        if (hn == NULL) {
            hn = zalloc(sizeof(HunkListNode));
            hn->Node.ln_Type = hunk->Node.ln_Type;
            hn->Node.ln_Name = hunk->Node.ln_Name;
            hn->HunkId = hunk->HunkId;

            switch(hn->Node.ln_Type) {
            case NT_CODE:
                hn->Node.ln_Pri = 64;
                break;
            case NT_DATA:
                hn->Node.ln_Pri = 32;
                break;
            case NT_BSS:
                hn->Node.ln_Pri = 16;
                break;
            default:
                hn->Node.ln_Pri = 8;
                break;
            }
            Enqueue(hlist, &hn->Node);
            NewList(&hn->HunkList);
        }
        /*
         *  Put the hunk after other hunks of the same name.
         */

        for (h = GetHead(&hn->HunkList); h; h = GetSucc(&h->Node)) {
            if (strcmp(h->Node.ln_Name, hunk->Node.ln_Name) == 0)
                break;
        }
        while (h && strcmp(h->Node.ln_Name, hunk->Node.ln_Name) == 0)
            h = GetSucc(&h->Node);

        if (h)                  /*  insert after last name that compared */
            h = GetPred(&h->Node);      /*  or head of list */
        else                    /*  couldn't find anything, append to end */
            h = GetTail(&hn->HunkList);

        Insert(&hn->HunkList, &hunk->Node, &h->Node);
        hunk->HL = hn;
        hunk->HX = hn;
        hn->FinalExtDefs += hunk->ExtDefs;
    }
}

/*
 *  If we are not running fragmented this routine places all BSS sections
 *  after DATA sections.  Currently this is only done for unqualified
 *  hunks (e.g. that can go into any type of memory).
 *
 *  e.g. this routine combines the DATA and BSS sections
 *
 *  note that due to enqueue priority DATA sections will come before BSS
 *  sections.
 *
 *  note that any section name beginning with 'far' has a HUNKIDF_FLAG
 *  set in its HunkId so it will not be improperly combined.
 */

void
FinalCombineHunkListNodes(hlist)
List *hlist;
{
    HunkListNode *hn;
    HunkListNode *hn_next;
    HunkListNode *hndata = NULL;
    Hunk *h;

    if (FragOpt)
        return;
    for (hn = GetHead(hlist); hn; hn = hn_next) {
        hn_next = GetSucc(&hn->Node);
        if (hn->Node.ln_Type == NT_DATA && (hn->HunkId & 0xFFFF0000) == 0) {
            hndata = hn;
            for (h = (Hunk *)GetHead(&hn->HunkList); h; h = (Hunk *)GetSucc(&h->Node)) {
                h->Flags |= HF_SMALLDATA;
            }
        }

        if (hn->Node.ln_Type == NT_BSS && (hn->HunkId & 0xFFFF0000) == 0) {
            if (hndata) {
                Remove(&hn->Node);
                while ((h = (Hunk *)RemHead(&hn->HunkList)) != NULL) {
                    AddTail(&hndata->HunkList, &h->Node);
                    h->HL = hndata;
                    h->HX = hndata;
                    h->Flags |= HF_DATABSS | HF_SMALLDATA;
                    hndata->FinalExtDefs += h->ExtDefs;
                }
            } else {
                for (h = (Hunk *)GetHead(&hn->HunkList); h; h = GetSucc(&h->Node))
                    h->Flags |= HF_DATABSS;
                hn->Node.ln_Type = NT_DATA;
                hn->HunkId = 0x3EA;
                hndata = hn;
            }
        }
    }
}


/*
 *  This routine fixes the symbol table for BSS sections combined with DATA
 *  sections.  Note that if we are Frag'd there are no combined BSS sections.
 *
 *  Only unqualified sections are considered.  e.g. a CHIP ram section is not
 *  considered.  qualified sections are assumed to be referenced with far
 *  declarations.
 *
 *  It is still possible to have some far data & bss hunks.  The combine
 *  only applies to the first two.
 */

void
FixInternalSymbols(list)
List *list;
{
    HunkListNode *data = NULL;
    HunkListNode *bss  = NULL;
    HunkListNode *code = NULL;
    Hunk *hunk;

    if (FragOpt)
        return;

    {
        HunkListNode *hn;
        for (hn = GetHead(list); hn; hn = GetSucc(&hn->Node)) {
            if ((hn->HunkId & 0xFFFF0000) == 0) {
                if (hn->Node.ln_Type == NT_CODE)
                    code = hn;
                if (hn->Node.ln_Type == NT_DATA) {
                    if (data == NULL)
                        data = hn;
                    else
                        cerror(EFATAL_556);
                }
                if (hn->Node.ln_Type == NT_BSS) {
                    if (bss == NULL)
                        bss = hn;
                    else
                        cerror(EFATAL_562);
                }
            }
        }
    }

    if (data == NULL)   /*  no data anyway  */
        return;
    if (bss && !FragOpt)
        cerror(EFATAL_571);

    hunk = GetHead(&data->HunkList);
    if (hunk == NULL)
        cerror(EFATAL_DATA_HUNK_EMPTY);

    BssLenSym->Hunk = hunk;
    BssLenSym->Value= data->AddSize >> 2;

    if (SymOpt) {
        AddTail(&hunk->SymList, (Node *)&BssLenSym->Node);
        BssLenSym->Flags |= SYMF_SYMLIST;
    }

    if (AbsWordOpt || PIOpt) {
        /*
         *  AbsWordOpt: __ABSOLUTE_BAS address already set to absolute value
         */
        if (code && (hunk = GetTail(&code->HunkList))) {
            DataBasSym->Type = 1;
            DataBasSym->Hunk = hunk;
            DataBasSym->Value= /* hunk->Offset + */ hunk->TotalBytes;

            /*
             *  position independant option, __ABSOLUTE_BAS and __DATA_BAS
             *  point to the same thing and reside in code hunk so can be
             *  accessed PC-relative
             */

            if (PIOpt) {
                AbsoluteBasSym->Type = 1;
                AbsoluteBasSym->Hunk = hunk;
                AbsoluteBasSym->Value= /* hunk->Offset + */ hunk->TotalBytes;
            }
        }
    } else {
        /*
         *  must set __ABSOLUTE_BAS address to location of real data, but
         *  only if the symbol exists (e.g. ResOpt == 0)
         */
        if (AbsoluteBasSym) {
            AbsoluteBasSym->Type = 1;
            AbsoluteBasSym->Hunk = hunk;
        }

        DataBasSym->Type = 1;
        DataBasSym->Hunk = hunk;
    }
    DataLenSym->Value = data->FinalSize >> 2;
    if (SymOpt) {
        AddTail(&hunk->SymList, (Node *)&DataBasSym->Node);
        AddTail(&hunk->SymList, (Node *)&DataLenSym->Node);
        DataBasSym->Flags |= SYMF_SYMLIST;
        DataLenSym->Flags |= SYMF_SYMLIST;

        if (AbsoluteBasSym) {
            AddTail(&hunk->SymList, (Node *)&AbsoluteBasSym->Node);
            AbsoluteBasSym->Flags |= SYMF_SYMLIST;
        }
    }

    if (ResOpt)
        IsResSym->Value = 1;
}

/*
 *  Position Independant option combine, Just before final output combine
 *  all hunks into a single CODE hunk.  However, offsets for DATA & BSS
 *  hunks are NOT modified because DATA & BSS is still referenced A4-REL.
 *
 *  There can be no 32 bit relocations
 */

void
PIOptCombineIntoCode(list)
List *list;
{
    HunkListNode *hn;
    HunkListNode *ht;
    Hunk *hunk;

    hn = (HunkListNode *)RemHead(list);
    if (hn == NULL)
        return;
    if (hn->Node.ln_Type != NT_CODE) {
        cerror(EERROR_PI_EXPECTED_CODE);
        return;
    }
    while ((ht = (HunkListNode *)RemHead(list)) != NULL) {
        if (ht->Node.ln_Type == NT_CODE) {
            cerror(EERROR_PI_EXPECTED_ONE_CODE);
            return;
        }
        while ((hunk = (Hunk *)RemHead(&ht->HunkList)) != NULL) {
            if (hunk->Flags & HF_DATABSS)
                hn->AddSize += (hunk->TotalBytes + 3) & ~3;
            else
                hn->FinalSize += (hunk->TotalBytes + 3) & ~3;
            AddTail(&hn->HunkList, &hunk->Node);
        }
    }
    NumExtHunks = 1;
    AddHead(list, &hn->Node);
}

