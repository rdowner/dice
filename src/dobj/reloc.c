/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  RELOC.C
 *
 */

#include "defs.h"

Prototype void rel_init(void);
Prototype void ResetReloc(void);
Prototype void AddRelocInfo(short srcHunk, short dstHunk, int32_t size, short flags, int32_t offset, Symbol *sym);
Prototype RelocInfo *FindRelocOffset(int32_t offset, short hunkNo);
Prototype RelocInfo *FindRelocNext(RelocInfo *rel);
Prototype int LoadRelocData(FILE *fi, RelocInfo *r);

List    RelList;
RelocInfo   *RelOffCache;

void
rel_init(void)
{
    NewList(&RelList);
}

void
ResetReloc(void)
{
    RelocInfo *r;

    if (RelList.lh_Head == NULL)
        cerror(EFATAL, "Software Error, ResetReloc");

    while ((r = (RelocInfo *)RemHead(&RelList)) != NULL)
        free(r);
    RelOffCache = NULL;
}

void
AddRelocInfo(short srcHunk, short dstHunk, int32_t size, short flags, int32_t offset, Symbol *sym)
{
    RelocInfo *ri = malloc(sizeof(RelocInfo));
    RelocInfo *r;

    if (ri == NULL)
        cerror(EFATAL, "malloc Failed");

    clrmem(ri, sizeof(RelocInfo));
    ri->ri_Sym = sym;
    ri->ri_SrcHunk = srcHunk;
    ri->ri_DstHunk = dstHunk;
    ri->ri_RelocSize = size;
    ri->ri_RelocFlags= flags;
    ri->ri_SrcOffset = offset;

    for (r = (RelocInfo *)GetTail(&RelList); r; r = (RelocInfo *)GetPred((Node *)&r->ri_Node)) {
        if (r->ri_SrcOffset < ri->ri_SrcOffset)
            break;
    }
    Insert(&RelList, (Node *)&ri->ri_Node, (Node *)&r->ri_Node);
}

RelocInfo *
FindRelocOffset(int32_t offset, short hunkNo)
{
    RelocInfo *r = RelOffCache;

    if (r == NULL)
        r = (RelocInfo *)GetHead(&RelList);

    while (r && r->ri_SrcOffset >= offset)
        r = (RelocInfo *)GetPred((Node *)&r->ri_Node);

    if (r == NULL)
        r = (RelocInfo *)GetHead(&RelList);

    while (r && r->ri_SrcOffset < offset)
        r = (RelocInfo *)GetSucc((Node *)&r->ri_Node);

    while (r && r->ri_SrcHunk != hunkNo)
        r = (RelocInfo *)GetSucc((Node *)&r->ri_Node);

    if (r)
        RelOffCache = r;

    return(r);
}

RelocInfo *
FindRelocNext(RelocInfo *rel)
{
    RelocInfo *r;

    for (r = (RelocInfo *)GetSucc((Node *)&rel->ri_Node); r && r->ri_SrcHunk != rel->ri_SrcHunk; r = (RelocInfo *)GetSucc((Node *)&r->ri_Node));
    return(r);
}

int
LoadRelocData(FILE *fi, RelocInfo *r)
{
    switch(r->ri_RelocSize) {
    case 0:
        return(0);
    case 1:
        {
            ubyte c;
            fread(&c, 1, 1, fi);
            return(c);
        }
    case 2:
        {
            uword c;
            freadl(&c, 2, 1, fi);
            return(c);
        }
    case 4:
        {
            uint32_t c;
            freadl(&c, 4, 1, fi);
            return(c);
        }
    default:
        fseek(fi, r->ri_RelocSize, 1);
        return(0);
    }
}

