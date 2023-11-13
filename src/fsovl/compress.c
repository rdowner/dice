/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 * COMPRESS.C
 */

#include "defs.h"

#define BITS  14        /* compression size */

Prototype void Compress(BPTR fh, char *buf, long bytes);
Prototype void DeCompress(BPTR fh, char *buf, long bytes);

int initcomp(short bits);
int compress(void);
int decompress(void);
void deletecomp(void);
short comp_in(void);
short decomp_in(void);

static long ErrorCode;
static long Fh;
static ubyte *MemBuf;
static long  MemBytes;

static ubyte Buf[4096];
static long BufIdx;
static long BufLen;


/*    WritePacket(fh, buf, bytes); */
/*    ReadPacket(fh, buf, bytes);  */

void 
Compress(BPTR fh, char *buf, long bytes)
{
    MemBytes = bytes;
    MemBuf = buf;
    BufIdx = 0;
    BufLen = 0;
    ErrorCode = 0;
    Fh = fh;
    if (initcomp(BITS) >= 0) {
        compress();
        WritePacket(fh, Buf, BufIdx);
    }
    deletecomp();
}

void 
DeCompress(BPTR fh, char *buf, long bytes)
{
    MemBytes = bytes;
    MemBuf = buf;
    Fh = fh;
    BufIdx = 0;
    BufLen = 0;
    ErrorCode = 0;
    if (initcomp(0) >= 0) {
        decompress();
    }
    deletecomp();
}

void
comp_out(ubyte c)
{
    if (BufIdx == sizeof(Buf)) {
        WritePacket(Fh, Buf, sizeof(Buf));
        BufIdx = 0;
    }
    Buf[BufIdx++] = c;
}

void
comp_outn(ubyte *buf, long n)
{
    while (n) {
        long r;

        if ((r = sizeof(Buf) - BufIdx) == 0) {
            WritePacket(Fh, Buf, sizeof(Buf));
            BufIdx = 0;
            r = sizeof(Buf);
        }
        if (n < r)
            r = n;
        movmem(buf, Buf + BufIdx, r);
        buf += r;
        n -= r;
        BufIdx += r;
    }
}

short
comp_in()
{
    if (MemBytes) {
        --MemBytes;
        return(*MemBuf++);
    }
    return(EOF);
}

short
decomp_in()
{
    if (BufIdx == BufLen) {
        BufIdx = 0;
        BufLen = ReadPacket(Fh, Buf, sizeof(Buf));
        if (BufLen == 0)
            return(EOF);
    }
    return(Buf[BufIdx++]);
}

long
decomp_inn(ubyte *buf, long n)
{
    long ttl = 0;

    while (n) {
        long r = BufLen - BufIdx;

        if (r == 0) {
            BufIdx = 0;
            BufLen = ReadPacket(Fh, Buf, sizeof(Buf));
            if (BufLen == 0)
                break;
            r = BufLen;
        }
        if (n < r)
            r = n;
        movmem(Buf + BufIdx, buf, r);
        buf += r;
        n -= r;
        BufIdx += r;
        ttl += r;
    }
    return(ttl);
}

void
decomp_out(ubyte c)
{
    if (MemBytes) {
        *MemBuf++ = c;
        --MemBytes;
    }
}

/*
 *  NOTE NOTE NOTE This file is included by uucp:src/lib/comp.c to
 *  provide compression code, do not delete!
 */

/*
 * Copyright (c) 1985, 1986 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * James A. Woods, derived from original work by Spencer Thomas
 * and Joseph Orost.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that: (1) source distributions retain this entire copyright
 * notice and comment, and (2) distributions including binaries display
 * the following acknowledgement:  ``This product includes software
 * developed by the University of California, Berkeley and its contributors''
 * in the documentation or other materials provided with the distribution
 * and in all advertising materials mentioning features or use of this
 * software. Neither the name of the University nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * NOTE!!!!!!!!! This file is #include'd by src/lib/[un]comp.c
 */


#ifndef lint
static char copyright[] =
"@(#) Copyright (c) 1985, 1986 The Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)compress.c  5.12 (Berkeley) 6/1/90";
#endif /* not lint */

void error_decompress();
void error_compress();

/*
 * Compress - data compression program
 */
#define min(a,b)        ((a>b) ? b : a)

/*
 * a code_int must be able to hold 2**BITS values of type int, and also -1
 */

#if BITS > 15
typedef long int        code_int;
#else
typedef int             code_int;
#endif

typedef long int          count_int;

typedef unsigned char   char_type;
static char_type magic_header[] = { "\037\235" };      /* 1F 9D */

/* Defines for third byte of header */
#define BIT_MASK        0x1f
#define BLOCK_MASK      0x80
/* Masks 0x40 and 0x20 are free.  I think 0x20 should mean that there is
   a fourth header byte (for expansion).
*/
#define INIT_BITS 9                     /* initial number of bits/code */

/*
 * compress.c - File compression ala IEEE Computer, June 1984.
 *
 * Authors:     Spencer W. Thomas       (decvax!utah-cs!thomas)
 *              Jim McKie               (decvax!mcvax!jim)
 *              Steve Davies            (decvax!vax135!petsd!peora!srd)
 *              Ken Turkowski           (decvax!decwrl!turtlevax!ken)
 *              James A. Woods          (decvax!ihnp4!ames!jaw)
 *              Joe Orost               (decvax!vax135!petsd!joe)
 *
 */

void cl_hash(count_int);
void cl_block(void);
void output(code_int);
code_int getcode(void);



#define ARGVAL() (*++(*argv) || (--argc && *++argv))

static int n_bits;              /* number of bits/code */
static int maxbits;             /* user settable max # bits/code */
static code_int maxcode;        /* maximum code, given n_bits */
static code_int maxmaxcode;     /* should NEVER generate this code */

#define MAXCODE(n_bits)        ((1 << (n_bits)) - 1)

static count_int *htab[9];
static unsigned short *codetab[5];

#define htabof(i)       (htab[(i) >> 13][(i) & 0x1fff])
#define codetabof(i)    (codetab[(i) >> 14][(i) & 0x3fff])

static code_int hsize;  /* for dynamic table sizing */
static count_int fsize;
static int gc_offset = 0, gc_size = 0;

/*
 * To save much memory, we overlay the table used by compress() with those
 * used by decompress().  The tab_prefix table is the same size and type
 * as the codetab.  The tab_suffix table needs 2**BITS characters.  We
 * get this from the beginning of htab.  The output stack uses the rest
 * of htab, and contains characters.  There is plenty of room for any
 * possible stack (stack used to be 8000 characters).
 */

#define tab_prefixof(i)         codetabof(i)
#define tab_suffixof(i)        ((char_type *)htab[(i)>>15])[(i) & 0x7fff]

/*
 *  it looked wrong the way it was before, so de_stack is back to normal
 */

static char_type de_stack[24000];

static code_int free_ent = 0;                  /* first unused entry */
static int exit_stat = 0;                      /* per-file status */
static int perm_stat = 0;                      /* permanent status */

static int nomagic = 0;        /* Use a 3-byte magic number header, unless old file */
static int zcat_flg = 0;       /* Write output on stdout, suppress messages */
static int precious = 1;       /* Don't unlink output file on interrupt */
static int quiet = 1;          /* don't tell me about compression */

/*
 * block compression parameters -- after all codes are used up,
 * and compression rate changes, start over.
 */
static int block_compress = BLOCK_MASK;
static int clear_flg = 0;
static long int ratio = 0;
#define CHECK_GAP 10000 /* ratio check interval */
static count_int checkpoint = CHECK_GAP;
/*
 * the next two codes should not be changed lightly, as they must not
 * lie within the contiguous general code space.
 */
#define FIRST   257     /* first free entry */
#define CLEAR   256     /* table clear output code */

static int force = 0;
static char ofname [100];
static int bgnd_flag;

static int do_decomp = 0;

static int offset;
static long int in_count = 1;           /* length of input */
static long int bytes_out;              /* length of compressed output */
static long int out_count = 0;          /* # of codes output (for debugging) */


/*
 * compress stdin to stdout
 *
 * Algorithm:  use open addressing double hashing (no chaining) on the
 * prefix code / next character combination.  We do a variant of Knuth's
 * algorithm D (vol. 3, sec. 6.4) along with G. Knott's relatively-prime
 * secondary probe.  Here, the modular division first probe is gives way
 * to a faster exclusive-or manipulation.  Also do block compression with
 * an adaptive reset, whereby the code table is cleared when the compression
 * ratio decreases, but after the table fills.  The variable-length output
 * codes are re-sized at this point, and a special CLEAR code is generated
 * for the decompressor.  Late addition:  construct the table according to
 * file size for noticeable speed improvement on small files.  Please direct
 * questions about this implementation to ames!jaw.
 */

int
compress() {
    register long fcode;
    register code_int i = 0;
    register int c;
    register code_int ent;
    register code_int disp;
    register code_int hsize_reg;
    register int hshift;

    if (nomagic == 0) {
        comp_out(magic_header[0]); comp_out(magic_header[1]);
        comp_out((char)(maxbits | block_compress));
    }

    offset = 0;
    bytes_out = 3;              /* includes 3-byte header mojo */
    out_count = 0;
    clear_flg = 0;
    ratio = 0;
    in_count = 1;
    checkpoint = CHECK_GAP;
    maxcode = MAXCODE(n_bits = INIT_BITS);
    free_ent = ((block_compress) ? FIRST : 256 );

    ent = comp_in ();

    hshift = 0;
    for ( fcode = (long) hsize;  fcode < 65536L; fcode *= 2L )
        hshift++;
    hshift = 8 - hshift;                /* set hash code range bound */

    hsize_reg = hsize;
    cl_hash( (count_int) hsize_reg);            /* clear hash table */

    while ( (c = comp_in()) != EOF ) {
        in_count++;
        fcode = (long) (((long) c << maxbits) + ent);
        i = ((c << hshift) ^ ent);      /* xor hashing */

        if ( htabof (i) == fcode ) {
            ent = codetabof (i);
            continue;
        } else if ( (long)htabof (i) < 0 )      /* empty slot */
            goto nomatch;
        disp = hsize_reg - i;           /* secondary hash (after G. Knott) */
        if ( i == 0 )
            disp = 1;
probe:
        if ( (i -= disp) < 0 )
            i += hsize_reg;

        if ( htabof (i) == fcode ) {
            ent = codetabof (i);
            continue;
        }
        if ( (long)htabof (i) > 0 )
            goto probe;
nomatch:
        output ( (code_int) ent );
        out_count++;
        ent = c;
        if ( free_ent < maxmaxcode ) {
            codetabof (i) = free_ent++; /* code -> hashtable */
            htabof (i) =  fcode;
        }
        else if ( (count_int)in_count >= checkpoint && block_compress )
            cl_block ();
    }
    /*
     * Put out the final code.
     */
    output( (code_int)ent );
    out_count++;
    output( (code_int)-1 );

    if(bytes_out > in_count)    /* exit(2) if no savings */
        exit_stat = 2;
    return(ErrorCode);
}


/*****************************************************************
 * TAG( output )
 *
 * Output the given code.
 * Inputs:
 *      code:   A n_bits-bit integer.  If == -1, then EOF.  This assumes
 *              that n_bits =< (long)wordsize - 1.
 * Outputs:
 *      Outputs code to the file.
 * Assumptions:
 *      Chars are 8 bits long.
 * Algorithm:
 *      Maintain a BITS character long buffer (so that 8 codes will
 * fit in it exactly).  Use the VAX insv instruction to insert each
 * code in turn.  When the buffer fills up empty it and start over.
 */

static char buf[BITS];

#ifndef vax
static char_type lmask[9] = {0xff, 0xfe, 0xfc, 0xf8, 0xf0, 0xe0, 0xc0, 0x80, 0x00};
static char_type rmask[9] = {0x00, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff};
#endif /* vax */

void
output( code )
code_int  code;
{
    /*
     * On the VAX, it is important to have the register declarations
     * in exactly the order given, or the asm will break.
     */
    register int r_off = offset, bits= n_bits;
    register char * bp = buf;

    if ( code >= 0 ) {
/*
 * byte/bit numbering on the VAX is simulated by the following code
 */
        /*
         * Get to the first byte.
         */
        bp += (r_off >> 3);
        r_off &= 7;
        /*
         * Since code is always >= 8 bits, only need to mask the first
         * hunk on the left.
         */
        *bp = (*bp & rmask[r_off]) | (code << r_off) & lmask[r_off];
        bp++;
        bits -= (8 - r_off);
        code >>= 8 - r_off;
        /* Get any 8 bit parts in the middle (<=1 for up to 16 bits). */
        if ( bits >= 8 ) {
            *bp++ = code;
            code >>= 8;
            bits -= 8;
        }
        /* Last bits. */
        if(bits)
            *bp = code;

        offset += n_bits;
        if ( offset == (n_bits << 3) ) {
            bp = buf;
            bits = n_bits;
            bytes_out += bits;
            do
                comp_out(*bp++);
            while(--bits);
            offset = 0;
        }

        /*
         * If the next entry is going to be too big for the code size,
         * then increase it, if possible.
         */
        if ( free_ent > maxcode || (clear_flg > 0))
        {
            /*
             * Write the whole buffer, because the input side won't
             * discover the size increase until after it has read it.
             */
            if ( offset > 0 ) {
                comp_outn(buf, n_bits);
                bytes_out += n_bits;
            }
            offset = 0;

            if ( clear_flg ) {
                maxcode = MAXCODE (n_bits = INIT_BITS);
                clear_flg = 0;
            }
            else {
                n_bits++;
                if ( n_bits == maxbits )
                    maxcode = maxmaxcode;
                else
                    maxcode = MAXCODE(n_bits);
            }
        }
    } else {
        /*
         * At EOF, write the rest of the buffer.
         */
        if ( offset > 0 )
            comp_outn( buf, (offset + 7) / 8);
        bytes_out += (offset + 7) / 8;
        offset = 0;
    }
}

/*
 * Decompress stdin to stdout.  This routine adapts to the codes in the
 * file building the "string" table on-the-fly; requiring no table to
 * be stored in the compressed file.  The tables used herein are shared
 * with those of the compress() routine.  See the definitions above.
 */

int
decompress() {
    register char_type *stackp;
    register int finchar;
    register code_int code, oldcode, incode;

    /*
     * As above, initialize the first 256 entries in the table.
     */
    maxcode = MAXCODE(n_bits = INIT_BITS);
    for ( code = 255; code >= 0; code-- ) {
        tab_prefixof(code) = 0;
        tab_suffixof(code) = (char_type)code;
    }
    free_ent = ((block_compress) ? FIRST : 256 );

    finchar = oldcode = getcode();
    if(oldcode == -1)   /* EOF already? */
        return(-1);             /* Get out of here */
    decomp_out( (char)finchar );       /* first code must be 8 bits = char */
    if (ErrorCode)
        return(-1);
    stackp = de_stack;

    while ( (code = getcode()) > -1 ) {

        if ( (code == CLEAR) && block_compress ) {
            for ( code = 255; code >= 0; code-- )
                tab_prefixof(code) = 0;
            clear_flg = 1;
            free_ent = FIRST - 1;
            if ( (code = getcode ()) == -1 )    /* O, untimely death! */
                break;
        }
        incode = code;
        /*
         * Special case for KwKwK string.
         */
        if ( code >= free_ent ) {
            *stackp++ = finchar;
            code = oldcode;
        }

        /*
         * Generate output characters in reverse order
         */
        while ( code >= 256 ) {
            *stackp++ = tab_suffixof(code);
            code = tab_prefixof(code);

            if (stackp == de_stack + sizeof(de_stack)) {
                --stackp;
                return(-1);
            }
        }
        *stackp++ = finchar = tab_suffixof(code);

        /*
         * And put them out in forward order
         */
        do
            decomp_out ( *--stackp );
        while ( stackp > de_stack );

        /*
         * Generate the new entry.
         */
        if ( (code=free_ent) < maxmaxcode ) {
            tab_prefixof(code) = (unsigned short)oldcode;
            tab_suffixof(code) = finchar;
            free_ent = code+1;
        }
        /*
         * Remember previous code.
         */
        oldcode = incode;
    }
    return(ErrorCode);
}

/*****************************************************************
 * TAG( getcode )
 *
 * Read one code from the standard input.  If EOF, return -1.
 * Inputs:
 *      stdin
 * Outputs:
 *      code or -1 is returned.
 */


code_int
getcode() {
    /*
     * On the VAX, it is important to have the register declarations
     * in exactly the order given, or the asm will break.
     */
    register code_int code;
    static char_type buf[BITS];
    register int r_off, bits;
    register char_type *bp = buf;

    if ( clear_flg > 0 || gc_offset >= gc_size || free_ent > maxcode ) {
        /*
         * If the next entry will be too big for the current code
         * size, then we must increase the size.  This implies reading
         * a new buffer full, too.
         */
        if ( free_ent > maxcode ) {
            n_bits++;
            if ( n_bits == maxbits )
                maxcode = maxmaxcode;   /* won't get any bigger now */
            else
                maxcode = MAXCODE(n_bits);
        }
        if ( clear_flg > 0) {
            maxcode = MAXCODE (n_bits = INIT_BITS);
            clear_flg = 0;
        }
        gc_size = decomp_inn( buf, n_bits);
        if ( gc_size <= 0 )
            return -1;                  /* end of file */
        gc_offset = 0;
        /* Round size down to integral number of codes */
        gc_size = (gc_size << 3) - (n_bits - 1);
    }
    r_off = gc_offset;
    bits = n_bits;
        /*
         * Get to the first byte.
         */
        bp += (r_off >> 3);
        r_off &= 7;
        /* Get first part (low order bits) */
        code = (*bp++ >> r_off);
        bits -= (8 - r_off);
        r_off = 8 - r_off;              /* now, offset into code word */
        /* Get any 8 bit parts in the middle (<=1 for up to 16 bits). */
        if ( bits >= 8 ) {
            code |= *bp++ << r_off;
            r_off += 8;
            bits -= 8;
        }
        /* high order bits. */
        code |= (*bp & rmask[bits]) << r_off;
    gc_offset += n_bits;
    if (code >= maxmaxcode) {
        return(-1);
    }

    return code;
}

void
cl_block ()             /* table clear for block compress */
{
    register long int rat;

    checkpoint = in_count + CHECK_GAP;

    if(in_count > 0x007fffff) { /* shift will overflow */
        rat = bytes_out >> 8;
        if(rat == 0) {          /* Don't divide by zero */
            rat = 0x7fffffff;
        } else {
            rat = in_count / rat;
        }
    } else {
        rat = (in_count << 8) / bytes_out;      /* 8 fractional bits */
    }
    if ( rat > ratio ) {
        ratio = rat;
    } else {
        ratio = 0;
        cl_hash ( (count_int) hsize );
        free_ent = FIRST;
        clear_flg = 1;
        output ( (code_int) CLEAR );
    }
}

void
cl_hash(hsize)          /* reset code table */
        count_int hsize;
{
        long j;
        long k = hsize;
        count_int *htab_p;
        long i;
        long m1 = -1;

    for(j=0; j<=8 && k>=0; j++,k-=8192) {
        i = 8192;
        if(k < 8192) {
                i = k;
        }
        htab_p = &(htab[j][i]);
        i -= 16;
        if(i > 0) {
        do {                            /* might use Sys V memset(3) here */
                *(htab_p-16) = m1;
                *(htab_p-15) = m1;
                *(htab_p-14) = m1;
                *(htab_p-13) = m1;
                *(htab_p-12) = m1;
                *(htab_p-11) = m1;
                *(htab_p-10) = m1;
                *(htab_p-9) = m1;
                *(htab_p-8) = m1;
                *(htab_p-7) = m1;
                *(htab_p-6) = m1;
                *(htab_p-5) = m1;
                *(htab_p-4) = m1;
                *(htab_p-3) = m1;
                *(htab_p-2) = m1;
                *(htab_p-1) = m1;
                htab_p -= 16;
        } while ((i -= 16) >= 0);
        }
    }
        for ( i += 16; i > 0; i-- )
                *--htab_p = m1;
}

/*
 *  dynamic decompression tag
 */

int
initcomp(bits)
short bits;
{
    if (bits == 0) {
        if ((decomp_in()!=(magic_header[0] & 0xFF))
        || (decomp_in()!=(magic_header[1] & 0xFF))) {
                return(-1);
        }
        bits = decomp_in();  /* set -b from file */
    }

    block_compress = bits & BLOCK_MASK;
    bits &= BIT_MASK;
    maxmaxcode = ((code_int)1) << bits;
    maxbits = bits;

    /*
     *  some of these might be redundant
     */

    offset = clear_flg = ratio = 0;
    gc_offset = gc_size = 0;
    in_count = 1;
    checkpoint = CHECK_GAP;
    n_bits = INIT_BITS;
    maxcode = MAXCODE(INIT_BITS);
    free_ent = ((block_compress) ? FIRST : 256);

    /*
     *  set hsize
     */

    switch(bits) {
    case 16:
        hsize = 69001;
        break;
    case 15:
        hsize = 35023;
        break;
    case 14:
        hsize = 18013;
        break;
    case 13:
        hsize = 9001;
        break;
    default:
        hsize = 5003;       /*  note: de_stack still fits   */
        break;
    }
    {
        int i;
        long n;

        for (i = 0, n = hsize; i < 9 && n > 0; ++i) {
            long entries = (n > 8192) ? 8192 : n;

            if ((htab[i] = calloc(1, sizeof(count_int) * entries)) == NULL)
                return(-1);
            n -= entries;
        }

        for (i = 0, n = hsize; i < 5 && n > 0; ++i) {
            long entries = (n > 16384) ? 16384 : n;

            if ((codetab[i] = calloc(1, sizeof(short) * entries)) == NULL)
                return(-1);
            n -= entries;
        }
    }
    return(0);
}

void
deletecomp(void)
{
    int i;

    for (i = 0; i < sizeof(htab)/sizeof(htab[0]); ++i) {
        if (htab[i]) {
            free(htab[i]);
            htab[i] = NULL;
        }
    }
    for (i = 0; i < sizeof(codetab)/sizeof(codetab[0]); ++i) {
        if (codetab[i]) {
            free(codetab[i]);
            codetab[i] = NULL;
        }
    }
}


