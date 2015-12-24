;/* Sample use of CatComp

CatComp helloworld.cd cfile helloworld_strings.h objfile helloworld_strings.o
Quit
*/

#include <exec/types.h>
#include <libraries/locale.h>
#include <stdio.h>

#include <clib/exec_protos.h>
#include <clib/locale_protos.h>

/*
#include <pragmas/exec_pragmas.h>
#include <pragmas/locale_pragmas.h>
*/

#define CATCOMP_NUMBERS
#define CATCOMP_BLOCK
#define CATCOMP_STRINGS 	// for demonstrating the reverse index functions
#include "helloworld_strings.h"


extern struct Library *SysBase;
struct Library *LocaleBase;


#define Prototype extern

Prototype STRPTR GetString(LONG stringNum);
Prototype LONG GetIndex(STRPTR string);
Prototype void openCatalog(char *name);
Prototype void closeCatalog(void);

APTR Catalog = NULL;

VOID main(VOID)
{
    LocaleBase = OpenLibrary("locale.library",38);
    openCatalog("helloworld.catalog");

    printf("%s\n",GetString(MSG_HELLO));
    printf("%s\n",GetString(MSG_BYE));

    printf("Cat Index of [%s] is %d\n",MSG_BYE_STR,GetIndex(MSG_BYE_STR));
    printf("Cat Index of [%s] is %d\n",MSG_HELLO_STR,GetIndex(MSG_HELLO_STR));

    printf("%s\n",GetString(GetIndex(MSG_HELLO_STR)));

    closeCatalog();
    if (LocaleBase)CloseLibrary(LocaleBase);
}


STRPTR GetString(LONG stringNum)
{
LONG   *l;
UWORD  *w;
STRPTR	builtIn;

    l = (LONG *)CatCompBlock;

    while (*l != stringNum)
    {
	w = (UWORD *)((ULONG)l + 4);
	l = (LONG *)((ULONG)l + (ULONG)*w + 6);
    }
    builtIn = (STRPTR)((ULONG)l + 6);

    printf("Catalog %08lx\n", Catalog);

    printf("GetString: %d %08lx\n", stringNum, GetCatalogStr(Catalog, stringNum, NULL));
    printf("GetString: %d %s\n", 1, GetCatalogStr(Catalog, 1, NULL));
    printf("GetString: %d %08lx\n", 0, GetCatalogStr(Catalog, 0, NULL));
    printf("GetString: %d %08lx\n", 2, GetCatalogStr(Catalog, 2, NULL));

    if (LocaleBase)
	return(GetCatalogStr(Catalog,stringNum,builtIn));

    return(builtIn);
}


/* Given a string, scan through the local strings and find the index number
 * which is used to access locale.library to get the string in the
 * current catalog
 */

LONG GetIndex(STRPTR string)
{
LONG   *l;
UWORD  *w;
STRPTR	builtIn;

    l = (LONG *)CatCompBlock;
    builtIn = (STRPTR)((ULONG)l + 6);

    while (Stricmp(builtIn,string)) {
	w = (UWORD *)((ULONG)l + 4);
	l = (LONG *)((ULONG)l + (ULONG)*w + 6);
	builtIn = (STRPTR)((ULONG)l + 6);
    }

    return *l;
}

void openCatalog(char *name)
{
static struct TagItem tags[3] = {
    OC_Language,"english",
    OC_BuiltInLanguage,"none",
    TAG_DONE, NULL};

   if(LocaleBase)Catalog  = OpenCatalogA(NULL,name,tags);
}

void closeCatalog(void)
{
   if(LocaleBase && Catalog)CloseCatalog(Catalog);
}
