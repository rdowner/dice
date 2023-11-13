
/*
 *  CARYA.C
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 *
 */

#include <ctype.h>

#define AN  __CF_ISALNUM
#define AL  __CF_ISALPHA
#define CT  __CF_ISCNTRL
#define NU  __CF_ISDIGIT
#define GR  __CF_ISGRAPH
#define LO  __CF_ISLOWER
#define SP  __CF_ISSPACE
#define UP  __CF_ISUPPER

const unsigned char __CArya[257] = {
    0,
    CT          ,CT         ,CT         ,CT         ,CT         ,CT         ,CT         ,CT         ,
    CT          ,CT|SP      ,CT|SP      ,CT|SP      ,CT|SP      ,CT|SP      ,CT         ,CT         ,
    CT          ,CT         ,CT         ,CT         ,CT         ,CT         ,CT         ,CT         ,
    CT          ,CT         ,CT         ,CT         ,CT         ,CT         ,CT         ,CT         ,
    SP          ,GR         ,GR         ,GR         ,GR         ,GR         ,GR         ,GR         ,
    GR          ,GR         ,GR         ,GR         ,GR         ,GR         ,GR         ,GR         ,
    GR|NU|AN    ,GR|NU|AN   ,GR|NU|AN   ,GR|NU|AN   ,GR|NU|AN   ,GR|NU|AN   ,GR|NU|AN   ,GR|NU|AN   ,
    GR|NU|AN    ,GR|NU|AN   ,GR         ,GR         ,GR         ,GR         ,GR         ,GR         ,
    GR          ,GR|AN|AL|UP,GR|AN|AL|UP,GR|AN|AL|UP,GR|AN|AL|UP,GR|AN|AL|UP,GR|AN|AL|UP,GR|AN|AL|UP,
    GR|AN|AL|UP ,GR|AN|AL|UP,GR|AN|AL|UP,GR|AN|AL|UP,GR|AN|AL|UP,GR|AN|AL|UP,GR|AN|AL|UP,GR|AN|AL|UP,
    GR|AN|AL|UP ,GR|AN|AL|UP,GR|AN|AL|UP,GR|AN|AL|UP,GR|AN|AL|UP,GR|AN|AL|UP,GR|AN|AL|UP,GR|AN|AL|UP,
    GR|AN|AL|UP ,GR|AN|AL|UP,GR|AN|AL|UP,GR         ,GR         ,GR         ,GR         ,GR         ,
    GR          ,GR|AN|AL|LO,GR|AN|AL|LO,GR|AN|AL|LO,GR|AN|AL|LO,GR|AN|AL|LO,GR|AN|AL|LO,GR|AN|AL|LO,
    GR|AN|AL|LO ,GR|AN|AL|LO,GR|AN|AL|LO,GR|AN|AL|LO,GR|AN|AL|LO,GR|AN|AL|LO,GR|AN|AL|LO,GR|AN|AL|LO,
    GR|AN|AL|LO ,GR|AN|AL|LO,GR|AN|AL|LO,GR|AN|AL|LO,GR|AN|AL|LO,GR|AN|AL|LO,GR|AN|AL|LO,GR|AN|AL|LO,
    GR|AN|AL|LO ,GR|AN|AL|LO,GR|AN|AL|LO,GR         ,GR         ,GR         ,GR         ,CT         ,
    CT          ,CT         ,CT         ,CT         ,CT         ,CT         ,CT         ,CT         ,
    CT          ,CT         ,CT         ,CT         ,CT         ,CT         ,CT         ,CT         ,
    CT          ,CT         ,CT         ,CT         ,CT         ,CT         ,CT         ,CT         ,
    CT          ,CT         ,CT         ,CT         ,CT         ,CT         ,CT         ,CT         ,
    SP          ,GR         ,GR         ,GR         ,GR         ,GR         ,GR         ,GR         ,
    GR          ,GR         ,GR         ,GR         ,GR         ,GR         ,GR         ,GR         ,
    GR          ,GR         ,GR         ,GR         ,GR         ,GR         ,GR         ,GR         ,
    GR          ,GR         ,GR         ,GR         ,GR         ,GR         ,GR         ,GR         ,
    GR|AN|AL|UP ,GR|AN|AL|UP,GR|AN|AL|UP,GR|AN|AL|UP,GR|AN|AL|UP,GR|AN|AL|UP,GR|AN|AL|UP,GR|AN|AL|UP,
    GR|AN|AL|UP ,GR|AN|AL|UP,GR|AN|AL|UP,GR|AN|AL|UP,GR|AN|AL|UP,GR|AN|AL|UP,GR|AN|AL|UP,GR|AN|AL|UP,
    GR|AN|AL|UP ,GR|AN|AL|UP,GR|AN|AL|UP,GR|AN|AL|UP,GR|AN|AL|UP,GR|AN|AL|UP,GR|AN|AL|UP,GR|AN|AL|UP,
    GR|AN|AL|UP ,GR|AN|AL|UP,GR|AN|AL|UP,GR|AN|AL|UP,GR|AN|AL|UP,GR|AN|AL|UP,GR         ,GR|AN|AL|LO|UP,
    GR|AN|AL|LO ,GR|AN|AL|LO,GR|AN|AL|LO,GR|AN|AL|LO,GR|AN|AL|LO,GR|AN|AL|LO,GR|AN|AL|LO,GR|AN|AL|LO,
    GR|AN|AL|LO ,GR|AN|AL|LO,GR|AN|AL|LO,GR|AN|AL|LO,GR|AN|AL|LO,GR|AN|AL|LO,GR|AN|AL|LO,GR|AN|AL|LO,
    GR|AN|AL|LO ,GR|AN|AL|LO,GR|AN|AL|LO,GR|AN|AL|LO,GR|AN|AL|LO,GR|AN|AL|LO,GR|AN|AL|LO,GR         ,
    GR|AN|AL|LO ,GR|AN|AL|LO,GR|AN|AL|LO,GR|AN|AL|LO,GR|AN|AL|LO,GR|AN|AL|LO,GR         ,GR|AN|AL|LO
};


