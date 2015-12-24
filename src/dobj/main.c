/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  MAIN.C
 *
 *  DISOBJ [-nc] [-pc<startpc>] [-do<dataorg>] objectfile(s)
 *
 *  Utility to disassemble an object module
 */

#include "defs.h"

#ifdef AMIGA
#include <lib/version.h>
#else
#include <include/lib/version.h>
#endif

Prototype short DDebug;
Prototype short OutAsm;
Prototype short NoCode;
Prototype short NoData;
Prototype short NumHunks;
Prototype int  StartPc;
Prototype int  StartDo;
Prototype ProgramUnit *PUAry[MAXUNITS];

void help(void);
int32_t ScanObjectFile(FILE *fi, int32_t endPos);
void UnAssembleObjectFile(FILE *fi);
void ResetHashTables(void);

IDENT("DOBJ",".3");
DCOPYRIGHT;

short DDebug;
short OutAsm;
short NumHunks;
short NoCode;
short NoData;
ProgramUnit *PUAry[MAXUNITS];

int  StartPc = 0;
int  StartDo = 0;

int
main(int ac, char **av)
{
    short i;

    rel_init();
    sym_init();

    if (ac == 1)
	help();

    for (i = 1; i < ac; ++i) {
	char *ptr = av[i];

	if (*ptr == '-') {
	    ptr += 2;

	    switch(ptr[-1]) {
	    case 'n':
		switch(*ptr) {
		case 'd':
		    NoData = 1;
		    break;
		case 'c':
		    NoCode = 1;
		    break;
		default:
		    help();
		}
		break;
	    case 'a':
		OutAsm = 1;
		break;
	    case 'd':
		if (*ptr == 'o') {
		    StartDo = strtol(ptr + 1, NULL, 0);
		} else {
		    if (*ptr)
			DDebug = atoi(ptr);
		    else
			DDebug = 1;
		}
		break;
	    case 'o':
		if (*ptr == 0)
		    ptr = av[++i];

		freopen(ptr, "w", stdout);
		*ptr = 0;
		break;
	    case 'p':
		if (*ptr == 'c') {
		    StartPc = strtol(ptr + 1, NULL, 0);
		} else {
		    help();
		}
		break;
	    default:
		help();
		break;
	    }
	}
    }

    InitCodeList();

    for (i = 1; i < ac; ++i) {
	char *ptr = av[i];
	FILE *fi;

	if (*ptr == '-')
	    continue;
	if (*ptr == 0)
	    continue;

	if ((fi = fopen(ptr, "r")) != NULL) {
	    int32_t endPos;
	    int32_t pos = 0;

	    fseek(fi, 0L, 2);
	    endPos = ftell(fi);

	    do {
		fseek(fi, pos, 0);
		ResetHashTables();
		pos = ScanObjectFile(fi, endPos);
		UnAssembleObjectFile(fi);
	    } while (pos < endPos);

	    fclose(fi);
	} else {
	    cerror(EERROR, "Unable to open %s", ptr);
	}
    }
    return(0);
}

void
help(void)
{
    printf("%s\n%s\n", Ident, DCopyright);
    puts("DISOBJ objfile(s) [-o outfile] [-d[#]]");
    exit(1);
}

/*
 *  Disassemble a module
 */

int32_t
ScanObjectFile(FILE *fi, int32_t endPos)
{
    int32_t unitData[2];
    short thisHunk = 0;

    while (freadl(unitData, 4, 1, fi) == 1) {
	ProgramUnit *unit;

	if (thisHunk == 0) {
	    int bytes;

	    switch(unitData[0]) {
	    case 0x3E7:
		if (freadl(&bytes, 4, 1, fi) != 1)
		    cerror(EFATAL, "unexpected EOF");
		bytes = bytes * 4;
		printf("HUNK_UNIT ");
		while (bytes) {
		    short c;
		    if ((c = getc(fi)) != 0)
			putc(c, stdout);
		    --bytes;
		}
		puts("");
		break;
	    case 0x3F3:     /*	hunk_header (executable)    */
		printf("HUNK_HEADER ");
		while (freadl(&bytes, 4, 1, fi) == 1 && bytes) {
		    bytes = bytes * 4;
		    printf("(");
		    while (bytes) {
			short c;
			if ((c = getc(fi)) != 0)
			    putc(c, stdout);
			--bytes;
		    }
		    printf(")");
		}
		if (freadl(&bytes, 4, 1, fi) == 1) {     /*  table size  */
		    int begNo;
		    int endNo;

		    printf(" tableSize=%d\n", bytes);

		    freadl(&begNo, 4, 1, fi);
		    freadl(&endNo, 4, 1, fi);
		    while (begNo <= endNo ) {
			int32_t value;
			freadl(&value, 4, 1, fi);
			printf("    hunk #%d  %d bytes\n", begNo, value * 4);
			++begNo;
		    }
		}
		break;
	    default:
		cerror(EFATAL, "Expected HUNK_UNIT (0x3E7), got %08x", unitData[0]);
		break;
	    }
	    freadl(unitData, 4, 1, fi);
	} else {
	    if (unitData[0] == 0x3E7) {     /*  library */
		fseek(fi, -4L, 1);
		break;
	    }
	}

	unit = malloc(sizeof(ProgramUnit));
	clrmem(unit, sizeof(*unit));
	if (unit == NULL)
	    cerror(EFATAL, "malloc failed");

	PUAry[thisHunk] = unit;

	/*
	 *  Scan rest of hunk
	 */

	while (unitData[0] != 0x3F2) {
	    if (DDebug)
		fprintf(stderr, "scan hunk %02x %08x\n", thisHunk, unitData[0]);

	    switch((uword)unitData[0]) {
	    case 0x3E8:     /*	hunk_Name   */
		{
		    int32_t bytes;
		    if (freadl(&bytes, 4, 1, fi) != 1)
			cerror(EFATAL, "unexpected EOF");
		    bytes = bytes * 4;
		    if ((unit->pu_Name = malloc(bytes + 1)) == NULL)
			cerror(EFATAL, "malloc failed");
		    clrmem(unit->pu_Name, bytes + 1);
		    fread(unit->pu_Name, bytes, 1, fi);
		}
		break;
	    case 0x3E9:
	    case 0x3EA:
	    case 0x3EB:
		{
		    int32_t bytes;

		    unit->pu_Type = unitData[0];
		    if (freadl(&bytes, 4, 1, fi) != 1)
			cerror(EFATAL, "Unexpected EOF");
		    unit->pu_Size = bytes * 4;
		    if ((uword)unitData[0] != 0x3EB) {
			unit->pu_Offset = ftell(fi);
			fseek(fi, unit->pu_Size, 1);    /*  skip contents   */
		    }
		}
		break;
	    case 0x3EC:     /*	hunk_reloc32	*/
	    case 0x3ED:     /*	hunk_reloc16	*/
	    case 0x3EE:     /*	hunk_reloc8	*/
	    case 0x3F8:     /*	hunk_reloc16D	*/
		{
		    short size = 4;
		    short flags= 0;
		    int32_t numOffsets;
		    int32_t hunkNo;
		    int32_t offset;

		    switch((uword)unitData[0]) {
		    case 0x3ED:
			size = 2;
			flags |= RF_PCREL;
			break;
		    case 0x3F8:
			size = 2;
			flags |= RF_A4REL;
			break;
		    case 0x3EE:
			size = 1;
			flags |= RF_PCREL;
			break;
		    }
		    while (freadl(&numOffsets, 4, 1, fi) == 1 && numOffsets) {
			if (freadl(&hunkNo, 4, 1, fi) == 1) {
			    while (numOffsets) {
				if (freadl(&offset, 4, 1, fi) != 1)
				    break;
				AddRelocInfo(thisHunk, hunkNo, size, flags, offset, NULL);
				--numOffsets;
			    }
			}
		    }
		}
		break;
	    case 0x3EF:     /*	hunk_ext    */
	    case 0x3F0:     /*	hunk_sym    */
		{
		    uint32_t symHdr;

		    while (freadl(&symHdr, 4, 1, fi) == 1 && symHdr) {
			int32_t bytes = (symHdr & 0x00FFFFFF) * 4;
			int32_t numRefs;
			short size;
			short flags;
			Symbol *sym;

			if ((sym = malloc(sizeof(Symbol) + bytes + 1)) == NULL)
			    cerror(EFATAL, "No Memory");

			clrmem(sym, sizeof(Symbol) + 1 + bytes);
			sym->sm_Type = symHdr >> 24;
			fread(sym->sm_Name, bytes, 1, fi);
			if (sym->sm_Type <= 3)
			    freadl(&sym->sm_Value, 4, 1, fi);
			sym->sm_DefHunk = thisHunk;

			sym = AddSymbol(sym);

			size = 4;
			flags = 0;

			switch(sym->sm_Type) {
			case 0:     /*	symb	*/
			case 1:     /*	def	*/
			case 2:     /*	abs	*/
			case 3:     /*	ref	*/
			    break;
			case 130:   /*	refcomm */
			    freadl(&sym->sm_CommonSize, 4, 1, fi);
			    size = sym->sm_CommonSize;
			    break;
			case 129:   /*	ref32	*/
			case 131:   /*	ref16	*/
			    if (sym->sm_Type == 131) {
				size = 2;
				flags |= RF_PCREL;
			    }
			case 132:   /*	ref8	*/
			    if (sym->sm_Type == 132) {
				size = 1;
				flags |= RF_PCREL;
			    }
			case 134:   /*	ref 16D */
			    if (sym->sm_Type == 134) {
				size = 2;
				flags |= RF_A4REL;
			    }

			    if (freadl(&numRefs, 4, 1, fi) != 1)
				cerror(EFATAL, "Unexpected EOF");
			    while (numRefs) {
				int32_t offset;
				freadl(&offset, 4, 1, fi);
				AddRelocInfo(thisHunk, -1, size, flags, offset, sym);
				--numRefs;
			    }
			}
		    }
		}
		break;
	    case 0x3F1: /*  hunk_debug	*/
		{
		    int32_t bytes;
		    if (freadl(&bytes, 4, 1, fi) == 1) {
			bytes = bytes * 4;
			fseek(fi, bytes, 1);    /*  skip it */
		    }
		}
		break;
	    default:
		cerror(EFATAL, "Unknown hunk type 0x%08lx offset 0x%x", unitData[0], ftell(fi));
		break;
	    }
	    if (freadl(unitData, 4, 1, fi) != 1)
		cerror(EFATAL, "Unexpected EOF");
	}
	++thisHunk;
	if (thisHunk == MAXUNITS)
	    cerror(EFATAL, "Can only support %d UNITs in obj file!", MAXUNITS);
    }
    NumHunks = thisHunk;
    return(ftell(fi));
}

void
UnAssembleObjectFile(FILE *fi)
{
    short srcHunk;

    for (srcHunk = 0; srcHunk < NumHunks; ++srcHunk) {
	ProgramUnit *su = PUAry[srcHunk];

	puts("");
	switch((uword)su->pu_Type) {
	case 0x3E9:
	    printf("HUNK_CODE");
	    fseek(fi, su->pu_Offset, 0);
	    break;
	case 0x3EA:
	    printf("HUNK_DATA");
	    fseek(fi, su->pu_Offset, 0);
	    break;
	case 0x3EB:
	    printf("HUNK_BSS ");
	    break;
	}
	printf(" #%02d (%-15s) Hunk=%08x Size=%d bytes\n",
	    srcHunk,
	    ((su->pu_Name) ? su->pu_Name : ""),
	    (int32_t)su->pu_Type,
	    su->pu_Size
	);

	{
	    int offset = 0;
	    Symbol *sym;
	    Symbol *symNext;

	    for (sym = FindSymbolOffset(offset, srcHunk); sym; sym = symNext) {
		symNext = FindSymbolNext(sym);

		if (sym->sm_Type > 1) {
		    printf(" %02x.%08x %-20s TYPE %d\n", srcHunk, (int32_t)sym->sm_Value, sym->sm_Name, sym->sm_Type);
		    continue;
		}
		if (sym->sm_Value != offset) {
		    if (sym->sm_Value < offset)
			cerror(EFATAL, "Soft Error, Symbol/sort");

		    switch((uword)su->pu_Type) {
		    case 0x3E9:
			if (NoCode == 0)
			    DumpCode(fi, srcHunk, offset, sym->sm_Value);
			break;
		    case 0x3EA:
			if (NoData == 0)
			    DumpData(fi, srcHunk, offset, sym->sm_Value);
			break;
		    }
		    offset = sym->sm_Value;
		}
		if ((uword)su->pu_Type == 0x3E9) {
		    if (NoCode == 0)
			puts("");
		    printf("*%02x.%08x %s:\t(%d bytes)\n",
			srcHunk,
			(int32_t)offset,
			sym->sm_Name,
			symNext ? symNext->sm_Value - offset : su->pu_Size - offset
		    );
		} else {
		    printf(" %02x.%08x %s:\n", srcHunk, (int32_t)offset, sym->sm_Name);
		}
	    }

	    if (offset != su->pu_Size) {
		switch((uword)su->pu_Type) {
		case 0x3E9:
		    if (NoCode == 0)
			DumpCode(fi, srcHunk, offset, su->pu_Size);
		    break;
		case 0x3EA:
		    if (NoData == 0)
			DumpData(fi, srcHunk, offset, su->pu_Size);
		    break;
		}
	    }
	}
    }
}

void
ResetHashTables(void)
{
    ResetReloc();
    ResetSymbol();
}

