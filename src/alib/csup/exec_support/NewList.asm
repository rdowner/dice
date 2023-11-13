
******* amiga.lib/NewList *****************************************************
*
*   NAME
*       NewList -- prepare a list structure for use
*
*   SYNOPSIS
*       NewList(list)
*
*       VOID NewList(struct List *);
*       VOID NewList(struct MinList *);
*
*   FUNCTION
*       Perform the magic needed to prepare a List header structure for
*       use; the list will be empty and ready to use.  (If the list is the
*       full featured type, you may need to initialize lh_Type afterwards)
*
*       Assembly programmers may want to use the NEWLIST macro instead.
*
*   INPUTS
*       list - pointer to a List or MinList.
*
*   SEE ALSO
*       <exec/lists.h>
*
*******************************************************************************

                INCLUDE "exec/types.i"
                INCLUDE "exec/lists.i"


                SECTION _NewList
                XDEF    _NewList
                XDEF    @NewList

_NewList:       move.l  4(sp),a0        ;Get pointer from C's stack

;This next code is equavalent to the NEWLIST macro (but faster)
@NewList:
                clr.l   LH_TAIL(a0)
                move.l  a0,LH_TAILPRED(a0)
                addq.l  #LH_TAIL,a0     ;pointer plus 4...
                move.l  a0,-(a0)        ;...back down to LH_HEAD

;;;;            move.l  a0,d0           :pass the list back in D0
                rts

                END
