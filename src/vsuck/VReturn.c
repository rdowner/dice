/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */
/*
**      $Id: VReturn.c,v 30.0 1994/06/10 18:09:35 dice Exp $
**
**      Convert a directory of examples into a directory of ventura
**      @EXAMPLE lines.
*/

#include <stdio.h>
#include <string.h>

#define unless(x)       if(!(x))
#define MAX_NAME        256

#define unless(x)       if(!( x ))

char *SetSName(char *, char *);
char *TailPath(char *path);

main(xac, xav)
int xac;
char *xav[];
{
    short i;
    FILE *fi=0;
    FILE *fo=0;
    int ac;
    char **av;

    expand_args(xac, xav, &ac, &av);

    if (ac < 2) {
        puts("VReturn <pattern>");
        puts("; Convert .c files to .v ventura @EXAMPLE_LIB entries");
        exit(1);
    }

    for (i = 1; i < ac; ++i) {
        char *file = av[i];
        short len = strlen(file);
        short doth = 1;

        if (!( len >= 2 && (file[len-1] == 'c' || file[len-1] == 'C') && file[len-2] == '.'))
                {
                doth =0;
                printf("Error: name %s does not end with .c\n",file);
                }

        fi = fopen(file, "r");
        if (fi && doth) {

            file[len-1]='v';
            printf("Processing file %s\n",file);

            fo = fopen(file, "w");
            if (!fo) {
                printf("Error: Unable to open %s for write\n", file);
                exit(1);
            }

            scancfile(fi, fo, file);

            fclose(fo);
            fclose(fi);
            fo = fi = 0;
        } else {
            printf("Unable to read %s\n", file);
        }
    }
    return(0);
}

scancfile(FILE *in, FILE *out, char*name)
{
char    c;

        fprintf(out,"\r\n@EXAMPLE_LIB = ");

        while( EOF != (c=fgetc(in)) )
                {
                switch ( c ) {
                        case 10:        // LineFeed
                                fprintf(out,"<R>\r\n");
                                break;
                        case '<':
                                fprintf(out,"<<");
                                break;
                        case '>':
                                fprintf(out,">>");
                                break;
                        default:
                                unless( c==13 )        // We force CR, LF
                                        fputc(c, out);
                        }
                }
        fprintf(out,"\r\n");
        fprintf(out,"\r\n");
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
