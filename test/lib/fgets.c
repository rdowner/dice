
/*
 *  test/FGETS.C
 *
 */

#include <stdio.h>

main(ac, av)
char *av[];
{
    FILE *fi;
    char buf[256];

    if (ac == 1) {
	puts("fgets file");
	exit(1);
    }
    if (fi = fopen(av[1], "r")) {
	while (fgets(buf, 256, fi))
	    fputs(buf, stdout);
	fclose(fi);
    }
}

