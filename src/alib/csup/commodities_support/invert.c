

#include <exec/memory.h>
#include <devices/keymap.h>
#include <libraries/commodities.h>

#include <clib/commodities_protos.h>
#include <clib/exec_protos.h>
#include <clib/alib_protos.h>

#include <pragmas/commodities_pragmas.h>
#include <pragmas/exec_pragmas.h>


extern struct Library * __far CxBase;
extern struct Library * __far SysBase;


int doangle(STRPTR *,struct InputEvent *);
BYTE doesc(BYTE);


struct InputEvent *InvertString(STRPTR str, struct Keymap *km)
{
   /* bugs:
      can't escape '>'
      puts '\0' on closing angles
    */
   /* allocate input event for each character in string   */

   struct InputEvent   *chain = NULL;
   /* struct InputEvent   *head; */
   struct InputEvent   *ie;
   int               abort = 0;
   UBYTE            cc;

   if (!str || !*str) return (NULL);


   /*head = (struct InputEvent *) &chain;*/   /* jvudu   */

   do {   /* have checked that str is not null    */
      /* allocate the next ie and link it in   */
      if (!(ie =
         AllocMem((LONG) sizeof (struct InputEvent), (LONG) MEMF_CLEAR)))
      {
         abort = 1;
         goto DONE;
      }
      ie->ie_NextEvent = chain;
      chain = ie;
      /*
      tail->ie_NextEvent = ie;
      tail = ie;
      */

      /* now fill in the input event   */
      switch (cc = *str)
      {
      case   '<':
         ++str;
         if (!doangle(&str, ie))
         {
            abort = 1;
            goto DONE;
         }
         break;

      case   '\\':
         if ((cc = doesc( *(++str) )) == -1)   /* get escaped character   */
         {
            abort = 1;
            goto DONE;
         }
         /* fall through   */
      default:
         InvertKeyMap((ULONG) cc, ie,(struct KeyMap *)km);
      }


   } while (*(++str));

DONE:
   if (abort)
   {
      FreeIEvents(ie);
      return (NULL);
   }
   return (chain);
}

BYTE doesc(BYTE cc)
{
    switch(cc)
    {
        case   'n' :
        case   'r' : return ('\r');
                     break;

        case   't' : return ('\t');
                     break;

        case   '0' : return ('\0');
                     break;

        case   '\\':
        case   '\"':
        case   '\'':
        case   '<' : break;

        default    : cc = -1;
    }

    return (cc);
}

/* return 0 if problem   */
int doangle(STRPTR *strp, struct InputEvent *ie)
{
   IX   ix;
   char   *cp;
   ULONG   retval;

   /* find closing angle '>', put a null there   */
   for (cp = *strp; *cp; ++cp)
   {
      if (*cp == '>')
      {
         *cp = '\0';
         break;
      }
   }

   retval = ParseIX(*strp, &ix);

   if (cp)
   {
      *cp = '>';   /* fix it   */
      *strp = cp;   /* point to char following '>'   */
   }
   else
   {
      *strp = cp - 1;   /* ++will point to terminating null   */
   }

   if (retval)    /* error */
   {
      return (0);
   }

   /* use IX to describe a suitable InputEvent */
   ie->ie_Class     = ix.ix_Class;
   ie->ie_Code      = ix.ix_Code;
   ie->ie_Qualifier = ix.ix_Qualifier;

   return (1);
}
