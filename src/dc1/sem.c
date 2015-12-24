/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  SEM.C
 *
 *  Semantics junk.  Doesn't let you overload reserved tokens (that have no
 *  semantic lists and are not TokId)
 */

#include "defs.h"

static short SemLevel = 0;
static SemInfo *MasterBase;

Prototype void SemanticLevelDown(void);
Prototype void SemanticLevelUp(void);
Prototype void SemanticAddTop(Symbol *, short, void *);
Prototype void SemanticAddTopBlock(Symbol *, short, void *);
Prototype void SemanticAdd(Symbol *, short, void *);
Prototype void SemanticScanTopVars(int (*func)(Var *var, Type *type, const char *name, int flags));

void
SemanticLevelDown()
{
    ++SemLevel;
}

void
SemanticLevelUp()
{
    SemInfo *sem;
    Symbol *sym;

    while ((sem = MasterBase) && sem->SemLevel == SemLevel) {
	MasterBase = sem->MasterNext;
	if (sem->Next)
	    sem->Next->Prev = sem->Prev;
	sym = sem->Sym;
	if (sym->SemBase == (void *)sem && (sem->LexId & TOKF_PRIVATE) == 0) {
	    if (sem->Next) {
		sym->LexId= sem->Next->LexId;
		sym->Data = sem->Next->Data;
	    } else {
		sym->LexId= TokId;
		sym->Data = NULL;
	    }
	}
	*sem->Prev = sem->Next;
	/*zfree(sem);*/
    }
    --SemLevel;
}

void
SemanticAddTop(Symbol *sym, short lexid, void *data)
{
    SemInfo *sem = AllocStructure(SemInfo);

    sem->Sym = sym;
    sem->LexId = lexid;
    sem->Data  = data;

    {
	SemInfo **sp;
	for (sp = &MasterBase; *sp; sp = &(*sp)->MasterNext);
	sem->MasterNext = *sp;
	sem->SemLevel = 1;
	*sp  = sem;
    }
    {
	SemInfo **sp;
	for (sp = (SemInfo **)&sym->SemBase; *sp; sp = &(*sp)->Next);
	sem->Prev = sp;
	sem->Next = NULL;
	*sp  = sem;
    }
    if ((lexid & TOKF_PRIVATE) == 0 && sym->SemBase == (void *)sem) {
	sym->LexId = sem->LexId;
	sym->Data  = sem->Data;
    }
}

void
SemanticAddTopBlock(Symbol *sym, short lexid, void *data)
{
    SemInfo *sem = AllocStructure(SemInfo);

    sem->Sym = sym;
    sem->LexId = lexid;
    sem->Data  = data;
    sem->SemLevel = 2;

    {
	SemInfo **sp;
	SemInfo *s;

	for (sp = &MasterBase; (s = *sp) != NULL; sp = &s->MasterNext) {
	    if (s->SemLevel <= 2)
		break;
	}
	sem->MasterNext = *sp;
	*sp  = sem;
    }
    {
	SemInfo **sp;
	SemInfo *s;
	for (sp = (SemInfo **)&sym->SemBase; (s = *sp) != NULL; sp = &s->Next) {
	    if (s->SemLevel <= 2)
		break;
	}
	sem->Prev = sp;
	sem->Next = *sp;
	*sp  = sem;
    }
    if ((lexid & TOKF_PRIVATE) == 0 && sym->SemBase == (void *)sem) {
	sym->LexId = sem->LexId;
	sym->Data  = sem->Data;
    }
}


void
SemanticAdd(Symbol *sym, short lexid, void *data)
{
    SemInfo *sem;

    if (SemLevel > 1)
	sem = AllocTmpStructure(SemInfo);
    else
	sem = AllocStructure(SemInfo);


    sem->Sym = sym;
    if ((lexid & TOKF_PRIVATE) == 0) {
	sym->LexId = sem->LexId = lexid;
	sym->Data  = sem->Data	= data;
    }

    sem->SemLevel = SemLevel;
    sem->MasterNext = MasterBase;
    sem->Next = (SemInfo *)sym->SemBase;
    sem->Prev = (SemInfo **)&sym->SemBase;
    MasterBase = sem;

    /*
     *	check for duplicate symbols at same semantic level and for
     *	overides of procedure arguments
     */

    {
	SemInfo *scan;

	for (scan = sem->Next; scan; scan = scan->Next) {
	    if (scan->SemLevel == sem->SemLevel)
		zerror(EWARN_DUPLICATE_SYMBOL, sym->Len, sym->Name);
	    else if (scan->SemLevel == 2)
		zerror(EWARN_VARIABLE_OVERIDES_ARG, sym->Len, sym->Name);
	}
    }

    sym->SemBase = (void *)sem;
}

void
SemanticScanTopVars(int (*func)(Var *var, Type *type, const char *name, int flags))
{
    SemInfo *sem;

    for (sem = MasterBase; sem; sem = sem->MasterNext) {
	if (sem->LexId == TokVarId) {
	    Var *var = sem->Data;
	    char *name;

	    asprintf(&name, "%*.*s",
		sem->Sym->Len, sem->Sym->Len, sem->Sym->Name);
	    func(var, var->Type, name, SSCAN_NAME|SSCAN_TOP);
	    free(name);
	}
    }
}

