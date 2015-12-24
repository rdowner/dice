#ifndef  CLIB_MATHTRANS_PROTOS_H
#define  CLIB_MATHTRANS_PROTOS_H
/*
**	$VER: mathtrans_protos.h 1.2 (07.11.90)
**	Includes Release 39.108
**
**	C prototypes. For use with 32 bit integers only.
**
**	(C) Copyright 1990-1992 Commodore-Amiga, Inc.
**	    All Rights Reserved
*/
#ifndef  EXEC_TYPES_H
#include <exec/types.h>
#endif
FLOAT SPAtan( FLOAT parm );
FLOAT SPSin( FLOAT parm );
FLOAT SPCos( FLOAT parm );
FLOAT SPTan( FLOAT parm );
FLOAT SPSincos( FLOAT *cosResult, FLOAT parm );
FLOAT SPSinh( FLOAT parm );
FLOAT SPCosh( FLOAT parm );
FLOAT SPTanh( FLOAT parm );
FLOAT SPExp( FLOAT parm );
FLOAT SPLog( FLOAT parm );
FLOAT SPPow( FLOAT power, FLOAT arg );
FLOAT SPSqrt( FLOAT parm );
FLOAT SPTieee( FLOAT parm );
FLOAT SPFieee( FLOAT parm );
/*--- functions in V31 or higher (distributed as Release 1.1) ---*/
FLOAT SPAsin( FLOAT parm );
FLOAT SPAcos( FLOAT parm );
FLOAT SPLog10( FLOAT parm );
#endif	 /* CLIB_MATHTRANS_PROTOS_H */
