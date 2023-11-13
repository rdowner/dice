
/* Graphics tag call */

#include <exec/types.h>
#include <clib/graphics_protos.h>

#ifndef HYPER
#define HYPER
#endif

typedef struct TagItem   TagItem;

LONG 
HYPER ## ChangeExtSprite( struct ViewPort *vp, struct ExtSprite *oldsprite, 
        struct ExtSprite *newsprite, unsigned long tag1Type, ... )
{
        return ChangeExtSpriteA(vp, oldsprite, newsprite, (TagItem *)&tag1Type);
}
