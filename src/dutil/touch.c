/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
 *  TOUCH.C
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#ifdef AMIGA
#include <lib/version.h>
#else
#include <include/lib/version.h>
#endif

IDENT("touch",".2");
DCOPYRIGHT;

main(ac, av)
int ac;
char *av[];
{
    int i;

    if (ac == 1) {
    	puts(Ident);
    	puts(DCopyright);
	printf("touch files/dirs (wildcards ok)\n");
	return(0);
    }

#ifndef unix
    expand_args(ac, av, &ac, &av);
#endif
    for (i = 1; i < ac; ++i) {
	char *fn = av[i];
	int fd;

	fd = open(fn, O_RDWR);
	if (fd >= 0) {
	    char c;
	    if (read(fd, &c, 1) == 1) {
		if (lseek(fd, 0L, 0) == 0) {
		    write(fd, &c, 1);
		    printf("%s touched\n", fn);
		}
	    }
	    close(fd);
	} else if (errno = ENOFILE) {
	    fd = open(fn, O_WRONLY|O_CREAT|O_TRUNC);
	    if (fd >= 0) {
		printf("%s created\n", fn);
		close(fd);
	    } else {
		char buf[256];
		sprintf(buf, "%s/___touch___", fn);
		fd = open(buf, O_WRONLY|O_CREAT|O_TRUNC);
		if (fd >= 0) {
		    close(fd);
		    remove(buf);
		    printf("directory %s touched\n", fn);
		}
	    }
	}
    }
    return(0);
}

