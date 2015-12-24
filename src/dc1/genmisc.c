/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  GENMISC.C
 *
 *	Miscellanious stuff and some of the more esoteric generation routines
 *	such as -> . &lvalue [], etc...
 */

#include "defs.h"

Prototype void GenCast(Exp **);
Prototype void GenSizeof(Exp **);
Prototype void GenAddr(Exp **);
Prototype void GenInd(Exp **);
Prototype void GenPreInc(Exp **);
Prototype void GenPreDec(Exp **);
Prototype void GenIntConst(Exp **);
Prototype void GenStrConst(Exp **);
Prototype void GenFltConst(Exp **);
Prototype void GenVarRef(Exp **);
Prototype void GenCall(Exp **);
Prototype void GenCastArgs(Type *, Exp *, Exp **);
Prototype void GenPosInc(Exp **);
Prototype void GenPosDec(Exp **);
Prototype void GenStructInd(Exp **);
Prototype void GenStructElm(Exp **);
Prototype void GenArray(Exp **);
Prototype void GenBFExt(Exp **);
Prototype void BitFieldResultExp(Exp *);
Prototype void BitFieldResultType(Exp **, int);

Local void GenRegArgs(Exp *, char *, char *, ulong, char *, short);
Local void SaveCopyConflictsRegArgs(Exp *, char *, char *, ulong, short);
Local void FreeRegArgs(Exp *, char *);
Local long GenPushArgs(Exp *);
Local int  PushArgExp(Exp *);
Local void InMaskPush(Exp *, char *);
Local void InMaskPop(char *);

/*
 *  GenCast()
 */

void
GenCast(pexp)
Exp **pexp;
{
    Exp *exp = *pexp;	    /*	left side exp, right side type	*/
    Exp *e1;

    Assert(exp->ex_Type);
    if (GenPass == 0) {
	if ((exp->ex_Flags & EF_ICAST) == 0) {		    /*	see insert cast */
	    if (exp->ex_ExpL->ex_Type == NULL)
		exp->ex_ExpL->ex_Type = exp->ex_Type;	    /*	optimize    */
	    CallLeft();
	}
	e1 = exp->ex_ExpL;

	if (e1->ex_Stor.st_Type == ST_IntConst) {
	    e1->ex_Type = exp->ex_Type;
	    e1->ex_Stor.st_Size = *e1->ex_Type->Size;

	    if (exp->ex_Type->Id == TID_FLT) {
		e1->ex_Stor.st_FltConst = IntToFPStr(e1->ex_Stor.st_IntConst, e1->ex_Stor.st_Flags & SF_UNSIGNED, &e1->ex_Stor.st_FltLen);
		e1->ex_Type = exp->ex_Type;
		e1->ex_Stor.st_Size = *e1->ex_Type->Size;
		e1->ex_Stor.st_Type = ST_FltConst;
		e1->ex_Token = TokFltConst;
		e1->ex_Func  = GenFltConst;
		*pexp = e1;
		return;
	    }

	    if (e1->ex_Type->Flags & TF_UNSIGNED) {
		switch(*e1->ex_Type->Size) {
		case 0:
		    e1->ex_Stor.st_UIntConst = 0;
		    break;
		case 1:
		    e1->ex_Stor.st_UIntConst = (ubyte)e1->ex_Stor.st_UIntConst;
		    break;
		case 2:
		    e1->ex_Stor.st_UIntConst = (uword)e1->ex_Stor.st_UIntConst;
		    break;
		case 4:
		    break;
		case 8:		/* XXX */
		default:
		    yerror(exp->ex_LexIdx, EERROR_ILLEGAL_CAST);
		    break;
		}
		e1->ex_Stor.st_Flags |= SF_UNSIGNED;
	    } else {
		switch(*e1->ex_Type->Size) {
		case 0:
		    e1->ex_Stor.st_IntConst = 0;
		    break;
		case 1:
		    e1->ex_Stor.st_IntConst = (char)e1->ex_Stor.st_IntConst;
		    break;
		case 2:
		    e1->ex_Stor.st_IntConst = (short)e1->ex_Stor.st_IntConst;
		    break;
		case 4:
		    break;
		case 8:		/* XXX */
		default:
		    yerror(exp->ex_LexIdx, EERROR_ILLEGAL_CAST);
		    break;
		}
		e1->ex_Stor.st_Flags &= ~SF_UNSIGNED;
	    }
	    *pexp = e1;
	    return;
	}

	/*
	 *  Floating constant to integer
	 *
	 *  e1->ex_StrConst  e1->ex_StrLen
	 */

	if (e1->ex_Stor.st_Type == ST_FltConst) {
	    if (exp->ex_Type->Id == TID_FLT) {	/*  flt->flt no change */
		*pexp = e1;
		e1->ex_Type = exp->ex_Type;
		return;
	    }
	    if (exp->ex_Type->Id == TID_INT) {
		e1->ex_Stor.st_IntConst = FPStrToInt(e1, e1->ex_Stor.st_FltConst, e1->ex_Stor.st_FltLen);
		e1->ex_Type = exp->ex_Type;
		e1->ex_Stor.st_Size = *e1->ex_Type->Size;
		e1->ex_Stor.st_Type = ST_IntConst;
		e1->ex_Token = TokIntConst;
		e1->ex_Func  = GenIntConst;
		if (e1->ex_Stor.st_Size == 0)	/*  cast to void    */
		    e1->ex_Stor.st_IntConst = 0;
		*pexp = e1;

		if (exp->ex_Type->Flags & TF_UNSIGNED)
		    exp->ex_Stor.st_Flags |= SF_UNSIGNED;
		else
		    exp->ex_Stor.st_Flags &= ~SF_UNSIGNED;

		return;
	    }
	}

	/*
	 *  If Flt->Flt, Flt->Int, or Int->Flt conversion then
	 *  set EF_CALL bit.  (doesn't handle void case here)
	 */

	if (exp->ex_Type->Id == TID_FLT || e1->ex_Type->Id == TID_FLT) {
	    exp->ex_Flags |= EF_CALL;
	    GenFlagCallMade();
	}
	exp->ex_Flags |= e1->ex_Flags & EF_CALL;
    } else {
	Type *t = exp->ex_Type;

	e1 = exp->ex_ExpL;

	if (exp->ex_Flags & EF_RNU) {
	    e1->ex_Flags |= EF_RNU;
	    CallLeft();
	    return;
	}

	if (e1->ex_Type->Id == TID_ARY) {
	    CallLeft();
	    if (exp->ex_Flags & (EF_PRES|EF_ASSEQ)) {
		CreateUnaryResultStorage(exp, 1);
		asm_lea(exp, &e1->ex_Stor, 0, &exp->ex_Stor);
	    } else {
		FreeStorage(&e1->ex_Stor);
		/* X1 */
		asm_getlea(exp, &e1->ex_Stor, &exp->ex_Stor);
	    }
	    return;
	}

	/*
	 *  Floating point conversions	fp -> int,  int -> fp
	 */

	if (exp->ex_Type->Id == TID_FLT || e1->ex_Type->Id == TID_FLT) {
	    CallLeft();
	    CreateUnaryResultStorage(exp, 1);

	    if (exp->ex_Type->Id == TID_FLT) {
		if (e1->ex_Type->Id == TID_FLT) {   /* FLT -> FLT */
		    asm_fptofp(exp, &e1->ex_Stor, &exp->ex_Stor);
		} else {			    /* INT -> FLT */
		    asm_inttofp(exp, &e1->ex_Stor, &exp->ex_Stor);
		}
	    } else {	    /*	FLT -> INT  (if not cast to void)   */
		if (exp->ex_Stor.st_Size)
		    asm_fptoint(exp, &e1->ex_Stor, &exp->ex_Stor);
	    }

	    if (exp->ex_Type->Flags & TF_UNSIGNED)
		exp->ex_Stor.st_Flags |= SF_UNSIGNED;
	    else
		exp->ex_Stor.st_Flags &= ~SF_UNSIGNED;

	    return;
	}

	/*
	 *  Bitfield Conversions
	 *
	 *  XXX Enabled 23 Dec 95, Matt Dillon, to handle case where
	 *  bitfield assignment occurs within expression and then must
	 *  be extracted out again.
	 */

	if (exp->ex_Type->Id == TID_INT && e1->ex_Type->Id == TID_BITFIELD) {
	    CallLeft();
	    CreateUnaryResultStorage(exp, 1);

	    if (t->Flags & TF_UNSIGNED)
		exp->ex_Stor.st_Flags |= SF_UNSIGNED;
	    else
		exp->ex_Stor.st_Flags &= ~SF_UNSIGNED;

	    asm_bfext(exp, &e1->ex_Stor, &exp->ex_Stor);

	    return;
	}

	if ((exp->ex_Flags & (EF_PRES|EF_ASSEQ)) == 0) {
	    if (*t->Size == *e1->ex_Type->Size) {
		CallLeft();
		FreeStorage(&e1->ex_Stor);
		ReuseStorage(&e1->ex_Stor, &exp->ex_Stor);

		if (t->Flags & TF_UNSIGNED)
		    exp->ex_Stor.st_Flags |= SF_UNSIGNED;
		else
		    exp->ex_Stor.st_Flags &= ~SF_UNSIGNED;

		return;
	    }
	}

	/*
	 *  If storage size is the same & storage passed down from parent
	 *  then pass to subexp.
	 */

	if (*t->Size == *e1->ex_Type->Size && (exp->ex_Flags & EF_PRES) && (e1->ex_Flags & (EF_CRES|EF_ASSEQ)) == 0) {
	    e1->ex_Flags |= EF_PRES;
	    e1->ex_Stor = exp->ex_Stor;

	    if (e1->ex_Type->Flags & TF_UNSIGNED)	/* ??? XXX */
		e1->ex_Stor.st_Flags |= SF_UNSIGNED;
	    else
		e1->ex_Stor.st_Flags &= ~SF_UNSIGNED;

	    CallLeft();
	    return;
	}

	CallLeft();

	CreateUnaryResultStorage(exp, 1);

	if (t->Flags & TF_UNSIGNED)
	    exp->ex_Stor.st_Flags |= SF_UNSIGNED;
	else
	    exp->ex_Stor.st_Flags &= ~SF_UNSIGNED;

	if (exp->ex_Stor.st_Size)
	    asm_ext(exp, &e1->ex_Stor, &exp->ex_Stor, e1->ex_Stor.st_Flags);
    }
}

/*
 *  Strings use a dummy CharAryType ... not a real type because its size does
 *  not match the size of the string.  So we hack the only operator that will
 *  ever need the size.
 */

void
GenSizeof(pexp)
Exp **pexp;
{
    Exp *exp = *pexp;
    Type *type;
    long size;

    if (GenPass == 0) {
	if (exp->ex_ExpL) {
	    Exp *e1;

	    CallLeft();
	    e1 = exp->ex_ExpL;

	    type = e1->ex_Type;
	    size = *type->Size;

	    if (e1->ex_Stor.st_Type == ST_StrConst) {
		DelStrList(e1->ex_Stor.st_Label);
		size = e1->ex_Stor.st_StrLen;
	    }
	} else {
	    type = exp->ex_Type;
	    size = *type->Size;
	    if (size == 0)
	    {
		if ((type->Id == TID_STRUCT) || (type->Id == TID_UNION))
		    Undefined_Tag(type, NULL, exp->ex_LexIdx);
		else
		    yerror(exp->ex_LexIdx, EERROR_SIZEOF_TYPE_0);
	    }
	}
	exp->ex_ExpL = NULL;
	AllocConstStor(&exp->ex_Stor, size, &LongType);
	exp->ex_Type = &LongType;
	exp->ex_Stor.st_Flags |= SF_NOSA;
	exp->ex_Func = GenIntConst;
	exp->ex_Flags |= EF_CRES;
    }
}

void
GenAddr(pexp)
Exp **pexp;
{
    Exp *exp = *pexp;
    Exp *e1;

    CallLeft();
    e1 = exp->ex_ExpL;

    if (GenPass == 0) {
	exp->ex_Flags |= e1->ex_Flags & EF_CALL;
	if (e1->ex_Token == TokVarRef)
	    e1->ex_Var->Flags |= VF_ADDR;

#ifdef NOTDEF
	if (e1->ex_Type->Id == TID_ARY) {
	    if (e1->ex_Stor.st_Type != ST_IntConst)	/*  no warning for offsetof()	*/
		yerror(e1->ex_LexIdx, EWARN_ADDR_ARRAY_REDUNDANT);
	    exp->ex_Type = e1->ex_Type;
	} else
#endif
	if (e1->ex_Type->Id == TID_BITFIELD) {
	    yerror(e1->ex_LexIdx, EERROR_ADDR_BITFIELD_ILLEGAL);
	} else {
	    exp->ex_Type = TypeToPtrType(e1->ex_Type);
	}
	exp->ex_Flags |= EF_CRES;

	/*
	 *  handle address of constant pointer, make it handlable as a
	 *  constant expression.
	 */

	if (e1->ex_Stor.st_Type == ST_PtrConst) {
	    asm_getlea(exp, &e1->ex_Stor, &exp->ex_Stor);
#ifdef NOTDEF
	    exp->ex_Stor.st_Type = ST_IntConst;
	    exp->ex_Stor.st_IntConst = e1->ex_Stor.st_PtrConst;
#endif
	    exp->ex_Stor.st_Flags |= SF_NOSA;
	    exp->ex_Func = GenIntConst;
	    exp->ex_Flags |= EF_CRES;
	    if (exp->ex_Type->Id == TID_ARY)
		exp->ex_Stor.st_Flags |= SF_LEA;
	} else if (e1->ex_Stor.st_Type == ST_IntConst) {
	    exp->ex_Stor = e1->ex_Stor;
	    exp->ex_Stor.st_Flags &= ~SF_LEA;
	}
    } else {
	if ((exp->ex_Flags & EF_RNU) == 0) {
	    if (e1->ex_Type->Id == TID_ARY) {
		exp->ex_Stor = e1->ex_Stor;
	    } else {
		FreeStorage(&e1->ex_Stor);
		asm_getlea(exp, &e1->ex_Stor, &exp->ex_Stor);
	    }
	} else {
	    FreeStorage(&e1->ex_Stor);
	}
    }
}

void
GenInd(pexp)
Exp **pexp;
{
    Exp *exp = *pexp;
    Exp *e1;

    CallLeft();
    e1 = exp->ex_ExpL;

    if (GenPass == 0) {
	exp->ex_Type = e1->ex_Type;
	exp->ex_Flags |= e1->ex_Flags & EF_CALL;

	if (exp->ex_Type->Id != TID_PTR && exp->ex_Type->Id != TID_ARY) {
	    yerror(exp->ex_LexIdx, EERROR_INDIRECTION_NOT_PTR);
	    return;
	}
	exp->ex_Type = e1->ex_Type->SubType;
	/*exp->ex_Flags |= EF_CRES;*/
    } else {
	FreeStorage(&e1->ex_Stor);
	if ((exp->ex_Flags & EF_RNU) == 0) {
	    asm_getind(exp, e1->ex_Type, &e1->ex_Stor, &exp->ex_Stor, -1, -1, AutoResultStorage(exp));
	}
	return;
    }
}

void
GenPreInc(pexp)
Exp **pexp;
{
    Exp *exp = *pexp;
    Exp *e1;

    CallLeft();
    e1 = exp->ex_ExpL;

    if (GenPass == 0) {
#ifdef NOTDEF
	if (e1->ex_Stor.st_Type == ST_IntConst) {
	    e1->ex_Stor.st_IntConst = !e1->ex_Stor.st_IntConst;
	    e1->ex_Type = &LongType;
	    *pexp = e1;
	    return;
	}
#endif
	exp->ex_Type = e1->ex_Type;
	exp->ex_Flags |= e1->ex_Flags & EF_CALL;
	exp->ex_Flags |= EF_CRES;

	if (e1->ex_Token == TokVarRef)
	    e1->ex_Var->RegFlags |= RF_MODIFIED;
	if (e1->ex_Type->Id == TID_FLT) {
	    exp->ex_Flags |= EF_CALL;
	    GenFlagCallMade();
	}
    } else {
	Stor con;

	if (exp->ex_Type->Id == TID_FLT) {
	    AllocFltConstStor(&con, "1", 1, exp->ex_Type);
	    asm_fpadd(exp, exp->ex_Type, &con, &e1->ex_Stor, &e1->ex_Stor);
	} else {
	    AllocConstStor(&con, AutoIncDecSize(e1), e1->ex_Type);
	    asm_add(exp, &con, &e1->ex_Stor, &e1->ex_Stor);
	}
	FreeStorage(&e1->ex_Stor);

	if ((exp->ex_Flags & EF_RNU) == 0)
	    ReuseStorage(&e1->ex_Stor, &exp->ex_Stor);
    }
}

void
GenPreDec(pexp)
Exp **pexp;
{
    Exp *exp = *pexp;
    Exp *e1;

    CallLeft();
    e1 = exp->ex_ExpL;

    if (GenPass == 0) {
#ifdef NOTDEF
	if (e1->ex_Stor.st_Type == ST_IntConst) {
	    e1->ex_Stor.st_IntConst = !e1->ex_Stor.st_IntConst;
	    e1->ex_Type = &LongType;
	    *pexp = e1;
	    return;
	}
#endif
	exp->ex_Type = e1->ex_Type;
	exp->ex_Flags |= e1->ex_Flags & EF_CALL;
	exp->ex_Flags |= EF_CRES;

	if (e1->ex_Token == TokVarRef)
	    e1->ex_Var->RegFlags |= RF_MODIFIED;
	if (e1->ex_Type->Id == TID_FLT) {
	    exp->ex_Flags |= EF_CALL;
	    GenFlagCallMade();
	}
    } else {
	Stor con;

	if (exp->ex_Type->Id == TID_FLT) {
	    AllocFltConstStor(&con, "1", 1, exp->ex_Type);
	    asm_fpsub(exp, exp->ex_Type, &e1->ex_Stor, &con, &e1->ex_Stor);
	} else {
	    AllocConstStor(&con, AutoIncDecSize(e1), e1->ex_Type);
	    asm_sub(exp, &e1->ex_Stor, &con, &e1->ex_Stor);
	}
	FreeStorage(&e1->ex_Stor);

	if ((exp->ex_Flags & EF_RNU) == 0)
	    ReuseStorage(&e1->ex_Stor, &exp->ex_Stor);
    }
}

/*
 *  GenIntConst()
 */

void
GenIntConst(pexp)
Exp **pexp;
{
    Exp *exp = *pexp;

    if (GenPass == 0) {
	Type *type;
	long value = exp->ex_Stor.st_IntConst;

	if (exp->ex_Stor.st_Flags & SF_UNSIGNED) {
	    type = &ULongType;

	    if ((unsigned long)value < 65536)
		type = &UShortType;
	    if ((unsigned long)value < 256)
		type = &UCharType;
	} else {
	    type = &LongType;

	    if (value >= -32768 && value < 32768)
		type = &ShortType;
	    if (value >= -128 && value < 128)
		type = &CharType;
	}


	exp->ex_Type = type;
	exp->ex_Flags |= EF_CRES;
	AllocConstStor(&exp->ex_Stor, value, type);
    } else {
	exp->ex_Stor.st_Size = *exp->ex_Type->Size;

	if (exp->ex_Flags & EF_COND) {
	    exp->ex_Flags |= EF_CONDACK;
	    if (exp->ex_Cond >= 0 && exp->ex_Stor.st_IntConst)
		asm_branch(exp->ex_LabelT);
	    if (exp->ex_Cond < 0  && !exp->ex_Stor.st_IntConst)
		asm_branch(exp->ex_LabelF);
	}
	if (exp->ex_Type->Flags & TF_UNSIGNED)
	    exp->ex_Stor.st_Flags |= SF_UNSIGNED;
	else
	    exp->ex_Stor.st_Flags &= ~SF_UNSIGNED;
    }
}

void
GenStrConst(pexp)
Exp **pexp;
{
    Exp *exp = *pexp;
    long l;

    if (GenPass == 0) {
	long iidx = -1;		/* internationalization index */
	l = AllocLabel();

	exp->ex_Flags |= EF_CRES;

#ifdef COMMERCIAL
	iidx = Internationalize(exp->ex_StrConst, exp->ex_StrLen);
#endif
	AddStrList(exp->ex_StrConst, exp->ex_StrLen, l, iidx);
#ifdef COMMERCIAL
	if (iidx >= 0) {
	    l = -l;
	    exp->ex_Type = &CharPtrType;
	} else
#endif
	{
	    exp->ex_Type = &CharAryType;
	}

	exp->ex_Stor.st_StrConst = exp->ex_StrConst;
	exp->ex_Stor.st_StrLen	 = exp->ex_StrLen;
	exp->ex_Stor.st_Type = ST_StrConst;
	exp->ex_Stor.st_Flags = SF_NOSA;
	exp->ex_Stor.st_Label = l;
    } else {
#ifdef COMMERCIAL
	if (exp->ex_Stor.st_Label < 0) {
	    exp->ex_Stor.st_Label = -exp->ex_Stor.st_Label;
	    exp->ex_Stor.st_Flags= 0;
	    exp->ex_Stor.st_Type = ST_RelLabel;
	} else
#endif
	{
	    exp->ex_Stor.st_Flags= SF_LEA;
	    exp->ex_Stor.st_Type = ST_RelLabel;

	    if (ConstCode)
		exp->ex_Stor.st_Flags |= SF_CODE;   /*	in code section */
	}
	exp->ex_Stor.st_Size = PTR_SIZE;
	exp->ex_Stor.st_Offset = 0;
    }
}

void
GenFltConst(pexp)
Exp **pexp;
{
    Exp *exp = *pexp;

    if (GenPass == 0) {
	Assert(exp->ex_Type);
	exp->ex_Flags |= EF_CRES;
	AllocFltConstStor(&exp->ex_Stor, exp->ex_Stor.st_FltConst, exp->ex_Stor.st_FltLen, exp->ex_Type);
    } else {
	exp->ex_Stor.st_Size = *exp->ex_Type->Size;

	if (exp->ex_Flags & EF_COND) {
	    exp->ex_Flags |= EF_CONDACK;
	    if (exp->ex_Cond >= 0 && FltIsZero(exp, exp->ex_Stor.st_FltConst, exp->ex_Stor.st_FltLen))
		asm_branch(exp->ex_LabelT);
	    if (exp->ex_Cond < 0  && !FltIsZero(exp, exp->ex_Stor.st_FltConst, exp->ex_Stor.st_FltLen))
		asm_branch(exp->ex_LabelF);
	}

#ifdef NOTDEF
	/*
	 *  Convert an fp string constant into a memory constant and change
	 *  the storage type to label.
	 */

	asm_data_mask(0);   XXX
	asm_fltconst(&exp->ex_Stor, 1);
	asm_code_mask(0);   XXX
#endif
    }
}

/*
 *  left side is ex_Var, a variable.  Right side is ex_Symbol, a symbol.
 *
 *  For array references we return the effective address of a pointer to
 *  the array.	BUT, if the array is passed as an argument we return only
 *  a pointer to the array (since we can't get the eff.addr without generating
 *  code).
 *
 *  If a register variable we have a problem... The register could have been
 *  forced and reused.
 *
 *  __dynamic variables are automatically indirected through.
 */

void
GenVarRef(pexp)
Exp **pexp;
{
    Exp *exp = *pexp;
    Var *var = exp->ex_Var;

    if (GenPass == 0) {
	Assert(var);

	/*
	 *  dynamic variables are automatically indirected
	 */

#ifdef DYNAMIC
	if ((var->Flags & TF_DYNAMIC) && (exp->ex_Flags & EF_DYNAMIC) == 0) {
	    exp->ex_Flags |= EF_DYNAMIC;
	    InsertNot(pexp);
	    exp = *pexp;
	    exp->ex_Func = GenInd;
	    (*exp->ex_Func)(pexp);
	    return;
	}
#endif
	exp->ex_Type = var->Type;
	exp->ex_Flags |= EF_CRES;
	exp->ex_Stor.st_Flags |= SF_VAR;

	if (var->Type->Id == TID_PROC)
	    exp->ex_Type = TypeToPtrType(var->Type);

	if (var->Type->Id == TID_ARY && (var->Flags & VF_ARG))
	    exp->ex_Type = TypeToPtrType(exp->ex_Type->SubType);
    } else {
	exp->ex_Stor = var->var_Stor;
	exp->ex_Stor.st_Flags |= SF_VAR;

	/*
	 *  check for a change in type.  In order to modify this
	 *  expression's type without adding another expression node
	 *  the type's Id and size may not be changed (see InsertCast())
	 */

	if (exp->ex_Type->Flags & TF_UNSIGNED)
	    exp->ex_Stor.st_Flags |= SF_UNSIGNED;
	else
	    exp->ex_Stor.st_Flags &= ~SF_UNSIGNED;

	if (var->Type->Id == TID_PROC) {
	    if (var->var_Stor.st_Type == ST_RelName) {
		if (var->var_Stor.st_Name->Len == 6 && strncmp(var->var_Stor.st_Name->Name, "alloca", 6) == 0) {
		    yerror(exp->ex_LexIdx, EERROR_ALLOCA);
		}
		if (var->var_Stor.st_Name->Len == 12 && strncmp(var->var_Stor.st_Name->Name, "_dice_alloca", 12) == 0) {
		    ForceLinkFlag = 1;
		    puts("; FORCE LINK");
		}
	    }
	}

	/*
	 *  XXX - handling of registerized procedure name access
	 *	  ('@' or '_')  -mR option. XXX
	 */

#ifdef NOTDEF

	if (var->Type->Flags & TF_REGCALLOK) {
	    if (RegCallOpt > 2 || (exp->ex_Flags & EF_DIRECT)) {
		/* printf("VARFLAG = %08lx\n", var->Flags); */
		if (IsRegCall(var->Flags)) {
		    exp->ex_Stor.st_Flags |= SF_REGARGS;
		    var->var_Stor.st_Flags |= SF_REGARGSUSED;
		}
	    }
	}
#endif
	if (var->Type->Flags & TF_REGCALL) {
	    exp->ex_Stor.st_Flags |= SF_REGARGS;
	    var->var_Stor.st_Flags |= SF_REGARGSUSED;
	}


	if (exp->ex_Stor.st_Type == 0)
	{
	    yerror(exp->ex_LexIdx, EERROR_ILLEGAL_ASSIGNMENT);
	    dbprintf(("var %s has no type!", SymToString(var->Sym)));
	    Assert(0);
	}

#ifdef NOTDEF
	if ((var->Flags & TF_EXTERN) && !(var->Flags & VF_DECLD)) {
	    AddExternList(var);
	    var->Flags |= VF_DECLD;
	}
#endif
	if ((var->Flags & VF_DECLD) == 0) {
	    if (var->Flags & TF_EXTERN)
		AddExternList(var);
	    var->Flags |= VF_DECLD;
	}

	exp->ex_Stor.st_Size = *exp->ex_Type->Size;
	if (var->Type->Id == TID_ARY) {
	    if (var->Flags & VF_ARG) {
		exp->ex_Stor.st_Size = PTR_SIZE;
	    } else {
		exp->ex_Stor.st_Flags |= SF_LEA;
		exp->ex_Stor.st_Size = PTR_SIZE;
	    }
	}
	if (var->Type->Id == TID_PROC) {
	    exp->ex_Stor.st_Flags |= SF_LEA;	    /*	ptr to procedure */
	    exp->ex_Stor.st_Size = PTR_SIZE;
	}
    }
}

/*
 *  right side of call is an expression list rather than just an expression
 */

void
GenCall(pexp)
Exp **pexp;
{
    Exp *exp = *pexp;
    Exp *e1;

    if (GenPass == 0) {
	Exp **pe = &exp->ex_ExpR;
	Exp *e2;
	Exp *en;

	CallLeft();
	e1 = exp->ex_ExpL;

	while ((e2 = *pe) != NULL) {
	    en = e2->ex_Next;

	    (*e2->ex_Func)(pe);

	    (*pe)->ex_Next = en;
	    pe = &(*pe)->ex_Next;
	}
#ifdef NOTDEF
	if (e1->ex_Stor.st_Type == ST_IntConst) {
	    e1->ex_Stor.st_IntConst = !e1->ex_Stor.st_IntConst;
	    *pexp = e1;
	    return;
	}
#endif
	{
	    Type *t = e1->ex_Type;

	    if (t->Id == TID_PTR)
		t = t->SubType;

	    if (t->Id != TID_PROC) {
		yerror(e1->ex_LexIdx, EERROR_LHS_NOT_PROCEDURE);
		exp->ex_Type = &LongType;
	    } else {
		exp->ex_Type = t->SubType;  /*	return type of proc */
		if (ProtoOnlyOpt && !(t->Flags & TF_PROTOTYPE))
		    yerror(e1->ex_LexIdx, EERROR_UNPROTOTYPED_CALL);
	    }

	    GenCastArgs(t, exp, &exp->ex_ExpR);
	}
	exp->ex_Flags |= EF_CALL;
	GenFlagCallMade();
    } else {
	Stor *calls;
	Stor ts;
	short struct_ret;
	PragNode *pragma_call;
	Type *type = exp->ex_ExpL->ex_Type;
	char prgno[16];     /*	pragma register ordering    */

	if (type->Id == TID_PTR)
	    type = type->SubType;
	Assert(type->Id == TID_PROC);

	e1 = exp->ex_ExpL;
	e1->ex_Flags |= EF_DIRECT;

	if (type->SubType->Id == TID_STRUCT || type->SubType->Id == TID_UNION)
	    struct_ret = 1;
	else
	    struct_ret = 0;

	if (exp->ex_ExpL->ex_Token == TokVarRef) {
	    pragma_call = TestPragmaCall(exp->ex_ExpL->ex_Var, prgno);
	    if (pragma_call && (type->Flags & TF_PROTOTYPE) == 0)
		yerror(exp->ex_LexIdx, EERROR_PROTO_REQUIRED_INLINE);
	} else {
	    pragma_call = NULL;
	}

	if (pragma_call == NULL && (type->Flags & TF_REGCALL) == 0) {
	    /*
	     *	STACK BASED CALL
	     */

	    short autop;
	    long bytes;

	    if (exp->ex_ExpR)
		bytes = GenPushArgs(exp->ex_ExpR);
	    else
		bytes = 0;

	    if ((exp->ex_Flags & EF_STACK) && struct_ret == 0) {
		exp->ex_Flags |= EF_STACKACK;
		exp->ex_Flags |= EF_RNU;
		autop = 1;
	    } else {
		CreateUnaryResultStorage(exp, 0);
		autop = 0;
	    }

	    /*
	     *	Calculate call ea
	     */

	    CallLeft();
	    e1 = exp->ex_ExpL;
	    calls = &e1->ex_Stor;

	    FreeStorage(&e1->ex_Stor);

	    if (e1->ex_Type->Id == TID_PTR) {
		asm_getind(exp, e1->ex_Type, &e1->ex_Stor, &ts, -1, -1, 0);
		calls = &ts;
		FreeStorage(calls);
	    }

	    /*
	     *	stack call, must clear SF_REGARGS so '_' label is used
	     */

	    calls->st_Flags &= ~SF_REGARGS;

	    if (exp->ex_Flags & EF_RNU)
		asm_call(exp, calls, exp->ex_Type, NULL, bytes, autop);
	    else
		asm_call(exp, calls, exp->ex_Type, &exp->ex_Stor, bytes, autop);

	} else {
	    /*
	     *	REGISTERIZED PROCEDURE CALL
	     *
	     *	    (1) calculate those arguments whos register destinations
	     *		do not conflict directly to the appropriate
	     *		register destinations
	     *
	     *	    (2) calculate the call address to a temporary
	     *
	     *	    (3) calculate those arguments whos register destinations
	     *		conflict, calculate them to non-conflicting temporary
	     *		variables.
	     *
	     *	    (4) save registers in the conflict zone
	     *
	     *	    (5) move remaining temporary results to actual registers
	     *
	     *	    (6) make call (do not assign result storage yet)
	     *
	     *	    (7) restore registers in the conflict zone, move result
	     *		storage elsewhere if it conflicts.
	     *
	     *	    (8) assign result storage
	     */

	    ulong conMask;	/*  conflict mask		     */
	    ulong ignMask = -1; /*  conflict save/restore mask	     */
	    char argno[16];	/*  procedure call register ordering */
	    char actno[16];	/*  actual regs for initial arg load */
	    char scReg[5];	/*  scratch registers saved	     */
	    Stor res;		/*  result storage backing store     */
	    short resBack = 0;	/*  backing store active	     */

	    scReg[0] = -1;

	    /*
	     *	Generate result storage
	     */

	    CreateUnaryResultStorage(exp, 0);

	    /*
	     *	Generate a register list and a conflict mask.  This call
	     *	also sets the 'used' bit for the registers if it is not
	     *	already set.
	     */

	    conMask = RegCallOrder(type, argno, (pragma_call) ? prgno : NULL);

	    dbprintf((";conMask = %08lx\n", conMask));

	    /*
	     *	Generate arguments for those bits that do not conflict with
	     *	anything.  The storage for these arguments will be locked.
	     */

	    GenRegArgs(exp->ex_ExpR, argno, actno, ~conMask, scReg, 0);

	    /*
	     *	If the result storage conflicts with a register and is
	     *	not register-direct then we have to assign it to temporary
	     *	storage first.
	     *
	     *	If it is register direct (& conflict) then we do not have
	     *	to assign it to temporary storage but must clear the
	     *	conflict bit so we do not restore over our result
	     *
	     *	Note that since the storage for the result in the case of
	     *	a hard conflict is allocated after non-conflicting registers
	     *	are allocated it will thus not conflict with anything and
	     *	therefore not get restored over.
	     */

	    if (RegisterMaskConflict(&exp->ex_Stor, conMask)) {
		if (exp->ex_Stor.st_Type == ST_Reg) {
		    ignMask = ~(1 << exp->ex_Stor.st_RegNo);
		} else {
		    res = exp->ex_Stor;
		    resBack = 1;
		    AllocAnyRegister(&exp->ex_Stor, exp->ex_Type, NULL);
		}
	    }

	    /*
	     *	Generate arguments for those bits that conflict with
	     *	something.  Results will be written to other temporary
	     *	registers or pushed onto the stack, and locked.
	     */

	    GenRegArgs(exp->ex_ExpR, argno, actno, conMask & ignMask, scReg, 1);

	    /*
	     *	Generate call address to non-conflicting temporary
	     *
	     *	If the call is a __libcall we offset it by the first
	     *	argument.
	     *
	     *	Otheriwse,
	     *
	     *	If the call address is a pointer we have to resolve it,
	     *	if the register conflicts with an argument we have to
	     *	copy it to another register.
	     *
	     *	We special case TID_PTR where the pointer is the same as
	     *	the first argument in the call (the library base variable)
	     *
	     *	XXX call address not guarenteed to be in non-conflicting
	     *	register!
	     */

	    InMaskPush(e1, scReg);

	    CallLeft();
	    e1 = exp->ex_ExpL;
	    calls = &e1->ex_Stor;

	    if (pragma_call) {
		AllocRegisterAbs(&ts, RB_A6, 4);
		ts.st_Type = ST_RelReg;
		ts.st_Offset = -pragma_call->pn_Offset;
		calls = &ts;
	    } else if (RegisterMaskConflict(calls, conMask)) {
		AllocAddrRegister(&ts);
		if (e1->ex_Type->Id == TID_PTR)
		    asm_move(exp, calls, &ts);
		else
		    asm_lea(exp, calls, 0, &ts);
		FreeStorage(&ts);
		asm_getind(exp, &LongPtrType, &ts, &ts, -1, -1, 0);
		FreeStorage(calls);
		calls = &ts;
	    } else if (e1->ex_Type->Id == TID_PTR) {
		asm_getind(e1, e1->ex_Type, &e1->ex_Stor, &ts, -1, -1, 0);
		FreeStorage(calls);
		calls = &ts;
	    }

	    /*
	     *	Optimize result storage if it was a conflicting register,
	     *	as well as prevent restoring over the result.  We force
	     *	the argument into the conflicted register (and do this last
	     *	so there will be no further access to the now destructed
	     *	register)
	     */

	    if (ignMask != -1)
		GenRegArgs(exp->ex_ExpR, argno, actno, ~ignMask, scReg, -1);

	    /*
	     *	While calculating arguments or the call address if any
	     *	subexpression had made a call we had to (and did) save
	     *	any allocated scratch registers we need.  We have to
	     *	restore them here.
	     */

	    InMaskPop(scReg);

	    /*
	     *	Save conflicting variables and move already-generated
	     *	arguments associated with said conflicts into their
	     *	proper registers.  (note: since this survives the call
	     *	we cannot save conflicts into scratch registers)
	     *
	     *	We do not save any variable that represents the result
	     *	storage as this would cause the result storage to be
	     *	overwritten on restore.
	     */

	    SaveCopyConflictsRegArgs(exp->ex_ExpR, argno, actno, conMask & ignMask, 0);

	    /*
	     *	Make the call
	     */

	    FreeStorage(calls);
	    if (exp->ex_Flags & EF_RNU)
		asm_call(exp, calls, exp->ex_Type, NULL, 0, 2);
	    else
		asm_call(exp, calls, exp->ex_Type, &exp->ex_Stor, 0, 2);

	    /*
	     *	Restore conflicted variables, either from other registers
	     *	or off the stack.
	     */

	    SaveCopyConflictsRegArgs(exp->ex_ExpR, argno, actno, conMask & ignMask, 1);

	    FreeRegArgs(exp->ex_ExpR, actno);

	    /*
	     *	If the result would have conflicted we created a temporary
	     *	for it and now must store it back after all other registers
	     *	have been restored.
	     */

	    if (resBack) {
		FreeStorage(&exp->ex_Stor);
		asm_move(exp, &exp->ex_Stor, &res);
		exp->ex_Stor = res;
	    }
	}
    }
}

/*
 *  Generate arguments to registers.  If tmpOk is 1 then argument
 *  will be generated to a temporary if the associated register is
 *  not available.
 *
 *  Must handle case where char or short quantity is placed in an
 *  address register
 *
 *  Must handle pragma case where extra register in argno/actno is the
 *  base register
 *
 *  tmpOk
 *	-1	no, force register to be used even if a conflict
 *	 0	no, register must be free
 *	 1	yes
 */

void
GenRegArgs(
    Exp *exp,
    char *argno,
    char *actno,
    ulong mask,
    char *scReg,
    short tmpOk
) {
    Stor t;

    for (; exp; exp = exp->ex_Next, ++argno, ++actno) {
	short rno = -1;

	/*
	 *  Do only the requested arguments
	 */

	if ((mask & (1 << *argno)) == 0)
	    continue;

	/*printf(";GenRegArgs tmp = %d argno = %d RegAlloc=%08lx RegLocked=%08lx RegUsed=%08lx\n", tmpOk, *argno, RegAlloc, RegLocked, RegUsed);*/

	/*
	 *  Attempt to optimize destination register for the subexpression,
	 *  but cannot do this if the subexpression contains a call
	 *  (screws up register save & restore).  Also, due to __dp calls
	 *  D0 must be the last to be resolved.  XXX ???
	 *
	 *  Do not optimize ADDRESS REG destination if exptype size is not 4,
	 *  don't think expression generation routines can handle it.
	 */

	if (*argno < RB_ADDR || *exp->ex_Type->Size == 4) {
	    if ((exp->ex_Flags & (EF_CRES|EF_ASSEQ)) == 0 && !(exp->ex_Flags & EF_CALL)) {
		if (tmpOk < 0)
		    rno = AllocRegisterAbs(&t, *argno, *exp->ex_Type->Size);
		else
		    rno = AttemptAllocRegisterAbs(&t, *argno, *exp->ex_Type->Size);
		exp->ex_Stor = t;
		exp->ex_Flags |= EF_PRES;
	    }
	}

	InMaskPush(exp, scReg);
	(*exp->ex_Func)(&exp);

	if ((exp->ex_Flags & EF_PRES) == 0) {
	    if (((1 << *argno) & REGSCRATCH) == 0 && exp->ex_Stor.st_Type == ST_Reg && exp->ex_Stor.st_RegNo == *argno && *exp->ex_Type->Size == exp->ex_Stor.st_Size) {
		rno = *argno;
		AllocRegisterAbs(&t, rno, exp->ex_Stor.st_Size);
	    } else {
		FreeStorage(&exp->ex_Stor);
		if (tmpOk < 0)
		    rno = AllocRegisterAbs(&t, *argno, *exp->ex_Type->Size);
		else
		    rno = AttemptAllocRegisterAbs(&t, *argno, *exp->ex_Type->Size);

		/*
		 *  Hack the destination register size.  Set to 2 if short
		 *  or char type.  Even though we are moving 'garbage'
		 *  into the high bits of the register, the type sized bits
		 *  will be correct.
		 */

		if (rno >= RB_ADDR && *exp->ex_Type->Size < 4) {
		    exp->ex_Stor.st_Size = 2;
		    t.st_Size = 2;
		}
		asm_move_cast(exp, &exp->ex_Stor, &t);
	    }
	}
	*actno = rno;

	/*printf(";GenRegArgs actual = %d\n", rno);*/

	if ((1 << rno) & REGSCRATCH) {
	    short i;
	    for (i = 0; i < 4 && (short)(scReg[i] & 0x7F) != rno; ++i) {
		if (scReg[i] == -1) {
		    scReg[i] = rno;
		    scReg[i+1] = -1;
		    break;
		}
	    }
	}

	if (tmpOk <= 0 && rno != *argno)
	{
	    dbprintf(("rno alloc failed %d/%d (%d)\n", rno, *argno, tmpOk));
	    Assert(0);
	}
    }
}

/*
 *  We have to save any scratch registers we have in use if the expression
 *  we are about to evaluate contains a call.  We do not re-save registers
 *  which have already been saved.
 */

void
InMaskPush(exp, scReg)
Exp *exp;
char *scReg;
{
    if (exp->ex_Flags & EF_CALL) {
	short i;
	char c;

	for (i = 0; i < 4 && (c = scReg[i]) != -1; ++i) {
	    if ((c & 0x80) == 0) {
		scReg[i] |= 0x80;
		asm_push_mask(1 << c);
	    }
	}
    }
}

/*
 *  Before completing the call we have to pop any registers we had saved,
 *  and must be sure to pop them in the correct order.
 */

void
InMaskPop(scReg)
char *scReg;
{
    short i;
    char c;

    for (i = 0; i < 4 && scReg[i] != -1; ++i)
	;
    for (--i; i >= 0; --i) {
	if ((c = scReg[i]) & 0x80) {
	    asm_pop_mask(1 << (c & 0x7F));
	}
    }
}

/*
 *  Save conflicting registers while simultaniously moving the stored
 *  temporary into the conflicting register spot.  The stored temporaries
 *  do not conflict with any of these registers.
 *
 *  Only those registers represented by <mask> might conflict and are
 *  considered.  If possible, we EXG the temporaries with the real
 *  registers.	Otherwise we store the real registers on the stack and
 *  MOVE from the temporaries.	On restore the opposite action occurs
 *
 *  Since the register(s) are already allocated we can allocate/free
 *  without actually freeing them up.
 */

void
SaveCopyConflictsRegArgs(
    Exp *expBase,
    char *argno,
    char *actno,
    ulong mask,
    short restore
) {
    Stor t;
    Stor a;
    Exp *exp;
    long regMask;
    short i;

    for (i = regMask = 0, exp = expBase; exp; exp = exp->ex_Next, ++i) {
	if (mask & (1 << argno[i])) {
	    if ((1 << actno[i]) & REGSCRATCH) {
		if (restore == 0)
		    regMask |= 1 << argno[i];
	    }
	}
    }
    asm_push_mask(regMask);

    for (i = regMask = 0, exp = expBase; exp; exp = exp->ex_Next, ++i) {
	if (mask & (1 << argno[i])) {
	    AllocRegisterAbs(&t, argno[i], exp->ex_Stor.st_Size);
	    AllocRegisterAbs(&a, actno[i], exp->ex_Stor.st_Size);

	    /*
	     *	AllocRegisterAbs() forces the size of an address register
	     *	to 4 currently, XXX this is a hack
	     */

	    if (exp->ex_Stor.st_Size < 4) {
		t.st_Size = 2;
		a.st_Size = 2;
	    }

	    if ((1 << actno[i]) & REGSCRATCH) {
		if (restore) {
		    regMask |= 1 << argno[i];
		} else {
		    asm_move(exp, &a, &t);
		}
	    } else {
		asm_exg(&a, &t);
	    }
	    FreeRegister(&a);
	    FreeRegister(&t);
	}
    }
    asm_pop_mask(regMask);
}

/*
 *  Free registers allocated for the call arguments, clean up.	Note
 *  that if we do not properly free arguments a software error, such
 *  as 'rno alloc failed' from the level above may occur.
 */

void
FreeRegArgs(exp, actno)
Exp *exp;
char *actno;
{
    Stor t;

    for (; exp; exp = exp->ex_Next, ++actno) {
	AllocRegisterAbs(&t, *actno, exp->ex_Stor.st_Size);
	FreeStorage(&t);
	FreeStorage(&t);
    }
}

Local long
GenPushArgs(exp)
Exp *exp;
{
    long bytes;

    if (exp->ex_Next)
	bytes = GenPushArgs(exp->ex_Next);
    else
	bytes = 0;
    return(bytes + PushArgExp(exp));
}

int
PushArgExp(exp)
Exp *exp;
{
    exp->ex_Flags |= EF_STACK;
    (*exp->ex_Func)(&exp);

    if (exp->ex_Flags & EF_STACKACK) {
	return(asm_stackbytes(exp->ex_Type));
    } else {
	FreeStorage(&exp->ex_Stor);
	return(asm_push(exp, exp->ex_Type, &exp->ex_Stor));
    }
}

void
GenCastArgs(type, cexp, pexp)
Type *type;
Exp *cexp;
Exp **pexp;
{
    Exp *exp = *pexp;
    short i;

    /*
     *	if type->Args < 0 it is not a prototype, but that is ok the way
     *	our loop works.
     */

    dbprintf((";%d args, flags = %08lx\n", type->Args, type->Flags));

    for (i = 0; i < type->Args; ++i) {
	Type *vtype;
	if (exp == NULL || (exp->ex_Flags & EF_SPECIAL)) {
	    yerror(cexp->ex_LexIdx, EWARN_TOO_FEW_PARAMETERS);
	    break;
	}

	vtype = ActualPassType(type, type->Vars[i]->Type, 0);

	dbprintf((";arg %d size=%ld argsize=%ld use=%ld\n", i, *exp->ex_Type->Size, *type->Vars[i]->Type->Size, *vtype->Size));

	if (exp->ex_Type != vtype) {
	    if (exp->ex_Type->Id == TID_INT && (vtype->Id == TID_INT || (exp->ex_Stor.st_Type == ST_IntConst && exp->ex_Stor.st_IntConst == 0))) {
		InsertCast(pexp, vtype);
	    } else if (CheckConversion(exp, exp->ex_Type, vtype)) {
		InsertCast(pexp, vtype);    /*	exp no longer valid */
	    }
	}

	exp = *pexp;
	pexp = &exp->ex_Next;
	exp = *pexp;
    }
    {
	Exp *ex;

	for (ex = exp; ex && (ex->ex_Flags & EF_SPECIAL); ex = ex->ex_Next)
	    ;
	if (ex && (type->Flags & TF_DOTDOTDOT) == 0 && type->Args >= 0)
	    yerror(ex->ex_LexIdx, EWARN_TOO_MANY_PARAMETERS);
    }

    while (exp) {
	Type *vtype = ActualPassType(type, exp->ex_Type, 1);

	if (exp->ex_Type != vtype)
	    InsertCast(pexp, vtype);	/*  exp no longer valid */
	exp = *pexp;
	pexp = &exp->ex_Next;
	exp = *pexp;
    }
}

void
GenPosInc(pexp)
Exp **pexp;
{
    Exp *exp = *pexp;
    Exp *e1;

    CallLeft();
    e1 = exp->ex_ExpL;

    if (GenPass == 0) {
#ifdef NOTDEF
	if (e1->ex_Stor.st_Type == ST_IntConst) {
	    e1->ex_Stor.st_IntConst = !e1->ex_Stor.st_IntConst;
	    *pexp = e1;
	    return;
	}
#endif
	exp->ex_Type = e1->ex_Type;
	exp->ex_Flags |= e1->ex_Flags & EF_CALL;

	if (e1->ex_Token == TokVarRef)
	    e1->ex_Var->RegFlags |= RF_MODIFIED;
	if (e1->ex_Type->Id == TID_FLT) {
	    exp->ex_Flags |= EF_CALL;
	    GenFlagCallMade();
	}
    } else {
	Stor con;

	if (exp->ex_Type->Id == TID_FLT)
	    AllocFltConstStor(&con, "1", 1, exp->ex_Type);
	else
	    AllocConstStor(&con, AutoIncDecSize(e1), e1->ex_Type);

	CreateUnaryResultStorage(exp, 0);
	if ((exp->ex_Flags & EF_RNU) == 0) {
	    exp->ex_Stor.st_Flags |= e1->ex_Stor.st_Flags & SF_UNSIGNED;
	    asm_move(exp, &e1->ex_Stor, &exp->ex_Stor);
	}
	FreeStorage(&e1->ex_Stor);
	if (exp->ex_Type->Id == TID_FLT)
	    asm_fpadd(exp, exp->ex_Type, &con, &e1->ex_Stor, &e1->ex_Stor);
	else
	    asm_add(exp, &con, &e1->ex_Stor, &e1->ex_Stor);
    }
}

void
GenPosDec(pexp)
Exp **pexp;
{
    Exp *exp = *pexp;
    Exp *e1;

    CallLeft();
    e1 = exp->ex_ExpL;

    if (GenPass == 0) {
#ifdef NOTDEF
	if (e1->ex_Stor.st_Type == ST_IntConst) {
	    e1->ex_Stor.st_IntConst = !e1->ex_Stor.st_IntConst;
	    *pexp = e1;
	    return;
	}
#endif
	exp->ex_Flags |= e1->ex_Flags & EF_CALL;
	exp->ex_Type = e1->ex_Type;

	if (e1->ex_Token == TokVarRef)
	    e1->ex_Var->RegFlags |= RF_MODIFIED;
	if (e1->ex_Type->Id == TID_FLT) {
	    exp->ex_Flags |= EF_CALL;
	    GenFlagCallMade();
	}
    } else {
	Stor con;

	if (exp->ex_Type->Id == TID_FLT)
	    AllocFltConstStor(&con, "1", 1, exp->ex_Type);
	else
	    AllocConstStor(&con, AutoIncDecSize(e1), e1->ex_Type);

	CreateUnaryResultStorage(exp, 0);
	if ((exp->ex_Flags & EF_RNU) == 0) {
	    exp->ex_Stor.st_Flags |= e1->ex_Stor.st_Flags & SF_UNSIGNED;
	    asm_move(exp, &e1->ex_Stor, &exp->ex_Stor);
	}
	FreeStorage(&e1->ex_Stor);
	if (exp->ex_Type->Id == TID_FLT)
	    asm_fpsub(exp, exp->ex_Type, &e1->ex_Stor, &con, &e1->ex_Stor);
	else
	    asm_sub(exp, &e1->ex_Stor, &con, &e1->ex_Stor);
    }
}

/*
 *  For Ind/Elm, exp->ex_Sym (right side is a symbol)
 *
 *  BITFIELD NOTE:  Only asm_getind() is guarenteed to leave the
 *		    st_BOffset and st_BSize fields alone.
 */

void
GenStructInd(pexp)
Exp **pexp;
{
    Exp *exp = *pexp;
    Exp *e1;
    int bfo;

    CallLeft();
    e1 = exp->ex_ExpL;

    if (GenPass == 0) {
	if (e1->ex_Type->Id != TID_PTR && e1->ex_Type->Id != TID_ARY) {
	    yerror(e1->ex_LexIdx, EERROR_INDIRECTION_NOT_PTR);
	    exp->ex_Type = &LongType;
	    return;
	}
	exp->ex_Offset = FindStructUnionElm(e1->ex_Type->SubType, exp, &bfo);
	if (exp->ex_Type == NULL)
	    exp->ex_Type = &LongType;
	if (exp->ex_Type->Id == TID_BITFIELD) {
	    BitFieldResultType(pexp, bfo);
	    exp = *pexp;
	    e1 = exp->ex_ExpL;
	} else {
	    exp->ex_Stor.st_BOffset = -1;
	    exp->ex_Stor.st_BSize   = -1;
	}
	/*exp->ex_Flags |= EF_CRES;*/
	exp->ex_Flags |= e1->ex_Flags & EF_CALL;

	/*
	 *  constant pointer
	 */

	if (exp->ex_Type->Id != TID_BITFIELD && e1->ex_Stor.st_Type == ST_IntConst) {
	    if (exp->ex_Type->Id == TID_ARY) {
		e1->ex_Stor.st_Type = ST_IntConst;
		e1->ex_Stor.st_Size = PTR_SIZE;
		e1->ex_Stor.st_IntConst = e1->ex_Stor.st_IntConst + exp->ex_Offset;
		e1->ex_Stor.st_Flags |= SF_LEA | SF_UNSIGNED;
	    } else {
		e1->ex_Stor.st_Type = ST_PtrConst;
		e1->ex_Stor.st_Size = *exp->ex_Type->Size;
		e1->ex_Stor.st_Offset = e1->ex_Stor.st_IntConst + exp->ex_Offset;
		e1->ex_Stor.st_Flags &= ~SF_LEA;
	    }
	    e1->ex_Type = exp->ex_Type;
	    *pexp = e1;
	}
    } else {
	Stor con;
	Stor t;

	FreeStorage(&e1->ex_Stor);

	if ((exp->ex_Flags & EF_RNU) == 0) {
	    AllocConstStor(&con, exp->ex_Offset, &LongType);
	    asm_getindex(exp, e1->ex_Type, &e1->ex_Stor, &con, 1, &t, 1, 0);
	    FreeStorage(&t);
	    if (!(exp->ex_Stor.st_Flags & SF_BITFIELD))
		exp->ex_Stor.st_BSize = -1;
	    asm_getind(exp, TypeToPtrType(exp->ex_Type), &t, &exp->ex_Stor, exp->ex_Stor.st_BOffset, exp->ex_Stor.st_BSize, AutoResultStorage(exp));

	    /*
	    if (exp->ex_Stor.st_Flags & SF_BITFIELD) {
		BitFieldResultExp(exp);
	    }
	    */
	}
    }
}

void
GenStructElm(pexp)
Exp **pexp;
{
    Exp *exp = *pexp;
    Exp *e1;
    int bfo;	    /*	for bitfields	*/

    CallLeft();
    e1 = exp->ex_ExpL;

    if (GenPass == 0) {
	{
	    Type *t = e1->ex_Type;

	    if (t->Id != TID_STRUCT && t->Id != TID_UNION)
		yerror(e1->ex_LexIdx, EERROR_NOT_STRUCT_UNION);

	    exp->ex_Offset = FindStructUnionElm(t, exp, &bfo);
	}
	if (exp->ex_Type == NULL)
	    exp->ex_Type = &LongType;
	if (exp->ex_Type->Id == TID_BITFIELD) {
	    BitFieldResultType(pexp, bfo);
	    exp = *pexp;
	    e1 = exp->ex_ExpL;
	} else {
	    exp->ex_Stor.st_BOffset = -1;
	    exp->ex_Stor.st_BSize   = -1;
	}
	/*exp->ex_Flags |= EF_CRES;*/
	exp->ex_Flags |= e1->ex_Flags & EF_CALL;

	/*
	 *  constant pointer
	 */

	if (exp->ex_Type->Id != TID_BITFIELD && e1->ex_Stor.st_Type == ST_IntConst) {
	    if (exp->ex_Type->Id == TID_ARY) {
		e1->ex_Stor.st_Type = ST_IntConst;
		e1->ex_Stor.st_Size = PTR_SIZE;
		e1->ex_Stor.st_IntConst = e1->ex_Stor.st_IntConst + exp->ex_Offset;
		e1->ex_Stor.st_Flags |= SF_LEA | SF_UNSIGNED;
	    } else {
		e1->ex_Stor.st_Type = ST_PtrConst;
		e1->ex_Stor.st_Size = *exp->ex_Type->Size;
		e1->ex_Stor.st_Offset = e1->ex_Stor.st_IntConst + exp->ex_Offset;
		e1->ex_Stor.st_Flags &= ~SF_LEA;
	    }
	    e1->ex_Type = exp->ex_Type;
	    *pexp = e1;
	}
    } else {
	Stor con;
	Stor t;
	Stor u;

	FreeStorage(&e1->ex_Stor);

	if ((exp->ex_Flags & EF_RNU) == 0) {
	    AllocConstStor(&con, exp->ex_Offset, &LongType);

	    asm_getlea(exp, &e1->ex_Stor, &t);
	    FreeStorage(&t);

	    asm_getindex(exp, TypeToPtrType(e1->ex_Type), &t, &con, 1, &u, 1, 0);
	    FreeStorage(&u);

	    if (!(exp->ex_Stor.st_Flags & SF_BITFIELD))
		exp->ex_Stor.st_BSize = -1;
	    asm_getind(exp, TypeToPtrType(exp->ex_Type), &u, &exp->ex_Stor, exp->ex_Stor.st_BOffset, exp->ex_Stor.st_BSize, AutoResultStorage(exp));
	    /*
	    if (exp->ex_Stor.st_Flags & SF_BITFIELD) {
		BitFieldResultExp(exp);
	    }
	    */
	}
    }
}

/*
 *  Handle bitfield result types.  If this isn't the LHS of a straight
 *  assignment, the type is coerced into an integer.
 */

void
BitFieldResultType(pexp, bfo)
Exp **pexp;
int bfo;
{
    Exp *exp = *pexp;

    exp->ex_Stor.st_BOffset = bfo;
    exp->ex_Stor.st_BSize   = *exp->ex_Type->Size;
    exp->ex_Stor.st_Flags |= SF_BITFIELD;

    /*
     *	bit field return type is an integer unless lhs of
     *	assignment.
     */

    if ((exp->ex_Flags & (EF_LHSASSIGN|EF_COND)) == 0) {
	InsertNot(pexp);
	exp = *pexp;
	if (exp->ex_ExpL->ex_Flags & EF_LHSASSEQ)
	    exp->ex_Flags |= EF_LHSASSEQ;
	exp->ex_Func = GenBFExt;
	exp->ex_Token = TokBFExt;   /*	FOR +=, -=, etc... */

	if (exp->ex_ExpL->ex_Type->Flags & TF_UNSIGNED)
	    exp->ex_Type = &ULongType;
	else
	    exp->ex_Type = &LongType;
    }
}

#ifdef NOTDEF
/*
 *  BitFieldResultExp
 */

void
BitFieldResultExp(exp)
Exp *exp;
{
    if (exp->ex_Flags & EF_LHSASSIGN)
	return;
    if (exp->ex_Flags & EF_COND)	/*  can handle some conds.*/
	return;
    /*
     *	but otherwise must store into integer
     */
    if (exp->ex_Stor.st_Flags & SF_BITFIELD) {
	Stor st = exp->ex_Stor;

	FreeStorage(&exp->ex_Stor);
	CreateUnaryResultStorage(exp, 0);
	asm_bfext(exp, &st, &exp->ex_Stor);
    }
}
#endif

void
GenBFExt(pexp)
Exp **pexp;
{
    Exp *e1;
    Exp *exp = *pexp;

    CallLeft();
    e1 = exp->ex_ExpL;

    if (GenPass == 0) {
	;
    } else {
	Assert(e1->ex_Stor.st_Flags & SF_BITFIELD);
	if ((exp->ex_Flags & EF_LHSASSEQ) == 0)
	    FreeStorage(&e1->ex_Stor);
	CreateUnaryResultStorage(exp, 0);
	asm_bfext(exp, &e1->ex_Stor, &exp->ex_Stor);
    }
}


void
GenArray(pexp)
Exp **pexp;
{
    Exp *exp = *pexp;
    Exp *e1;
    Exp *e2;

    CallLeft();
    EnsureReturnStorageLeft();
    CallRight();
    e1 = exp->ex_ExpL;
    e2 = exp->ex_ExpR;

    if (GenPass == 0) {
	Type *t1 = e1->ex_Type;

	exp->ex_Flags |= /* EF_CRES |*/ ((e1->ex_Flags | e2->ex_Flags) & EF_CALL);
	if (t1->Id == TID_PTR || t1->Id == TID_ARY) {
	    exp->ex_Type = t1->SubType;

	    if (e2->ex_Stor.st_Type != ST_IntConst) {
		if (asm_mul_requires_call(*t1->SubType->Size)) {
		    exp->ex_Flags |= EF_CALL;
		    GenFlagCallMade();
		}
		if (e2->ex_Type->Id != TID_INT)
		    yerror(e2->ex_LexIdx, EERROR_EXPECTED_INT_TYPE);
	    }
	    return;
	}
	yerror(e1->ex_LexIdx, EERROR_INDIRECTION_NOT_PTR);
	exp->ex_Type = &VoidPtrType;
    } else {
	Stor t;

	FreeStorage(&e1->ex_Stor);
	FreeStorage(&e2->ex_Stor);
	if ((exp->ex_Flags & EF_RNU) == 0) {
	    asm_getindex(exp, e1->ex_Type, &e1->ex_Stor, &e2->ex_Stor, *exp->ex_Type->Size, &t, 1, 0);
	    FreeStorage(&t);
	    asm_getind(exp, e1->ex_Type, &t, &exp->ex_Stor, -1, -1, AutoResultStorage(exp));
	}
    }
}


