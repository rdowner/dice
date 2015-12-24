
/*
 *  CAT.C
 */

#include <stdio.h>
#include <stdlib.h>

int
brk()
{
    puts("Well, if you insist...");
    return(1);
}

main(ac, av)
char *av[];
{
    short i;
    char buf[256];

    if (ac == 1) {
	puts("cat <file>");
	exit(1);
    }

    onbreak(brk);

    for (i = 1; i < ac; ++i) {
	char *fileName = av[i];
	FILE *fi;

	if (fi = fopen(fileName, "r")) {
	    while (fgets(buf, sizeof(buf), fi))
		fputs(buf, stdout);
	    fclose(fi);
	} else {
	    printf("Unable to open %s\n", fileName);
	}
    }
    return(0);
}

