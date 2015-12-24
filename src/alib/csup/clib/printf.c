
extern int _doprnt(long stream, char *fmt, char **args);

extern long stdout;

printf( fmt, args )
char *fmt, *args;
{
    return( _doprnt( stdout, fmt, &args ) );
}
