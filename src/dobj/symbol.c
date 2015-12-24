/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  SYMBOL.C
 *
 */

#include "defs.h"

List	SymList;
Symbol *SymOffCache;

Prototype void sym_init(void);
Prototype void ResetSymbol(void);
Prototype Symbol *AddSymbol(Symbol *sym);
Prototype Symbol *FindSymbolName(char *name);
Prototype Symbol *FindSymbolOffset(int32_t offset, short hunkNo);
Prototype Symbol *FindSymbolNext(Symbol *sym);
Prototype Symbol *FindSymbolPrev(Symbol *sym);

void
sym_init(void)
{
    NewList(&SymList);
}

void
ResetSymbol(void)
{
    Symbol *s;

    if (SymList.lh_Head == NULL)
	cerror(EFATAL, "Software Error, ResetSymbolHash");

    while ((s = RemHead(&SymList)) != NULL)
	free(s);
    SymOffCache = NULL;
}

Symbol *
AddSymbol(Symbol *sym)
{
    Symbol *s;

    if (DDebug)
	printf("AddSymbol type=%d value=%d %s\n", sym->sm_Type, sym->sm_Value, sym->sm_Name);
    if ((s = FindSymbolName(sym->sm_Name)) != NULL) {
	if (s->sm_Type == 1 || s->sm_Type == 2) {
	    if (sym->sm_Type == 1 || sym->sm_Type == 2)
		cerror(EWARN, "Duplicate symbol def: %s, hunks %d and %d", s->sm_Name, s->sm_DefHunk, sym->sm_DefHunk);
	    free(sym);
	    return(s);
	} else {
	    Remove((Node *)&s->sm_Node);
	    *s = *sym;
	}
	free(sym);
	sym = s;
    }
    if (sym->sm_Type > 2)
	sym->sm_DefHunk = -1;
    for (s = GetTail(&SymList); s; s = GetPred((Node *)&s->sm_Node)) {
	if (s->sm_Value < sym->sm_Value)
	    break;
    }
    Insert(&SymList, (Node *)&sym->sm_Node, (Node *)&s->sm_Node);
    return(sym);
}

Symbol *
FindSymbolName(char *name)
{
    Symbol *s;

    for (s = GetHead(&SymList); s; s = GetSucc((Node *)&s->sm_Node)) {
	if (strcmp(s->sm_Name, name) == 0)
	    return(s);
    }
    return(NULL);
}

/*
 *  find nearest offset >= offset
 */

Symbol *
FindSymbolOffset(int32_t offset, short hunkNo)
{
    Symbol *s = SymOffCache;

    if (s == NULL)
	s = GetHead(&SymList);

    while (s && s->sm_Value >= offset)
	s = GetPred((Node *)&s->sm_Node);

    if (s == NULL)
	s = GetHead(&SymList);

    while (s && s->sm_Value < offset)
	s = GetSucc((Node *)&s->sm_Node);

    while (s && s->sm_DefHunk != hunkNo)
	s = GetSucc((Node *)&s->sm_Node);

    if (s)
	SymOffCache = s;

    return(s);
}

Symbol *
FindSymbolNext(Symbol *sym)
{
    Symbol *s;

    for (s = GetSucc((Node *)&sym->sm_Node); s && s->sm_DefHunk != sym->sm_DefHunk; s = GetSucc((Node *)&s->sm_Node));
    return(s);
}

Symbol *
FindSymbolPrev(Symbol *sym)
{
    Symbol *s;

    for (s = GetPred((Node *)&sym->sm_Node); s && s->sm_DefHunk != sym->sm_DefHunk; s = GetPred((Node *)&s->sm_Node));
    return(s);
}

