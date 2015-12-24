#include <clib/dos_protos.h>

extern stdin;

getchar()
{
    char    c[4];               /* with padding */

    if (Read (stdin, &c[0], 1) != 1) {
	return (-1);
    }

    return (c[0]);
}
