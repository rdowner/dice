/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  TOPLEVEL.C
 */

/*
**      $Filename: toplevel.c $
**      $Author: dice $
**      $Revision: 30.326 $
**      $Date: 1995/12/24 06:09:50 $
**      $Log: toplevel.c,v $
 * Revision 30.326  1995/12/24  06:09:50  dice
 * .
 *
 * Revision 30.325  1995/12/24  05:38:17  dice
 * .
 *
 * Revision 30.6  1994/08/04  04:49:46  dice
 * fixed regargs bug.  A previous fix involving automatic registerized
 * arguments broke explicit registerization of arguments when more then
 * four arguments are specified.
 *
 * Revision 30.0  1994/06/10  18:04:58  dice
 * .
 *
 * Revision 1.14  1993/11/22  00:28:36  jtoebes
 * Final cleanup to eliminate all cerror() messages with strings.
 *
 * Revision 1.14  1993/11/22  00:28:36  jtoebes
 * Final cleanup to eliminate all cerror() messages with strings.
 *
 * Revision 1.13  1993/09/18  21:03:26  jtoebes
 * Minor fix encountered when recursively recompiling compiler.  Previous
 * fix failed to set the auto bit under some circumstances for local function
 * pointers.
 *
 * Revision 1.12  1993/09/13  21:28:23  jtoebes
 * Fixed BUG00117 - Problems with typdef'd functions.
 * Made error logic ignore function pointers declared in a local procedure.
 *
 * Revision 1.11  1993/09/11  23:25:16  jtoebes
 * Fixed BUG00047 - DC1 failes to warn about missing , in initialized expression.
 * Changed to conform to new calling parameters for CompBracedAssign.
 *
 * Revision 1.10  1993/09/06  23:37:12  jtoebes
 * Fixed BUG00102 - Constant expressions not allowed for bitfields.
 *
 * Revision 1.9  1993/09/06  14:10:15  jtoebes
 * fixed BUG00113 - const keyword was not allowed after the type.
 * Simply inserted parsing code for immediately after the type.
 *
 * Revision 1.8  1993/09/06  13:21:34  jtoebes
 * Fixed BUG01091 - Bad error for undefined structure tag.
 * Added code for arrays to ensure that structures and unions are defined
 * before attempting to make an array out of them.
 *
 * Revision 1.7  1993/09/06  11:06:50  jtoebes
 * Fixed BUG06063 - Error for initialization of an external array of chars.
 * Turned off extern bit when something was initialized.
 *
 * Revision 1.6  1993/09/05  23:54:32  jtoebes
 * Fixed BUG06045 - Enforcer hit for invalid structure definitions.
 * Added code to skip over current level of nesting when too many
 * initializers are given for a structure.
 *
**/

#include "defs.h"


#define MAXCPX	64	/*  maximum type declarator complexity	*/

static short StructLevel;   /*	sp CompVar() does not call SemanticAdd() */
static ExtVarNode *ExtBase;
static ExtStrNode *StrBase;

Prototype short TopLevel(short);
Prototype short CompDecl(short, Var **, long);
Prototype short CompType(short, Type **, long *, long *);
Prototype short CompVar(short, Type *, long, long, Var **);
Prototype short CompTypeDeclarators(short, Type **, Symbol **, long);
Prototype short CompStructType(short, Type **, long);
Prototype short CompEnumType(short, Type **);
Prototype short CompTypeofType(short, Type **);
Prototype short ResolveStructUnionType(Type *);

Prototype void AddExternList(Var *);
Prototype void DumpExternList(long);
Prototype void AddStrList(char *, long, long, long);
Prototype void MakeStringConst(long);
Prototype void DelStrList(long);
Prototype void DumpStrList(void);

Local short PointerOpt(Type **, Symbol **, short);
Local short DirectDeclarator(Type **, Symbol **, short, long);
Local void ReverseFeed(Type **, Type *);
Local int ExplicitRegistersDeclared(Var **vars, long args);


/*
 *  TopLevel() expects:
 *
 *	<decl>
 *	typedef <decl>
 */

short
TopLevel(short t)
{
    Var *var;
    Var *nv;

    /*
     *	Do not absort the semicolon at the end of the statement as this
     *	would require another lexical token causing a #pragma DCCOPTS
     *	after a procedure definition to be interpreted before the previous
     *	procedure's code was generated.
     */

    t = CompDecl(t, &var, 0);

    while (var) {
	nv = var->Next;
	GenerateVar(var);
	var = nv;
    }

    /*
     *	Clear out any lingering expression structures here ??
     *	Clear out all procedural sub-variables EXCEPT arguments to
     *	    the procedure and the procedure var itself. ??
     */

    t = GetToken();

    return(t);
}

short
CompDecl(short t, Var **pvar, long absorbSemi)
{
    short typeDef = 0;		/*  preceeded by typedef    */
    Var *var = NULL;
    Type *baseType;
    long  baseFlags;
    long  regFlags;
    long li = LFBase->lf_Index;

    if (t == TokTypeDef) {	/*  opt typedef 	*/
	t = GetToken();
	typeDef = 1;
    }

    /*
     *	type spec
     */

    t = CompType(t, &baseType, &baseFlags, &regFlags);

    /*
     *	list of type-qualfied variables (i.e. char *foo, the '*foo').  CompVar()
     *	also parses entire subroutines and returns a pseudo ';' for them.
     *
     *	The variables are semantically added
     */

    if (typeDef) {
	t = CompVar(t, baseType, baseFlags, regFlags, NULL);
	while (t == TokComma) {
	    t = GetToken();
	    t = CompVar(t, baseType, baseFlags, regFlags, NULL);
	}
    } else if (t != TokSemi) {
	Var *vlast;
	Var *v;

	t = CompVar(t, baseType, baseFlags, regFlags, &var);
	vlast = var;

	/*
	 * Handle case where we compiled a procedure, the lexical index
	 * must indicate the beginning of the procedure so debugging
	 * info is aligned properly
	 */

	if (var->Type->Id == TID_PROC)
	    var->LexIdx = li;

	/*
	 * Handle list of vars
	 */

	while (t == TokComma) {
	    t = GetToken();
	    t = CompVar(t, baseType, baseFlags, regFlags, &v);

	    vlast->Next = v;
	    vlast = v;
	}
	vlast->Next = NULL;
    }
    if (t != TokSemi) {
	zerror(EWARN_EXPECTED_SEMICOLON);
	if (LFBase->lf_Index == li)
	    t = GetToken();
    }

    if (absorbSemi)
	t = GetToken();

    *pvar = var;
    return(t);
}

short
CompType(short t, Type **ptype, long *pflags, long *rflags)
{
    Type *type;
    long  flags = 0;

    *rflags = 0;

    /*
     *	Allow register qualifiers before or after type qualifiers for
     *	SAS/C compatibility.  Personally, DICE likes them before
     */

    while (t == TokTypeQual) {
	flags |= (long)LexData;
	t = GetToken();
    }
    while (t == TokRegQual) {
	if (*rflags)
	    zerror(EERROR_ILLEGAL_REGSPEC);
	else
	    *rflags = (long)LexData | RF_REGISTER;
	t = GetToken();
    }
    while (t == TokTypeQual) {
	flags |= (long)LexData;
	t = GetToken();
    }
    if (t == TokTypeId) {
	type = (Type *)LexData;
	t = GetToken();
	while (t == TokTypeQual) {
	    flags |= (long)LexData;
	    t = GetToken();
	}
	if (t == TokTypeId) {
#if 0
	    if ((Type *)LexData == &LongType && (type == &ShortType || type == &LongType)) {
		t = GetToken();
#endif
	    /*
	     * handle 'long long'
	     */
	    if ((Type *)LexData == &LongType && type == &LongType) {
		t = GetToken();
		type = &LongLongType;
	    } else if ((Type *)LexData == &DoubleType && type == &LongType) {
		type = &LongDoubleType;
		t = GetToken();
	    }
	    /* note, can't else error since structure elements may
	     * be type names
	     */
	}
    } else if (t == TokStruct || t == TokUnion) {
	t = CompStructType(t, &type, flags);
    } else if (t == TokEnum) {
	t = CompEnumType(t, &type);
    } else if (t == TokTypeof) {
	t = CompTypeofType(t, &type);
    } else {
	type = &LongType;
    }

    flags |= type->Flags;

    while (t == TokTypeQual && ((long)LexData & TF_STORQUALMASK)) {
	/*
	 *  Generate warning, but accept type.	Note that the qualifier
	 *  will apply to ALL declarations in this statement rather then
	 *  just the first as in SAS/C, thus the warning.
	 */
	zerror(EWARN_SASC_QUALIFIER_PLACEMENT);
	flags |= (long)LexData;
	t = GetToken();
    }

    if ((type->Flags & TF_TYPEQUALMASK) != (flags & TF_TYPEQUALMASK))
	type = TypeToQualdType(type, flags & TF_TYPEQUALMASK);

    *ptype = type;
    *pflags= flags & TF_STORQUALMASK;

    return(t);
}

/*
 *  Compile a variable... handle *, [], (), etc...  void (*ptr)()
 *		     also handle '= expression'
 *		     also handle bit fields though currently they are
 *		     NOT implemented properly.
 *
 *  if pvar == NULL then we are compiling a typedef.  Return TokSemi for
 *  procedural declarations that end with '}'.
 *
 *  baseType->Flags holds TYPEQUALMASK flags while
 *  storFlags	    holds STORQUALMASK flags.
 */

short
CompVar(short t, Type *baseType, long storFlags, long regFlags, Var **pvar)
{
    Var *var = NULL;
    Type *type;
    Symbol *sym = NULL;
    long extraTypeFlags = 0;	/*  propogated through override */
    short protoVoid = 0;

    /*
     *	step 1, parse the name .. get the name symbol and representitive type
     */

    type = baseType;

    t = CompTypeDeclarators(t, &type, &sym, storFlags);

    if (type->Flags & TF_STKCALL) storFlags &= ~TF_REGCALL;

    if (t == TokColon) {
	long bits;
	Exp *exp = NULL;

	if (StructLevel == 0)
	    zerror(EERROR_ILLEGAL_BITFIELD_OP);

	if (type->Id != TID_INT || (*type->Size != 4 && *type->Size != 2))
	    zerror(EERROR_ILLEGAL_BITFIELD_OP);

	t = GetToken();
	t = CompExp(t, &exp, 0);
	bits = ExpToConstant(exp);

	if (bits > 32 || bits < 0) {
	    zerror(EERROR_ILLEGAL_BITFIELD_OP);
	    bits = 32;
	}
	type = MakeBitfieldType(type->Flags & TF_UNSIGNED, bits);
    }

    /*
     *	Create final type that includes storage qualifiers.
     */

    if ((storFlags & TF_STORNOTLOCAL) == 0 && State != SOUTSIDE)
    {
        /* Check to see that this is a pointer to a function */
        if (type->Id != TID_PTR   ||
            type->SubType == NULL ||
            type->SubType->Id != TID_PROC)
        {
	    if (storFlags & TF_AUTOILLEGAL)
		zerror(EERROR_ILLEGAL_QUALIFIER);
	}
	storFlags |= TF_AUTO;
    }

    {
	long tflags = (type->Flags | storFlags) & ~TF_NOTINTYPE;

	if (type->Flags != tflags)
	    type = TypeToQualdType(type, tflags);
    }

    /*
     *	If typedef we stop here
     */

    if (pvar == NULL) {
	if (type->Flags != (type->Flags | storFlags))
	    type = TypeToQualdType(type, type->Flags | storFlags);

	if (t != TokSemi && t != TokComma)
	    zerror(EERROR_ILLEGAL_TYPEDEF);
	if (sym)
	    SemanticAdd(sym, TokTypeId, type);
	else
	    zerror(EERROR_ILLEGAL_TYPEDEF);
	return(t);
    }

    /*
     *	definitely a variable.	Check for overide of previous def
     *	in this block level, or global block level if not auto.
     *
     *	STATIC VARIABLES inside subroutines are special, they do not bother
     *	global variables outside subroutines of the same name.
     *
     *	check opt '= exp' and optional code if a procedure
     */

    if (sym == NULL)
    {
	/* Well, we have a syntax error */
	zerror(EERROR_SYNTAX_ERROR_DECL);
	sym = MakeSymbol("<PHONEYSYM>", 11, LFBase->lf_Index, NULL);
	storFlags |= VF_DECLD;
    }

    if (StructLevel == 0 && (var = BlockRemoveDuplicateVar(storFlags, sym, type))) {
	extraTypeFlags = (var->Type->Flags | storFlags) & TF_KEEPPROC;
	if (var->Type->Args == 0)   /*	proto was ... foo(void) */
	    protoVoid = 1;

	if ((type->Flags & TF_KEEPPROC) != (extraTypeFlags & TF_KEEPPROC))
	    type = TypeToQualdType(type, type->Flags | extraTypeFlags);
    }

    if (var == NULL) {
	if (StructLevel == 0 && State != SOUTSIDE && (storFlags & TF_STORNOTLOCAL) == 0 && type->Id != TID_PROC) {
	    var = AllocTmpStructure(Var);   /*	does not survive subroutine */
	} else {
	    var = AllocStructure(Var);	    /*	permanent   */
	}
	if (sym && StructLevel == 0) {
	    if (type->Id == TID_PROC || (storFlags & TF_EXTERN))
		SemanticAddTop(sym, TokVarId, var);
	    else
		SemanticAdd(sym, TokVarId, var);
	}
    }

    var->Type = type;
    var->Sym  = sym;
    var->LexIdx = LFBase->lf_Index;

    if (storFlags & TF_REGISTER)
	++var->Refs;

    if ((type->Id == TID_STRUCT || type->Id == TID_UNION) && (*type->Size == 0))
    {
	Undefined_Tag(type, sym, LFBase->lf_Index);
    }

    *pvar = var;

    if (State == SOUTSIDE && (storFlags & TF_AUTO))
	zerror(EERROR_ILLEGAL_QUALIFIER);

    var->Flags = type->Flags | storFlags | extraTypeFlags | (var->Flags & VF_DECLD);
    var->RegFlags = regFlags;

#ifdef DYNAMIC
    if ((var->Flags & (TF_EXTERN|TF_DYNAMIC)) == TF_DYNAMIC) {
	var->Type = type = TypeToPtrType(var->Type);
	asm_dynamictag(var);
    }
#endif
    if (type->Id == TID_PROC) {
	if (t != TokSemi && t != TokComma && CToDClass == NULL) {
	    /*
	     * Procedure definition
	     */
	    var->Flags &= ~(VF_DECLD | TF_EXTERN);

	    /*
	     *	if not a prototyped procedure must munge arguments properly.
	     *	This applies to floats, for example, which are really doubles
	     *	for non-prototyped procedures.
	     */

	    if ((type->Flags & TF_PROTOTYPE) == 0) {
		short i;
		for (i = 0; i < type->Args; ++i) {
		    Type **t2 = &type->Vars[i]->Type;
		    *t2 = ActualArgType(*t2);
		}
	    }

	    /*
	     *	munge routine prototyped as foo(void) but declared foo()
	     */

	    if (protoVoid && type->Args < 0)
		var->Type = type = TypeToProcType(type->SubType, NULL, 0, type->Flags | TF_PROTOTYPE);

	    if (var->u.Block && !(var->Flags & TF_STATIC))
		zerror(EERROR_MULTIPLY_DEFINED_PROC);

	    t = CompProcedure(t, var);
	} else if (t != TokSemi && t != TokComma && CToDClass) {
	    /*
	     * Convert procedure definitions to extern procedure
	     * declarations if -CTOD.  That is, with -CTOD we are trying
	     * to proceduce glue information for D (or some other language)
	     * so it can interface with C.
	     */
	    int count = 1;

	    if (t != TokLBrace)
		zerror(EERROR_EXPECTED_OCBRACE_PROC);
	    while ((t = GetToken()) != 0) {
		if (t == TokLBrace)
		    ++count;   
		if (t == TokRBrace) {   
		    if (--count == 0)
			break;
		}   
	    }
	    if (t == TokRBrace)
		t = TokSemi;
	    if (!(var->Flags & TF_STATIC))
		var->Flags |= TF_EXTERN;
	} else {
	    /*
	     * Procedure declaration
	     */
	    if (var->u.Block == NULL) {
		if (!(var->Flags & TF_STATIC))
		    var->Flags |= TF_EXTERN;
	    } else {
		zerror(EERROR_MULTIPLY_DEFINED_PROC);
	    }

	    /*
	     *	Generate Register Specification Output Data
	     */

	    if (RegSpecOutputOpt)
		GenerateRegSpecOutput(var);
	}
    } else {
	if (t == TokEq) {	/*  assigned expression */
	    Exp *exp;

            /* Any assignment to a global variable will eliminate the extern */
            /* portion of the symbol.  This is in accordance to ANSI.        */
	    var->Flags &= ~TF_EXTERN;

	    if (StructLevel)
		zerror(EERROR_ILLEGAL_ASSIGNMENT);

	    t = GetToken();
	    if (t == TokLBrace)
	    {
		t = CompBracedAssign(GetToken(), var->Type, &exp, 1, sym);
		if (t != TokRBrace)
		{
		    zerror(EERROR_TOO_MANY_INITIALIZERS);
		    t = match_nesting(t);
		}
		t = GetToken();
	    }
	    else if (t == TokStrConst && var->Type->Id == TID_ARY)
	    {
		t = CompBracedAssign(t, var->Type, &exp, 0, sym);
            }
	    else if ((var->Flags & TF_AUTO) == 0)
	    {
		t = CompBracedAssign(t, var->Type, &exp, 0, sym);
	    } else
	    {
	       /* If we get here, we know that the TF_AUTO bit is on.  If  */
	       /* the TF_STATIC flag is also set, this should be diagnosed */
	       /* This logic used to be in the previous test, but since it */
	       /* really can never occur, we moved it down here and expect */
	       /* to take it out after the beta.                           */
	       /* Note that the assignment code will be pulled out by the  */
	       /* block code in stmt.c (only simple assignments)           */
 	       if (var->Flags & TF_STATIC)
 	       {
 	       	  dbprintf(("Unexpected Type %ld\n", var->Flags));
 	          Assert(0);
 	       }
	       t = CompExp(t, &exp, 0);
            }
	    var->u.AssExp = exp;
	}
    }
    return(t);
}

/*
 *  abstract:
 *	PTR
 *	PTRopt direct
 *
 *  direct:
 *	(abstract)
 *	DIRECTopt [constant-exp]
 *	DIRECTopt (parameter-type-list)
 *
 *  storFlags might apply to some lower level type (procedure spec)
 */

short
CompTypeDeclarators(short t, Type **ptype, Symbol **psym, long storFlags)
{
    Type *head = NULL;

    /* *ptype = baseType; */

    if (t == TokStar)
	t = PointerOpt(ptype, psym, t);

    switch(t) {
    case TokLParen:	/*  late eval	*/
	head = &CharType;
	t = CompTypeDeclarators(GetToken(), &head, psym, 0);
	t = SkipToken(t, TokRParen);
	break;
    case TokVarId:	/*  symbol	*/
    case TokId:
    case TokTypeId:
    case TokEnumConst:
	*psym = LexSym;
	t = GetToken();
	break;
    /* default: break; */   /*	other	*/
    }

    t = DirectDeclarator(ptype, psym, t, storFlags);

    /*
     *	Stick head in front of ptype.  Must scan head in reverse.
     */

    if (head)
	ReverseFeed(ptype, head);

    return(t);
}

void
ReverseFeed(ptype, type)
Type **ptype;
Type *type;
{
    if (type) {
	ReverseFeed(ptype, type->SubType);

	switch(type->Id) {
	case TID_INT:
	    break;	/*  done    */
	case TID_PTR:
	    *ptype = TypeToPtrType(*ptype);
	    break;
	case TID_ARY:
	    {
		long entries = 0;

		if (*type->SubType->Size)
		    entries = *type->Size / *type->SubType->Size;
		*ptype = TypeToAryType(*ptype, NULL, entries);
	    }
	    break;
	case TID_PROC:
	    *ptype = TypeToProcType(*ptype, type->Vars, type->Args, type->Flags);
	    break;
	default:
	    dbprintf(("typedecl/tid %d\n", type->Id));
	    Assert(0);
	    break;
	}
    }
}

short
PointerOpt(Type **ptype, Symbol **psym, short t)
{
    while (t == TokStar) {
	long flags = 0;

	t = GetToken();
	*ptype = TypeToPtrType(*ptype);
	while (t == TokTypeQual) {
	    flags |= (long)LexData;
	    t = GetToken();
	}
	if (flags) {
	    Type *type = *ptype;
	    *ptype = TypeToQualdType(type, (flags | type->Flags) & TF_TYPEQUALMASK);
	}
    }
    return(t);
}

short
DirectDeclarator(Type **ptype, Symbol **psym, short t, long storFlags)
{
    switch(t) {
    case TokLParen:
	{
	    Var **vars;
	    long args;
	    long flags;
	    t = CompProcedureArgDeclarators(GetToken(), &vars, &args, &flags);

	    t = DirectDeclarator(ptype, psym, t, 0);

	    /*
	     *	Finish up.  If procedure is not already registerized determine
	     *	whether to make it so or not.  (flags & TF_STKCALL) is set when
	     *	automatic registerization is not possible, (storFlags & TF_STKCALL)
	     *	when the user explicitly does not want automatic registerization
	     *
	     *  If explicit registers have been declared, do NOT set 
	     *  TF_STKCALL.
	     */

	    if (args <= 0 || args > 4) {
	    	if (ExplicitRegistersDeclared(vars, args) == 0)
		    flags |= TF_STKCALL;
	    }
	    if ((flags & TF_REGCALL) == 0)
	    {
		if (((storFlags|flags) & TF_STKCALL) == 0 &&
		    (RegCallOpt || (storFlags & TF_REGCALL)) && !(flags & TF_DOTDOTDOT)
		      && args > 0 && args <= 4)
		    flags |= TF_REGCALL;
	    }
	    storFlags &= ~TF_REGCALL;

	    if (flags & TF_REGCALL)
		AutoAssignRegisteredArgs(vars, args);
	    *ptype = TypeToProcType(*ptype, vars, args, flags | storFlags);
	}
	break;
    case TokLBracket:
	{
	    Exp *exp = NULL;

	    t = GetToken();
	    if (t != TokRBracket)
		t = CompExp(t, &exp, 1);

	    if (((*ptype)->Id == TID_STRUCT || (*ptype)->Id == TID_UNION) && (*(*ptype)->Size == 0))
	    {
		Undefined_Tag(*ptype, NULL, LFBase->lf_Index);
	    }

	    t = SkipToken(t, TokRBracket);

	    t = DirectDeclarator(ptype, psym, t, storFlags);

	    if (*(*ptype)->Size == 0) {
		zerror(EERROR_SIZEOF_TYPE_0);
	    }
	    *ptype = TypeToAryType(*ptype, exp, 0);
	}
	break;
    default:		    /*	fix the ordering    */
	break;
    }
    return(t);
}

/*
 * Scan procedure variables to determine if explicit
 * registers have been declared.
 */

int
ExplicitRegistersDeclared(Var **vars, long args)
{
    long i;
    int r = 0;

    for (i = 0; i < args; ++i, ++vars) {
        if ((*vars)->RegFlags & RF_REGISTER) {
            r = 1;
            break;
	}
    }
    return(r);
}

/*
 *  Access/compile a structural type that may or may not exist yet.
 *
 *  t == TokStruct or TokUnion.     s/u [optid] [{ structure}]
 */

short
CompStructType(short t, Type **ptype, long flags)
{
    short isUnion = (t == TokUnion);
    Type *type;

    t = GetToken();
    if (t == TokId || t == TokTypeId || t == TokVarId || t == TokEnumConst) {
	if ((type = FindStructUnionType(LexSym, isUnion)) == NULL)
	    type = MakeStructUnionType(LexSym, isUnion);
	t = GetToken();
    } else {
	type = MakeStructUnionType(NULL, isUnion);
    }

    if (t == TokLBrace) {
	t = ResolveStructUnionType(type);
    }

    *ptype = type;
    return(t);
}

short
ResolveStructUnionType(type)
Type *type;
{
    short t;

    {
	Var **vars = NULL;
	Var *var;
	short n = 0;
	short siz = 0;

	t = GetToken();
	++StructLevel;
	while (t && t != TokRBrace) {
	    t = CompDecl(t, &var, 1);
	    while (var) {
		if (n == siz) {
		    siz += 4;
		    vars = zrealloc(vars, sizeof(Var *), n, siz);
		}
		vars[n++] = var;
		var = var->Next;
	    }
	}
	--StructLevel;
	if (siz == 0)
	{
	    zerror(EERROR_NO_MEMBERS, SymToString(FindStructUnionTag(type)));
	    vars = zrealloc(vars, sizeof(Var *), 1, 1);
	    var = AllocStructure(Var);
	    var->Type = &LongType;
	    vars[0] = var;
	    n = 1;
	}

	SetStructUnionType(type, vars, n, type->Flags);
	if (t == TokRBrace)
	    t = GetToken();
    }
    return(t);
}

/*
 *  Access/compile an enum type that may or may not exist yet.
 *
 *  enum [enumName] [{ specifiers }]
 */

short
CompEnumType(short t, Type **ptype)
{
    Type *type;

    t = GetToken();
    if (t == TokId || t == TokTypeId || t == TokVarId || t == TokEnumConst) {
	if ((type = FindEnumType(LexSym)) == NULL)
	    type = MakeEnumType(LexSym);
	t = GetToken();
    } else {
	type = MakeEnumType(NULL);
    }
    if (t == TokLBrace) {
	long eval = 0;
	t = GetToken();
	while (t != TokRBrace) {
	    Symbol *sym;
	    if (t != TokId) {
		if (t == TokVarId || t == TokTypeId) {
		    zerror(EWARN_ENUM_OVERIDE);
		} else {
		    zerror(EERROR_SYNTAX_ERROR_DECL);
		    break;
		}
	    }
	    sym = LexSym;
	    t = GetToken();
	    if (t == TokEq) {
		Exp *exp;

		t = GetToken();
		t = CompExp(t, &exp, 0);
		eval = ExpToConstant(exp);
	    }
	    AddEnumIdent(type, sym, eval);
	    ++eval;
	    if (t == TokComma) {
		t = GetToken();
	    } else if (t != TokRBrace) {
		zerror(EERROR_SYNTAX_ERROR_DECL);
		break;
	    }
	}
	if (t == TokRBrace)
	    t = GetToken();
    }
    *ptype = type;
    return(t);
}

/*
 *  Compile typeof(exp)
 */

short
CompTypeofType(short t, Type **ptype)
{
    Exp *exp;

    t = SkipToken(GetToken(), TokLParen);
    t = CompExp(t, &exp, 1);
    t = SkipToken(t, TokRParen);
    *ptype = ExpToType(exp);
    return(t);
}

void
AddExternList(var)
Var *var;
{
    ExtVarNode *evn = AllocStructure(ExtVarNode);

    evn->Next = ExtBase;
    evn->Var = var;
    ExtBase = evn;
}

void
DumpExternList(long procs)
{
    ExtVarNode *evn;
    Var *var;

    for (evn = ExtBase; evn; evn = evn->Next) {
	var = evn->Var;
	if ((procs && var->Type->Id == TID_PROC) || (!procs && var->Type->Id != TID_PROC)) {
	    if ((var->Flags & TF_EXTERN) && (var->Flags & VF_DECLD) && !(var->Flags & VF_EXTD))
		asm_extern(var);
	    if (var->Flags & VF_DECLD)
		var->Flags |= VF_EXTD;
	}
    }
}

void
AddStrList(name, len, label, iidx)
char *name;
long len;
long label;
long iidx;
{
    ExtStrNode *esn = AllocStructure(ExtStrNode);

    esn->Next = StrBase;
    esn->Str = name;
    esn->Len = len;
    esn->Label = label;
    esn->IIdx = iidx;

    /*
     * Internationalizable strings are always constant so we can have a
     * relocation reference and still support residentable code.
     */

    if (ConstCode || (iidx >= 0))
	esn->Flags = TF_CONST;
    else
	esn->Flags = 0;
    StrBase = esn;
}

void
DelStrList(label)
long label;
{
    ExtStrNode **ep;

    for (ep = &StrBase; *ep && (*ep)->Label != label; ep = &(*ep)->Next);
    if (*ep)
	*ep = (*ep)->Next;
}

void
DumpStrList()
{
    ExtStrNode *esn;

    for (esn = StrBase; esn; esn = esn->Next)
	asm_string(esn->Label, esn->Str, esn->Len, esn->Flags, esn->IIdx);
}
