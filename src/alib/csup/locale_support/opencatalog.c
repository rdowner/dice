
/* Locale tag call */

#include <exec/types.h>
#include <clib/locale_protos.h>

#ifndef HYPER
#define HYPER
#endif

typedef struct TagItem	 TagItem;


struct Catalog *
HYPER ## OpenCatalog( struct Locale *locale, STRPTR name, Tag tag1, ... )
{
	return OpenCatalogA(locale,name,(TagItem *)&tag1);
}
