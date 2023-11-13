/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */
/*
**      $Id: vsuck.c,v 30.0 1994/06/10 18:09:35 dice Exp $
**
**              See what can be done with WIDEEXAMPLE
**
**              Start processing [$Marker] references
**
**      BUGS:
**              Huge AslBase... line blows MAJOR_HEADING
**
**              strftime man page
**              Notice <R> in tables
**              Notice <197> 
**              
**
**              !!!!!!!!!!!!!!!!!!!!!!!!!End-of-line blanks
**
**              Table mismatch in chap01.txt
**
**
**              Flubs on:
**                      Spaces at end of line
**
**              Flubs on:
**                      @NAME = <TAB>KEYWORD<R>
**                      <TAB>SUBTITUTION
**
**              Flubs on:
**                      @OPTION = <TAB>-S0<R>
**                      <TAB>-S1
**
**              Does not handle <TAB> in @INPUT, @OUTPUT, etc.
**
**              atexit example has NULL in it.
**
**      FEATURES:
**              The final line of paragrah terminated with <R> will have
**              leading spaces stripped:
**                              #include <<stdio.h>><R>
**                              #include <<cats.h>>
**              This will not be fixed; VReturn puts a final <R> on the
**              last line.
**
**      TODO:
**              Tab optimize
**              Check exact length of MAJOR_HEADING wrap.
**
*/
#define D(x)    
#define D0(x)   
#define D1(x)
#define D2(x)   
#define D3(x)   x;      // Warnings
#define LINE_WIDTH      76      // So text won't wrap on 640 pixel wide screens
#define MAX_PARAGRAPH   8000    // Yes... build that same limitation in.

#define MIN_INDENT      4
#define BODY_INDENT     8
#define TABLE_INDENT    1
#define OPTION_INDENT   12
#define D_INDENT        28      // Text after options, results
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
**      Column Stuff
*/
#define DC1             0x11  // Standin for ',' in tables 0x40.
#define DC2             0x12  // Standin for <R> in tables 0x23.
#define MAXCOLUMNS      10

short   hasheader;      // Set if table has a header (thick line used)

struct ColumnInfo {     // Hold information about each of n columns

        // Prescan information
        short maxfirst;   // Largest first word
        short maxword;    // Largest word (make column no smaller!)
        short maxline;    // Largest line (try to fit in this space)
        short tablelines; // Number of rows (redundant in each column)
        int   total;      // Total characters in this column (for ratio calc)
        short flags;      //

        // Formatting information
        short col_start;        // Start of actual text
        short col_width;        // During format, location of vertical bar
        short col_temp;
        char *wrappointer;      // Current index, or zero if column is done
};

/*******************************************************************************
**      Globals
*/
FILE    *outfile;                       // FP for final output

struct  ColumnInfo ColInfo[MAXCOLUMNS]; // Array of ColumnInfo

char    allspaces[] ="                                                                                ";
char    allhash[]   ="################################################################################";
char    alldash[]   ="--------------------------------------------------------------------------------";
char    outbuf[LINE_WIDTH*2];           // Leave room for CR,LF,NULL, slop
char    formatbuf[MAX_PARAGRAPH];       // Yes... build the same limitation in.

#define INDENT_INVALID  0
#define INDENT_OPTION   1               // *must* be followed by text
#define INDENT_VFORMAT  2               // *may*  be followed by text
char    indenttext[LINE_WIDTH*2];       // For text in the left margin
char    indenttextvalid=0;              //      -option  This is an option

int     bodyindent      =BODY_INDENT;
int     optionindent    =OPTION_INDENT;
int     indent          =BODY_INDENT;

char          *startinp;                // Start of input
register char *inp;                     // Walking input pointer

int     enum_val=1;                     // For @ENUM auto-counting



/*******************************************************************************
**      Prototypes
*/
void ProcessFile();
void putn(char *string,char terminator);
void FormatParagraph(int slen,char *prestring1,char *prestring2);
int  CollectParagraph(void);
int CollectTable(void);
FilterSpaces(register char *leading);
FilterCR(register char *leading);


/******************************************************************************/
main(int argc,char *argv[])
{
FILE    *infile;
long     insize;

    if (argc != 3 || ( argc == 2  &&  *argv[1]=='?' ) ) {
        printf("VSuck <infile> <outfile>\n");
        printf("; Format a Ventura document as text.  Like NROFF\n");
        exit(5);
        }

    unless( infile=fopen(argv[1],"r" ) ) {
        printf("Error: Can't read \"%s\"\n",argv[1]);
        exit(10);
        }
    unless( outfile=fopen(argv[2],"w" ) ) {
        printf("Error: Can't write to \"%s\"\n",argv[2]);
        fclose(infile);
        exit(10);
        }

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
        FilterCR(inp);  // Delete all CR
        ProcessFile();
        free( startinp );
        }

    fclose( infile );
    fclose( outfile );
    printf("Exiting. Remember to tab & space optimize output!!!!\n");
    printf("         Remember to tab & space optimize output!!!! Have a nice day.\n");
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

// Do in-place removal of spaces
FilterSpaces(register char *leading)
{
register char *trailing=leading;
register char c;

        while( c=*leading++ )
                {
                if (c != ' ')
                        *trailing++=c;
                }
        *trailing=0;
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

char *VFORMAT()
{
int     len=0;

        len=CollectParagraph();

        unless(len && (len < LINE_WIDTH) ) {
            printf("Warning: @INPUTV/RESULTV no text/too long (%d)\n",ErrorLine(startinp,inp));
            return(0);
            }
        D(printf("@VFORMAT: inp=%lx,len=%d,indent=%d,string=%s\n",inp,len,indent,formatbuf));

        /* Treat VFORMAT like an option.  New line if it won't fit properly.
        */
        if ( len+3 > (D_INDENT-indent-1))
            {
            memcpy(&outbuf[indent],&formatbuf[0],len+1);  // Copy null
            fprintf(outfile,"%s\n",outbuf);
            }
        else{
            memcpy(&outbuf[indent],&formatbuf[0],len);
            strcpy(indenttext,outbuf);      // Copy string + lots of spaces
            indenttextvalid=INDENT_VFORMAT;
            }

        return(0);
}


char *OPTION()
{
int     len=0;

        len=CollectParagraph();

        unless(len && (len < LINE_WIDTH) ) {
            printf("Warning: @OPTION no text/too long (%d)\n",ErrorLine(startinp,inp));
            return(0);
            }

        D1(printf("@OPTION: inp=%lx,len=%d,indent=%d,string=%s\n",inp,len,indent,formatbuf));

        if ( formatbuf[0] == TAB )
                printf("Warning: TAB before OPTION text (%d)\n", ErrorLine(startinp,inp));

        /*
        ** If the option fits in the indent area, right justify it.
        ** else insert a line with just the option on it.
        **
        */
        if ( len+3 > indent)
            {
            memcpy(&outbuf[1]           ,&formatbuf[0],len+1);  // Copy null
            fprintf(outfile,"%s\n",outbuf);
            }
        else{
            memcpy(&outbuf[indent-len-2],&formatbuf[0],len);
            strcpy(indenttext,outbuf);      // Copy string + lots of spaces
            indenttextvalid=INDENT_OPTION;
            }

        return(0);
}

char *MAJOR_HEADING()
{
int     len;
char    *description=0; // Second part of new style MAJOR_HEADING lines
                        // @MAJOR_HEADING = wc - Count words in file

        CollectParagraph();

        if ( description = strchr(formatbuf,'-') ) {
            if ( (description[-1] != ' ') || (description[1] != ' ') )
                printf("Warning: Non-standard dashed MAJOR_HEADING (%d)\n",ErrorLine(startinp,inp));
            description[-1] = 0;
            description += 2;
            }

        FilterSpaces(formatbuf);
        len=strlen(formatbuf);

        if ( 6+5+len+len+1 > LINE_WIDTH)
                {
                D3(printf("Warning: MAJOR_HEADING very long (%d bytes) (%d)\n",len , ErrorLine(startinp,inp)));
                fprintf(outfile,"dice/%s\n\n",formatbuf);
                }
        else    {
                memcpy(outbuf  ,"dice/",6);
                memcpy(outbuf+6,formatbuf,len);
                memcpy(outbuf+LINE_WIDTH-len-5+1,"dice/",5);
                memcpy(outbuf+LINE_WIDTH-len  +1,formatbuf,len);
                outbuf[LINE_WIDTH+1]=0;
                fprintf(outfile,"%s\n\n",outbuf);
                }

        if ( description ) {
                fprintf(outfile,"",outfile);
                fwrite(allspaces,MIN_INDENT,1,outfile);
                fprintf(outfile,"FUNCTION\n");
                fwrite(allspaces,BODY_INDENT,1,outfile);
                fprintf(outfile,"%s\n\n",description);
        }
}

char *BEGINNER()
{
        FormatParagraph(CollectParagraph(),":: ",":: ");
}

char *NOTE()
{
        FormatParagraph(CollectParagraph(),"|| ","|| ");
}

char *WARNING()
{
        FormatParagraph(CollectParagraph(),"## ","## ");
}

char *BULLET()
{
        FormatParagraph(CollectParagraph(),"o ","  ");
}

char *GENERIC()
{
        FormatParagraph(CollectParagraph(),"","");
}

char *EXAMPLE()
{
        // indent += EXAMPLE_INDENT;
        FormatParagraph(CollectParagraph(),"","");
        // indent -= EXAMPLE_INDENT;
}

char *ENUM()
{
char    temp[16];

        sprintf(temp,"%d) ",enum_val);
        enum_val++;
        FormatParagraph(CollectParagraph(),temp,"   ");
}

char *CATEGORY()
{
        fwrite(allspaces,MIN_INDENT,1,outfile);
        fprintf(outfile,"NAME\n");
        FormatParagraph(CollectParagraph(),"","");
}

char *NAME()
{
int      len;
char    *temp;

        len=CollectParagraph();
        temp=formatbuf;
        if (len)
            if (formatbuf[0]==TAB)      // TAB means right justify
                temp++;
        strcpy(&outbuf[MIN_INDENT],temp);
        fprintf(outfile,"%s\n",outbuf);
}

char *MINOR_HEADING()
{
int      len;
char    *temp;

        len=CollectParagraph();
        temp=formatbuf;

        if (len)
            if (formatbuf[0]==TAB) {    // TAB means right justify
                temp++;
                len--;
                printf("Warning: TAB in MINOR_HEADING (%d)\n", ErrorLine(startinp,inp));
            }
        if (len) {
            strcpy(&outbuf[MIN_INDENT],temp);
            fprintf(outfile,"%s\n",outbuf);
            memcpy(&outbuf[MIN_INDENT],alldash,len);
            outbuf[MIN_INDENT+len]=0;
            fprintf(outfile,"%s\n",outbuf);
        }
}

char *Z_TBL_BEG()
{

        // Skip @Z_TBL_BEG.  Turns out, we don't use any information from it!
        while( *inp && !(inp[0]==LF && inp[1]==LF) )
            inp++;

        // Strip formatting
        CollectTable();
        D1(printf("@TABLE: inp=%lx,string=\n%s\n",inp,formatbuf));

        // Collect statistics, then Write to output
        WriteTable(PrescanTable(formatbuf),formatbuf);
}


/*
**      Figure out how to format the table
*/
#define MAGIC_SMALLCOLUMN 11     // Treat small first columns preferentially

int ThinkDeeply(int columns)
{
int     leftmargin;
int     i,j,whatif,whatif2;
short   columnspace;            // Space available to assign widths

    columnspace = LINE_WIDTH-TABLE_INDENT-((columns-1)*3);
    D1(printf("ThinkDeeply: Assignable columns = %d\n",columnspace));

    /*
    **  Determine if table is impossible, and clear out col_start/col_width
    */
    whatif=0;
    for ( i=0; i < columns; i++) {
        whatif += (ColInfo[i].maxword);
        ColInfo[i].col_start   = 0;
        ColInfo[i].col_width   = 0;
        ColInfo[i].col_temp    = 0;
        ColInfo[i].wrappointer = 0;
    }
    if (whatif > columnspace ) {
        printf("Error: Impossible table; words too large (%d)\n",ErrorLine(startinp,inp));
        return(0);
    }

    /*
    **  Now try to fit all text on a single line
    */
    whatif=0;
    for ( i=0; i < columns; i++)
        whatif += (ColInfo[i].maxline);
    D1(printf("Total maximum length (whatif)=%d\n",whatif));

    /*
    **  Ideal Situation.  Everything fits on one line.  Center the column
    */
    if (whatif < columnspace ) {
        leftmargin = ((columnspace-whatif) / 2) + TABLE_INDENT;
        j = leftmargin;
        for ( i=0; i< columns; i++) {
            ColInfo[i].col_start = j;
            ColInfo[i].col_width = ColInfo[i].maxline;
            j = j + 3 + ColInfo[i].maxline;
        }
        D1(printf("ThinkDeeply chooses: Single line\n"));
        return(leftmargin);
    }

    /*
    **  Next, fit small first column with single large second column
    */
    if ( columns == 2 && ColInfo[0].maxline < MAGIC_SMALLCOLUMN ) {
        ColInfo[0].col_start = TABLE_INDENT;
        ColInfo[0].col_width = ColInfo[0].maxline;
        ColInfo[1].col_start = ColInfo[0].maxline+TABLE_INDENT+3;
        ColInfo[1].col_width = LINE_WIDTH-TABLE_INDENT-3-ColInfo[0].maxline;
        D1(printf("ThinkDeeply chooses: Small first column\n"));
        return(TABLE_INDENT);
    }

    /*
    **  No tricks worked :-(.  Assign widths based on weight of text.  The
    **  more text in a column, the wider.  No column may be smaller than
    **  the largest word.
    **
    */
    // First, add up total of all text.
    whatif = 0;
    for ( i=0; i < columns; i++) {
        whatif += ColInfo[i].total;
    }
    D1(printf("ThinkDeeply brute force: %d total characters\n",whatif));

    // Second, calculate ratios.  More text means larger column.
    whatif  *= 1000;                    // Total chars
    whatif2  = whatif / columnspace;    // Space avail

    // Third.  Assign columns based on ratios.
    for ( i=0; i < columns; i++) {
        j = ColInfo[i].total;
        D1(printf("Aproximate column width = %d\n",(j * 1000) / whatif2));
        ColInfo[i].col_temp = ((j * 1000) / whatif2);
        ColInfo[i].col_width= 0;
    }

    // Fourth.  Evaluate results against rules and assign hard values
    for ( i=0; i < columns; i++) {
        j = ColInfo[i].col_temp;

        // If assigned column was too small, hard assign minimum width
        if ( j < ColInfo[i].maxword )
                ColInfo[i].col_width=ColInfo[i].maxword;

        // If assigned column larger than longest line, trim to minimum
        if ( j > ColInfo[i].maxline )
                ColInfo[i].col_width=ColInfo[i].maxline;

        D1(printf("Hard-set column width = %d\n",ColInfo[i].col_width));
    }

    // Fifth, add up text in columns which don't yet have a fixed size,
    //  and number of already-assigned columns.
    whatif  = 0;
    whatif2 = 0;
    for ( i=0; i < columns; i++ ) {
        if ( ColInfo[i].col_width )
            whatif2 += ColInfo[i].col_width;
        else
            whatif  += ColInfo[i].total;
    }
    D1(printf("After fixed assignments: %d total, %d preassigned\n",whatif,whatif2));


    // Second, calculate ratios.  More text means larger column.
    whatif  *= 1000;                   // Total chars in unassigned columns
    whatif2  = whatif  / (columnspace-whatif2);    // Space avail

    // Third.  Assign columns based on ratios.
    for ( i=0; i < columns; i++) {
        j = ColInfo[i].total;
        if ( 0==ColInfo[i].col_width) {
            D1(printf("Col %d new aprox. width = %d\n",i,(j * 1000) / whatif2));
            ColInfo[i].col_width = ((j * 1000) / whatif2);
        }
    }

    //
    // !!! :TODO: Readjust plus or minus to fix any rounding errors
    // in total assignment.
    //
    j = TABLE_INDENT;
    for ( i=0; i< columns; i++) {
        ColInfo[i].col_start = j;
        j = j + 3 + ColInfo[i].col_width;
    }
    D1(printf("ThinkDeeply chooses: Complex fit\n"));
    return(TABLE_INDENT);
}




/*
**      Dump a ColumnInfo structure.
*/
PrintCI(int x)
{
printf("## %02d First=%02d Word=%02d Line=%04d Total=%04d Lines=%02d/Start=%02d Width=%02d Wrap=%lx\n",
x,ColInfo[x].maxfirst,ColInfo[x].maxword,ColInfo[x].maxline,ColInfo[x].total,
ColInfo[x].tablelines,ColInfo[x].col_start,ColInfo[x].col_width,
ColInfo[x].wrappointer);
}

/*
**      Write table. Given:
**              Columns
**              Information for each column
**              Formatting-stripped, comma separated entries
*/
WriteTable(int columns,char *table)
{
short   flag;
short   cl_width,cl_start;
int     i,j;

char *  start;
char *  current;
char *  lastword;       // End of last word encountered (for word wrap).
char *  temp;
char    rulechar='=';   // Character used for thick horizontal ruling lines

short   leftmargin;


    leftmargin = ThinkDeeply(columns);


    D1(for ( i=0; i< columns; i++))
        D1(PrintCI(i));
    D1(printf("^^ Final statistics for table ending at line %d ^^\n",ErrorLine(startinp,inp)));

    unless( leftmargin ) {
        printf("Error: ThinkDeeply() could not resolve table (%d)\n",ErrorLine(startinp,inp));
        return(0);
    }

    while( *table )
        {
        // Position wrappointer at start of each column (just past ',')
        temp = table;
        for ( i=0; i< columns; i++) {
            ColInfo[i].wrappointer = temp;
            while( *temp && (*temp != DC1) && (*temp != LF) )
                temp++;
            temp++;
        }
        temp--;
        if ( *temp != LF )
            printf("Warning: column count mismatch!\n");

#ifdef  NDEF
        for ( i=0; i< columns; i++) {
            printf("Col %d=%lx,%c%c\t",i,ColInfo[i].wrappointer,*(ColInfo[i].wrappointer),*(ColInfo[i].wrappointer+1));
        }
        printf("\n");
#endif

        flag    =1;     // Zero when all columns have flowed all text
        while( flag ) {
            for(i=0; i<LINE_WIDTH+15; i++)
                outbuf[i]=' ';

            for ( i=0; i< columns; i++) {
                lastword = NULL;
                cl_width=ColInfo[i].col_width;
                cl_start=ColInfo[i].col_start;
                if ( i != columns-1 )   // Except for last column
                        outbuf[cl_width+cl_start+1]=':';
                if ( start=current=ColInfo[i].wrappointer ) {
                    while( *current==' ' || *current=='\t' || *current==DC2 )
                        start++, current++;
                    int noDC2seen = 1;
                    while( *current ) {
                        if ( *current == ' ' || *current == '\t' || *current == DC1 || *current == LF || *current == DC2 )
                            if ( current-start <= cl_width )
                                if ( noDC2seen ) {
                                        lastword = current; // Last full word
                                        if( *current == DC2 )
                                                noDC2seen = 0;
                                        }
                        if ( *current == DC1 || *current == LF ) {
                            if (lastword && (lastword-start)) {
                                memcpy(&outbuf[cl_start],start,lastword-start);
                                }
                            if ( lastword == current)
                                ColInfo[i].wrappointer = NULL;
                            else
                                ColInfo[i].wrappointer = lastword;
                            break;
                        }
                        current++;
                    }
                }
            }

            outbuf[LINE_WIDTH+14]=0;
            fprintf(outfile,"%s\n",outbuf);

            flag =0;
            for ( i=0; i< columns; i++)
                if (ColInfo[i].wrappointer)
                        flag=1;
        }

        /*
        **      Print middle ruling lines
        */
        if( !hasheader )
                rulechar='-';   // Switch immediatly to thin lines

        for(i=0; i<LINE_WIDTH; i++)
            outbuf[i]=' ';
        for ( i=0; i< columns; i++) {
            cl_width=ColInfo[i].col_width;
            cl_start=ColInfo[i].col_start;

            for( j=cl_start; j < cl_start+cl_width+1; j++)
                outbuf[j]=rulechar;
            outbuf[j]=0;
            if ( i != columns-1 )
                outbuf[j]='+', outbuf[j+1]=rulechar;
        }
        fprintf(outfile,"%s\n",outbuf);
        rulechar='-';   // Switch immediatly to thin lines

        while( *table && *table != LF)    // Skip to next line
            table++;
        table++;
        }
        fprintf(outfile,"\n");
}

/*
**      Scan table, counting colums and gathering statistics.  Assumes
**      all formatting codes have been stripped.
**
**      Count number of columns.  For each column, record:
**              Max first word.
**              Max word.
**              Max line.
**              Total characters.
*/
int PrescanTable(char *table)
{
short cnt_chars=0;      // Running total of characters on line
short cnt_wordchars=0;  // Running count of current word

short cnt_columns=0;    // Count of columns on current line
short tot_columns=0;    // Check point.  Do all lines have same # columns?

short cnt_firstword=0;  // Size of first word, copied to ColInfo
short cnt_maxword=0;    // Size of max word, copid to ColInfo
int   i;

    for ( i=0; i < MAXCOLUMNS; i++) {
        clrmem( ColInfo, sizeof(ColInfo) );
    }

    while( *table )
        {
        switch ( *table )
            {
            case ' ':                           // (Note dupe in next case)
            case DC2:
                if( !cnt_firstword )
                    cnt_firstword = cnt_chars;   // Length of first word
                if( cnt_wordchars > cnt_maxword )
                    cnt_maxword = cnt_wordchars; // Length of largest word

                cnt_chars++;
                cnt_wordchars=0;
                break;
            case DC1:
            case LF:
                if( !cnt_firstword )
                    cnt_firstword = cnt_chars;   // Length of first word
                if( cnt_wordchars > cnt_maxword )
                    cnt_maxword = cnt_wordchars; // Length of largest word

                if (ColInfo[cnt_columns].maxfirst < cnt_firstword)
                    ColInfo[cnt_columns].maxfirst = cnt_firstword;
                if (ColInfo[cnt_columns].maxline  < cnt_chars)
                    ColInfo[cnt_columns].maxline  = cnt_chars;
                if (ColInfo[cnt_columns].maxword  < cnt_maxword)
                    ColInfo[cnt_columns].maxword  = cnt_maxword;
                ColInfo[cnt_columns].total       += cnt_chars;
                ColInfo[cnt_columns].tablelines  += 1;

                cnt_columns++;                  // Move to next column
                if ( *table == LF)
                {
                    if ( tot_columns )
                        if ( cnt_columns != tot_columns)
                            printf("Error: Column count mismatch %d != %d (%d)\n",
                                   tot_columns, cnt_columns, ErrorLine(startinp,inp));
                    tot_columns   =cnt_columns;
                    if ( tot_columns > MAXCOLUMNS-1 )
                        printf("Error: Too many columns! (%d)\n", ErrorLine(startinp,inp));
                    cnt_columns   =0;
                }

                cnt_firstword =0;
                cnt_maxword   =0;
                cnt_chars     =0;
                cnt_wordchars =0;
                break;
            default:
                cnt_chars++;
                cnt_wordchars++;
            }
            table++;
        }

        return( tot_columns );
}

/*
**      Strip off all table junk to product comma separated list.
**      Record if a table heading (@Z_TBL_HEAD) was seen.
*/
int CollectTable()
{
int     i=0;

 hasheader=0;

while( *inp )
{

    while (*inp == LF)
        inp++;

    if ( *inp == '@' )
    {
        if ( 0 == strncmp(inp, "@Z_TBL_END", 10) )
        {
            D1(printf("TBL_END\n"));
            while( *inp && !(inp[0]==LF && inp[1]==LF) )
                inp++;
            break;
        }
        if ( 0 == strncmp(inp, "@Z_TBL_HEAD", 10) )
        {
            D1(printf("TBL_TBL_HEAD\n"));
            hasheader=1;
        }
        while (*inp && *inp != LF)      // Skip @ command
            inp++;
    } else {
        while( *inp )
        {
            if ( i > (MAX_PARAGRAPH-100) ) {
                printf("Error: Paragraph too large!\n");
                break;
            }

            /*  Escape ++, -- and ,,.  What a spitwad.
            **  Does not handle joined columns "+," and "^,"
            */
            if      ( inp[0] == LF  &&  inp[1] == LF  ) {
                formatbuf[i++]= LF,     inp+=2;
                break;
            }
            else if ( inp[0] == '+' &&  inp[1] == '+' )
                formatbuf[i++] ='+',    inp++;
            else if ( inp[0] == '^' &&  inp[1] == '^' )
                formatbuf[i++]= '^',    inp++;
            else if ( inp[0] == '<' &&  inp[1] == '<' )
                formatbuf[i++]= '<',    inp++;
            else if ( inp[0] == '>' &&  inp[1] == '>' )
                formatbuf[i++]= '>',    inp++;
            else if ( inp[0] == '<' &&  inp[1] == 'R' && inp[2] == '>' )
                formatbuf[i++]= DC2,    inp+=3;
            else if ( inp[0] == ',' &&  inp[1] == ',' )
                formatbuf[i++]= ',',    inp++;
            else if ( inp[0] == ',' &&  inp[1] == ' ' )
                formatbuf[i++]= DC1,    inp++;
            else if ( inp[0] == ',' )
                formatbuf[i++]= DC1;
            else if ( inp[0] == LF )
                formatbuf[i++]= ' ';
            else if ( inp[0] == '<' )
                {
                short nest=1;
                while( *inp++ && nest )
                    {
                    switch( *inp )
                        {
                        case '>':
                            nest--;
                            break;
                        case '<':
                            nest++;
                        default:
                        }
                    }
                    inp--;
                }
            else
                formatbuf[i++]=*inp;
            inp++;
        }
    }
}
formatbuf[i]=0;
formatbuf[i+1]=0;
formatbuf[i+2]=0;
formatbuf[i+3]=0;
return(i);
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
        if      ( inp[0] == '<' && inp[1] == '<' )
                formatbuf[i++]='<',inp += 2;
        else if ( inp[0] == '>' && inp[1] == '>' )
                formatbuf[i++]='>',inp += 2;
        else if ( inp[0] == '<' ) {
                if ( inp[1]=='R' && inp[2]=='>' ) // Convert <R> to LF
                        {
                        formatbuf[i++]=LF;
                        if ( inp[3] == '\n' )
                                inp++;
                        inp += 3;
                        }
                else if ( inp[1]=='N' && inp[2]=='>' ) // Convert <N> to ` `
                        {
                        formatbuf[i++]=' ';
                        if ( inp[3] == '\n' )
                                inp++;
                        inp += 3;
                        }
                else if ( inp[1]=='$' && inp[2]=='R' )
                        {
                        while( *inp && *inp++ != ']' )
                                ;
                        if( *inp == '>')
                            printf("Warning: No chapter number in Cross-Ref (%d)\n",ErrorLine(startinp,inp));
                        while( *inp && (*inp != '>') ) {
                                formatbuf[i++] = *inp;
                                inp++;
                                }
                        inp++;
                        }
                else    {
                        short nest=1;
                        inp++;
                        while( *inp && nest )
                            {
                            switch( *inp )
                                {
                                case '>':
                                    nest--;
                                    break;
                                case '<':
                                    nest++;
                                default:
                                }
                            inp++;
                            }
                        }
                }
        /*
        **      Ventura seems to put a space at the end of lines, but also
        **      accept files without the space.  As a guess, I force a single
        **      space to the end of the line if none, else accept any number
        **      of spaces found there.
        */
        else if ( *inp == LF ) {
                if(!( formatbuf[i-1]==' ' ))
                        formatbuf[i++]=' ';
            inp++;
            }
        else
                formatbuf[i++]=*inp,inp++;
        }
    formatbuf[i]=0;
    formatbuf[i+1]=0;
    formatbuf[i+2]=0;
    formatbuf[i+3]=0;

    return(i);
}

void FormatParagraph(int slen,char *prestring1,char *prestring2)
{
int     pass=0;
int     pos=0;
int     scan=0;
int     perline= LINE_WIDTH + 1 - strlen(prestring1) - indent;
int     i;

    D2(printf("FormatParagraph. Slen=%d, pre1=%s,pre2=%s\n", slen, prestring1,prestring2));
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
                while ( formatbuf[pos]==' ' || formatbuf[pos]=='\t' )
                        pos++;
                if ( pos >= slen )
                        break;

                scan = pos + perline;   // Just past end of line

                if ( scan > slen)
                        scan = slen;    // Set to EOF

                // Scan backwards for NULL, LF, space separator or start of line
                while( scan > pos && formatbuf[scan] && formatbuf[scan] != ' ' && formatbuf[scan] != '\t' )
                        scan--;

                D(printf("Word-wrapped to %d bytes\n",scan-pos));

                if (pos == scan)
                        printf("ERROR: Huge word (near line %d)\n",ErrorLine(startinp,inp));
                }

        if ( indenttextvalid )
                fwrite(indenttext,indent,1,outfile), indenttextvalid=0;
        else
                fwrite(allspaces,indent,1,outfile);

        if ( pass )
                fprintf(outfile,prestring2);
        else
                fprintf(outfile,prestring1);

        fwrite(&formatbuf[pos],scan-pos,1,outfile);
        if ( formatbuf[scan-1] != '\n' )
                fprintf(outfile,"\n");

        pos = scan;
        pass++;
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
#define AF_ENUM         0x0008  // Don't reset enum_val
#define AF_INDENTOK     0x0010  // For @ command, indent text is ok.

// SNAME is like NAME, but should have no space above.  We can't do
// this, however :-).
void *(*atstuff[])()={
"@NAME",                NAME,           AF_BODY,        BODY_INDENT,
"@SNAME",               NAME,           AF_BODY,        BODY_INDENT,
"@OPTION",              OPTION,         AF_OPT,         OPTION_INDENT,
"@INPUTV",              VFORMAT,        AF_BODY,        BODY_INDENT,
"@RESULTV",             VFORMAT,        AF_BODY,        BODY_INDENT,
"@INPUTD",              GENERIC,        AF_INDENTOK,    D_INDENT,
"@RESULTD",             GENERIC,        AF_INDENTOK,    D_INDENT,
"@MINOR HEADING",       MINOR_HEADING,  AF_BODY,        BODY_INDENT,
"@MAJOR HEADING",       MAJOR_HEADING,  AF_BODY,        BODY_INDENT,
"@CATEGORY",            CATEGORY,       AF_BODY,        BODY_INDENT,
"@BULLET",              BULLET,         AF_BODY,        BODY_INDENT,
"@BEGINNER",            BEGINNER,       AF_BODY,        0,
"@NOTE",                NOTE,           AF_BODY,        0,
"@WARNING",             WARNING,        AF_BODY,        0,
"@VERBATIM",            GENERIC,        AF_BODY,        0,
"@FUNCTION",            GENERIC,        AF_BODY,        BODY_INDENT,
"@LIBRARY",             GENERIC,        AF_BODY,        BODY_INDENT,
"@SYNTAX",              GENERIC,        AF_BODY,        BODY_INDENT,
"@INPUTS",              GENERIC,        AF_BODY,        BODY_INDENT,
"@RESULTS",             GENERIC,        AF_BODY,        BODY_INDENT,
"@DESCRIPTION",         GENERIC,        AF_BODY,        BODY_INDENT,
"@AME_BODY",            GENERIC,        AF_BODY,        BODY_INDENT,
"@EXAMPLE_LIB",         EXAMPLE,        AF_OPT,         0,
"@EXAMPLE",             EXAMPLE,        AF_OPT,         0,
"@WIDEEXAMPLE",         EXAMPLE,        AF_OPT,         0,
"@SEE ALSO",            GENERIC,        AF_BODY,        BODY_INDENT,
"@ENUM",                ENUM,           AF_ENUM,        BODY_INDENT,
"@Z_TBL_BEG",           Z_TBL_BEG,      AF_BODY,        0,
"@Body Text",           GENERIC,        AF_BODY,        BODY_INDENT,
0,                      0,              0,              BODY_INDENT,
};

#ifdef  NOTDEF
struct STRINGS partypes[] = {
   { 0, "CHAPTER HEAD",    NULL         },
   { 0, "INPUTV",          NULL         },
   { 0, "INPUTD",          NULL         },
   { 0, "Z_TBL_BEG",       NULL         },
   { 0, "Z_TBL_BODY",      NULL         },
   { 0, "Z_TBL_HEAD",      NULL         },
   { 0, "Z_TBL_END",       NULL         },
   { 0, "STEP2",           NULL         },
   { 0, "PARAFILTER ON",   NULL         },
   { 0, "ERRMESS",         NULL         },
   { 0, "INDEN1",          NULL         },
   { 0, "INDEN2",          NULL         },
   { 0, "INDEN2M",         NULL         },
   { 0, "INDEN2B",         NULL         },
   { 0, "INDENT",          NULL         },
   { 0, "INDEN2MID",       NULL         },
   { 0, "INDEN1MID",       NULL         },
   { 0, "INDEN1BOT",       NULL         },
   { 0, "INDEN2BOT",       NULL         },
   { 0, "LISTCAP",         NULL         },
   { 0, "LISTBODY",        NULL         },
   { 0, "CAPTION",         NULL         },
   { 0, "DEFINES",         NULL         },
   { 0, "TBL TITLE",       NULL         },
};
#endif

/*******************************************************************************
**      Processfile.  Until reaching a null, scan the input file for
**      @XXX keywords.  Executue @ function, or print warning and keep
**      scanning.
*/
void ProcessFile()
{
int     j;
int     atindex;
//      char    *temp;  DICE BUG

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
                printf("Warning: Unmatched tag: "),putn(inp,'='),printf("\n");

                // Skip to next signifcant thing
                inp++;
                while( *inp && !(*inp=='@') && !(inp[0]==LF && inp[1]==LF) )
                    inp++;
                }
            else{
                D2( printf("Matched tag: %s\n",atstuff[atindex]) );

                while( *inp && !(*inp=='=') )   // Skip to '='
                    inp++;
                unless( inp[1]==' ' && inp[2])
                        {
                        D3(printf("Warning: nothing after @ tag (%d)\n",ErrorLine(startinp,inp)));
                        while( *inp && !(*inp=='@') && !(inp[0]==LF && inp[1]==LF) )
                               inp++;
                        }
                else    {
                        inp += 2;

                        unless( ((int)atstuff[atindex+2]) & AF_ENUM )
                                enum_val = 1;   // Reset ENUM count

                        if( atstuff[atindex+3] )
                            indent = (int)atstuff[atindex+3];

                        unless( ((int)atstuff[atindex+2]) & AF_INDENTOK ) {
                                if( indenttextvalid == INDENT_OPTION ) {
                                    printf("Warning: Left margin text lost (%d)\n", ErrorLine(startinp,inp));
                                    printf("Warning: Text lost was \"%s\"\n", indenttext);
                                    }
                                else if( indenttextvalid == INDENT_VFORMAT )
                                    fprintf(outfile,"%s\n\n",indenttext);

                                for( j=0; j<LINE_WIDTH+2; j++)
                                        outbuf[j]=' ';
                                outbuf[j+1]=0;

                                indenttextvalid =0;
                        }

                        (*atstuff[atindex+1])(inp);
                        }
                }
            }
        else{
            enum_val = 1;       // Body text resets ENUM to base

            FormatParagraph(CollectParagraph(),"","");
            }
        }
    D(printf("End ProccessFile: outfile=%lx, inp=%lx\n",outfile,inp));
}
