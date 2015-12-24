
extern fputc( char c, long stream);

/* print the string s to standard output */
fputs( s, stream )
   char *s;
	long stream;
{
   for (; *s != '\0'; s++) { fputc( *s, stream ); }
}  /* puts() */
