
#include <stdio.h>

char buf[256];

main(ac, av)
char *av[];
{
    short i;

    for (i = 1; i < ac; ++i) {
	puts(av[i]);
    }
    puts("input:");
    gets(buf);
    puts(buf);
}

