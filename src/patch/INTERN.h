/* $Header: /home/dice/com/src/patch/RCS/INTERN.h,v 30.8 1994/08/18 05:51:54 dice Exp dice $
 *
 * $Log: INTERN.h,v $
 * Revision 30.8  1994/08/18  05:51:54  dice
 * .
 *
 * Revision 30.0  1994/06/10  18:08:50  dice
 * .
 *
 * Revision 30.0  1994/06/10  18:08:50  dice
 * .
 *
 * Revision 2.0  86/09/17  15:35:58  lwall
 * Baseline for netwide release.
 * 
 */

#ifdef EXT
#undef EXT
#endif
#define EXT

#ifdef INIT
#undef INIT
#endif
#define INIT(x) = x

#define DOINIT
