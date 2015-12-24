/*
 *  CARYB.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 *
 */

#include <ctype.h>

#define PU  __CF_ISPUNCT
#define XD  __CF_ISXDIGIT
#define IP  __CF_ISPRINT

const unsigned char __CAryb[257] = {
    0,
    0		,0	    ,0		,0	    ,0		,0	    ,0		,0	    ,
    0		,0	    ,0		,0	    ,0		,0	    ,0		,0	    ,
    0		,0	    ,0		,0	    ,0		,0	    ,0		,0	    ,
    0		,0	    ,0		,0	    ,0		,0	    ,0		,0	    ,
    IP		,IP|PU	    ,IP|PU	,IP|PU	    ,IP|PU	,IP|PU	    ,IP|PU	,IP|PU	    ,
    IP|PU	,IP|PU	    ,IP|PU	,IP|PU	    ,IP|PU	,IP|PU	    ,IP|PU	,IP|PU	    ,
    IP|XD	,IP|XD	    ,IP|XD	,IP|XD	    ,IP|XD	,IP|XD	    ,IP|XD	,IP|XD	    ,
    IP|XD	,IP|XD	    ,IP|PU	,IP|PU	    ,IP|PU	,IP|PU	    ,IP|PU	,IP|PU	    ,
    IP|PU	,IP|XD	    ,IP|XD	,IP|XD	    ,IP|XD	,IP|XD	    ,IP|XD	,IP	    ,
    IP		,IP	    ,IP 	,IP	    ,IP 	,IP	    ,IP 	,IP	    ,
    IP		,IP	    ,IP 	,IP	    ,IP 	,IP	    ,IP 	,IP	    ,
    IP		,IP	    ,IP 	,IP|PU	    ,IP|PU	,IP|PU	    ,IP|PU	,IP|PU	    ,
    IP|PU	,IP|XD	    ,IP|XD	,IP|XD	    ,IP|XD	,IP|XD	    ,IP|XD	,IP	    ,
    IP		,IP	    ,IP 	,IP	    ,IP 	,IP	    ,IP 	,IP	    ,
    IP		,IP	    ,IP 	,IP	    ,IP 	,IP	    ,IP 	,IP	    ,
    IP		,IP	    ,IP 	,IP|PU	    ,IP|PU	,IP|PU	    ,IP|PU	,0	    ,
    0		,0	    ,0		,0	    ,0		,0	    ,0		,0	    ,
    0		,0	    ,0		,0	    ,0		,0	    ,0		,0	    ,
    0		,0	    ,0		,0	    ,0		,0	    ,0		,0	    ,
    0		,0	    ,0		,0	    ,0		,0	    ,0		,0	    ,
    IP		,IP|PU	    ,IP|PU	,IP|PU	    ,IP|PU	,IP|PU	    ,IP|PU	,IP|PU	    ,
    IP|PU	,IP|PU	    ,IP|PU	,IP|PU	    ,IP|PU	,IP|PU	    ,IP|PU	,IP|PU	    ,
    IP|PU	,IP|PU	    ,IP|PU	,IP|PU	    ,IP|PU	,IP|PU	    ,IP|PU	,IP|PU	    ,
    IP|PU	,IP|PU	    ,IP|PU	,IP|PU	    ,IP|PU	,IP|PU	    ,IP|PU	,IP|PU	    ,
    IP		,IP	    ,IP 	,IP	    ,IP 	,IP	    ,IP 	,IP	    ,
    IP		,IP	    ,IP 	,IP	    ,IP 	,IP	    ,IP 	,IP	    ,
    IP		,IP	    ,IP 	,IP	    ,IP 	,IP	    ,IP 	,IP	    ,
    IP		,IP	    ,IP 	,IP	    ,IP 	,IP	    ,IP   	,IP	    ,
    IP		,IP	    ,IP 	,IP	    ,IP 	,IP	    ,IP 	,IP	    ,
    IP		,IP	    ,IP 	,IP	    ,IP 	,IP	    ,IP 	,IP	    ,
    IP		,IP	    ,IP 	,IP	    ,IP 	,IP	    ,IP 	,IP|PU	    ,
    IP		,IP	    ,IP 	,IP	    ,IP 	,IP	    ,IP 	,IP	    ,
};

