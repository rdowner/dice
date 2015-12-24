/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  LIBTOS.C
 *
 *  This program takes two arguments, AMIGA.LIB, and an OUTPUT FILE,
 *  and systematically converts all references to the variables below
 *  from ABSOLUTE to A4-RELATIVE (Using Lattice's DATA-REL16 relocation
 *  info and symbol extensions).
 *
 *  It converts absolute move's into an A4 relative move and a nop to
 *  take up the extra word which is now no longer used.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef AMIGA
#include <lib/version.h>
#else
#include <include/lib/version.h>
#endif

IDENT("libtos",".4");
DCOPYRIGHT;

short SymLens[64];

char *FixSymbols[64] = {
    "_MathBase",
    "_ClistBase",
    "_ConsoleDevice",
    "_DiskfontBase",
    "_DOSBase",
    "_SysBase",
    "_GfxBase",
    "_IconBase",
    "_IntuitionBase",
    "_LayersBase",
    "_PotgoBase",
    "_TimerBase",
    "_TranslatorBase",
    "_MathIeeeDoubBasBase",
    "_MathTransBase",
    "_ExpansionBase",
    "_RomBootBase",
    "_MathIeeeDoubTransBase",

    /*
     *	2.0 stuff (doesn't effect anything if you
     *		   run on a 1.3 amiga.lib)
     */


    "_BattClockBase",
    "_BattMemBase",
    "_CxBase",
    "_GadToolsBase",
    "_KeymapBase",
    "_MathIeeeSingBasBase",
    "_MathIeeeSingTransBase",
    "_UtilityBase",
    "_WorkbenchBase",
    "_MiscBase",
    "_IFFParseBase",
    "_InputBase",
    "_AslBase",

    /*
     *	3.0 stuff (doesn't effect anything if you
     *		   run on a 1.3 or 2.0 amiga.lib)
     */

    "_AmigaGuideBase",
    "_BulletBase",
    "_ColorWheelBase",
    "_DataTypesBase",
    "_DiskBase",
    "_DTClassBase",
    "_LocaleBase",
    "_RexxSysBase",

    NULL
};

short DDebug;
short Error;

typedef unsigned char ubyte;
typedef unsigned short uword;

void InitSyms(void);
int ScanRelocations(int32_t, int32_t *, ubyte *, int32_t, char *, int);

#define MAXHUNKS    32

int
main(int ac, char **av)
{
    FILE *fi;
    FILE *fo;
    short hunkno;
    int32_t *base;
    int32_t *scan;
    int32_t *bend;
    int32_t bytes;

    InitSyms();
    if (ac < 3) {
        puts(Ident);
        puts(DCopyright);
	puts("libtos amiga.lib dlib:amigas.lib");
	exit(1);
    }
    if (ac > 3)
	DDebug = 1;
    fi = fopen(av[1], "r");
    if (fi == NULL) {
	printf("couldn't open %s\n", av[1]);
	exit(1);
    }
    fseek(fi, 0L, 2);
    bytes = ftell(fi);
    fseek(fi, 0L, 0);
    base = malloc(bytes);
    if (base == NULL) {
	puts("ran out of memory");
	exit(1);
    }
    fread((char *)base, bytes, 1, fi);
    fclose(fi);
    bend = base + (bytes >> 2);

    for (scan = base; scan < bend;) {
	ubyte *data;
	int32_t dlen;
	short htype;

	hunkno = 0;
	if (*scan != 0x3E7) {
	    printf("Expected hunk_unit, got %08x\n", *scan);
	    exit(1);
	}
	scan += scan[1] + 2;

	/*
	 *  scan the object module
	 */

	data = NULL;
	dlen = 0;
	htype = 0;

	while (scan < bend && *scan != 0x3E7) {
	    uint32_t len;

	    switch((uword)*scan) {
	    case 0x3E8: 	    /*	HUNK_NAME	*/
		scan += scan[1] + 2;
		break;
	    case 0x3E9: 	    /*	HUNK_CODE	*/
	    case 0x3EA: 	    /*	HUNK_DATA	*/
		data = (ubyte *)(scan + 2);
		dlen = scan[1] << 2;
		if (*scan == 0x3EA)
		    htype = 1;
		else
		    htype = 2;

		{
		    ++hunkno;
		}
		scan += scan[1] + 2;
		break;
	    case 0x3EB: 	    /*	HUNK_BSS	*/
		htype = 0;
		scan += 2;
		break;
	    case 0x3EC: 	    /*	HUNK_RELOC32	*/
	    case 0x3ED: 	    /*	HUNK_RELOC16-PC */
	    case 0x3EE: 	    /*	HUNK_RELOC8	*/
	    case 0x3F8: 	    /*	HUNK_RELOC16-D	(special) */
		++scan;
		while (*scan)
		    scan += *scan + 2;
		++scan;
		break;
	    case 0x3EF: 	    /*	HUNK_EXT	*/
	    case 0x3F0: 	    /*	HUNK_SYMBOL	*/
		++scan;
		while ((len = *scan) != 0) {
		    ubyte type = len >> 24;
		    int32_t *base = scan;

		    len &= 0x00FFFFFF;
		    if (DDebug)
			printf("extsym %3d %.*s\n", type, (int)(len*4), (char *)(scan + 1));
		    scan += len + 1;

		    switch(type) {
		    case 0:	    /*	SYMB	*/
		    case 1:	    /*	DEF	*/
		    case 2:	    /*	ABS	*/
		    case 3:	    /*	RES	*/
			++scan;     /*	skip value  */
			break;
		    case 130:	    /*	COMMON	*/
			++scan;     /*	skip common size */
			/* fall through */
		    case 129:	    /*	REF32	*/
			if (type == 129 && htype == 2) {
			    if (ScanRelocations(scan[0], scan + 1, data, dlen, (char *)(base + 1), len*4))
				*base = (*base & 0x00FFFFFF) | (134 << 24);	/*  convert from ABS32 to DATA-REL16	*/
			}
		    case 131:	    /*	REF16	*/
		    case 132:	    /*	REF8	*/
		    case 134:	    /*	REF16D	*/
				    /*	skip relocation info */
			scan += scan[0] + 1;
			break;
		    default:
			printf("Symbol type %d unknown", type);
			exit(1);
		    }
		}
		++scan;     /*	skip 0 terminator */
		break;
	    case 0x3F1: 	    /*	HUNK_DEBUG	    */
		scan += scan[1] + 2;
		break;
	    case 0x3F2: 	    /*	HUNK_END	    */
		data = NULL;
		dlen = 0;
		++hunkno;
		++scan;
		break;
	    default:
		printf("Unknown hunk type $%08x", *scan);
		exit(1);
	    }
	}
    }

    if (Error) {
	puts("ERROR OCCURED, OUTPUT NOT WRITTEN");
	return(1);
    }

    fo = fopen(av[2], "w");
    if (fo == NULL) {
	printf("couldn't create %s\n", av[2]);
	return(1);
    }
    fwrite((char *)base, bytes, 1, fo);
    fclose(fo);
    return(0);
}

void
InitSyms()
{
    short i;
    for (i = 0; FixSymbols[i]; ++i) {
	SymLens[i] = strlen(FixSymbols[i]);
    }
}

int
ScanRelocations(entries, scan, data, dlen, name, len)
int32_t entries;
int32_t *scan;
ubyte *data;
int32_t dlen;
char *name;
int len;
{

    printf("Relocation: %2d relocations for symbol %-20.*s",
	   entries, len, name);

    {
	short i;
	static char Buf[256];

	strncpy(Buf, name, len);
	Buf[len] = 0;

	for (i = 0; SymLens[i]; ++i) {
	    if (strcmp(FixSymbols[i], Buf) == 0)
		break;
	}
	if (SymLens[i] == 0) {
	    printf("(%s %zd not modified)\n", Buf, strlen(Buf));
	    return(0);
	}
    }
    puts("(modifying)");

    while (entries) {
	int32_t index = *scan;
	uword *tscan;
	uword opcode;

	if (index < 0 || index > dlen - 4) {
	    printf("relocation index out of range: %d/%d\n", index, dlen);
	    Error = 1;
	}
	tscan = (uword *)(data + index - 2);

	opcode = *tscan;
	printf("\topcode %04x\n", opcode);

	/*
	 *  convert abs to off(A4)  0010 0000 0110 1100
	 */

	switch(opcode) {
	case 0x2079:	/*  move.l  abslong,A0	*/
	    tscan[0] = 0x206C;
	    tscan[1] = tscan[2];
	    tscan[2] = 0x4E71;
	    break;
	case 0x2C79:	/*  move.l  abslong,A6	*/
	    tscan[0] = 0x2C6C;
	    tscan[1] = tscan[2];
	    tscan[2] = 0x4E71;
	    break;
	case 0xD1F9:	/*  adda.l  abslong,A0	*/
	    tscan[0] = 0xD1EC;
	    tscan[1] = tscan[2];
	    tscan[2] = 0x4E71;
	    break;
	default:
	    printf("\tWarning, opcode not understood, could not modify\n");
	    Error = 1;
	    break;
	}
	++scan;
	--entries;
    }
    return(1);
}

