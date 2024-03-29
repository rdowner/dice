/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */
/*
 *  GENASS.C
 *
 *      Assignment operators
 */

/*
**      $Filename: genass.c $
**      $Author: dice $
**      $Revision: 30.326 $
**      $Date: 1995/12/24 06:09:40 $
**      $Log: genass.c,v $
 * Revision 30.326  1995/12/24  06:09:40  dice
 * .
 *
 * Revision 30.154  1995/01/09  13:19:15  dice
 * added check for illegal assignment with constant on lhs
 *
 * Revision 30.5  1994/06/13  18:37:28  dice
 * byte ordering portability
 *
 * Revision 30.0  1994/06/10  18:04:51  dice
 * .
 *
 * Revision 1.4  1993/11/22  00:28:36  jtoebes
 * Final cleanup to eliminate all cerror() messages with strings.
 *
 * Revision 1.3  1993/04/10  17:14:00  jtoebes
 * Inplemented static initialization of bitfields in structures.
 *
**/

#include "defs.h"

Prototype void GenEq(Exp **);
Prototype void GenBracEq(Exp **);
Prototype void GenBracedAssign(Exp **);
Prototype void GenPercentEq(Exp **);
Prototype void GenAndEq(Exp **);
Prototype void GenStarEq(Exp **);
Prototype void GenMiEq(Exp **);
Prototype void GenDivEq(Exp **);
Prototype void GenLtLtEq(Exp **);
Prototype void GenGtGtEq(Exp **);
Prototype void GenPlEq(Exp **);
Prototype void GenOrEq(Exp **);
Prototype void GenCaratEq(Exp **);

Prototype void GenAssEq(Exp **);
Prototype void GenSpecialAssignment(Exp **, void (*)(Exp **));
          void PadStructure(int, int32_t *);

/*
 *  Note: to change propogation to rhs, the lhs must still be propogated
 *  when bitfields are involved.  Also what about &(a = 4); ?
 */

void
GenEq(pexp)
Exp **pexp;
{
    Exp *exp = *pexp;
    Exp *e1;
    Exp *e2;

    if (GenPass == 0) {
#ifdef NOTDEF   /* future opt */
        e1 = exp->ex_ExpL;
        if (e1->ex_Token == TokPreDec)
            exp->ex_Flags |= EF_ASSPREDEC;
        if (e1->ex_Token == TokPosInc)
            exp->ex_Flags |= EF_ASSPOSINC;
#endif
        exp->ex_ExpL->ex_Flags |= EF_LHSASSIGN;

        CallLeft();

        if (exp->ex_ExpL->ex_Token == TokVarRef)
            exp->ex_ExpL->ex_Var->RegFlags |= RF_MODIFIED;

        /*
         *  Presetting the type is an optimization that is not required.
         *  example:    char a, b; a = ~b; so ~b is done w/ chars.
         */

        if (exp->ex_ExpR->ex_Type == NULL)
            exp->ex_ExpR->ex_Type = exp->ex_ExpL->ex_Type;

        CallRight();
        AssignRules(exp);
        exp->ex_Flags |= EF_CRES;
        exp->ex_Flags |= (exp->ex_ExpL->ex_Flags | exp->ex_ExpR->ex_Flags) & EF_CALL;
    } else {
        e2 = exp->ex_ExpR;

        /*
         * a[i] = a[j] = a[k] = ... x;  must to rhs first or would build
         * up scratch regs
         */

        exp->ex_ExpL->ex_Flags |= EF_LHSASSIGN;

        if (e2->ex_Flags & (EF_ASSEQ|EF_CRES)) {
            CallRight();
            EnsureReturnStorageRight();
            CallLeft();
            e1 = exp->ex_ExpL;
            e2 = exp->ex_ExpR;
        } else {
            CallLeft();
            EnsureReturnStorageLeft();
            e1 = exp->ex_ExpL;
            e2 = exp->ex_ExpR;
            if (e1->ex_Type->Id != TID_BITFIELD && (e2->ex_Flags & EF_CRES) == 0) {
                e2->ex_Stor = e1->ex_Stor;
                e2->ex_Flags |= EF_PRES;
            }
            CallRight();
        }

        e1 = exp->ex_ExpL;
        e2 = exp->ex_ExpR;

        if (e1->ex_Type->Id == TID_BITFIELD) {
            e2 = exp->ex_ExpR;
            asm_bfsto(exp, &e2->ex_Stor, &e1->ex_Stor);
            FreeStorage(&e2->ex_Stor);
        } else if (e2->ex_Flags & (EF_CRES|EF_ASSEQ)) {
            e2 = exp->ex_ExpR;
            asm_move(exp, &e2->ex_Stor, &e1->ex_Stor);
            if (e1->ex_Stor.st_Type < ST_Reg && e1->ex_Stor.st_Type != ST_PtrConst)
                yerror(exp->ex_LexIdx, EERROR_ILLEGAL_ASSIGNMENT);
            FreeStorage(&e2->ex_Stor);
        }
        FreeStorage(&e1->ex_Stor);
        if ((exp->ex_Flags & EF_RNU) == 0)
            ReuseStorage(&e1->ex_Stor, &exp->ex_Stor);
    }
}

/*
 *  Generate a braced assignment.  This is a two step process.  Step 1
 *  copies static data to the local variable, Step 2 generates additonal
 *  code to handle variable expressions.
 */

void
GenBracEq(pexp)
Exp **pexp;
{
    Exp *exp = *pexp;

    if (GenPass == 0) {
        exp->ex_ExpL->ex_Flags |= EF_LHSASSIGN;
        CallLeft();
        CallRight();
    } else {
        Stor t;
        Stor ss;

        /*
         *  process lhs, obtain effective address, then sequence
         *  the storage structure according to the type
         */

        CallLeft();
        asm_getlea(exp, &exp->ex_ExpL->ex_Stor, &t);
        exp->ex_ExpR->ex_Stor = t;
        AutoAggregateBeg(&ss, exp->ex_ExpL->ex_Type);
        t.st_Flags &= ~SF_LEA;  /*  XXX hack    */
        t.st_Size = *exp->ex_ExpL->ex_Type->Size;
        asm_move(exp, &ss, &t);
        CallRight();
        AutoAggregateEnd();
    }
}

void PadStructure(size, bfhold)
int size;
int32_t *bfhold;
{
   if (size > 4)
   {
        AutoAggregate(bfhold, 4);
        AutoAggregate(NULL, size-4);
   }
   else
   {
        AutoAggregate(bfhold, size);
   }
   *bfhold = 0;
}

/*
 *  Generate a braced assignment.   Essentially run through the structure
 *  generated by GCompBracedAssign() generating code for non-constant
 *  objects and building a table for constant objects.
 *
 *  Each TokExpAssBlock already has an ex_Type assigned to it, for pass 2
 *  storage is tracked for auto's
 *
 *  A TokExpAssBlock expression node may contain a linked list of
 *  constants before it's nominal linked list of expressions.  This
 *  hack was introduced to save massive amounts of space for large
 *  static-initialized arrays.
 */

void
GenBracedAssign(pexp)
Exp **pexp;
{
    Exp *exp = *pexp;

    if (GenPass == 0)
    {
        Type *type = exp->ex_Type;
        int32_t index = 0;

        Assert(type);

        if (exp->ex_ConstAry) {
            int32_t *captr;

            for (captr = exp->ex_ConstAry; captr; captr = *(int32_t **)captr, ++index) {
                ;
            }
        }
        if (exp->ex_ExpL) {
            Exp **pnext;

            for (pnext = &exp->ex_ExpL; (exp = *pnext) != NULL; pnext = &exp->ex_Next, ++index) {
                Type *actType;
                Exp *nextExp = exp->ex_Next;

                if (exp->ex_Token == TokExpAssBlock) {
                    CallCenter();
                } else {
                    if (type->Id == TID_ARY)
                        actType = type->SubType;
                    else if (type->Id == TID_STRUCT)
                        actType = type->Vars[index]->Type;
                    else if (type->Id == TID_UNION)
                        actType = type->Vars[0]->Type;
                    else
                        actType = type;

                   /*
                    *  type usually refers to the parent type, e.g. ary
                    *  actType refers to the data type, e.g. int
                    */


                    if (exp->ex_Type == NULL)
                        exp->ex_Type = actType;

                    /* Check the type to ensure that it is compatible with what */
                    /* they are asking for.  The only exception is when we are  */
                    /* assigning a strings constant to an array of bytes.  This */
                    /* is just one of those funny cases that C allows through   */
                    if (exp->ex_Token != TokStrConst ||
                        type->Id != TID_ARY ||
                        *actType->Size != 1)
                    {
                        CallCenter();
                        InitRules(&exp, actType);
                    }
                    exp->ex_Next = nextExp;
                    *pnext = exp;
                }
            }
        }
    } else {

        Type *type = exp->ex_Type;
        Type *actType;
        Exp *nextExp;
        Stor t;
        Stor c;
        int32_t index = 0;     /*       index, per char for string, else struct/union */
        int32_t baseOffset;    /*       bytes accumulated as we go      */
        int32_t bfhold = 0;    /*  Used to accumulate bitfield initializations   */
        Exp **next;

        /*
         *  When generating global data we track just the laid-down bytes.
         *  When generating local auto-aggregate data we track both the
         *  laid-down constants (bytes)
         */

        Assert(type);

        if (GenGlobal == 0)
            t = exp->ex_Stor;           /*  base base offset (real offset)  */

        baseOffset = 0;                 /*  relative offset                 */

        if (exp->ex_ConstAry) {
            int32_t *captr;

            for (captr = exp->ex_ConstAry; captr; captr = *(int32_t **)captr, ++index) {
                int caval = *(int32_t *)((char *)captr + sizeof(void *));

                if (type->Id == TID_ARY)
                    actType = type->SubType;
                else if (type->Id == TID_STRUCT)
                    actType = type->Vars[index]->Type;
                else if (type->Id == TID_UNION)
                    actType = type->Vars[0]->Type;
                else
                    actType = type;
                Assert(actType);

                if (type->Id == TID_STRUCT) {
                    int32_t offset = type->Vars[index]->var_Stor.st_Offset;
                    if (offset > baseOffset) {
                        PadStructure(offset - baseOffset, &bfhold);
                        baseOffset = offset;
                    }
                }

                if (actType->Id == TID_BITFIELD)
                {
                    int32_t offset = INT_SIZE * 8;
                    if (type->Id == TID_STRUCT)
                        offset = type->Vars[index]->u.BOffset;
                    else if (type->Id != TID_INT)
                        yerror(exp->ex_LexIdx, EERROR_ILLEGAL_ASSIGNMENT);

                    bfhold |= (caval & ((1 << *actType->Size) - 1)) << offset;
                    /* We don't have the fix baseOffset because we are just */
                    /* a bitfield and may be putting more into the bitfield */
                    /* We instead rely upon the structure alignment code to */
                    /* Adjust the offset and flush out the bitfield that we */
                    /* have gathered.                                       */
                }
                else
                {
                    /* If we have any bitfield residuals, we need to get it */
                    /* to be popped out.                                    */
                    switch(*actType->Size)
                    {
                    case 0:
                        break;
                    case 1:
                        {
                            char c = caval;
                            AutoAggregate(&c, 1);
                        }
                        break;
                    case 2:
                        {
                            short c = ToMsbOrderShort(caval);
                            AutoAggregate(&c, 2);
                        }
                        break;
                    case 4:
                        {
                            int32_t c = ToMsbOrder(caval);
                            AutoAggregate(&c, 4);
                        }
                        break;
                    default:
                        Assert(0);
                        break;
                    }

                    baseOffset += *actType->Size;
                }
            }
        }
        for (next = &exp->ex_ExpL; (exp = *next) != NULL; next = &exp->ex_Next, ++index) {
            nextExp = exp->ex_Next;
            if (type->Id == TID_ARY)
                actType = type->SubType;
            else if (type->Id == TID_STRUCT)
                actType = type->Vars[index]->Type;
            else if (type->Id == TID_UNION)
                actType = type->Vars[0]->Type;
            else
                actType = type;
            Assert(actType);

            /*
             *  Determine starting alignment -- applies to structures
             */

            if (type->Id == TID_STRUCT) {
                int32_t offset = type->Vars[index]->var_Stor.st_Offset;
                if (offset > baseOffset) {
                    PadStructure(offset - baseOffset, &bfhold);
                    baseOffset = offset;
                }
            }

            if (exp->ex_Token == TokExpAssBlock) {
                /*
                 *  Propogate real offset down
                 */

                if (GenGlobal == 0) {
                    exp->ex_Stor = t;
                    exp->ex_Stor.st_Offset += baseOffset;
                }

                CallCenter();

                baseOffset += *actType->Size;
            } else {

                /*
                 *  string constants are a special case.  They are forced
                 *  into their own subblock since the index represents
                 *  the byte index rather then a structure field index
                 */

                if (exp->ex_Token == TokStrConst && type->Id == TID_ARY && *actType->Size == 1) {
                    int32_t offset = exp->ex_StrLen;
                    AutoAggregate(exp->ex_StrConst, offset);

                    baseOffset += offset;
                    index += offset - 1;
                } else {
                    CallCenter();
                    *next = exp;
                    exp->ex_Next = nextExp;

                    /*
                     *  If constant then lay it out in the static portion of
                     *  the initialization routine, else lay out 0 in the
                     *  static portion and generate code.
                     */

                    if (actType->Id == TID_BITFIELD)
                    {
                        int32_t offset = INT_SIZE * 8;
                        int32_t val    = 0;

                        if (exp->ex_Stor.st_Type == ST_IntConst)
                            val = exp->ex_Stor.st_IntConst;
                        else
                            yerror(exp->ex_LexIdx, EERROR_ILLEGAL_ASSIGNMENT);

                        if (type->Id == TID_STRUCT)
                            offset = type->Vars[index]->u.BOffset;
                        else if (type->Id != TID_INT)
                            yerror(exp->ex_LexIdx, EERROR_ILLEGAL_ASSIGNMENT);

                        bfhold |= (val & ((1 << *actType->Size) - 1)) << offset;

                        /* We don't have the fix baseOffset because we are just */
                        /* a bitfield and may be putting more into the bitfield */
                        /* We instead rely upon the structure alignment code to */
                        /* Adjust the offset and flush out the bitfield that we */
                        /* have gathered.                                       */
                    }
                    else
                    {

                        switch(exp->ex_Stor.st_Type) {
                        case ST_IntConst:
                            switch(*actType->Size) {
                            case 0:
                                break;
                            case 1:
                                {
                                    char c = exp->ex_Stor.st_IntConst;
                                    AutoAggregate(&c, 1);
                                }
                                break;
                            case 2:
                                {
                                    short c = ToMsbOrderShort(exp->ex_Stor.st_IntConst);
                                    AutoAggregate(&c, 2);
                                }
                                break;
                            case 4:
                                {
                                    int32_t c = ToMsbOrder(exp->ex_Stor.st_IntConst);
                                    AutoAggregate(&c, 4);
                                }
                                break;
                            default:
                                Assert(0);
                                break;
                            }
                            break;
                        case ST_FltConst:
                            {
                                int32_t ary[4];

                                asm_fltconst(exp, &exp->ex_Stor, ary);
                                AutoAggregate(ary, exp->ex_Stor.st_Size);
                            }
                            break;
                        case ST_RelLabel:
                        case ST_RelName:
                            if (GenGlobal) {
                                AutoAggregateSync();
                                if (AbsData == 0) {
                                    int32_t l;

                                    if (PIOpt) {            /*  must relocate period    */
                                        l = AllocLabel();

                                        printf(
                                            "x%d\tdc.%c\t0\n"
                                            "\tsection\tautoinit0,code\n"
                                            "\tlea\t%s,A0\n",
                                            l, SizC[*actType->Size],
                                            StorToString(&exp->ex_Stor, NULL)
                                        );
                                        if ((uword)AsmState == ASM_CODE) {
                                            printf("\tlea\tx%d(pc),A1\n", l);
                                            printf("\tmove.%c\tA0,(A1)\n", SizC[*actType->Size]);
                                        } else {
                                            printf("\tmove.%c\tA0,x%d(A4)\n", SizC[*actType->Size], l);
                                        }
                                        puts(LastSectBuf);
                                    } else if (ResOpt && ((uword)AsmState == ASM_DATA || (uword)AsmState == ASM_BSS) && !(exp->ex_Stor.st_Flags & SF_CODE)) {
                                        l = AllocLabel();

                                        printf(
                                            "x%d\tdc.%c\t0\n"
                                            "\tsection\tautoinit0,code\n"
                                            "\tlea\t%s,A0\n",
                                            l, SizC[*actType->Size],
                                            StorToString(&exp->ex_Stor, NULL)
                                        );
                                        printf("\tmove.%c\tA0,x%d", SizC[*actType->Size], l);
                                        if (SmallData == 2)
                                            puts(".W");
                                        else
                                            puts("(A4)");
                                        puts(LastSectBuf);
                                    } else {
                                        if ((uword)AsmState == ASM_CODE && ResOpt && !(exp->ex_Stor.st_Flags & SF_CODE))
                                            zerror(EERROR_BADRELOC_IN_CONST);
                                        goto skip;
                                    }
                                } else {
skip:
                                    printf("\tdc.%c\t", SizC[*actType->Size]);

                                    /*
                                     *  must handle label, name indep.  Due to munging if data/code
                                     */


                                    if (exp->ex_Stor.st_Type == ST_RelLabel) {
                                        if (exp->ex_Stor.st_Flags & SF_REGARGS)
                                            putc('@', stdout);
                                        printf("l%d+%d\n", exp->ex_Stor.st_Label, exp->ex_Stor.st_Offset);
                                    } else if (exp->ex_Stor.st_Type == ST_RelName) {
                                        printf("%c%.*s+%d\n", (
                                            (exp->ex_Stor.st_Flags & SF_REGARGS) ? '@' : '_'),
                                            exp->ex_Stor.st_Name->Len,
                                            exp->ex_Stor.st_Name->Name,
                                            exp->ex_Stor.st_Offset
                                        );
                                    } else {
                                        puts(StorToString(&exp->ex_Stor, NULL));
                                    }
                                }
                                break;
                            }
                        default:
                            /*
                             *  For auto-aggregate generate the real offset
                             *  for the instruction.  t.st_Offset holds the
                             *  real base offset but does not include the
                             *  relative base offset.
                             */

                            if (GenGlobal == 0) {
                                t.st_Offset += baseOffset;
                                asm_getind(exp, TypeToPtrType(actType), &t, &c, -1, -1, 0);
                                asm_move(exp, &exp->ex_Stor, &c);
                                AutoAggregate(NULL, *actType->Size);
                                t.st_Offset -= baseOffset;
                            } else {
                                yerror(exp->ex_LexIdx, EERROR_NOT_LVALUE);
                            }
                            break;
                        }

                        baseOffset += *actType->Size;
                    }
                    FreeStorage(&exp->ex_Stor);
                }
            }
        }

        /*
         *  calculate bytes remaining then zero fill them (use baseOffset
         *  as temporary)
         */

        baseOffset = *type->Size - baseOffset;

        if (baseOffset > 0) {
            PadStructure(baseOffset, &bfhold);
        } else if (baseOffset < 0) {
            yerror((*pexp)->ex_LexIdx, EERROR_TOO_MANY_INITIALIZERS);
        }
    }
}

void
GenPercentEq(pexp)
Exp **pexp;
{
    GenSpecialAssignment(pexp, GenPercent);
}

void
GenAndEq(pexp)
Exp **pexp;
{
    GenSpecialAssignment(pexp, GenAnd);
}

void
GenStarEq(pexp)
Exp **pexp;
{
    GenSpecialAssignment(pexp, GenStar);
}

void
GenMiEq(pexp)
Exp **pexp;
{
    GenSpecialAssignment(pexp, GenMi);
}

void
GenDivEq(pexp)
Exp **pexp;
{
    GenSpecialAssignment(pexp, GenDiv);
}

void
GenLtLtEq(pexp)
Exp **pexp;
{
    GenSpecialAssignment(pexp, GenLShf);
}

void
GenGtGtEq(pexp)
Exp **pexp;
{
    GenSpecialAssignment(pexp, GenRShf);
}

void
GenPlEq(pexp)
Exp **pexp;
{
    GenSpecialAssignment(pexp, GenPl);
}

void
GenOrEq(pexp)
Exp **pexp;
{
    GenSpecialAssignment(pexp, GenOr);
}

void
GenCaratEq(pexp)
Exp **pexp;
{
    GenSpecialAssignment(pexp, GenXor);
}

/*
 *  handle special ass= expressions.  Setting the EF_ASSEQ flag will cause
 *  structure bitfield refs to request EF_BFNOFREE, then use
 *  CreateBinaryResult to copy the unfree storage.  bitfield += x; e.g.
 */

void
GenSpecialAssignment(pexp, genFunc)
Exp **pexp;
void (*genFunc)(Exp **);
{
    Exp *exp = *pexp;
    int32_t flags = exp->ex_Flags;

    exp->ex_Func = genFunc;
    exp->ex_Flags |= EF_ASSEQ;
    exp->ex_Flags &= ~EF_RNU;
    exp->ex_ExpL->ex_Flags |= EF_LHSASSEQ;
    (*exp->ex_Func)(pexp);
    InsertNot(pexp);
    exp = *pexp;
    exp->ex_Func= GenAssEq;
    exp->ex_Type = exp->ex_ExpL->ex_Type;
    if (flags & EF_RNU)
        exp->ex_Flags |= EF_RNU;
    exp->ex_Flags |= EF_CRES;
}

void
GenAssEq(pexp)
Exp **pexp;
{
    Exp *exp = *pexp;
    Exp *e1;

    CallLeft();
    e1 = exp->ex_ExpL;

    if (GenPass) {
        if (e1->ex_ExpL->ex_Token == TokBFExt) {
            FreeStorage(&e1->ex_ExpL->ex_ExpL->ex_Stor);
            asm_bfsto(exp, &e1->ex_Stor, &e1->ex_ExpL->ex_ExpL->ex_Stor);
        }
        if (exp->ex_Flags & EF_RNU)
            FreeStorage(&e1->ex_Stor);
        else
            exp->ex_Stor = e1->ex_Stor;
    }
}

