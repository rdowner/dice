/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  CODE.C
 */

#include "defs.h"

#define DISOBJ_C

#ifdef AMIGA
#include "/das/ops.c"
#else
#include "../das/ops.c"
#endif

OpCod	*BaseCod;

Prototype void InitCodeList(void);
Prototype void DumpCode(FILE *fi, short srcHunk, int32_t begOffset, int32_t endOffset);
Prototype RelocInfo *RelocAtOffset(int32_t offset, short hunkNo);
Prototype OpCod *DecodeOpCode(uword opCode, short *pmode1, short *pmode2, short *psiz, short *prs, short *prd);
Prototype int MatchEa(uword opCode, short ea, short reg, int32_t modes);
Prototype int MatchSiz(uword opCode, OpCod *oc);
Prototype int ExtensionWords(OpCod *oc, short mode, short opsize, uword *oper, FILE *fi);
Prototype int IndexFormatExtWords(uword ext);
Prototype char *ModeToStr(int32_t offset, short srcHunk, uword opCode, uword **pwp, short mode, int altMode, short siz, short reg, short special);

/*
 *  Reduce search requirements by splitting opcodes into a set of 16
 *  lists based on their msb 4 bits (which never have dynamic info in them)
 */

void
InitCodeList(void)
{
    OpCod *oc;

    for (oc = Codes; oc->OpName; ++oc) {
	if (oc->Id < 0)
	    continue;
	oc->SibNext = BaseCod;
	BaseCod = oc;
    }
}

void
DumpCode(FILE *fi, short srcHunk, int32_t begOffset, int32_t endOffset)
{
    RelocInfo *r;

    while (begOffset < endOffset) {
	uword	opCode;
	static uword   oper[32];
	short	words1;
	short	words2;
	short	i;
	short	col;
	OpCod	*oc;
	short	mode1;
	short	mode2;
	short	siz;
	short	rs;
	short	rd;

	col = cprintf(" %02x.%08x ", srcHunk, begOffset + StartPc);

	if ((r = RelocAtOffset(begOffset, srcHunk)) != NULL) {
	    int32_t data = LoadRelocData(fi, r);
	    col += cprintf(" %s\n", RelocToStr(r, data, 1, 0, -1));
	    begOffset += r->ri_RelocSize;
	    continue;
	}
	if (freadl(&opCode, 2, 1, fi) != 1)
	    cerror(EFATAL, "Unexpected EOF");
	begOffset += 2;

	col += cprintf(" %04x", opCode);

	oc = DecodeOpCode(opCode, &mode1, &mode2, &siz, &rs, &rd);
	if (oc) {
	    words1 = ExtensionWords(oc, mode1, siz, oper, fi);
	    words2 = ExtensionWords(oc, mode2, siz, oper + words1, fi);
	} else {
	    words1 = 0;
	    words2 = 0;
	}

	for (i = 0; i < words1 + words2; ++i) {
	    r = RelocAtOffset(begOffset + i * 2, srcHunk);
	    if (r) {
		char *str;

		switch(r->ri_RelocSize) {
		case 1:
		    str = RelocToStr(r, oper[i] >> 8, 1, 1, -1);
		    break;
		case 2:
		    str = RelocToStr(r, oper[i], 1, 2, -1);
		    break;
		case 4:
		    str = RelocToStr(r, (oper[i] << 16) | oper[i+1], 1, 4, -1);
		    ++i;
		    break;
		default:
		    str = RelocToStr(r, 0, 1, 0, -1);
		    break;
		}
		col += cprintf(" %s", str);
	    } else {
		if ((r = RelocAtOffset(begOffset + i * 2 + 1, -1)) != NULL) {
		    if (r->ri_RelocSize != 1)
			cerror(EWARN, "Expected 8 bit reloc");
		    col += cprintf(" %s", RelocToStr(r, oper[i] & 0xFF, 1, 2, -1));
		} else {
		    col += cprintf(" %04x", oper[i]);
		}
	    }
	}

	++col;
	putc(' ', stdout);
	while (col < 40) {
	    ++col;
	    putc(' ', stdout);
	}

	/*
	 *  decode effective addresses
	 */

	if (oc) {
	    col += cprintf("%s", oc->OpName);

	    col += 3;

	    switch(siz) {
	    case 0:
		printf("   ");
		break;
	    case 1:
		printf(".B ");
		break;
	    case 2:
		printf(".W ");
		break;
	    case 4:
		printf(".L ");
		break;
	    default:
		printf(".? ");
		break;
	    }
	    while (col < 48) {
		putc(' ', stdout);
		++col;
	    }
	    {
		uword *iptr = oper;	/*  ptr to beginning of extension */

		if (mode2 == AB_REGS)
		    ++iptr;
		if (mode1)
		    printf("%s", ModeToStr((iptr - oper) * 2 + begOffset, srcHunk, opCode, &iptr, mode1, mode2, siz, rs, oc->Special));
		if (mode1 && mode2)
		    printf(",");
		if (mode2) {
		    if (mode2 == AB_REGS)
			iptr = oper;
		    printf("%s", ModeToStr((iptr - oper) * 2 + begOffset + words1, srcHunk, opCode, &iptr, mode2, mode1, siz, rd, oc->Special));
		}
	    }
	}
	puts("");
	begOffset += (words1 + words2) * 2;
    }
    if (begOffset > endOffset)
	fseek(fi, endOffset - begOffset, SEEK_CUR);
}

RelocInfo *
RelocAtOffset(int32_t offset, short hunkNo)
{
    RelocInfo *r = FindRelocOffset(offset, hunkNo);

    if (r && r->ri_SrcOffset == offset)
	return(r);
    return(NULL);
}

/*
 *  returns # of extension words and fills up ea structures
 *  accordingly.
 */

OpCod *
DecodeOpCode(uword opCode, short *pmode1, short *pmode2, short *psiz, short *prs, short *prd)
{
    OpCod *oc;

    *pmode1 = 0;
    *pmode2 = 0;
    *psiz = 0;

    /*
     *	find op-code
     */

    for (oc = BaseCod; oc; oc = oc->SibNext) {
	short am;

	{
	    uword template;

	    if ((opCode & oc->Template) != oc->Template)
		continue;

	    if ((oc->SModes & AF_BBRANCH) && (char)opCode == 0)
		continue;
	    if ((oc->SModes & AF_WBRANCH) && (char)opCode)
		continue;

	    template = opCode;

	    if (oc->SModes & (AF_BBRANCH|AF_WBRANCH))
		template &= 0xFF00;

	    switch(oc->Special) {
	    case 0:
	    case IMM07: /*  unused  */
		break;
	    case IMM18:
		template &= 0xF1FF;
		break;
	    case IMM0F:
		template &= 0xFFF0;
		break;
	    case IMM256:
		template &= 0xFF00;
		break;
	    case IMMBR: /*  unused  */
	    case IMMB:
		break;
	    default:
		cerror(ESOFT, "Unknown special code %s/%d", oc->OpName, oc->Special);
	    }

	    if (oc->B_EAs >= 0)
		template &= ~(7 << oc->B_EAs);
	    if (oc->B_Rs >= 0)
		template &= ~(7 << oc->B_Rs);
	    if (oc->B_EAd >= 0)
		template &= ~(7 << oc->B_EAd);
	    if (oc->B_Rd >= 0)
		template &= ~(7 << oc->B_Rd);
	    if (oc->B_Siz >= 0)
		template &= ~(3 << oc->B_Siz);

	    if (template != oc->Template) {
		if (DDebug)
		    printf("Template mismatch %04x %04x (%d %d %d %d %d)\n",
			template, oc->Template, oc->B_EAs, oc->B_Rs, oc->B_EAd, oc->B_Rd, oc->B_Siz
		    );
		continue;
	    }
	}
	if (DDebug)
	    printf("template match %s\n", oc->OpName);

	am = MatchEa(opCode, oc->B_EAs, oc->B_Rs, oc->SModes);
	if (DDebug)
	    printf("r = %d\n", am);
	if (am < 0)
	    continue;
	if (oc->B_Rs >= 0)
	    *prs = (opCode >> oc->B_Rs) & 7;
	*pmode1 = am;

	am = MatchEa(opCode, oc->B_EAd, oc->B_Rd, oc->DModes);
	if (DDebug)
	    printf("r = %d\n", am);
	if (am < 0)
	    continue;
	if (oc->B_Rd >= 0)
	    *prd = (opCode >> oc->B_Rd) & 7;
	*pmode2 = am;

	am = MatchSiz(opCode, oc);
	if (am < 0)
	    continue;
	*psiz = am;
	break;
    }
    return(oc);
}

int
MatchEa(uword opCode, short ea, short reg, int32_t modes)
{
    short i = 0;

    if (DDebug)
	printf("matchea %04x %d %d %08x\n", opCode, ea, reg, modes);
    if (ea >= 0) {
	if (reg >= 0) {
	    reg = (opCode >> reg) & 7;
	} else {
	    reg = 0;
	}
	ea = (opCode >> ea) & 7;
	switch(ea) {
	case 0:
	    i = AB_DN;
	    break;
	case 1:
	    i = AB_AN;
	    break;
	case 2:
	    i = AB_INDAN;
	    break;
	case 3:
	    i = AB_INDPP;
	    break;
	case 4:
	    i = AB_MMIND;
	    break;
	case 5:
	    i = AB_OFFAN;
	    break;
	case 6:
	    i = AB_OFFIDX;
	    break;
	case 7:
	    switch(reg) {
	    case 0:
		i = AB_ABSW;
		break;
	    case 1:
		i = AB_ABSL;
		break;
	    case 2:
		i = AB_OFFPC;
		break;
	    case 3:
		i = AB_OFFIDXPC;
		break;
	    case 4:
		i = AB_IMM;
		break;
	    default:
		return(-1);
	    }
	}
	if ((modes & (1 << i)) == 0)
	    i = -1;
    } else if (modes) {
	for (i = 0; i < 32; ++i) {
	    if (modes - (1 << i) == 0)
		break;
	}
    }
    return(i);
}

int
MatchSiz(uword opCode, OpCod *oc)
{
    short siz = oc->B_Siz;
    short sizes = oc->Sizes;

    if (siz >= 0) {
	switch((opCode >> siz) & 3) {
	case 0:
	    sizes &= S_B;
	    break;
	case 1:
	    sizes &= S_W;
	    break;
	case 2:
	    sizes &= S_L;
	    break;
	case 3:
	    return(-1);
	}
    }
    switch(sizes) {
    case S_B:
	return(1);
    case S_W:
	return(2);
    case S_L:
	return(4);
    }
    if (sizes == 0)
	return(0);
    cerror(EFATAL, "Software error, opcode/siz %04x %d %04x\n", opCode, siz, sizes);
    return(-1);
}

int
ExtensionWords(OpCod *oc, short mode, short opsize, uword *oper, FILE *fi)
{
    short i = ExtWords[mode];

    if (i == -1) {    /*  imm/branch */
	if (oc->Special == IMMB)
	    i = 1;
	else if (oc->Special || (oc->SModes & AF_BBRANCH))
	    i = 0;
	else
	    i = (opsize + 1) >> 1;  /* 1 for bw, 2 for int32_t */
	if (i)
	    freadl(oper, 2, i, fi);
    } else if (i == -2) {
	freadl(oper, 2, 1, fi);
	i = IndexFormatExtWords(*oper);
	if (i > 1)
	    freadl(oper + 1, 2, i - 1, fi);
    } else if (i) {
	freadl(oper, 2, i, fi);
    }
    return(i);
}

int
IndexFormatExtWords(uword ext)
{
    int n = 1;

    if (ext & 0x0100) {
	switch(ext & 0x0030) {	    /*	base displacement   */
	case 0x0000:
	case 0x0010:
	    break;
	case 0x0020:
	    ++n;
	    break;
	case 0x0030:
	    ++n;
	    ++n;
	    break;
	}
	switch(ext & 0x0003) {	    /*	outder displacement */
	case 0x0000:
	case 0x0001:
	    break;
	case 0x0002:
	    ++n;
	    break;
	case 0x0003:
	    ++n;
	    ++n;
	    break;
	}
    }
    return(n);
}

/*
 *  offset is the offset of the first extension word for this ea.
 */

char *
ModeToStr(int32_t offset, short srcHunk, uword opCode, uword **pwp, short mode, int altMode, short siz, short reg, short special)
{
    uword *wptr = *pwp;
    RelocInfo *r = NULL;
    static char Buf[SMAX_BUF+128];


    switch(mode) {
    case AB_DN:
	sprintf(Buf, "D%d", reg);
	break;
    case AB_AN:
	sprintf(Buf, "A%d", reg);
	break;
    case AB_INDAN:
	sprintf(Buf, "(A%d)", reg);
	break;
    case AB_INDPP:
	sprintf(Buf, "(A%d)+", reg);
	break;
    case AB_MMIND:
	sprintf(Buf, "-(A%d)", reg);
	break;
    case AB_OFFAN:
	r = RelocAtOffset(offset, srcHunk);
	sprintf(Buf, "%s(A%d)", RelocToStr(r, (short)wptr[0], 0, 2, -1), reg);
	++wptr;
	break;
    case AB_OFFIDX:
    case AB_OFFIDXPC:
	{
	    char regbuf[4];	/*  base register   */
	    char idxbuf[8];	/*  idx  register   */
	    uword ext = *wptr;

	    if (mode == AB_OFFIDX) {
		regbuf[0] = 'A';
		regbuf[1] = reg + '0';
	    } else {
		regbuf[0] = 'p';
		regbuf[1] = 'c';
	    }
	    regbuf[2] = 0;

	    {
		idxbuf[0] = (ext & 0x8000) ? 'A' : 'D';
		idxbuf[1] = ((ext >> 12) & 7) + '0';
		idxbuf[2] = '.';
		idxbuf[3] = (ext & 0x0800) ? 'L' : 'W';
		idxbuf[4] = 0;

		switch(ext & 0x0600) {
		case 0x0200:
		    strcpy(idxbuf + 4, "*2");
		    break;
		case 0x0400:
		    strcpy(idxbuf + 4, "*4");
		    break;
		case 0x0600:
		    strcpy(idxbuf + 4, "*8");
		    break;
		}
	    }
	    if (ext & 0x0100) {       /*  MC68020 full format */
		char *ptr = Buf;
		short hda = 0;

		++wptr;
		offset += 2;	      /*  skip first extension word */

		*ptr++ = '(';
		if (ext & 0x0007)
		    *ptr++ = '[';

		switch(ext & 0x0030) {
		case 0x0000:
		    ptr += csprintf(ptr, "<BADBD>");
		    hda = 1;
		    break;
		case 0x0010:	/*  null    */
		    break;
		case 0x0020:	/*  word    */
		    r = RelocAtOffset(offset, srcHunk);
		    ptr += csprintf(ptr, "%s", RelocToStr(r, (short)*wptr, 0, 2, (mode == AB_OFFIDXPC) ? srcHunk : -1));
		    ++wptr;
		    offset += 2;
		    hda = 1;
		    break;
		case 0x0030:	/*  int32_t    */
		    r = RelocAtOffset(offset, srcHunk);
		    ptr += csprintf(ptr, "%s", RelocToStr(r, *(int32_t *)wptr, 0, 4, (mode == AB_OFFIDXPC) ? srcHunk : -1));
		    ++wptr;
		    ++wptr;
		    offset += 4;
		    hda = 1;
		    break;
		}
		if (hda) {
		    *ptr++ = ',';
		    hda = 0;
		}
		if ((ext & 0x0080) == 0) {
		    ptr += csprintf(ptr, "%s", regbuf);
		    hda = 1;
		}
		/*
		 *  if indirect post-indexed close bracket
		 */
		if ((ext & 0x0044) == 0x0004) {
		    *ptr++ = ']';
		    hda = 1;
		}
		if (hda) {
		    *ptr++ = ',';
		    hda = 0;
		}

		/*
		 *  index reg
		 */

		if ((ext & 0x0040) == 0) {
		    ptr += csprintf(ptr, "%s", idxbuf);
		    hda = 1;
		}

		if ((ext & 0x0044) != 0x0004) {     /*	not ind post indexed */
		    if (ext & 0x0007) {
			*ptr++ = ']';
			hda = 1;
		    }
		}

		/*
		 *  outer displacement
		 */

		switch(ext & 0x0003) {
		case 0x0000:
		case 0x0001:	/*  null    */
		    break;
		case 0x0002:	/*  word    */
		    r = RelocAtOffset(offset, srcHunk);
		    ptr += csprintf(ptr, "%s", RelocToStr(r, (short)*wptr, 0, 2, -1));
		    ++wptr;
		    hda = 1;
		    break;
		case 0x0003:	/*  int32_t    */
		    r = RelocAtOffset(offset, srcHunk);
		    ptr += csprintf(ptr, "%s", RelocToStr(r, *(int32_t *)wptr, 0, 4, -1));
		    ++wptr;
		    ++wptr;
		    hda = 1;
		    break;
		}
		*ptr++ = ')';
		*ptr = 0;
	    } else {			/*  MC68000/020 brief	*/
		r = RelocAtOffset(offset, srcHunk);
		if (r == NULL)
		    r = RelocAtOffset(offset + 1, srcHunk);
		sprintf(Buf, "%s(%s,%s)", RelocToStr(r, (char)ext, 0, 1, (mode == AB_OFFIDXPC) ? srcHunk : -1), regbuf, idxbuf);
		++wptr;
	    }
	}
	break;
    case AB_ABSW:
	r = RelocAtOffset(offset, srcHunk);
	sprintf(Buf, "%s.W", RelocToStr(r, wptr[0], 0, 2, srcHunk));
	++wptr;
	break;
    case AB_ABSL:
	r = RelocAtOffset(offset, srcHunk);
	if (r == NULL)
	    r = RelocAtOffset(offset + 2, srcHunk);
	sprintf(Buf, "%s", RelocToStr(r, (wptr[0] << 16) | wptr[1], 0, 4, srcHunk));
	wptr += 2;
	break;
    case AB_OFFPC:
	{
	    int32_t addoff;

	    /*
	     *	Attempt to do the relocation.  If we can get our hands on
	     *	a symbol we are in good shape.
	     */

	    r = RelocAtOffset(offset, srcHunk);
	    addoff = FixRelocOffset(r, offset);

	    sprintf(Buf, "%s(pc)", RelocToStr(r, (short)wptr[0] + addoff, 0, 0, srcHunk));
	}
	++wptr;
	break;
    case AB_IMM:
	switch(special) {
	case IMM18:
	    {
		short cnt = (opCode >> 9) & 7;

		if (cnt == 0)
		    cnt = 8;
		sprintf(Buf, "#%d", cnt);
	    }
	    break;
	case IMM0F:
	    sprintf(Buf, "#%d", opCode & 15);   /*  warning, decimal    */
	    break;
	case IMM256:
	    r = RelocAtOffset(offset - 1, srcHunk);
	    if (r == NULL)
		r = RelocAtOffset(offset - 2, srcHunk);
	    sprintf(Buf, "#%s", RelocToStr(r, (char)opCode, 0, 1, -1));
	    break;
	case IMMB:  /*	next byte   */
	    r = RelocAtOffset(offset, srcHunk);
	    if (r == NULL)
		r = RelocAtOffset(offset + 1, srcHunk);
	    sprintf(Buf, "#%s", RelocToStr(r, (char)wptr[0], 0, 1, -1));
	    ++wptr;
	    break;
	default:
	    r = RelocAtOffset(offset, srcHunk);
	    switch(siz) {
	    case 0:
		sprintf(Buf, "#?");
		break;
	    case 1:
		if (r == NULL)
		    r = RelocAtOffset(offset + 1, srcHunk);
		sprintf(Buf, "#%s", RelocToStr(r, (short)wptr[0], 0, 1, -1));
		++wptr;
		break;
	    case 2:
		sprintf(Buf, "#%s", RelocToStr(r, (short)wptr[0], 0, 2, -1));
		++wptr;
		break;
	    case 4:
		sprintf(Buf, "#%s", RelocToStr(r, (wptr[0] << 16) | wptr[1], 0, 4, -1));
		wptr += 2;
		break;
	    default:
		sprintf(Buf, "#?");
		break;
	    }
	    break;
	}
	break;
    case AB_REGS:
	{
	    short i;
	    short j = 0;
	    short beg = -1;

	    for (i = 0; i < 16; ++i) {
		short regNo;

		if (altMode == AB_MMIND)
		    regNo = wptr[0] & (1 << (15 - i));
		else
		    regNo = wptr[0] & (1 << i);

		if (regNo) {
		    if (beg == -2)
			j += csprintf(Buf + j, "/");
		    if (beg < 0) {
			if (i < 8)
			    j += csprintf(Buf + j, "D%d", i);
			else
			    j += csprintf(Buf + j, "A%d", i - 8);
			beg = i;
		    }
		}
		if (regNo == 0 || i == 8) {
		    if (beg >= 0) {
			--i;
			if (beg < i) {
			    if (i < 8)
				j += csprintf(Buf + j, "-D%d", i);
			    else
				j += csprintf(Buf + j, "-A%d", i - 8);
			}
			++i;
			beg = -2;
		    }
		}
	    }
	}
	++wptr;
	break;
    case AB_BBRANCH:
    case AB_WBRANCH:
	{
	    int32_t addoff;

	    if (opCode & 0xFF) {
		r = RelocAtOffset(offset - 1, srcHunk);
		addoff = FixRelocOffset(r, offset);

		if (r == NULL) {
		    r = RelocAtOffset(offset - 2, srcHunk);
		    addoff = FixRelocOffset(r, offset);
		}
		sprintf(Buf, "%s", RelocToStr(r, (char)opCode + addoff, 0, 4, srcHunk));
	    } else {
		r = RelocAtOffset(offset, srcHunk);
		addoff = FixRelocOffset(r, offset);

		sprintf(Buf, "%s", RelocToStr(r, (short)wptr[0] + addoff, 0, 4, srcHunk));
		/* XXX ? */
		++wptr;
	    }
	}
	break;
    case AB_CCR:
	sprintf(Buf, "CCR");
	break;
    case AB_SR:
	sprintf(Buf, "SR");
	break;
    case AB_USP:
	sprintf(Buf, "USP");
	break;
    default:
	return("?");
    }
    *pwp = wptr;
    return(Buf);
}

