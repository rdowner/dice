
/*
 *  DC1/VAR.H
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 *
 *  Note that tracking of global variables within subroutines is accomplished
 *  by generating a dummy semantic variable at the top level of the subroutine.
 *  (VF_DUMMY).  At registerization time if the proper requirements are met
 *  and references satisfied the global is LEAd into an address register.
 *
 *  This, I believe, gives more flexibility than reserving A4 for a small-data
 *  model.  I would much rather have as many address registers available for
 *  register variables as possible.
 */

typedef struct Var {
    struct Var *Next;	    /*	list of variables?	     */
    struct Var *RegVar;     /*	if global placed in register */
    struct Type *Type;
    struct Symbol *Sym;
    struct Stor var_Stor;
    int32_t    Flags;	    /*	mainly storage classes		*/
    int32_t    LexIdx;	    /*	lexical index of declaration	*/
    short   Refs;	    /*	references to the var	*/
    short   RegFlags;	    /*	register spec / flags	*/
    union {		    /*	procedure block or assigned expression	  */
	struct BlockStmt *Block;
	struct Exp *AssExp;
	int32_t BOffset;	    /*	Offset of bitfield	*/
    } u;
} Var;

typedef struct ExtVarNode {
    struct ExtVarNode *Next;
    Var *Var;
} ExtVarNode;

typedef struct ExtStrNode {
    struct ExtStrNode *Next;
    char *Str;
    int32_t Len;
    int32_t Label;
    int32_t Flags;
    int32_t IIdx;	/* internationalization index */
} ExtStrNode;


/*
 *  The procedure structure contains information about the procedure
 */

typedef struct Proc {
    struct Type *Type;	/*  return type 			*/
    struct Var	*Vars;	/*  variables representing arguments	*/
    short   NumArgs;
    short   Reserved1;
    struct BlockStmt *Base; /*	procedure block 		    */
} Proc;

/*
 *  An expression is a structure which returns a quantity.
 *
 *  Most flag passage requires an ack.	For example, a routine with the
 *  capability to work from condition codes requests that the result be
 *  returned as a condition but only utilizes such if the sub-expression
 *  tells it it can.
 *
 *  To determine when scratch variables may be used to hold a temporary
 *  result (so as not to get blown away by a procedure call), EF_CALL is
 *  propogated backwards in pass 1, then EF_SCROK is propogated forwards
 *  in pass2 (scratch-ok).
 */

/*
 *  These are set in pass 0
 */

#define EF_RNU	    0x0001	/*  child's result will not be used     */
#define EF_COND     0x0002	/*  request branch on condition 	*/
#define EF_CRES     0x0004	/*  result storage allocated by child	*/
#define EF_PRES     0x0008	/*  result storage allocated by parent	*/
#define EF_STACK    0x0010	/*  request result be placed on stack (ints only)   */
#define EF_ASSEQ    0x0020	/*  assign-equal (e1 is result) 	*/

#define EF_CALL     0x0040	/*  call made in this sub-tree		    */
#define EF_ICAST    0x0080	/*  cast - sub-call already made pass 0!    */

/*
 *  These are returned in pass 1
 */

#define EF_CONDACK	0x0100	/*  can do request to branch	    */
#define EF_STACKACK	0x0200	/*  result was pushed on stack	    */
#define EF_ASSPOSINC	0x0400	/*  post incr on assignment	    */
#define EF_ASSPREDEC	0x0800	/*  pre dec on assignment	    */
#define EF_DIRECT	0x1000
#define EF_LHSASSIGN	0x2000	/*  lhs is for assignment (bitflds) */
#define EF_LHSASSEQ	0x4000
#define EF_SPECIAL	0x8000	/*  special (inline special arg)    */

typedef struct Exp {
    void       (*ex_Func)(struct Exp **);   /*	generative procedure		*/
    struct Exp	*ex_Next;	/*  used during generation	    */
    struct Type *ex_Type;	/*  return type of expression	    */
    uword	ex_Flags;
    char	ex_Res1;
    char	ex_Cond;
    u_short	ex_Token;	/*  generator dependant 	    */
    short	ex_Res2;
    struct Stor ex_Stor;	/*  machine storage for result (code gen)   */
    int32_t	ex_LexIdx;	/*  line number in input file	    */

    union {
	struct Exp *Exp;	/*  left hand side		    */
	struct Var *Var;	/*  terminating variable	    */
	char	*StrConst;
    } u;

    union {
	struct Exp *Exp;	/*  right hand side / arglist for p */
	struct Symbol *Sym;	/*  structure element		    */
	int32_t	Label;		/*  branch condition		    */
	int32_t	StrLen;
	int32_t	Offset; 	/*  structure . ->		    */
	int32_t	*ConstAry;	/*  linked list of constants	    */
    } v;

    int32_t ex_LabelT;
    int32_t ex_LabelF;
    void *ex_Reserved1;
} Exp;

#define ex_Precedence	ex_Flags
#define ex_Order	ex_Cond

#define ex_ExpL 	u.Exp
#define ex_Var		u.Var
#define ex_StrConst	u.StrConst

#define ex_Offset	v.Offset
#define ex_Symbol	v.Sym
#define ex_ExpR 	v.Exp
#define ex_Label	v.Label
/*#define ex_XType	  v.XType*/
#define ex_StrLen	v.StrLen
#define ex_ConstAry	v.ConstAry

typedef struct PragNode {
    struct PragNode *pn_Next;
    char    *pn_Func;
    char    *pn_Off;
    char    *pn_Ctl;
    short   pn_FuncLen;
    short   pn_OffLen;
    short   pn_CtlLen;
    short   pn_Offset;
    Symbol  *pn_Sym;	/*  base variable   */
} PragNode;

