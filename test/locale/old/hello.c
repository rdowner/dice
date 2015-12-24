;/* Sample use of CatComp

CatComp helloworld.cd cfile helloworld_strings.h objfile helloworld_strings.o
Quit
*/

#include <exec/types.h>
#include <libraries/locale.h>
#include <stdio.h>

#include <clib/exec_protos.h>
#include <clib/locale_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/locale_pragmas.h>

#define CATCOMP_STRINGS		// for demonstrating the reverse index functions
				// in the real program, the strings would appear
				// in this program;  I'm using the defines
				// for convenience

#include "helloworld_strings.h"


extern struct Library *SysBase;
struct Library *LocaleBase;


#define Prototype extern

Prototype STRPTR GetString(LONG stringNum);
Prototype LONG GetIndex(STRPTR string);
Prototype void openCatalog(char *name);
Prototype void closeCatalog(void);

APTR Catalog = NULL;
char builtin[100][80]={NULL};

// fool locale.library into opening the english catalog
static struct TagItem tags[3] = {
    OC_Language,"english",
    OC_BuiltInLanguage,"none",
    TAG_DONE, NULL};

VOID main(VOID)
{
int i;
char *string;

    LocaleBase = OpenLibrary("locale.library",38);

    // built translation table
    if(LocaleBase) {
	APTR catalog;
        catalog = OpenCatalogA(NULL,"helloworld.catalog",tags);
        for(i=0; i<100; i++) {	// build string table
	    if( *(string = GetCatalogStr(catalog,i,"")) == NULL)break;
	    strcpy(builtin[i],string);
        }
	CloseCatalog(catalog);
    }
    openCatalog("helloworld.catalog");
    printf("Cat Index of [%s] is %d\n",MSG_BYE_STR,GetIndex(MSG_BYE_STR));
    printf("Cat Index of [%s] is %d\n",MSG_HELLO_STR,GetIndex(MSG_HELLO_STR));

    printf("%s\n",GetString(GetIndex(MSG_HELLO_STR)));

    closeCatalog();
    if (LocaleBase)CloseLibrary(LocaleBase);
}

STRPTR GetString(LONG stringNum)
{

    if (LocaleBase)
        return(GetCatalogStr(Catalog,stringNum,builtin[stringNum]));

    return(builtin[stringNum]);
}


/* Given a string, scan through the local strings and find the index number
 * which is used to access locale.library to get the string in the
 * current catalog
 */

LONG GetIndex(STRPTR string)
{
int i;

    for(i=0; i<100; i++) {
	if(*builtin[i] == NULL)break;	// end of table
	if(!Stricmp(builtin[i],string))return i;
    }
    return NULL;
}

void openCatalog(char *name)
{
   if(LocaleBase)Catalog  = OpenCatalogA(NULL,name,NULL);
}

void closeCatalog(void)
{
   if(LocaleBase && Catalog)CloseCatalog(Catalog);
}