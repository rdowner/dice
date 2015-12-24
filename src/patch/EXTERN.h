/* $Header: /home/dice/com/src/patch/RCS/EXTERN.h,v 30.8 1994/08/18 05:51:52 dice Exp dice $
 *
 * $Log: EXTERN.h,v $
 * Revision 30.8  1994/08/18  05:51:52  dice
 * .
 *
 * Revision 30.0  1994/06/10  18:08:49  dice
 * .
 *
 * Revision 30.0  1994/06/10  18:08:49  dice
 * .
 *
 * Revision 2.0  86/09/17  15:35:37  lwall
 * Baseline for netwide release.
 * 
 */

#ifdef EXT
#undef EXT
#endif
#define EXT extern

#ifdef INIT
#undef INIT
#endif
#define INIT(x)

#ifdef DOINIT
#undef DOINIT
#endif
