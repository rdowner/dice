#ifndef CLIB_MACROS_H
#define CLIB_MACROS_H
/*
**      $Filename: clib/macros.h $
**      $Release: 2.04 $
**      $Revision: 30.156 $
**      $Date: 1995/01/11 05:18:14 $
**
**      C prototypes
**
**      (C) Copyright 1990 Commodore-Amiga, Inc.
**          All Rights Reserved
*/

#define MAX(a,b)    ((a)>(b)?(a):(b))
#define MIN(a,b)    ((a)<(b)?(a):(b))
#define ABS(x)      ((x<0)?(-(x)):(x))

#endif  /* CLIB_MACROS_H */
