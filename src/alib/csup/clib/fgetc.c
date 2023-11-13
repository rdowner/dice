#include <clib/dos_protos.h>

extern stdin;

fgetc(stream)
long stream;
{
    char    c[4];               /* with padding */

    if (Read (stream, &c[0], 1) != 1) {
        return (-1);
    }

    return (c[0]);
}
