#ifndef  CLIB_COLORWHEEL_PROTOS_H
#define  CLIB_COLORWHEEL_PROTOS_H
/*
**      $VER: colorwheel_protos.h 39.1 (21.07.92)
**      Includes Release 39.108
**
**      C prototypes. For use with 32 bit integers only.
**
**      (C) Copyright 1990-1992 Commodore-Amiga, Inc.
**          All Rights Reserved
*/
#ifndef  EXEC_TYPES_H
#include <exec/types.h>
#endif
#ifndef  GADGETS_COLORWHEEL_H
#include <gadgets/colorwheel.h>
#endif
/*--- functions in V39 or higher (beta release for developers only) ---*/

/* Public entries */

void ConvertHSBToRGB( struct ColorWheelHSB *hsb, struct ColorWheelRGB *rgb );
void ConvertRGBToHSB( struct ColorWheelRGB *rgb, struct ColorWheelHSB *hsb );
#endif   /* CLIB_COLORWHEEL_PROTOS_H */
