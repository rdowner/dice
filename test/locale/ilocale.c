
/*
 * ILOCALE.C
 *
 * Internationalization auto-load, brought in by DCC for commercial DICE
 */

#include <exec/types.h>
#include <clib/locale_protos.h>
#include <stdio.h>

extern long _AutoLocale[];
extern struct Library *LocaleBase;

static APTR Catalog;

__autoexit static void
ILocaleExit(void)
{
    if (Catalog) {
        CloseCatalog(Catalog);
        Catalog = NULL;
    }
}

int
SetCatalog(char *localeName, char *catName)
{
    int r = -1;

    struct TagItem tags[3] = {
        OC_Language, localeName,
        OC_BuiltInLanguage, "none",
        TAG_DONE, NULL
    };
    if (localeName == NULL)
        tags[0].ti_Tag = TAG_IGNORE;

    ILocaleExit();
    if (LocaleBase) {
        if (Catalog = OpenCatalogA(NULL, catName, tags)) {
            long *al;

            r = 0;

            for (al = _AutoLocale + 1; *al; al += 2) {
                /* printf("%s ->", (char *)*al); */
                printf("GetCat: %d %08lx\n", al[1], GetCatalogStr(Catalog, al[1], NULL));
                *al = GetCatalogStr(Catalog, al[1], (char *)al[0]);
                /* printf("%s\n", (char *)*al); */
            }
        }
    }
    return(r);
}

