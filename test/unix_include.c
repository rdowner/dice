
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>

/*
 * Dummy procedure to test that -CTOD does not generate funny assembly
 * when it is supposed to be generating D code.
 */
int
main(int ac, char **av)
{
    return(0);
}

struct fubar {
	int a;
	int b;
};

void charlie(struct fubar *f1, const struct fubar *f2);

