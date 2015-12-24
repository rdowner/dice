
/*
 *
 */

#include <stdio.h>

main(ac, av)
char *av[];
{
    if (ac == 1) {
	puts("exit <code>");
	exit(1);
    }
    exit(atoi(av[1]));
}
