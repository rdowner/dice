
/*
 *  EasyRequest(Window, easyStruct, IDCMP_ptr, ArgList)
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 *
 */

#include <exec/types.h>
#include <intuition/intuition.h>
#include <clib/intuition_protos.h>
#include <stdarg.h>

LONG
EasyRequest(win, es, idcmp_ptr, ...)
struct Window *win;
struct EasyStruct *es;
ULONG *idcmp_ptr;
{
   long  tmp;
   va_list va;
   va_start(va,idcmp_ptr);
   tmp = EasyRequestArgs(win, es, idcmp_ptr, va);
   va_end(va);
   return(tmp);
}

