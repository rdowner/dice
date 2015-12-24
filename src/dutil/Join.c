/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  JOIN.C
 *
 *  JOIN files AS outfile
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#ifdef AMIGA
#include <libc.h>
#else
#include <unistd.h>
#endif

char Buf[65536];

int
main(int ac, char **av)
{
    int i;
    int fdo;
    int error = 0;
    char *outFile = NULL;
    char *ptr;

    for (i = 1; i < ac; ++i) {
	ptr = av[i];

	if (strcmp(ptr, "AS") == 0 || strcmp(ptr, "as") == 0) {
	    outFile = av[i + 1];
	    av[i] = NULL;
	    break;
	}
    }
    if (outFile == NULL) {
	fprintf(stderr, "No AS option specified\n");
	exit(20);
    }
    if ((fdo = open(outFile, O_WRONLY|O_CREAT|O_TRUNC, 0666)) < 0) {
	fprintf(stderr, "couldn't create %s\n", outFile);
	exit(20);
    }
    for (i = 1; error == 0 && (ptr = av[i]); ++i) {
	int fd;
	long n;

	if ((fd = open(ptr, O_RDONLY)) < 0) {
	    fprintf(stderr, "couldn't open %s\n", ptr);
	    error = 20;
	} else {
	    while ((n = read(fd, Buf, sizeof(Buf))) > 0) {
		if (write(fdo, Buf, n) != n) {
		    fprintf(stderr, "write error\n");
		    error = 20;
		    break;
		}
	    }
	}
    }
    close(fdo);
    if (error)
	unlink(outFile);
    exit(error);
}

