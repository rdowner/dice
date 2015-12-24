/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */
/*
 *  $Id: romable.c,v 1.1.1.1 1998/10/10 06:23:23 dillon Exp $
 *
 *  Romable exefile -o outfile [-o outfile2] [-pi] [-C addr -D addr]
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef AMIGA
#include <lib/version.h>
#else
#include <suplib/memory.h>
#include <include/lib/version.h>
#include <unistd.h>
#endif

#define D(x)	;	/* Debugging Disabled */
/* #define D(x)    x;	   // Debugging Enabled */

/* #define	BSS_IN_OUTPUT_FILE	// Write zeros to output file for BSS */

int32_t	NumHunks;
int32_t	FirstHunk;
int32_t	LastHunk;

int32_t	CodeStart;
int32_t	DataStart;
short	DataStartAfterCode = 0;
char	DataStartSpecified;	/*	flag, argument specified */
char	CodeStartSpecified;	/*	flag, argument specified */
char	PIOpt;			/*	-pi (position independent) */

int	_bufsiz = 8192; 	/*	Buffered file I/O */

typedef unsigned char ubyte;

/*
**	One of these is allocated per hunk in the file.
*/
typedef struct {
    int32_t    Type;	/* AmigaDOS hunk type */
    int32_t    Len;	/* Initialized data length only */
    int32_t    Pc;
    char    *Data;	/* Allocated memory for hunk */
} Hunk;

Hunk	*Hunks;

int32_t	fgetl(FILE *);
void	LoadHeaderInfo(FILE *);
void	ScanHunks(FILE *);
void	RelocHunks(FILE *);
void	DumpHunks(FILE *, FILE *);
int	fwrite_2(int, FILE *, FILE *, char *, int32_t);

int
main(int ac, char **av)
{
    short i;
    char *inFile = NULL;
    char *outFile1= NULL;
    char *outFile2= NULL;
    FILE *fi;
    FILE *fo1;
    FILE *fo2 = NULL;


    if (ac == 1 || ( ac == 2  &&  *av[1]=='?' )  ) {
	puts("Romable input.exe -o out_even [-o out_odd] -C 0xADDR -D[C] 0xADDR -pi");
	puts(";Convert Amiga executable files into binary files");
	exit(1);
    }

    for (i = 1; i < ac; ++i) {
	char *ptr = av[i];
	char *dummy;

	if (*ptr != '-') {
	    inFile = ptr;
	    continue;
	}
	ptr += 2;
	switch(ptr[-1]) {
	case 'o':
	    if (*ptr == 0)
		ptr = av[++i];

	    if (outFile1 == NULL) {
		outFile1 = ptr;
	    } else if (outFile2 == NULL) {
		outFile2 = ptr;
	    } else {
		puts("Only two image files may be specified");
		exit(1);
	    }
	    break;
	case 'p':   /*  -pi */
	    PIOpt = 1;	    /*	position independant	*/
	    break;
	case 'C':
	    if (*ptr == 0)
		ptr = av[++i];
	    CodeStart = strtoul(ptr, &dummy, 0);
	    CodeStartSpecified = 1;
	    break;
	case 'D':
	    if (*ptr == 'C') {
		DataStartSpecified = 1;
		DataStartAfterCode = 1;
		break;
	    }
	    if (*ptr == 0)
		ptr = av[++i];
	    DataStart = strtoul(ptr, &dummy, 0);
	    DataStartSpecified = 1;
	    break;
	default:
	    printf("Bad option: %s\n", av[i]);
	    exit(1);
	}
    }
    if (i > ac) {
	puts("Expected argument");
	exit(1);
    }
    if (PIOpt) {
	DataStart = 0;	    /*	all relative accesses	*/
	CodeStart = 0;
	DataStartSpecified = 1;
	CodeStartSpecified = 1;
    } else {
	if (DataStartSpecified == 0 || CodeStartSpecified == 0) {
	    puts("-D and/or -C options not specified!");
	    exit(1);
	}
    }
    if (inFile == NULL) {
	puts("Expected input file");
	exit(1);
    }
    if (outFile1 == NULL) {
	puts("Expected output file");
	exit(1);
    }
    fi = fopen(inFile, "r");
    if (fi == NULL) {
	printf("Unable to open input file %s\n",inFile);
	exit(1);
    }
    if (fgetl(fi) != 0x3F3) {
	printf("%s is not an executable",inFile);
	exit(1);
    }

    /*
     *	skip name
     */
    {
	int n = fgetl(fi);
	while (n--)
	    fgetl(fi);
    }

    /*
     *	header info
     */

    NumHunks	= fgetl(fi);
    FirstHunk	= fgetl(fi);
    LastHunk	= fgetl(fi);

    Hunks = malloc(NumHunks * sizeof(Hunk));
    if (Hunks == NULL) {
	puts("malloc failed");
	exit(1);
    }
    LoadHeaderInfo(fi);
    {
	int32_t pos = ftell(fi);
	    D(puts("--scan--"));

	ScanHunks(fi);

	fseek(fi, pos, 0);
	    D(puts("--reloc--"));

	RelocHunks(fi);
    }
    fclose(fi);

    fo1 = fopen(outFile1, "w");
    if (fo1 == NULL) {
	printf("Unable to open output file %s\n", outFile1);
	exit(1);
    }
    if (outFile2) {
	fo2 = fopen(outFile2, "w");
	if (fo2 == NULL) {
	    printf("Unable to open output file %s\n", outFile2);
	    fclose(fo1);
	    remove(outFile1);
	    exit(1);
	}
    }
	D(puts("--dump--"));
    DumpHunks(fo1, fo2);
    fclose(fo1);
    if (fo2)
	fclose(fo2);
    return(0);
}

void
LoadHeaderInfo(fi)
FILE *fi;
{
    int n;
    Hunk *h;

    for (n = 0, h = Hunks; n < NumHunks; ++n, ++h) {
	h->Len = fgetl(fi) * 4;
	h->Data= malloc(h->Len + 1);	/*  +1 so can malloc 0 wo/error    */
	if (h->Data == NULL) {
	    printf("malloc failed [hunk %d,length %d]\n", n, h->Len);
	    exit(1);
	}
    }
}

int32_t	CodePc;
int32_t	DataPc;

/*
 *  Determine PC start and TYPE.  Read data into preallocated memory.
 */

void
ScanHunks(fi)
FILE *fi;
{
    int n;
    Hunk *h;

    n = 0;
    h = Hunks;
    while (n < NumHunks) {
	int32_t len;
	int32_t blen;
	int32_t t = fgetl(fi);

	    D(printf("Header %08x\n", t));

	switch(t) {
	case 0x3E9:	/*  CODE    */
	case 0x3EA:	/*  DATA    */
	    h->Type = t;
	    len = fgetl(fi) * 4;
	    if (len > h->Len || len < 0) {
		printf("Hunk Error: Text len %d/%d\n", len, h->Len);
		exit(1);
	    }
	    h->Len = len;
	    if (fread(h->Data, 1, (size_t)len, fi) != len) {
		puts("Unexpected EOF");
		exit(1);
	    }
	    break;
	case 0x3EB:	/*  BSS     */
	    D(printf("BSS Length=%d\n",h->Len));
	    h->Type = t;
	    setmem(h->Data, h->Len, 0);
	    if ((blen=(fgetl(fi)*4)) != h->Len)
		printf("Size mismatch on BSS hunk #%d: %d != %d\n",n,blen,h->Len);
	    break;
	case 0x3EC:	/*  HUNK_RELOC32    */
	    while ((len = fgetl(fi) * 4) != 0) {
		    D(printf("Reloc 32 len = %d\n", len));
		fgetl(fi);	/*  skip hunk#	*/
		fseek(fi, len, 1);
	    }
	    break;
	case 0x3F0:	/*  SYMBOLS */
	    /*
	     *	ignore
	     */
	    while ((len = fgetl(fi)) != 0) {
		ubyte type = len >> 24;

		len &= 0x00FFFFFF;
		fseek(fi, len * 4, 1);

		switch(type) {
		case 0: 	/*  SYMB    */
		case 1: 	/*  DEF     */
		case 2: 	/*  ABS     */
		case 3: 	/*  RES     */
		    fgetl(fi);
		    break;
		case 130:	/*  COMMON  */
		    puts("COMMON symbol not supported");
		    fgetl(fi);	/*  skip common size */
		    /* fall through */
		case 129:	/*  REF32   */
		case 131:	/*  REF16   */
		case 132:	/*  REF8    */
		case 134:	/*  REF16D  */
				/*  skip relocation info */
		    fseek(fi, fgetl(fi) * 4 + 4, 1);
		    break;
		default:
		    printf("Symbol type %d unknown\n", type);
		    exit(1);
		}
	    }
	    break;
	case 0x3F2:	/*  ignore HUNK_END */
	    ++n;
	    ++h;
	    break;
	default:
	    printf("Hunk type %08x unknown\n", (uint32_t)h->Type);
	    exit(1);
	}
    }


    CodePc = CodeStart;

    /*
     *	Calculate CODE PC's (base of each hunk, based on size of other hunks)
     */
    for (n = 0, h = Hunks; n < NumHunks; ++n, ++h) {
	if (h->Type == 0x3E9) {
		D(printf("Code hunk: CodePc=%lx\n",CodePc));
	    h->Pc = CodePc;
	    CodePc += h->Len;
	}
    }
    D(printf("Code end : CodePc=%lx\n",CodePc));

    /*
     *	DATA PC's.  If -DC (data start after code) is specified then
     *		    jam the now determined address in (no duplication
     *		    of data occurs in this case)
     */
    if (DataStartAfterCode)
	DataStart = CodePc;

    DataPc = DataStart;

    for (n = 0, h = Hunks; n < NumHunks; ++n, ++h) {
	if (h->Type == 0x3EA) {
		D(printf("Data hunk: DataPc=%lx\n",DataPc));
	    h->Pc = DataPc;
	    DataPc += h->Len;
	    CodePc += h->Len;
	}
    }

    /*
     *	Start BSS at the end of data
     */
    for (n = 0, h = Hunks; n < NumHunks; ++n, ++h) {
	if (h->Type == 0x3EB) {
		D(printf("BSS hunk : DataPc=%lx\n",DataPc));
	    h->Pc = DataPc;
	    DataPc += h->Len;
	}
    }
    D(printf("Data/BSS end : DataPc=%lx\n",DataPc));
}

void
RelocHunks(fi)
FILE *fi;
{
    int n;
    int dhno;
    Hunk *h;
    Hunk *dh;

    n = 0;
    h = Hunks;

    while (n < NumHunks) {
	int32_t len;
	int32_t t;

	t = fgetl(fi);
	switch(t) {
	case 0x3EB:	/*  BSS     */
	    fgetl(fi);	/*  skip length */
	    break;
	case 0x3E9:	/*  CODE    */
	case 0x3EA:	/*  DATA    */
	    len = fgetl(fi) * 4;
	    fseek(fi, len, 1);
	    break;
	case 0x3EC:	/*  HUNK_RELOC32    */
	    if (PIOpt) {
		puts("32 bit relocations exist, cannot make position independant!");
		exit(1);
	    }
	    while ((len = fgetl(fi)) != 0) {
		dhno = fgetl(fi);	/*  hunk to relocate to */
		if (dhno < FirstHunk || dhno >= NumHunks + FirstHunk) {
		    printf("hunk# in reloc32 bad: %d/%d\n", dhno, NumHunks);
		    exit(1);
		}
		dh = Hunks + dhno - FirstHunk;
		while (len) {		/*  offsets to relocate */
		    int32_t off = fgetl(fi);

		    if (off < 0 || off > h->Len - 4) {
			printf("Bad offset: %d len=%d\n", off, h->Len);
			exit(1);
		    }
		    *(int32_t *)(h->Data + off) = ToMsbOrder(FromMsbOrder(*(int32_t *)(h->Data + off)) + dh->Pc);
			D(printf("Reloc in %d to %d offset %d add %04x\n", n, dhno, off, dh->Pc));
		    --len;
		}
	    }
	    break;
	case 0x3F0:	/*  SYMBOLS */
	    /*
	     *	ignore
	     */
	    while ((len = fgetl(fi)) != 0) {
		ubyte type = len >> 24;

		len &= 0x00FFFFFF;
		fseek(fi, len * 4, 1);

		switch(type) {
		case 0: 	/*  SYMB    */
		case 1: 	/*  DEF     */
		case 2: 	/*  ABS     */
		case 3: 	/*  RES     */
		    fgetl(fi);
		    break;
		case 130:	/*  COMMON  */
		    puts("COMMON symbol not supported");
		    fgetl(fi);	/*  skip common size */
		    /* fall through */
		case 129:	/*  REF32   */
		case 131:	/*  REF16   */
		case 132:	/*  REF8    */
		case 134:	/*  REF16D  */
				/*  skip relocation info */
		    fseek(fi, fgetl(fi) * 4 + 4, 1);
		    break;
		default:
		    printf("Symbol type %d unknown\n", type);
		    exit(1);
		}
	    }
	    break;
	case 0x3F2:	/*  ignore HUNK_END */
	    ++n;
	    ++h;
	    break;
	default:
	    printf("Hunk type %08x unknown\n", (uint32_t)h->Type);
	    exit(1);
	}
    }
}

/*
 *  Dump the stuff.  CODE is first, then DATA and finally BSS (yay!)
 */
void
DumpHunks(fo1, fo2)
FILE *fo1;
FILE *fo2;
{
    int32_t endpc __unused = CodeStart;
    short n;
    short nextIsEven = 1;
    Hunk *h;

    for (n = 0, h = Hunks; n < NumHunks; ++n, ++h) {
	if (h->Type == 0x3E9) {
		D(printf("$%04x bytes text @ %08x\n", h->Len, h->Pc));
	    endpc = h->Pc + h->Len;
	    if (fo2) {
		nextIsEven = fwrite_2(nextIsEven, fo1, fo2, h->Data, h->Len);
	    } else {
		fwrite(h->Data, 1, h->Len, fo1);
	    }
	}
    }

    for (n = 0, h = Hunks; n < NumHunks; ++n, ++h) {
	if (h->Type == 0x3EA) {
		D(printf("$%04x bytes data @ %08x reloc to %08x\n", h->Len, endpc, h->Pc));
	    endpc = h->Pc + h->Len;
	    if (fo2) {
		nextIsEven = fwrite_2(nextIsEven, fo1, fo2, h->Data, h->Len);
	    } else {
		fwrite(h->Data, 1, h->Len, fo1);
	    }
	}
    }

#ifdef	BSS_IN_OUTPUT_FILE
    for (n = 0, h = Hunks; n < NumHunks; ++n, ++h) {
	if (h->Type == 0x3EB) {
		D(printf("$%04x bytes bss  @ %08x reloc to %08x\n", h->Len, endpc, h->Pc));
	    endpc = h->Pc + h->Len;
	    if (fo2) {
		nextIsEven = fwrite_2(nextIsEven, fo1, fo2, h->Data, h->Len);
	    } else {
		fwrite(h->Data, 1, h->Len, fo1);
	    }
	}
    }
    D(printf("End at $%08x\n", endpc));
#endif

}


int32_t
fgetl(fi)
FILE *fi;
{
    int32_t v;

    v = getc(fi) << 24;
    v |= getc(fi) << 16;
    v |= getc(fi) << 8;
    v |= getc(fi);

    if (feof(fi)) {
	puts("Unexpected EOF");
	exit(1);
    }
	/*	D(printf("get %08x\n", v)); */
    return(v);
}

int
fwrite_2(nextIsEven, fo_even, fo_odd, ptr, len)
int nextIsEven;
FILE *fo_even;
FILE *fo_odd;
char *ptr;
int32_t len;
{
    while (len--) {
	if (nextIsEven)
	    putc(*ptr, fo_even);
	else
	    putc(*ptr, fo_odd);
	nextIsEven = 1 - nextIsEven;
	++ptr;
    }
    return(nextIsEven);
}
