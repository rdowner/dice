
/*
 *  HELLO.C
 */

#include <lib/ilocale.h>

main(ac, av)
char *av[];
{
    int r = SetCatalog(av[1], "hello.catalog");

    if (r < 0)
	puts("couldn't open locale");
    else if (r > 0)
	puts("couldn't open catalog in locale");

    puts("Hello world");
    puts("This is a test");
    puts("This is not in the catalog");
    return(0);
}

