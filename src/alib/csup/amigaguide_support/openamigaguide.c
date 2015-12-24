
/* AmigaGuide Tag Calls */

#include <exec/types.h>
#include <utility/tagitem.h>
#include <clib/amigaguide_protos.h>

#ifndef HYPER
#define HYPER
#endif

typedef struct TagItem TagItem;

APTR 
HYPER ## OpenAmigaGuide( struct NewAmigaGuide *nag, Tag tag1, ... )
{
	return OpenAmigaGuideA(nag,(TagItem *)&tag1);
}
