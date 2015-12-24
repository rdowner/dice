
/*
 *  COUNT.C
 */

#include <stdio.h>

main(ac, av)
char *av[];
{
    long i;

    if (ac == 1) {
	puts("count <startvalue>");
	exit(1);
    }
    for (i = atoi(av[1]); i; --i)
	printf("%d\n", i);
    return(0);
}

