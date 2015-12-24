
/*
 *  DC1/STMT.H
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
**      $Filename: stmt.h $
**      $Author: dice $
**      $Revision: 30.0 $
**      $Date: 1994/06/10 18:04:57 $
**      $Log: stmt.h,v $
 * Revision 30.0  1994/06/10  18:04:57  dice
 * .
 *
 * Revision 1.3  1993/09/19  13:03:16  jtoebes
 * Fixed BUG00148 - Compiler does not catch gotos to non-existent label.
 * Changed intermediate gotolabel information to know more about the label.
 *
 * Revision 1.2  1993/09/11  16:11:53  jtoebes
 * Fixed BUG01010 - Code not allowed in switch statement before the first case.
 * Added place in statement structure to hold the orphaned code.
 *
**/

#define st_Func     Hdr.Func
#define st_Tok	    Hdr.Tok
#define st_Next     Hdr.Next
#define st_LexIdx   Hdr.LexIdx

typedef struct StmtHdr {
    void    (*Func)(void *);
    short   Tok;
    ubyte   Reserved1;
    ubyte   Reserved2;
    long    LexIdx;
    struct Stmt *Next;
} StmtHdr;

typedef struct Stmt {
    StmtHdr Hdr;
} Stmt;

/*
 *  Sub statements.  Note that labels are allocated at parse time whether or
 *  not they are used.	Normally the statement generator will insert conditional
 *  branch nodes into the condition expressions and/or generate it's own
 *  expression to handle branch to the beginning of a loop, for example.
 */

typedef struct ExpStmt {
    StmtHdr Hdr;
    struct Exp	*Expr;
} ExpStmt;

typedef struct DeclStmt {
    StmtHdr Hdr;
    struct Var	*Var;
} DeclStmt;

typedef struct LabelStmt {
    StmtHdr Hdr;
    long    Label;
    Stmt    *Stmt1;	    /*	a: stmt, stmt is optional   */
} LabelStmt;


typedef struct ForStmt {
    StmtHdr Hdr;
    struct BlockStmt *Block;
    Stmt    *Stmt0;	/*  available for initial jump	*/
    Stmt    *Stmt1;	/*  init    */
    Stmt    *Stmt2;	/*  exp     */
    Stmt    *Stmt3;	/*  stmt    */
    Stmt    *Stmt4;	/*  code    */

    long    LabelBegin;
} ForStmt;

typedef struct WhileStmt {
    StmtHdr Hdr;
    struct BlockStmt *Block;
    Stmt    *Stmt0;	/*  available for initial jump	*/
    Stmt    *Stmt1;	/*  really an exp   */
    Stmt    *Stmt2;	/*  code    */
} WhileStmt;

typedef struct DoStmt {
    StmtHdr Hdr;
    struct BlockStmt *Block;
    Stmt    *Stmt0;	/*  available for initial jump	XXX */
    Stmt    *Stmt1;	/*  code    */
    Stmt    *Stmt2;	/*  test    */
} DoStmt;

typedef struct IfStmt {
    StmtHdr Hdr;
    Stmt    *Stmt1;	/*  cond	    */
    Stmt    *StmtT;	/*  if true	    */
    Stmt    *StmtF;	/*  if false/NULL   */
    long    LabelIf;
    long    LabelElse;
    long    LabelEnd;
} IfStmt;

/*
 *  Some confusion may occur as to why CaseAry and DefStmt pointer to a
 *  pointer to the statement.  This occurs because at the time of the
 *  case or default label we do not yet have a statement list for it, but
 *  we DO have a pointer to where that statement list will begin.
 */

typedef struct SwitchStmt {
    StmtHdr Hdr;
    struct BlockStmt *Block;
    Stmt    *Stmt1;		/*  switch exp		    */
    long    NumCases;		/*  number of cases	    */
    long    DefCaseNo;		/*  default case insertion  */
    long    *Cases;		/*  switch constants	    */
    struct BlockStmt **CaseAry; /*  cases for switch	    */
    struct BlockStmt *DefBlock; /*  case for default/NULL   */
    struct BlockStmt *BeforeBlock; /*  Code which appears before any case   */
    long    *Labels;		/*  label id's              */
} SwitchStmt;

typedef struct ReturnStmt {
    StmtHdr Hdr;
    Stmt    *Stmt1;	    /*	return expression	*/
} ReturnStmt;

typedef struct BreakPointStmt {
    StmtHdr Hdr;
} BreakPointStmt;

typedef struct GotoStmt {
    StmtHdr Hdr;
    SemInfo *GotoLabel;
} GotoStmt;

typedef struct ContinueStmt {
    StmtHdr Hdr;
    long    ContLabel;
} ContinueStmt;

typedef struct BreakStmt {
    StmtHdr Hdr;
    long    BreakLabel;
} BreakStmt;

#define BT_PROC     1
#define BT_BLOCK    2
#define BT_FOR	    3
#define BT_WHILE    4
#define BT_DO	    5
#define BT_SWITCH   6

typedef struct BlockStmt {
    StmtHdr Hdr;
    short   Bid;
    short   Reserved1;
    Frame   Frame;	    /*	allocation frame	    */
    struct BlockStmt *Parent;
    struct Var	*VarBase;   /*	variables declared in block */
    struct Var	**LastVar;
    struct Stmt *Base;	    /*	first statement in list     */
    struct Stmt **Last;     /*	last statement in list	    */

    long    LabelLoop;
    long    LabelTest;
    long    LabelBreak;
    long    LastLexIdx;	    /*  lexical index terminating block */
} BlockStmt;

