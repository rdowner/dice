
/* Intuition tag call */

#include <exec/types.h>
//#include <clib/intuition_protos.h>

#ifndef HYPER
#define HYPER
#endif

typedef struct TagItem   TagItem;
struct Window *BuildEasyRequestArgs( struct Window *window,
        struct EasyStruct *easyStruct, unsigned long idcmp, APTR args );

struct Window *
HYPER ## BuildEasyRequest( struct Window *window,struct EasyStruct *easyStruct, unsigned long idcmp, APTR args ) 
{
        return BuildEasyRequestArgs(window,easyStruct,idcmp,(TagItem *)&args);
}
