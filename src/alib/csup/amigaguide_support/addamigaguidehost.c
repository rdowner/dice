
/* AmigaGuide Tag Calls */

#include <exec/types.h>
#include <utility/tagitem.h>
#include <clib/amigaguide_protos.h>

#ifndef HYPER
#define HYPER
#endif

typedef struct TagItem TagItem;

APTR 
HYPER ## AddAmigaGuideHost( struct Hook *h, STRPTR name, Tag tag1, ... )
{
	return AddAmigaGuideHostA(h,name,(TagItem *)&tag1);
}
