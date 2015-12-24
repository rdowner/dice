/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  SYM.C
 *
 */

#include "defs.h"

Prototype int hash(char *, short);
Prototype Sym *FindSymbol(char *, short);
Prototype int UndefSymbol(char *, short);
Prototype void DefineOptSymbol(char *);
Prototype Sym *DefineSimpleSymbol(char *, char *, short);
Prototype Sym *DefineSymbol(char *, short, short, short, char **, short *, char *, short, short, int32_t);

Prototype void DumpPrecompSymbols(FILE *);
Prototype void DefinePrecompSymbol(Sym *);

Prototype int32_t SymGroup;

static Sym *SymHash[HSIZE];
static Sym *SymCache;

int32_t	SymGroup;

#ifdef NO_ASM

int
hash(char *ptr, short len)
{
    int32_t hv = 0x1234FCD1;

    while (len) {
	hv = (hv >> 23) ^ (hv << 5) ^ (ubyte)*ptr;
	++ptr;
	--len;
    }
    return(hv & HMASK);
}

#endif

Sym *
FindSymbol(char *name, short len)
{
    Sym *sym;
    short hv = hash(name, len);

    for (sym = SymHash[hv & HMASK]; sym; sym = sym->Next) {
	if (sym->Hv == hv && sym->SymLen == len && cmpmem(name, sym->SymName, len) == 0) {
	    if ((sym->Type & SF_SPECIAL) || (sym->Type & SF_RECURSE) == 0) {
		if (sym->Type & SF_SPECIAL)
		    ModifySymbolText(sym, sym->Type);
		break;
	    }
	}
    }
    return(sym);
}

int
UndefSymbol(char *name, short len)
{
    Sym **psym;
    Sym *sym;
    short hv = hash(name, len);

    for (psym = &SymHash[hv & HMASK]; (sym = *psym) != NULL; psym = &sym->Next) {
	if (sym->Hv == hv && sym->SymLen == len && cmpmem(name, sym->SymName, len) == 0)
	    break;
    }
    if (sym == NULL)
	return(0);
    *psym = sym->Next;
    sym->Next = SymCache;
    SymCache = sym;
    return(1);
}

/*
 *  handle single level define
 */

void
DefineOptSymbol(str)
char *str;
{
    char *ptr;

    for (ptr = str; *ptr && *ptr != '='; ++ptr);
    if (*ptr == '=') {
	*ptr = ' ';
	++ptr;
	ptr += strlen(ptr);
    }
    do_define(str, ptr - str, NULL);
#ifdef NOTDEF
    DefineSimpleSymbol(str, ptr, 0);
#endif
}

Sym *
DefineSimpleSymbol(char *symName, char *symText, short symType)
{
    return(DefineSymbol(symName, strlen(symName), symType, -1, NULL, NULL, symText, 0, 0, strlen(symText)));
}

Sym *
DefineSymbol(
    char *name,
    short len,
    short type,
    short numArgs,
    char **args,
    short *lens,
    char *text,
    short allocName,
    short allocText,
    int32_t textSize
) {
    short hv = hash(name, len);
    Sym **psym = &SymHash[hv & HMASK];
    Sym *sym;

    if ((sym = SymCache) != NULL) {
	SymCache = sym->Next;	    /*	note, fields not zerod	*/
    } else {
	sym = zalloc(sizeof(Sym));
    }
    sym->Next = *psym;
    *psym = sym;

    if (allocName)
	name = AllocCopy(name, len);
    sym->SymName = name;
    sym->SymLen  = len;
    sym->Type	 = type;
    sym->NumArgs = numArgs;
    sym->Hv	 = hv;

    if (numArgs > 0) {
	short i;

	sym->Args    = AllocCopy(args, numArgs * sizeof(char *));
	sym->ArgsLen = AllocCopy(lens, numArgs * sizeof(short));
	for (i = 0; i < numArgs; ++i)
	    sym->Args[i] = AllocCopy(args[i], lens[i]);
    } else {
	sym->Args      = NULL;
	sym->ArgsLen   = NULL;
    }
    if (allocText)
	text = AllocCopy(text, textSize);
    sym->Text	 = text;
    sym->TextLen = textSize;
    sym->SymGroup = SymGroup;

    return(sym);
}

/*
 *  Precompiled header file routines
 */

void
DefinePrecompSymbol(sym)
Sym *sym;
{
    Sym **psym;

    /*
     *	adjust pointers
     */

    sym->SymName = (int32_t)(intptr_t)sym->SymName + (char *)sym;
    if (sym->Args) {
	int i;

	sym->Args = (char **)((int32_t)(intptr_t)sym->Args + (char *)sym);
	sym->ArgsLen = (short *)((int32_t)(intptr_t)sym->ArgsLen + (char *)sym);
	for (i = 0; i < sym->NumArgs; ++i) {
	    sym->Args[i] = (int32_t)(intptr_t)sym->Args[i] + (char *)sym;
	}
    }
    if (sym->Text)
	sym->Text = (int32_t)(intptr_t)sym->Text + (char *)sym;

    /*
     *	enter into hash table
     */

    psym = &SymHash[sym->Hv & HMASK];
    sym->Next = *psym;
    *psym = sym;
}

/*
 *  dump symbols in current SymGroup
 */

void
DumpPrecompSymbols(fo)
FILE *fo;
{
    int32_t i;
    Sym **psym;

    for (i = 0, psym = SymHash; i < HSIZE; ++i, ++psym) {
	Sym *sym;

	for (sym = *psym; sym; sym = sym->Next) {
	    if (sym->SymGroup == SymGroup) {
		int32_t bytes = sizeof(Sym);
		Sym xsym = *sym;

		/*
		 *  dump symbol
		 */

		xsym.SymName = (char *)(intptr_t)bytes;
		bytes += sym->SymLen;

		xsym.Text = (char *)(intptr_t)bytes;
		bytes += sym->TextLen;

		bytes = (bytes + 3) & ~3;   /*	LW-ALIGN    */

		if (sym->Args) {
		    int i;

		    xsym.Args = (char **)(intptr_t)bytes;
		    bytes += sym->NumArgs * sizeof(char *);

		    xsym.ArgsLen = (short *)(intptr_t)bytes;
		    bytes += sym->NumArgs * sizeof(sym->ArgsLen[0]);

		    for (i = 0; i < sym->NumArgs; ++i)
			bytes += sym->ArgsLen[i];
		}

		bytes = (bytes + 3) & ~3;   /*	LW-ALIGN    */

		fwrite(&bytes, sizeof(int32_t), 1, fo);

		bytes = sizeof(Sym) + sym->SymLen + sym->TextLen;
		fwrite(&xsym, sizeof(Sym), 1, fo);
		fwrite(sym->SymName, sym->SymLen, 1, fo);
		fwrite(sym->Text, sym->TextLen, 1, fo);

		while (bytes & 3) {
		    putc(0, fo);
		    ++bytes;
		}

		if (sym->Args) {
		    int i;

		    bytes += sym->NumArgs * sizeof(char *);
		    bytes += sym->NumArgs * sizeof(sym->ArgsLen[0]);

		    for (i = 0; i < sym->NumArgs; ++i) {
			fwrite(&bytes, sizeof(int32_t), 1, fo);
			bytes += sym->ArgsLen[i];
		    }
		    fwrite(sym->ArgsLen, sizeof(*sym->ArgsLen), sym->NumArgs, fo);

		    /*
		     *	write argument text
		     */

		    for (i = 0; i < sym->NumArgs; ++i)
			fwrite(sym->Args[i], 1, sym->ArgsLen[i], fo);
		}

		while (bytes & 3) {
		    putc(0, fo);
		    ++bytes;
		}

		fwrite(&bytes, sizeof(int32_t), 1, fo);
	    }
	}
    }
}

