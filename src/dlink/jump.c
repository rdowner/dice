/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  JUMP.C
 *
 *  Deal with making a jump table to handle PC-relative code >+/-32K rel offset.
 *
 *  Consider only HUNK_CODE lists
 */

#include "defs.h"

Prototype int	HandleJumpTable(List *);
Prototype int	HunkExtSymJUMP(Hunk *, ubyte, ulong, ulong *);

int
HandleJumpTable(list)
List *list;
{
    HunkListNode *hl;
    Hunk *hunk;
    int hadToAddTable = 0;

    for (hl = GetHead(list); hl; hl = GetSucc(&hl->Node)) {
	if (hl->Node.ln_Type != NT_CODE)
	    continue;
	for (hunk = GetHead(&hl->HunkList); hunk; hunk = GetSucc(&hunk->Node)) {
	    dbprintf(4, ("jump hunk %08lx %s\n", hunk, HunkToStr(hunk)));
	    hadToAddTable += ScanHunkExt(hunk, SCAN_RELOC_JUMP);
	}
    }
    return(hadToAddTable);
}


/*
 *  JUMP
 *
 *  This routine returns a dummy 0 always.  This routine checks 16 bit PC-rel
 *  relocations for validity.  Any failures force a jump table hunk to be added
 *  after this hunk with the PC-Rel symbol redirected to it.  The symbol name
 *  is changed to:
 *			%xxx	where xxx is a unique identifier.
 *
 *  And said symbol is created under the new hunk.
 */

int
HunkExtSymJUMP(Hunk *hunk, ubyte type, ulong len, ulong *scan)
{
    Sym *sym;
    Hunk *destHunk;
    long n;
    ulong *newScan;
    short didJmp = 0;

    /*
     *	ignore other types and only deal with resolved relocations.  Only deal
     *	with PC-rel refs to a standard XDEF.
     *
     *	Any PC-rel refs that are either out of range or to a differently named
     *	hunk are changed to jump table entries.  If we are forced to change
     *	one rel call to a jump table we change them all
     */

    if (type != 131)
	return(0);
    if ((sym = FindSymbol(scan, len)) == NULL || (destHunk = sym->Hunk) == NULL || sym->Type != 1)
	return(0);
    if (destHunk->Node.ln_Type != NT_CODE) {
	if (AbsWordOpt == 0) {
	    cerror(EERROR_PCREL_RELOC_TO_DATA,
		hunk->Module->Node.ln_Name, hunk->Node.ln_Name,
		sym->SymLen, sym->SymName
	    );
	}
	return(0);
    }

    if (destHunk->HX != hunk->HL)
	didJmp = 1;

    newScan = scan + len;

    if (didJmp == 0 && (n = FromMsbOrder(*newScan))) {
	char *dbase = (char *)hunk->Data;

	for (++newScan; n--; ++newScan) {
	    ulong doff = FromMsbOrder(*newScan);
	    long pcrel;
	    uword dcontents = FromMsbOrderShort(*(uword *)(dbase + doff));

	    if (dcontents) {	/*  XXX  */
		dbprintf(0, ("DCONTENTS != 0 in PC-Rel access\n"));
		return(0);
	    }

	    pcrel = (destHunk->Offset + sym->Value + dcontents) - (hunk->Offset + doff);
	    if (pcrel < -32768 || pcrel > 32767) {
		if (hunk == destHunk)
		    cerror(EFATAL_PC_REL_RANGE);
		didJmp = 1;
		break;
	    }
	}
    }
    if (didJmp == 0)
	return(0);

    /*
     *	Create Jump Table Entry.
     *
     *	(1) manually relocate PC-relative jumps
     *	(2) change symbol from ext_ref16 to <reserved> and modify offsets
     *	    to beyond end of data (table created in final phase).
     */

    dbprintf(0, ("Gen Jump Table %s to symbol %.*s\n", HunkToStr(hunk), sym->SymLen, sym->SymName));

    newScan = scan + len;
    n = FromMsbOrder(*newScan);

    for (++newScan; n--; ++newScan) {
	ulong doff = FromMsbOrder(*newScan);	/*  offset in hunk  */
	long  rv = hunk->TotalBytes - doff;

	if (rv < -32768 || rv > 32767)
	    cerror(EFATAL_PC_REL_RANGE);

	if (doff > hunk->Bytes - 2)
	    cerror(EFATAL_RANGE_HUNK, HunkToStr(hunk), doff);
	else
	    *(short *)((char *)hunk->Data + doff) = ToMsbOrderShort(rv);

	/*
	 * change to jump table entry
	 */

	*newScan = ToMsbOrder(hunk->TotalBytes + 2);
    }
    {
	int n = hunk->TotalBytes - hunk->Bytes;

	hunk->JmpData = realloc(hunk->JmpData, n + 8);
	if (hunk->JmpData == NULL)
	    NoMemory();
	setmem((char *)hunk->JmpData + n, 8, 0);
	*(uword *)((char *)hunk->JmpData + n) = ToMsbOrderShort(0x4EF9);
    }
    scan[-1] = ToMsbOrder((FromMsbOrder(scan[-1]) & 0x00FFFFFF) | (RESERVED_PCJMP_TYPE << 24));
    hunk->TotalBytes += 6;		  /*  'allocate' space for jmp  */
    return(1);
}

