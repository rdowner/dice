
/* AmigaGuide Tag Calls */

#include <exec/types.h>
#include <utility/tagitem.h>
#include <clib/amigaguide_protos.h>

#ifndef HYPER
#define HYPER
#endif

typedef struct TagItem TagItem;

APTR 
HYPER ## OpenAmigaGuideAsync( struct NewAmigaGuide *nag, Tag tag1, ...)
{

	return OpenAmigaGuideAsyncA(nag,(TagItem *)&tag1);
}
