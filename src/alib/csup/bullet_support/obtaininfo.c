
/* Bullet tag calls */

#include <exec/types.h>
#include <clib/bullet_protos.h>


#ifndef HYPER
#define HYPER
#endif

typedef struct TagItem   TagItem;

ULONG ObtainInfo( struct GlyphEngine *glyphEngine, Tag tag1, ... )
{       
        return ObtainInfoA(glyphEngine, (TagItem *)&tag1);
}
