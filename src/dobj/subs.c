/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  SUBS.C
 */

#include "defs.h"
#include <stdarg.h>

Prototype int cprintf(const char *ctl, ...);
Prototype int csprintf(char *buf, const char *ctl, ...);
Prototype char * RelocToStr(RelocInfo *r, int offset, int ext, int size, int srcHunk);
Prototype int32_t FixRelocOffset(RelocInfo *r, int32_t offset);
Prototype void cerror(int code, char *ctl, ...);
Prototype int freadl(void *buf, int elsize, int nel, FILE *fi);

/*
 *  This is so I can compile under non-ANSI unix which doesn't handle
 *  the return code properly.  DICE does it right, so for DICE compilation
 *  one can simply use printf() and sprintf().
 */

#ifndef _DCC

int
cprintf(const char *ctl, ...)
{
    va_list va;
    int n;
    char buf[1024];

    va_start(va, ctl);
    vsprintf(buf, ctl, va);
    va_end(va);
    n = strlen(buf);
    fwrite(buf, n, 1, stdout);
    return(n);
}

int
csprintf(char *buf, const char *ctl, ...)
{
    va_list va;
    int n;

    va_start(va, ctl);
    vsprintf(buf, ctl, va);
    n = strlen(buf);
    va_end(va);
    return(n);
}

#endif

/*
 *  Convert relocation info to string
 */

char *
RelocToStr(RelocInfo *r, int offset, int ext, int size, int srcHunk)
{
    Symbol *s;
    static char Buf[SMAX_BUF];

    if (r == NULL) {
	Symbol *sym = NULL;

	if (srcHunk >= 0)
	    sym = FindSymbolOffset(offset, srcHunk);
	if (sym && sym->sm_Value != offset)
	    sym = NULL;

	if (sym) {
	    sprintf(Buf, "%s", sym->sm_Name);
	} else {
	    switch(size) {
	    case 1:
		sprintf(Buf, "%02x", offset & 0xFF);
		break;
	    case 2:
		sprintf(Buf, "%04x", offset & 0xFFFF);
		break;
	    case 4:
		sprintf(Buf, "%08x", offset);
		break;
	    default:
		sprintf(Buf, "%04x", offset);
		break;
	    }
	}
	return(Buf);
    }

    if ((s = r->ri_Sym) == NULL) {
	s = FindSymbolOffset(offset, r->ri_DstHunk);
	if (s && s->sm_Value != offset) {
	    s = FindSymbolPrev(s);
	    while (s && s->sm_Type > 1)
		s = FindSymbolPrev(s);
	}
	if (s)
	    offset -= s->sm_Value;
    }
    if (s) {
	if (strlen(s->sm_Name) > sizeof(Buf) - 16)
	    s->sm_Name[sizeof(Buf) - 16] = 0;
	if (offset) {
	    switch(size) {
	    case 1:
		sprintf(Buf, "%s+%02x", s->sm_Name, offset & 0xFF);
		break;
	    case 2:
		sprintf(Buf, "%s+%04x", s->sm_Name, offset & 0xFFFF);
		break;
	    case 4:
		sprintf(Buf, "%s+%08x", s->sm_Name, offset);
		break;
	    default:
		sprintf(Buf, "%s+%04x", s->sm_Name, offset);
		break;
	    }
	} else {
	    sprintf(Buf, "%s", s->sm_Name);
	}
    } else {
	switch(size) {
	case 1:
	    sprintf(Buf, "%02x.%02x", r->ri_DstHunk, offset & 0xFF);
	    break;
	case 2:
	    sprintf(Buf, "%02x.%04x", r->ri_DstHunk, offset & 0xFFFF);
	    break;
	case 4:
	    sprintf(Buf, "%02x.%08x", r->ri_DstHunk, offset);
	    break;
	default:
	    sprintf(Buf, "%02x.%04x", r->ri_DstHunk, offset);
	    break;
	}
    }
    if (ext) {
	if (r->ri_RelocFlags & RF_PCREL)
	    strcat(Buf, "(pc)");
	if (r->ri_RelocFlags & RF_A4REL)
	    strcat(Buf, "(A4)");
    }
    return(Buf);
}

int32_t
FixRelocOffset(RelocInfo *r, int32_t offset)
{
    if (r == NULL)
	return(offset);
    if (r->ri_Sym)          /*  symbol relative */
	return(0);
    return(offset);
}

void
cerror(int code, char *ctl, ...)
{
    va_list va;

    switch(code) {
    case EWARN:
	printf("Warning ");
	break;
    case EERROR:
	printf("Error   ");
	break;
    case EFATAL:
	printf("Fatal   ");
	break;
    }

    va_start(va, ctl);
    vprintf(ctl, va);
    va_end(va);
    puts("");

    if (code == EFATAL)
	exit(20);
}

int 
freadl(void *buf, int elsize, int nel, FILE *fi)
{
    int n = fread(buf, elsize, nel, fi);

#ifdef INTELBYTEORDER
    int i;

    if (elsize == 2) {
	uword *bptr;

	for (bptr = buf, i = n; i > 0; --i, ++bptr) {
	    *bptr = ntohs(*bptr);
	}
    } else if (elsize == 4) {
	uint32_t *bptr;

	for (bptr = buf, i = n; i > 0; --i, ++bptr) {
	    *bptr = ntohl(*bptr);
	}
    }
#endif
    return(n);
}


