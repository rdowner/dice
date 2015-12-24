
extern int _doprnt(long stream, char *fmt, char **args);

fprintf( out, fmt, args )
long out;
char *fmt, *args;
{
    return( _doprnt( out, fmt, &args ) );
}
