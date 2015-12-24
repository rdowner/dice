/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  RULES.C
 *
 *  Note that certain operations are available for store-down optimization.
 *  That is, char a, b; a = ~b; ... we can do the invert operation on a byte.
 *  These operators are:    and,or,xor,invert,add,sub
 *
 *  Note: void detection rules should only apply for TID_INT types since
 *  external arrays[] will have size 0
 *
 *  only code gen routines associated with data movement need
 *  understand bit field storage types and only GenCast() &
 *  assignment exp tree routines.
 */

#include "defs.h"

Prototype   void BinaryRules(Exp *);
Prototype   void BinaryArithRules(Exp *);
Prototype   void BinaryLogicRules(Exp *);
Prototype   void UnaryLogicRules(Exp *);
Prototype   void UnaryRules(Exp *);
Prototype   void UnaryArithRules(Exp *);
Prototype   void ShiftRules(Exp *);
Prototype   void AddRules(Exp *);
Prototype   void SubRules(Exp *);
Prototype   void FloatingRules(Exp *);
Prototype   void AssignRules(Exp *);
Prototype   void InitRules(Exp **, Type *);
Prototype   void CompareRules(Exp *, int);
Prototype   void MatchRules(Exp *);
Prototype   void OptBinaryArithRules(Exp *);
Prototype   void OptBinaryRules(Exp *);

Prototype   int  CheckConversion(Exp *, Type *, Type *);

Prototype   int  CreateBinaryResultStorage(Exp *, short);
Prototype   int  CreateUnaryResultStorage(Exp *, short);

Local	    int  CastIfConstantFit(Exp **, Type *);
Prototype   short   AutoResultStorage(Exp *);

/*
 *  Standard Binary Rules
 */

void
BinaryRules(exp)
Exp *exp;
{
    Type *t1;
    Type *t2;
    short doUnsigned;

    if (exp->ex_Flags & EF_ASSEQ)
	return(AssignRules(exp));

    t1 = exp->ex_ExpL->ex_Type;
    t2 = exp->ex_ExpR->ex_Type;
    doUnsigned = 0;

    Assert(t1);
    Assert(t2);

    if (t1->Id != TID_INT || t2->Id != TID_INT) {
	yerror(exp->ex_LexIdx, EERROR_EXPECTED_INT_TYPE);
	exp->ex_Type = &LongType;
	return;
    }
    if ((*t1->Size==0 && t1->Id==TID_INT) || (*t2->Size==0 && t2->Id==TID_INT)) {
	yerror(exp->ex_LexIdx, EERROR_UNEXPECTED_VOID_TYPE);
	exp->ex_Type = &LongType;
	return;
    }


    if (t1->Id == TID_INT && (t1->Flags & TF_UNSIGNED))
	doUnsigned = 1;
    if (t2->Id == TID_INT && (t2->Flags & TF_UNSIGNED))
	doUnsigned = 1;

    if (t1->Id == TID_INT && (*t1->Size < INT_SIZE || (doUnsigned && (t1->Flags & TF_UNSIGNED) == 0))) {
	if (doUnsigned)
	    InsertCast(&exp->ex_ExpL, &ULongType);
	else
	    InsertCast(&exp->ex_ExpL, &LongType);
    }
    if (t2->Id == TID_INT && (*t2->Size < INT_SIZE || (doUnsigned && (t2->Flags & TF_UNSIGNED) == 0))) {
	if (doUnsigned)
	    InsertCast(&exp->ex_ExpR, &ULongType);
	else
	    InsertCast(&exp->ex_ExpR, &LongType);
    }
    if (doUnsigned)
	exp->ex_Type = &ULongType;
    else
	exp->ex_Type = &LongType;
}

void
BinaryArithRules(exp)
Exp *exp;
{
    if (exp->ex_ExpL->ex_Type->Id == TID_FLT || exp->ex_ExpR->ex_Type->Id == TID_FLT) {
	FloatingRules(exp);
	return;
    }
    BinaryRules(exp);
}


/*
 *  Optimized Binary Arithmatic Rules.	This allows the two source
 *  operands to be of smaller types than the destination.  For
 *  example, in genarith.c/GenStar() (multiply), so the MULS/MULU
 *  instruction may be optimized in.
 *
 *  Note that in this case the result type is allowed to be either
 *  a short or a long, but not a char.	This compromise makes code
 *  generation easier.
 */

void
OptBinaryRules(exp)
Exp *exp;
{
    Type *t1;
    Type *t2;
    Type *t;
    short doUnsigned;
    short size;

    if (exp->ex_Flags & EF_ASSEQ)
	return(AssignRules(exp));

    t1 = exp->ex_ExpL->ex_Type;
    t2 = exp->ex_ExpR->ex_Type;
    doUnsigned = 0;

    Assert(t1);
    Assert(t2);

    if (t1->Id != TID_INT || t2->Id != TID_INT) {
	yerror(exp->ex_LexIdx, EERROR_EXPECTED_INT_TYPE);
	exp->ex_Type = &LongType;
	return;
    }
    if (*t1->Size == 0 || *t2->Size == 0) {
	yerror(exp->ex_LexIdx, EERROR_UNEXPECTED_VOID_TYPE);
	exp->ex_Type = &LongType;
	return;
    }

    if (t1->Flags & TF_UNSIGNED)
	doUnsigned = 1;
    if (t2->Flags & TF_UNSIGNED)
	doUnsigned = 1;

    size = 2;
    if (*t1->Size > size)
	size = *t1->Size;
    if (*t2->Size > size)
	size = *t2->Size;

    if (doUnsigned) {
	if (size == 2)
	    t = &UShortType;
	else
	    t = &ULongType;
    } else {
	if (size == 2)
	    t = &ShortType;
	else
	    t = &LongType;
    }
    if (t1 != t)
	InsertCast(&exp->ex_ExpL, t);
    if (t2 != t)
	InsertCast(&exp->ex_ExpR, t);

    if (exp->ex_Type && exp->ex_Type->Id == TID_INT) {
	if (*exp->ex_Type->Size == 1)
	    exp->ex_Type = (doUnsigned) ? &UShortType : &ShortType;
    } else {
	exp->ex_Type = (doUnsigned) ? &ULongType : &LongType;
    }
}

void
OptBinaryArithRules(exp)
Exp *exp;
{
    if (exp->ex_ExpL->ex_Type->Id == TID_FLT || exp->ex_ExpR->ex_Type->Id == TID_FLT) {
	FloatingRules(exp);
	return;
    }
    OptBinaryRules(exp);
}

/*
 *  For logic operations.  If the result type is known we
 *  optimize to the largest type of the three (two args and
 *  result type).
 */

void
BinaryLogicRules(exp)
Exp *exp;
{
    Exp *e1;
    Exp *e2;
    Type *type;

    if ((type = exp->ex_Type) == NULL || type->Id != TID_INT) {
	BinaryRules(exp);
	return;
    }

    /*
     *	Extend or truncate to the return type since all other bits will be
     *	ignored (and for logic operations we can cut it off like this no prob)
     */

    e1 = exp->ex_ExpL;
    e2 = exp->ex_ExpR;

    if (e1->ex_Type->Id == TID_FLT || e2->ex_Type->Id == TID_FLT) {
	yerror(exp->ex_LexIdx, EERROR_EXPECTED_INT_TYPE);
    }

    if (e1->ex_Type != type)
	InsertCast(&exp->ex_ExpL, type);
    if (e2->ex_Type != type)
	InsertCast(&exp->ex_ExpR, type);
}

/*
 *  The result is the type of the argument or the type the parent expects,
 *  whichever is larger.  Example (byte).  If parent expects byte we do not
 *  need to cast to <partype>,	but if it does not we MUST cast to the parent's
 *  type.
 *
 *	[00]80 ^ -1 (byte) -> [FF]7F
 */

void
UnaryLogicRules(exp)
Exp *exp;
{
    Exp *e1;
    Type *type = exp->ex_Type;
    Type *t1;

    if (type == NULL || type->Id != TID_INT) {
	UnaryRules(exp);
	return;
    }
    e1 = exp->ex_ExpL;
    t1 = e1->ex_Type;

    if (*type->Size == 0) {
	yerror(e1->ex_LexIdx, EERROR_UNEXPECTED_VOID_TYPE);
	type = &LongType;
    }
    if (t1->Id == TID_FLT) {
	yerror(e1->ex_LexIdx, EERROR_EXPECTED_INT_TYPE);
	type = &LongType;
    }

    if (type != t1)
	InsertCast(&exp->ex_ExpL, type);
}

void
UnaryRules(exp)
Exp *exp;
{
    Type *t;
    short doUnsigned;

    t = exp->ex_ExpL->ex_Type;
    doUnsigned = 0;

    Assert(t);

    if (t->Id == TID_INT && (t->Flags & TF_UNSIGNED))
	doUnsigned = 1;

    if (t->Id == TID_INT && *t->Size < INT_SIZE) {
	if (*t->Size == 0)
	    yerror(exp->ex_LexIdx, EERROR_UNEXPECTED_VOID_TYPE);
	if (doUnsigned)
	    InsertCast(&exp->ex_ExpL, &ULongType);
	else
	    InsertCast(&exp->ex_ExpL, &LongType);
    }
    if (doUnsigned)
	exp->ex_Type = &ULongType;
    else
	exp->ex_Type = &LongType;
}

void
UnaryArithRules(exp)
Exp *exp;
{
    Exp *e1 = exp->ex_ExpL;

    if (e1->ex_Type->Id == TID_FLT) {
	exp->ex_Type = e1->ex_Type;
	return;
    }
    UnaryRules(exp);
}

void
ShiftRules(exp)
Exp *exp;
{
    Exp *e1 = exp->ex_ExpL;
    Exp *e2 = exp->ex_ExpR;
    Type *t1 = e1->ex_Type;
    Type *t2 = e2->ex_Type;

    Assert(t1);
    Assert(t2);

    if (exp->ex_Flags & EF_ASSEQ) {
	exp->ex_Type = t1;
	return;
    }

    if (*t1->Size != 4) {
    	if (*t1->Size == 0)
	    yerror(e1->ex_LexIdx, EERROR_UNEXPECTED_VOID_TYPE);
	if (t1->Flags & TF_UNSIGNED)
	    InsertCast(&exp->ex_ExpL, &ULongType);
	else
	    InsertCast(&exp->ex_ExpL, &LongType);
    }
    exp->ex_Type = exp->ex_ExpL->ex_Type;
}

/*
 *  AddRules(exp)
 *
 *  Basically handle the special case of (ary/ptr + int) and (int + ary/ptr)
 *  Note that in this case the size of the 'int' is not cast.
 */

void
AddRules(exp)
Exp *exp;
{
    Type *t1 = exp->ex_ExpL->ex_Type;
    Type *t2 = exp->ex_ExpR->ex_Type;

    Assert(t1);
    Assert(t2);
    exp->ex_Token = 0;

    if ((t1->Id == TID_PTR || t1->Id == TID_ARY) && t2->Id == TID_INT) {
    	if (*t1->SubType->Size == 0 && t1->SubType->Id == TID_INT)
	    yerror(exp->ex_LexIdx, EERROR_UNEXPECTED_VOID_TYPE);
        if (*t2->Size == 0)
	    yerror(exp->ex_LexIdx, EERROR_UNEXPECTED_VOID_TYPE);
        exp->ex_Token = 1;
	exp->ex_Type = t1;
	/* xxx if autoassign insert type */
    } else if (t1->Id == TID_INT && (t2->Id == TID_PTR || t2->Id == TID_ARY)) {
	Exp *etmp = exp->ex_ExpL;

    	if (*t1->Size == 0)
	    yerror(exp->ex_LexIdx, EERROR_UNEXPECTED_VOID_TYPE);
        if (*t2->SubType->Size == 0 && t2->SubType->Id == TID_INT)
	    yerror(exp->ex_LexIdx, EERROR_UNEXPECTED_VOID_TYPE);

	exp->ex_ExpL = exp->ex_ExpR;
	exp->ex_ExpR = etmp;
	exp->ex_Type = t2;
	exp->ex_Token = 1;
	if (exp->ex_Flags & EF_ASSEQ)
	    yerror(exp->ex_LexIdx, EERROR_ILLEGAL_PTR_ARITH);
	/* xxx if autoassign insert type */
    } else {
    	if (*t1->Size == 0)
	    yerror(exp->ex_LexIdx, EERROR_UNEXPECTED_VOID_TYPE);
    	if (*t2->Size == 0)
	    yerror(exp->ex_LexIdx, EERROR_UNEXPECTED_VOID_TYPE);

	if (exp->ex_Type && exp->ex_Type->Id == TID_INT && t1->Id == TID_INT && t2->Id == TID_INT) {
	    if (exp->ex_Type != t1)
		InsertCast(&exp->ex_ExpL, exp->ex_Type);
	    if (exp->ex_Type != t2)
		InsertCast(&exp->ex_ExpR, exp->ex_Type);
	} else {
	    BinaryArithRules(exp);
	}
    }
}

/*
 *  SubRules()	Basically handle ptr - ptr and let other routines
 *		worry about other combinations.
 */

void
SubRules(exp)
Exp *exp;
{
    Type *t1 = exp->ex_ExpL->ex_Type;
    Type *t2 = exp->ex_ExpR->ex_Type;

    if (t1->Id == TID_ARY) {
	InsertCast(&exp->ex_ExpL, TypeToPtrType(t1->SubType));
	t1 = exp->ex_ExpL->ex_Type;
    }
    if (t2->Id == TID_ARY) {
	InsertCast(&exp->ex_ExpR, TypeToPtrType(t2->SubType));
	t2 = exp->ex_ExpR->ex_Type;
    }
    if (t1->Id == TID_PTR && t2->Id == TID_PTR) {
	exp->ex_Token = 2;
	exp->ex_Type = &LongType;
    } else {
	if (t2->Id == TID_PTR)
	    yerror(exp->ex_ExpR->ex_LexIdx, EERROR_ILLEGAL_PTR_ARITH);
	AddRules(exp);
    }
}

/*
 *  floating rules (binary).  One or both sides are FP, cast to the largest
 *  of the two, except if EF_ASSEQ cast to lhs.
 */

void
FloatingRules(exp)
Exp *exp;
{
    Type *t1 = exp->ex_ExpL->ex_Type;
    Type *t2 = exp->ex_ExpR->ex_Type;

#ifdef REGISTERED
    if (exp->ex_Flags & EF_ASSEQ) {
	if (t1 != t2)
	    InsertCast(&exp->ex_ExpR, t1);
	exp->ex_Type = t1;
	return;
    }
    if (t1->Id == TID_FLT && t2->Id == TID_FLT) {
	if (*t1->Size > *t2->Size)
	    InsertCast(&exp->ex_ExpR, t1);
	else if (*t2->Size > *t1->Size) {
	    InsertCast(&exp->ex_ExpL, t2);
	    t1 = t2;
	}
    } else if (t1->Id == TID_FLT) {
	InsertCast(&exp->ex_ExpR, t1);
    } else {
	InsertCast(&exp->ex_ExpL, t2);
	t1 = t2;
    }
    exp->ex_Type = t1;
#else
    yerror(exp->ex_LexIdx, EUNREG_FLOATINGPT);
#endif
}

/*
 *  cast the right side to the left side's type
 *
 *  exception:	if left side is a bitfield right side is cast to a long
 *		if right side is a bitfield it is cast to a long
 *
 *		assign takes this into account
 */

void
AssignRules(exp)
Exp *exp;
{
    Assert(exp->ex_ExpL);
    Assert(exp->ex_ExpR);

    InitRules(&exp->ex_ExpR, exp->ex_ExpL->ex_Type);

    /*
     * Old code: returned the right hand type.
     *
     * New code: returns the left hand type if a bitfield, else the righthand
     * type.
     *
     * Properly, this code should always return the LEFT hand type.  It
     * turns out that the right hand and left hand types are the same in all
     * cases except for bitfields.  at least, they are supposed to be... I'm
     * not 100% sure, so for release 13 I am fixing it to return the left 
     * hand type JUST for bitfields, so as not to break something that is
     * known to work.
     */

    if (exp->ex_ExpL->ex_Type->Id == TID_BITFIELD)
	exp->ex_Type = exp->ex_ExpL->ex_Type;
    else
	exp->ex_Type = exp->ex_ExpR->ex_Type;
}

/*
 *  cast the right side to the left side's type
 *
 *  exception:	if left side is a bitfield right side is cast to a long
 *		if right side is a bitfield it is cast to a long
 *
 *		assign takes this into account
 */

void
InitRules(pexp, t1)
Exp **pexp;
Type *t1;
{
    Exp *exp = *pexp;
    Type *t2;
    int errorcode = EERROR_ILLEGAL_ASSIGNMENT;

    t2 = exp->ex_Type;

    Assert(t1);
    Assert(t2);

    if ((*t1->Size == 0) ||
        (*t2->Size == 0 && t2->Id == TID_INT))
    {
	errorcode = EERROR_UNEXPECTED_VOID_TYPE;
    }

    else if (t1->Id == TID_BITFIELD)
    {
    	if (t1->Flags & TF_UNSIGNED)
	    t1 = &ULongType;
	else
	    t1 = &LongType;
	if (t2->Id != TID_INT)
	    yerror(exp->ex_LexIdx, EERROR_ILLEGAL_BITFIELD_OP);
	InsertCast(pexp, t1);

    	/*
    	 * XXX ifdef'd out code entirely ... totally broken XXX
    	 *
    	 * First of all, it is overriding ex_Type for the RHS which
    	 * screws up storage access to the rhs.  Secondly, it cannot 
    	 * deal with a bitfield in the rhs.
    	 */
#ifdef NOTDEF
	if (t1->Flags & TF_UNSIGNED)
	    t1 = &ULongType;
	else
	    t1 = &LongType;
	if (t2->Id != TID_INT)
	    yerror(exp->ex_LexIdx, EERROR_ILLEGAL_BITFIELD_OP);

	/*
	 *  do not insert cast as this is the lhs.  GENASS.C handles assignment
	 *  by not attempting to thrust lhs storage into rhs's return storage.
	 */

	exp->ex_Type = t1;
#endif
	return;
    }

    /* exp->ex_Type = t1; */


    /*
     *	Convert array of x to pointer to x
     */

    if (t2->Id == TID_ARY)
    {
	InsertCast(pexp, t2 = TypeToPtrType(t2->SubType));
    }

    if (t1->Id == t2->Id)
    {
        errorcode = 0;

        /* If they are both pointers, make sure that they point to the same */
        /* thing before issuing a warning.                                  */
        if (t1->Id == TID_PTR)
        {
	   CheckPointerType(exp->ex_LexIdx, exp->ex_LexIdx, t1, t2);
	}
        /* Not pointers, but we should check to see that the objects are the */
        /* same size (since they are the same type).  This check currently   */
        /* lets through structures which are the same size, but since we     */
        /* are not expecting them here we can ignore it for now???           */
	else if (*t1->Size != *t2->Size)
	{
	    if (t1->Id != TID_INT && t1->Id != TID_FLT)
	       errorcode = EERROR_ILLEGAL_ASSIGNMENT;
	}
    }

    /* * * * * * * * * * * * * * * * * * * * * * * * * * * */
    /*                                                     */
    /* Check for conversions from a pointer to an integer  */
    /*                                                     */
    /* * * * * * * * * * * * * * * * * * * * * * * * * * * */
    else if (t1->Id == TID_PTR && t2->Id == TID_INT)
    {
        errorcode = EWARN_INT_PTR_CONVERSION;

        /* If we are converting from a constant integer to a pointer, */
        /* we can let them get away with converting a constant NULL   */
	if (exp->ex_Stor.st_Type == ST_IntConst)
	{
	    /* Supress the error message when they are casting a constant NULL */
	    if (exp->ex_Stor.st_IntConst == 0)
	       errorcode = 0;
	}
	else
	{
	    /* We need to tell them that converting from an integer to a */
	    /* pointer is a bad thing.  We do want to give them a more   */
	    /* meaningful message when they are doing something that is  */
	    /* of a different size.                                      */
	    if (*t1->Size != *t2->Size)
	       errorcode = EERROR_ILLEGAL_PTR_INT_SIZE;
	}
    }

    /* * * * * * * * * * * * * * * * * * * * * * * * * * * */
    /*                                                     */
    /* Check for conversions from an integer to a pointer  */
    /*                                                     */
    /* * * * * * * * * * * * * * * * * * * * * * * * * * * */
    else if (t1->Id == TID_INT && t2->Id == TID_PTR)
    {
	if (*t1->Size != *t2->Size)
	    errorcode = EERROR_ILLEGAL_PTR_INT_SIZE;
    }

    /* * * * * * * * * * * * * * * * * * * * * * * * * * * */
    /*                                                     */
    /* Check for conversions between floats and integers   */
    /*                                                     */
    /* * * * * * * * * * * * * * * * * * * * * * * * * * * */
    else if ((t1->Id == TID_INT && t2->Id == TID_FLT) ||
        (t1->Id == TID_FLT && t2->Id == TID_INT))
    {
        /* This type of conversion is really considered silent */
        errorcode = 0;
    }

    /* * * * * * * * * * * * * * * * * * * * * * * * * * * */
    /*                                                     */
    /* When we get here, we have run out of cases to check */
    /* We must therefore assume that what they are doing   */
    /* is illegal and give them an error message           */
    /*                                                     */
    /* * * * * * * * * * * * * * * * * * * * * * * * * * * */

    if (errorcode)
       yerror(exp->ex_LexIdx, errorcode);
    InsertCast(pexp, t1);
}



/*
 *  CompareRules(exp)
 *
 *	If either side is a bit field it is cast into a long/ulong.
 *
 *	Whichever side is the largest type, the other side is promoted.  For
 *	example, if one side is a short and the other a byte then the
 *	byte is promoted to a short.
 *
 *	If either side is unsigned or a pointer then both sides are promoted
 *	to unsigned.
 *
 *	If one side is an integer constant and the other side is an
 *	integer quantity attempt to fit the integer constant into
 *	the size of the int.
 *
 *	If either side is fp convert both sides to whichever is largest
 */

void
CompareRules(exp, bigok)
Exp *exp;
int bigok;
{
    Type *t1 = exp->ex_ExpL->ex_Type;
    Type *t2 = exp->ex_ExpR->ex_Type;
    short doUnsigned = 0;
    short size;
    Type *type = &VoidType;

    Assert(t1);
    Assert(t2);

    if (exp->ex_Flags & EF_ASSEQ)
	Assert(0);
    exp->ex_Type = &LongType;

    if (t1->Id == TID_STRUCT || t1->Id == TID_UNION || t2->Id == TID_STRUCT || t2->Id == TID_UNION) {
	if (!bigok)
	    yerror(exp->ex_LexIdx, EERROR_ILLEGAL_STRUCT_OP);
	else if (*t1->Size != *t2->Size)
	    yerror(exp->ex_LexIdx, EERROR_ILLEGAL_STRUCT_OP);
	return;
    }

    if (t1->Id == TID_FLT || t2->Id == TID_FLT) {
	FloatingRules(exp);		/*  messes up ex_Type	*/
	exp->ex_Type = &LongType;
	return;
    }

    if (*t1->Size != *t2->Size) {
	if (exp->ex_ExpL->ex_Stor.st_Type == ST_IntConst && t2->Id == TID_INT) {
	    if (CastIfConstantFit(&exp->ex_ExpL, t2))
		return;
	}
	if (exp->ex_ExpR->ex_Stor.st_Type == ST_IntConst && t1->Id == TID_INT) {
	    if (CastIfConstantFit(&exp->ex_ExpR, t1))
		return;
	}
    }

    if (t1->Id == TID_INT && (t1->Flags & TF_UNSIGNED))
	doUnsigned = 1;
    if (t2->Id == TID_INT && (t2->Flags & TF_UNSIGNED))
	doUnsigned = 1;

    size = (*t1->Size > *t2->Size) ? *t1->Size : *t2->Size;

    if (t1->Id == TID_ARY)
	InsertCast(&exp->ex_ExpL, t1 = TypeToPtrType(t1->SubType));
    if (t2->Id == TID_ARY)
	InsertCast(&exp->ex_ExpR, t2 = TypeToPtrType(t2->SubType));

    if (*t1->Size == 0)
	yerror(exp->ex_LexIdx, EERROR_UNEXPECTED_VOID_TYPE);
    if (*t2->Size == 0)
	yerror(exp->ex_LexIdx, EERROR_UNEXPECTED_VOID_TYPE);

    if (t1->Id == TID_PTR && t2->Id == TID_PTR) {
	CheckPointerType(exp->ex_LexIdx, exp->ex_LexIdx, t1, t2);
	return;
    } else if (t1->Id == TID_PTR || t2->Id == TID_PTR) {
	if (t1->Id == TID_INT) {
	    Exp *e1 = exp->ex_ExpL;
	    if (e1->ex_Stor.st_Type != ST_IntConst || e1->ex_Stor.st_IntConst != 0)
		yerror(e1->ex_LexIdx, EWARN_PTR_INT_MISMATCH);
	    type = t2;
	}
	if (t2->Id == TID_INT) {
	    Exp *e2 = exp->ex_ExpR;
	    if (e2->ex_Stor.st_Type != ST_IntConst || e2->ex_Stor.st_IntConst != 0)
		yerror(e2->ex_LexIdx, EWARN_PTR_INT_MISMATCH);
	    type = t1;
	}
	doUnsigned = 1;
    } else {
	switch(size) {
	case 1:
	    type = (doUnsigned) ? &UCharType : &CharType;
	    break;
	case 2:
	    type = (doUnsigned) ? &UShortType : &ShortType;
	    break;
	case 4:
	    type = (doUnsigned) ? &ULongType : &LongType;
	    break;
	case 8:
	    type = (doUnsigned) ? &ULongLongType : &LongLongType;
	    break;
	default:
	    yerror(exp->ex_LexIdx, ESOFT_ILLEGAL_COMPARE);
	    break;
	}
    }
    InsertCast(&exp->ex_ExpL, type);
    InsertCast(&exp->ex_ExpR, type);
}

/*
 *  Compare rules but result is the combined type, not an integer.
 *  ( question-colon operator )
 */

void
MatchRules(exp)
Exp *exp;
{
    Type *t1 = exp->ex_ExpL->ex_Type;
    Type *t2 = exp->ex_ExpR->ex_Type;

    if (*t1->Size == 0 && *t2->Size == 0 && t1->Id != TID_ARY && t2->Id != TID_ARY) {
	exp->ex_Type = &VoidType;
    } else {
	CompareRules(exp, 1);
	exp->ex_Type = exp->ex_ExpL->ex_Type;
    }
}

/*
 *  For CompareRules().  If we are comparing an integer quantity < sizeof(int)
 *  to an integer constant and that constant fits in the quantity size, cast
 *  the constant to the quantity type instead of the type to an int
 */

Local  int
CastIfConstantFit(pexp, type)
Exp **pexp;
Type *type;
{
    Exp *exp = *pexp;	/*  integer constant  */
    short uns = exp->ex_Type->Flags & TF_UNSIGNED;

    if (uns && !(type->Flags & TF_UNSIGNED))	/*  not so simple */
	return(0);

    if (uns) {
	switch(*type->Size) {
	case 1:
	    if (exp->ex_Stor.st_UIntConst < 0x100) {
		InsertCast(pexp, &UCharType);
		return(1);
	    }
	    break;
	case 2:
	    if (exp->ex_Stor.st_UIntConst < 0x10000) {
		InsertCast(pexp, &UShortType);
		return(1);
	    }
	    break;
	case 4:
	    break;
	case 8:	/* XXX */
	default:
	    Assert(0);
	}
    } else {
	switch(*type->Size) {
	case 1:
	    if (exp->ex_Stor.st_IntConst >= -128 && exp->ex_Stor.st_IntConst < 128) {
		InsertCast(pexp, &CharType);
		return(1);
	    }
	    break;
	case 2:
	    if (exp->ex_Stor.st_IntConst >= -32768 && exp->ex_Stor.st_IntConst < 32768) {
		InsertCast(pexp, &ShortType);
		return(1);
	    }
	case 4:
	    break;
	case 8:	/* XXX */
	default:
	    Assert(0);
	}
    }
    return(0);
}


/*
 *  Result Storage creation required?
 *
 *  0	yes
 *  1	no, result storage already exists (even if no return storage)
 *  2	no, there is no return storage
 *
 */

short
AutoResultStorage(exp)
Exp *exp;
{
    uword flags = exp->ex_Flags;

    if (flags & EF_COND) {
	if (flags & EF_CONDACK) /*  was able to handle condition */
	    return(1);
    }
    if (flags & (EF_CRES|EF_PRES|EF_STACKACK))
	return(1);
    if (flags & EF_ASSEQ) {	/*  exp->ex_Stor == e1->ex_Stor */
	/*
	 *  ass= (+=, -=, ...).  If a bit field, we allocate normal
	 *  storage which will be bfsto'd later
	 */

	if (exp->ex_ExpL->ex_Token == TokBFExt) {
	    Assert(exp->ex_ExpL->ex_ExpL->ex_Flags & EF_LHSASSEQ);
	    AllocTmpStorage(&exp->ex_Stor, exp->ex_Type, NULL);
	} else {
	    ReuseStorage(&exp->ex_ExpL->ex_Stor, &exp->ex_Stor);
	}
	if (exp->ex_Flags & EF_RNU)
	    FreeStorage(&exp->ex_Stor); 	/*  no return storage,	*/
	return(1);			    /*	but still have result storage	*/
    }
    if (flags & EF_RNU) {	    /*	no result   */
	exp->ex_Stor.st_Size = 0;   /*	make illegal	*/
	return(2);
    }
    return(0);
}

/*
 *  These functions return true if result storage is available.  Result
 *  storage is not available when the result will not be used, but we
 *  must be careful about EF_ASSEQ since ASSEQ does have result storage,
 *  just no return storage to the higher level.
 *
 *  returns 0	<fillmeXXX>
 *	    1	<fillmeXXX>
 *	    2	temporary allocated
 */

int
CreateBinaryResultStorage(Exp *exp, short freeSub)
{
    int r = 0;

    if (freeSub) {
	if (exp->ex_ExpL->ex_Stor.st_Type != ST_RegIndex)
	    FreeStorage(&exp->ex_ExpL->ex_Stor);
	if (exp->ex_ExpR->ex_Stor.st_Type != ST_RegIndex)
	    FreeStorage(&exp->ex_ExpR->ex_Stor);
    }

    Assert(exp->ex_Type);
    switch (AutoResultStorage(exp)) {
    case 0:
	Assert(exp->ex_ExpR);
	AllocTmpStorage(&exp->ex_Stor, exp->ex_Type, (freeSub) ? &exp->ex_ExpR->ex_Stor : NULL);
	r = 2;
	break;
    case 1:
	r = 1;
	break;
    }
    if (freeSub) {
	if (exp->ex_ExpL->ex_Stor.st_Type == ST_RegIndex)
	    FreeStorage(&exp->ex_ExpL->ex_Stor);
	if (exp->ex_ExpR->ex_Stor.st_Type == ST_RegIndex)
	    FreeStorage(&exp->ex_ExpR->ex_Stor);
    }
    return(r);
}

int
CreateUnaryResultStorage(Exp *exp, short freeSub)
{
    if (freeSub)
	FreeStorage(&exp->ex_ExpL->ex_Stor);

    Assert(exp->ex_Type);
    switch (AutoResultStorage(exp)) {
    case 0:
	if (exp->ex_Type == &VoidType)
	    AllocTmpStorage(&exp->ex_Stor, exp->ex_Type, NULL);
	else
	    AllocTmpStorage(&exp->ex_Stor, exp->ex_Type, &exp->ex_ExpL->ex_Stor);
    case 1:
	return(1);
    }
    return(0);
}

int
CheckConversion(exp, t1, t2)
Exp *exp;
Type *t1;
Type *t2;
{
    switch(t1->Id) {
    case TID_INT:
    case TID_BITFIELD:
    case TID_FLT:
	switch(t2->Id) {
	case TID_INT:
	case TID_BITFIELD:
	case TID_FLT:
	    break;
	case TID_PTR:
	case TID_ARY:
	    yerror(exp->ex_LexIdx, EWARN_INT_PTR_CONVERSION);
	    break;
	case TID_PROC:
	case TID_STRUCT:
	case TID_UNION:
	    yerror(exp->ex_LexIdx, EERROR_ILLEGAL_INT_CONVERSION);
	    return(0);
	}
	break;
    case TID_PTR:
    case TID_ARY:
	switch(t2->Id) {
	case TID_INT:
	case TID_BITFIELD:
	    yerror(exp->ex_LexIdx, EWARN_PTR_INT_CONVERSION);
	    break;
	case TID_FLT:
	    yerror(exp->ex_LexIdx, EERROR_ILLEGAL_PTR_CONVERSION);
	    return(0);
	case TID_PTR:
	case TID_ARY:
	    CheckPointerType(exp->ex_LexIdx, exp->ex_LexIdx, t1, t2);
/*	    if (ProtoOnlyOpt && *t1->SubType->Size != *t2->SubType->Size && *t1->SubType->Size && *t2->SubType->Size)
		yerror(exp->ex_LexIdx, EWARN_PTR_PTR_MISMATCH); */
	    break;
	case TID_PROC:
	case TID_STRUCT:
	case TID_UNION:
	    yerror(exp->ex_LexIdx, EERROR_ILLEGAL_PTR_CONVERSION);
	    return(0);
	}
	break;
    case TID_PROC:
	yerror(exp->ex_LexIdx, EERROR_ILLEGAL_CAST);
	break;
    case TID_STRUCT:
    case TID_UNION:
	if (*t1->Size != *t2->Size) {
	    yerror(exp->ex_LexIdx, EERROR_ILLEGAL_STRUCT_CVT);
	    return(0);
	}
	break;
    }
    return(1);
}
