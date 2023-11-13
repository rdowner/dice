
/*
 *  SYSTEM.C        Dynamically check whether we are running under 1.3 or 1.4
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#include <exec/types.h>
#include <exec/libraries.h>
#include <stdlib.h>

#ifdef INCLUDE_VERSION      /*  doesn't exist in 1.3 */
#include <dos/dostags.h>
#include <clib/dos_protos.h>
#endif

extern struct DosLibrary *DOSBase;

system(buf)
const char *buf;
{
    int r;

    if (buf == NULL)
        return(1);

#ifdef INCLUDE_VERSION
    if (DOSBase->dl_lib.lib_Version >= 36) {
        struct TagItem TI[] = {
            SYS_Input , Input(),
            SYS_Output, Output(),
            _SystemBoolTag, _SystemBoolTagValue,
            TAG_DONE, 0
        };
        r = System(buf, TI);
    } else
#endif
    {
        r = Execute(buf, NULL, NULL);
        if (r == 0)
            r = -1;
        else if (r == -1)
            r = 0;
    }
    return(r);
}

