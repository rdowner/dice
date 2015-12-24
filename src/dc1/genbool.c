/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  GENBOOL.C	Boolean operators, compare
 *		and logic (!).
 *
 *  You must be careful not to free storage if it will get overwritten
 *  before it is used.
 */

/*
**      $Filename: bool.c $
**      $Author: dice $
**      $Revision: 30.156 $
**      $Date: 1995/01/11 05:04:46 $
**      $Log: genbool.c,v $
 * Revision 30.156  1995/01/11  05:04:46  dice
 * added test for void compare against int constant in optimization
 *
 * Revision 30.5  1994/06/13  18:37:29  dice
 * .
 *
 * Revision 30.0  1994/06/10  18:04:51  dice
 * .
 *
 * Revision 1.7  1993/11/15  21:01:43  jtoebes
 * Fixed BUG01143 - Problem with ?: constructs generating an error for void types.
 *
 * Revision 1.6  1993/09/11  22:36:22  jtoebes
 * Fixed BUG06009.
 * Minor cleanup of some duplicated code.
 *
 * Revision 1.5  1993/09/05  23:40:19  jtoebes
 * Fixed BUG06037 - Failure to warn on if test for structure when
 * involved with the ! operation.
 *
**/

#include "defs.h"

Prototype void BoolLabels(Exp *, int32_t, int32_t);
Prototype void GenCondBranch(Exp **);
Prototype void GenAndAnd(Exp **);
Prototype void GenOrOr(Exp **);
Prototype void GenBoolCompareSame(Exp **);
Prototype void GenBoolCompare(Exp **);
Prototype void GenNot(Exp **);
Prototype void GenColon(Exp **);
Prototype void GenQuestion(Exp **);

Local void TerminateCondition(Exp *, Exp *);

/*
 *  Basically, if the parent exp node requests a condition branch of the
 *  child it may ask the child to branch on either a false or a true
 *  condition but not both.
 */

void
BoolLabels(Exp *exp, int32_t subcond1, int32_t subcond2)
{
    if (exp->ex_Flags & EF_COND) {
	exp->ex_Flags |= EF_CONDACK;
	if (exp->ex_Cond < 0)
	    exp->ex_LabelT = AllocLabel();
	else
	    exp->ex_LabelF = AllocLabel();
    } else {
	exp->ex_LabelT = AllocLabel();
	exp->ex_LabelF = AllocLabel();
	exp->ex_Cond = 0;
    }
    if (subcond1) {
	Exp *e = exp->ex_ExpL;

	e->ex_Flags |= EF_COND;
	e->ex_Cond = subcond1;
	if (subcond1 > 0)
	    e->ex_LabelT = exp->ex_LabelT;
	else
	    e->ex_LabelF = exp->ex_LabelF;
    }
    if (subcond2) {
	Exp *e = exp->ex_ExpR;

	e->ex_Flags |= EF_COND;
	e->ex_Cond = subcond2;
	if (subcond2 > 0)
	    e->ex_LabelT = exp->ex_LabelT;
	else
	    e->ex_LabelF = exp->ex_LabelF;
    }
}

/*
 *  the GenCondBranch() is usually inserted by statement label nodes such
 *  as 'if'.  These nodes set ex_Cond to 0/1 or -1 depending on whether
 *  they want to branch on true or false, and set ex_LabelT/ex_LabelF
 *  appropriately.
 */

void
GenCondBranch(pexp)
Exp **pexp;
{
    Exp *exp = *pexp;
    Exp *e1;
    Type *t1;

    if (GenPass == 0) {
	e1 = exp->ex_ExpL;
	e1->ex_Flags |= EF_COND;    /*	allow pass 1 opt of left */
	CallLeft();
	e1 = exp->ex_ExpL;

        t1 = e1->ex_Type;
        /* If they didn't optimize the expression for us, we need to check   */
        /* the type to ensure that it is acceptable for a conditional branch */
        if (!(e1->ex_Flags & (EF_CONDACK|EF_RNU)))
        {
            if (t1 == NULL || t1->Id > TID_ARY) {
		if (t1->Id != TID_BITFIELD)
		    yerror(e1->ex_LexIdx, EERROR_EXPECTED_INT_TYPE);
            }
            else if (*t1->Size == 0) {
		yerror(e1->ex_LexIdx, EERROR_UNEXPECTED_VOID_TYPE);
            }
        }
	exp->ex_Type = &VoidType;
	exp->ex_Flags |= e1->ex_Flags & EF_CALL;
	if (t1 && t1->Id == TID_FLT)
	{
	    exp->ex_Flags |= EF_CALL;
	    GenFlagCallMade();
	}
    } else {
	e1 = exp->ex_ExpL;
	e1->ex_Flags |= EF_COND;
	if (exp->ex_Cond >= 0) {    /*  default case 0 == true  */
	    e1->ex_Cond = COND_T;
	    e1->ex_LabelT = exp->ex_LabelT;
	} else {
	    e1->ex_Cond = COND_F;
	    e1->ex_LabelF = exp->ex_LabelF;
	}

	CallLeft();

	exp->ex_Flags |= EF_CONDACK;
	if (e1->ex_Flags & EF_CONDACK) {    /*  they branched for us    */
	    ;
	} else {			    /*	they didn't             */
	    if (e1->ex_Type->Id == TID_FLT)
		asm_fptest(exp, &e1->ex_Stor);
	    else
		asm_test(exp, &e1->ex_Stor);
	    FreeStorage(&e1->ex_Stor);
	    if (exp->ex_Cond >= 0)
		asm_condbra(COND_NEQ, exp->ex_LabelT);
	    else
		asm_condbra(COND_EQ, exp->ex_LabelF);
	}
    }
}

/*
 *  If exp1 && exp2
 */

void
GenAndAnd(pexp)
Exp **pexp;
{
    Exp *exp = *pexp;
    Exp *e1;
    Exp *e2;

    if (GenPass == 0) {
	CallLeft();
	CallRight();
	e1 = exp->ex_ExpL;
	e2 = exp->ex_ExpR;

	if (e1->ex_Stor.st_Type == ST_IntConst) {
	    if (e1->ex_Stor.st_IntConst == 0) {
		*pexp = e1;
		e1->ex_Flags |= exp->ex_Flags & EF_RNU;
		return;
	    }
	    if (e2->ex_Stor.st_Type == ST_IntConst) {
		e1->ex_Stor.st_IntConst = (e1->ex_Stor.st_IntConst && e2->ex_Stor.st_IntConst);
		*pexp = e1;
		e1->ex_Flags |= exp->ex_Flags & EF_RNU;
		return;
	    }
	}
	exp->ex_Type = &LongType;
	exp->ex_Flags |= (e1->ex_Flags | e2->ex_Flags) & EF_CALL;

	if ((e1->ex_Type && e1->ex_Type->Id == TID_FLT) || (e2->ex_Type && e2->ex_Type->Id == TID_FLT)) {
	    exp->ex_Flags |= EF_CALL;
	    GenFlagCallMade();
	}
    } else {
	if (exp->ex_Cond >= 0)
	    BoolLabels(exp, COND_F, COND_T);
	else
	    BoolLabels(exp, COND_F, COND_F);

	CallLeft();
	EnsureReturnStorageLeft();

	e1 = exp->ex_ExpL;

	if ((e1->ex_Flags & EF_CONDACK) == 0) {
	    if (e1->ex_Type->Id == TID_FLT)
		asm_fptest(exp, &e1->ex_Stor);
	    else
		asm_test(exp, &e1->ex_Stor);
	    FreeStorage(&e1->ex_Stor);
	    asm_condbra(COND_EQ, exp->ex_LabelF);
	}

	CallRight();
	e2 = exp->ex_ExpR;

	if ((e2->ex_Flags & EF_CONDACK) == 0) {
	    if (e2->ex_Type->Id == TID_FLT)
		asm_fptest(exp, &e2->ex_Stor);
	    else
		asm_test(exp, &e2->ex_Stor);
	    FreeStorage(&e2->ex_Stor);
	    if (exp->ex_Cond >= 0)
		asm_condbra(COND_NEQ, exp->ex_LabelT);   /*  true, fall through false    */
	    else
		asm_condbra(COND_EQ, exp->ex_LabelF);   /*  false, fall through true    */
	}

	if (exp->ex_Flags & EF_COND) {      /*  parent wants us to cond.bra */
	    if (exp->ex_Cond > 0) {         /*  on true, we own the false label */
		asm_label(exp->ex_LabelF);
	    } else {			    /*	on false, we own the true label */
		asm_label(exp->ex_LabelT);
	    }
	} else {			    /*	we own both labels  (fall through false)    */
	    if (exp->ex_Flags & EF_RNU) {
		asm_label(exp->ex_LabelF);
		asm_label(exp->ex_LabelT);
	    } else {
		int32_t skip = AllocLabel();

		CreateBinaryResultStorage(exp, 0);

		asm_label(exp->ex_LabelF);
		asm_movei(exp, 0, &exp->ex_Stor);
		asm_branch(skip);
		asm_label(exp->ex_LabelT);
		asm_movei(exp, 1, &exp->ex_Stor);
		asm_label(skip);
	    }
	}
    }
}

/*
 *  ||
 */

void
GenOrOr(pexp)
Exp **pexp;
{
    Exp *exp = *pexp;
    Exp *e1;
    Exp *e2;

    if (GenPass == 0) {
	CallLeft();
	CallRight();
	e1 = exp->ex_ExpL;
	e2 = exp->ex_ExpR;

	if (e1->ex_Stor.st_Type == ST_IntConst) {
	    if (e1->ex_Stor.st_IntConst != 0) {
		*pexp = e1;
		e1->ex_Flags |= exp->ex_Flags & EF_RNU;
		return;
	    }
	    if (e2->ex_Stor.st_Type == ST_IntConst) {
		e1->ex_Stor.st_IntConst = (e1->ex_Stor.st_IntConst || e2->ex_Stor.st_IntConst);
		*pexp = e1;
		e1->ex_Flags |= exp->ex_Flags & EF_RNU;
		return;
	    }
	}
	exp->ex_Type = &LongType;
	exp->ex_Flags |= (e1->ex_Flags | e2->ex_Flags) & EF_CALL;
	if ((e1->ex_Type && e1->ex_Type->Id == TID_FLT) || (e2->ex_Type && e2->ex_Type->Id == TID_FLT)) {
	    exp->ex_Flags |= EF_CALL;
	    GenFlagCallMade();
	}
    } else {
	if (exp->ex_Cond >= 0)
	    BoolLabels(exp, COND_T, COND_T);
	else
	    BoolLabels(exp, COND_T, COND_F);

	CallLeft();
	EnsureReturnStorageLeft();
	e1 = exp->ex_ExpL;

	if ((e1->ex_Flags & EF_CONDACK) == 0) {
	    if (e1->ex_Type->Id == TID_FLT)
		asm_fptest(exp, &e1->ex_Stor);
	    else
		asm_test(exp, &e1->ex_Stor);
	    FreeStorage(&e1->ex_Stor);
	    asm_condbra(COND_NEQ, exp->ex_LabelT);
	}

	CallRight();
	e2 = exp->ex_ExpR;

	if ((e2->ex_Flags & EF_CONDACK) == 0) {
	    if (e2->ex_Type->Id == TID_FLT)
		asm_fptest(exp, &e2->ex_Stor);
	    else
		asm_test(exp, &e2->ex_Stor);
	    FreeStorage(&e2->ex_Stor);
	    if (exp->ex_Cond >= 0)
		asm_condbra(COND_NEQ, exp->ex_LabelT);   /*  true, fall through false    */
	    else
		asm_condbra(COND_EQ, exp->ex_LabelF);   /*  false, fall through true    */
	}

	if (exp->ex_Flags & EF_COND) {      /*  parent wants us to cond.bra */
	    if (exp->ex_Cond > 0) {         /*  on true, we own the false label */
		asm_label(exp->ex_LabelF);
	    } else {			    /*	on false, we own the true label */
		asm_label(exp->ex_LabelT);
	    }
	} else {			    /*	we own both labels  (fall through false)    */
	    if (exp->ex_Flags & EF_RNU) {
		asm_label(exp->ex_LabelF);
		asm_label(exp->ex_LabelT);
	    } else {
		int32_t skip = AllocLabel();

		CreateBinaryResultStorage(exp, 0);

		asm_label(exp->ex_LabelF);
		asm_movei(exp, 0, &exp->ex_Stor);
		asm_branch(skip);
		asm_label(exp->ex_LabelT);
		asm_movei(exp, 1, &exp->ex_Stor);
		asm_label(skip);
	    }
	}
    }
}

/*
 *  Compare exp == exp
 */

void
GenBoolCompareSame(pexp)
Exp **pexp;
{
    Exp *exp = *pexp;
    Exp *e1;
    Exp *e2;

    if (GenPass == 0) {
	CallLeft();
	CallRight();

	e1 = exp->ex_ExpL;
	e2 = exp->ex_ExpR;

	exp->ex_Flags |= (e1->ex_Flags | e2->ex_Flags) & EF_CALL;

	/*
	 *  handle constant case
	 */

	if (e1->ex_Stor.st_Type == ST_IntConst && e2->ex_Stor.st_Type == ST_IntConst) {
	    if (exp->ex_Token == TokEqEq)
		e1->ex_Stor.st_IntConst = (e1->ex_Stor.st_IntConst == e2->ex_Stor.st_IntConst);
	    else
		e1->ex_Stor.st_IntConst = (e1->ex_Stor.st_IntConst != e2->ex_Stor.st_IntConst);
	    e1->ex_Type = &LongType;
	    *pexp = e1;
	    return;
	}

	/*
	 *  optimize conditional.  If either side is 0 then remove
	 *  our rule if != 0, change our rule to '!exp' if == 0.  Only
	 *  works if EF_COND set in pass 1.
	 */

	if (exp->ex_Flags & EF_COND) {
	    if (e1->ex_Stor.st_Type == ST_IntConst && e1->ex_Stor.st_IntConst == 0) {
#ifdef NOTDEF
	    	if (*e2->ex_Type->Size == 0)
		    yerror(e2->ex_LexIdx, EERROR_UNEXPECTED_VOID_TYPE);
#endif

                if (exp->ex_Token == TokEqEq) { /*  change to '!exp'    */
		    exp->ex_Func = GenNot;
		    exp->ex_ExpL = e2;
 	            exp->ex_Type = &LongType;
		} else {			/*  change to just 'exp'*/
		    *pexp = e2;
		    e2->ex_Flags |= exp->ex_Flags & EF_RNU;
		}
		return;
	    } else if (e2->ex_Stor.st_Type == ST_IntConst && e2->ex_Stor.st_IntConst == 0) {
#ifdef NOTDEF
	    	if (*e1->ex_Type->Size == 0)
		    yerror(e1->ex_LexIdx, EERROR_UNEXPECTED_VOID_TYPE);
#endif
		if (exp->ex_Token == TokEqEq) { /*  change to '!exp'    */
		    exp->ex_Func = GenNot;
 	            exp->ex_Type = &LongType;
		} else {			/*  change to just 'exp'*/
		    *pexp = e1;
		    e1->ex_Flags |= exp->ex_Flags & EF_RNU;
		}
		return;
	    }
	}
	CompareRules(exp, 1);
	e1 = exp->ex_ExpL;
	e1 = exp->ex_ExpR;
	exp->ex_Flags |= (e1->ex_Flags | e2->ex_Flags) & EF_CALL;
	if (e1->ex_Type->Id == TID_FLT) {   /*  either e1 or e2     */
	    exp->ex_Flags |= EF_CALL;
	    GenFlagCallMade();
	}
    } else {
	short cond = (exp->ex_Token == TokEqEq) ? COND_EQ : COND_NEQ;

	BoolLabels(exp, 0, 0);

	CallLeft();
	EnsureReturnStorageLeft();
	CallRight();
	e1 = exp->ex_ExpL;
	e2 = exp->ex_ExpR;

	if (exp->ex_Flags & EF_COND) {      /*  parent wants us to cond.bra */
	    if (e1->ex_Type->Id == TID_FLT)
		asm_fpcmp(exp, &e1->ex_Stor, &e2->ex_Stor, &cond);
	    else
		asm_cmp(exp, &e1->ex_Stor, &e2->ex_Stor, &cond);

	    FreeStorage(&e1->ex_Stor);
	    FreeStorage(&e2->ex_Stor);

	    if (exp->ex_Cond > 0) {         /*  on true, we own the false label */
		asm_condbra(cond, exp->ex_LabelT);
		asm_label(exp->ex_LabelF);
	    } else {			    /*	on false, we own the true label */
		asm_condbra(-cond, exp->ex_LabelF);
		asm_label(exp->ex_LabelT);
	    }
	} else {			    /*	we own both labels  */
	    if (exp->ex_Flags & EF_RNU) {
		FreeStorage(&e1->ex_Stor);
		FreeStorage(&e2->ex_Stor);
		yerror(exp->ex_LexIdx, EWARN_RESULT_NOT_USED);
	    } else {
		CreateBinaryResultStorage(exp, 1);

		asm_cond_scc(exp, e1->ex_Type->Id, &e1->ex_Stor, &e2->ex_Stor, &cond, &exp->ex_Stor);
	    }
	}
    }
}

void
GenBoolCompare(pexp)
Exp **pexp;
{
    Exp *exp = *pexp;
    Exp *e1;
    Exp *e2;
    short cond;

    if (GenPass == 0) {
	CallLeft();
	CallRight();

	CompareRules(exp, 0);

	e1 = exp->ex_ExpL;
	e2 = exp->ex_ExpR;

	exp->ex_Flags |= (e1->ex_Flags | e2->ex_Flags) & EF_CALL;
	if (e1->ex_Type->Id == TID_FLT) {   /*  either e1 or e2 */
	    exp->ex_Flags |= EF_CALL;
	    GenFlagCallMade();
	}


	if (e1->ex_Stor.st_Type == ST_IntConst && e2->ex_Stor.st_Type == ST_IntConst) {
	    int result = 0;
	    int32_t v1 = e1->ex_Stor.st_IntConst;
	    int32_t v2 = e2->ex_Stor.st_IntConst;

	    if (e1->ex_Type->Flags & TF_UNSIGNED) {
		switch(exp->ex_Token) {
		case TokLt:
		    result = (uint32_t)v1 < (uint32_t)v2;
		    break;
		case TokLtEq:
		    result = (uint32_t)v1 <= (uint32_t)v2;
		    break;
		case TokGt:
		    result = (uint32_t)v1 > (uint32_t)v2;
		    break;
		case TokGtEq:
		    result = (uint32_t)v1 >= (uint32_t)v2;
		    break;
		default:
		    Assert(0);
		}
	    } else {
		switch(exp->ex_Token) {
		case TokLt:
		    result = v1 < v2;
		    break;
		case TokLtEq:
		    result = v1 <= v2;
		    break;
		case TokGt:
		    result = v1 > v2;
		    break;
		case TokGtEq:
		    result = v1 >= v2;
		    break;
		default:
		    Assert(0);
		}
	    }
	    e1->ex_Type = &LongType;
	    e1->ex_Stor.st_IntConst = result;
	    *pexp = e1;
	    return;
	}
	exp->ex_Type = &LongType;
    } else {
	BoolLabels(exp, 0, 0);

	CallLeft();
	EnsureReturnStorageLeft();
	CallRight();

	e1 = exp->ex_ExpL;
	e2 = exp->ex_ExpR;

	switch(exp->ex_Token) {
	    case TokLt:		cond = COND_LT;		break;
	    case TokLtEq:	cond = COND_LTEQ;	break;
	    case TokGt:		cond = COND_GT;		break;
	    case TokGtEq:	cond = COND_GTEQ;	break;
	    default:		Assert(0);
	}
	if (e1->ex_Type->Flags & TF_UNSIGNED) {
	    cond |= CF_UNS;
	}

	if (exp->ex_Flags & EF_COND) {      /*  parent wants us to cond.bra */
	    if (e1->ex_Type->Id == TID_FLT)
		asm_fpcmp(exp, &e1->ex_Stor, &e2->ex_Stor, &cond);     /*  if cmp reverses args it negates cond */
	    else
		asm_cmp(exp, &e1->ex_Stor, &e2->ex_Stor, &cond);     /*  if cmp reverses args it negates cond */
	    FreeStorage(&e1->ex_Stor);
	    FreeStorage(&e2->ex_Stor);
	    if (exp->ex_Cond > 0) {         /*  on true, we own the false label */
		asm_condbra(cond, exp->ex_LabelT);
		asm_label(exp->ex_LabelF);
	    } else {			    /*	on false, we own the true label */
		asm_condbra(-cond, exp->ex_LabelF);
		asm_label(exp->ex_LabelT);
	    }
	} else {			    /*	we own both labels  */
	    if (exp->ex_Flags & EF_RNU) {
		FreeStorage(&e1->ex_Stor);
		FreeStorage(&e2->ex_Stor);
		yerror(exp->ex_LexIdx, EWARN_RESULT_NOT_USED);
	    } else {
		CreateBinaryResultStorage(exp, 1);
		asm_cond_scc(exp, e1->ex_Type->Id, &e1->ex_Stor, &e2->ex_Stor, &cond, &exp->ex_Stor);
	    }
	}
	return;
    }
}

void
GenNot(pexp)
Exp **pexp;
{
    Exp *exp = *pexp;
    Exp *e1;

    if (GenPass == 0) {
	CallLeft();
	e1 = exp->ex_ExpL;

	if (e1->ex_Stor.st_Type == ST_IntConst) {
	    e1->ex_Stor.st_IntConst = !e1->ex_Stor.st_IntConst;
	    e1->ex_Type = &LongType;
	    *pexp = e1;
	}


        /* If they didn't optimize the expression for us, we need to check   */
        /* the type to ensure that it is acceptable for a conditional branch */
        if (!(e1->ex_Flags & (EF_CONDACK|EF_RNU)))
        {
	    Type *t1;
	    t1 = e1->ex_Type;

            if (t1 == NULL || t1->Id > TID_ARY) {
		if (t1->Id != TID_BITFIELD)
		    yerror(e1->ex_LexIdx, EERROR_EXPECTED_INT_TYPE);
            }
            else if (*t1->Size == 0) {
		yerror(e1->ex_LexIdx, EERROR_UNEXPECTED_VOID_TYPE);
	    }
	}


	exp->ex_Type = &LongType;
	exp->ex_Flags |= e1->ex_Flags & EF_CALL;
	if (e1->ex_Type && e1->ex_Type->Id == TID_FLT) {
	    exp->ex_Flags |= EF_CALL;
	    GenFlagCallMade();
	}
    } else {
	e1 = exp->ex_ExpL;
	if (exp->ex_Flags & EF_COND) {
	    exp->ex_Flags |= EF_CONDACK;

	    /*
	     *	if parent wants true condition we request false condition
	     *	if parent wants false condition we request true condition
	     */

	    if (exp->ex_Cond > 0) {
		e1->ex_Flags |= EF_COND;
		e1->ex_Cond = -1;
		e1->ex_LabelF = exp->ex_LabelT;
	    } else {
		e1->ex_Flags |= EF_COND;
		e1->ex_Cond = 1;
		e1->ex_LabelT = exp->ex_LabelF;
	    }
	} else {
	    ;	/*  should we ask the lower routine to branch for us? */
	}

	CallLeft();
	e1 = exp->ex_ExpL;

	if (exp->ex_Flags & EF_COND) {
	    if (e1->ex_Flags & EF_CONDACK) {
		;				/*  nothing to do */
	    } else {
		if (e1->ex_Type->Id == TID_FLT)
		    asm_fptest(exp, &e1->ex_Stor);
		else
		    asm_test(exp, &e1->ex_Stor);
		FreeStorage(&e1->ex_Stor);
		if (exp->ex_Cond >= 0)
		    asm_condbra(COND_EQ, exp->ex_LabelT);
		else
		    asm_condbra(COND_NEQ, exp->ex_LabelF);
	    }
	} else {
	    if (exp->ex_Flags & EF_RNU) {
		FreeStorage(&e1->ex_Stor);
		yerror(exp->ex_LexIdx, EWARN_RESULT_NOT_USED);
	    } else {
		CreateUnaryResultStorage(exp, 1);

		asm_test_scc(exp, e1->ex_Type->Id, &e1->ex_Stor, COND_EQ, &exp->ex_Stor);
	    }
	}
	return;
    }
}

/*
 *		:
 *	       / \
 *	      ?   c
 *	     a b   e3
 *	    e1 e2
 *  We are talking massive constant & branch optimization here
 */

void
GenColon(pexp)
Exp **pexp;
{
    Exp *exp = *pexp;
    Exp *e1;
    Exp *e2;
    Exp *e3;
    int32_t lmiddle;
    int32_t lend;

    Assert(exp->ex_ExpL->ex_Token == TokQuestion);
    e1 = exp->ex_ExpL->ex_ExpL;
    e2 = exp->ex_ExpL->ex_ExpR;
    e3 = exp->ex_ExpR;

    if (GenPass == 0) {
	(*e1->ex_Func)(&e1);
	(*e2->ex_Func)(&e2);
	(*e3->ex_Func)(&e3);

	exp->ex_Flags |= (e1->ex_Flags | e2->ex_Flags | e3->ex_Flags) & EF_CALL;
	if (e1->ex_Type->Id == TID_FLT || e2->ex_Type->Id == TID_FLT || e3->ex_Type->Id == TID_FLT) {
	    exp->ex_Flags |= EF_CALL;
	    GenFlagCallMade();
	}

	exp->ex_ExpL->ex_ExpL = e1;
	exp->ex_ExpL->ex_ExpR = e2;
	exp->ex_ExpR = e3;

	if (e1->ex_Stor.st_Type == ST_IntConst) {
	    if (e1->ex_Stor.st_IntConst) {
		*pexp = e2;
		e2->ex_Flags |= exp->ex_Flags & EF_RNU;
	    } else {
		*pexp = e3;
		e3->ex_Flags |= exp->ex_Flags & EF_RNU;
	    }
	    return;
	}
	InsertBranch(&e1, COND_F, 0);
	/*
	 *  Temporarily supercede exp.	el, e3
	 */
	if ((exp->ex_Flags & EF_RNU) == 0)
	{
	    Exp *el = exp->ex_ExpL;

	    exp->ex_ExpL = e2;
	    MatchRules(exp);      /*  e2,e3 no longer valid */
	    el->ex_ExpR = exp->ex_ExpL;
	    exp->ex_ExpL = el;
	}
	exp->ex_ExpL->ex_ExpL = e1;
    } else {
	short cbr;

	lmiddle = AllocLabel();
	lend	= AllocLabel();

	if (exp->ex_Cond >= 0)      /*  branch on true      */
	    exp->ex_LabelF = lend;  /*	we own false label  */
	else			    /*	branch on false...  */
	    exp->ex_LabelT = lend;  /*	we own true label   */

	e1->ex_LabelF = lmiddle;

	/*
	 *  an EF_COND on exp really means an EF_COND for subexps e2 and e3,
	 *  NOT e1.  If one of e2 or e3 cannot handle the condition we must
	 *  put in a test-branch ourselves.
	 */

	if (exp->ex_Flags & EF_COND) {
	    exp->ex_Flags |= EF_CONDACK;
	    e2->ex_Flags |= EF_COND;
	    e2->ex_Cond = exp->ex_Cond;
	    e2->ex_LabelT = exp->ex_LabelT;
	    e2->ex_LabelF = exp->ex_LabelF;

	    e3->ex_Flags |= EF_COND;
	    e3->ex_Cond = exp->ex_Cond;
	    e3->ex_LabelT = exp->ex_LabelT;
	    e3->ex_LabelF = exp->ex_LabelF;
	} else
	if (exp->ex_Flags & EF_RNU) {
	    e2->ex_Flags |= EF_RNU;
	    e3->ex_Flags |= EF_RNU;
	}

	/*
	 *  Use a trick to allocate the result storage.  Since the storage
	 *  is not used until the end we allocate and free it here (otherwise
	 *  the temporary register is unusable inside the possibly complex
	 *  expression)
	 */

        if (exp->ex_Type == NULL)
            exp->ex_Type = &VoidType;

	cbr = CreateBinaryResultStorage(exp, 0);
	if (cbr == 2)
	    FreeStorage(&exp->ex_Stor);

	/*
	 *  a not a constant.  a branches to either e2 or e3
	 */

	(*e1->ex_Func)(&e1);   /*  branches to e2 or e3    */

	(*e2->ex_Func)(&e2);
	if (cbr == 2)
	    ReuseStorage(&exp->ex_Stor, &exp->ex_Stor);
	TerminateCondition(e2, exp);
	asm_branch(lend);

	asm_label(lmiddle);

	if (cbr == 2)
	    FreeStorage(&exp ->ex_Stor);
	(*e3->ex_Func)(&e3);
	if (cbr == 2)
	    ReuseStorage(&exp->ex_Stor, &exp->ex_Stor);

	TerminateCondition(e3, exp);

	asm_label(lend);
    }
}

Local void
TerminateCondition(sub, exp)
Exp *sub;
Exp *exp;
{
    switch(exp->ex_Flags & (EF_RNU|EF_CONDACK)) {
    case 0:			/*  normal return value */
	FreeStorage(&sub->ex_Stor);
	asm_move_cast(exp, &sub->ex_Stor, &exp->ex_Stor);
	break;
    case EF_RNU:	     /*  no return value or condition	 */
	if ((sub->ex_Flags & EF_RNU) == 0) {
	    FreeStorage(&sub->ex_Stor);
	}
	break;
    case EF_CONDACK:		/*  conditional branch	    */
	if ((sub->ex_Flags & EF_CONDACK) == 0) {
	    if (sub->ex_Type->Id == TID_INT && sub->ex_Stor.st_Type == ST_IntConst) {
		if (exp->ex_Cond >= 0 && sub->ex_Stor.st_IntConst)
		    asm_branch(exp->ex_LabelT);
		else if (exp->ex_Cond < 0 && sub->ex_Stor.st_IntConst == 0)
		    asm_branch(exp->ex_LabelF);
	    } else {
		if (sub->ex_Type->Id == TID_FLT)
		    asm_fptest(exp, &sub->ex_Stor);
		else
		    asm_test(exp, &sub->ex_Stor);
		FreeStorage(&sub->ex_Stor);
		if (exp->ex_Cond >= 0)      /*  branch on true  */
		    asm_condbra(COND_NEQ, exp->ex_LabelT);
		else			    /*	branch on false */
		    asm_condbra(COND_EQ, exp->ex_LabelF);
	    }
	}
	break;
    case EF_RNU|EF_CONDACK:
	Assert(0);
    }
}

void
GenQuestion(pexp)
Exp **pexp;
{
    Assert(0);
}

