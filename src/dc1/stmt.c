/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  STMT.C
 *
 *  Parse procedural junk: declarations, statements, etc...
 */

/*
**      $Filename: stmt.c $
**      $Author: dice $
**      $Revision: 30.152 $
**      $Date: 1995/01/08 22:30:25 $
**      $Log: stmt.c,v $
 * Revision 30.152  1995/01/08  22:30:25  dice
 *  test message
 *
 * Revision 30.5  1994/06/13  18:37:37  dice
 * .
 *
 * Revision 30.0  1994/06/10  18:04:57  dice
 * .
 *
 * Revision 1.8  1994/04/15  21:18:54  jtoebes
 * Handle labels before the start of a case clause properly.
 *
 * Revision 1.7  1993/10/17  04:03:38  jtoebes
 * Eliminate compiler warning about unused variable.
 *
 * Revision 1.6  1993/10/15  22:44:24  jtoebes
 * Eliminated old code which called cerror.
 *
 * Revision 1.5  1993/09/19  13:14:10  jtoebes
 * Fixed BUG00148 - Compiler does not catch gotos to non-existent label.
 * Changed intermediate gotolabel information to know more about the label.
 * Also added code to identify when more than one label has been entered.
 *
 * Revision 1.4  1993/09/11  16:12:41  jtoebes
 * Fixed BUG01010 - Code not allowed in switch statement before the first case.
 * Added code to catch the code before the first case statement and store it
 * to be emitted later.
 *
 * Revision 1.3  1993/09/06  22:18:01  jtoebes
 * Fixed BUG06004 - DC1 fails to detect reuse of a register in a function parameter
 * list.
 *
**/

#include "defs.h"

Prototype short CompProcedureArgDeclarators(short, Var ***, int32_t *, int32_t *);
Prototype short CompProcedure(short, Var *);
Prototype short CompStmtDeclExp(short, Stmt **, int32_t);
Prototype short CompBlock(short, Stmt **);
Prototype short CompFor(short, Stmt **);
Prototype short CompWhile(short, Stmt **);
Prototype short CompDo(short, Stmt **);
Prototype short CompIf(short, Stmt **);
Prototype short CompSwitch(short, Stmt **);
Prototype short CompBreak(short, Stmt **);
Prototype short CompContinue(short, Stmt **);
Prototype short CompGoto(short, Stmt **);
Prototype short CompLabel(short, Stmt **);
Prototype short CompReturn(short, Stmt **);
Prototype short CompBreakPoint(short, Stmt **);

/*
 *  Deals with argument declarations to a procedure.  Does NOT compile
 *  the procedure itself.
 *
 *  Determines registerization, if any, for both externs and procedure
 *  definitions.
 *
 * void foo ( {START} a, b, c, d )   <more>
 * void foo ( {START} short, short)   <more>
 * void foo ( {START} short a, short b)  <more>
 *
 *  more =  old style declarators (which I like much better than proto style anyway),
 *          optional .. could be ';' if just a prototype.  If a procedure
 *          definition then returns TokSemi.
 */

short
CompProcedureArgDeclarators(short t, Var ***pvars, int32_t *pargs, int32_t *pflags)
{
    Var **vars = NULL;
    short i;
    short n = -1;
    short siz = 0;
    short oldState = State;

    State = SARG;

    *pargs = 0;
    *pvars = NULL;
    *pflags = 0;

    if (t == TokDotDotDot) {
        *pflags |= TF_DOTDOTDOT | TF_PROTOTYPE;
        t = GetToken();
        if (t != TokRParen)
            zerror(EERROR_SYNTAX_ERROR_DECL);
        n = 0;
    }

    while (t != TokRParen) {
        Type *baseType;
        Type *type;
        Var *var;
        Symbol *sym = NULL;
        int32_t baseFlags;
        int32_t regFlags;

        if (n < 0)
            n = 0;

        /*
         *  If not an ID we assume it is a type declaration and thus this
         *  is a prototype.
         */

        if (t != TokId && t != TokVarId && t != TokEnumConst)
            *pflags |= TF_PROTOTYPE;

        t = CompType(t, &baseType, &baseFlags, &regFlags);      /*  defaults to int if none */
        type = baseType;
        t = CompTypeDeclarators(t, &type, &sym, baseFlags);
        if (regFlags & RF_REGISTER)
        {
            int i;

            *pflags |= TF_REGCALL;
            /* Ensure that the register is not used by any other parameter */
            for(i = 0; i < n; i++)
            {
                if (vars[i]->RegFlags == regFlags)
                {
                    zerror(EERROR_ILLEGAL_REGSPEC);
                    regFlags = 0;
                }
            }
        }

        if (type == &VoidType) {
            if (n)
                zerror(EERROR_SYNTAX_ERROR_DECL);
            if (t != TokRParen)
                zerror(EERROR_SYNTAX_ERROR_DECL);
            break;
        }

        if (n == siz) {
            siz += 4;
            vars = zrealloc(vars, sizeof(Var *), n, siz);
        }

        /*
         *  argument
         */

        var = AllocStructure(Var);
        var->Type = type;
        var->Sym  = sym;
        var->Flags = (baseFlags & TF_STORQUALMASK) | type->Flags | (VF_ARG | TF_AUTO);
        var->RegFlags = regFlags;

        if (type->Id != TID_INT && type->Id != TID_PTR && type->Id != TID_ARY)
            *pflags |= TF_STKCALL;

        if (baseFlags & (TF_STATIC|TF_EXTERN))
            zerror(EERROR_ILLEGAL_QUALIFIER);

        /*
         *  Bump the refs for the 'register' keyword to give the variable
         *  a better chance of being put in a register.
         */

        if (var->Flags & TF_REGISTER)
            ++var->Refs;

        vars[n++] = var;

        if (t != TokRParen && t != TokComma) {
            zerror(EERROR_SYNTAX_ERROR_DECL);
            goto bad;
        }
        if (t == TokComma) {
            t = GetToken();
            if (t == TokDotDotDot) {
                *pflags |= TF_DOTDOTDOT;
                t = GetToken();
                if (t != TokRParen)
                    zerror(EERROR_SYNTAX_ERROR_DECL);
            }
        }
    }
    *pargs = n;
    *pvars = vars;

    /*
     *  have right paren, now is this a real procedure or just a declaration
     *  of some sort?
     */

    t = GetToken();

    if (t == TokComma || t == TokEq || t == TokSemi || t == TokRParen) {
        if (!(*pflags & TF_PROTOTYPE))
            *pflags |= TF_STKCALL;
        goto skip;
    }

    /*
     *  Else a real procedure.. handle old-style argument decls.  We return
     *  when we get a '{'.  Also make sure all the variables are named.
     */

    for (i = 0; i < n; ++i) {
        Var *var = vars[i];

        if (var->Sym == NULL) {
            zerror(EERROR_ID_MISSING_PROC);
            var->Sym = MakeSymbol("____dummy", 9, TokId, 0);
            /* goto bad; */
            /* break; */
        }
    }

    while (t != TokLBrace) {
        Type *baseType;
        Type *type;
        Var *var = NULL;
        Symbol *sym = NULL;     /*  XXX needed? */
        int32_t  baseFlags;
        int32_t  regFlags;
        int32_t  li = LFBase->lf_Index;

        t = CompType(t, &baseType, &baseFlags, &regFlags);     /*  defaults to int if none */

        for (;;) {
            type = baseType;
            t = CompTypeDeclarators(t, &type, &sym, baseFlags);

            for (i = 0; i < n; ++i) {
                var = vars[i];
                if (var->Sym == sym)
                    break;
            }
            if (i == n) {
                zerror(EERROR_ID_NOT_IN_LIST);
            } else {
                var->Type = type;
                var->Flags = (baseFlags & TF_STORQUALMASK) | type->Flags | (VF_ARG | TF_AUTO);
                var->RegFlags = regFlags;
                if (var->Flags & TF_REGISTER)
                    ++var->Refs;
                if (regFlags & RF_REGISTER)
                    *pflags |= TF_REGCALL;
            }
            if (type->Id != TID_INT && type->Id != TID_PTR && type->Id != TID_ARY)
                *pflags |= TF_STKCALL;

            if (t == TokComma) {
                t = GetToken();
                continue;
            }
            break;
        }
        if (t != TokSemi) {
            zerror(EERROR_EXPECTED_SEMICOLON);
            if (li == LFBase->lf_Index)     /*  prevent inifinite loop  */
                t = GetToken();
            break;
        }
        t = GetToken();
    }

skip:

    /*
     *  If explicitly registerized then clear TF_STKCALL bit (which gets
     *  set when automatic registerization is not possible)
     */

    if (*pflags & TF_REGCALL)
        *pflags &= ~TF_STKCALL;

bad:
    State = oldState;
    return(t);
}

/*
 *  Compiles a procedure, t had better be TokLBrace.
 *
 *  returns TokSemi to simply toplevel.c
 *
 *  Normally assigns in decls work as follows:  the variable is added to the
 *  current block's variable list and the decl-stmt is added to the current
 *  block's statement list (which retains placement).
 */

short
CompProcedure(short t, Var *var)
{
    Stmt *stmt;
    short oldState = State;
    int32_t lexIdx = LFBase->lf_Index;

    if (t != TokLBrace)
        zerror(EERROR_EXPECTED_OCBRACE_PROC);

    State = SARG;           /*  first properly add args */
    var->u.Block = BlockDown(BT_PROC);

    {
        short i;
        Type *type = var->Type;

        for (i = 0; i < type->Args; ++i) {
            Var *xvar = type->Vars[i];

            BlockAddVar(xvar);
            Assert(xvar->Sym);
            SemanticAdd(xvar->Sym, TokVarId, xvar);
        }
    }

    State = SINSIDE;
    t = CompBlock(GetToken(), &stmt);
    BlockAddStmt(stmt);

    State = SARG;
    BlockUp();
    State = SOUTSIDE;

    /*
     *  t should be TokRBrace.. return TokSemi instead
     */

    if (t != TokRBrace)
        yerror(lexIdx, EERROR_EXPECTED_OCBRACE_PROC);

    State = oldState;
    return(TokSemi);
}

short
CompStmtDeclExp(short t, Stmt **pstmt, int32_t semiexp)
{
    *pstmt = NULL;

    switch(t) {
    case TokLBrace:
        t = CompBlock(GetToken(), pstmt);
        t = GetToken();
        break;
    case TokBreak:
        t = CompBreak(GetToken(), pstmt);
        break;
    case TokCase:
        zerror(EERROR_CASE_DEFAULT_OUTSIDE);
        t = GetToken();
        break;
    case TokContinue:
        t = CompContinue(GetToken(), pstmt);
        break;
    case TokDefault:
        zerror(EERROR_CASE_DEFAULT_OUTSIDE);
        t = GetToken();
        break;
    case TokDo:
        BlockCost += 2;
        t = CompDo(GetToken(), pstmt);
        BlockCost -= 2;
        break;
    case TokElse:
        zerror(EERROR_ELSE_NO_IF);
        t = GetToken();
        break;
    case TokFor:
        BlockCost += 2;
        t = CompFor(GetToken(), pstmt);
        BlockCost -= 2;
        break;
    case TokGoto:
        t = CompGoto(GetToken(), pstmt);
        break;
    case TokIf:
        t = CompIf(GetToken(), pstmt);
        break;
    case TokReturn:
        t = CompReturn(GetToken(), pstmt);
        break;
    case TokSwitch:
        t = CompSwitch(GetToken(), pstmt);
        break;
    case TokWhile:
        BlockCost += 2;
        t = CompWhile(GetToken(), pstmt);
        BlockCost -= 2;
        break;
    case TokBreakPoint:
        t = CompBreakPoint(GetToken(), pstmt);
        break;
    case TokSemi:
        if (semiexp)
            t = GetToken();
        break;

    /*
     *  declaration
     */

    case TokStruct:
    case TokEnum:
    case TokUnion:
    case TokTypeQual:
    case TokTypeDef:
    case TokTypeId:
    case TokTypeof:
        {
            Var *var = NULL;
            BlockStmt *block = NULL;

            t = CompDecl(t, &var, semiexp);

            /*
             *  add variables to the current block, any procedure decls
             *  will be moved to the top level.  Returns base of list
             *  of newly added variables to the current block (minus any
             *  procdures added to the top)
             */

            if (var)
                var = BlockAddVar(var);

            if (var && var->Next)              /*  more 'n one!    */
                block = BlockDown(BT_BLOCK);

            while (var) {               /*  for each...     */
                if (var->Type->Id != TID_PROC && var->u.AssExp && (var->Flags & TF_AUTO)) {
                    ExpStmt *es = AllocTmpStructure(ExpStmt);

                    es->st_Func = (void (*)(void *))GenExp;
                    es->st_Tok  = TokExp;
                    es->st_LexIdx = LFBase->lf_Index;
                    InsertAssign(&var->u.AssExp, var);
                    es->Expr = var->u.AssExp;
                    ++var->Refs;
                    if (block)
                        BlockAddStmt((Stmt *)es);
                    else
                        *pstmt = (Stmt *)es;
                }
                var = var->Next;
            }
            if (block) {
                *pstmt = (Stmt *)block;
                BlockUp();
            }
        }
        break;
    case TokLabelId:
        Assert(0);
        /* t = CompLabel(t, pstmt); */
        break;
    case TokId:     /*  label or subroutine call ?    */
    case TokVarId:  /*  label or variable ?           */
        if (LexHackColon) {     /*  label (lexical hack checks for a :) */
            t = CompLabel(t, pstmt);
            break;
        }
        /* fall through to exp */
    default:

        {
            Exp *exp = NULL;
            ExpStmt *es = AllocTmpStructure(ExpStmt);
            int32_t li = LFBase->lf_Index;

            t = CompExp(t, &exp, 1);
            if (semiexp) {
                if (t == TokSemi) {
                    t = GetToken();
                } else {
                    zerror(EWARN_EXPECTED_SEMICOLON);
                    if (LFBase->lf_Index == li)  /*  prevent endless loop */
                        t = GetToken();
                }
            }
            if (exp == NULL)
                zerror(EFATAL_SYNTAX_ERROR_EXP);

            es->st_Func = (void (*)(void *))GenExp;
            es->st_Tok  = TokExp;
            es->st_LexIdx = exp->ex_LexIdx;
            es->Expr = exp;
            *pstmt = (Stmt *)es;
        }
        break;
    }
    return(t);
}

/*
 *  CompBlock() works slightly differently in that it returns t = TokRBrace,
 *  allowing toplevel.c to simply throw away t (makes the code easier)
 *
 *  All Comp*() for statements have access to their parent blocks... they
 *  need not set it (it's done automatically on return) UNLESS they make
 *  calls to other Comp*() statements.
 *
 *  Attempt to set block->LastLexIdx for debug info synchronization to just
 *  before the close brace.  This does not always work perfectly (but does
 *  not cause a screwup in those cases either).
 */

short
CompBlock(short t, Stmt **pstmt)
{
    BlockStmt *block = BlockDown(BT_BLOCK);

    *pstmt = (Stmt *)block;

    while (t != TokRBrace) {
        Stmt *stmt;

        t = CompStmtDeclExp(t, &stmt, 1);
        BlockAddStmt(stmt);
        if (!t)
            break;
    }
    BlockUp();

    return(t);
}

/*
 *  for (stmt ; exp; stmt) stmt
 *
 *  Note: The Block junk is to add another variable level allowing
 *  temporary declarations within the for.. for (int i = 0; ...)
 */

short
CompFor(short t, Stmt **pstmt)
{
    ForStmt *stmt = AllocTmpStructure(ForStmt);
    BlockStmt *block = BlockDown(BT_FOR);

    *pstmt = (Stmt *)block;
    stmt->Block = block;
    block->LabelLoop  = AllocLabel();
    block->LabelTest  = AllocLabel();
    block->LabelBreak = AllocLabel();


    BlockAddStmt((Stmt *)stmt);

    stmt->st_Func = (void (*)(void *))GenFor;
    stmt->st_Tok  = TokFor;
    stmt->st_LexIdx = LFBase->lf_Index;
    stmt->LabelBegin = AllocLabel();

    t = SkipToken(t, TokLParen);

    if (t != TokSemi)
        t = CompStmtDeclExp(t, &stmt->Stmt1, 0);
    t = SkipToken(t, TokSemi);
    if (t != TokSemi)
        t = CompStmtDeclExp(t, &stmt->Stmt2, 0);
    t = SkipToken(t, TokSemi);
    if (t != TokRParen)
        t = CompStmtDeclExp(t, &stmt->Stmt3, 0);
    t = SkipToken(t, TokRParen);
    t = CompStmtDeclExp(t, &stmt->Stmt4, 1);

    if (stmt->Stmt2) {
        if (stmt->Stmt2->st_Tok != TokExp)
            zerror(EFATAL_STMT_COND_NOT_EXP);

        InsertBranch(&((ExpStmt *)stmt->Stmt2)->Expr, COND_T, block->LabelLoop);
    }

    BlockUp();

    return(t);
}

short
CompWhile(short t, Stmt **pstmt)
{
    WhileStmt *stmt = AllocTmpStructure(WhileStmt);
    BlockStmt *block = BlockDown(BT_WHILE);

    *pstmt = (Stmt *)block;
    stmt->Block = block;
    block->LabelLoop  = AllocLabel();
    block->LabelTest  = AllocLabel();
    block->LabelBreak = AllocLabel();

    BlockAddStmt((Stmt *)stmt);

    stmt->st_Func = (void (*)(void *))GenWhile;
    stmt->st_Tok  = TokWhile;
    stmt->st_LexIdx = LFBase->lf_Index;

    t = SkipToken(t, TokLParen);

    t = CompStmtDeclExp(t, &stmt->Stmt1, 0);
    t = SkipToken(t, TokRParen);
    t = CompStmtDeclExp(t, &stmt->Stmt2, 1);

    if (stmt->Stmt1->st_Tok != TokExp)
        zerror(EFATAL_STMT_COND_NOT_EXP);
    InsertBranch(&((ExpStmt *)stmt->Stmt1)->Expr, COND_T, block->LabelLoop);

    BlockUp();

    return(t);
}

short
CompDo(short t, Stmt **pstmt)
{
    DoStmt *stmt = AllocTmpStructure(DoStmt);
    BlockStmt *block = BlockDown(BT_DO);

    *pstmt = (Stmt *)block;
    stmt->Block = block;
    block->LabelLoop  = AllocLabel();
    block->LabelTest  = AllocLabel();
    block->LabelBreak = AllocLabel();

    BlockAddStmt((Stmt *)stmt);

    stmt->st_Func = (void (*)(void *))GenDo;
    stmt->st_Tok  = TokDo;
    stmt->st_LexIdx = LFBase->lf_Index;

    t = CompStmtDeclExp(t, &stmt->Stmt1, 1);      /*  code    */
    t = SkipToken(t, TokWhile);
    t = CompStmtDeclExp(t, &stmt->Stmt2, 1);      /*  test    */

    if (stmt->Stmt2->st_Tok != TokExp)
        zerror(EFATAL_STMT_COND_NOT_EXP);
    InsertBranch(&((ExpStmt *)stmt->Stmt2)->Expr, COND_T, block->LabelLoop);

    BlockUp();

    return(t);
}

short
CompIf(short t, Stmt **pstmt)
{
    IfStmt *stmt = AllocTmpStructure(IfStmt);

    *pstmt = (Stmt *)stmt;
    stmt->st_Func = (void (*)(void *))GenIf;
    stmt->st_Tok  = TokIf;
    stmt->st_LexIdx = LFBase->lf_Index;

    stmt->LabelIf   = AllocLabel();
    stmt->LabelElse = AllocLabel();
    stmt->LabelEnd  = AllocLabel();

    t = SkipToken(t, TokLParen);
    t = CompStmtDeclExp(t, &stmt->Stmt1, 0);      /*  cond    */
    /* Make sure that the expression actually returns a value */

    t = SkipToken(t, TokRParen);
    t = CompStmtDeclExp(t, &stmt->StmtT, 1);      /*  true    */
    if (t == TokElse)
        t = CompStmtDeclExp(GetToken(), &stmt->StmtF, 1); /*  false */

    if (stmt->Stmt1->st_Tok != TokExp)
        zerror(EFATAL_STMT_COND_NOT_EXP);
    InsertBranch(&((ExpStmt *)stmt->Stmt1)->Expr, COND_F, stmt->LabelElse);

    return(t);
}

/*
 *  switch (exp) { ... }
 *
 *  Not only is exp in a semantic level, but each case/default is as well, so
 *  you can:        case 1:
 *                      int i = 4;
 *                      ...
 *                  case 2:
 *                      int i = 5;
 *                      ...
 *
 */

short
CompSwitch(short t, Stmt **pstmt)
{
    SwitchStmt *stmt = AllocTmpStructure(SwitchStmt);
    BlockStmt *block = BlockDown(BT_SWITCH);
    short siz = 0;
    short haveCase = 0;

    *pstmt = (Stmt *)block;

    BlockAddStmt((Stmt *)stmt);

    stmt->st_Func = (void (*)(void *))GenSwitch;
    stmt->st_Tok  = TokSwitch;
    stmt->st_LexIdx = LFBase->lf_Index;

    stmt->Block = block;

    block->LabelLoop  = AllocLabel();
    block->LabelBreak = AllocLabel();

    t = SkipToken(t, TokLParen);
    t = CompStmtDeclExp(t, &stmt->Stmt1, 0);    /*  exp  */
    Assert(stmt->Stmt1->st_Tok == TokExp);
                                            /*  don't deallocate result!  */
    stmt->Stmt1->st_Func = (void (*)(void *))GenExpResult;

    t = SkipToken(t, TokRParen);
    t = SkipToken(t, TokLBrace);

    /*
     *  now, each case and default is in its own block.
     */

    while (t && t != TokRBrace) {
        if (t == TokCase) {
            Exp *exp;

            if (siz == stmt->NumCases) {
                if (siz >= 32)
                    siz *= 2;
                else
                    siz += 4;
                stmt->Cases = zrealloc(stmt->Cases, sizeof(stmt->Cases[0]), stmt->NumCases, siz);
                stmt->Labels= zrealloc(stmt->Labels, sizeof(stmt->Labels[0]), stmt->NumCases, siz);
                stmt->CaseAry = zrealloc(stmt->CaseAry, sizeof(stmt->CaseAry[0]), stmt->NumCases, siz);
            }

            if (haveCase)
                BlockUp();
            stmt->CaseAry[stmt->NumCases] = BlockDown(BT_BLOCK);
            {
                int32_t l = AllocLabel();

                stmt->CaseAry[stmt->NumCases]->LabelTest = l;
                stmt->Labels[stmt->NumCases] = l;
            }
            t = CompExp(GetToken(), &exp, 1);
            stmt->Cases[stmt->NumCases] = ExpToConstant(exp);
            ++stmt->NumCases;
            t = SkipToken(t, TokColon);
            haveCase = 1;
        } else if (t == TokDefault) {
            if (haveCase)
                BlockUp();
            if (stmt->DefBlock)
                zerror(EERROR_DUPLICATE_DEFAULT);
            stmt->DefBlock = BlockDown(BT_BLOCK);
            stmt->DefBlock->LabelTest = AllocLabel();
            stmt->DefCaseNo = stmt->NumCases;   /*  insert before...    */

            t = SkipToken(GetToken(), TokColon);
            haveCase = 1;
        } else {
            Stmt *caseStmt;

            if (haveCase == 0 &&
                t != TokStruct   &&
                t != TokEnum     &&
                t != TokUnion    &&
                t != TokTypeQual &&
                t != TokTypeDef  &&
                t != TokTypeId   &&
                t != TokTypeof)
            {
                stmt->BeforeBlock = BlockDown(BT_BLOCK);
                haveCase = 1;
            }

            t = CompStmtDeclExp(t, &caseStmt, 1);

            /* Handle the case where they put code before the first case statement */
            BlockAddStmt(caseStmt);
        }
    }

    if (haveCase)
        BlockUp();

    t = GetToken();

    BlockUp();

    return(t);
}

/*
 *  break;
 */

short
CompBreak(short t, Stmt **pstmt)
{
    BreakStmt *stmt = AllocTmpStructure(BreakStmt);

    t = SkipToken(t, TokSemi);

    *pstmt = (Stmt *)stmt;
    stmt->st_Func = (void (*)(void *))GenBreak;
    stmt->st_Tok  = TokBreak;
    stmt->st_LexIdx = LFBase->lf_Index;
    if ((stmt->BreakLabel = FindBreakLabel()) == 0)
        zerror(EERROR_BREAK_OUTSIDE_LOOPSW);
    return(t);
}

short
CompContinue(short t, Stmt **pstmt)
{
    ContinueStmt *stmt = AllocTmpStructure(ContinueStmt);

    t = SkipToken(t, TokSemi);

    *pstmt = (Stmt *)stmt;
    stmt->st_Func = (void (*)(void *))GenContinue;
    stmt->st_Tok  = TokContinue;
    stmt->st_LexIdx = LFBase->lf_Index;
    if ((stmt->ContLabel = FindContinueLabel()) == 0)
        zerror(EERROR_CONT_OUTSIDE_LOOP);
    return(t);
}

/*
 *  goto label;
 */

short
CompGoto(short t, Stmt **pstmt)
{
    GotoStmt *stmt = AllocTmpStructure(GotoStmt);
    SemInfo *sem = NULL;

    *pstmt = (Stmt *)stmt;
    stmt->st_Func = (void (*)(void *))GenGoto;
    stmt->st_Tok  = TokGoto;
    stmt->st_LexIdx = LFBase->lf_Index;

    switch(t) {
    case TokId:
    case TokVarId:
    case TokEnumConst:
        if ((sem = FindSymbolId(LexSym, TokLabelId)) == NULL)
        {
            SemanticAddTopBlock(LexSym, TokLabelId, (void *)0);
            sem = FindSymbolId(LexSym, TokLabelId);
        }
        break;
    case TokLabelId:
        Assert(0);
        break;
    default:
        zerror(EERROR_SYNTAX_ERROR_EXP);
        break;
    }
    stmt->GotoLabel = sem;
    t = GetToken();
    t = SkipToken(t, TokSemi);
    return(t);
}

short
CompLabel(short t, Stmt **pstmt)
{
    LabelStmt *stmt = AllocTmpStructure(LabelStmt);
    SemInfo *sem;
    int32_t label;

    *pstmt = (Stmt *)stmt;
    stmt->st_Func = (void (*)(void *))GenLabel;
    stmt->st_Tok  = TokLabelId;
    stmt->st_LexIdx = LFBase->lf_Index;

    if ((sem = FindSymbolId(LexSym, TokLabelId)) == NULL) {
        label = AllocLabel();
        SemanticAddTopBlock(LexSym, TokLabelId, (void *)(intptr_t)label);
    } else {
        /* How can this be... We have to issue an error message because */
        /* They have defined the label twice...                         */
        label = (int32_t)(intptr_t)sem->Data;
        if (label)
        {
            zerror(EWARN_DUPLICATE_SYMBOL, LexSym->Len, LexSym->Name);
        }
        else
        {
            label = AllocLabel();
            sem->Data = (void *)(intptr_t)label;
        }
    }
    stmt->Label = label;
    t = SkipToken(GetToken(), TokColon);

    /*
     *  normally label: is bundled with a statement.  However, there are
     *  a few special cases where this is not so.
     */

    if (t != TokRBrace && t != TokCase && t != TokDefault)
        t = CompStmtDeclExp(t, &stmt->Stmt1, 1);
    return(t);
}


/*
 *  return exp;
 */

short
CompReturn(short t, Stmt **pstmt)
{
    ReturnStmt *stmt = AllocTmpStructure(ReturnStmt);

    *pstmt = (Stmt *)stmt;
    stmt->st_Func = (void (*)(void *))GenReturn;
    stmt->st_Tok  = TokReturn;
    stmt->st_LexIdx = LFBase->lf_Index;

    if (t != TokSemi) {
        t = CompStmtDeclExp(t, &stmt->Stmt1, 0);    /*  exp  */
        if (stmt->Stmt1->st_Tok != TokExp)
            zerror(EFATAL_SYNTAX_ERROR_EXP);
                                                    /*  save result */
        stmt->Stmt1->st_Func = (void (*)(void *))GenExpResult;
    }
    t = SkipToken(t, TokSemi);

    return(t);
}

/*
 *  __breakpoint
 */

short
CompBreakPoint(short t, Stmt **pstmt)
{
    BreakPointStmt *stmt = AllocTmpStructure(BreakPointStmt);

    *pstmt = (Stmt *)stmt;
    stmt->st_Func = (void (*)(void *))GenBreakPoint;
    stmt->st_Tok  = TokBreakPoint;
    stmt->st_LexIdx = LFBase->lf_Index;

    return(t);
}


