/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  SYM.C
 *
 *  Symbol Handling
 */

#include "defs.h"

Prototype int   hash(char *, short);
Prototype Sym   *FindSymbol(void *, short);
Prototype Sym   *CreateSymbol(void *, short, Hunk *, int32_t, int32_t);
Prototype void  SetSymbol(Sym *, Hunk *, int32_t, int32_t);

static Sym *SymHash[HSIZE];


#ifdef NO_ASM

int
hash(char *name, short len)
{
    int32_t hv = 0x1234FCB4;

    while (len) {
        hv = (hv >> 23) ^ (hv << 5) ^ *name;
        ++name;
        --len;
    }
    return(hv & HMASK);
}

#endif

Sym *
FindSymbol(void *name, short len)
{
    Sym *sym;

    len <<= 2;
    while (len && ((char *)name)[len-1] == 0)
        --len;

    for (sym = SymHash[hash((char *)name, len)]; sym; sym = sym->HNext) {
        if (sym->SymLen == len && strncmp((char *)name, sym->SymName, len) == 0) {
            dbprintf(1, ("FindSym SUCC %.*s\n", len, name));
            return(sym);
        }
    }
    dbprintf(8, ("FindSym FAIL %.*s\n", len, name));
    return(NULL);
}

Sym *
CreateSymbol(void *name, short len, Hunk *hunk, int32_t value, int32_t type)
{
    Sym *sym;
    Sym **ps;

    len <<= 2;
    while (len && ((char *)name)[len-1] == 0)
        --len;

    ++MemNumSyms;
    sym = zalloc(sizeof(Sym));
    ps = SymHash + hash((char *)name, len);
    sym->HNext = *ps;
    sym->SymName = (char *)name;
    sym->SymLen = len;
    sym->Hunk = hunk;
    sym->Value = value;
    sym->Type = type;
    *ps = sym;

    if (hunk /*&& SymOpt*/) {
        AddTail(&hunk->SymList, (Node *)&sym->Node);
        sym->Flags |= SYMF_SYMLIST;
    }

    dbprintf(1, ("CreSym type %-3d value %-3d %.*s\n", sym->Type, sym->Value, sym->SymLen, sym->SymName));
    return(sym);
}

void
SetSymbol(Sym *sym, Hunk *hunk, int32_t value, int32_t type)
{
    sym->Hunk = hunk;
    sym->Value = value;
    sym->Type = type;

    if (sym->Flags & SYMF_SYMLIST) {
        Remove((Node *)&sym->Node);
        sym->Flags &= ~SYMF_SYMLIST;
    }
    if (hunk /* && SymOpt*/) {
        sym->Flags |= SYMF_SYMLIST;
        AddTail(&hunk->SymList, (Node *)&sym->Node);
    }
}

