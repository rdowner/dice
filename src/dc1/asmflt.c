/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  ASMFLT.C
 */

#include "defs.h"
#include "asm.h"
#undef abs
#define abs   dummy	/*  bug in libraries/mathffp.h	  */
#ifdef AMIGA
#include <exec/types.h>
#include <exec/libraries.h>
#include <clib/exec_protos.h>
#endif

#ifdef NOTDEF
#include <proto/mathffp.h>
#include <proto/mathieeesingbas.h>
#include <proto/mathieeedoubbas.h>
#include <proto/mathieeedoubtrans.h>
#endif

#include "ieee.h"

extern __stkargs int32_t afp(char *);

typedef struct Library Library;

/*
 *  warning, lib not automatically openned for lc_ version.  Thus, no
 *  floating pt may be used in the compiler.  Also, we do not want the
 *  compiler constant fp stuff to depend on the amiga libraries.
 */

char MathMode = 'a';

static char TmpBuf[256];

Prototype void asm_fpadd(Exp *, Type *, Stor *, Stor *, Stor *);
Prototype void asm_fpsub(Exp *, Type *, Stor *, Stor *, Stor *);
Prototype void asm_fpmul(Exp *, Type *, Stor *, Stor *, Stor *);
Prototype void asm_fpdiv(Exp *, Type *, Stor *, Stor *, Stor *);
Prototype void asm_fpneg(Exp *, Type *, Stor *, Stor *);
Prototype void asm_fpcmp(Exp *, Stor *, Stor *, short *);
Prototype void asm_fptest(Exp *, Stor *);
Prototype void asm_fptoint(Exp *, Stor *, Stor *);
Prototype void asm_inttofp(Exp *, Stor *, Stor *);
Prototype void asm_fptofp(Exp *, Stor *, Stor *);
Prototype void asm_fltconst(Exp *, Stor *, int32_t *);
Prototype void CallFPSupport(Exp *, int32_t, Stor *, Stor *, Stor *, char *, short);
Prototype void asm_layoutfpconst(Exp *, Stor *, Stor *);

Prototype void ConstFpNeg(Exp *, Stor *, Stor *);
Prototype void ConstFpAdd(Exp *, Stor *, Stor *, Stor *);
Prototype void ConstFpSub(Exp *, Stor *, Stor *, Stor *);
Prototype void ConstFpMul(Exp *, Stor *, Stor *, Stor *);
Prototype void ConstFpDiv(Exp *, Stor *, Stor *, Stor *);
Prototype void CloseLibsExit(void);

Library *MathIeeeDoubTransBase;
Library *MathIeeeDoubBasBase;
Library *MathIeeeSingTransBase;
Library *MathIeeeSingBasBase;
Library *MathBase;
Library *MathTransBase;

Local void TmpFpAdd(TmpFlt *, TmpFlt *);

void
asm_fpadd(exp, t, s1, s2, d)
Exp *exp;
Type *t;
Stor *s1, *s2, *d;
{
    CallFPSupport(exp, *t->Size, s1, s2, d, "add", -1);
}

void
asm_fpsub(exp, t, s1, s2, d)
Exp *exp;
Type *t;
Stor *s1, *s2, *d;
{
    CallFPSupport(exp, *t->Size, s1, s2, d, "sub", 1);
}

void
asm_fpmul(exp, t, s1, s2, d)
Exp *exp;
Type *t;
Stor *s1, *s2, *d;
{
    CallFPSupport(exp, *t->Size, s1, s2, d, "mul", -1);
}

void
asm_fpdiv(exp, t, s1, s2, d)
Exp *exp;
Type *t;
Stor *s1, *s2, *d;
{
    CallFPSupport(exp, *t->Size, s1, s2, d, "div", 1);
}

void
asm_fpneg(exp, t, s, d)
Exp *exp;
Type *t;
Stor *s, *d;
{
    Assert(t);
    /*printf("type %08lx id %d\n", t, t->Id);*/
    CallFPSupport(exp, *t->Size, s, NULL, d, "neg", 1);
}

void
asm_fpcmp(exp, s1, s2, pcond)
Exp *exp;
Stor *s1, *s2;
short *pcond;
{
    CallFPSupport(exp, s1->st_Size, s1, s2, NULL, "cmp", 1);
}

void
asm_fptest(exp, s)
Exp *exp;
Stor *s;
{
    CallFPSupport(exp, s->st_Size, s, NULL, NULL, "tst", 1);
}

void
asm_fptoint(exp, s, d)
Exp *exp;
Stor *s;
Stor *d;
{
    Stor rd0;

    CallFPSupport(exp, s->st_Size, s, NULL, NULL, "fix", 1);
    AllocDataRegisterAbs(&rd0, 4, RB_D0);
    rd0.st_Size = d->st_Size;
    asm_move(exp, &rd0, d);
    FreeRegister(&rd0);
}

void
asm_inttofp(exp, s, d)
Exp *exp;
Stor *s;
Stor *d;
{
    Stor rd0;
    AllocDataRegisterAbs(&rd0, 4, RB_D0);
    asm_ext(exp, s, &rd0, s->st_Flags);
    FreeRegister(&rd0);
    CallFPSupport(exp, d->st_Size, NULL, NULL, d, "flt", 1);
}

void
asm_fptofp(exp, s, d)
Exp *exp;
Stor *s;
Stor *d;
{
    char name[16];

    if (s->st_Size == d->st_Size) {
	asm_move(exp, s, d);
	return;
    }

    switch(s->st_Size) {
    case 4:
	if (FFPOpt)
	    strcpy(name, "ffp");
	else
	    strcpy(name, "sp");
	break;
    case 8:
	strcpy(name, "dp");
	break;
    case 16:
	strcpy(name, "xp");
	break;
    default:
	dbprintf(("illegal/unsupported fp-fp conversion"));
	Assert(0);
	break;
    }

    strcat(name, "to");

    switch(d->st_Size) {
    case 4:
	if (FFPOpt)
	    strcat(name, "ffp");
	else
	    strcat(name, "sp");
	break;
    case 8:
	strcat(name, "dp");
	break;
    case 16:
	strcat(name, "xp");
	break;
    default:
	dbprintf(("illegal/unsupported fp-fp conversion"));
	Assert(0);
	break;
    }

    CallFPSupport(exp, 99, s, NULL, d, name, 1);
}

void
CloseLibsExit(void)
{
#ifdef AMIGA
    if (MathIeeeDoubTransBase) {
	CloseLibrary((void *)MathIeeeDoubTransBase);
	MathIeeeDoubTransBase = 0;
    }
    if (MathIeeeDoubBasBase) {
	CloseLibrary((void *)MathIeeeDoubBasBase);
	MathIeeeDoubBasBase = 0;
    }
    if (MathIeeeSingTransBase) {
	CloseLibrary((void *)MathIeeeSingTransBase);
	MathIeeeSingTransBase = 0;
    }
    if (MathIeeeSingBasBase) {
	CloseLibrary((void *)MathIeeeSingBasBase);
	MathIeeeSingBasBase = 0;
    }
    if (MathBase) {
	CloseLibrary((void *)MathBase);
	MathBase = 0;
    }
    if (MathTransBase) {
	CloseLibrary((void *)MathTransBase);
	MathTransBase = 0;
    }
#endif
}

/*
 *  lay down an fp value according to size and format and modify the storage
 *  accordingly.
 */

#ifdef AMIGA

void
asm_fltconst(exp, s, ary)
Exp *exp;
Stor *s;
int32_t *ary;
{
    static int LibsOpen = 0;

    ary[0] = 0;
    ary[1] = 0;

    switch(LibsOpen) {
    case 0:
	{
	    int error = 0;
	    int xerr;

	    if (FFPOpt == 0) {
		if ((MathIeeeSingBasBase = (void *)OpenLibrary("mathieeesingbas.library", 0)) == NULL)
		    ++error;
		if ((MathIeeeSingTransBase= (void *)OpenLibrary("mathieeesingtrans.library", 0)) == NULL)
		    ++error;
		if (error) {
		    zerror(EERROR_CANT_OPEN_MATHLIB, "mathieeesing[bas,trans].library");
		}
	    } else {
		if ((MathBase = (void *)OpenLibrary("mathffp.library", 0)) == NULL)
		    ++error;
		if ((MathTransBase = (void *)OpenLibrary("mathtrans.library", 0)) == NULL)
		    ++error;
		if (error)
		    zerror(EERROR_CANT_OPEN_MATHLIB, "math[ffp,trans].library");
	    }
	    xerr = error;
	    if ((MathIeeeDoubBasBase = (void *)OpenLibrary("mathieeedoubbas.library", 0)) == NULL)
		++error;
	    if ((MathIeeeDoubTransBase = (void *)OpenLibrary("mathieeedoubtrans.library", 0)) == NULL)
		++error;

	    if (error)
		LibsOpen = 2;
	    else
		LibsOpen = 1;
	    if (xerr != error)
		zerror(EERROR_CANT_OPEN_MATHLIB, "mathieeedoub[bas,trans].library");
	}
	break;
    case 1:
	break;
    case 2:
	return;
    }

    switch(s->st_Size) {
    case 4:
	strncpy(TmpBuf, s->st_FltConst, s->st_FltLen);
	TmpBuf[s->st_FltLen] = 0;

	if (FFPOpt == 0) {
	    float acc;	       /*  accumulated value   */
	    float ten = IEEESPFlt(10);
	    short x;

	    /*
	     *	<TmpBuf> x 10^X    Convert a digit at a time, exponent
	     *	counts digits
	     */

	    x = FPrefix(exp, s->st_FltConst, s->st_FltLen, TmpBuf);

	    acc = IEEESPFlt(0);
	    {
		char c;
		short i;

		for (i = 0; (c = TmpBuf[i+1]) && i < 9; ++i) {
		    acc = IEEESPAdd(IEEESPMul(acc, ten), IEEESPFlt(c - '0'));
		    --x;
		}
	    }
	    if (x < 0)
		acc = IEEESPDiv(acc, IEEESPPow(IEEESPFlt(-x), ten));
	    if (x > 0)
		acc = IEEESPMul(acc, IEEESPPow(IEEESPFlt(x), ten));

	    if (TmpBuf[0] == -1)
		acc = IEEESPNeg(acc);

	    ary[0] = ((int32_t *)&acc)[0];
	} else {
	    {				    /*	fix bug in afp()    */
		char *ptr;
		if (ptr = strchr(TmpBuf, 'e'))
		    *ptr = 'E';
	    }
	    ary[0] = afp(TmpBuf);	    /*	amiga.lib routine */
	}
	break;
    case 8:
	{
	    double acc; 	/*  accumulated value	*/
	    double ten = IEEEDPFlt(10);
	    short x;

	    /*
	     *	<TmpBuf> x 10^X    Convert a digit at a time.
	     */

	    x = FPrefix(exp, s->st_FltConst, s->st_FltLen, TmpBuf);

	    acc = IEEEDPFlt(0);
	    {
		char c;
		short i;

		for (i = 0; (c = TmpBuf[i+1]) && i < 17; ++i) {
		    acc = IEEEDPAdd(IEEEDPMul(acc, ten), IEEEDPFlt(c - '0'));
		    --x;
		}
	    }
	    if (x < 0)
		acc = IEEEDPDiv(acc, IEEEDPPow(IEEEDPFlt(-x), ten));
	    if (x > 0)
		acc = IEEEDPMul(acc, IEEEDPPow(IEEEDPFlt(x), ten));

	    if (TmpBuf[0] == -1)
		acc = IEEEDPNeg(acc);

	    ary[0] = ((int32_t *)&acc)[0];
	    ary[1] = ((int32_t *)&acc)[1];
	}
	break;
    case 16:
	dbprintf(("int32_t dbl flt const not implemented"));
	Assert(0);
	break;
    default:
	dbprintf(("asm_fltconst: bad size %d\n", s->st_Size));
	Assert(0);
	break;
    }
}

#else

void
asm_fltconst(exp, s, ary)
Exp *exp;
Stor *s;
int32_t *ary;
{
    ary[0] = 0;
    ary[1] = 0;
    ary[2] = 0;
    ary[3] = 0;
    TmpBuf[0] = 0;	/* prevent unused var warning */
    dbprintf(("asm_fltconst: not implemented\n"));
    Assert(0);
}

#endif

/*
 *  Compiler support call (passing arbitrarily sized arguments)
 *
 *  order = 1	    order = 0	    order = -1
 *
 *  LEA(s1),A0	    LEA(s1),A1	    any order
 *  LEA(s2),A1	    LEA(s2),A0
 *
 *  call routine    (such as __fpsub_a : s1 - s2)
 *  result in D0  or  D0/D1  or  D0/D1/A0/A1,  write -> d
 *  done
 */

#ifdef REGISTERED

void
CallFPSupport(
    Exp *exp,
    int32_t prec,
    Stor *s1,
    Stor *s2,
    Stor *d,
    char *fnam,
    short orderReq
) {
    char buf[32];
    char *mop;
    short len;
    Stor ss1;
    Stor ss2;

    switch(prec) {
    case 99:
	strcpy(buf, "__cv");
	break;
    case 4:
	if (FFPOpt)
	    strcpy(buf, "__ffp");
	else
	    strcpy(buf, "__sp");
	break;
    case 8:
	strcpy(buf, "__dp");
	break;
    case 16:
	strcpy(buf, "__xp");
	break;
    default:
	strcpy(buf, "__??");
	break;
    }
    strcat(buf, fnam);
    len = strlen(buf);
    buf[len++] = '_';
    buf[len++] = MathMode;
    buf[len] = 0;

    mop = strdup(buf);

    if (prec == 4) {	    /*	use Dn interface for single precision fp    */
	CallAsmSupport(exp, mop + 2, s1, s2, d, orderReq);
	return;
    }

    GenFlagCallMade();
    AddAuxSub(mop + 2);

    /*
     *	LEA s1 -> A0	    EXCEPT FOR FLOATS, PASS IN D0 (s2 is NULL)
     *	LEA s2 -> A1
     *	result in D0 or D0/D1 or D0/D1/A0/A1
     *
     *	Handle floating constant
     */

    if (s1 && s1->st_Type == ST_FltConst) {
	asm_layoutfpconst(exp, s1, &ss1);
	s1 = &ss1;
    }
    if (s2 && s2->st_Type == ST_FltConst) {
	asm_layoutfpconst(exp, s2, &ss2);
	s2 = &ss2;
    }

    {
	Stor rd0, rd1;
	Stor ra0, ra1;

	GenFlagCallMade();

	AllocDataRegisterAbs(&rd0, 4, RB_D0);
	AllocDataRegisterAbs(&rd1, 4, RB_D1);
	AllocAddrRegisterAbs(&ra0, RB_A0);
	AllocAddrRegisterAbs(&ra1, RB_A1);

	/*
	 *  lea s1 -> A0    but if s2 is rel A0 then use temporary
	 *  lea s2 -> A1
	 */

	if (s1 && s2) {
	    Assert(prec != 99);

	    if (SameRegister(s2, &ra0)) {
		if (SameRegister(s1, &ra1)) {
		    /*
		     *	put into reversed regs. If orderReq then exg
		     *	back to normal
		     */

		    asm_lea(exp, s1, 0, &ra1);
		    asm_lea(exp, s2, 0, &ra0);
		    if (orderReq > 0)
			printf("\texg\tA0,A1\n");
		} else {
		    if (orderReq > 0) {
			asm_lea(exp, s2, 0, &ra1);
			asm_lea(exp, s1, 0, &ra0);
		    } else {
			asm_lea(exp, s2, 0, &ra0);
			asm_lea(exp, s1, 0, &ra1);
		    }
		}
	    } else {
		/*
		 *  put into reg A0 first, then A1, if orderReq is 0 then
		 *  reverse them.
		 */

		asm_lea(exp, s1, 0, &ra0);
		asm_lea(exp, s2, 0, &ra1);
		if (orderReq == 0)
		    printf("\texg\tA0,A1\n");
	    }
	} else if (s1) {
	    if (s1->st_Size == 4) {
		/*printf("** BUF = %s, prec = %d, s1 = %08lx siz=%d\n", buf, prec, s1, s1->st_Size);*/
		Assert(prec == 99);	/*  only way!	*/
		asm_move(exp, s1, &rd0);
	    } else {
		asm_lea(exp, s1, 0, &ra0);
	    }
	} else if (s2) {
	    Assert(prec != 99);
	    asm_lea(exp, s2, 0, &ra1);
	}

	if (SmallCode)
	    printf("\tjsr\t%s(pc)\n", mop);
	else
	    printf("\tjsr\t%s\n", mop);

	/*
	 *  Result in D0 [D1 [ A0 A1]] depending on size.
	 */

	if (d) {
	    if ((d->st_Type == ST_Reg || d->st_Type == ST_RelReg) && (d->st_RegNo == RB_A0 || d->st_RegNo == RB_A1)) {
		RegFlagTryAgain();  /*	required?   */
		/*
		dbprintf(("dest is A0 or A1"));
		*/
	    }

	    if (d->st_Size == 4) {	/*  move    */
		asm_move(exp, &rd0, d);
	    } else {			/*  movem   */
		switch(d->st_Size) {
		case 4: 		/*  case occurs when converting */
		    printf("\tmove.l\tD0");
		    break;
		case 8:
		    printf("\tmovem.l\tD0/D1");
		    break;
		case 16:
		    printf("\tmovem.l\tD0/D1/A0/A1");
		    break;
		}
		printf(",%s\n", StorToString(d, NULL));
	    }
	}

	FreeRegister(&rd0);
	FreeRegister(&rd1);
	FreeRegister(&ra0);
	FreeRegister(&ra1);
    }
}

#else

void
CallFPSupport(Exp *exp, int32_t prec, Stor *s1, Stor *s2, Stor *d, char *fnam, short orderReq)
{
    cerror(EUNREG, "asm_fp: floating point");
}

#endif

void
asm_layoutfpconst(exp, s, d)
Exp *exp;
Stor *s;
Stor *d;
{
    int32_t fpv[4];
    int32_t l = AllocLabel();

    asm_segment(&DummyDataVar);
    asm_fltconst(exp, s, fpv);
    puts("\tds.w\t0");
    printf("l%d\tdc.l\t$%08x", l, fpv[0]);
    if (s->st_Size >= 8)
	printf(",$%08x", fpv[1]);
    if (s->st_Size >= 16) {
	printf(",$%08x", fpv[2]);
	printf(",$%08x", fpv[3]);
    }
    puts("");
    asm_segment(&DummyCodeVar);
    d->st_Type = ST_RelLabel;
    d->st_Offset = 0;
    d->st_Label = l;
    d->st_Flags = 0;
    d->st_Size = s->st_Size;
}

/*
 *			CONSTANT FLOATING POINT ROUTINES
 */

static TmpFlt f1, f2;

void
ConstFpNeg(exp, s, d)
Exp *exp;
Stor *s, *d;
{
    StorToTmpFlt(exp, s, &f1);
    f1.tf_Negative = !f1.tf_Negative;
    TmpFltToStor(exp, &f1, d);
}


void
ConstFpAdd(exp, s1, s2, d)
Exp *exp;
Stor *s1, *s2, *d;
{
    StorToTmpFlt(exp, s1, &f1);
    StorToTmpFlt(exp, s2, &f2);
    TmpFpAdd(&f1, &f2);
    TmpFltToStor(exp, &f2, d);
}

void
ConstFpSub(exp, s1, s2, d)
Exp *exp;
Stor *s1, *s2, *d;
{
    StorToTmpFlt(exp, s1, &f1);
    StorToTmpFlt(exp, s2, &f2);
    f2.tf_Negative = !f2.tf_Negative;
    TmpFpAdd(&f1, &f2);
    TmpFltToStor(exp, &f2, d);
}

Local void
TmpFpAdd(s, d)
TmpFlt *s;
TmpFlt *d;
{
    BalanceTmpFlt(s, d);

    if ((s->tf_Negative ^ d->tf_Negative) == 0) {
	short i;
	uint32_t v;
	uint32_t c = 0;

	if (s->tf_WMantissa[0] + d->tf_WMantissa[0] + 1 > 0xFFFF) {
	    dbprintf(("acx"));
	    TmpFltMantDiv(s->tf_WMantissa, 8, 10);
	    TmpFltMantDiv(d->tf_WMantissa, 8, 10);
	    ++s->tf_Exponent;
	    ++d->tf_Exponent;
	}
	for (i = 7; i >= 0; --i) {
	    v = s->tf_WMantissa[i] + d->tf_WMantissa[i] + c;
	    d->tf_WMantissa[i] = v;
	    c = v >> 16;
	}
    } else {
	/*
	 *  do: d = d - s;  keep d's sign, if the result of the absolute
	 *		    subtraction is negative the negate d
	 */

	short i;
	uint32_t b = 0;

	for (i = 7; i >= 0; --i) {
	    uint32_t v = d->tf_WMantissa[i] - s->tf_WMantissa[i] - b;
	    if ((int)v < 0)
		b = 1;
	    else
		b = 0;
	    d->tf_WMantissa[i] = v;
	}
	if (b) {	/*  abs(d) < abs(s)   */
	    /* b = 1 */
	    for (i = 7; i >= 0; --i) {
		uint32_t v = (uword)~d->tf_WMantissa[i] + b;
		if (v > 0xFFFF)
		    b = 1;
		else
		    b = 0;
		d->tf_WMantissa[i] = v;
	    }
	    d->tf_Negative = !d->tf_Negative;
	}
    }
    NormalizeTmpFlt(d);
}

/*
 *  Constant floating point multiply
 */

void
ConstFpMul(exp, s1, s2, d)
Exp *exp;
Stor *s1, *s2, *d;
{
    short i, j;
    union {
	uword w16[16];
	uint32_t w32[8];
    } res;

    StorToTmpFlt(exp, s1, &f1);
    StorToTmpFlt(exp, s2, &f2);

    setmem(&res, sizeof(res), 0);

    for (j = 7; j >= 0; --j) {
	uword *rptr;
	for (i = 7, rptr = res.w16 + j + 8; i >= 0; --i, --rptr) {
	    uint32_t v = f1.tf_WMantissa[i] * f2.tf_WMantissa[j];
	    uword *tptr = rptr;

	    while (v) {
		Assert(tptr >= res.w16);
		v = v + *tptr;
		*tptr = v;
		v >>= 16;
		--tptr;
	    }
	}
    }

    /*
     *	Divide until solution fits in 16 bytes
     */

    while (res.w32[0] | res.w32[1] | res.w32[2] | res.w32[3]) {
	TmpFltMantDiv(res.w16, 16, 10000);
	f2.tf_Exponent += 4;
    }

    f2.tf_Negative = f1.tf_Negative ^ f2.tf_Negative;
    f2.tf_Exponent += f1.tf_Exponent;
    f2.tf_LMantissa[0] = res.w32[4];
    f2.tf_LMantissa[1] = res.w32[5];
    f2.tf_LMantissa[2] = res.w32[7];
    f2.tf_LMantissa[3] = res.w32[7];
    NormalizeTmpFlt(&f2);
    TmpFltToStor(exp, &f2, d);
}

void
ConstFpDiv(exp, s1, s2, d)
Exp *exp;
Stor *s1, *s2, *d;
{
    union {
	    uword w16[16];		    /*	result	    */
	    uint32_t w32[8];
    } res;
    uword cmp[8];		    /*	compare     */
    short bit;

    StorToTmpFlt(exp, s1, &f1);
    StorToTmpFlt(exp, s2, &f2);

    /*
     * If msb bit is a 1, our shift/compare will overflow,
     * so we have to shift it right one.
     */

    if (f2.tf_WMantissa[0] > 0x7FFF) {
        TmpFltMantDiv(f2.tf_WMantissa, 8, 10);
        ++f2.tf_Exponent;
    }

    setmem(&res, sizeof(res), 0);    /*	clear result	*/
    setmem(cmp, sizeof(cmp), 0);    /*	clear compare	*/

    for (bit = 0; bit < 256; ++bit) {
	short i;
	uint32_t b;

	/*
	 *  shift bit into cmp from f1
	 */

	{
	    uint32_t *lp;
	    uint32_t v;

	    for (lp = f1.tf_LMantissa + 3, b = 0; lp >= f1.tf_LMantissa; --lp) {
		v = (*lp << 1) | b;
		b = 0;
		if ((int32_t)*lp < 0)
		    b = 1;
		*lp = v;
	    }
	    for (lp = (uint32_t *)cmp + 3; lp >= (uint32_t *)cmp; --lp) {
		v = (*lp << 1) | b;
		b = 0;
		if ((int32_t)*lp < 0)
		    b = 1;
		*lp = v;
	    }
	}
	if (b)
	{
	    dbprintf(("fp constant divide"));
	    Assert(0);
	}

	dbprintf(("cmp: %04x%04x%04x%04x%04x%04x%04x%04x\n", cmp[0], cmp[1], cmp[2], cmp[3], cmp[4], cmp[5], cmp[6], cmp[7]));

	/*
	 *  can we subtract ? i.e. f2 <= cmp ?
	 */

	for (i = 0; i < 7; ++i) {
	    if (f2.tf_WMantissa[i] < cmp[i])
		break;
	    if (f2.tf_WMantissa[i] > cmp[i])
		goto skip;
	}

	dbprintf((";subok\n"));

	/*
	 *  yes, set bit in result
	 */

	res.w16[bit >> 4] |= 0x8000 >> (bit & 15);

	/*
	 *  subtract
	 */

	for (i = 7, b = 0; i >= 0; --i) {
	    uint32_t v = cmp[i] - f2.tf_WMantissa[i] - b;
	    b = 0;
	    if ((int32_t)v < 0)
		b = 1;
	    cmp[i] = v;
	}
skip:
	;
    }

    dbprintf(("res: %04x%04x%04x%04x%04x%04x%04x%04x %04x%04x%04x%04x%04x%04x%04x%04x\n",
	res.w16[0], res.w16[1], res.w16[2], res.w16[3],
	res.w16[4], res.w16[5], res.w16[6], res.w16[7],
	res.w16[8], res.w16[9], res.w16[10],res.w16[11],
	res.w16[12],res.w16[13],res.w16[14],res.w16[15]
    ));

    /*
     *	Multiply until left justified, solution is first 4 longwords
     *
     *	bit limits loop incase we are dealing with a 0 result.
     */

    for (bit = 0; res.w16[0] == 0 && bit < 32; ++bit) {
	TmpFltMantMul(res.w16, 16, 10000);
	f1.tf_Exponent -= 4;
    }
    for (bit = bit; res.w16[0] < (uword)(0xFFFF / 10 - 10) && bit < 32; ++bit) {
	TmpFltMantMul(res.w16, 16, 10);
	f1.tf_Exponent -= 1;
    }

    f2.tf_Negative = f1.tf_Negative ^ f2.tf_Negative;
    f2.tf_Exponent = f1.tf_Exponent - f2.tf_Exponent;
    f2.tf_LMantissa[0] = res.w32[0];
    f2.tf_LMantissa[1] = res.w32[1];
    f2.tf_LMantissa[2] = res.w32[2];
    f2.tf_LMantissa[3] = res.w32[3];
    NormalizeTmpFlt(&f2);
    TmpFltToStor(exp, &f2, d);
}


