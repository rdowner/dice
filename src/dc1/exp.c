/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  EXP.C
 */

#include "defs.h"

Prototype int32_t ExpToConstant(Exp *);
Prototype Type *ExpToType(Exp *);
Prototype void ExpToLValue(Exp *, Stor *, Type *);
Prototype void InsertCast(Exp **, Type *);
Prototype void InsertNot(Exp **);
Prototype void InsertBranch(Exp **, int32_t, int32_t);
Prototype void InsertAssign(Exp **, Var *);
Prototype int32_t AutoIncDecSize(Exp *);

int32_t
ExpToConstant(exp)
Exp *exp;
{
    int32_t value;
    short genPass = GenPass;

    Assert(exp);

    GenPass = 0;
    (*exp->ex_Func)(&exp);  /*	last arg illegal ptr */
    GenPass = genPass;

    if (exp->ex_Stor.st_Type != ST_IntConst) {
	yerror(exp->ex_LexIdx, EERROR_EXPECTED_INT_CONST);
	return(0);
    }
    value = exp->ex_Stor.st_IntConst;
    return(value);
}

Type *
ExpToType(exp)
Exp *exp;
{
    short genPass = GenPass;

    Assert(exp);

    GenPass = 0;
    (*exp->ex_Func)(&exp);  /*	last arg illegal ptr */
    GenPass = genPass;

    if (exp->ex_Type == NULL) {
	yerror(exp->ex_LexIdx, EERROR_ILLEGAL_RETURN_TYPE);
	return(&VoidType);
    }
    return(exp->ex_Type);
}

/*
 *  Convert an expression to an lvalue whos result type should be
 *  'type'.
 */

void
ExpToLValue(exp, stor, type)
Exp *exp;
Stor *stor;
Type *type;
{
    short genPass = GenPass;

    Assert(exp);

#ifdef NOTDEF
    /*
     *	Propogate const class for string constants
     */

    if (exp->ex_Stor.st_Type == ST_StrConst && type->Id == TID_PTR) {
	if (type->SubType->Flags & TF_CONST)
	    exp->ex_Flags |= EF_CONST;
    }
#endif

    GenPass = 0;
    (*exp->ex_Func)(&exp);
    GenPass = 1;
    (*exp->ex_Func)(&exp);
    GenPass = genPass;

    *stor = exp->ex_Stor;
    if (stor->st_Type == ST_IntConst || stor->st_Type == ST_StrConst || stor->st_Type == ST_FltConst) {
	stor->st_Size = *type->Size;
	return;
    }
    if (exp->ex_Type->Id == TID_ARY) {
	stor->st_Flags |= SF_LEA;
	return;
    }

    if ((stor->st_Flags & SF_LEA) == 0)
	yerror(exp->ex_LexIdx, EERROR_NOT_LVALUE);
    if (stor->st_Type == ST_RelReg || stor->st_Type == ST_RelArg)
	yerror(exp->ex_LexIdx, EERROR_NOT_LVALUE);
}

/*
 *  Casting (BEFORE pass 0)
 *
 *  Note that an explicit (cast) does not call this routine, but goes
 *  directly to GenCast.
 */

void
InsertCast(pexp, type)
Exp **pexp;
Type *type;
{
    Exp *exp;
    Exp *e1 = *pexp;
    Type *etype = e1->ex_Type;

    Assert(etype);

    if (etype->Id == TID_BITFIELD)	/*  force the cast  */
	goto force;

    /*
     *	Conversions that require no modifications other then unsignedness,
     *	which is handled by other routines (example: GenVarRef checks for
     *	ex_Type changes)
     */

    if (*type->Size == *etype->Size && etype->Id == type->Id && (type->Id == TID_INT || type->Id == TID_FLT)) {
	e1->ex_Type = type;
	return;
    }

    /*
     *	Integer  Constant to (Integer | Pointer)
     *	Floating Constant to (Integer | Pointer)
     */


    if (type->Id == TID_INT || type->Id == TID_PTR) {
	if (e1->ex_Stor.st_Type == ST_IntConst) {
	    short sv = *type->Size;

	    if (type->Flags & TF_UNSIGNED)
		sv |= 256;
	    switch(sv) {
	    case 0:
		e1->ex_Stor.st_IntConst = 0;
		/* XXX */
		break;
	    case 1:
		e1->ex_Stor.st_IntConst = (int32_t)(char)e1->ex_Stor.st_IntConst;
		break;
	    case 2:
		e1->ex_Stor.st_IntConst = (int32_t)(short)e1->ex_Stor.st_IntConst;
		break;
	    case 4:
		break;
	    case 256:
		/* XXX */
		e1->ex_Stor.st_IntConst = 0;
		break;
	    case 256|1:
		e1->ex_Stor.st_IntConst = (int32_t)(unsigned char)e1->ex_Stor.st_IntConst;
		break;
	    case 256|2:
		e1->ex_Stor.st_IntConst = (int32_t)(unsigned short)e1->ex_Stor.st_IntConst;
		break;
	    case 256|4:
		break;
	    case 8:	/* XXX */
	    case 256|8:	/* XXX */
	    default:
		dbprintf(("InsertCast: bad size const-cast: %ld\n",*type->Size));
	    	Assert(0);
		break;
	    }
	    e1->ex_Type = type;
	    return;
	}

	/*
	 *  Floating constant to integer
	 *
	 *  e1->ex_StrConst  e1->ex_StrLen
	 */

	if (e1->ex_Token == TokFltConst) {
	    AllocConstStor(&e1->ex_Stor, FPStrToInt(e1, e1->ex_Stor.st_FltConst, e1->ex_Stor.st_FltLen), type);
	    e1->ex_Token = TokIntConst;
	    e1->ex_Func = GenIntConst;
	    e1->ex_Type = type;
	    return;
	}
    }

    /*
     *	Floating Constant to Floating	(no chg)
     *	Integer Constant to  Floating	??
     */

    if (type->Id == TID_FLT) {
	if (e1->ex_Stor.st_Type == ST_IntConst) {
	    e1->ex_Stor.st_FltConst = IntToFPStr(e1->ex_Stor.st_IntConst, e1->ex_Stor.st_Flags & SF_UNSIGNED, &e1->ex_Stor.st_FltLen);
	    e1->ex_Type = type;
	    e1->ex_Stor.st_Size = *e1->ex_Type->Size;
	    e1->ex_Stor.st_Type = ST_FltConst;
	    e1->ex_Token = TokFltConst;
	    e1->ex_Func  = GenFltConst;
	    return;
	}
	if (e1->ex_Token == TokFltConst) {
	    e1->ex_Type = type;
	    return;
	}
    }

force:
    exp = AllocTmpStructure(Exp);
    exp->ex_Next = e1->ex_Next;
    exp->ex_LexIdx = e1->ex_LexIdx;
    exp->ex_Func = GenCast;
    exp->ex_ExpL = e1;
    exp->ex_Type = type;
    exp->ex_Flags= (e1->ex_Flags & EF_CALL) | EF_ICAST;
    exp->ex_Token = TokCast;

    *pexp = exp;
    (*exp->ex_Func)(pexp);
}

/*
 *  InsertNot() (also used to insert general exp's)
 */

void
InsertNot(pexp)
Exp **pexp;
{
    Exp *exp = AllocStructure(Exp);
    Exp *e1 = *pexp;

    exp->ex_Func = GenNot;
    exp->ex_ExpL = e1;
    exp->ex_LexIdx = e1->ex_LexIdx;
    exp->ex_Flags = e1->ex_Flags & EF_CALL;

    *pexp = exp;
}

/*
 *  insert branch on COND_T (1) or COND_F (-1).  See GenBool() for the
 *  specific generation routine.  In many cases conditionals can be 100%
 *  optimized.
 */

void
InsertBranch(pexp, cond, label)
Exp **pexp;
int32_t cond;
int32_t label;
{
    Exp *exp;
    Exp *e1 = *pexp;

    exp = AllocStructure(Exp);
    exp->ex_Func = GenCondBranch;
    if (cond >= 0)
	exp->ex_LabelT= label;
    else
	exp->ex_LabelF= label;
    exp->ex_Cond = cond;
    exp->ex_ExpL = e1;
    exp->ex_LexIdx = e1->ex_LexIdx;
    exp->ex_Flags= e1->ex_Flags & EF_CALL;
    *pexp = exp;
}

/*
 *  Converts exp to var = exp
 */

void
InsertAssign(pexp, var)
Exp **pexp;
Var *var;
{
    Exp *exp =	AllocStructure(Exp);
    Exp *evar = AllocStructure(Exp);
    Exp *e1 = *pexp;

    Assert(e1);

    evar->ex_Func   = GenVarRef;
    evar->ex_Token  = TokVarRef;
    evar->ex_Symbol = var->Sym;
    evar->ex_Var    = var;
    evar->ex_LexIdx = e1->ex_LexIdx;

    if (e1->ex_Token == TokExpAssBlock)
	exp->ex_Func = GenBracEq;
    else
	exp->ex_Func = GenEq;
    exp->ex_ExpL = evar;
    exp->ex_ExpR = e1;
    exp->ex_LexIdx= (*pexp)->ex_LexIdx;
    exp->ex_Flags = e1->ex_Flags & EF_CALL;

    *pexp = exp;
}

/*
 *  Determine amount of increment/decrement.
 */

int32_t
AutoIncDecSize(exp)
Exp *exp;
{
    Type *type = exp->ex_Type;

    Assert(type);
    if (type->Id == TID_PTR)
	return(*type->SubType->Size);
    if (type->Id == TID_INT)
	return(1);
    yerror(exp->ex_LexIdx, EERROR_NOT_LVALUE);
    return(0);
}

