/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */
/* $Header: /home/dice/com/src/patch/RCS/version.c,v 30.8 1994/08/18 05:51:59 dice Exp dice $
 *
 * $Log: version.c,v $
 * Revision 30.8  1994/08/18  05:51:59  dice
 * .
 *
 * Revision 30.0  1994/06/10  18:08:52  dice
 * .
 *
 * Revision 30.0  1994/06/10  18:08:52  dice
 * .
 *
 * Revision 2.0  86/09/17  15:40:11  lwall
 * Baseline for netwide release.
 * 
 */

#include "EXTERN.h"
#include "common.h"
#include "util.h"
#include "INTERN.h"
#include "patchlevel.h"
#include "version.h"

/* Print out the version number and die. */

void
version()
{
    extern char rcsid[];

#ifdef lint
    rcsid[0] = rcsid[0];
#else
    fatal3("%s\nPatch level: %s\n", rcsid, PATCHLEVEL);
#endif
}
