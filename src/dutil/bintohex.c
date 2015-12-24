/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */
/*
 *  $Id: bintohex.c,v 1.1.1.1 1998/10/10 06:23:22 dillon Exp $
 *
 *  bintohex infile [outfile]
 *
 *  (Was) compilable on an IBM-PC or Amiga  _fmode is for Lattice C on the IBM,
 *  is IGNORED by Aztec C on the Amiga.  Note that INT and CHAR are not
 *  used as ibm's lattice C uses 16 bit ints and unsigned chars.  Change
 *  as needed.	No guarentees for the IBMPC version.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <include/lib/version.h>

IDENT("bintohex",".2");
DCOPYRIGHT;

#ifdef IBM
typedef char ubyte;
typedef unsigned uword;
typedef int void;
#else
typedef unsigned char ubyte;
typedef unsigned short uword;
#endif

#define PERLINE 16

void exiterr (ubyte *);
void convert (int, FILE *, FILE *, uint32_t);
uword getwlh (FILE *);
ubyte puth (ubyte, FILE *);

uword _fmode = 0;

int
main(int ac, char **av)
{
    char format=1;    /* 'i'=Intel.  1,2,3 = Motorola S1-S3 formats. */
    FILE *infile;
    FILE *outfile;
    char *inName=0;
    char *outName=0;
    int  i;
    uint32_t   org=0;	/* Hex file origin */

    _fmode = 0x8000;
    if (ac == 1 || ( ac == 2  &&  *av[1]=='?' ) ) {
        puts(Ident);
        puts(DCopyright);
	puts("bintohex <infile> -o [outfile] -s[1,2,3] [-i] [-O offset]");
	puts("; Convert <infile> to Motorola or Intel Hex format");
	exit(5);
    }

    for (i = 1; i < ac; ++i) {
	char *ptr = av[i];
	char *dummy;

	if (*ptr != '-') {
	    inName = ptr;
	    continue;
	}
	ptr += 2;
	switch(ptr[-1]) {
	case 'o':
	    if (*ptr == 0)		/* Space is optional */
		ptr = av[++i];

	    outName = ptr;
	    break;
	case 'i':
	case 'I':
	    format='i';
	    break;
	case 's':
	case 'S':
	    format=*ptr-'0';
	    if (format<1 || format >3) {
		printf("Format %s not supported.  Use  S1, S2 or S3.\n",av[i]);
		exit(10);
		}
	    break;
	case 'O':
	    if (*ptr == 0)
		ptr = av[++i];
	    org = strtol(ptr, &dummy, 0);
	    break;
	default:
	    printf("Bad option: %s\n", av[i]);
	    exit(10);
	}
    }

    if(!inName) {
	printf("No input file specified\n");
	exit(10);
	}
    infile = fopen(inName, "r");
    if (infile == NULL) {
	printf("Unable to open input file %s\n",inName);
	exit(10);
	}
    outfile = (outName) ? fopen(outName, "w") : stdout;
    if (outfile == NULL) {
	printf("Unable to open output file %s\n",outName);
	exit(10);
	}
    convert(format, infile, outfile, org);
    putc('z'&0x1F, outfile);    /* ^Z as end of file marker (for BP programmer) */
    fclose(infile);
    fclose(outfile);
    return(0);
}


/*
 *  Intel Hex output (MCS80 format):
 *
 *  :lloooott(ll bytes hex code)cc  ll=# of bytes
 *				    oooo=origin (high/low format)
 *				    tt="record type"
 *				    (00=norm,01=end,02=extended,03=code start)
 *				    cc=invert of checksum all codes
 *				    <cr/lf> after each line
 *
 *  Extended Intel Hex (MCS-86 format).  Adds segment:	20 bits, 1 Megabyte:
 *		:ll0000ffaaaass     ll-# of bytes (set to 02)
 *				    ff-format type (set to 02)
 *				    aaaa-segment offset (address/16)
 *				    ss-checksum
 *
 *
 *  Motorola Hex files:
 *		Sxnn[[[aaaa]aa]aa]dd..ddss
 *			x	=Type 1=4 byte address, 64KB limit
 *				 Type 2=6 byte address, 16MB limit
 *				 Type 3=8 byte address, 4 GB limit
 *			nn	=Number of bytes+3
 *			aaaa	=address
 *			dd..dd	=Data
 *			ss	=Checksum
 *
 */
void
convert(format, in, out, org)
int  format;
FILE *in;
FILE *out;
uint32_t	org;
{
uword		idx;
uint32_t	len;
ubyte		buf[256];

  fseek(in, 0L, 2);	      /* Seek to end of file */
  len = ftell(in);
  fseek(in, 0, 0);

  if(format == 'i') {
    /************************ Intel **********************************/
    for (;;) {
	while (len > 0) {
	    register ubyte chk;
	    register short i;

	    idx = (len > PERLINE) ? PERLINE : len;
	    fread(buf, idx, 1, in);
	    putc(':', out);
	    puth((ubyte)idx, out);
	    puth((ubyte)(org >> 8), out);
	    puth((ubyte)(org & 0xFF), out);
	    putc('0', out);
	    putc('0', out);
	    chk = idx + (org >> 8) + (org & 0xFF);
	    for (i = 0; (uword)i < idx; ++i) {
		chk += buf[i];
		puth(buf[i], out);
	    }
	    puth((ubyte)-chk, out);
	    putc('\r', out);
	    putc('\n', out);
	    len -= idx;
	    org += idx;
	}
	break;
    }
    fprintf(out, ":00000001FF\r\n");
    /* End Intel */
    } else {
    /*********************** Motorola ********************************/

    /*
    if(format==1)
	if(len > 0xFFFF)
		printf("Warning: file size greater than 64K S1 limit. Use S2\n");
    if(format==2)
	if(len > 0xFFFFFF)
		printf("Warning: file size greater than 16Mb S2 limit. Use S3\n");
    */

    for (;;) {
	while (len > 0) {
	    register ubyte chk;
	    register short i;

	    idx = (len > PERLINE) ? PERLINE : len;
	    fread(buf, idx, 1, in);

	    putc('S', out);
	    putc(format+'0',out);

	    puth((ubyte)idx+2+format, out);

	    chk=idx+2+format;
	    if(format==3) {
		    chk += puth((ubyte)(org >>24), out);
		    chk += puth((ubyte)(org >>16), out);
		    }
	    if(format==2) {
		    chk += puth((ubyte)(org >>16), out);
		    }
	    chk += puth((ubyte)(org >> 8), out);
	    chk += puth((ubyte)(org & 0xFF), out);

	    for (i = 0; (uword)i < idx; ++i) {
		chk += puth(buf[i], out);
	    }
	    puth((ubyte)0xff-chk, out);
	    putc('\r', out);
	    putc('\n', out);
	    len -= idx;
	    org += idx;
	}
	break;
    }
    fprintf(out, "S%c\r\n",10-format+'0');     /* S9,S8,S7 */
  } /* End Motorola */
}

uword
getwlh(in)
FILE *in;
{
    uword result;

    result = getc(in);
    result += getc(in) << 8;
    return(result);
}

ubyte puth(ubyte c, FILE *out)
{
    static ubyte dig[] = { "0123456789ABCDEF" };
    putc(dig[(c>>4)&15], out);
    putc(dig[c&15], out);
    return(c);
}
