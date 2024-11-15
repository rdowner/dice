/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  RELOC.C
 *
 *
 *  hunk usually points to the hunk needing parts of itself
 *  relocated.
 *
 *  HL points to the HunkListNode that the data is actually defined
 *     in.
 *
 *  HX points to the HunkListNode... the final HunkNo that we will
 *     use in the relocation list.
 */

/*
**      $Filename: reloc.c $
**      $Author: dice $
**      $Revision: 30.6 $
**      $Date: 1994/08/04 04:50:55 $
**      $Log: reloc.c,v $
 * Revision 30.6  1994/08/04  04:50:55  dice
 * .
 *
 * Revision 30.5  1994/06/13  18:38:37  dice
 * byte ordering portability
 *
 * Revision 30.0  1994/06/10  18:05:40  dice
 * .
 *
 * Revision 1.2  1993/10/13  19:48:08  jtoebes
 * Fix BUG01125 - Enforce hit in Dlink.
 *
**/

#include "defs.h"

Prototype void  copy_reloc(uint32_t *, uint32_t *, int32_t, int32_t);
Prototype void  ScanHunkReloc32(Hunk *, int32_t);
Prototype void  ScanHunkReloc8_16(uint32_t *, Hunk *, int32_t);
Prototype void  ScanHunkReloc8(Hunk *);
Prototype void  ScanHunkReloc16(Hunk *);
Prototype void  ScanHunkRelocD8(Hunk *);
Prototype void  ScanHunkRelocD16(Hunk *);
Prototype void  ScanHunkRelocD32(Hunk *);
Prototype int   ScanHunkExt(Hunk *, int32_t);
Prototype int   HunkExtSymCK(Hunk *, ubyte, uint32_t, uint32_t *);
Prototype int   HunkExtSymIN(Hunk *, ubyte, uint32_t, uint32_t *);
Prototype int   HunkExtSymCNT(Hunk *, ubyte, uint32_t, uint32_t *);
Prototype int   HunkExtSymRUN(Hunk *, ubyte, uint32_t, uint32_t *);
Prototype int   HunkExtSymComResolve(Hunk *, ubyte, uint32_t, uint32_t *);
Prototype void  IllegalHunk(short, Hunk *);

/*
 *  Copy relocation information to the final relocation array, modifying it
 *  as we go along.  When hunks are combined together the relocation info
 *  for them must also be modified to point to the new location of the hunk
 *  in the section.
 */

void
copy_reloc(s, d, nlw, off)
uint32_t *s;
uint32_t *d;
int32_t nlw;
int32_t off;
{
    if (off == 0) {
        movmem(s, d, nlw * 4);
        return;
    }
    while (nlw--) {
        *d = ToMsbOrder(FromMsbOrder(*s) + off);
        ++s;
        ++d;
    }
}

void
ScanHunkReloc32(hunk, copy)
Hunk *hunk;
int32_t copy;
{
    uint32_t *scan = hunk->Reloc32;

    if (scan == NULL)
        return;

    Assert(hunk->Module);

    while (FromMsbOrder(*scan)) {
        Hunk *destHunk = hunk->Module->Hunks[FromMsbOrder(scan[1])];
        int32_t hunkNo;

        if ((uint32_t)FromMsbOrder(scan[1]) >= hunk->Module->NumHunks)
            IllegalHunk(EF_FATAL, NULL);
        if (destHunk->Node.ln_Type != NT_BSS && destHunk->Node.ln_Type != NT_CODE && destHunk->Node.ln_Type != NT_DATA)
            IllegalHunk(EF_FATAL, destHunk);
        if (PIOpt)
            cerror(EERROR_RELOC32_ILLEGAL_PI, hunk->Module->Node.ln_Name, hunk->Node.ln_Name);
        if (ResOpt && !AbsWordOpt && (destHunk->Node.ln_Type == NT_BSS || destHunk->Node.ln_Type == NT_DATA))
            cerror(EERROR_RELOC32_DATABSS_RES, hunk->Module->Node.ln_Name, hunk->Node.ln_Name);

        hunkNo = destHunk->HX->FinalHunkNo;
        if (hunkNo < 0 || hunkNo >= NumExtHunks)
            cerror(EFATAL_RELOCSCAN_HUNK_RANGE, hunkNo, NumExtHunks);
        if (copy == 0) {                        /*  just count # of relocs */
            hunk->HL->CntReloc32[hunkNo] += FromMsbOrder(*scan);
        } else {                                /*  actually copy stuff    */
            {
                int32_t bas = hunk->HL->CpyReloc32[hunkNo];
                int32_t fnl = bas + FromMsbOrder(*scan);
                if (fnl > hunk->HL->CntReloc32[hunkNo])
                    cerror(EFATAL_RELOC_NOEXIST_HUNK, fnl, hunk->HL->CntReloc32[hunkNo]);
                copy_reloc(scan + 2, hunk->HL->ExtReloc32[hunkNo] + bas, FromMsbOrder(*scan), hunk->Offset);
                hunk->HL->CpyReloc32[hunkNo] = fnl;
            }

            /*
             *  Handle fixing up the base offset.
             */

            Assert(hunk->Module);

            if (FromMsbOrder(scan[0])) {
                char *dbase = (char *)hunk->Data;
                uint32_t dlen = hunk->Bytes;
                Hunk *destHunk = hunk->Module->Hunks[FromMsbOrder(scan[1])];
                int32_t n;
                int32_t destOff;
                uint32_t *newScan;

                if ((uint32_t)FromMsbOrder(scan[1]) >= hunk->Module->NumHunks)
                    IllegalHunk(EF_FATAL, NULL);
                if (destHunk->Node.ln_Type != NT_BSS && destHunk->Node.ln_Type != NT_CODE && destHunk->Node.ln_Type != NT_DATA)
                    IllegalHunk(EF_FATAL, destHunk);

                destOff = destHunk->Offset;
                if (destHunk->HL != destHunk->HX)
                    destOff += destHunk->HX->FinalSize;

                if (hunk->Node.ln_Type == NT_BSS)
                    cerror(EFATAL_RELOC_BSS_ILLEGAL);

                newScan = scan + 2;     /*  base of offsets */
                n = FromMsbOrder(*scan);/*  # of offsets    */

                while (n--) {
                    uint32_t doff = FromMsbOrder(*newScan);
                    int32_t rval;

                    dbprintf(2, ("mod --- reloc32 %s@%ld(%ld) to %s@%ld\n", HunkToStr(hunk), doff, FromMsbOrder(*(uint32_t *)(dbase + doff)), HunkToStr(destHunk), destOff));

                    if (doff >= dlen)
                        cerror(EFATAL_RANGE_HUNK, doff, HunkToStr(hunk));
                    rval = destOff + FromMsbOrder(*(uint32_t *)(dbase + doff));
                    *(uint32_t *)(dbase + doff) = ToMsbOrder(rval);
                    ++newScan;
                }
            }
        }
        scan += FromMsbOrder(scan[0]) + 2;
    }
}

/*
 *  This runs actual 8 and 16 bit PC relative relocations.
 *       runs actual 16 bit DATA relative relocations
 */

void
ScanHunkReloc8_16(scan, hunk, bits)
uint32_t *scan;
Hunk *hunk;
int32_t bits;
{
    char *dbase;
    int32_t dlen;

    if (scan == NULL)
        return;

    dbase = (char *)hunk->Data;
    dlen = hunk->Bytes;

    Assert(hunk->Module);

    while (FromMsbOrder(*scan)) {
        Hunk *destHunk = hunk->Module->Hunks[FromMsbOrder(scan[1])];
        int32_t hunkNo;
        int32_t n;
        int32_t destOff;
        short absWord = 0;

        if ((uint32_t)FromMsbOrder(scan[1]) >= hunk->Module->NumHunks)
            IllegalHunk(EF_FATAL, NULL);
        if (destHunk->Node.ln_Type != NT_BSS && destHunk->Node.ln_Type != NT_CODE && destHunk->Node.ln_Type != NT_DATA)
            IllegalHunk(EF_FATAL, destHunk);
        hunkNo = destHunk->HX->FinalHunkNo;
        if (hunkNo < 0 || hunkNo >= NumExtHunks)
            cerror(EFATAL_RELOCSCAN_HUNK_RANGE, hunkNo, NumExtHunks);

        destOff = destHunk->Offset;
        if (destHunk->HL != destHunk->HX)
            destOff += destHunk->HX->FinalSize;

        /*
         *  modify hunk->Data
         */

        if (bits > 0) {     /*  PC-RELATIVE RELOCATION  */
            if (hunk->Node.ln_Type == NT_BSS) {
                cerror(EERROR_RELOCPCREL_BSS_ILLEGAL);
                return;
            }
            if (hunk->HL != destHunk->HX) {
                if (AbsWordOpt && hunk->Node.ln_Type == NT_CODE) {
                    absWord = 1;
                    dbprintf(0, ("Absolute Word Relocation"));
                } else {
                    cerror(EERROR_RELOCPCREL_ILLEGAL,
                        hunk->Module->Node.ln_Name, hunk->Node.ln_Name,
                        destHunk->Module->Node.ln_Name, destHunk->Node.ln_Name
                    );
                    return;
                }
            }
        } else {            /*  DATA BASE RELATIVE RELOCATION   */
            if (destHunk->Node.ln_Type == NT_CODE) {
                cerror(EERROR_RELOC_LABREL_CODE_ILL);
                return;
            }
            /* XXX warning msg if not to same data segment! */
        }

        n = FromMsbOrder(*scan);  /*    # of relocs         */
        scan += 2;  /*  beginning of relocs */

        while (n--) {
            uint32_t doff = FromMsbOrder(*scan); /* offset into this hunk's data */
            int32_t pcrel;

            if (doff >= dlen) {
                cerror(EERROR_RANGE_HUNK, doff);
                return;
            }
            switch(bits) {
            case 8:
                dbprintf(2, ("mod --- reloc08 %s@%ld(%ld) to %s\n", HunkToStr(hunk), doff, *(ubyte *)(dbase + doff), HunkToStr(destHunk)));
                pcrel = (destOff + *(char *)(dbase + doff)) - (hunk->Offset + doff);
                if (pcrel < -128 || pcrel > 127)
                    cerror(EERROR_RELOC8_RANGE, pcrel, HunkToStr(hunk));
                *(ubyte *)(dbase + doff) = pcrel;
                break;
            case -8:       /*  8 bit data relative    */
                dbprintf(2, ("mod --- reloD08 %s@%ld(%ld) to %s\n", HunkToStr(hunk), doff, *(ubyte *)(dbase + doff) - 32766, HunkToStr(destHunk)));
                pcrel = (destOff + *(char *)(dbase + doff)) - 32766;
                if (pcrel < -128 || pcrel > 127)
                    cerror(EERROR_RELOC8_RANGE, pcrel, HunkToStr(hunk));
                *(ubyte *)(dbase + doff) = pcrel;
                break;
            case 16:
                dbprintf(2, ("mod --- reloc16 %s@%ld(%ld) to %s\n", HunkToStr(hunk), doff, FromMsbOrderShort(*(short *)(dbase + doff)), HunkToStr(destHunk)));
                if (absWord)
                    pcrel = (destOff + FromMsbOrderShort(*(short *)(dbase + doff))) + WordBaseAddr;
                else
                    pcrel = (destOff + FromMsbOrderShort(*(short *)(dbase + doff))) - (hunk->Offset + doff);
                if (pcrel < -32768 || pcrel > 32767)
                    cerror(EERROR_RELOC16_RANGE, pcrel, HunkToStr(hunk));
                *(short *)(dbase + doff) = ToMsbOrderShort(pcrel);
                break;
            case -16:       /*  16 bit data relative    */
                dbprintf(2, ("mod --- reloD16 %s@%ld(%ld) to %s\n", HunkToStr(hunk), doff, FromMsbOrderShort(*(short *)(dbase + doff)) - 32766, HunkToStr(destHunk)));
                pcrel = (destOff + FromMsbOrderShort(*(short *)(dbase + doff))) - 32766;
                if (pcrel < -32768 || pcrel > 32767)
                    cerror(EERROR_RELOC16_RANGE, pcrel, HunkToStr(hunk));
                *(short *)(dbase + doff) = ToMsbOrderShort(pcrel);
                break;
            case -32:       /*  32 bit data relative    */
                dbprintf(2, ("mod --- reloD32 %s@%ld(%ld) to %s\n", HunkToStr(hunk), doff, FromMsbOrder(*(int32_t *)(dbase + doff)) - 32766, HunkToStr(destHunk)));
                pcrel = (destOff + FromMsbOrder(*(int32_t *)(dbase + doff))) - 32766;
                *(int32_t *)(dbase + doff) = ToMsbOrder(pcrel);
                break;
            }
            ++scan;
        }
    }
}

void
ScanHunkReloc8(hunk)
Hunk *hunk;
{
    ScanHunkReloc8_16(hunk->Reloc8, hunk, 8);
}

void
ScanHunkReloc16(hunk)
Hunk *hunk;
{
    ScanHunkReloc8_16(hunk->Reloc16, hunk, 16);
}

void
ScanHunkRelocD8(hunk)
Hunk *hunk;
{
    ScanHunkReloc8_16(hunk->Reloc8D, hunk, -8);
}

void
ScanHunkRelocD16(hunk)
Hunk *hunk;
{
    ScanHunkReloc8_16(hunk->Reloc16D, hunk, -16);
}

void
ScanHunkRelocD32(hunk)
Hunk *hunk;
{
    ScanHunkReloc8_16(hunk->Reloc32D, hunk, -32);
}


/*
 *                              SYMBOLS
 *          -----------------------------------------------------
 */

/*
 *  External Symbol Scan
 *
 *  Count / handle references from the external symbol table.  This functions
 *  returns the sum of all subroutine calls.
 */

int
ScanHunkExt(hunk, cmd)
Hunk *hunk;
int32_t cmd;
{
    uint32_t *scan = hunk->Ext;
    ubyte type;
    uint32_t len;
    uint32_t result = 0;

    if (scan == NULL)
        return(0);

    /*
     *  scan symbol units for hunk.
     */

    while (FromMsbOrder(*scan)) {
        uint32_t *newScan;


        type = FromMsbOrder(*scan) >> 24;
        len  = FromMsbOrder(*scan) & 0x00FFFFFF;
        ++scan;

        switch(cmd) {
        case SCAN_RELOC_SYMCK:
            result += HunkExtSymCK(hunk, type, len, scan);
            break;
        case SCAN_RELOC_SYMIN:
            result += HunkExtSymIN(hunk, type, len, scan);
            break;
        case SCAN_RELOC_CNT:
            result += HunkExtSymCNT(hunk, type, len, scan);
            break;
        case SCAN_RELOC_RUN:
            result += HunkExtSymRUN(hunk, type, len, scan);
            break;
        case SCAN_RELOC_JUMP:
            result += HunkExtSymJUMP(hunk, type, len, scan);
            break;
        case SCAN_COMMON_RESOLVE:
            if (type == 130)
                result += HunkExtSymComResolve(hunk, type, len, scan);
            break;
        }

        newScan = scan + len;       /*  skip symbol name    */

        switch(type) {
        case 0:         /*  symbol table        */
        case 1:         /*  exported reloc def  */
        case 2:         /*  exported abs def    */
        case 3:         /*  exported res lib def*/
            scan = newScan + 1;
            break;
        case 130:       /*  common              */
            scan = newScan + 1;
            break;
        case RESERVED_PCJMP_TYPE:
        case 135:       /*  imported ref32-pc   */
        case 129:       /*  imported ref32      */
        case 131:       /*  imported ref16      */
        case 132:       /*  imported ref8       */
        case 134:       /*  imported ref16D     */
            scan = newScan + FromMsbOrder(newScan[0]) + 1;
            break;
        default:
            cerror(EERROR_UNKNOWN_SYM_TYPE, type, len, scan);
            return(0);
        }
    }
    return((int32_t)result);
}

/*
 *  CK
 *
 *  This is called to check exported (xdef)d symbols in a library module
 *  searching for undefined symbols to see if we have to include the library
 *  or not.  It returns (1) if a previously undefined symbol is matched with
 *  an xdef.
 *
 *  HunkListNode has not been created at this point, so hunk->HL is invalid
 *  as well as hn.  Also, sym->Hunk may be NULL for linker ABS symbols.
 *
 *  COMMON symbols are references for this routine
 */

int
HunkExtSymCK(Hunk *hunk, ubyte type, uint32_t len, uint32_t *scan)
{
    Sym *sym;

    if (type == 1 || type == 2 || type == 3) {
        if ((sym = FindSymbol(scan, len)) != NULL) {
            if (sym->Type == 0)
                return(1);
        }
    }
    return(0);
}

/*
 *  IN
 *
 *  This is called to bring the ref'd and def'd symbols associated with a module
 *  into the master symbol table.  This routine returns (1) If adding the
 *  symbol creates a NEW undefined reference (to force another pass through
 *  the library).
 *
 *  HunkListNode has not been created at this point, so hunk->HL is invalid
 *  as well as hn
 *
 *  COMMON symbols are ignored for this routine
 */

int
HunkExtSymIN(Hunk *hunk, ubyte type, uint32_t len, uint32_t *scan)
{
    Sym *sym;

    if (type == 1 || type == 2 || type == 3) {      /*  def */
        /*
         *  Note that a def symbol may overide a common symbol
         */

        if ((sym = FindSymbol(scan, len)) != NULL) {
            /*
             *  If duplicate def and both hunks are object modules or both
             *  hunks are the *same* library then complain.
             */

            if (sym->Hunk) {        /*  duplicate   */
                Assert(sym->Hunk->Module);
                Assert(hunk->Module);

                if (sym->Type == 130) {
                    /*
                     *  always overide a common symbol
                     */
                    SetSymbol(sym, hunk, FromMsbOrder(scan[len]), type);
                } else if (sym->Hunk->Module->Node.ln_Type == NT_FTOBJ && hunk->Module->Node.ln_Type == NT_FTOBJ) {
                    cerror(EWARN_MULT_DEF_SYM, len*4, scan, sym->Hunk->Module->Node.ln_Name, hunk->Module->Node.ln_Name);
                }
                if (sym->Hunk->Module->Node.ln_Type == NT_FTLIB && hunk->Module->Node.ln_Type == NT_FTLIB && sym->Hunk->Module->FNode == hunk->Module->FNode)
                    cerror(EWARN_MULT_DEF_SYM_LIB, len*4, scan);
            } else {                /*  first def   */
                SetSymbol(sym, hunk, FromMsbOrder(scan[len]), type);
                dbprintf(1, ("SetSym type %-3d value %-3d %.*s\n", type, sym->Value, sym->SymLen, sym->SymName));
            }
        } else {
            CreateSymbol(scan, len, hunk, FromMsbOrder(scan[len]), type);
        }
    } else if (type == 129 || type == 131 || type == 132 || type == 134 || type == 135) {     /*  ref */
        if ((sym = FindSymbol(scan, len)) != NULL) {
            ++sym->Refs;                        /*  def'd or undef'd    */
        } else {
            sym = CreateSymbol(scan, len, NULL, 0, 0);    /*  new undef'd         */
            ++sym->Refs;
            return(1);
        }
    } else if (type == 130) {
        /*
         *  A common symbol is ignored if it already exists, else it is
         *  entered.
         */

        if ((sym = FindSymbol(scan, len)) == NULL) {
            CreateSymbol(scan, len, hunk, FromMsbOrder(scan[len]), type);
            /*printf("Create common %d\n", type);*/
        } else {
            if (sym->Type == 0) {
                SetSymbol(sym, hunk, FromMsbOrder(scan[len]), type);
                /*printf("OVR common %d\n", type);*/
            } else {
                /*printf("Ignore common %d\n", type);*/
            }
        }
    } else if (type != 0) {
        cerror(EERROR_UNKNOWN_SYM_TYPE, type, len * 4, scan);
    }
    return(0);
}

/*
 *  COMMON_RESOLVE
 *
 *  This routine picks out unresolved common symbols and resolves them.
 *  It returns a dummy 0.  Note that the passed hunk is ignored since
 *  common variables overide later common variables, only sym->Hunk is
 *  valid.
 *
 *  This routine is only called with type == 130
 */

int
HunkExtSymComResolve(Hunk *hunk, ubyte type, uint32_t len, uint32_t *scan)
{
    Sym *sym;
    int32_t size;


    if ((sym = FindSymbol(scan, len)) != NULL) {
        hunk = sym->Hunk;   /*  hunk symbol applies to  */

        if (sym->Type == 130) {
            Assert(sym->Hunk);
            size = (sym->Value + 3) & ~3;

            /*printf("CommonSymbolResolve: offset=%d size=%d %.*s\n", hunk->Bytes, size, len * 4, scan);*/

            SetSymbol(sym, hunk, hunk->Bytes, 1);
            hunk->Bytes += size;
            hunk->TotalBytes += size;
        }
    }
    return(0);
}

/*
 *  CNT
 *
 *  This routine returns a dummy 0 always.  This routine updates the
 *  the HunkListNode's CntReloc32 for 32 bit Reloc32 references.  We are
 *  counting the number of relocs required for this hunk to allocate the master
 *  reloc array.
 *
 *  This routine also handles Reloc16 and Reloc8's
 */

int
HunkExtSymCNT(Hunk *hunk, ubyte type, uint32_t len, uint32_t *scan)
{
    Sym *sym;

    if (type == 129) {
        if ((sym = FindSymbol(scan, len)) != NULL) {
            Hunk *destHunk;
            int32_t hunkNo;

            if (sym->Type == 2)
                return(0);

            if ((destHunk = sym->Hunk) != NULL) {
                hunkNo = destHunk->HX->FinalHunkNo;
                if (hunkNo < 0 || hunkNo >= NumExtHunks)
                    cerror(EFATAL_RELOC_NOEXIST_HUNK, hunkNo, NumExtHunks);
                if (sym->Type == 1)     /*  32 bit relocatable def */
                    hunk->HL->CntReloc32[hunkNo] += FromMsbOrder(scan[len]);
                return(0);
            }
        }
        cerror(EERROR_UNDEF_SYM, len*4, scan, hunk->Module->Node.ln_Name);
    } else if (type == 131 || type == 132 || type == 134 || type == 135) {     /*  16dr,pc 8pc, 32pc    */
        if ((sym = FindSymbol(scan, len)) != NULL) {
            Hunk *destHunk = sym->Hunk;         /*  might be NULL            */
            int32_t hunkNo;
            int32_t n;
            uint32_t *newScan = scan + len;

            if (destHunk) {
                hunkNo = destHunk->HX->FinalHunkNo;
                if (hunkNo < 0 || hunkNo >= NumExtHunks)
                    cerror(EFATAL_RELOC_NOEXIST_HUNK, hunkNo, NumExtHunks);
            }
            if (destHunk || sym->Type == 2) {
                int32_t destOff = 0;

                if (sym->Type != 2) {               /*  DATA symbol */
                    destOff = destHunk->Offset;
                    if (destHunk->HL != destHunk->HX)
                        destOff += destHunk->HX->FinalSize;
                }
                if ((n = FromMsbOrder(*newScan)) != 0) {
                    char *dbase = (char *)hunk->Data;
                    uint32_t dlen = hunk->Bytes;

                    /*
                     *  relocate each 8/16 bit PC relative reference
                     */

                    for (++newScan; n--; ++newScan) {
                        uint32_t doff = FromMsbOrder(*newScan);
                        int32_t pcrel = 0;

                        if (doff >= dlen)   /*  illegal or jump table reloc */
                            continue;
                        switch(type) {
                        case 134:   /*  16 bit data-base relative reloc (NOT pcrel) */
                            dbprintf(2, ("ext sym t=%d reloc16D %s@%ld(%ld) to %s@%ld\n", sym->Type, HunkToStr(hunk), doff, FromMsbOrderShort(*(short *)(dbase + doff)), HunkToStr(destHunk), sym->Value));
                            switch(sym->Type) {
                            case 2:     /*  ABS symbol  */
                                pcrel = FromMsbOrderShort(*(short *)(dbase + doff)) + sym->Value;
                                break;
                            case 1:     /*  DATA symbol */
                                pcrel = (destOff + sym->Value + FromMsbOrderShort(*(short *)(dbase + doff))) - 32766;
                                if ((destHunk->Flags & HF_SMALLDATA) == 0)
                                    cerror(EERROR_A4REL_TO_FAR_OBJECT, sym->SymLen, sym->SymName, HunkToStr(destHunk), HunkToStr(hunk));
                                break;
                            default:
                                cerror(EERROR_RELOC_ILLEGAL_SYM, sym->Type);
                                break;
                            }
                            if (pcrel < -32768 || pcrel > 32767)
                                cerror(EERROR_RELOC16DATA_SYM_RANGE, pcrel, sym->SymLen, sym->SymName, HunkToStr(hunk));
                            *(short *)(dbase + doff) = ToMsbOrderShort(pcrel);
                            break;
                        case 132:   /*  8 bit reloc */
                            dbprintf(2, ("ext sym t=%d reloc08 %s@%ld(%ld) to %s@%ld\n", sym->Type, HunkToStr(hunk), doff, *(char *)(dbase + doff), HunkToStr(destHunk), sym->Value));
                            switch(sym->Type) {
                            case 2:
                                pcrel = *(char *)(dbase + doff) + sym->Value;
                                break;
                            case 1:
                                pcrel = (destOff + sym->Value + *(char *)(dbase + doff)) - (hunk->Offset + doff);
                                break;
                            default:
                                cerror(EERROR_RELOC_ILLEGAL_SYM, sym->Type);
                                break;
                            }
                            if (pcrel < -128 || pcrel > 127)
                                cerror(EERROR_RELOC8_RANGE, pcrel);
                            *(ubyte *)(dbase + doff) = pcrel;
                            break;
                        case 131:   /*  16 bit pc-relative reloc */
                            dbprintf(2, ("ext sym t=%d reloc16pc %s@%ld(%ld) to %s@%ld\n", sym->Type, HunkToStr(hunk), doff, FromMsbOrderShort(*(short *)(dbase + doff)), HunkToStr(destHunk), sym->Value));

                            switch(sym->Type) {
                            case 2:     /*  Absolute Symbol */
                                pcrel = FromMsbOrderShort(*(short *)(dbase + doff)) + sym->Value;
                                break;
                            case 1:     /*  Data Symbol     */
                                if (hunk->HL != destHunk->HX) {
                                    if (AbsWordOpt) {
                                        pcrel = (destOff + sym->Value + FromMsbOrderShort(*(short *)(dbase + doff))) + WordBaseAddr;
                                    } else {
                                        cerror(EERROR_PCREL_RELOC_TO_DATA,
                                            hunk->Module->Node.ln_Name,
                                            hunk->Node.ln_Name,
                                            sym->SymLen, sym->SymName
                                        );
                                    }
                                } else {
                                    pcrel = (destOff + sym->Value + FromMsbOrderShort(*(short *)(dbase + doff))) - (hunk->Offset + doff);
                                }
                                break;
                            default:
                                cerror(EERROR_RELOC_ILLEGAL_SYM, sym->Type);
                                break;
                            }
                            if (pcrel < -32768 || pcrel > 32767)
                                cerror(EERROR_RELOC16DATA_SYM_RANGE, pcrel, sym->SymLen, sym->SymName, HunkToStr(hunk));
                            *(short *)(dbase + doff) = ToMsbOrderShort(pcrel);
                            break;
                        case 135:   /*  32 bit pc-relative reloc */
                            dbprintf(2, ("ext sym t=%d reloc32pc %s@%ld(%ld) to %s@%ld\n", sym->Type, HunkToStr(hunk), doff, FromMsbOrder(*(int32_t *)(dbase + doff)), HunkToStr(destHunk), sym->Value));

                            switch(sym->Type) {
                            case 2:     /*  Absolute Symbol     */
                                pcrel = FromMsbOrder(*(int32_t *)(dbase + doff)) + sym->Value;
                                break;
                            case 1:     /*  std label Symbol    */
                                pcrel = (destOff + sym->Value + FromMsbOrder(*(int32_t *)(dbase + doff))) - (hunk->Offset + doff);
                                break;
                            default:
                                cerror(EERROR_RELOC_ILLEGAL_SYM, sym->Type);
                                break;
                            }
                            *(int32_t *)(dbase + doff) = ToMsbOrder(pcrel);
                            break;
                        default:
                            break;
                        }
                    }
                }
                return(0);
            }
        }
        cerror(EERROR_UNDEF_SYM, len*4, scan, hunk->Module->Node.ln_Name);
    } else if (type == RESERVED_PCJMP_TYPE) {
        if ((sym = FindSymbol(scan, len)) != NULL) {
            Hunk *destHunk;
            int32_t hunkNo;

            if ((destHunk = sym->Hunk) != NULL) {
                hunkNo = destHunk->HX->FinalHunkNo;
                if (hunkNo < 0 || hunkNo >= NumExtHunks)
                    cerror(EFATAL_RANGE_HUNK, hunkNo, NumExtHunks);
                if (sym->Type != 1)     /*  32 bit relocatable def */
                    cerror(EFATAL_PC_REL_ILLEGAL_SYMBOL, sym->Type);
                ++hunk->HL->CntReloc32[hunkNo];
                return(0);
            }
        }
        cerror(EERROR_UNDEF_SYM, len*4, scan, hunk->Module->Node.ln_Name);
    }
    return(0);
}

/*
 *  RUN
 *
 *  This routine returns a dummy value, 0.  This routine modifies the hunk
 *  data appropriately for a reloc32.
 */

int
HunkExtSymRUN(Hunk *hunk, ubyte type, uint32_t len, uint32_t *scan)
{
    Sym *sym;
    Hunk *destHunk;
    int32_t hunkNo = 0;

    if (type != 129 && type != RESERVED_PCJMP_TYPE)
        return(0);
    if ((sym = FindSymbol(scan, len)) == NULL)
        return(0);

    if ((destHunk = sym->Hunk) != NULL) {
        hunkNo = destHunk->HX->FinalHunkNo;
        if (hunkNo < 0 || hunkNo >= NumExtHunks)
            cerror(EFATAL_EXTSYM_RELOC);
    }

    /*
     *  Special case... single relocation jump table beyond end of
     *  hunk->Bytes.
     *
     *  if scan[len] > 1 we actually only do one... see 'jump.c'.  We
     *  converted N pc-relative jumps to external jumps.
     */

    if (type == RESERVED_PCJMP_TYPE) {
        SanityCheck(32);
        if (destHunk == NULL)
            return(0);
        SanityCheck(33);
        if (FromMsbOrder(scan[len])) {              /*  # of relocs */
            int32_t bas = hunk->HL->CpyReloc32[hunkNo];
            int32_t n = FromMsbOrder(scan[len+1]) - hunk->Bytes;     /* first reloc */

            hunk->HL->ExtReloc32[hunkNo][bas] = ToMsbOrder(hunk->Offset + hunk->Bytes + n);
            ++bas;
            hunk->HL->CpyReloc32[hunkNo] = bas;

            if (n < 0 || n >= hunk->TotalBytes - hunk->Bytes)
                cerror(ESOFT_JUMP_TABLE);

            *(int32_t *)((char *)hunk->JmpData + n) = ToMsbOrder(destHunk->Offset + sym->Value);
        }
        SanityCheck(34);
        return(0);
    }

    /*
     *  ABS symbols need no destination hunk.
     */

    if (destHunk == NULL && sym->Type != 2)
        return(0);

    /*
     *  copy relocation info
     */

    SanityCheck(35);
    if (sym->Type == 1) {       /*  only if relocatable def    */
        int32_t bas = hunk->HL->CpyReloc32[hunkNo];
        int32_t fnl = bas + FromMsbOrder(scan[len]);

#ifdef NOTDEF
        printf("HUNK %s hunkno %d vs %d NUMEXT %d\n", HunkToStr(hunk), hunk->HunkNo, hunkNo, NumExtHunks);
        printf("BAS %d FNL %d CNT %d\n", bas, fnl, hunk->HL->CntReloc32[hunkNo]);
        printf("LEN=%d SCAN[LEN]= %d relocs OFF=%d\n", len, scan[len], hunk->Offset);
#endif

        if (fnl > hunk->HL->CntReloc32[hunkNo])
            cerror(EFATAL_EXTSYM_RELOC);
        copy_reloc(scan + len + 1, hunk->HL->ExtReloc32[hunkNo] + bas, FromMsbOrder(scan[len]), hunk->Offset);

        hunk->HL->CpyReloc32[hunkNo] = fnl;
    }

    SanityCheck(36);
    /*
     *  handle relocation
     */

    if (FromMsbOrder(scan[len])) {
        uint32_t n = FromMsbOrder(scan[len]);
        uint32_t *newScan;
        char *dbase = (char *)hunk->Data;
        uint32_t dlen = hunk->Bytes;
        uint32_t destOff = 0;

        if (sym->Type != 2) {
            destOff = destHunk->Offset;
            if (destHunk->HL != destHunk->HX)
                destOff += destHunk->HX->FinalSize;
        }

        if ((ResOpt || PIOpt) && sym->Type != 2) {
            if (destHunk->Node.ln_Type == NT_BSS && !AbsWordOpt)
                cerror(EERROR_ABS_REFS_DATA_BSS, sym->SymLen, sym->SymName);
            if (destHunk->Node.ln_Type == NT_DATA && sym != DataBasSym && !AbsWordOpt)
                cerror(EERROR_ABS_REFS_DATA_BSS, sym->SymLen, sym->SymName);
        }

        for (newScan = scan + len + 1; n--; ++newScan) {
            uint32_t doff = FromMsbOrder(*newScan);
            int32_t rval = 0;

            dbprintf(3, ("ext sym t=%d reloc32 %s@%ld to %s@%ld (%.*s)\n", sym->Type, HunkToStr(hunk), doff, HunkToStr(destHunk), sym->Value, sym->SymLen, sym->SymName));

            if (doff >= dlen)   /*  illegal or jump table */
                continue;
            if (doff & 1)
                cerror(EFATAL_RELOC_WORD_ALIGN);

            switch(sym->Type) {
            default:
                cerror(EFATAL_RELOC_ILLEGAL_SYM, sym->Type);
                break;
            case 2:         /*  abs  */
                rval = FromMsbOrder(*(uint32_t *)(dbase + doff)) + sym->Value;
                break;
            case 1:         /*  def  */
                rval = destOff + FromMsbOrder(*(uint32_t *)(dbase + doff)) + sym->Value;
                break;
            }
            *(uint32_t *)(dbase + doff) = ToMsbOrder(rval);
        }
    }
    SanityCheck(37);
    return(0);
}

void
IllegalHunk(short er_flag, Hunk *hunk)
{
    if (hunk)
        cerror(er_flag | EILLEGAL_HUNK, hunk, hunk->Node.ln_Type, hunk->Module->Node.ln_Name);
    else
        cerror(er_flag | EILLEGAL_HUNK, NULL, -1, "???");
}

