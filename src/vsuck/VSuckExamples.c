/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */
/*
**      $Id: VSuckExamples.c,v 30.0 1994/06/10 18:09:36 dice Exp $
**
**      Hack of early VSuck to extract examples for testing.
**
**      (C) Copyright 1992, Obvious Implementations, All Rights Reserved
*/
#define D(x)    ;
#define D1(x)   ;
#define D2(x)   ;
#define LINE_WIDTH      77      // So text won't wrap on 640 pixel wide screens
#define MAX_PARAGRAPH   8000    // Yes... build that same limitation in.

#define MIN_INDENT      4
#define BODY_INDENT     8
#define OPTION_INDENT   12
#define EXAMPLE_INDENT  4       // Extra bytes past normal indent

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define unless(x)       if(!(x))
#define CR              13
#define LF              10
#define FF              12
#define TAB             9

/*******************************************************************************
**      Globals
*/
FILE    *outfile;                       // FP for final output

char    allspaces[] ="                                                                                ";
char    allhash[]   ="################################################################################";
char    filename[]  ="**INVALID***********************************************************************";
char    filename_nopath[]  ="**INVALID*************************";
char    *pathname;
char    *examplename=0;
char    outbuf[LINE_WIDTH+3];           // Leave room for CR,LF,NULL
char    formatbuf[MAX_PARAGRAPH];       // Yes... build the same limitation in.
char    indenttext[LINE_WIDTH+3];       // For text in the left margin
char    indenttextvalid=0;              //      -option  This is an option

int     bodyindent      =BODY_INDENT;
int     optionindent    =OPTION_INDENT;
int     indent          =0;

char          *startinp;                // Start of input
register char *inp;                     // Walking input pointer

int     enum_val=0;                     // For @ENUM auto-counting



/*******************************************************************************
**      Prototypes
*/
FilterCR(register char *leading);
void ProcessFile();
void putn(char *string,char terminator);
void FormatParagraph(int slen,char *prestring);
int  CollectParagraph(void);


/******************************************************************************/
main(int argc,char *argv[])
{
FILE    *infile;
long     insize;

    if (argc < 3 || ( argc == 2  &&  *argv[1]=='?' ) ) {
        printf("VSuckExamples <infile> <outdir> [example_name]\n");
        printf("; Extract examples from Ventura.  Use for compile-checking.\n");
        exit(5);
        }

    D2(printf("Argc=%d, Arg1=%s, Arg2=%s\n",argc,argv[1],argv[2]));

    unless( infile=fopen(argv[1],"r" ) ) {
        printf("Error: Can't read \"%s\"\n",argv[1]);
        exit(10);
        }
    outfile=0;
    pathname=argv[2];

    if( argc == 4 ) {
        examplename=argv[3];
        printf("Extracting only example %s\n", examplename);
        }

    /*
    unless( outfile=fopen(argv[2],"w" ) ) {
        printf("Error: Can't write to \"%s\"\n",argv[2]);
        fclose(infile);
        exit(10);
        }
     */

    inp=0;
    fseek(infile,0,2);
    if( insize=ftell(infile) )
        {
        fseek(infile,0,0);
        if ( inp=malloc( insize+4 ) )
            {
            if ( insize==fread( inp,1,insize,infile ) )
                {
                inp[insize]  =0;        // Lots of space for sloppy code
                inp[insize+1]=0;        // to read past without harm
                inp[insize+2]=0;
                inp[insize+3]=0;
                }
                else
                {
                printf("Error: Can't read \"%s\"\n",argv[1]);
                free( inp );
                inp=0;
                }
            }
            else
            {
            printf("Error: Out of memory (%ld bytes required)\n",insize+2);
            }
        }

    startinp=inp;
    if( inp ) {
        FilterCR(inp);          // Nuke all CR's
        ProcessFile();
        free( startinp );
        }

    fclose( infile );
    if (outfile)
        fclose( outfile );
    printf("Exiting.  Have a _very_ nice day.\n");
}


/*******************************************************************************
**      Misc utility functions
*/
//      Like puts, put terminates at given character
void putn(char *string,char terminator)
{
char *temp;

        temp=strchr(string,terminator);
        if (temp)
            fwrite(string,temp-string+1,1,stdout);
}

//      Like strnlen, but counts to given character
int strlenc(s,c)
const char *s;
char    c;
{
    const char *b = s;

    while (*s != c)
        ++s;

    return(s - b);
}

//      Given start and current, count newlines to come up with a line number
//              !!! TODO: Add cache
int ErrorLine(char *start, char *current)
{
int     linenum =1;

        for(;start < current; start++)
                if ( *start==LF )
                        linenum++;

        return(linenum);
}

// Do in-place removal of CR
FilterCR(register char *leading)
{
register char *trailing=leading;
register char c;

        while( c=*leading++ )
                {
                if (c != CR)
                        *trailing++=c;
                }
        *trailing=0;
        trailing++;
        *trailing=0;
        trailing++;
        *trailing=0;
        trailing++;
        *trailing=0;
        trailing++;
}

/*******************************************************************************
**      @ Functions
*/

char *MAJOR_HEADING()           // Set filename for result file
{
int     len;
char    *temp;
char    c;

        len=CollectParagraph();

        if (temp=strchr(formatbuf,',')) // Use only first item of a multiple
                *temp=0;                // function example

        len = strlen( formatbuf );      // Recalculate length

        if (len < LINE_WIDTH-10 ) {
                strcpy(filename, pathname);

                if( strlen(pathname) ) {
                        c=pathname[strlen(pathname)-1];
                        if (!( c==':' || c=='/' ))
                                strcat(filename, "/");
                }
                strcat(filename, formatbuf);
                strcpy(filename_nopath, formatbuf);
                strcat(filename, ".c");
                strcat(filename_nopath, ".c");
        }
        else    {
                strcpy(filename, pathname);
                strcat(filename, "/");
                strcat(filename, "** INVALID **");
        }
}

char *EXAMPLE()
{
        if( examplename )
                if( strcmp( examplename, filename_nopath ) ) {
                        CollectParagraph();
                        return(0);
                        }

        unless( outfile=fopen(filename,"w" ) ) {
            printf("Error: Can't write to \"%s\"\n",filename);
            return(0);
        }
        printf("Processing file \"%s\"\n",filename);

        FormatParagraph(CollectParagraph(),"");

        fclose(outfile);
        outfile=0;
}

int CollectParagraph()
{
int     i=0;

    while( *inp )
        {
        if ( (*inp==LF) && (*(inp+1)==LF) )     // Two LF's signal end of para
                break;

        if ( *inp==CR ) {
            printf("Error: File must not contain CR's\n");
            break;
            }

        if ( i > (MAX_PARAGRAPH-100) ) {
            printf("Error: Paragraph too large!\n");
            break;
            }

        //      Escape double <<brackets>>. Skip single <brackets>
        //
        if ( inp[0] == '<' && !(inp[1] == '<') ) {
                if ( inp[1]=='R' && inp[2]=='>' )
                        {
                        formatbuf[i++]=LF;
                        if ( inp[3] == '\n' )
                                inp++;
                        inp += 3;
                        }
                else    {
                        while( *inp && !(*inp=='>') )
                                inp++;
                        if (*inp)
                                inp++;  // Skip trailing >
                        }
                }
        else if ( inp[0] == '<' && inp[1] == '<' )
                formatbuf[i++]='<',inp += 2;
        else if ( inp[0] == '>' && inp[1] == '>' )
                formatbuf[i++]='>',inp += 2;
        else if ( *inp == LF )
                formatbuf[i++]=' ',inp++;
        else
                formatbuf[i++]=*inp,inp++;
        }
    formatbuf[i]=0;
    formatbuf[i+1]=0;
    formatbuf[i+2]=0;
    formatbuf[i+3]=0;

    return(i);
}

void FormatParagraph(int slen,char *prestring)
{
int     pos=0;
int     scan=0;
int     perline= LINE_WIDTH + 1 - strlen(prestring) - indent;
int     i;

    D1(printf("FormatParagraph. Slen=%d, prestring=%s\n", slen, prestring));
    D2(printf("%s", formatbuf));

    // Process normal text paragraph.  Wrap words, nuke spaces at start
    // of line, etc.  Handle <R>, grumble.
    while ( pos < slen )
        {

        // If there is a newline within the current line range, pump out
        // that string.  Else set scan to just past end of line (or EOF)
        // and work backwards to word-wrap.
        scan=0;
        for (i=0; i < perline; i++)
                {
                if ( !formatbuf[pos+i] )
                        break;
                if ( formatbuf[pos+i] == '\n' )
                        {
                        scan = pos+i+1; // Include linefeed
                        D(printf("Newline found at %d\n",i));
                        break;
                        }
                }

        if( !scan )
                {
                // Skip leading spaces
                while ( formatbuf[pos]==' ' )
                        pos++;
                if ( pos >= slen )
                        break;

                scan = pos + perline;   // Just past end of line

                if ( scan > slen)
                        scan = slen;    // Set to EOF

                // Scan backwards for NULL, LF, space separator or start of line
                while( scan > pos && formatbuf[scan] && formatbuf[scan] != ' ' )
                        scan--;

                D(printf("Word-wrapped to %d bytes\n",scan-pos));

                if (pos == scan)
                        printf("ERROR: Huge word (near line %d)\n",ErrorLine(startinp,inp));
                }

        if ( indenttextvalid )
                fwrite(indenttext,indent,1,outfile) , indenttextvalid=0;
        else
                fwrite(allspaces,indent,1,outfile);

        fprintf(outfile,prestring);
        fwrite(&formatbuf[pos],scan-pos,1,outfile);
        if ( formatbuf[scan-1] != '\n' )
                fprintf(outfile,"\n");

        pos = scan;
        }
    fprintf(outfile,"\n");
}


/*******************************************************************************
**      @ Function tables & flags
*/
#define ATSKIP          4       // Longs per atstuff entry

#define AF_OPT          0x0001  // Option indenting
#define AF_BODY         0x0002  // Body indenting
#define AF_NOFLUSH      0x0004  // Disable flush before next @COMMAND

char *(*atstuff[])()={
"@MAJOR HEADING",       MAJOR_HEADING,  AF_BODY,        0,
"@EXAMPLE",             EXAMPLE,        AF_OPT,         0,
"@WIDEEXAMPLE",         EXAMPLE,        AF_OPT,         0,
"@EXAMPLE_LIB",         EXAMPLE,        AF_OPT,         0,
0,                      0,              0,              0,
};

/*******************************************************************************
**      Processfile.  Until reaching a null, scan the input file for
**      @XXX keywords.  Executue @ function, or print warning and keep
**      scanning.
*/
void ProcessFile()
{
int     j;
int     atindex;

    D(printf("ProccessFile: outfile=%lx, inp=%lx\n",outfile,inp));

    while( *inp )
        {
        if (*inp == LF)
            *inp++;
        else if (*inp == CR)
            {
            printf("Error: File must not contain CR's\n");
            break;
            }
        else if (*inp == '@')
            {
            /*
            **  Find matching @ command
            */
            for( atindex=0; atstuff[atindex]; atindex += ATSKIP)
                {
                if( !strncmp(atstuff[atindex], inp, strlen(atstuff[atindex])) )
                    break;
                }

            /*
            **  Execute @ command, if any.  Else flush any pending line.
            */
            if( !atstuff[atindex] )
                {
                // Skip to next signifcant thing
                inp++;
                while( *inp && !(*inp=='@') && !(inp[0]==LF && inp[1]==LF) )
                    inp++;
                }
            else{
                D1( printf("Matched tag: %s\n",atstuff[atindex]) );

                while( *inp && !(*inp=='=') )   // Skip to '='
                    inp++;
                unless( inp[1]==' ' && inp[2])
                        {
                        printf("Warning: badly formatted @ tag or no example (%d)\n",ErrorLine(startinp,inp));
                        while( *inp && !(*inp=='@') && !(inp[0]==LF && inp[1]==LF) )
                               inp++;
                        }
                else    {
                        inp += 2;

                        for( j=0; j<LINE_WIDTH+2; j++)
                            outbuf[j]=' ';
                        outbuf[j+1]=0;


                        //      if( atstuff[atindex+3] )
                        //          indent = atstuff[atindex+3];

                        if( indenttextvalid )
                            printf("Warning: Left margin text lost (%d)\n",ErrorLine(startinp,inp));

                        (*atstuff[atindex+1])(inp);
                        }
                }
            }
        else{
            FormatParagraph(CollectParagraph(),"");
            }
        }
    D(printf("End ProccessFile: outfile=%lx, inp=%lx\n",outfile,inp));
}

