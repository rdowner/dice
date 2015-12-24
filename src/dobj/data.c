/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  DATA.C
 */

#include "defs.h"

Prototype void DumpData(FILE *fi, short srcHunk, int32_t begOffset, int32_t endOffset);
Prototype void DumpHex(FILE *fi, short srcHunk, int32_t begOffset, int32_t endOffset);

void
DumpData(FILE *fi, short srcHunk, int32_t begOffset, int32_t endOffset)
{
    RelocInfo *r;
    int32_t data;

    for (r = FindRelocOffset(begOffset, srcHunk); r && r->ri_SrcOffset < endOffset; r = FindRelocNext(r)) {
	DumpHex(fi, srcHunk, begOffset, r->ri_SrcOffset);

	if (r->ri_SrcOffset < begOffset)
	    cerror(EFATAL, "Software Error, Reloc-Offset");

	data = LoadRelocData(fi, r);
	printf(" %02x.%08x  %s\n", srcHunk, (int32_t)(r->ri_SrcOffset + StartDo), RelocToStr(r, data, 1, 0, -1));
	begOffset = r->ri_SrcOffset + r->ri_RelocSize;
    }
    if (begOffset != endOffset)
	DumpHex(fi, srcHunk, begOffset, endOffset);
}

void
DumpHex(FILE *fi, short srcHunk, int32_t begOffset, int32_t endOffset)
{
    int32_t index = 0;

    if (begOffset == endOffset)
	return;

    while (begOffset < endOffset) {
	if ((index & 15) == 0)
	    printf(" %02x.%08x ", srcHunk, (int32_t)(begOffset + StartDo));

	if (begOffset + 1 == endOffset) {
	    ubyte c = -1;

	    fread(&c, 1, 1, fi);
	    printf(" %02x", c);
	    ++begOffset;
	    ++index;
	} else {
	    uword c = -1;

	    freadl(&c, 2, 1, fi);
	    printf(" %04x", c);
	    begOffset += 2;
	    index += 2;
	}
	if ((index & 15) == 0)
	    puts("");
    }
    if (index & 15)
	puts("");
}

