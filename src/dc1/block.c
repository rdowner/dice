/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  BLOCK.C
 */

#include "defs.h"

Prototype BlockStmt *CurGen;
Prototype short BlockCost;

BlockStmt *CurBlock;
BlockStmt *CurGen;
Var *TopVarBase;        /*  list of top-level variables */

short   BlockCost;      /*  how costly will operations be time-relative */
                        /*  (e.g. keep loop variables in registers when */
                        /*  we run out!)                                */

Prototype BlockStmt *BlockDown(short);
Prototype BlockStmt *BlockUp(void);
Prototype Var *BlockAddVar(Var *);
Prototype void BlockAddStmt(Stmt *);
Prototype void BlockFlagCallMade(void);
Prototype int32_t FindContinueLabel(void);
Prototype int32_t FindBreakLabel(void);
Prototype void GenPush(BlockStmt *);
Prototype void GenFlagCallMade(void);
Prototype void GenPop(BlockStmt *);
Prototype void BlockAddTop(Var *);
Prototype Var *BlockRemoveDuplicateVar(int32_t, Symbol *, Type *);
Prototype int CompareTypes(int32_t, int32_t, Type *, Type *);

BlockStmt *
BlockDown(short bid)
{
    BlockStmt *block = AllocTmpStructure(BlockStmt);

    SemanticLevelDown();

    block->st_Func = (void (*)(void *))GenBlock;
    block->st_Tok  = TokBlock;
    block->Bid = bid;
    block->Parent = CurBlock;
    block->Last = &block->Base;
    block->LastVar = &block->VarBase;

    CurBlock = block;

    return(block);
}

BlockStmt *
BlockUp()
{
    BlockStmt *block = CurBlock;

    Assert(CurBlock);

    block->LastLexIdx = LFBase->lf_Index;

    if (block->Parent) {
        if (block->Frame.Flags & FF_CALLMADE)
            block->Parent->Frame.Flags |= FF_CALLMADE;
    }
    CurBlock = block->Parent;

    SemanticLevelUp();

    return(CurBlock);
}

/*
 *  static variables are added in front so they are run BEFORE any
 *  lea'd dummies of them.
 *
 *  Returns base of list added to current block minus any variables added
 *  to the top level (i.e. procedures)
 */

Var *
BlockAddVar(var)
Var *var;
{
    BlockStmt *block;
    Var *vnext;
    Var **vbase;

    block = CurBlock;
    vbase = block->LastVar;

    while (var) {
        vnext = var->Next;
        var->Next = NULL;

        if (var->Type->Id == TID_PROC || (var->Flags & TF_EXTERN)) {
            BlockAddTop(var);
        } else {
            Assert(block);
            if (State == SARG)
                var->Flags |= VF_ARG;
            *block->LastVar = var;
            block->LastVar = &var->Next;
        }
        var = vnext;
    }
    return(*vbase);
}

void
BlockAddStmt(stmt)
Stmt *stmt;
{
    BlockStmt *block = CurBlock;

    Assert(block);
    if (stmt) {
        *block->Last = stmt;
        block->Last = &stmt->st_Next;
    }
}

void
BlockFlagCallMade()
{
    Assert(CurBlock);
    /*CurBlock->Frame.RegCantUse |= REGSCRATCH;*/
    CurBlock->Frame.Flags |= FF_CALLMADE;
}


int32_t
FindContinueLabel()
{
    BlockStmt *block = CurBlock;

    while (block) {
        switch(block->Bid) {
        case BT_FOR:
        case BT_WHILE:
        case BT_DO:
            return(block->LabelTest);
        }
        block = block->Parent;
    }
    return(0);
}

int32_t
FindBreakLabel()
{
    BlockStmt *block = CurBlock;

    while (block) {
        switch(block->Bid) {
        case BT_FOR:
        case BT_WHILE:
        case BT_DO:
        case BT_SWITCH:
            return(block->LabelBreak);
        }
        block = block->Parent;
    }
    return(0);
}

/*
 *                      Generation phase
 */

void
GenPush(child)
BlockStmt *child;
{
    BlockStmt *block = CurGen;

    Assert(child);
    Assert(child->Parent == block);
    /* cerror(ESOFT, "Block mismatch %08lx %08lx %08lx", child, child->Parent, block); */
    if (block) {
        child->Frame.StackParent = block->Frame.StackParent + block->Frame.StackUsed;
    } else {
        ResetRegAlloc();
        child->Frame.StackParent = 0;
    }
    child->Frame.StackUsed     = 0;
    child->Frame.DownStackUsed = 0;
    child->Frame.ArgsStackUsed = 0;
    child->Frame.SubARegOver = 0;
    child->Frame.SubDRegOver = 0;
    child->Frame.CurARegOver = 0;
    child->Frame.CurDRegOver = 0;

    PushStackStorage();

    /*
    if (child->Frame.RegCantUse)
        RegDisableRegs(child->Frame.RegCantUse);
    else
        RegEnableRegs();
    */

    CurGen = child;
}

void
GenFlagCallMade()
{
    Assert(CurGen);
    /*CurGen->Frame.RegCantUse |= REGSCRATCH;*/
    if ((CurGen->Frame.Flags & FF_CALLMADE) == 0) {
        RegFlagTryAgain();
        CurGen->Frame.Flags |= FF_CALLMADE;
    }
}

/*
 *  pop a frame.  Count # of registers over-allocated.
 */

void
GenPop(chk)
BlockStmt *chk;
{
    BlockStmt *block;
    BlockStmt *child;

    Assert(chk == CurGen);
    Assert(child = CurGen);

    PopStackStorage();
    if ((block = chk->Parent) != NULL) {
        if (block->Frame.DownStackUsed < child->Frame.StackUsed + child->Frame.DownStackUsed)
            block->Frame.DownStackUsed = child->Frame.StackUsed + child->Frame.DownStackUsed;
        block->Frame.DownStackUsed = Align(block->Frame.DownStackUsed, StackAlign);

        /*
         *  asm2.c, support routines call.  RegCantUse effects register
         *  allocation of scratch registers.
         */

        if (child->Frame.Flags & FF_CALLMADE)
            block->Frame.Flags |= FF_CALLMADE;

        if (block->Frame.SubARegOver < child->Frame.CurARegOver + child->Frame.SubARegOver)
            block->Frame.SubARegOver = child->Frame.CurARegOver + child->Frame.SubARegOver;
        if (block->Frame.SubDRegOver < child->Frame.CurDRegOver + child->Frame.SubDRegOver)
            block->Frame.SubDRegOver = child->Frame.CurDRegOver + child->Frame.SubDRegOver;
    }
    CurGen = block;
}

/*
 *  Deal with procedure declarations and possibly externs that exist
 *  within
 */

void
BlockAddTop(var)
Var *var;
{
    Var *first = var;

    for (;;) {
        AllocExternalStorage(var->Sym, &var->var_Stor, var->Type, var->Flags);
        if (var->Next == NULL)
            break;
        var = var->Next;
    }
    var->Next = TopVarBase;
    TopVarBase = first;
}

/*
 *  This handles subroutine declarations within subroutines by bumping them up
 *  to the top level, and forward declarations (i.e. an extern at top and the
 *  actual variable declared later on).
 */

Var *
BlockRemoveDuplicateVar(storFlags, sym, type)
int32_t storFlags;
Symbol *sym;
Type *type;
{
    Var **base;
    Var *var;

    if (type->Id == TID_PROC || ((storFlags & TF_AUTO) == 0 && State == SOUTSIDE)) {
        base = &TopVarBase;

        while (*base && (*base)->Sym != sym)
            base = &(*base)->Next;
        if ((var = *base) != NULL) { 
            /*
             * uh oh, duplicate... extern?.  If proc auto ext
             */
            *base = var->Next;
            var->Next = NULL;

            /*
             *  If a procedure use prototype'd type.  Compare args.
             *
             *  If a normal declaration we have a duplicate.  However, if
             *  -mu (UNIX common variables) are implemented allow a new
             *  declaration to overide the old.
             */

            if (type->Id != TID_PROC && (storFlags & TF_EXTERN) == 0 && (var->Flags & TF_EXTERN) == 0)
            {
                if (UnixCommonOpt && var->Refs == 0 && var->u.AssExp == NULL)
                    ;
                else
                    yerror(var->LexIdx, EERROR_DUPLICATE_SYMBOL,
                           var->Sym->Len, var->Sym->Name);
                var = NULL;
            } else if (type != var->Type &&
                       CompareTypes(var->LexIdx, LFBase->lf_Index, type, var->Type) < 0) {
                var = NULL;
            }
        }
    } else {
        Assert(CurBlock);
        var = NULL;
    }
    return(var);
}

int
CompareTypes(int32_t olexIdx, int32_t lexIdx, Type *tn, Type *to)
{
    while (tn != to) {
        if (tn == NULL || to == NULL)
            break;
/* what about TID_FLT */
        switch(tn->Id) {
        case TID_INT:
            if (*tn->Size == *to->Size && ((tn->Flags ^ to->Flags) & TF_COMPAREQUALS) == 0)
                return(0);
            if (to->Id != TID_INT) return(-1);
            break;
        case TID_PTR:
        case TID_ARY:
            /*
             *  XXX extern fubar[]; verses declaration *fubar; not checked
             */
            if ((to->Id != TID_PTR) && (to->Id != TID_ARY)) return(-1);
            if ((tn->Id == TID_PTR && *tn->SubType->Size == 0) ||
                (to->Id == TID_PTR && *to->SubType->Size == 0))
                return(0);
            tn = tn->SubType;
            to = to->SubType;
            continue;
        case TID_PROC:
            /* (ProtoOnlyOpt) ? EERROR : EWARN */
            if (to->Id != TID_PROC)
            {
               yerror(lexIdx, EERROR_ILLEGAL_ASSIGNMENT);
               return(-1);
            }

            if (CompareTypes(olexIdx, lexIdx, tn->SubType, to->SubType) < 0)
            {
                yerror(lexIdx,  EWARN_RETURN_MISMATCH, TypeToProtoStr(tn->SubType, 0));
                yerror(olexIdx, EWARN_DOES_NOT_MATCH,  TypeToProtoStr(to->SubType, 0));
            }
            if (to->Args > 0 && tn->Args < 0)
            {
                yerror(lexIdx, (ProtoOnlyOpt) ? EERROR_NON_PROTOTYPE
                                              : EWARN_NON_PROTOTYPE);
                break;
            }
            if (to->Args < 0 || tn->Args < 0)
                return(0);

            /*
             *  prototyped
             */

            if (to->Args == tn->Args)
            {
                short i;

                for (i = 0; i < tn->Args; ++i) {
                    Var *v1 = tn->Vars[i];
                    Var *v2 = to->Vars[i];

                    if (CompareTypes(olexIdx, lexIdx, v1->Type, v2->Type) < 0) {
                        yerror(lexIdx, EERROR_ARGUMENT_MISMATCH,
                                       i + 1, TypeToProtoStr(v1->Type, 0));
                        yerror(olexIdx, EWARN_DOES_NOT_MATCH,
                                       TypeToProtoStr(v2->Type, 0));
                        break;
                    }
                    if ((v1->RegFlags ^ v2->RegFlags) & (RF_REGISTER|RF_REGMASK)) {
                        yerror(lexIdx, EERROR_REGISTER_MISMATCH, i + 1);
                        break;
                    }
                }
                if (i == tn->Args)
                    return(0);
            }
            else
            {
                yerror(lexIdx, EERROR_ARGUMENT_COUNT);
            }
            break;
        case TID_STRUCT:
        case TID_UNION:
        case TID_BITFIELD:
            if (tn->Id == to->Id && tn->Args == to->Args && tn->Vars == to->Vars)
                return(0);
            yerror(lexIdx, EERROR_INCOMPATIBLE_STRUCTURE, TypeToProtoStr(tn, 0));
            yerror(olexIdx, EWARN_DOES_NOT_MATCH, TypeToProtoStr(to, 0));
            break;
        }
        break;
    }
    if (tn != to)
        return(-1);
    return(0);
}

