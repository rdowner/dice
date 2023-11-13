#include <clib/dos_protos.h>

fputc( c, stream )
        char c;
        long stream;
{
        Write( stream, &c, 1 );
}       /* fputc() */
