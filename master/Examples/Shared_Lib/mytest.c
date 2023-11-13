
/*
 *  TEST.C
 */

/*
 *  The #ifdef is to demonstrate what is required to compile this
 *  test program with registered args.
 */

#include <stdio.h>

main(ac, av)
short ac;
char *av[];
{
    if (ac == 1) {
        char buf[256];
        int cnt = 0;

        while (GetString(buf, sizeof(buf)) >= 0) {
            printf("GET: %s\n", buf);
            ++cnt;
        }
        if (cnt == 0) {
            puts("no strings were entered, first do:");
            puts("    mytest <string>");
            puts("then do:");
            puts("    mytest");
        }
    } else {
        short i;

        for (i = 1; i < ac; ++i) {
            PostString(av[i]);
            printf("POSTED: %s\n", av[i]);
        }
    }
    return(0);
}

