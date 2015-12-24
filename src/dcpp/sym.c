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
Prototype Sym *DefineSymbol(char *, short, short, short, char **, short *, char *, short, short, long);

Prototype void DumpPrecompSymbols(FILE *);
Prototype void DefinePrecompSymbol(Sym *);

Prototype long SymGroup;

static Sym *SymHash[HSIZE];
static Sym *SymCache;

long	SymGroup;

#ifdef NO_ASM

int
hash(char *ptr, short len)
{
    long hv = 0x1234FCD1;

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
    long textSize
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

    sym->SymName = (long)sym->SymName + (char *)sym;
    if (sym->Args) {
	int i;

	sym->Args = (char **)((long)sym->Args + (char *)sym);
	sym->ArgsLen = (short *)((long)sym->ArgsLen + (char *)sym);
	for (i = 0; i < sym->NumArgs; ++i) {
	    sym->Args[i] = (long)sym->Args[i] + (char *)sym;
	}
    }
    if (sym->Text)
	sym->Text = (long)sym->Text + (char *)sym;

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
    long i;
    Sym **psym;

    for (i = 0, psym = SymHash; i < HSIZE; ++i, ++psym) {
	Sym *sym;

	for (sym = *psym; sym; sym = sym->Next) {
	    if (sym->SymGroup == SymGroup) {
		long bytes = sizeof(Sym);
		Sym xsym = *sym;

		/*
		 *  dump symbol
		 */

		xsym.SymName = (char *)bytes;
		bytes += sym->SymLen;

		xsym.Text = (char *)bytes;
		bytes += sym->TextLen;

		bytes = (bytes + 3) & ~3;   /*	LW-ALIGN    */

		if (sym->Args) {
		    int i;

		    xsym.Args = (char **)bytes;
		    bytes += sym->NumArgs * sizeof(char *);

		    xsym.ArgsLen = (short *)bytes;
		    bytes += sym->NumArgs * sizeof(sym->ArgsLen[0]);

		    for (i = 0; i < sym->NumArgs; ++i)
			bytes += sym->ArgsLen[i];
		}

		bytes = (bytes + 3) & ~3;   /*	LW-ALIGN    */

		fwrite(&bytes, sizeof(long), 1, fo);

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
			fwrite(&bytes, sizeof(long), 1, fo);
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

		fwrite(&bytes, sizeof(long), 1, fo);
	    }
	}
    }
}

