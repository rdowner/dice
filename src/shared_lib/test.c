/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  TEST.C
 */

#include "test-protos.h"
#include <stdio.h>

main(ac, av)
short ac;
char *av[];
{
    if (ac == 1) {
        char buf[256];
        while (GetString(buf, sizeof(buf)) >= 0)
            printf("GET: %s\n", buf);
    } else {
        short i;

        for (i = 1; i < ac; ++i) {
            PostString(av[i]);
            printf("POSTED: %s\n", av[i]);
        }
    }
    return(0);
}

