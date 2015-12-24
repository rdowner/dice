
/*
 *  EasyRequest(Window, easyStruct, IDCMP_ptr, ArgList)
 */

#include <exec/types.h>
#include <intuition/intuition.h>
//#include <clib/intuition_protos.h>

extern LONG EasyRequestArgs( struct Window *window, struct EasyStruct *easyStruct,
	ULONG *idcmpPtr, APTR args );


LONG
EasyRequest(win, es, idcmp_ptr, APTR arg1, ...)
struct Window *win;
struct EasyStruct *es;
ULONG *idcmp_ptr;
APTR arg1;
{
    return(EasyRequestArgs(win, es, idcmp_ptr, &arg1));
}

