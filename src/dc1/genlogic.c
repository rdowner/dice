/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  GENLOGIC.C
 *
 *      logic functions like &, |, etc..
 *
 *  WARNING:    asm_shift must deal with source & dest being different sizes
 *              due to auto-assign
 *
 *  NOTE:  &=, |=, etc.. with lhs bit fields, CreateBinaryResult will
 *         bfext the lhs.  It is expected that the lhs is already freed.
 */

#include "defs.h"

Prototype void GenAnd(Exp **);
Prototype void GenOr(Exp **);
Prototype void GenXor(Exp **);
Prototype void GenLShf(Exp **);
Prototype void GenRShf(Exp **);
Prototype void GenCompl(Exp **);

/*
 *  AND.  We do not have to extend results to an integer:
 *
 *      0 & 0 -> 0
 *      0 & 1 -> 0
 *      1 & 0 -> 0
 *      1 & 1 -> 1
 *
 *  We also optimize for conditionals here.  However, this does not
 *  work for the &= operator so we have to check
 */

void
GenAnd(pexp)
Exp **pexp;
{
    Exp *exp = *pexp;
    Exp *e1;
    Exp *e2;

    if (exp->ex_Type) {
        if (exp->ex_ExpL->ex_Type == NULL)
            exp->ex_ExpL->ex_Type = exp->ex_Type;
        if (exp->ex_ExpR->ex_Type == NULL)
            exp->ex_ExpR->ex_Type = exp->ex_Type;
    }

    CallLeft();
    EnsureReturnStorageLeft();

    if (GenPass == 0) {
        if (exp->ex_Flags & EF_ASSEQ) { /*  not req'd. optmizio */
            Type *t = exp->ex_ExpL->ex_Type;
            if (exp->ex_ExpR->ex_Type == NULL)
                exp->ex_ExpR->ex_Type = t;
            exp->ex_Type = t;
        }
        CallRight();

        e1 = exp->ex_ExpL;
        e2 = exp->ex_ExpR;

        /*
         *  for conditionals, unsignedness can matter.  For example,
         *  char a;  if ((a & 0x80) < 0) ... would not work if we allowed
         *  the return type to be a char.  (the latter example is NEVER
         *  less than 0 since the char is supposed to be extended first.
         *
         *  Do not reduce return size requirements for longs to allow btst
         *  optimization without requiring a move
         */

        if (exp->ex_Type == NULL) {
#ifdef _DCC
            short isUnsigned = 0;
#else
            short isUnsigned __attribute__((unused)) = 0;
#endif
            if ((e1->ex_Type->Flags | e2->ex_Type->Flags) & TF_UNSIGNED)
                isUnsigned = 1;

            if (e1->ex_Stor.st_Type == ST_IntConst) {
                if (exp->ex_Flags & EF_COND) {
                    if (e1->ex_Stor.st_IntConst < 0x8000 && *e2->ex_Type->Size < 4)
                        exp->ex_Type = e2->ex_Type;
                    if (e1->ex_Stor.st_IntConst < 0x80 && *e2->ex_Type->Size == 1)
                        exp->ex_Type = e2->ex_Type;
                }
            }
            if (e2->ex_Stor.st_Type == ST_IntConst) {
                if (exp->ex_Flags & EF_COND) {
                    if (e2->ex_Stor.st_IntConst < 0x8000 && *e1->ex_Type->Size < 4)
                        exp->ex_Type = e1->ex_Type;
                    if (e2->ex_Stor.st_IntConst < 0x80 && *e1->ex_Type->Size == 1)
                        exp->ex_Type = e1->ex_Type;
                }
            }
        }

        BinaryLogicRules(exp);

        e1 = exp->ex_ExpL;
        e2 = exp->ex_ExpR;

        if (e1->ex_Stor.st_Type == ST_IntConst && e2->ex_Stor.st_Type == ST_IntConst) {
            e1->ex_Stor.st_IntConst = e1->ex_Stor.st_IntConst & e2->ex_Stor.st_IntConst;
            *pexp = e1;
        }
        exp->ex_Flags |= (e1->ex_Flags | e2->ex_Flags) & EF_CALL;
    } else {
        CallRight();
        e1 = exp->ex_ExpL;
        e2 = exp->ex_ExpR;

        if ((exp->ex_Flags & EF_COND) && !(exp->ex_Flags & EF_ASSEQ)) {
            FreeStorage(&e1->ex_Stor);
            FreeStorage(&e2->ex_Stor);
            exp->ex_Flags |= EF_CONDACK;
            asm_test_and(exp, &e1->ex_Stor, &e2->ex_Stor);
            if (exp->ex_Cond >= 0)
                asm_condbra(COND_NEQ, exp->ex_LabelT);
            else
                asm_condbra(COND_EQ, exp->ex_LabelF);
        } else {
            if (CreateBinaryResultStorage(exp, 1))
                asm_and(exp, &e1->ex_Stor, &e2->ex_Stor, &exp->ex_Stor);
        }
    }
}

/*
 *  OR.  We do not have to extend results to an integer:
 *
 *      0 | 0 -> 0
 *      0 | 1 -> 1
 *      1 | 0 -> 1
 *      1 | 1 -> 1
 */

void
GenOr(pexp)
Exp **pexp;
{
    Exp *exp = *pexp;
    Exp *e1;
    Exp *e2;

    if (exp->ex_Type) {
        if (exp->ex_ExpL->ex_Type == NULL)
            exp->ex_ExpL->ex_Type = exp->ex_Type;
        if (exp->ex_ExpR->ex_Type == NULL)
            exp->ex_ExpR->ex_Type = exp->ex_Type;
    }
    CallLeft();
    EnsureReturnStorageLeft();

    if (GenPass == 0) {
        if (exp->ex_Flags & EF_ASSEQ) { /*  not req'd. optmizio */
            Type *t = exp->ex_ExpL->ex_Type;
            if (exp->ex_ExpR->ex_Type == NULL)
                exp->ex_ExpR->ex_Type = t;
            exp->ex_Type = t;
        }
        CallRight();
        BinaryLogicRules(exp);

        e1 = exp->ex_ExpL;
        e2 = exp->ex_ExpR;

        if (e1->ex_Stor.st_Type == ST_IntConst && e2->ex_Stor.st_Type == ST_IntConst) {
            e1->ex_Stor.st_IntConst = e1->ex_Stor.st_IntConst | e2->ex_Stor.st_IntConst;
            *pexp = e1;
        }
        exp->ex_Flags |= (e1->ex_Flags | e2->ex_Flags) & EF_CALL;
    } else {
        CallRight();
        e1 = exp->ex_ExpL;
        e2 = exp->ex_ExpR;

        if (CreateBinaryResultStorage(exp, 1))
            asm_or(exp, &e1->ex_Stor, &e2->ex_Stor, &exp->ex_Stor);
    }
}

/*
 *  XOR.  We do not have to extend results to an integer:
 *
 *      0 | 0 -> 0
 *      0 | 1 -> 1
 *      1 | 0 -> 1
 *      1 | 1 -> 0
 */

void
GenXor(pexp)
Exp **pexp;
{
    Exp *exp = *pexp;
    Exp *e1;
    Exp *e2;

    if (exp->ex_Type) {
        if (exp->ex_ExpL->ex_Type == NULL)
            exp->ex_ExpL->ex_Type = exp->ex_Type;
        if (exp->ex_ExpR->ex_Type == NULL)
            exp->ex_ExpR->ex_Type = exp->ex_Type;
    }
    CallLeft();
    EnsureReturnStorageLeft();

    if (GenPass == 0) {
        if (exp->ex_Flags & EF_ASSEQ) { /*  not req'd. optmizio */
            Type *t = exp->ex_ExpL->ex_Type;
            if (exp->ex_ExpR->ex_Type == NULL)
                exp->ex_ExpR->ex_Type = t;
            exp->ex_Type = t;
        }
        CallRight();
        BinaryLogicRules(exp);

        e1 = exp->ex_ExpL;
        e2 = exp->ex_ExpR;

        if (e1->ex_Stor.st_Type == ST_IntConst && e2->ex_Stor.st_Type == ST_IntConst) {
            e1->ex_Stor.st_IntConst = e1->ex_Stor.st_IntConst ^ e2->ex_Stor.st_IntConst;
            *pexp = e1;
        }
        exp->ex_Flags |= (e1->ex_Flags | e2->ex_Flags) & EF_CALL;
    } else {
        CallRight();
        e1 = exp->ex_ExpL;
        e2 = exp->ex_ExpR;

        if (CreateBinaryResultStorage(exp, 1))
            asm_xor(exp, &e1->ex_Stor, &e2->ex_Stor, &exp->ex_Stor);
    }
}

void
GenLShf(pexp)
Exp **pexp;
{
    Exp *exp = *pexp;
    Exp *e1;
    Exp *e2;

    CallLeft();
    EnsureReturnStorageLeft();
    CallRight();

    if (GenPass == 0) {
        ShiftRules(exp);

        e1 = exp->ex_ExpL;
        e2 = exp->ex_ExpR;

        if (e1->ex_Stor.st_Type == ST_IntConst && e2->ex_Stor.st_Type == ST_IntConst) {
            e1->ex_Stor.st_IntConst = e1->ex_Stor.st_IntConst << e2->ex_Stor.st_IntConst;
            *pexp = e1;
        }
        exp->ex_Flags |= (e1->ex_Flags | e2->ex_Flags) & EF_CALL;
    } else {
        e1 = exp->ex_ExpL;
        e2 = exp->ex_ExpR;

        if (CreateBinaryResultStorage(exp, 1))
            asm_shift(exp, -1, &e1->ex_Stor, &e2->ex_Stor, &exp->ex_Stor);
    }
}

void
GenRShf(pexp)
Exp **pexp;
{
    Exp *exp = *pexp;
    Exp *e1;
    Exp *e2;

    CallLeft();
    EnsureReturnStorageLeft();
    CallRight();

    if (GenPass == 0) {
        ShiftRules(exp);

        e1 = exp->ex_ExpL;
        e2 = exp->ex_ExpR;

        if (e1->ex_Stor.st_Type == ST_IntConst && e2->ex_Stor.st_Type == ST_IntConst) {
            if (e1->ex_Type->Flags & TF_UNSIGNED)
                e1->ex_Stor.st_IntConst = e1->ex_Stor.st_UIntConst >> e2->ex_Stor.st_UIntConst;
            else
                e1->ex_Stor.st_IntConst = e1->ex_Stor.st_IntConst >> e2->ex_Stor.st_IntConst;
            *pexp = e1;
        }
        exp->ex_Flags |= (e1->ex_Flags | e2->ex_Flags) & EF_CALL;
    } else {
        e1 = exp->ex_ExpL;
        e2 = exp->ex_ExpR;

        if (CreateBinaryResultStorage(exp, 1))
            asm_shift(exp, 1, &e1->ex_Stor, &e2->ex_Stor, &exp->ex_Stor);
    }
}


void
GenCompl(pexp)
Exp **pexp;
{
    Exp *exp = *pexp;
    Exp *e1;

    if (exp->ex_Type && exp->ex_ExpL->ex_Type == NULL)
        exp->ex_ExpL->ex_Type = exp->ex_Type;

    CallLeft();

    if (GenPass == 0) {
        UnaryLogicRules(exp);

        e1 = exp->ex_ExpL;

        if (e1->ex_Stor.st_Type == ST_IntConst) {
            e1->ex_Stor.st_IntConst = ~e1->ex_Stor.st_IntConst;
            *pexp = e1;
        }
        exp->ex_Flags |= e1->ex_Flags & EF_CALL;
    } else {
        Stor con;

        e1 = exp->ex_ExpL;

        AllocConstStor(&con, -1, exp->ex_Type);
        if (CreateUnaryResultStorage(exp, 1))
            asm_xor(exp, &con, &e1->ex_Stor, &exp->ex_Stor);
    }
}

void
filler_gen_logic(void)
{
    Assert(1);
}

