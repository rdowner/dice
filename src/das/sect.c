/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  SECT.C
 */

#include "defs.h"

Prototype Sect	*CurSection;
Prototype Sect	*SectBase;
Prototype Sect	**LastSect;
Prototype short Hunks;

Prototype void	InitSect(void);
Prototype void	ResetSectAddrs(void);
Prototype void	HandleInstReloc(Sect *, Label *, long, short, long);
Prototype void	DumpSectionData(Sect *, void *, long);
Prototype void	NewSection(char *);
Prototype void	SectCreateObject(FILE *, char *);
Prototype void	CreateHunkReloc(FILE *, long, Reloc *);
Prototype void	DumpRelocs(FILE *, Reloc *, long, long);
Prototype void	CreateExtHunkReloc(FILE *, ubyte, Reloc *);
Prototype void	DumpExtReloc(FILE *, Reloc *, long, Label *, long);
Prototype void	CreateExportSDU(FILE *, ubyte, Label *);
Prototype void	CreateSymbols(FILE *, Label *);
Prototype void	zwrite(FILE *, long);

Sect	*CurSection;
Sect	*SectBase;
Sect	**LastSect;
short	Hunks;
static long Ary[64];

void
InitSect()
{
    LastSect = &SectBase;
    NewSection(strdup("____DUMMY____,code"));
    Hunks = 0;
    CurSection->Type = SECT_DUMMY;
    DebugLineNo = 0;
}

void
ResetSectAddrs()
{
    Sect *sect;
    for (sect = SectBase; sect; sect = sect->Next)
	sect->Addr = 0;
    DebugLineNo = 0;
}

/*
 *  Create a relocation entry and add it to the section.
 */

void
HandleInstReloc(
    Sect *sect, 	/* section this occurs in	*/
    Label *label,	/* label in question		*/
    long offset, 	/* offset in section sect of info to be relocated */
    short size, 	/* size of information needing relocation	  */
    long reloc_type	/* type of relocation		*/
) {
    Reloc *reloc = AllocStructure(Reloc);
    Reloc **list = sect->RelocAry;
    /*Reloc **scan;*/

    dbprintf(0, ("@%04lx Reloc %s sect %s size %d\n", offset, label->Name, sect->Name, size));

    reloc->Label = label;
    reloc->Offset = offset;

    if (label->l_Type == LT_EXT)
	list += RA_EXT;

    switch(size | reloc_type) {
    case 4 | RELOC_PCREL:	/*  long-pc-relative	    */
	++list;
    case 2 | RELOC_DATAREL:	/*  word-data reloc	    */
	++list;
    case 4:			/*  long-absolute	    */
	++list;
    case 2:			/*  absolute const reloc    */
    case 2 | RELOC_PCREL:	/*  word-pc relative	    */
	++list;
    case 1:			/*  byte-pc relative?	    */
	break;
    default:
	cerror(EERROR_BAD_RELOC);
	break;
    }

    /*
     *  Internal relocs must be sorted by hunk number so the dump routines
     *  can group them together in output.
     *
     *  External relocs must be sorted by label.  THE LABEL->SECT FIELD IS
     *  NULL FOR EXTERNAL RELOCS!
     */

    if (label->l_Type != LT_EXT) {
    while (*list && (*list)->Label->Sect->Hunk < label->Sect->Hunk)
	list = &(*list)->RNext;

	while (*list && (*list)->Label->Sect->Hunk == label->Sect->Hunk) {
	    if ((*list)->Label == reloc->Label)
		break;
	    list = &(*list)->RNext;
	}
#ifdef NOTDEF
        for (scan = list;
	    *scan && (*scan)->Label->Sect->Hunk == label->Sect->Hunk;
	    scan = &(*scan)->RNext)
	{
	if ((*scan)->Label == reloc->Label) {
	    reloc->RNext = *scan;
	    *scan = reloc;
	    return;
	}
    }
#endif
    } else {
	while (*list && (*list)->Label != reloc->Label)
	    list = &(*list)->RNext;
    }
    reloc->RNext = *list;
    *list = reloc;
}

/*
 *  buf can be NULL, indicating DS or 0's
 */

void
DumpSectionData(sect, buf, bytes)
Sect *sect;
void *buf;
long bytes;
{
    DBlock *block = sect->LastBlock;

    if (bytes < 0)
	cerror(EFATAL_DUMPSECT_NEG);

    if (bytes == 0)
	return;

#ifdef DEBUG
    if (DDebug) {
	if (buf) {
	    long i;
	    ubyte *ptr = (ubyte *)buf;
	    printf("@%04lx dump ", sect->Addr);
	    for (i = 0; i < bytes; i += 2) {
		printf("%02x", *ptr++);
		printf("%02x ", *ptr++);
	    }
	    printf("\n");
	} else {
	    printf("@%04lx dump %ld bytes\n", sect->Addr, bytes);
	}
    }
#endif

    if (block && bytes <= block->Max - block->Size) {
	if (buf)
	    movmem(buf, (char *)block->Data + block->Size, bytes);
	else
	    setmem((char *)block->Data + block->Size, bytes, 0);
	block->Size += bytes;
	sect->ObjLen += bytes;
	sect->Addr   += bytes;
	return;
    }
    if (buf == NULL) {
	if (sect->Block == NULL) {
	    sect->ObjLen += bytes;
	    sect->Addr	 += bytes;
	    return;
	}
	if (sect->LastBlock->Data == NULL) {
	    sect->LastBlock->Size += bytes;
	    sect->LastBlock->Max  += bytes;
	    sect->ObjLen += bytes;
	    sect->Addr += bytes;
	    return;
	}
	/*
	 *  fall through to append a dummy block.
	 */
    }

    block = AllocStructure(DBlock);
    block->Next = NULL;

    if (sect->LastBlock) {
	sect->LastBlock->Next = block;
    } else {
	sect->Block = block;

	/*
	 *  This is the first real block
	 *
	 *  If there was previous bss data (i.e. no blocks but non-zero
	 *  object length) then create a dummy block whos buf is NULL
	 *  with the # of zeros to write.
	 */

	if (sect->ObjLen) {
	    block->Max = block->Size = sect->ObjLen;

	    block = AllocStructure(DBlock);
	    sect->Block->Next = block;
	}
    }
    sect->LastBlock = block;

    if (buf) {
	block->Max = SECTBLKSIZE;
	if (block->Max < bytes)
	    block->Max = bytes;
	block->Data = malloc(block->Max);
	if (block->Data == NULL)
	    NoMemory();
	block->Size = bytes;
	movmem(buf, (char *)block->Data, bytes);
    } else {
	block->Max = block->Size = bytes;
    }
    sect->ObjLen += bytes;
    sect->Addr	 += bytes;
}

/*
 *  [name],type[,$mask]     type is:	code,data,bss,abs
 *			    mask used for mem flags, resource flags
 */

void
NewSection(ops)
char *ops;
{
    char *ptr = ops;
    char *maskStr;
    Sect *sect;
    short type;
    long mask = 0;

    while (*ptr && *ptr != ',')
	++ptr;
    if (*ptr != ',') {
	cerror(EERROR_BAD_SECTION_DIRECT);
	NewSection(strdup("code,code"));    /*  to prevent program crash */
	return;
    }
    *ptr++ = 0;
    for (maskStr = ptr; *maskStr && *maskStr != ','; ++maskStr);
    if (*maskStr == ',')
	*maskStr++ = 0;

    if (*maskStr)
	mask = ParseIntExp(maskStr);

    cvtstrtolower(ptr);

    type = SECT_CODE;
    if (strcmp(ptr, "code") == 0)
	type = SECT_CODE;
    else if (strcmp(ptr, "data") == 0)
	type = SECT_DATA;
    else if (strcmp(ptr, "bss") == 0)
	type = SECT_BSS;
    else if (strcmp(ptr, "abs") == 0)
	type = SECT_ABS;
    else if (strncmp(ptr, "com", 3) == 0)
	type = SECT_COMMON;
    else
	cerror(EERROR_BAD_SECTION_TYPE);

    /*
     *	Does the section exist already?
     */

    for (sect = SectBase; sect; sect = sect->Next) {
	if (strcmp(ops, sect->Name) == 0 && sect->HunkMask == mask) {
	    if (sect->Type != type)
		cerror(EERROR_SECTION_NAME_CONFLICT);
	    break;
	}
    }
    if (sect == NULL) {
	sect = AllocStructure(Sect);
	sect->Name = ops;
	sect->Type = type;
	sect->Hunk = Hunks++;
	sect->HunkMask = mask;

	*LastSect = sect;
	LastSect = &sect->Next;
    }
    CurSection = sect;
}

/*
 *  Generate the object file.  Begin with a HUNK_UNIT containing the name
 *  of the object file.  Continue with the following for EACH section:
 *	-hunk_name
 *	-hunk_code/data/bss
 *	-hunk_reloc32
 *	-hunk_reloc16
 *	-hunk_reloc8
 *	-hunk_ext	(external references and external defs, ordering:
 *			 extref32, extref16, extref8, extdef)
 *	-hunk_symbol	(optional)
 *	-hunk_debug	(optional)
 *	-hunk_end	end of this hunk...
 */

void
SectCreateObject(fo, unitName)
FILE *fo;
char *unitName;
{
    Sect *sect;

    /*
     *	hunk_unit
     */
    {
	int len;
	short i;
	char c = 0;

	i = strlen(unitName);
	while (i >= 0 && unitName[i] != '.')
	    --i;
	if (NoUnitName)
	    i = 0;
	if (i >= 0) {
	    c = unitName[i];
	    unitName[i] = 0;
	}

	len = (strlen(unitName) + 3) >> 2;

	Ary[0] = ToMsbOrder(0x3E7);
	Ary[1] = ToMsbOrder(len);
	setmem(Ary + 2, len*4, 0);
	strcpy((char *)(Ary + 2), unitName);
	fwrite((char *)Ary, sizeof(long), len + 2, fo);
	if (i >= 0)
	    unitName[i] = c;
    }
    /*
     *	For each section...
     */
    for (sect = SectBase; sect; sect = sect->Next) {
	if (sect->Type == SECT_DUMMY)
	    continue;
	/*
	 *  hunk_name
	 */
	{
	    int len = (strlen(sect->Name) + 3) >> 2;
	    Ary[0] = ToMsbOrder(0x3E8);
	    Ary[1] = ToMsbOrder(len);
	    setmem(Ary + 2, len*4, 0);
	    strcpy((char *)(Ary + 2), sect->Name);
	    fwrite((char *)Ary, sizeof(long), len + 2, fo);
	}
	/*
	 *  hunk_code,data, or bss
	 */
	{
	    static long TConvAry[] = { 0, 0x3E9, 0x3EA, 0x3EB, 0x3EB, 0x3EB };

	    Ary[0] = ToMsbOrder(TConvAry[sect->Type] | sect->HunkMask);

	    if (sect->Type == SECT_COMMON)
		Ary[1] = 0;
	    else
		Ary[1] = ToMsbOrder((sect->ObjLen + 3) >> 2);

	    fwrite((char *)Ary, sizeof(long), 2, fo);

	    if (sect->Type != SECT_BSS && sect->Type != SECT_ABS && sect->Type != SECT_COMMON) {   /* if not BSS/COMMON */
		DBlock *block;
		int n;
		if (sect->Block == NULL && sect->ObjLen)
		    zwrite(fo, sect->ObjLen);
		for (block = sect->Block; block; block = block->Next) {
		    if (block->Data) {
			fwrite((char *)block->Data, 1, block->Size, fo);
		    } else {
			zwrite(fo, block->Size);
		    }
		}

		/*
		 *  When forcing alignment align with NOPs in a CODE section.
		 */

		if ((n = AlignDelta(sect->ObjLen, 4)) != 0) {
		    if ((n & 1) == 0 && sect->Type == SECT_CODE) {
			Ary[0] = ToMsbOrder(0x4E714E71);
		    } else {
			Ary[0] = 0;
		    }
		    fwrite((char *)Ary, 1, n, fo);
		}
	    }
	}

	/*
	 *  local byte-pc, word-pc, long, and word-data reloc
	 */

	{
	    CreateHunkReloc(fo, 0x3EE, sect->r_ByteReloc);
	    CreateHunkReloc(fo, 0x3ED, sect->r_WordRelocPc);
	    CreateHunkReloc(fo, 0x3EC, sect->r_LongReloc);
	    CreateHunkReloc(fo, 0x3F8, sect->r_WordDataReloc);
	    CreateHunkReloc(fo, 0x400, sect->r_LongRelocPc);
	}

	/*
	 *  external byte, word-pc, long, and word-data relocation
	 *
	 *  symbols
	 */

	if (sect->r_ExtByteReloc || sect->r_ExtWordRelocPc || sect->r_ExtLongReloc || sect->r_ExtWordDataReloc || sect->r_LongRelocPc || sect->XDefLab) {
	    Ary[0] = ToMsbOrder(0x3EF);
	    fwrite((char *)Ary, sizeof(long), 1, fo);

	    CreateExtHunkReloc(fo, 134, sect->r_ExtWordDataReloc);
	    CreateExtHunkReloc(fo, 132, sect->r_ExtByteReloc);
	    CreateExtHunkReloc(fo, 131, sect->r_ExtWordRelocPc);
	    CreateExtHunkReloc(fo, 129, sect->r_ExtLongReloc);
	    CreateExtHunkReloc(fo, 135, sect->r_ExtLongRelocPc);
	    CreateExportSDU(fo, ((sect->Type == SECT_COMMON) ? 130 : 1), sect->XDefLab);
	    Ary[0] = ToMsbOrder(0);
	    fwrite((char *)Ary, sizeof(long), 1, fo);
	}

	if (AddSym)
	{
	    CreateSymbols(fo, sect->XDefLab);
	}

	/*
	 *  hunk_debug
	 */

	if (sect->DebugAry && sect->DebugIdx) {
	    Ary[0] = ToMsbOrder(0x3F1);

	    Ary[2] = ToMsbOrder(0);
	    Ary[3] = ToMsbOrder(0x4C494E45);  /* 'LINE'; */
	    Ary[4] = ToMsbOrder((strlen(SrcFileName) + 3) >> 2);

	    setmem(Ary + 5, ToMsbOrder(Ary[4]) * 4, 0);
	    strcpy((char *)(Ary + 5), SrcFileName);

	    Ary[1] = ToMsbOrder(sect->DebugIdx * 2 + 3 + FromMsbOrder(Ary[4]));

	    fwrite((char *)Ary, sizeof(long), 5 + FromMsbOrder(Ary[4]), fo);
	    fwrite(sect->DebugAry, sizeof(DebugNode), sect->DebugIdx, fo);
	}

	/*
	 *  hunk_end
	 */
	{
	    Ary[0] = ToMsbOrder(0x3F2);
	    fwrite((char *)Ary, sizeof(long), 1, fo);
	}
    }
}

void
CreateHunkReloc(fo, hunk, reloc)
FILE *fo;
long hunk;
Reloc *reloc;
{
    long n;
    long t;
    long baseHunk;
    Reloc *base;

    if (reloc == NULL)
	return;

    t = ToMsbOrder(hunk);
    fwrite((char *)&t, sizeof(long), 1, fo);

    base = reloc;
    baseHunk = reloc->Label->Sect->Hunk;
    n = 0;

    while (reloc) {
	if (baseHunk != reloc->Label->Sect->Hunk) {
	    DumpRelocs(fo, base, n, baseHunk);
	    baseHunk = reloc->Label->Sect->Hunk;
	    base = reloc;
	    n = 0;
	}
	++n;
	reloc = reloc->RNext;
    }
    if (n)
	DumpRelocs(fo, base, n, baseHunk);

    t = ToMsbOrder(0);
    fwrite((char *)&t, sizeof(long), 1, fo);
}

void
DumpRelocs(fo, base, n, baseHunk)
FILE *fo;
Reloc *base;
long n;
long baseHunk;
{
    long t;

    t = ToMsbOrder(n);
    fwrite((char *)&t, sizeof(long), 1, fo);
    t = ToMsbOrder(baseHunk);
    fwrite((char *)&t, sizeof(long), 1, fo);

    while (n--) {
        t = ToMsbOrder(base->Offset);
	fwrite((char *)&t, sizeof(long), 1, fo);
	base = base->RNext;
    }
}

void
CreateExtHunkReloc(FILE *fo, ubyte type, Reloc *reloc)
{
    Reloc *base;
    Label *baseLabel;
    long n;

    if (reloc == NULL)
	return;

    base = reloc;
    baseLabel = reloc->Label;
    n = 0;

    while (reloc) {
	if (baseLabel != reloc->Label) {
	    DumpExtReloc(fo, base, n, baseLabel, type);
	    baseLabel = reloc->Label;
	    base = reloc;
	    n = 0;
	}
	++n;
	reloc = reloc->RNext;
    }
    if (n)
	DumpExtReloc(fo, base, n, baseLabel, type);
}

void
DumpExtReloc(fo, base, n, label, type)
FILE *fo;
Reloc *base;
long n;
Label *label;
long type;
{
    int len = (strlen(label->Name) + 3) >> 2;

    Ary[0] = ToMsbOrder((type << 24) | len);
    setmem(Ary + 1, len*4, 0);
    strcpy((char *)(Ary + 1), label->Name);

    Ary[1 + len] = ToMsbOrder(n);		/*  # references */

    fwrite((char *)Ary, sizeof(long), len + 2, fo);
    while (n--) {
        long t = ToMsbOrder(base->Offset);
	fwrite((char *)&t, sizeof(long), 1, fo);
	base = base->RNext;
    }
}

/*
 *  XDEFs
 */

void
CreateExportSDU(FILE *fo, ubyte type, Label *label)
{
    while (label)
    {
	int len = (strlen(label->Name) + 3) >> 2;

	if (label->l_Type == LT_INT)
	    Ary[0] = ToMsbOrder((2 << 24) | len);
	else
	    Ary[0] = ToMsbOrder((type << 24) | len);

	setmem(Ary + 1, len*4, 0);
	strcpy((char *)(Ary + 1), label->Name);
	Ary[1 + len] = ToMsbOrder(label->l_Offset);    /* Offset or Size */
	fwrite((char *)Ary, sizeof(long), len + 2, fo);
	label = label->XDefLink;
    }
}


void
CreateSymbols(FILE *fo, Label *label)
{
    int header = 0;

    while (label) {
	int len = (strlen(label->Name) + 3) >> 2;

	if (label->l_Type != LT_INT)
	{
	    if (header == 0)
	    {
        	Ary[0] = ToMsbOrder(0x3F0);
        	fwrite((char *)Ary, sizeof(long), 1, fo);
        	header = 1;
            }

	    Ary[0] = ToMsbOrder(len);

	    setmem(Ary + 1, len*4, 0);
	    strcpy((char *)(Ary + 1), label->Name);
	    Ary[1 + len] = ToMsbOrder(label->l_Offset);   /* Offset or Size */
	    fwrite((char *)Ary, sizeof(long), len + 2, fo);
	}
	label = label->XDefLink;
    }

    if (header)
    {
	Ary[0] = 0;
	fwrite((char *)Ary, sizeof(long), 1, fo);
    }
}

void
zwrite(fo, bytes)
FILE *fo;
long bytes;
{
    static char Zero[512];

    while (bytes > sizeof(Zero)) {
	fwrite(Zero, 1, sizeof(Zero), fo);
	bytes -= sizeof(Zero);
    }
    fwrite(Zero, 1, bytes, fo);
}
