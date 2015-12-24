/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  GenArith.C
 *
 *  Normal arithmatic functions like +, -, /, % ..
 *
 *  NOTE:  +=, -=, etc.. with lhs bit fields, CreateBinaryResult will
 *	   bfext the lhs.  It is expected that the lhs is already freed.
 */

/*
**      $Filename: genarith.c $
**      $Author: dice $
**      $Revision: 30.157 $
**      $Date: 1995/01/11 13:19:27 $
**      $Log: genarith.c,v $
 * Revision 30.157  1995/01/11  13:19:27  dice
 * .
 *
 * Revision 30.0  1994/06/10  18:04:50  dice
 * .
 *
 * Revision 1.3  1993/10/17  11:02:49  jtoebes
 * FIXED BUG01133 - Strange warning about mismatch of return type for the assignment
 * of a function pointer.  This change now causes the compiler to tell you the type
 * that it was comparing with as well as the expected type.  This should help a lot
 * of people in figuring out what the error was.
 *
 * Revision 1.2  1993/09/11  21:25:12  jtoebes
 * Fixed BUG00108 - if (1, 0) not allowed
 * Added in logic similar to GenNot to test for the condition and generate the
 * branches as appropriate.  It also involves propagating the cond bits.
 *
**/

#include "defs.h"

Prototype void GenDiv(Exp **);
Prototype void GenPercent(Exp **);
Prototype void GenStar(Exp **);
Prototype void GenMi(Exp **);
Prototype void GenPl(Exp **);
Prototype void GenNeg(Exp **);
Prototype void GenParen(Exp **);
Prototype void GenComma(Exp **);

void
GenDiv(pexp)
Exp **pexp;
{
    Exp *exp = *pexp;
    Exp *e1;
    Exp *e2;

    CallLeft();
    EnsureReturnStorageLeft();
    CallRight();

    if (GenPass == 0) {
	OptBinaryArithRules(exp);

	e1 = exp->ex_ExpL;
	e2 = exp->ex_ExpR;

	if (e1->ex_Stor.st_Type == ST_IntConst && e2->ex_Stor.st_Type == ST_IntConst) {
	    if (e2->ex_Stor.st_IntConst == 0) {
		yerror(e2->ex_LexIdx, EERROR_CONST_DIVMOD_0);
		e2->ex_Stor.st_IntConst = 1;
	    }
	    if (e1->ex_Type->Flags & TF_UNSIGNED)
		e1->ex_Stor.st_UIntConst = (ulong)e1->ex_Stor.st_IntConst / (ulong)e2->ex_Stor.st_IntConst;
	    else
		e1->ex_Stor.st_IntConst = e1->ex_Stor.st_IntConst / e2->ex_Stor.st_IntConst;
	    *pexp = e1;
	} else if (e1->ex_Stor.st_Type == ST_FltConst && e2->ex_Stor.st_Type == ST_FltConst) {
	    ConstFpDiv(exp, &e1->ex_Stor, &e2->ex_Stor, &e1->ex_Stor);
	    *pexp = e1;
	} else {
	    /*exp->ex_Flags |= (e1->ex_Flags | e2->ex_Flags) & EF_CALL;*/
	    exp->ex_Flags |= EF_CALL;	/*  XXX ask assembly if call made */
	    GenFlagCallMade();
	}
    } else {
	e1 = exp->ex_ExpL;
	e2 = exp->ex_ExpR;

	if (CreateBinaryResultStorage(exp, 1)) {
	    if (exp->ex_Type->Id == TID_FLT)
		asm_fpdiv(exp, exp->ex_Type, &e1->ex_Stor, &e2->ex_Stor, &exp->ex_Stor);
	    else
		asm_div(exp, &e1->ex_Stor, &e2->ex_Stor, &exp->ex_Stor, 0);
	}
    }
}

void
GenPercent(pexp)
Exp **pexp;
{
    Exp *exp = *pexp;
    Exp *e1;
    Exp *e2;

    CallLeft();
    EnsureReturnStorageLeft();
    CallRight();

    if (GenPass == 0) {
	OptBinaryArithRules(exp);
	/*BinaryArithRules(exp);*/

	e1 = exp->ex_ExpL;
	e2 = exp->ex_ExpR;

	if (e1->ex_Stor.st_Type == ST_IntConst && e2->ex_Stor.st_Type == ST_IntConst) {
	    if (e2->ex_Stor.st_IntConst == 0) {
		yerror(e2->ex_LexIdx, EERROR_CONST_DIVMOD_0);
		e2->ex_Stor.st_IntConst = 1;
	    }
	    if (e1->ex_Type->Flags & TF_UNSIGNED)
		e1->ex_Stor.st_UIntConst = (ulong)e1->ex_Stor.st_IntConst % (ulong)e2->ex_Stor.st_IntConst;
	    else
		e1->ex_Stor.st_IntConst = e1->ex_Stor.st_IntConst % e2->ex_Stor.st_IntConst;
	    *pexp = e1;
	} else {
	    /*exp->ex_Flags |= (e1->ex_Flags | e2->ex_Flags) & EF_CALL;*/
	    exp->ex_Flags |= EF_CALL;	/*  XXX ask assembly if call made */
	    GenFlagCallMade();
	}
    } else {
	e1 = exp->ex_ExpL;
	e2 = exp->ex_ExpR;

	if (CreateBinaryResultStorage(exp, 1))
	    asm_div(exp, &e1->ex_Stor, &e2->ex_Stor, &exp->ex_Stor, 1);
    }
}

void
GenStar(pexp)
Exp **pexp;
{
    Exp *exp = *pexp;
    Exp *e1;
    Exp *e2;

    CallLeft();
    EnsureReturnStorageLeft();
    CallRight();

    if (GenPass == 0) {
	OptBinaryArithRules(exp);


	e1 = exp->ex_ExpL;
	e2 = exp->ex_ExpR;

	if (e1->ex_Stor.st_Type == ST_IntConst && e2->ex_Stor.st_Type == ST_IntConst) {
	    if (e1->ex_Type->Flags & TF_UNSIGNED)
		e1->ex_Stor.st_UIntConst = (ulong)e1->ex_Stor.st_IntConst * (ulong)e2->ex_Stor.st_IntConst;
	    else
		e1->ex_Stor.st_IntConst = e1->ex_Stor.st_IntConst * e2->ex_Stor.st_IntConst;
	    *pexp = e1;
	} else if (e1->ex_Stor.st_Type == ST_FltConst && e2->ex_Stor.st_Type == ST_FltConst) {
	    ConstFpMul(exp, &e1->ex_Stor, &e2->ex_Stor, &e1->ex_Stor);
	    *pexp = e1;
	} else {
	    /*exp->ex_Flags |= (e1->ex_Flags | e2->ex_Flags) & EF_CALL;*/
	    exp->ex_Flags |= EF_CALL;	/*  XXX ask assembly if call made */
	    GenFlagCallMade();
	}
    } else {
	e1 = exp->ex_ExpL;
	e2 = exp->ex_ExpR;

	if (CreateBinaryResultStorage(exp, 1)) {
	    if (exp->ex_Type->Id == TID_FLT)
		asm_fpmul(exp, exp->ex_Type, &e1->ex_Stor, &e2->ex_Stor, &exp->ex_Stor);
	    else
		asm_mul(exp, &e1->ex_Stor, &e2->ex_Stor, &exp->ex_Stor);
	}
    }
}

void
GenMi(pexp)
Exp **pexp;
{
    Exp *exp = *pexp;
    Exp *e1;
    Exp *e2;

    /*
     *	Note: since we do not know whether it is int - int or ptr - int or
     *	ptr - ptr we try to optimize the lhs (which could be a ptr which will
     *	not optimize) and only if it turns out to be an int may we optimize
     *	the rhs.
     */

    if (exp->ex_Type) {
	e1 = exp->ex_ExpL;
	if (e1->ex_Type == NULL)
	    e1->ex_Type = exp->ex_Type;
	CallLeft();
	if (exp->ex_ExpL->ex_Type->Id == TID_INT) {
	    e2 = exp->ex_ExpR;
	    if (e2->ex_Type == NULL)
		e2->ex_Type = exp->ex_Type;
	}
    } else {
	CallLeft();
    }
    EnsureReturnStorageLeft();

    if (GenPass == 0) {
	if (exp->ex_Flags & EF_ASSEQ) {
	    Type *t = exp->ex_ExpL->ex_Type;
	    if (t->Id == TID_INT) {
		if (exp->ex_ExpR->ex_Type == NULL)
		    exp->ex_ExpR->ex_Type = t;
		exp->ex_Type = t;
	    }
	}
	CallRight();

	SubRules(exp);

	e1 = exp->ex_ExpL;
	e2 = exp->ex_ExpR;

	if (e1->ex_Stor.st_Type == ST_IntConst && e2->ex_Stor.st_Type == ST_IntConst) {
	    switch(exp->ex_Token) {
	    case 0:
		e1->ex_Stor.st_IntConst = e1->ex_Stor.st_IntConst - e2->ex_Stor.st_IntConst;
		break;
	    case 1:
		e1->ex_Stor.st_IntConst = e1->ex_Stor.st_IntConst - *exp->ex_Type->SubType->Size * e2->ex_Stor.st_IntConst;
		break;
	    case 2:
		if (*e1->ex_Type->SubType->Size != *e2->ex_Type->SubType->Size)
		{
	            CheckPointerType(e1->ex_LexIdx, e2->ex_LexIdx, e1->ex_Type, e2->ex_Type);
/*		    yerror(e2->ex_LexIdx, EERROR_PTR_PTR_MISMATCH); */
		}
		if (*e1->ex_Type->SubType->Size)
		{
		    e1->ex_Stor.st_IntConst =
		    (e1->ex_Stor.st_IntConst - e2->ex_Stor.st_IntConst) /
		        *e1->ex_Type->SubType->Size;
		}
		else
		{
  		   yerror(e2->ex_LexIdx, EERROR_UNEXPECTED_VOID_TYPE);
		}
		break;
	    }
	    e1->ex_Type = exp->ex_Type;
	    *pexp = e1;
	} else if (e1->ex_Stor.st_Type == ST_FltConst && e2->ex_Stor.st_Type == ST_FltConst) {
	    ConstFpSub(exp, &e1->ex_Stor, &e2->ex_Stor, &e1->ex_Stor);
	    *pexp = e1;
	} else {
	    short makecall = 0;

	    exp->ex_Flags |= (e1->ex_Flags | e2->ex_Flags) & EF_CALL;
	    if (exp->ex_Type->Id == TID_FLT) {
		makecall = 1;
	    } else {
		switch(exp->ex_Token) {
		case 1:     /*	ptr - int   */
		    if (asm_mul_requires_call(-*exp->ex_Type->SubType->Size))
			makecall = 1;
		    break;
		case 2:     /*	ptr - ptr   */
		    if (PowerOfTwo(*e1->ex_Type->SubType->Size) < 0)
			makecall = 1;
		    break;
		}
	    }
	    if (makecall) {
		exp->ex_Flags |= EF_CALL;
		GenFlagCallMade();
	    }
	}
    } else {
	Stor con;

	CallRight();
	e1 = exp->ex_ExpL;
	e2 = exp->ex_ExpR;

	switch(exp->ex_Token) {
	case 0:
	    if (CreateBinaryResultStorage(exp, 1)) {
		if (exp->ex_Type->Id == TID_FLT)
		    asm_fpsub(exp, exp->ex_Type, &e1->ex_Stor, &e2->ex_Stor, &exp->ex_Stor);
		else
		    asm_sub(exp, &e1->ex_Stor, &e2->ex_Stor, &exp->ex_Stor);
	    }
	    break;
	case 1: /*  ptr - int	*/
	    if (exp->ex_Flags & (EF_PRES|EF_ASSEQ)) {
		CreateBinaryResultStorage(exp, 1);
		asm_getindex(exp, e1->ex_Type, &e1->ex_Stor, &e2->ex_Stor, *exp->ex_Type->SubType->Size, &exp->ex_Stor, -1, 1);
	    } else {
		FreeStorage(&e1->ex_Stor);
		FreeStorage(&e2->ex_Stor);
		if ((exp->ex_Flags & EF_RNU) == 0)
		    asm_getindex(exp, e1->ex_Type, &e1->ex_Stor, &e2->ex_Stor, *exp->ex_Type->SubType->Size, &exp->ex_Stor, -1, 0);
	    }
	    break;
	case 2: /*  ptr - ptr	*/
	    if (CreateBinaryResultStorage(exp, 1)) {
		asm_sub(exp, &e1->ex_Stor, &e2->ex_Stor, &exp->ex_Stor);
		if (*e1->ex_Type->SubType->Size > 1) {
		    short n;

		    AllocConstStor(&con, *e1->ex_Type->SubType->Size, &LongType);

		    /*
		     *	If power of 2 can use ASR since pointers are expected to be
		     *	a multiple of the SubType->Size apart.	Thus, 0 never occurs
		     *	for negative quantities (unless already 0).
		     */

		    if ((n = PowerOfTwo(con.st_IntConst)) >= 0) {
			con.st_IntConst = n;
			asm_shift(exp, 1, &exp->ex_Stor, &con, &exp->ex_Stor);
		    } else {
			asm_div(exp, &exp->ex_Stor, &con, &exp->ex_Stor, 0);
		    }
		}
	    }
	    break;
	default:
	    Assert(0);
	    break;
	}
    }
}


void
GenPl(pexp)
Exp **pexp;
{
    Exp *exp = *pexp;
    Exp *e1;
    Exp *e2;

    /*
     *	store-down optimize integer adds cannot be accomplished at the
     *	moment because we do not know about the int + ptr case.
     */

    CallLeft();
    EnsureReturnStorageLeft();

    if (GenPass == 0) {
	if (exp->ex_Flags & EF_ASSEQ) {
	    Type *t = exp->ex_ExpL->ex_Type;
	    if (t->Id == TID_INT) {
		if (exp->ex_ExpR->ex_Type == NULL)
		    exp->ex_ExpR->ex_Type = t;
		exp->ex_Type = t;
	    }
	}
	CallRight();

	AddRules(exp);

	e1 = exp->ex_ExpL;
	e2 = exp->ex_ExpR;

	if (e1->ex_Stor.st_Type == ST_IntConst && e2->ex_Stor.st_Type == ST_IntConst) {
	    if (exp->ex_Token)
		e1->ex_Stor.st_IntConst = e1->ex_Stor.st_IntConst + *exp->ex_Type->SubType->Size * e2->ex_Stor.st_IntConst;
	    else
		e1->ex_Stor.st_IntConst = e1->ex_Stor.st_IntConst + e2->ex_Stor.st_IntConst;
	    e1->ex_Type = exp->ex_Type;
	    *pexp = e1;
	} else if (e1->ex_Stor.st_Type == ST_FltConst && e2->ex_Stor.st_Type == ST_FltConst) {
	    ConstFpAdd(exp, &e1->ex_Stor, &e2->ex_Stor, &e1->ex_Stor);
	    *pexp = e1;
	} else {
	    short makecall = 0;

	    exp->ex_Flags |= (e1->ex_Flags | e2->ex_Flags) & EF_CALL;
	    if (exp->ex_Type->Id == TID_FLT) {
		makecall = 1;
	    } else {
		if (exp->ex_Token && e2->ex_Stor.st_Type != ST_IntConst) {
		    if (asm_mul_requires_call(*exp->ex_Type->SubType->Size))
			makecall = 1;
		}
	    }
	    if (makecall) {
		exp->ex_Flags |= EF_CALL;
		GenFlagCallMade();
	    }
	}
    } else {
	CallRight();
	e1 = exp->ex_ExpL;
	e2 = exp->ex_ExpR;

	if (exp->ex_Token) {
	    if (exp->ex_Flags & (EF_PRES|EF_ASSEQ)) {
		CreateBinaryResultStorage(exp, 1);
		asm_getindex(exp, e1->ex_Type, &e1->ex_Stor, &e2->ex_Stor, *exp->ex_Type->SubType->Size, &exp->ex_Stor, 1, 1);
	    } else {
		FreeStorage(&e1->ex_Stor);
		FreeStorage(&e2->ex_Stor);
		if ((exp->ex_Flags & EF_RNU) == 0)
		    asm_getindex(exp, e1->ex_Type, &e1->ex_Stor, &e2->ex_Stor, *exp->ex_Type->SubType->Size, &exp->ex_Stor, 1, 0);
	    }
	} else {
	    if (CreateBinaryResultStorage(exp, 1)) {
		if (exp->ex_Type->Id == TID_FLT)
		    asm_fpadd(exp, exp->ex_Type, &e1->ex_Stor, &e2->ex_Stor, &exp->ex_Stor);
		else
		    asm_add(exp, &e1->ex_Stor, &e2->ex_Stor, &exp->ex_Stor);
	    } else {
		FreeStorage(&e1->ex_Stor);
		FreeStorage(&e2->ex_Stor);
	    }
	}
    }
}

void
GenNeg(pexp)
Exp **pexp;
{
    Exp *exp = *pexp;
    Exp *e1;

    if (exp->ex_Type && exp->ex_ExpL->ex_Type == NULL)
	exp->ex_ExpL->ex_Type = exp->ex_Type;

    CallLeft();

    if (GenPass == 0) {
	UnaryArithRules(exp);

	e1 = exp->ex_ExpL;

	switch(e1->ex_Stor.st_Type) {
	case ST_IntConst:
	    e1->ex_Stor.st_IntConst = -e1->ex_Stor.st_IntConst;
	    e1->ex_Type = (e1->ex_Type->Flags & TF_UNSIGNED) ? &ULongType : &LongType;
	    *pexp = e1;
	    break;
	case ST_FltConst:
	    ConstFpNeg(exp, &e1->ex_Stor, &e1->ex_Stor);
	    *pexp = e1;
	    break;
	default:
	    exp->ex_Flags |= e1->ex_Flags & EF_CALL;
	    if (exp->ex_Type->Id == TID_FLT) {
		exp->ex_Flags |= EF_CALL;
		GenFlagCallMade();
	    }
	    break;
	}
    } else {
	e1 = exp->ex_ExpL;

	if (CreateUnaryResultStorage(exp, 1)) {
	    if (exp->ex_Type->Id == TID_FLT)
		asm_fpneg(exp, exp->ex_Type, &e1->ex_Stor, &exp->ex_Stor);
	    else
		asm_neg(exp, &e1->ex_Stor, &exp->ex_Stor);
	}
    }
}

void
GenParen(pexp)
Exp **pexp;
{
    Exp *exp = *pexp;
    Exp *e1 = exp->ex_ExpL;

    if (e1 == NULL) {
	if (GenPass == 0) {
	    exp->ex_Type = &VoidType;
	} else {
	    if ((exp->ex_Flags & EF_RNU) == 0)
		yerror(exp->ex_LexIdx, EERROR_UNEXPECTED_VOID_TYPE);
	}
	return;
    }

    if (e1->ex_Type == NULL)
	e1->ex_Type = exp->ex_Type;	/*  optimize storage	*/
    e1->ex_Flags = exp->ex_Flags;
    *pexp = exp->ex_ExpL;
    (*exp->ex_ExpL->ex_Func)(pexp);
}

void
GenComma(pexp)
Exp **pexp;
{
    Exp *exp = *pexp;
    Exp *e1;
    Exp *e2;

    if (GenPass == 0) {
	e1 = exp->ex_ExpL;
	e2 = exp->ex_ExpR;
	e1->ex_Flags |= EF_RNU;
	if (exp->ex_Flags & EF_RNU)
	    e2->ex_Flags |= EF_RNU;
	CallLeft();
	CallRight();

	e1 = exp->ex_ExpL;
	e2 = exp->ex_ExpR;

	exp->ex_Type = e2->ex_Type;

	if (e1->ex_Stor.st_Type == ST_IntConst && e2->ex_Stor.st_Type == ST_IntConst)
	    *pexp = e2;
	else
	    exp->ex_Flags |= (e1->ex_Flags | e2->ex_Flags) & EF_CALL;
    } else {
	CallLeft();

	e2 = exp->ex_ExpR;
	if (exp->ex_Flags & EF_COND) {
	    e2->ex_Flags |= EF_COND;
	    e2->ex_Cond   = exp->ex_Cond;
	    e2->ex_LabelT = exp->ex_LabelT;
	    e2->ex_LabelF = exp->ex_LabelF;
	}

	CallRight();

	if ((exp->ex_Flags & EF_COND) && (e2->ex_Flags & EF_CONDACK)) {
	    exp->ex_Flags |= EF_CONDACK;
	} else {
	    if ((exp->ex_Flags & EF_RNU) == 0) {
		if (exp->ex_Flags & EF_PRES)
		    asm_move(exp, &e2->ex_Stor, &exp->ex_Stor);
		else
		    ReuseStorage(&e2->ex_Stor, &exp->ex_Stor);
	    }
	}

#ifdef NOTDEF
	if (exp->ex_Flags & EF_COND) {
	    if (e2->ex_Flags & EF_CONDACK)
		exp->ex_Flags |= EF_CONDACK;
	} else {
	    if ((exp->ex_Flags & EF_RNU) == 0)
	    {
		if (exp->ex_Flags & EF_PRES)
		    asm_move(exp, &e2->ex_Stor, &exp->ex_Stor);
		else
		    ReuseStorage(&e2->ex_Stor, &exp->ex_Stor);
	    }
	}
#endif
	FreeStorage(&e2->ex_Stor);
    }
}

