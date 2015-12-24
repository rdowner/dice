/* argarray.c -- tool types and startup parameters.   */

#include <libraries/commodities.h>
#include <workbench/workbench.h>
#include <workbench/startup.h>
#include <exec/memory.h>

#include <clib/commodities_protos.h>
#include <clib/exec_protos.h>
#include <clib/icon_protos.h>
#include <clib/dos_protos.h>

#include <pragmas/commodities_pragmas.h>

#include <stdlib.h>


extern struct Library * __far CxBase;


ULONG CXLIB_argarray_size;
char  **CXLIB_argarray = 0;
static   struct   DiskObject   *disko = NULL;

CxObj *UserFilter(char **tt, STRPTR action_name, STRPTR default_descr);


char **ArgArrayInit(int argc, char **argv)
{
ULONG             x;
struct WBArg     *arg;
char            **tt = NULL;
struct WBStartup *msg;

   if (argc)      /* run from CLI */
   {
      if (argc==1) return(NULL);  /* skip command name */

      CXLIB_argarray_size = sizeof(char *)*argc;
      CXLIB_argarray = (char **)AllocVec(CXLIB_argarray_size,MEMF_CLEAR);
      if(! CXLIB_argarray) return(NULL);

      for(x=0;x<argc-1;x++)
      {
         CXLIB_argarray[x]=argv[x+1];
      }
      return (CXLIB_argarray);
   }

   /* run from WB */
   msg = (struct WBStartup *)argv;
   arg = msg->sm_ArgList;

   if (disko = GetDiskObject(arg->wa_Name))
   {
      tt = disko->do_ToolTypes;
   }

   return (tt);
}


VOID ArgArrayDone(VOID)
{
   if(CXLIB_argarray) FreeVec(CXLIB_argarray);
   if (disko) FreeDiskObject(disko);
}


STRPTR ArgString(char **tt, STRPTR entry,STRPTR defaultstring)
{
STRPTR result;

   if (tt && (result = FindToolType(tt,entry)))
      return (result);

   return (defaultstring);
}


LONG ArgInt(char **tt, STRPTR entry, LONG defaultval)
{
STRPTR result;

   if (tt && (result = FindToolType(tt,entry)))
   {
       StrToLong(result,&defaultval);
   }

   return (defaultval);
}


CxObj *UserFilter(char **tt, STRPTR action_name, STRPTR default_descr)
 /* tt            - null-terminated (char *(*)[])   */
 /* action_name   - name of your semantic action    */
 /* default_descr - used if user doesn't provide    */
{
STRPTR desc = NULL;

   if (tt)
       desc = FindToolType(tt,action_name);

   return ( CxFilter((ULONG)(desc? desc: default_descr)));
}
