/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */
/*
 *  TYPES.C
 */

/*
**      $Filename: types.c $
**      $Author: dice $
**      $Revision: 30.325 $
**      $Date: 1995/12/24 05:38:17 $
**      $Log: types.c,v $
 * Revision 30.325  1995/12/24  05:38:17  dice
 * .
 *
 * Revision 30.5  1994/06/13  18:37:41  dice
 * .
 *
 * Revision 30.0  1994/06/10  18:04:58  dice
 * .
 *
 * Revision 1.11  1994/04/15  21:19:38  jtoebes
 * Fix error message for undefined tag to correspond to the error message file.
 *
 * Revision 1.10  1993/11/22  00:28:36  jtoebes
 * Final cleanup to eliminate all cerror() messages with strings.
 *
 * Revision 1.9  1993/11/14  21:38:47  jtoebes
 * FIXED BUG01140 - DC1 is misaligning structure sizes.
 * Set the alignment size of the char array.
 *
 * Revision 1.8  1993/11/14  18:26:01  jtoebes
 * Fixed BUG00059 - Long Double constants not supported.
 * Slight kludge by setting the size to 8 so that it acts just like
 * a normal double.
 *
 * Revision 1.7  1993/11/14  17:20:35  jtoebes
 * Fix overzealous error messages for pointer mismatches.
 *
 * Revision 1.6  1993/10/17  11:02:49  jtoebes
 * FIXED BUG01133 - Strange warning about mismatch of return type for the assignment
 * of a function pointer.  This change now causes the compiler to tell you the type
 * that it was comparing with as well as the expected type.  This should help a lot
 * of people in figuring out what the error was.
 *
 * Revision 1.5  1993/09/06  21:39:04  jtoebes
 * Fix BUG01005 - Problems with typedef of a tentative volatile structure.
 * I am not 100% comfortable with this fix, but it does address the basic issue.
 *
**/

#include "defs.h"

Prototype int32_t NumTypesAlloc;

Prototype Type VoidType;
Prototype Type CharType;
Prototype Type ShortType;
Prototype Type LongType;
#ifdef SUPPORT_QUAD
Prototype Type LongLongType;
#endif
Prototype Type CharPtrType;
Prototype Type CharAryType;
Prototype Type VoidPtrType;
Prototype Type LongPtrType;

Prototype Type UCharType;
Prototype Type UShortType;
Prototype Type ULongType;
#ifdef SUPPORT_QUAD
Prototype Type ULongLongType;
#endif

Prototype Type FloatType;
Prototype Type DoubleType;
Prototype Type LongDoubleType;

Prototype Type DefaultProcType;

Prototype int   StackAlign;
Prototype int   CodeAlign;

typedef struct SUSym {
    struct SUSym *Next;
    struct Symbol *Sym;
    struct Type *Type;
} SUSym;

int32_t NumTypesAlloc;

#define HSIZE   256
#define HMASK   (HSIZE-1)

/*
 * 68K defaults
 */
int     PointerAlign = 2;
int     VoidAlign = 1;
int     CharAlign = 1;
int     ShortAlign = 2;
int     IntAlign = 2;
int     LongAlign = 2;
#ifdef SUPPORT_QUAD
int     QuadAlign = 2;
#endif
int     FloatAlign = 2;
int     DoubleAlign = 2;
int     LongDoubleAlign = 2;
int     BitFieldAlign = 2;
int     StackAlign = 4;
int     CodeAlign = 4;
int     StructAlign = 2;
int     ProcAlign = 4;

int     PointerSize = 4;
int     VoidSize = 0;
int     CharSize = 1;
int     ShortSize = 2;
int     IntSize = 4;
int     LongSize = 4;
#ifdef SUPPORT_QUAD
int     QuadSize = 4;
#endif
int     FloatSize = 4;
int     DoubleSize = 8;
int     LongDoubleSize = 8;
int     ProcSize = 4;   /* dummy, just shouldn't be 0 */

int     Size4 = 4;

Type VoidType       = { TID_INT,    &VoidAlign,         0,      &VoidSize };
Type CharType       = { TID_INT,    &CharAlign,         0,      &CharSize };
Type ShortType      = { TID_INT,    &ShortAlign,        0,      &ShortSize };
Type LongType       = { TID_INT,    &IntAlign,          0,      &IntSize };
#ifdef SUPPORT_QUAD
Type LongLongType   = { TID_INT,    &QuadAlign,         0,      &QuadSize };
#endif
Type CharPtrType    = { TID_PTR,    &IntAlign,          TF_UNSIGNED, &IntSize };
Type CharAryType    = { TID_ARY,    &CharAlign,         TF_UNSIGNED, &IntSize };
Type VoidPtrType    = { TID_PTR,    &PointerAlign,      TF_UNSIGNED, &PointerSize };
Type LongPtrType    = { TID_PTR,    &PointerAlign,      TF_UNSIGNED, &LongSize };

Type UCharType      = { TID_INT,    &CharAlign,         TF_UNSIGNED, &CharSize };
Type UShortType     = { TID_INT,    &ShortAlign,        TF_UNSIGNED, &ShortSize };
Type ULongType      = { TID_INT,    &LongAlign,         TF_UNSIGNED, &LongSize };
#ifdef SUPPORT_QUAD
Type ULongLongType  = { TID_INT,    &QuadAlign,         TF_UNSIGNED, &QuadSize };
#endif

Type FloatType      = { TID_FLT,    &FloatAlign,        0,      &FloatSize };
Type DoubleType     = { TID_FLT,    &DoubleAlign,       0,      &DoubleSize };
Type LongDoubleType = { TID_FLT,    &LongDoubleAlign,   0,      &LongDoubleSize };
Type DefaultProcType= { TID_PROC,   &ProcAlign,         0,      &ProcSize };

static Type *SBitfieldType[32];
static Type *UBitfieldType[32];

static SUSym *SUHash[HSIZE];

Prototype void InitTypes(int);
Prototype void LooseTypeLink(Type *, Type *);
Prototype void TypeLink(Type *, Type *);
Prototype void TypeLinkEnd(Type *, Type *);
Prototype Type *TypeToPtrType(Type *);
Prototype Type *TypeToAryType(Type *, Exp *, int32_t);
Prototype Type *TypeToProcType(Type *, Var **, short, int32_t);
Prototype Type *TypeToQualdType(Type *, int32_t);
Prototype Type *FindStructUnionType(Symbol *, int32_t);
Prototype Symbol *FindStructUnionTag(Type *);
Prototype int32_t FindStructUnionElm(Type *, Exp *, int *);
Prototype Type *MakeStructUnionType(Symbol *, int32_t);
Prototype void SetStructUnionType(Type *, Var **, int32_t, int32_t);
Prototype Type *MakeBitfieldType(int32_t, int);
Prototype Type *FindEnumType(Symbol *);
Prototype Type *MakeEnumType(Symbol *);
Prototype void AddEnumIdent(Type *, Symbol *, int32_t);
Prototype Type *ActualReturnType(Stmt *, Type *, Type *);
Prototype Type *ActualPassType(Type *, Type *, int);
Prototype Type *ActualArgType(Type *);
Prototype void CheckPointerType(int32_t, int32_t, Type *, Type *);
Prototype void GenerateRegSpecOutput(Var *);
Prototype void ScanStructUnionTypes(void (*func)(Type *type, const char *name, int flags));
Prototype void Undefined_Tag(Type *, Symbol *, int32_t);
Prototype int *AllocInt(int n);

int IntConstAry[64];

void
InitTypes(int enab)
{
    static short Refs;
    int i;

    /*
     * AllocInt preload
     */
    for (i = 0; i < arysize(IntConstAry); ++i)
        IntConstAry[i] = i;

    /*
     * Adjust alignments and sizes when in -CTOD mode
     */
    if (CToDClass) {
        PointerAlign = sizeof(void *);
        IntAlign = sizeof(int);
        LongAlign = sizeof(int);
#ifdef SUPPORT_QUAD
        QuadAlign = sizeof(int);
#endif
        FloatAlign = sizeof(int);
        DoubleAlign = sizeof(int);
        LongDoubleAlign = sizeof(int);
        PointerSize = sizeof(void *);
        IntSize = sizeof(int);
        LongSize = sizeof(long);        /* Rune ifc to machine native */
#ifdef SUPPORT_QUAD
        QuadSize = sizeof(quad_t);      /* Rune ifc to machine native */
#endif
        FloatSize = sizeof(float);
        DoubleSize = sizeof(double);
        LongDoubleSize = sizeof(long double);
    }

    DefaultProcType.Args = -1;
    if (enab == 1 && Refs++ == 0) {
        TypeLink(&CharType, &CharPtrType);
        TypeLink(&CharType, &CharAryType);
        TypeLink(&VoidType, &VoidPtrType);
        TypeLink(&LongType, &LongPtrType);
        TypeLink(&LongType, &DefaultProcType);
        LooseTypeLink(&CharType,  &UCharType);
        LooseTypeLink(&ShortType, &UShortType);
        LooseTypeLink(&LongType,  &ULongType);
#ifdef SUPPORT_QUAD
        LooseTypeLink(&LongLongType,  &ULongLongType);
#endif
    }
    if (enab == 0 && --Refs == 0) {
        ;
    }
}

int *
AllocInt(int n)
{
    int *ptr;

    if ((unsigned int)n < arysize(IntConstAry))
        return(&IntConstAry[n]);
    ptr = malloc(sizeof(int));
    *ptr = n;
    return(ptr);
}

void
LooseTypeLink(roottype, qualtype)
Type *qualtype;
Type *roottype;
{
    qualtype->Next = roottype->PList;
    roottype->PList = qualtype;
}

void
TypeLink(subtype, partype)
Type *subtype;
Type *partype;
{
    partype->SubType = subtype;
    partype->Next = subtype->PList;
    subtype->PList = partype;
}

void
TypeLinkEnd(subtype, partype)
Type *subtype;
Type *partype;
{
    Type **pt;

    for (pt = &subtype->PList; *pt; pt = &(*pt)->Next);

    partype->SubType = subtype;
    partype->Next = *pt;
    *pt = partype;
}


/*
 *  Note that we must also compare SubType to type because there are
 *  relationships other than parent-child stored here (TypeToQualdType)
 */

Type *
TypeToPtrType(type)
Type *type;
{
    Type *t;

    for (t = type->PList; t; t = t->Next) {
        if (t->Id == TID_PTR && t->SubType == type)
            return(t);
    }
    ++NumTypesAlloc;
    t = AllocStructure(Type);
    t->Id = TID_PTR;
    t->Align = &PointerAlign;
    t->Size = &PointerSize;
    TypeLink(type, t);
    return(t);
}

Type *
TypeToAryType(type, exp, entries)
Type *type;
Exp *exp;
int32_t entries;
{
    int32_t size;
    Type *t;

    if (exp)
        entries = ExpToConstant(exp);
    size = entries * *type->Size;
    if (entries) {
        for (t = type->PList; t; t = t->Next) {
            if (t->Id == TID_ARY && size == *t->Size && t->SubType == type)
                return(t);
        }
    }
    ++NumTypesAlloc;
    t = AllocStructure(Type);
    t->Id = TID_ARY;
    t->Align = type->Align;
    t->Size = AllocInt(size);
    if (type->Flags & TF_CONST)     /*  array of const is const array   */
        t->Flags |= TF_CONST;
    TypeLink(type, t);
    return(t);
}

/*
 *  Create a procedural type.
 *
 *  Here we try to optimize storage by searching if the procedural type is
 *  already declared.  If the 'vars' arg has symbols, however, we can't do
 *  that because the procedure generation needs to know the variable names.
 *
 *  if 'vars' has no symbols then we don't care if we optimize by giving it
 *  some, because they will never be used.  However, if vars does have
 *  symbols than the type list might be munged due to prototyping (as well
 *  as needing to preserve said symbols), so we make a copy.
 */

Type *
TypeToProcType(Type *type, Var **vars, short n, int32_t flags)
{
    Type *t;

    for (t = type->PList; t; t = t->Next) {
        if (t->Id == TID_PROC && t->SubType == type) {
            if (t->Args == n && (t->Flags & -1) == (flags & -1)) {  /* XXX */
                short i;
                for (i = 0; i < n; ++i) {
                    Var *v1 = vars[i];
                    Var *v2 = t->Vars[i];

                    if (v1->Type != v2->Type || v1->u.Block != v2->u.Block || v1->Sym)
                        break;
                    if ((v1->RegFlags ^ v2->RegFlags) & (RF_REGISTER|RF_REGMASK))
                        break;
                }
                if (n < 0 || i == n)
                    return(t);
            }
        }
    }
    {
        short i;
        Var **tvars = NULL;
        Var *xvars = NULL;

        if (n > 0) {
            tvars = zalloc(sizeof(Var *) * n);
            xvars = zalloc(sizeof(Var) * n);
        }
        for (i = 0; i < n; ++i) {
            *xvars = *vars[i];
            tvars[i] = xvars++;
        }
        ++NumTypesAlloc;
        t = AllocStructure(Type);
        t->Id = TID_PROC;
        t->Align = &ProcAlign;
        t->Size = &Size4; /* non-zero so asm_getind() does not complain */
        t->Vars = tvars;
        t->Args = n;
        t->Flags= flags;
        TypeLinkEnd(type, t);
    }
    return(t);
}

/*
 *  Qualified types are exactly their unqualified brothers
 *  except the Flags and PList fields are different
 */

Type *
TypeToQualdType(type, flags)
Type *type;
int32_t  flags;
{
    Type *t;

    for (t = type->PList; t; t = t->Next) {
        if (*t->Align == *type->Align && t->Flags == flags && *t->Size == *type->Size &&
            t->Id == type->Id && t->SubType == type->SubType && t->Args == type->Args &&
            t->Vars == type->Vars
        ) {
            return(t);
        }
    }

    /*
     *  note that SubType is not altered.. what if the type is a pointer or
     *  something!
     *
     *  Note that the t->Size pointer is shared with the parent.
     */
    ++NumTypesAlloc;
    t = AllocStructure(Type);
    *t = *type;
    t->Flags = flags;
    t->Next = type->PList;
    t->PList = NULL;
    t->BasedOnType = type;
    type->PList = t;
    return(t);
}

/*
 *  STRUCTURES AND UNIONS
 */


Type *
FindStructUnionType(Symbol *sym, int32_t isUnion)
{
    SUSym *su = SUHash[sym->Hv & HMASK];

    while (su && su->Sym != sym)
        su = su->Next;
    if (su) {
        return(su->Type);
    }
    return(NULL);
}

Symbol *
FindStructUnionTag(type)
Type *type;
{
    SUSym *su;
    short i;

    for (i = 0; i < HSIZE; ++i) {
        for (su = SUHash[i]; su; su = su->Next) {
            if (su->Type == type)
                return(su->Sym);
        }
    }
    return(NULL);
}

int32_t
FindStructUnionElm(type, exp, pbfo)
Type *type;
Exp *exp;
int *pbfo;
{
    short i;

    exp->ex_Type = NULL;
    if (type->Id == TID_PTR)
        type = type->SubType;

    if (type->Id != TID_STRUCT && type->Id != TID_UNION) {
        yerror(exp->ex_LexIdx, EERROR_NOT_STRUCT_UNION);
        return(0);
    }
    for (i = 0; i < type->Args; ++i) {
        Var *var = type->Vars[i];
        if (var->Sym == exp->ex_Symbol) {
            exp->ex_Type = var->Type;
            *pbfo = var->u.BOffset;
            return(var->var_Stor.st_Offset);
        }
    }
    yerror(exp->ex_LexIdx, EERROR_UNDEFINED_ELEMENT, SymToString(exp->ex_Symbol));
    return(0);
}

/*
 *  sym can be NULL!
 *
 */

Type *
MakeStructUnionType(sym, isUnion)
Symbol *sym;
int32_t isUnion;
{
    SUSym **psu = &SUHash[(sym) ? (sym->Hv & HMASK) : 0];
    SUSym *su = AllocStructure(SUSym);
    Type *t = AllocStructure(Type);

    ++NumTypesAlloc;
    t->Id = (isUnion) ? TID_UNION : TID_STRUCT;
    t->Size = AllocInt(0);
    t->Align= AllocInt(0);

    su->Next = *psu;
    su->Sym  = sym;
    su->Type = t;
    *psu = su;

    return(t);
}

void
SetStructUnionType(t, vars, nv, flags)
Type *t;
Var **vars;
int32_t nv;
int32_t flags;
{
    int32_t size;
    short boffset;
    short align;
    short i;
    Type *type;

    t->Vars = vars;
    t->Args = nv;

    align = StructAlign;
    size = 0;
    boffset = IntSize * 8;
    if (flags & TF_UNALIGNED)
        align = 1;

    for (i = 0; i < nv; ++i) {
        Var *var = vars[i];
        type = var->Type;

        if (var->Flags & TF_ALIGNED) {
            size = Align(size, 4);
            boffset = IntSize * 8; /* XXX */
        }
        if ((type->Id == TID_STRUCT || type->Id == TID_UNION) && *type->Size == 0)
        {
            Undefined_Tag(type, NULL, LFBase ? LFBase->lf_Index: 0);
        }

        if (align < *type->Align && !(flags & TF_UNALIGNED))
            align = *type->Align;
        if (t->Id == TID_STRUCT) {
            if (type->Id == TID_BITFIELD) {
                /*
                 *  alignment if bitfield does not fit or : 0 field.
                 */
                if (*type->Size > boffset || *type->Size == 0) {
                    if (boffset >= BitFieldAlign * 8) {
                        size += BitFieldAlign;
                        boffset = IntSize * 8;
                    } else {
                        size += IntSize;
                        boffset = IntSize * 8;
                    }
                }
                size = Align(size, *type->Align);
                boffset -= *type->Size;
                var->u.BOffset = boffset;
                var->var_Stor.st_Offset = size;
            } else {
                if (boffset != IntSize * 8) {
                    if (boffset >= BitFieldAlign * 8) {
                        size += BitFieldAlign;
                        boffset = IntSize * 8;
                    } else {
                        size += IntSize;
                        boffset = IntSize * 8;
                    }
                }
                if (!(flags & TF_UNALIGNED))
                    size = Align(size, *type->Align);
                var->var_Stor.st_Offset = size;
                size += *type->Size;
            }
        } else {
            if (type->Id == TID_BITFIELD) {
                var->u.BOffset = 0;
                if (boffset > IntSize * 8 - *type->Size)
                    boffset = IntSize * 8 - *type->Size;
            } else {
                if (size < *type->Size)
                    size = *type->Size;
            }
        }
    }
    if (boffset != IntSize * 8) {    /*  finish up bitfield  */
        if (t->Id == TID_STRUCT) {
            if (boffset >= BitFieldAlign * 8)
                size += BitFieldAlign;
            else
                size += IntSize;
        } else {
            if (boffset >= BitFieldAlign * 8 && size < StructAlign)
                size = StructAlign;
            if (boffset < BitFieldAlign * 8 && size < IntSize)
                size = IntSize;
        }
    }
    size = Align(size, align);
    t->Size = AllocInt(size);
    t->Align= AllocInt(align);

    /* We need to propagate the information to all the other copies of this type */
    for (type = t->PList; type; type = type->Next)
    {
        if (*type->Size) continue;
        if (type->Id != t->Id) continue;
        if (type->SubType != t->SubType) continue;
        type->Align = t->Align;
        type->Size  = t->Size;
        type->Args  = t->Args;
        type->Vars  = t->Vars;
    }
}

/*
 *  bitfields
 */

Type *
MakeBitfieldType(flags, bits)
int32_t flags;
int bits;
{
    Type **pt = ((flags & TF_UNSIGNED) ? UBitfieldType : SBitfieldType) + bits;
    Type *t;

    if (*pt == NULL) {
        *pt = t = AllocStructure(Type);
        t->Id = TID_BITFIELD;
        t->Flags = flags & TF_UNSIGNED;
        t->Size = AllocInt(bits);       /*  only type whos size is in bits */
        t->Align= &BitFieldAlign;
    }
    return(*pt);
}

/*
 *  ENUMERATIONS
 */

Type *
FindEnumType(name)
Symbol *name;
{
    return(&LongType);
}

Type *
MakeEnumType(name)
Symbol *name;
{
    return(&LongType);
}

void
AddEnumIdent(type, sym, value)
Type *type;
Symbol *sym;
int32_t value;
{
    SemanticAdd(sym, TokEnumConst, (void *)(intptr_t)value);
}

Type *
ActualReturnType(stmt, proctype, type)
Stmt *stmt;
Type *proctype;
Type *type;
{
    dbprintf(("proctype args=%d flags=%08lx\n", proctype->Args, proctype->Flags));

    switch(type->Id) {
    case TID_INT:
        if (*type->Size == 0)
            return(&VoidType);
        if (*type->Size < 4)
            return(&LongType);  /* XXX 68K C, int32_t == int */
        /* fall through return the actual type */
    case TID_PTR:
        return(type);
    case TID_FLT:
        return(type);
        /*
        if ((proctype->Flags & TF_PROTOTYPE) || type != &FloatType)
            return(type);
        return(&DoubleType);
        */
    case TID_STRUCT:
    case TID_UNION:
        return(type);
    case TID_ARY:
    case TID_PROC:
        yerror(stmt->st_LexIdx, EERROR_UNSUPPORTED_RETURN_TYPE);
        return(type);
    }
    dbprintf(("ActualReturnType: unknown type id %d\n", type->Id));
    Assert(0);
    return(0);  /* not reached */
}

/*
 *  Returns actual type of arg being passed.  GenCall() always pushes
 *  longs, but this determines what kind of extending is required.
 *
 *  note:   isn't careful about qualifiers
 */

Type *
ActualPassType(proctype, type, afterEnd)
Type *proctype;
Type *type;
int afterEnd;
{
    switch(type->Id) {
    case TID_BITFIELD:
        type = &LongType;
        break;
    case TID_INT:       /*  automatically cast to int32_t by GenCall   */
        if (afterEnd && *type->Size != IntSize) {
            type = &LongType;
            break;
        }
    case TID_PTR:
        break;
    case TID_FLT:
        if (afterEnd && type == &FloatType)
            return(&DoubleType);
        if ((proctype->Flags & TF_PROTOTYPE) || type != &FloatType)
            return(type);
        return(&DoubleType);
    case TID_ARY:
        return(TypeToPtrType(type->SubType));
    case TID_PROC:
    case TID_STRUCT:
    case TID_UNION:
        break;
    default:
        dbprintf(("ActualPassType: unknown type id %d\n", type->Id));
        Assert(0);
        break;
    }
    return(type);
}

/*
 *  Only called for non-prototyped (old style) type declarations.
 *  Integer types are already properly handled, we do NOT force integer
 *  arguments declared as shorts or chars to be accessed as longs!
 *
 *  However, floating point is a different story.  An old style fp argument
 *  declared as float is actually a double and unless we convert it to a
 *  float on every access (and convert assignments to doubles on every
 *  assign) we must silently change the float to a double type.
 */

Type *
ActualArgType(type)
Type *type;
{
    if (type->Id == TID_FLT) {
        if (type == &FloatType)
            type = &DoubleType;
    }
    return(type);
}

/*
 *  Returns whether two pointers are the same type or either one is
 *  a (void *) (pointer to anything).  If they do not match, it issues an
 *  error message indicating why.
 */

void CheckPointerType(olexIdx, lexIdx, t1, t2)
int32_t olexIdx;
int32_t lexIdx;
Type *t1;
Type *t2;
{
    if (t1 == t2 ||
        t1 == &VoidPtrType ||
        t2 == &VoidPtrType)
        return;

    /* Well, they don't strictly match.  See if we just happened to have */
    /* a simple type that has a difference of opinion about const.  You  */
    /* can assign a non-const pointer to a const pointer                 */

    if (*t1->Size == *t2->Size)
/*        && (((t1->Flags ^ t2->Flags) & ~TF_COMPAREQUALS) == 0)) */
    {
        /* Well, we have a good chance of them being the same.  See if we */
        /* have a subtype that also is compatible.                        */
        if (t1->SubType && t2->SubType)
        {
            if (t1->SubType->Id == TID_PROC)
               CompareTypes(olexIdx, lexIdx, t1->SubType, t2->SubType);
            else
            {
               if (*t1->SubType->Size != 0 && *t2->SubType->Size != 0)
                   CheckPointerType(olexIdx, lexIdx, t1->SubType, t2->SubType);
            }
            return;
        }
        if ((t1->SubType == NULL) && (t2->SubType == NULL))
            return;
    }

    /* Arrays and pointers are equivalent */
    if (t1->Id == TID_PTR && t2->Id == TID_ARY) return;
    if (t1->Id == TID_ARY && t2->Id == TID_PTR) return;

    yerror(lexIdx,  EWARN_PTR_PTR_MISMATCH, TypeToProtoStr(t1, 0));
    yerror(olexIdx, EWARN_DOES_NOT_MATCH,   TypeToProtoStr(t2, 0));
}

void
GenerateRegSpecOutput(var)
Var *var;
{
    char ano[16];
    short i;

    Assert(var->Sym);

    if (RegSpecOutputOpt == 1) {
        char prgno[16];
        void *pragma_call = TestPragmaCall(var, prgno);

        if (pragma_call || (var->Flags & TF_REGCALL)) {
            printf("##regspec @%s(", SymToString(var->Sym));

            RegCallOrder(var->Type, ano, ((pragma_call) ? prgno : NULL));
            for (i = 0; i < 16 && ano[i] >= 0; ++i) {
                if (i)
                    printf(",");
                if (ano[i] < RB_ADDR)
                    printf("D%d", ano[i]);
                else
                    printf("A%d", ano[i] - RB_ADDR);
            }
            puts(")");
        } else {
            printf("##regspec _%s(*)\n", SymToString(var->Sym));
        }
    } else {
        short i;

        printf("##typespec (%s)(%s)", SymToString(var->Sym), TypeToProtoStr(var->Type->SubType, 0));
        for (i = 0; i < var->Type->Args; ++i)
            printf("(%s)", TypeToProtoStr(var->Type->Vars[i]->Type, 0));
        puts("");
    }
}

void
ScanStructUnionTypes(void (*func)(Type *type, const char *name, int flags))
{
    SUSym *su;
    short i;

    for (i = 0; i < HSIZE; ++i) {
        for (su = SUHash[i]; su; su = su->Next) {
            char *name;

            if (su->Sym == NULL)
                continue;

            asprintf(&name, "%*.*s", su->Sym->Len, su->Sym->Len, su->Sym->Name);
            func(su->Type, name, SSCAN_TOP|SSCAN_NAME);
            free(name);
        }
    }
}


/*
 * Outputs a message indicating an invalid type
 */
void Undefined_Tag(type, sym, lexIdx)
Type *type;
Symbol *sym;
int32_t lexIdx;
{
    if (sym == NULL)
        sym = FindStructUnionTag(type);

    yerror(lexIdx, EERROR_UNDEFINED_TAG, SymToString(sym));
}
