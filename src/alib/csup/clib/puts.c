extern long stdout;

extern fprintf( long out, char *fmt, ... );

/* print the string s to standard output */
puts( s )
    char *s;
{
    fprintf (stdout, "%s\n", s);
}
