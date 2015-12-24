/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */
/*
**      $Id: MakeIndex.c,v 30.5 1994/06/13 18:43:09 dillon Exp dice $
**
**      Generate index file for use by DICEHelp.  Modified from the DME
**      program MakeIndex.
**
**		BUGS: Leaves LF at the end of last entry of some
**		document files (like exec.doc).
**
**              !!! TODO: If clip is within MAGIC_NUMBER of filesize,
**              just return the entire file.
**
**              !!! TODO: OpenDevice/CloseDevice/OpenLibrary/CMD_READ, etc
**              generally don't end up unique.  What to do, to do, to do.
**
**              !!! TODO: Scan for TAG_XXXXX
**
**              !!! TODO: Scan .fd files better
**
**              !!! TODO: On include files, scan backwards to nuke
**              include file header.
**
**              !!! TODO: Bug report on layers line
**
*/
#define D(x)    ;

/*
 *  MakeIndex output <pattern>
 *
 *  Given one or more autodoc or .H files (e.g. intuition.doc) or include
 *  files (e.g. exec/types.h), this program appends DICEHelp refence file
 *  <refsfile> appropriate lines.
 *
 *  MakeIndex determines the file type from the extension (.h for header
 *  files, otherwise assumed to be a doc file).
 */

#include <stdio.h>
#include <string.h>
#include <MakeIndex_Rev.h>

#define unless(x)       if(!(x))

char *SetSName(char *, char *);
char *TailPath(char *path);
char *strcpyc(char *d, char *s, char c);
int   globalerrors=0;

const char  IdString[] = { VERSTAG };

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
    if (ac == 1) {
	puts("MakeIndex outfile <pattern>");
	exit(1);
    }
    fo = fopen(av[1], "a");
    if (!fo) {
	printf("Error: Unable to open %s for append\n", av[1]);
	globalerrors++;
	exit(1);
    }
    for (i = 2; i < ac; ++i) {
	char *file = av[i];
	short len = strlen(file);
	short doth = 0;

	if (len >= 2 && (file[len-1] == 'h' || file[len-1] == 'H') && file[len-2] == '.')
	    doth = 1;

	if (len >= 2 && (file[len-1] == 'i' || file[len-1] == 'I') && file[len-2] == '.')
	    doth = 2;

	fi = fopen(file, "r");
	if (fi) {
	    if (doth == 1) {
		printf("Scanning .H  file: %s\n", file);
		scanhfile(fi, fo, file);
	    } else if (doth == 2) {
		printf("Recording name of .I file: %s\n", file);
		scanifile(fi, fo, file);
	    } else {
		printf("Scanning DOC file: %s\n", file);
		scandocfile(fi, fo, file);
	    }
	    fclose(fi);
	} else {
	    printf("Error: Unable to read %s\n", file);
   	    globalerrors++;
	}
    }
    printf("Operation complete.  %d errors\n",globalerrors);
    return(0);
}


/*
 *  Find the headers for each function entry and generate a ref entry.
 */

scandocfile(fi, fo, filename)
FILE *fi;
FILE *fo;
char *filename;
{
    char buf[256];
    long pos = 0;
    long pendpos = 0;   // Temp for eliminating ^L from resulting file
    long pending =0;	// For calculating # bytes in entry
    short lastLineFF = 0;
    char pendflag=0;    		// If flush of line pending
#define	MSIZE	128
    static char pendline[MSIZE];	// Function names copied to here
    char *temp1;
    char *temp2;
    long lin=1;

    // Start of new file
    fprintf(fo, "~%s\n", filename);
    fprintf(fo, "%s	%lx	%lx\n",TailPath(filename),0,0);

    while (fgets(buf, 256, fi)) {
	short len = strlen(buf) - 1;
	char *ptr = buf + len;
	char *bas = buf;
	char *header, *tail;

	buf[len] = 0;
	// Scan backwards to start, or first space or <tab>
	while (ptr != buf && ptr[-1] != ' ' && ptr[-1] != 9)
	    --ptr;
	// Skip all ^L
	while (bas < ptr && *bas == 12)
	    ++bas;

	// Process either:
	//      A>      dice/name                       dice/name
	//
	//      B>      <FF>
	//              anysingleword/withaslash
	//
	//      C>      <FF>anysingleword/withaslash
	//
	//
	if (ptr != bas && *ptr && strncmp(bas, ptr, strlen(ptr)) == 0) {

            D(printf("ptr=%s\nbas=%s\nRR:lastLineFF=%d\n",ptr,bas,lastLineFF));

            pendpos = pos;      // Snapshot before skipping FF
	    if (buf[0] == 12) {
		++pos;
		buf[0] = 0;
	    }
	    header = ptr;
	    for (ptr = buf + len; ptr != buf && IsAlphaNum(ptr[-1]); --ptr)
	        ;
	    tail = ptr;

            if (pendflag)
            	{
            	D(printf("%s pending\n",pendline));

                // Process all comma separated function names.
                temp1 = pendline;
                temp2 = pendline;
                while( temp2 && *temp1 )
                        {
                        temp2=strchr( temp1, ',' );
                        if( temp2 )
                            *temp2=0;

                        fprintf(fo, "%s	%lx	%lx\n",temp1,pending,pendpos-pending);

			temp1=temp2+1;	// Skip ,
	                }
                pendflag=0;
	     	}

            if(!( *tail ))
		{
                printf("Error: Bad function name! (line %d)\n",lin);
		globalerrors++;
		}
            else{
                pending  =pos;
                strncpy( pendline, tail, MSIZE-1 );
                pendline[MSIZE-1]=0;
                pendflag =1;
                }

	} else if (ptr == bas && *ptr && strchr(ptr,'/') && (lastLineFF || *ptr==12 )) {
            D(printf("ptr=%s\nbas=%s\nLL:lastLineFF=%d\n",ptr,bas,lastLineFF));

            pendpos = pos;      // Snapshot before skipping FF
	    if (buf[0] == 12) {
		++pos;
		buf[0] = 0;
	    }
	    for (ptr = buf + len; ptr != buf && IsAlphaNum(ptr[-1]); --ptr)
	        ;
            tail = ptr;

            if (pendflag)
            	{
            	D(printf("LL %s pending\n",pendline));

                // Process all comma separated function names.
                temp1 = pendline;
                temp2 = pendline;
                while( temp2 && *temp1 )
                        {
                        temp2=strchr( temp1, ',' );
                        if( temp2 )
                            *temp2=0;

                        fprintf(fo, "%s	%lx	%lx\n",temp1,pending,pendpos-pending);

			temp1=temp2+1;	// Skip ,
	                }
                pendflag=0;
	     	}

            if(!( *tail ))
		{
                printf("Error: Bad function name! (line %d)\n",lin);
		globalerrors++;
		}
            else{
                pending  =pos;
                strncpy( pendline, tail, MSIZE-1 );
                pendline[MSIZE-1]=0;
                pendflag =1;
                }

	}
	if (buf[0] == ('l'&0x1F))
	    lastLineFF = 1;
	else
	    lastLineFF = 0;
	pos = ftell(fi);
	lin++;
    }

        if (pendflag)
	    {
            D(printf("End: %s pending. %lx:%lx/%lx\n",pendline,pending,pendpos,ftell(fi)));
	    
            // Process all comma separated function names.
            temp1 = pendline;
            temp2 = pendline;
            while( temp2 && *temp1 )
                    {
                    temp2=strchr( temp1, ',' );
                    if( temp2 )
                        *temp2=0;

                    fprintf(fo, "%s\t%lx\t%lx\n",temp1,pending,ftell(fi)-pending);

		    temp1=temp2+1;	// Skip ,
	            }
            pendflag=0;
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
 *      Just record the name of .i files encountered.  Don't try to locate
 *      structures (yet).
 *
 */
scanifile(fi, fo, filename)
FILE *fi;
FILE *fo;
char *filename;
{
    // Start of new file
    fprintf(fo, "~%s\n", filename);
    fprintf(fo, "%s	%lx	%lx\n",TailPath(filename),0,0);
}


/*
 *  Find each structure definition (stupid search, assume struct on left
 *  hand side) then generate dme.refs entry from the end point of the
 *  previous structure to the beginning of the next structure.	That is,
 *  the reference refers to the structure and all fields before and after
 *  it until the next structure (before and after).
 */

scanhfile(fi, fo, filename)
FILE *fi;
FILE *fo;
char *filename;
{
    static char buf[256];
    static char sname[128];
    static char lname[128];
    long lin  = 1;
    long lin1;
    long lin2 = 1;
    long pos  = 0;
    long pos1 = 0;
    long pos2 = 0;
    short snameisvalid = 0;
    short newsname = 0;
    short includehack =0;

    // Start of new file
    fprintf(fo, "~%s\n", filename);
    fprintf(fo, "%s	%lx	%lx\n",TailPath(filename),0,0);

    while (fgets(buf, 256, fi)) {
	char *ptr = buf;

        /* Attempt to trim header from first structure.  Assume a #include
        ** is the end of any previous structure definition.
        */
        if ( ptr = strstr(buf,"#include") )
            includehack = 1;                // We're tracking one!
        else
            if (includehack && (ptr=strstr(buf,"#endif")) )
                pos2 = pos + strlen(buf);
            else
                includehack = 0;

	if ((ptr = strstr(buf, "struct")) || (ptr = strstr(buf, "union"))) {
	    if (ptr[0] == 's')
		++ptr;
	    ptr += 5;

	    ptr = SetSName(lname, ptr);

	    /*
	     *	search for '{'
	     */

	    {
		while (*ptr == ' ' || *ptr == '\t' || *ptr == '\n' || *ptr == 12)
		    ++ptr;
		if (*ptr == 0) {
		    short c = ' ';
		    long savpos = ftell(fi);
		    while (c == ' ' || c == '\t' || c == '\n' || c == 12)
			c = getc(fi);
		    ptr[0] = c;
		    ptr[1] = 0;
		    fseek(fi, savpos, 0);
		}
	    }

	    if (*ptr == '{' && lname[0]) {
		if (snameisvalid)
		    fprintf(fo, "%s	%lx	%lx\n", sname, pos1, pos2-pos1);
		strcpy(sname, lname);
		snameisvalid = 0;
		newsname = 1;
		pos1 = pos2;
		lin1 = lin2;
	    }
	}
	pos = ftell(fi);
	++lin;

	if (strstr(buf, "}")) {
	    pos2 = pos;
	    lin2 = lin;
	    snameisvalid = newsname;
	}
    }
    if (snameisvalid)
	fprintf(fo, "%s	%lx	%lx\n", sname, pos1, pos2-pos1);
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
