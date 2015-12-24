
/* Gadtools tag call */

#include <exec/types.h>
#include <clib/gadtools_protos.h>

#ifndef HYPER
#define HYPER
#endif

typedef struct TagItem	 TagItem;

struct Gadget *
HYPER ## CreateGadget( unsigned long kind, struct Gadget *gad, 
	struct NewGadget *ng, Tag tag1, ... )
{
	return CreateGadgetA(kind,gad,ng,(TagItem *)&tag1);
}
