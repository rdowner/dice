/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  GEN.C
 *
 *  Code Generator
 *
 *  genfunc(&stmt)
 *
 *  The placing of arguments into registers is somewhat of a hack, but
 *  necessary.  The parameters must be placed into registers before the LINK
 *  instruction to allow the assembler to optimize it out.  Otherwise, the
 *  assembler would also have to deal with sp offsets.
 *
 */

/*
**      $Filename: gen.c $
**      $Author: dice $
**      $Revision: 30.326 $
**      $Date: 1995/12/24 06:09:39 $
**      $Log: gen.c,v $
 * Revision 30.326  1995/12/24  06:09:39  dice
 * .
 *
 * Revision 30.156  1995/01/11  05:04:45  dice
 * added 5-procedure limit for MINIDICE
 *
 * Revision 30.5  1994/06/13  18:37:27  dice
 * .
 *
 * Revision 30.0  1994/06/10  18:04:50  dice
 * .
 *
 * Revision 1.4  1993/11/22  00:28:36  jtoebes
 * Final cleanup to eliminate all cerror() messages with strings.
 *
 * Revision 1.3  1993/09/19  13:14:59  jtoebes
 * Fixed BUG00148 - Compiler does not catch gotos to non-existent label.
 * Changed intermediate gotolabel information to know more about the label.
 * Changed error message to be more descriptive about the error.
 *
**/

#include "defs.h"

#define SubStmtCall(label)   if (stmt->label) (*stmt->label->st_Func)(stmt->label)


Prototype Var *ProcVar;
Prototype short GenPass;
Prototype short ForceLinkFlag;
Prototype short GenGlobal;  /*  global data gen for genass.c    */

Var *ProcVar;
short   GenPass;
short   GenGlobal;
short   RefCmp;
short   RefCmpNext;
short   ARegRecov;
short   DRegRecov;
short   ForceLinkFlag;
Exp     DummyExp;
int32_t SaveLastLexIdxBeg;      /*  place marker for retry pass     */
int32_t SaveLastLexLine;
int32_t LastLexIdxBeg;          /*  place marker for debug output   */
int32_t LastLexLine;


Prototype int32_t AllocLabel(void);
Prototype void InitGen(int32_t);
Prototype void GenerateVar(Var *);
Prototype void GenBlock(BlockStmt *);
Prototype void GenFor(ForStmt *);
Prototype void GenWhile(WhileStmt *);
Prototype void GenDo(DoStmt *);
Prototype void GenIf(IfStmt *);
Prototype void GenSwitch(SwitchStmt *);
Prototype void GenBreak(BreakStmt *);
Prototype void GenContinue(ContinueStmt *);
Prototype void GenGoto(GotoStmt *);
Prototype void GenLabel(LabelStmt *);
Prototype void GenReturn(ReturnStmt *);
Prototype void GenBreakPoint(BreakPointStmt *);
Prototype void GenExp(ExpStmt *);
Prototype void GenExpResult(ExpStmt *);
Prototype int IsRegCall(int32_t);
Prototype void DebugLine(int32_t);

int32_t
AllocLabel()
{
    static int32_t Label = 0;

    return(++Label);
}

void
InitGen(int32_t enab)
{
    static short Refs;

    if (enab == 1 && Refs++ == 0) {
        ;
    }
    if (enab == 0 && --Refs == 0) {
        ;
    }
}

/*
 *  This routine generates a top level declaration which includes all
 *  subroutine definitions.
 */

void
GenerateVar(var)
Var *var;
{
    Assert(var->Type);
    var->Next = NULL;
    BlockAddTop(var);

    if (var->Type->Id == TID_PROC) {
        AllocExternalStorage(var->Sym, &var->var_Stor, var->Type, var->Flags);

        if (var->u.Block == NULL) {     /*  reference   */
            ;
        } else {                        /*  definition  */
            int32_t opos;
            BlockStmt *block = var->u.Block;

            asm_segment(&DummyCodeVar);
            ProcVar = var;

            GenPass = 0;
            ForceLinkFlag = 0;
            RefCmpNext = 1;

            /*
             *  determine regcall
             */

#ifdef OLDREGCALL
            if (IsRegCall(ProcVar->Flags)) {
                if (RegCallOpt == 1)
                    ProcVar->Flags |= TF_REGCALL | TF_STKCALL;
                if (RegCallOpt >= 2)
                    ProcVar->Flags |= TF_REGCALL;
            } else {
                ProcVar->Flags &= ~TF_REGCALL;
            }
#endif

            /*
             *  Ensure there is a debug line delimiting procedures
             */

            if (DebugOpt)
                DebugLine(var->LexIdx);

            /*
             *  pass 0 must occur before procedure header output because
             *  -ms const items generated in pass 0 (and cannot intefere with
             *  normal code).  Also, code alignment must occur after pass 0
             *  for the same rason.... improperly aligned const data must be
             *  rectified.
             */


            (*var->u.Block->st_Func)(var->u.Block);

            asm_align(CodeAlign);

            asm_procbegin(var);

            opos = ftell(stdout);

            SaveLastLexIdxBeg = LastLexIdxBeg;
            SaveLastLexLine   = LastLexLine;

            do {
                RefCmp = RefCmpNext;
                RefCmpNext = 32767;
                dbprintf(("; REDO %d RECMP %d\n", GenPass, RefCmp));

                ARegRecov= 0;   /*  registers recovered */
                DRegRecov= 0;
                LastLexIdxBeg = SaveLastLexIdxBeg;
                LastLexLine   = SaveLastLexLine;

                /*
                 *  reset registers reserved
                 */

                RegReserved = REGSDPTR | GlobalRegReserved;
                if (SmallData == 2)
                    RegReserved &= ~REGSDPTR;

                if (GenPass) {
                    ++RetryCount;
                    if (GenPass > 64 || (GenPass > 4 && RefCmp == 32767))
                    {
                        dbprintf(("Too many redos (%d) RefCmp=%d", GenPass, RefCmp));
                        zerror(EFATAL_TOO_MANY_REDOS);
                    }
                }
                if (DDebug == 0) {
                    if (fseek(stdout, opos, 0) < 0 && GenPass)
                        puts("; ------- RETRY (seek failed)");
                }
                ResetRegAlloc();
                ++GenPass;

                (*var->u.Block->st_Func)(var->u.Block);
            } while (TooManyRegs() || block->Frame.SubARegOver + block->Frame.CurARegOver || block->Frame.SubDRegOver + block->Frame.CurDRegOver);

            GenPass = 0;
            asm_procend(var, ForceLinkFlag);

            /*
             * Handle procedure termination debug synchronization
             */

            if (DebugOpt)
                DebugLine(block->LastLexIdx);
        }
    } else {                            /*  top level decl  */
        if (var->Flags & TF_STATIC)
            AllocStaticStorage(var->Sym, &var->var_Stor, var->Type, var->Flags);
        else
            AllocExternalStorage(var->Sym, &var->var_Stor, var->Type, var->Flags);
        if ((var->Flags & TF_EXTERN) == 0) {
            if ((var->Flags & TF_STATIC) == 0)
                asm_export(var->Sym);
            asm_segment(var);
            GenStaticData(var);
        }
    }
}

#ifdef OLDREGCALL

/*
 *  IsRegCall()
 */

IsRegCall(flags)
int32_t flags;
{
#ifdef REGISTERED
    if (flags & TF_DOTDOTDOT)
        return(0);
    if (flags & TF_REGCALLOK) {
        if (flags & TF_REGCALL)
            return(1);
        if (RegCallOpt && !(flags & TF_STKCALL))
            return(1);
    }
#endif
    return(0);
}

#endif


/*
 *  GenBlock
 *
 *  Register allocation of variables is done bottom up, and only for variables
 *  which are referenced more then twice.  Registered variables are re-registered
 *  in the generation pass.
 *
 *  ARegRecov & DRegRecov only applies to lower level frames so we must save
 *  the current values when we call-down
 */

void
GenBlock(block)
BlockStmt *block;
{
    short lastARegOver = block->Frame.CurARegOver + block->Frame.SubARegOver;
    short lastDRegOver = block->Frame.CurDRegOver + block->Frame.SubDRegOver;

    GenPush(block);

    if (GenPass == 0) {
        Var *var;
        Var **pvar = &block->VarBase;

        /*
         *  non-stack declarations
         */

        while ((var = *pvar) != NULL) {
            if (var->Type->Id == TID_PROC) {
                Assert(0);
            }
            if ((var->Flags & TF_AUTO) == 0) {
                if (var->Flags & TF_STATIC) {
                    AllocStaticStorage(var->Sym, &var->var_Stor, var->Type, var->Flags);
                } else {
                    AllocExternalStorage(var->Sym, &var->var_Stor, var->Type, var->Flags);
                }
                if (var->Flags & TF_STATIC) {
                    asm_segment(var);
                    GenStaticData(var);
                    asm_segment(&DummyCodeVar);
                }
                *pvar = var->Next;
            } else {
                pvar = &var->Next;
            }
        }
    } else {
        Var *var;
        char argno[16];
        char prgno[16];
        int32_t argnoMask = 0;
        short i;
        short refcmp;
        PragNode *pragma_call = TestPragmaCall(ProcVar, prgno);

        refcmp = RefCmp;
        if (block->Bid == BT_PROC && ((ProcVar->Flags & TF_REGCALL) || pragma_call)) {
            RegCallOrder(ProcVar->Type, argno, (pragma_call) ? prgno : NULL);
            if (refcmp == 1)
                refcmp = 0;
            for (i = 0; i < 16 && argno[i] >= 0; ++i)
                argnoMask |= 1 << argno[i];
        }

        /*
         *  stack declarations.  We attempt to place all variables that meet
         *  the requirements into registers.  As more passes are taken, the
         *  number of variables placed in registers become fewer until the
         *  optimal case occurs.
         *
         *  step 1:  if too many register variables reduce the number.
         */

        dbprintf(("; ARCHK: %d > %d, %d > %d\n",
            lastARegOver,
            ARegRecov,
            lastDRegOver,
            DRegRecov
        ));

        if (lastARegOver > ARegRecov || lastDRegOver > DRegRecov) {
            for (var = block->VarBase; var; var = var->Next) {
                /*
                 *  if registerable
                 */

                dbprintf(("; %s %d,%d\n", SymToString(var->Sym), var->Refs, refcmp));

                /*
                 *  Ignore registerized arguments for A4 and A5, they MUST
                 *  be moved to a register before the link and/or __geta4
                 *  data segment load
                 */

                if ((var->RegFlags & RF_REGISTER) && ((1 << (var->RegFlags & RF_REGMASK)) & (RF_A4|RF_A5)))
                    continue;

                if (var->Type->Id == TID_INT && !(var->Flags & VF_ADDR)) {
                    if (lastDRegOver > DRegRecov) {
                        if (var->Refs <= refcmp) {
                            var->Flags |= VF_ADDR;
                            ++DRegRecov;
                        }
                    }
                } else if ((var->Type->Id == TID_PTR || ((var->Flags & VF_ARG) && var->Type->Id == TID_ARY)) && !(var->Flags & (TF_VOLATILE|TF_ALIGNED|VF_ADDR))) {
                    if (lastARegOver > ARegRecov) {
                        if (var->Refs <= refcmp) {
                            var->Flags |= VF_ADDR;
                            ++ARegRecov;
                        }
                    }
                }
            }
        }

        /*
         *  step 2, handle declarations & next-ref
         */

        for (i = 0, var = block->VarBase; var; (var = var->Next), ++i) {
            var->var_Stor.st_Type = 0;

            if ((var->Type->Id == TID_PTR || var->Type->Id == TID_INT || ((var->Flags & VF_ARG) && var->Type->Id == TID_ARY)) && !(var->Flags & (TF_VOLATILE|TF_ALIGNED|VF_ADDR))) {
                int32_t r;

                if (RefCmpNext > var->Refs && var->Refs > RefCmp)      /*  minimum next    */
                    RefCmpNext = var->Refs;

                /*
                 *  note that if AllocRegVarStorageReq() is able to allocate
                 *  the variable in its passed parameter AND the variable
                 *  is never modified or &of then the register will be added
                 *  to the RegReserved list to prevent it from being saved
                 *  or restored.
                 */

                if (block->Bid == BT_PROC && ((ProcVar->Flags & TF_REGCALL) || pragma_call))
                    r = AllocRegVarStorageReq(var, argno[i], argnoMask);
                else
                    r = AllocRegVarStorage(var);

                if (r == 0) {
                    dbprintf(("; ARO %s\n", SymToString(var->Sym)));
                    if (var->Type->Id == TID_PTR || var->Type->Id == TID_ARY)
                        ++block->Frame.CurARegOver;
                    else
                        ++block->Frame.CurDRegOver;
                }
            }

            /*
             *  Register variable.  Use rel-a7 for arguments-made-registers
             *                      unless LWP (where A7 is invalid for var
             *                      ref)
             *
             *  if procedure TF_REGCALL, instead of moving the vars from
             *  the stack, move them from other regs (argno)
             */

            if (var->var_Stor.st_Type == ST_Reg) {
                dbprintf(("; REGVAR: %s %d R%d\n", SymToString(var->Sym), var->Refs, var->var_Stor.st_RegNo));
                if (var->Flags & VF_ARG) {
                    Stor t;

                    Assert(block->Bid == BT_PROC);

                    if ((ProcVar->Flags & TF_REGCALL) || (pragma_call)) {
                        short swreg;

                        /*
                         *  A register-register move may involve an EXG.  If this
                         *  occurs we must force another pass and invalidate
                         *  them
                         *
                         */

                        DummyExp.ex_LexIdx = var->LexIdx;
                        if ((swreg = asm_rcomove(&DummyExp, argno, i, &var->var_Stor)) >= 0) {
                            if ((ProcVar->Type->Vars[i]->RegFlags & RF_MODIFIED) == 0) {
                                ProcVar->Type->Vars[i]->RegFlags |= RF_MODIFIED;
                                RegFlagTryAgain();
                            }
                            if ((ProcVar->Type->Vars[swreg]->RegFlags & RF_MODIFIED) == 0) {
                                ProcVar->Type->Vars[swreg]->RegFlags |= RF_MODIFIED;
                                RegFlagTryAgain();
                            }
                        }
                    } else {
                        AllocArgsStorage(&t, var->Type, 0, var->Flags);
                        DummyExp.ex_LexIdx = var->LexIdx;
                        asm_move(&DummyExp, &t, &var->var_Stor);
                    }
                }
                continue;
            }

            if (var->Flags & VF_ARG) {
                if (!((ProcVar->Flags & TF_REGCALL) || pragma_call))
                    AllocArgsStorage(&var->var_Stor, var->Type, 1, var->Flags);
            } else {
                AllocStackStorage(&var->var_Stor, var->Type, var->Flags);
                var->var_Stor.st_Flags &= ~SF_TMP;
                var->var_Stor.st_Flags |= SF_VAR;
            }
        }

        /*
         *  handle linking for top level of a procedure.  Also handle any
         *  arguments that could not be placed in registers.
         */

        if (block->Bid == BT_PROC) {
            asm_proclink(ProcVar);      /*  link stmt   */

            if ((ProcVar->Flags & TF_REGCALL) || pragma_call) {
                for (i = 0, var = block->VarBase; var; (var = var->Next), ++i) {
                    if (var->var_Stor.st_Type != ST_Reg) {
                        if (var->Refs) {
                            AllocStackStorage(&var->var_Stor, ActualPassType(ProcVar->Type, var->Type, 0), var->Flags);
                            var->var_Stor.st_Flags &= ~SF_TMP;
                            var->var_Stor.st_Flags |= SF_VAR;

                            DummyExp.ex_LexIdx = var->LexIdx;
                            asm_rcomove(&DummyExp, argno, i, &var->var_Stor);
                        } else {
                            argno[i] = -1;
                        }
                    }
                }
            }
        }
    }
    {
        short aRegRecov = ARegRecov;
        short dRegRecov = DRegRecov;
        {
            Stmt *stmt;

            for (stmt = block->Base; stmt; stmt = stmt->st_Next) {
                if (GenPass && DebugOpt) {
                    DebugLine(stmt->st_LexIdx);
                }
                (*stmt->st_Func)(stmt);
            }
        }
        ARegRecov = aRegRecov;
        DRegRecov = dRegRecov;
    }
    /*
     *  Free Register Variables and warn against unused variables.
     *  We set VF_DECLD to prevent multiple warnings for the same
     *  variable when multiple generation passes are incurred.
     *
     *  We do not warn of unused procedure arguments as this might
     *  be perfectly valid
     */
    {
        Var *var;

        for (var = block->VarBase; var; var = var->Next) {
            if (GenPass && (var->Flags & VF_DECLD) == 0) {
                var->Flags |= VF_DECLD;
                if (block->Bid != BT_PROC)
                    yerror(var->LexIdx, EWARN_VARIABLE_NOT_USED, var->Sym->Len, var->Sym->Name);
            }
            if (var->var_Stor.st_Type == ST_Reg && (var->var_Stor.st_Flags & SF_VAR)) {
                var->var_Stor.st_Flags |= SF_TMP;
                FreeStorage(&var->var_Stor);
            }
        }
    }
    GenPop(block);
}

void
GenFor(stmt)
ForStmt *stmt;
{
    if (GenPass == 0) {
        SubStmtCall(Stmt1);
        SubStmtCall(Stmt2);
        SubStmtCall(Stmt3);
        SubStmtCall(Stmt4);
    } else {
        SubStmtCall(Stmt1);     /*  initialize*/
        if (stmt->Stmt2)
            asm_branch(stmt->LabelBegin);
        asm_label(stmt->Block->LabelLoop);
        SubStmtCall(Stmt4);     /*  body      */
        asm_label(stmt->Block->LabelTest);
        SubStmtCall(Stmt3);     /*  increment */
        if (stmt->Stmt2) {
            asm_label(stmt->LabelBegin);
            SubStmtCall(Stmt2);
        } else {
            asm_branch(stmt->Block->LabelLoop);
        }
        asm_label(stmt->Block->LabelBreak);
    }
}

void
GenWhile(stmt)
WhileStmt *stmt;
{
    if (GenPass == 0) {
        SubStmtCall(Stmt1);
        SubStmtCall(Stmt2);
    } else {
        if (stmt->Stmt1)
            asm_branch(stmt->Block->LabelTest);
        asm_label(stmt->Block->LabelLoop);
        SubStmtCall(Stmt2);
        asm_label(stmt->Block->LabelTest);
        SubStmtCall(Stmt1);
        asm_label(stmt->Block->LabelBreak);
    }
}

void
GenDo(stmt)
DoStmt *stmt;
{
    if (GenPass == 0) {
        SubStmtCall(Stmt1);
        SubStmtCall(Stmt2);
    } else {
        asm_label(stmt->Block->LabelLoop);
        SubStmtCall(Stmt1);
        asm_label(stmt->Block->LabelTest);
        SubStmtCall(Stmt2);
        asm_label(stmt->Block->LabelBreak);
    }
}

void
GenIf(stmt)
IfStmt *stmt;
{
    if (GenPass == 0) {
        SubStmtCall(Stmt1);
        SubStmtCall(StmtT);
        SubStmtCall(StmtF);
    } else {
        SubStmtCall(Stmt1);
        asm_label(stmt->LabelIf);
        SubStmtCall(StmtT);
        if (stmt->StmtF)
            asm_branch(stmt->LabelEnd);
        asm_label(stmt->LabelElse);
        SubStmtCall(StmtF);
        asm_label(stmt->LabelEnd);
    }
}

void
GenSwitch(stmt)
SwitchStmt *stmt;
{
    short i;
    int32_t deflabel;

    if (GenPass == 0) {
        SubStmtCall(BeforeBlock);
        SubStmtCall(Stmt1);
        for (i = 0; i < stmt->NumCases; ++i) {
            SubStmtCall(CaseAry[i]);
        }
        SubStmtCall(DefBlock);
    } else {
        int32_t i;

        /*
         *  note, asm_switch() sorts cases & labels arrays
         */

        SubStmtCall(Stmt1);
        deflabel = (stmt->DefBlock) ? stmt->DefBlock->LabelTest : stmt->Block->LabelBreak;
        FreeStorage(&((ExpStmt *)stmt->Stmt1)->Expr->ex_Stor);
        asm_switch(((ExpStmt *)stmt->Stmt1)->Expr, stmt->NumCases, stmt->Cases, stmt->Labels, deflabel);

        SubStmtCall(BeforeBlock);
        for (i = 0; i < stmt->NumCases; ++i) {
            if (i == stmt->DefCaseNo && stmt->DefBlock) {
                asm_label(stmt->DefBlock->LabelTest);
                SubStmtCall(DefBlock);
            }
            asm_label(stmt->CaseAry[i]->LabelTest);
            SubStmtCall(CaseAry[i]);
        }
        if (i == stmt->DefCaseNo && stmt->DefBlock) {
            asm_label(stmt->DefBlock->LabelTest);
            SubStmtCall(DefBlock);
        }
        asm_label(stmt->Block->LabelBreak);
    }
}

void
GenBreak(stmt)
BreakStmt *stmt;
{
    if (GenPass)
        asm_branch(stmt->BreakLabel);
}

void
GenContinue(stmt)
ContinueStmt *stmt;
{
    if (GenPass)
        asm_branch(stmt->ContLabel);
}

void
GenGoto(stmt)
GotoStmt *stmt;
{
    int32_t labelid;

    if (GenPass) {
        labelid = (int32_t)(intptr_t)stmt->GotoLabel->Data;
        if (labelid == 0)
        {
            yerror(stmt->st_LexIdx, EERROR_GOTO_LABEL_NOT_FOUND,
                        stmt->GotoLabel->Sym->Len,
                        stmt->GotoLabel->Sym->Name);
        }
        asm_branch(labelid);
    }
}

void
GenLabel(stmt)
LabelStmt *stmt;
{
    if (GenPass) {
        asm_label(stmt->Label);
    }
    SubStmtCall(Stmt1);
}

/*
 *  note that we cannot propogate the result storage (D0) to the sub
 *  expression because it might contain a call.
 */

void
GenReturn(stmt)
ReturnStmt *stmt;
{
    ExpStmt *stmt1;
    Exp *exp;

    if (GenPass == 0) {
        if (*ProcVar->Type->SubType->Size == 0) {
            if ((stmt1 = (ExpStmt *)stmt->Stmt1) && (exp = stmt1->Expr))
                exp->ex_Flags |= EF_RNU;
        }
        SubStmtCall(Stmt1);
        if ((stmt1 = (ExpStmt *)stmt->Stmt1) && (exp = stmt1->Expr)) {
            Type *retType = ActualReturnType((Stmt *)stmt1, ProcVar->Type, ProcVar->Type->SubType);

            Assert(exp->ex_Type);
            Assert(ProcVar);
            Assert(ProcVar->Type);

            if (exp->ex_Type != ProcVar->Type->SubType) {
                if (*retType->Size == 0)
                    yerror(stmt->st_LexIdx, EERROR_ILLEGAL_RETURN_TYPE);
                InsertCast(&stmt1->Expr, ProcVar->Type->SubType);
            }
            if (ProcVar->Type->SubType != retType)
                InsertCast(&stmt1->Expr, retType);
        }
    } else {
        if ((stmt1 = (ExpStmt *)stmt->Stmt1) && (exp = stmt1->Expr)) {
            SubStmtCall(Stmt1);

            /*
             *  Don't bother to return void values
             */

            if ((exp->ex_Flags & EF_RNU) == 0 && exp->ex_Stor.st_Size) {
                FreeStorage(&exp->ex_Stor);
                asm_returnstorage(exp);
            }
        }
        asm_branch(LabelReturn);
    }
}

void
GenBreakPoint(stmt)
BreakPointStmt *stmt;
{
    if (GenPass == 0) {
        ;
    } else {
        asm_illegal();
    }
}

void
GenExp(stmt)
ExpStmt *stmt;
{
    if (GenPass == 0) {
        stmt->Expr->ex_Flags |= EF_RNU;
        (*stmt->Expr->ex_Func)(&stmt->Expr);
    } else {
        if (DebugOpt)
            DebugLine(stmt->Expr->ex_LexIdx);
        (*stmt->Expr->ex_Func)(&stmt->Expr);  /*  last arg illegal ptr */
    }
}

void
GenExpResult(stmt)
ExpStmt *stmt;
{
    if (GenPass == 0) {
        (*stmt->Expr->ex_Func)(&stmt->Expr);
    } else {
        if (DebugOpt)
            DebugLine(stmt->Expr->ex_LexIdx);
        (*stmt->Expr->ex_Func)(&stmt->Expr);
    }
}

/*
 *  DEBUGLINE()
 *
 *  Generate debugging info (-d) and commented assembly (-d -a).  To
 *  supply a readable result do not reverse index.  This occurs for
 *  for() loops, for example, due to the conditional being at the bottom
 *  of the loop.
 */

void
DebugLine(lexIdx)
int32_t lexIdx;
{
    int32_t lexIdxBeg;
    char *lexFile;

    if (lexIdx == 0)
        return;

    if (lexIdx >= LastLexIdxBeg) {
        {
            int32_t lexFileNameLen;
            int32_t lexLine = FindLexFileLine(lexIdx, &lexFile, &lexFileNameLen, &lexIdxBeg);
            if (lexLine <= LastLexLine)
                return;
            LastLexLine = lexLine;
            printf("\tdebug\t%d\n", lexLine);
        }

        if (AsmOnlyOpt) {
            int32_t i;
            short c;

            /*
             *  Ignore precomp garbage leading up to the first line
             */

            if (LastLexIdxBeg)
                lexIdxBeg = LastLexIdxBeg;

            /*
             *  Insert C source as comments beginning at LastLexIdx and moving
             *  towards lexIdx
             */

            printf("; ");

            for (i = lexIdxBeg; (c = FindLexCharAt(i)) != EOF; ++i) {
                fputc(c, stdout);
                if (c == '\n') {
                    if (i >= lexIdx)
                        break;
                    printf("; ");
                }
            }
            fputc('\n', stdout);
            lexIdxBeg = i;

        } else {
            ++lexIdxBeg;
        }
        LastLexIdxBeg = lexIdxBeg;
    }
}


