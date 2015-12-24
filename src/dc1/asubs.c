/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  ASUBS.C
 *
 */

/*
**      $Filename: asubs.c $
**      $Author: dice $
**      $Revision: 30.326 $
**      $Date: 1995/12/24 06:09:36 $
**      $Log: asubs.c,v $
 * Revision 30.326  1995/12/24  06:09:36  dice
 * .
 *
 * Revision 30.5  1994/06/13  18:37:23  dice
 * byte ordering portability
 *
 * Revision 30.0  1994/06/10  18:04:46  dice
 * .
 *
 * Revision 1.3  1993/11/22  00:28:36  jtoebes
 * Final cleanup to eliminate all cerror() messages with strings.
 *
 * Revision 1.2  1993/09/06  13:17:17  jtoebes
 * Fixed BUG01091 - Bad error for undefined structure tag.
 * Eliminated message because it is now caught in the front end parsing
 * routines for structures/unions/arrays.
 *
**/

#include "defs.h"
#include "asm.h"

Prototype char SizC[];
Prototype char StorBuf[2][1024];

char SizC[] = { '0', 'b', 'w', '0', 'l', '?' };
char StorBuf[2][1024];

Prototype char *RegMaskToString(int32_t, short *);
Prototype char *StorToString(Stor *, short *);
Prototype char *StorToStringBuf(Stor *, char *);
Prototype int32_t SameStorage(Stor *, Stor *);
Prototype int32_t SameRegister(Stor *, Stor *);
Prototype int32_t RegisterMaskConflict(Stor *, uint32_t);
Prototype int32_t ImmStorage(Stor *);
Prototype void outop(char *, short, Stor *, Stor *);
Prototype void GenStaticData(Var *);
Prototype void GenDataElm(Exp *, Type *);
Prototype char *itohex(char *, uint32_t);
Prototype char *itodec(char *, uint32_t);

Prototype void AutoAggregateBeg(Stor *, Type *);
Prototype void AutoAggregate(void *, int32_t);
Prototype void AutoAggregateEnd(void);
Prototype void AutoAggregateSync(void);

char *
RegMaskToString(mask, pcnt)
int32_t mask;
short *pcnt;
{
    static char buf[128];
    short i;
    short c = 0;
    char *ptr = buf;

    for (i = 0; i < 32; ++i) {
	if (mask & (1 << i)) {
	    if (c++)
		*ptr++ = '/';
	    if (i < RB_ADDR) {
		*ptr++ = 'D';
		*ptr++ = i + '0';
	    } else {
		*ptr++ = 'A';
		*ptr++ = i + ('0' - RB_ADDR);
	    }
	}
    }
    *pcnt = c;
    *ptr = 0;
    return(buf);
}

char *
StorToString(s, plen)
Stor *s;
short *plen;
{
    static char *Ptr = StorBuf[0];
    char *res;

    Ptr = ((Ptr == StorBuf[0]) ? StorBuf[1] : StorBuf[0]);
    res = StorToStringBuf(s, Ptr);
    if (plen)
	*plen = res - Ptr;
    return(Ptr);
}

char *
StorToStringBuf(s, base)
Stor *s;
char *base;
{
    char *t = base;

    switch(s->st_Type) {
    case ST_PtrConst:
	*t = '$';
	t = itohex(t + 1, s->st_PtrConst);
	if ((int32_t)s->st_PtrConst >= -32768 && (int32_t)s->st_PtrConst < 32768) {
	    *t = '.'; ++t;
	    *t = 'W'; ++t;
	}
	break;
    case ST_IntConst:		/*  complication is for pretty output */
	*t = '#'; ++t;
	if (s->st_IntConst < 0) {
	    *t = '-'; ++t;
	    *t = '$'; ++t;
	    t = itohex(t, -s->st_IntConst);
	} else {
	    *t = '$'; ++t;
	    t = itohex(t, s->st_IntConst);
	}
	break;
    case ST_FltConst:
	t += sprintf(t, "<flt-const>");
	break;
    case ST_StrConst:
	dbprintf(("asm-gen, string constant not turned into label"));
	Assert(0);
	t += sprintf(t, "<str-const>");
	break;
    case ST_Reg:
	if (s->st_RegNo < RB_ADDR) {
	    *t = 'D'; ++t;
	    if (s->st_RegNo < 10) {
		*t = s->st_RegNo + '0';
		++t;
	    } else {
		t[0] = '1';
		t[1] = s->st_RegNo + ('0' - 10);
		t += 2;
	    }
	} else {
	    *t = 'A'; ++t;
	    if (s->st_RegNo < RB_ADDR + 10) {
		*t = s->st_RegNo + ('0' - RB_ADDR);
		++t;
	    } else {
		t[0] = '1';
		t[1] = s->st_RegNo + ('0' - 10 - RB_ADDR);
		t += 2;
	    }
	}
	break;
    case ST_RelArg:
	{
	    int32_t offset = s->st_Offset;

	    *t = 'l'; ++t;
	    t = itodec(t, LabelRegsUsed);

	    /*
	     *	If procedure returns a structure take into account arguments
	     */

	    switch(ProcVar->Type->SubType->Id) {
	    case TID_STRUCT:
	    case TID_UNION:
		offset += 4;
		break;
	    }

	    if (s->st_RegNo == RB_SP)
		offset -= 4;

	    if (offset >= 0) {
		*t = '+';
		++t;
	    }
	    t = itodec(t, offset);

	    *t = '('; ++t;
	    *t = 'A'; ++t;
	    if (s->st_RegNo < RB_ADDR + 10) {
		*t = s->st_RegNo + ('0' - RB_ADDR);
		++t;
	    } else {
		t[0] = '1';
		t[1] = s->st_RegNo + ('0' - RB_ADDR - 10);
		t += 2;
	    }
	    *t = ')'; ++t;
	}
	break;
    case ST_RelReg:
	if (s->st_RegNo < RB_ADDR)
	{
	    dbprintf(("asubs: rel(Dn) request! ??"));
	    Assert(0);
	}
	if (s->st_Offset)
	    t = itodec(t, s->st_Offset);
	*t = '('; ++t;
	*t = 'A'; ++t;
	if (s->st_RegNo < RB_ADDR + 10) {
	    *t = s->st_RegNo + ('0' - RB_ADDR);
	    ++t;
	} else {
	    t[0] = '1';
	    t[1] = s->st_RegNo + ('0' - RB_ADDR - 10);
	    t += 2;
	}
	*t = ')'; ++t;
	break;
    case ST_RegIndex:	/*  offset(An,Dn.[W/L])     */
	if (s->st_Offset) {
	    t = itodec(t, s->st_Offset);
	} else {
	    *t = '0';
	    ++t;
	}
	*t = '('; ++t;
	*t = 'A'; ++t;
	if (s->st_RegNo < RB_ADDR + 10) {
	    *t = s->st_RegNo + ('0' - RB_ADDR);
	    ++t;
	} else {
	    t[0] = '1';
	    t[1] = s->st_RegNo + ('0' - RB_ADDR - 10);
	    t += 2;
	}

	*t = ','; ++t;
	if (s->st_RegNo2 >= RB_ADDR) {
	    t[0] = 'A';
	    t[1] = s->st_RegNo2 + ('0' - RB_ADDR);
	} else {
	    t[0] = 'D';
	    t[1] = s->st_RegNo2 + '0';
	}
	t += 2;

	*t = '.'; ++t;
	if (s->st_Flags & SF_IDXWORD)
	    *t = 'W';
	else
	    *t = 'L';
	++t;

	if (s->st_Flags & (SF_IDXSCAL2|SF_IDXSCAL4|SF_IDXSCAL8)) {
	    *t = '*'; ++t;
	    if (s->st_Flags & SF_IDXSCAL2)
		*t = '2';
	    else if (s->st_Flags & SF_IDXSCAL4)
		*t = '4';
	    else
		*t = '8';
	    ++t;
	}
	*t = ')'; ++t;
	break;
    case ST_RelLabel:
	if (s->st_Flags & SF_REGARGS) {
	    *t = '@';
	    ++t;
	}

	*t = 'l'; ++t;
	t = itodec(t, s->st_Label);

	if (s->st_Offset) {
	    if (s->st_Offset >= 0) {
		*t = '+';
		++t;
	    }
	    t = itodec(t, s->st_Offset);
	}
	if (s->st_Flags & SF_CODE) {
	    if ((SmallCode || (s->st_Flags & SF_NEAR)) && !(s->st_Flags & SF_FAR)) {
		*t = '('; ++t;
		*t = 'p'; ++t;
		*t = 'c'; ++t;
		*t = ')'; ++t;
	    }
	} else {
	    if ((SmallData || (s->st_Flags & SF_NEAR)) && !(s->st_Flags & SF_FAR)) {
		if (SmallData == 2) {
		    *t = '.'; ++t;
		    *t = 'W'; ++t;
		} else {
		    *t = '('; ++t;
		    *t = 'A'; ++t;
		    *t = '4'; ++t;
		    *t = ')'; ++t;
		}
	    }
	}
	break;
    case ST_RelName:
	if (s->st_Flags & SF_REGARGS)
	    *t = '@';
	else
	    *t = '_';
	++t;
	movmem(s->st_Name->Name, t, s->st_Name->Len);
	t += s->st_Name->Len;

	if (s->st_Offset) {
	    if (s->st_Offset >= 0) {
		*t = '+';
		++t;
	    }
	    t = itodec(t, s->st_Offset);
	}
	if (s->st_Flags & SF_CODE) {
	    if ((SmallCode || (s->st_Flags & SF_NEAR)) && !(s->st_Flags & SF_FAR)) {
		*t = '('; ++t;
		*t = 'p'; ++t;
		*t = 'c'; ++t;
		*t = ')'; ++t;
	    }
	} else {
	    if ((SmallData || (s->st_Flags & SF_NEAR)) && !(s->st_Flags & SF_FAR)) {
		if (SmallData == 2) {
		    *t = '.'; ++t;
		    *t = 'W'; ++t;
		} else {
		    *t = '('; ++t;
		    *t = 'A'; ++t;
		    *t = '4'; ++t;
		    *t = ')'; ++t;
		}
	    }
	}
	break;
    case ST_Push:
	*t = '-'; ++t;
	*t = '('; ++t;
	*t = 's'; ++t;
	*t = 'p'; ++t;
	*t = ')'; ++t;
	break;
    default:
	t += sprintf(t, "?%d", s->st_Type);
	break;
    }
    *t = 0;
    return(t);
}

/*
 *  Same physical storage, possibly different sizes
 */

int32_t
SameStorage(s1, s2)
Stor *s1, *s2;
{
    if ((s1->st_Flags ^ s2->st_Flags) & SF_LEA)
	return(0);
    if (s1->st_Type != s2->st_Type)
	return(0);
    if (s1->st_Type == ST_Reg)
	return(s1->st_RegNo == s2->st_RegNo);

    switch(s1->st_Type) {
    case ST_PtrConst:
	return(s1->st_PtrConst == s2->st_PtrConst);
    case ST_IntConst:
	return(s1->st_IntConst == s2->st_IntConst);
    case ST_FltConst:
	return(s1->st_FltConst == s2->st_FltConst);
    case ST_StrConst:
	return(s1->st_StrConst == s2->st_StrConst);
    case ST_RelArg:
	return(s1->st_Offset == s2->st_Offset);
    case ST_RelReg:
	return(s1->st_RegNo == s2->st_RegNo && s1->st_Offset == s2->st_Offset);
    case ST_RelLabel:
	return(s1->st_Label == s2->st_Label && s1->st_Offset == s2->st_Offset);
    case ST_RelName:
	return(s1->st_Name == s2->st_Name && s1->st_Offset == s2->st_Offset);
    case ST_RegIndex:
	return(s1->st_RegNo == s2->st_RegNo && s1->st_Offset == s2->st_Offset && s1->st_RegNo2 == s2->st_RegNo2);
    default:
        dbprintf(("Bad storage type for same-compare %d\n", s1->st_Type));
    }
    Assert(0);
    return(0); /* not reached */
}

/*
 *  Will the registers used in one storage structure interfere with the
 *  registers used in another?
 */

int32_t
SameRegister(s1, s2)
Stor *s1, *s2;
{
    switch(s1->st_Type) {
    case ST_Reg:
    case ST_RelReg:
	switch(s2->st_Type) {
	case ST_Reg:
	case ST_RelReg:
	    if (s1->st_RegNo == s2->st_RegNo)
		return(1);
	    break;
	case ST_RegIndex:
	    if (s1->st_RegNo == s2->st_RegNo || s1->st_RegNo == s2->st_RegNo2)
		return(1);
	    break;
	}
	break;
    case ST_RegIndex:
	switch(s2->st_Type) {
	case ST_Reg:
	case ST_RelReg:
	    if (s2->st_RegNo == s1->st_RegNo || s2->st_RegNo == s1->st_RegNo2)
		return(1);
	    break;
	case ST_RegIndex:
	    if (s2->st_RegNo == s1->st_RegNo || s2->st_RegNo == s1->st_RegNo2 || s2->st_RegNo2 == s1->st_RegNo || s2->st_RegNo2 == s1->st_RegNo2)
		return(1);
	    break;
	}
	break;
    }
    return(0);
}

int32_t
RegisterMaskConflict(s1, mask)
Stor *s1;
uint32_t mask;
{
    switch(s1->st_Type) {
    case ST_Reg:
    case ST_RelReg:
	if ((1 << s1->st_RegNo) & mask)
	    return(1);
	break;
    case ST_RegIndex:
	if (((1 << s1->st_RegNo) | (1 << s1->st_RegNo2)) & mask)
	    return(1);
	break;
    case ST_RelName:
    case ST_RelLabel:
	if ((s1->st_Flags & (SF_FAR|SF_CODE)) == 0 && (mask & RF_A4))
	    return(1);
	break;
    case ST_RelArg:
	if (mask & RF_A5)
	    return(1);
	break;
    }
    return(0);
}


/*
 *  Does the storage represent some sort of immediate value?  This includes
 *  effective addresses
 */

int32_t
ImmStorage(stor)
Stor *stor;
{
    if (stor->st_Type == ST_IntConst || stor->st_Type == ST_PtrConst)
	return(1);
    if ((stor->st_Flags & SF_LEA) && (stor->st_Type == ST_RelLabel || stor->st_Type == ST_RelName))
	return(1);
    return(0);
}

void
outop(char *op, short siz, Stor *s, Stor *d)
{
    char *ptr = StorBuf[0];

    if (siz == 0)
	siz = d->st_Size;
    if (siz > 4)
	siz = 5;

    *ptr = '\t'; ++ptr;
    while ((*ptr = *op) != 0) {
	++ptr;
	++op;
    }
    if (siz >= 0) {
	*ptr = '.'; ++ptr;
	*ptr = SizC[siz]; ++ptr;
    }
    *ptr = '\t'; ++ptr;
    if (s) {
	ptr = StorToStringBuf(s, ptr);
	*ptr = ','; ++ptr;
    }
    ptr = StorToStringBuf(d, ptr);
    *ptr = '\n'; ++ptr;
    fwrite(StorBuf[0], ptr - StorBuf[0], 1, stdout);
}


/*
 *  Generate structured static/global data.
 *
 *  If the resident (-r) option is used we cannot generate data-data
 *  relocations ... we must do them in an autoinit section using A4
 *  relative addressing.
 *
 *  Most of the structural considerations are handled by GenBracedEq/Assign
 *  which is forced for static/global initialization.
 */

void
GenStaticData(var)
Var *var;
{
    if ((var->Flags & TF_ALIGNED) || *var->Type->Align == 4)
	printf("\tds.l\t0\n");
    else if (*var->Type->Align != 1)
	printf("\tds.w\t0\n");

    if (var->var_Stor.st_Type == ST_RelName)
	printf("_%s", SymToString(var->var_Stor.st_Name));
    else
	printf("l%d", var->var_Stor.st_Label);

    if (var->u.AssExp == NULL) {
	printf("\tds.b\t%d\n", *var->Type->Size);
    } else {
	/*
	 *  Generate the data from the expression.
	 */
	puts("");
	GenDataElm(var->u.AssExp, var->Type);
    }
    OutAlign = *var->Type->Size & 3;

    /*
     *	If a __config variable then generate an autoconfig section for
     *	it.
     */

    if (var->Flags & TF_CONFIG) {
	printf("\tsection\tautoconfig,code\n");
	printf("\tpea\t%s\n", StorToString(&var->var_Stor, NULL));
	if (var->var_Stor.st_Size < 32768)
	    printf("\tpea\t%d.W\n", var->var_Stor.st_Size);
	else
	    printf("\tpea\t%d\n", var->var_Stor.st_Size);
	if (SmallCode || PIOpt)
	    printf("\tjsr\t__DiceConfig(pc)\n");
	else
	    printf("\tjsr\t__DiceConfig\n");
	AddAuxSub("DiceConfig");
	puts(LastSectBuf);
    }
}

/*
 *  GegnDataElm()
 *
 *  run through element generation for expression, which MUST be of type
 *  TokExpAssBlock
 */

void
GenDataElm(exp, type)
Exp *exp;
Type *type;
{
    short savePass = GenPass;

    Assert(exp->ex_Token == TokExpAssBlock);
    Assert(exp->ex_Type == type);

    /*
     *	Run through expression generation passes
     */

    GenGlobal = 1;
    GenPass = 0;
    (*exp->ex_Func)(&exp);
    GenPass = 1;
    AutoAggregateBeg(NULL, exp->ex_Type);
    (*exp->ex_Func)(&exp);
    AutoAggregateEnd();
    GenPass = savePass;
    GenGlobal = 0;
}


char *
itohex(ptr, val)
char *ptr;
uint32_t val;
{
    static char HA[] = { "0123456789ABCDEF" };
    short dig = 8;
    char *p;

    if (val < 0x10000) {
	dig = 4;
	if (val < 0x100)
	    dig = 2;
    }
    ptr += dig;
    p = ptr;
    *p-- = 0;
    while (dig--) {
	*p-- = HA[(short)val & 0xF];
	val >>= 4;
    }
    return(ptr);
}

char *
itodec(ptr, val)
char *ptr;
uint32_t val;
{
    if ((int32_t)val < 0) {
	val = -val;
	*ptr++ = '-';
    }
    if (val < 10) {
	*ptr++ = '0' + val;
    } else if (val < 100) {
	*ptr++ = '0' + val / 10;
	*ptr++ = '0' + val % 10;
    } else {
	char buf[16];
	short i = 15;

	while (val > 32767) {
	    buf[i--] = '0' + val % 10;
	    val /= 10;
	}
	while (val) {
	    buf[i--] = '0' + (uword)val % (uword)10;
	    val = (short)val / (short)10;
	}
	while (i != 15)
	    *ptr++ = buf[++i];
    }
    *ptr = 0;
    return(ptr);
}

static int32_t AGLabel;
static int32_t AGIndex;
static TmpAggregate *AGBase;
static TmpAggregate **AGNext;

void
AutoAggregateBeg(s, type)
Stor *s;
Type *type;
{
    AGLabel = AllocLabel();
    AGBase = NULL;
    AGNext = &AGBase;

    if (GenGlobal == 0) {
	setmem(s, sizeof(Stor), 0);
	s->st_Type = ST_RelLabel;
	s->st_Offset = 0;
	s->st_Size = *type->Size;
	s->st_Label = AGLabel;
	s->st_Flags = (SmallData) ? SF_NEAR : SF_FAR;
    }
}

void
AutoAggregate(ptr, n)
void *ptr;
int32_t n;
{
    TmpAggregate *ag = *AGNext;

    if (ag == NULL || ag->ta_Bytes - ag->ta_Index < n) {
	int32_t v = (n > 512) ? n : 512;

	if (ag)
	    AGNext = &ag->ta_Next;

	ag = malloc(sizeof(TmpAggregate) + v);

	if (ag == NULL)
	    NoMem();
	ag->ta_Next = NULL;
	ag->ta_Buf = (char *)(ag + 1);
	ag->ta_Index = 0;
	ag->ta_Bytes = v;
	*AGNext = ag;
    }
    {
	char *dptr = ag->ta_Buf + ag->ta_Index;

	if (ptr)
	    movmem(ptr, dptr, n);
	else
	    setmem(dptr, n, 0);
    }
    ag->ta_Index += n;
}

void
AutoAggregateSync()
{
    TmpAggregate *ag, *agNext;

    if (GenGlobal == 0 && AGLabel) {
	asm_segment(&DummyDataVar);
	printf("\tds.l\t0\n");
	printf("l%d\n", AGLabel);
	AGLabel = 0;
    }
    for (ag = AGBase; ag; ag = agNext) {
	int32_t i;
	short col = 0;

	agNext = ag->ta_Next;

	if ((AGIndex & 3) == 0 && (ag->ta_Index & 3) == 0) {
	    for (i = 0; i < ag->ta_Index; i += 4) {
		if (col == 0) {
		    printf("\n\tdc.l\t");
		} else {
		    putc(',', stdout);
		    ++col;
		}
		col += printf("$%x", ToMsbOrder(*(uint32_t *)(void *)(ag->ta_Buf + i)));
		if (col > 120)
		    col = 0;
	    }
	} else if ((AGIndex & 1) == 0 && (ag->ta_Index & 1) == 0) {
	    for (i = 0; i < ag->ta_Index; i += 2) {
		if (col == 0) {
		    printf("\n\tdc.w\t");
		} else {
		    putc(',', stdout);
		    ++col;
		}
		col += printf("$%x",
			ToMsbOrderShort(*(uint16_t *)(void *)(ag->ta_Buf + i)));
		if (col > 120)
		    col = 0;
	    }
	} else {
	    for (i = 0; i < ag->ta_Index; i += 1) {
		if (col == 0) {
		    printf("\n\tdc.b\t");
		} else {
		    putc(',', stdout);
		    ++col;
		}
		col += printf("$%x",
			(uint32_t)*(unsigned char *)(void *)(ag->ta_Buf + i));
		if (col > 120)
		    col = 0;
	    }
	}
	AGIndex += ag->ta_Index;
	puts("");
	free(ag);
    }
    AGBase = NULL;
    AGNext = &AGBase;
}

void
AutoAggregateEnd()
{
    if (GenGlobal == 0 && AGLabel) {
	asm_segment(&DummyDataVar);
	printf("\tds.l\t0\n");
	printf("l%d\n", AGLabel);
	AGLabel = 0;
    }
    AutoAggregateSync();
    puts("");
    if (GenGlobal == 0) {
	asm_segment(&DummyCodeVar);
    }
}

