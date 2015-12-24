/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */
/*
**      $Id: StripADoc.c,v 30.0 1994/06/10 18:05:51 dice Exp $
**
**	Process full Commodore AutoDoc files into a reduced version they'll
**	actually let us ship.
**
*/
#define D(x)    ;

/*
 *  StripADoc output <pattern>
 *
 */

#include <stdio.h>
#include <string.h>

#define unless(x)       if(!(x))

#define	FF	12

char *SetSName(char *, char *);
char *TailPath(char *path);
char *strcpyc(char *d, char *s, char c);

main(xac, xav)
int xac;
char *xav[];
{
    short i;
    FILE *fi;
    FILE *fo;
    int ac;
    char **av;

    expand_args(xac, xav, &ac, &av);

        // !!! Add ? support
    if (ac < 2) {
	puts("StripAdoc appendfile <sourcepattern> <sourcepattern>");
	puts(";Convert full AutoDocs into reduced format");
	exit(1);
    }
    fo = fopen(av[1], "a");
    if (!fo) {
	printf("Error: Unable to open %s for append\n", av[1]);
	exit(1);
    }
    for (i = 2; i < ac; ++i) {
	char *file = av[i];
	short len = strlen(file);

	fi = fopen(file, "r");
	if (fi)
	{
	    printf("Scanning DOC file: %s\n", file);
	    scandocfile(fi, fo, file);
	    fclose(fi);
	} else
	{
	    printf("Unable to read %s\n", file);
	}
    }
    return(0);
}




#define	ST_AFTER_RESULT	0
#define	ST_AFTER_FF	1

scandocfile(fi, fo, filename)
FILE *fi;
FILE *fo;
char *filename;
{
    char buf[256];
    int  state=ST_AFTER_FF;

    while (fgets(buf, 256, fi)) {
	short len = strlen(buf) - 1;
	char *bas = buf;

	buf[len] = 0;

	while( *bas ) {
		switch( *bas ) {
			case 'F':
				if( 0==strncmp(bas,"FUNCTION",8) )
					state = ST_AFTER_RESULT;
				break;
			case 'I':
				if( 0==strncmp(bas,"INPUT",5) )
					state = ST_AFTER_RESULT;
				break;
			case FF:
				state = ST_AFTER_FF;
				break;
			default:
		}
		bas++;
	}

	if( state )
	    fprintf(fo,"%s\n",buf);
	}

}


//      Like strcpy, but stops at the given character
char *strcpyc(char *d, char *s, char c)
{
    char *base = d;

    while (*d = *s) {
        if (*s == c) {
            *d = 0;
            break;
            }
	++s;
	++d;
    }
    return(base);
}


/*
**      Extract name from whitespace
*/
char *
SetSName(buf, ptr)
char *buf, *ptr;
{
    while (*ptr == ' ' || *ptr == 9)
	++ptr;
    while (*ptr && *ptr != '\n' && *ptr != ' ' && *ptr != 9 && *ptr != 12)
	*buf++ = *ptr++;
    *buf = 0;
    return(ptr);
}


/*      0=not alpha
**      1=alpha
*/
IsAlphaNum(c)
char c;
{
    if ((c >= 'a' && c <= 'z') ||
	(c >= 'A' && c <= 'Z') ||
	(c >= '0' && c <= '9') ||
	(c == '_') || (c == '-') || (c == ',') || (c == '.') || (c == '(') || (c == ')')
    )
	return(1);
    return(0);
}


/*
**      Return filename portion of complete path
*/
char *TailPath(char *path)
{
char *last;

    unless( last=strrchr(path,'/' ) )    // Return last slash...
        unless( last=strrchr(path,':') ) // or if no slash, last :
            return( path );             // or if neither, input string
    return( last+1 );
}
