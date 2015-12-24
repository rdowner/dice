

#include <exec/types.h>
#include <exec/libraries.h>
#include <libraries/commodities.h>
#include <clib/commodities_protos.h>
#include <pragmas/commodities_pragmas.h>


extern struct Library   * __far CxBase;


CxObj *HotKey(STRPTR descr, struct MsgPort *port, LONG ID)
{
CxObj *filter;

   if (filter = CxFilter(descr))
   {
       AttachCxObj(filter,CxSender(port,ID));
       AttachCxObj(filter,CxTranslate(NULL));

       if (CxObjError(filter))
       {
          DeleteCxObjAll(filter);
          filter = NULL;
       }
   }

   return (filter);
}
