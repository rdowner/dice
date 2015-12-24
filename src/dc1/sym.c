/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  SYM.C
 *
 *  Symbol Handler
 */

#include "defs.h"

#define ResSymbol(name,lexid)		MakeSymbol(name, sizeof(name)-1, lexid, NULL)
#define ResSymbol3(name,lexid,lexdata)	MakeSymbol(name, sizeof(name)-1, lexid, (void *)lexdata)
#define ResType(name, type)		MakeSymbol(name, sizeof(name)-1, TokTypeId, type)

#define HSIZE	1024
#define HMASK	(HSIZE-1)

#define XSIZE	16
#define XMASK	(XSIZE-1)

static Symbol *SymHash[HSIZE];
static Symbol *SymHashX[XSIZE];

Prototype void InitSym(int);
Prototype Symbol *MakeSymbol(const char *, short, short, void *);
Prototype Symbol *PragmaSymbol(char *, short);
Prototype SemInfo *FindSymbolId(Symbol *, short);

Prototype long hash(const char *, short);

void
InitSym(int enab)
{
    static short Refs;

    if (enab == 1 && Refs++ == 0) {

	ResSymbol("sizeof",     TokSizeof);
	ResSymbol("typeof",     TokTypeof);

	ResSymbol3("signed",        TokTypeQual,    TF_SIGNED);
	ResSymbol3("unsigned",      TokTypeQual,    TF_UNSIGNED);
	ResSymbol3("volatile",      TokTypeQual,    TF_VOLATILE);
	ResSymbol3("const",         TokTypeQual,    TF_CONST);
	ResSymbol3("static",        TokTypeQual,    TF_STATIC);
	ResSymbol3("auto",          TokTypeQual,    TF_AUTO);
	ResSymbol3("extern",        TokTypeQual,    TF_EXTERN);
	ResSymbol3("register",      TokTypeQual,    TF_REGISTER);

	ResSymbol3("__autoinit",    TokTypeQual,    TF_AUTOINIT);
	ResSymbol3("__autoexit",    TokTypeQual,    TF_AUTOEXIT);
	ResSymbol3("__interrupt",   TokTypeQual,    TF_INTERRUPT);
	ResSymbol3("__noprof",      TokTypeQual,    TF_NOPROF);
	ResSymbol3("__regargs",     TokTypeQual,    TF_REGCALL);
	/*
	 *  replaced with pragmas
	 * ResSymbol3("__libcall",     TokTypeQual,    TF_LIBCALL);
	 */
	ResSymbol3("__stkargs",     TokTypeQual,    TF_STKCALL);
	ResSymbol3("__stkcheck",    TokTypeQual,    TF_STKCHECK);
	ResSymbol3("__nostkcheck",  TokTypeQual,    TF_NOSTKCHECK);
	ResSymbol3("__unaligned",   TokTypeQual,    TF_UNALIGNED);
	ResSymbol3("__near",        TokTypeQual,    TF_NEAR);
	ResSymbol3("__far",         TokTypeQual,    TF_FAR);
	ResSymbol3("__chip",        TokTypeQual,    TF_CHIP);
	ResSymbol3("__aligned",     TokTypeQual,    TF_ALIGNED);
	ResSymbol3("__geta4",       TokTypeQual,    TF_GETA4);
	ResSymbol3("__saveds",      TokTypeQual,    TF_GETA4);

	ResSymbol3("__shared",      TokTypeQual,    TF_SHARED);
	ResSymbol3("__config",      TokTypeQual,    TF_CONFIG);
#ifdef DYNAMIC
	ResSymbol3("__dynamic",     TokTypeQual,    TF_DYNAMIC);
#endif
	ResSymbol3("__asm",         TokTypeQual,    0);     /*  SAS/C compat */

	ResSymbol3("__D0",          TokRegQual,     RB_D0);
	ResSymbol3("__D1",          TokRegQual,     RB_D1);
	ResSymbol3("__D2",          TokRegQual,     RB_D2);
	ResSymbol3("__D3",          TokRegQual,     RB_D3);
	ResSymbol3("__D4",          TokRegQual,     RB_D4);
	ResSymbol3("__D5",          TokRegQual,     RB_D5);
	ResSymbol3("__D6",          TokRegQual,     RB_D6);
	ResSymbol3("__D7",          TokRegQual,     RB_D7);
	ResSymbol3("__A0",          TokRegQual,     RB_A0);
	ResSymbol3("__A1",          TokRegQual,     RB_A1);
	ResSymbol3("__A2",          TokRegQual,     RB_A2);
	ResSymbol3("__A3",          TokRegQual,     RB_A3);
	ResSymbol3("__A4",          TokRegQual,     RB_A4);
	ResSymbol3("__A5",          TokRegQual,     RB_A5);
	ResSymbol3("__A6",          TokRegQual,     RB_A6);
	ResSymbol3("__A7",          TokRegQual,     RB_A7);

	ResSymbol3("__d0",          TokRegQual,     RB_D0);
	ResSymbol3("__d1",          TokRegQual,     RB_D1);
	ResSymbol3("__d2",          TokRegQual,     RB_D2);
	ResSymbol3("__d3",          TokRegQual,     RB_D3);
	ResSymbol3("__d4",          TokRegQual,     RB_D4);
	ResSymbol3("__d5",          TokRegQual,     RB_D5);
	ResSymbol3("__d6",          TokRegQual,     RB_D6);
	ResSymbol3("__d7",          TokRegQual,     RB_D7);
	ResSymbol3("__a0",          TokRegQual,     RB_A0);
	ResSymbol3("__a1",          TokRegQual,     RB_A1);
	ResSymbol3("__a2",          TokRegQual,     RB_A2);
	ResSymbol3("__a3",          TokRegQual,     RB_A3);
	ResSymbol3("__a4",          TokRegQual,     RB_A4);
	ResSymbol3("__a5",          TokRegQual,     RB_A5);
	ResSymbol3("__a6",          TokRegQual,     RB_A6);
	ResSymbol3("__a7",          TokRegQual,     RB_A7);

	ResSymbol("break",          TokBreak);
	ResSymbol("case",           TokCase);
	ResSymbol("continue",       TokContinue);
	ResSymbol("default",        TokDefault);
	ResSymbol("do",             TokDo);
	ResSymbol("else",           TokElse);
	ResSymbol("for",            TokFor);
	ResSymbol("goto",           TokGoto);
	ResSymbol("if",             TokIf);
	ResSymbol("return",         TokReturn);
	ResSymbol("struct",         TokStruct);
	ResSymbol("enum",           TokEnum);
	ResSymbol("switch",         TokSwitch);
	ResSymbol("union",          TokUnion);
	ResSymbol("while",          TokWhile);
	ResSymbol("typedef",        TokTypeDef);
	ResSymbol("__breakpoint",   TokBreakPoint);

	ResType("void",     &VoidType);
	ResType("char",     &CharType);
	ResType("short",    &ShortType);
	ResType("int",      &LongType);
	ResType("long",     &LongType);
	ResType("float",    &FloatType);
	ResType("double",   &DoubleType);
    }
    if (enab == 0 && --Refs == 0) {
	;
    }
}

#ifdef NO_ASM

long
hash(const char *buf, short bytes)
{
    long v = 0x12ABE49F;

    while (bytes) {
	--bytes;
	v = (v >> 23) ^ (v << 5) ^ (ubyte)*buf;
	++buf;
    }
    return(v);
}

#endif

/*
 *  create symbol if it does not exist, return symbol if it does.  lexid
 *  only applies when creating a new symbol.
 */

Symbol *
MakeSymbol(const char *name, short len, short lexid, void *lexdata)
{
    long hv = hash(name, len);
    Symbol **psym = SymHash + (hv & HMASK);
    Symbol *sym;

    for (sym = *psym; sym; sym = *psym) {
	if (hv == sym->Hv && len == sym->Len && cmpmem(name, sym->Name, len) == 0) {
	    if ((sym->LexId & TOKF_PRIVATE) == 0)
		break;
	}
	psym = &sym->Next;
    }
    if (sym == NULL) {
	sym = AllocStructure(Symbol);
	*psym = sym;
	sym->Name = name;
	sym->Hv = hv;
	sym->Len = len;
	sym->LexId = lexid;
	sym->Data = lexdata;
	++SymAllocs;
    }
    return(sym);
}

Symbol *
PragmaSymbol(char *name, short len)
{
    long hv = hash(name, len);
    Symbol **psym = SymHashX + (hv & XMASK);
    Symbol *sym;

    for (sym = *psym; sym; sym = *psym) {
	if (hv == sym->Hv && len == sym->Len && cmpmem(name, sym->Name, len) == 0)
	    break;
	psym = &sym->Next;
    }
    if (sym == NULL) {
	sym = AllocStructure(Symbol);
	sym->Name = name;
	sym->Hv = hv;
	sym->Len = len;
	*psym = sym;
	++SymAllocs;
    }
    return(sym);
}


SemInfo *
FindSymbolId(Symbol *sym, short lexid)
{
    SemInfo *sem;

    for (sem = sym->SemBase; sem; sem = sem->Next) {
	if (sem->LexId == lexid)
	    break;
    }
    return(sem);
}

