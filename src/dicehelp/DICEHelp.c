/*
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */
/*
**      $Id: DICEHelp.c,v 30.0 1994/06/10 18:05:48 dice Exp $
**
**      Dice help system.  Searches an index file for a given
**      keyword.  Returns either filename information, or the actual
**      clip to the console, clipboard, or Rexx RESULT.
**
*/
/*
**
**      BUGS
**              REXXSTARTUP should exit if already resident.  (No, kill old).
**
**              Looses $30 (48) bytes of memory when started from Workbench.
**
**              CLI/WB invocation should use resident index file
**
**              If memory is less than XX bytes free, flush self on exit.
**
**              DICEHelp from Workbench accounts for "reasonable" font
**              sizes.  Very large font sizes, however, blow it up.
**
**      !!!TODO
**              If cursor is on "console.doc" in
**              "work:doc/doc/console.doc/OpenDevice",
**              return the console doc in full.
**
**              (Return # of lines for editor to open window?
**              Useful for small clips).
**
**              Make search faster -- check case insensitive first.
**              (Actually this is obsolete -- new tokenized index file
**              is needed to deal with automatic multiple references).
**
**              Match plural mismatch with error index of -3? (See error
**              weighting system).
**
**              Workbench won't read full files directly.
**
**      New HELP Items:
**              Put Carolyn's 68000 cards on line.  She says this is OK,
**                      with credit.
**
**              Help on error messages.
**
**              Help on C keywords
**
*/
#define D(x)    ;
#define X(x)    ;
#undef CLIPSUPPORT      /* Enable/disable clipboard.device code.
        Clipboard support is slightly broken right now, but could be fixed */

#include <clib/exec_protos.h>
#include <clib/alib_protos.h>
#include <rexx/rxslib.h>
#include <clib/rexxsyslib_protos.h>
#include <dos/dos.h>
#include <exec/memory.h>
#ifdef  CLIPSUPPORT
#include <devices/clipboard.h>
#endif
#include <DICEHelp_rev.h>

#include <strings.h>
#include <stdio.h>
#include <ctype.h>


/*******************************************************************************
**      Errors.
*/
#define OK_TEMPFILE      1      // Tempfile has been created
#define OK_FULLFILE      2      // Full path name has been returned
#define ERROR_NOTOOLTYPE 3      // No VIEWER tooltype
#define ERROR_NOTFOUND   4      // Can't find search string
#define ERROR_BADFORMAT  5      // Badly formatted line in index file
#define ERROR_BADCOMMAND 6      // Unrecognised AREXX command
#define ERROR_READFILE   7      // Error reading help file
#define ERROR_NOMEMORY   8      // Out of memory error
#define ERROR_WRITEFILE  9      // Error writing temporary file to T:   !!!
#define ERROR_NOCLIP     10     // Can't open clipboard.device          !!!
#define ERROR_NOCLIPSUP  11     // Don't even support clipboard
#define ERROR_INTERNAL   12     // Ohh, baby!
#define ERROR_MAXERRNO   12     // Distinguish error from pointer

struct ExtRexxMsgPort {
   struct MsgPort  rmp_Port;
   struct MinList  rmp_SearchNodes;
};

struct SearchNode {
        struct MinNode  sn_Node;        // Linkage
        char *          sn_FileName;    // Full path to load file from
        char *          sn_FileData;    // Loaded file, or NULL if unloaded
        unsigned long   sn_FileSize;    // Length of file, if known
};

#define unless(x)       if(!(x))
#define until(x)        while(!(x))

/*******************************************************************************
**      Prototypes
*/
void    *rexxOpen(void);
void     rexxClose(void);
void     rexxReply(struct RexxMsg *msg,int Result1,int Result2,char *string);

void    *clipOpen(void);
void     clipClose(void);
LONG     clipWrite(char *string);
LONG     clipLongWrite(LONG *ldata);
char    *getHelp(int outmode, char *searchline, int curpos);

char    *searchOpen(void);
void     searchClose(void);
char    *searchMe(char *strings,char *searchfor,char **filename);
void     convertFile(char *file,ULONG length);

void     debugPrintSN(void);

void     startOutput( char outmode );
void     sendOutput( char outmode, char *outstring, ULONG outlength );
char    *endOutput( char outmode );

char    *StringScan( char *instring );
char    *tailpath_p( char *searchline );
int      isCollectable(char c);
char    *ErrorText( int error, void * auxdata );

/*******************************************************************************
**      Globals
*/
struct RsxLib           *RexxSysBase    =0;     // Rexxsyslib.library base
struct MsgPort          *rexxPort       =0;     // Our Rexx port

struct IOClipReq        *clipIO =0;             // Clipboard Access
struct List              SearchNodes;           // List of struct SearchNode

char                     rexxName[]="DICEHELP"; // !! No string optimization
#define                 PORT_PRI        5       // Slighly above the rabble
const char  IdString[] = { VERSTAG };

#define OUTMODE_CLIP            'C'
#define OUTMODE_STDOUT          'S'
#define OUTMODE_FILE            'T'
#define OUTMODE_CRUTCHES        'Z'

#define COMMAND_QUIT            'Q'


#define TEMPFILE_NAME           "t:DICEHelp.temp"       // hard coded, too
#define TEMPFILE_LEN            15      // DICE has no strlen() optimization


disable_break()
{
        return(0);
}

/*******************************************************************************
**
**      Workbench support
*/
#include "intuition/intuition.h"
#include <clib/intuition_protos.h>
#include <clib/icon_protos.h>
#include <workbench/workbench.h>
#include <workbench/startup.h>

#define STRING_MAX 90                           // !! Fixed size
char    buf_string[STRING_MAX];                 // !! Fixed size
char    inp_string[STRING_MAX];                 // !! Fixed size
char    commandline[STRING_MAX];                // !! Fixed size

#define GX      10
#define GY      25
#define GWIDTH  400
#define GHEIGHT 30

struct StringInfo si =
{
inp_string,     // String
NULL,           // Undo
0,              // Buffer position
STRING_MAX,     // Count
0,              // Disp Pos
0,              // Undo pos
0,              // Num  Chars
0,              // DispCount
0,0,            // CLeft,CTop
NULL,           // Layer
0,              // LongInt
NULL            // Alternate keymap
};

struct Gadget gt =
{
NULL,
GX,GY,
GWIDTH,GHEIGHT,
GADGHBOX,
RELVERIFY,
STRGADGET,
NULL,
NULL,
NULL,
0,
&si,
0,
NULL
};

struct NewWindow nw =
{
((640-GWIDTH+20)/2),75,         /* Left, Top */
GWIDTH+20,GHEIGHT+20,
-1,-1,
IDCMP_CLOSEWINDOW|IDCMP_GADGETUP,
WFLG_DRAGBAR|WFLG_DEPTHGADGET|WFLG_CLOSEGADGET|
WFLG_SMART_REFRESH|WFLG_ACTIVATE,
&gt,
NULL,
"DICEHelp: Please enter a word:",
NULL,NULL,
0,0,
0,0,
WBENCHSCREEN
};


int wbmain(struct WBStartup *wbs)
{
struct Window           * win;
struct IntuiMessage     * msg=0;
char                    * filename=0;
char                    * temp=0;

int                       rc=0;

    if ( win=OpenWindow(&nw) ) {
        SetAPen(win->RPort,2);
        if ((win->Height-win->BorderBottom-1) > 0) {
            RectFill(win->RPort,
                 win->BorderLeft,
                 win->BorderTop,
                 win->Width-win->BorderRight-1,
                 win->Height-win->BorderBottom-1,
                 );
            }
        ActivateGadget(&gt,win,NULL);


        // Collect VIEWER tooltype into commandline array, else
        // set RC=ERROR_NOTOOLTYPE
        {
        struct DiskObject       * dob;  // DICEHelp icon (for Tooltypes)
        BPTR                      saveLock;
        saveLock = CurrentDir((BPTR)wbs->sm_ArgList[0].wa_Lock);
        if (dob = GetDiskObject(wbs->sm_ArgList[0].wa_Name)) {
            if (temp=FindToolType(dob->do_ToolTypes,"VIEWER") ) {
                strncpy(commandline,temp,STRING_MAX-TEMPFILE_LEN-2);
                strncat(commandline," ",STRING_MAX);
                strncat(commandline,TEMPFILE_NAME,STRING_MAX);
            } else {
                SetWindowTitles(win,"No \"VIEWER\" ToolType in icon!",(void *)-1);
                rc= ERROR_NOTOOLTYPE;
                // Window stays open for message, but user can't do anything.
            }
            FreeDiskObject(dob);
        }
        CurrentDir(saveLock);
        }


        unless( searchOpen() ) {
            SetWindowTitles(win,"Can't open file \"s:DICEHelp.index\"!",(void *)-1);
            rc= ERROR_READFILE;
            // Window stays open for message, but user can't do anything.
            }

        // Loop until we process a message (Any message causes us to exit
        int looping=1;
        while(looping && Wait(1L << win->UserPort->mp_SigBit))
        {
            while(msg = (struct IntuiMessage *) GetMsg(win->UserPort) )
            {
                switch( msg->Class )
                {
                    case CLOSEWINDOW:
                        looping=0;
                        break;
                    case GADGETUP:
                        if (!rc) {
                            char *msg;
                            SetWindowTitles(win, commandline,(void *)-1);
                            msg = getHelp( OUTMODE_FILE, inp_string, 0 );
                            if (msg == NULL)
                               msg = "E Internal error!";
                            if (msg[0] != 'T') {
                                DisplayBeep(0L);
                                SetWindowTitles(win, msg+2, (void *)-1);
                                looping=1;      // Stay, show message
                                ActivateGadget(&gt,win,NULL);   // Re-juice gadget
                            } else {
                                //
                                // !!!! :TODO:
                                // Under 2.0: fh=Open("CON:0/0/200/200/DICEHelp Error/AUTO/CLOSE/WAIT",MODE_NEWFILE);
                                // Under 1.3: fh=Open("NIL:",MODE_NEWFILE);
                                //
                                // Note: Execute does not return a sane code under 2.0!
                                //
                                if( Execute(commandline,NULL,Open("NIL:",MODE_NEWFILE)) )
                                    looping=0;
                                else
                                    DisplayBeep(0L);
                            }
                        }
                        break;
                    default:
                }
                ReplyMsg( (struct Message *)msg );
            }
        }
        if (win)        CloseWindow(win);
        searchClose();

    }   // OpenWindow
}


/*******************************************************************************
**      CLI Startup
*/
main(int argc,char * argv[])
{
struct RexxMsg *msg;

char    *filename=0;
char    *refline =0;

char    *temp=0;
char     outmode=0;
char    *rexxresult=0;
int      returncode=0;

    onbreak( disable_break );

    if (argc != 2 || ( argc == 2  &&  *argv[1]=='?' ) ) {
        printf("DICEhelp <Searchstring> [REXXSTARTUP]\n");
        printf("; Return help for searchstring.  See MakeIndex & s:DICEHelp.index\n");
        exit(5);
        }

    unless( searchOpen() ) {
        puts( ErrorText( ERROR_READFILE, "s:DICEHelp.index" ));
        exit(10);       // !!! Modify to return REXX result
        }

    if( strcmp(argv[1],"REXXSTARTUP") )
        {
                temp=getHelp( OUTMODE_STDOUT, argv[1], 0 );
                unless( temp[0] == 'S')
                        puts( temp+2 ); // Error
        }
    else{       // REXXSTARTUP
        unless( rexxOpen() ) {  // !!! Error check
            printf("Error: Can't open \"rexxsyslib.library\"\n");
            }
        #ifdef  CLIPSUPPORT
        unless( clipOpen() ) {
            printf("Error: Can't open \"clipboard.device\"\n");
            }
        #endif

        D(printf("REXX port open for traffic!\n"));

        until( outmode == 'Q' || SIGBREAKF_CTRL_C &
               Wait(1L << rexxPort->mp_SigBit | SIGBREAKF_CTRL_C) ) {
            while( msg=(struct RexxMsg *)GetMsg((struct MsgPort *)rexxPort) ) {

                temp = msg->rm_Args[0];
                D(printf("Message %lx, rm_Args[0] @ %lx:\n%s\n",msg,temp,temp));

                rexxresult=0;
                returncode = ERROR_BADCOMMAND;
                if( (3 < strlen(temp)) && (temp[1]==' ') )
                    {
                    outmode = temp[0];
                        printf("Outmode = %c\n", outmode);
                    if ( outmode == 'Q' )
                        rexxresult = "QUITTING";
                    else if ( outmode == 'Z' )
                        rexxresult = getHelp( OUTMODE_FILE, &temp[2], -1 );
                    else
                        rexxresult = getHelp( OUTMODE_FILE, &temp[2], 0 );
                    }
                if( rexxresult )
                    returncode = 0;
                rexxReply( msg,returncode,0,rexxresult );
                }  // While messages
            }      // Until exit
        #ifdef  CLIPSUPPORT
        clipClose();
        #endif
        rexxClose();
        }

        D(printf("Exiting...\n"));
        searchClose();
        return(returncode);
}

#ifdef  NOTDEF
/*******************************************************************************
**      Debug functions
*/
void debugPrintSN(void)
{
struct SearchNode *nd=(struct SearchNode *)&SearchNodes;

        while((nd=nd->sn_Node.mln_Succ) && (nd->sn_Node.mln_Succ)) {
                printf("SearchNode at %lx\n",nd);
                printf("sn_FileName     =%s\n",nd->sn_FileName);
                printf("sn_FileSize     =%ld\n",nd->sn_FileSize);
                printf("sn_FileData     =%lx\n\n",nd->sn_FileData);
                }
}
#endif

/*******************************************************************************
**      Index file functions
*/
#define FILE_PAD        2       // Stick two NULLs at the end of the file

char *searchOpen(void)
{
struct  SearchNode *sn;
ULONG   fh=0;
ULONG   length;
char    *body=0;
char    *filename="s:DICEHelp.index";

    NewList(&SearchNodes);

    if (fh=Open(filename,MODE_OLDFILE)) {
        Seek(fh,0,OFFSET_END);
        if (length=Seek(fh,0,OFFSET_BEGINNING)) {
            if (sn=AllocMem(sizeof(struct SearchNode),0)) {
                sn->sn_FileName=filename;
                sn->sn_FileData=0;
                sn->sn_FileSize=length+FILE_PAD;
                AddTail(&SearchNodes,(struct Node *)sn);
                if (body=AllocMem(length+FILE_PAD,0)) {
                    if (length==Read(fh,body,length)) {
                        sn->sn_FileData=body;
                        convertFile(body,length);       // LF->NULL & Terminate
                        } else {
                        FreeMem(body,sn->sn_FileSize);
                        }
                    }
                }
            }
        }

    if(fh)
        Close(fh);

    return(body);
}

void searchClose(void)
{
struct SearchNode *sn;
struct SearchNode *nn;

    nn=(struct SearchNode *)&SearchNodes;
    nn=(struct SearchNode *)nn->sn_Node.mln_Succ;

    while( nn && nn->sn_Node.mln_Succ) {
        sn=nn;
        nn=(struct SearchNode *)nn->sn_Node.mln_Succ;

        D(printf("Freeing searchnode %s. %d bytes\n",sn->sn_FileName,
              sn->sn_FileSize));

        if( sn->sn_FileData )
            FreeMem(sn->sn_FileData,sn->sn_FileSize);
        FreeMem(sn,sizeof(struct SearchNode));
        }
}

/*******************************************************************************
**      Line parsing functions
**
**      Some editors have lousy "get word" functions.  Given a cursor
**      position and a line, we can clip out words too.
**
**      ??      If the cursor is ON punctuation, return it      ??
**
**      The result is NULL or a pointer into the string.  The end of
**      the parsed word is NULL terminated, ** MODIFYING ** the input.
**
**      Input lines are formatted with XX equal to the cursor position:
**              Z XX Clipped line
**      String is at least 4 characters long.
**
*/
int isCollectable(char c)
{
        if( isalnum(c) || c==':' || c=='/' || c=='_' || c=='.' || c=='\\' )
                return( 1 );
        return( 0 );
}

char * StringScan( char *instring )
{
char    *tail;
char    *result;
unsigned long curpos;
unsigned long curtemp;  //      Wordy()

        curpos = strtol( instring, &tail, 10 );
        D(printf("|Input pos=%ld, Line=%s\n", curpos, tail));
        if ( !curpos || (*tail != ' ') )        // !!! longer than string?
                return( NULL );

        curtemp = curpos;
        while( curtemp && isCollectable( tail[curtemp-1] ) )
                curtemp--;
        result = &tail[curtemp];
        D(printf("|Start pos=%ld, Tail=%s\n", curpos, result));

        curtemp = curpos;
        while( isCollectable( tail[curtemp] ) )
                curtemp++;      // Stops at trailing NULL, at worst
        tail[curtemp]=0;
        D(printf("|Final pos=%ld, Result=\"%s\"\n", curtemp, result) );

        if( curtemp == curpos ) // If no characters were valid
                result = 0;
        return( result );
}



/*******************************************************************************
**      Output functions. Start, send and end output based on outmode.
*/
ULONG   outfh;                  // For xxxOut functions

void startOutput(char outmode)
{
        switch(outmode) {
                case OUTMODE_CLIP:      // !!! IFF FTXT Length
                    outfh=1;
                    break;
                case OUTMODE_STDOUT:
                    outfh=1;            // Continuation flag, basically
                    break;
                case OUTMODE_FILE:
                case OUTMODE_CRUTCHES:
                    outfh=Open( TEMPFILE_NAME, MODE_NEWFILE );
                    break;
                default:
        }
}


void sendOutput(char outmode, char *outstring,ULONG outlength)
{
char *returnstring=0;

        switch(outmode) {
                case OUTMODE_CLIP:
                    #ifdef      CLIPSUPPORT
                    clipWrite(outstring);
                    #endif
                    break;
                case OUTMODE_STDOUT:
                    while( outfh && outlength && (outlength=nextLine(outstring)) ) {
                        if( outlength != fwrite(outstring,1,outlength,stdout) )
                                outlength=0;       // Terminate early
                        if( SIGBREAKF_CTRL_C & SetSignal( 0,SIGBREAKF_CTRL_C) ) {
                                outfh = 0;
                                outlength=0;
                                }
                        outstring += outlength;
                    }
                    break;
                case OUTMODE_FILE:
                case OUTMODE_CRUTCHES:
                    if( outfh )
                        if( outlength != Write( outfh,outstring,outlength )) {
                            Close( outfh );     // Prevent multiple "failed"
                            outfh = NULL;       // requesters in case of error
                            }
                    break;
                default:
        }
}


//  !! This strings should be moved to ErrorText for localization
char *endOutput( char outmode )
{
char *returnstring="E Internal error!";

        switch(outmode) {
                case OUTMODE_CLIP:
                    returnstring="E Clipboard not supported";
                    break;
                case OUTMODE_STDOUT:
                    returnstring="S stdout";
                    break;
                case OUTMODE_FILE:
                case OUTMODE_CRUTCHES:
                    if( outfh )
                        Close( outfh );
                    outfh = NULL;
                    returnstring="T t:DICEHelp.temp";
                    break;
                default:
        }
    return( returnstring );
}


/*******************************************************************************
**      Arexx functions
*/
void * rexxOpen(void)
{
    RexxSysBase = (struct RxsLib *)OpenLibrary("rexxsyslib.library",0L);

    if (rexxPort == NULL) {
        Forbid();
        if (!( rexxPort = FindPort(rexxName ) ))
                rexxPort = CreatePort(rexxName, PORT_PRI);
        Permit();
        }

    return( RexxSysBase );
}

void rexxClose(void)
{
    if (RexxSysBase) {
        CloseLibrary(RexxSysBase);
        RexxSysBase=NULL;
        }

    if (rexxPort)
        DeletePort(rexxPort);
}

void rexxReply(struct RexxMsg *msg,int Result1,int Result2,char *string)
{
    msg->rm_Result1 = Result1;
    msg->rm_Result2 = Result2;

    if (Result1 == 0 && (msg->rm_Action & (1L << RXFB_RESULT))) {
        D(printf("Rexx Result1=%lx  Result2=%s\n",0,string));
        if (string && RexxSysBase)
            msg->rm_Result2 =
                (LONG)CreateArgstring(string, (LONG)strlen(string)) ;
        else
            msg->rm_Result2 = NULL;
        }
        D(printf("Rexx Result1=%lx  Result2 =%lx\n",msg->rm_Result1,msg->rm_Result2));

    ReplyMsg((struct Message *)msg);
}

#ifdef  CLIPSUPPORT
/******************************************************************************
**      Clipboard functions
*/
void * clipOpen()
{
struct MsgPort *port;

    if  (port = CreatePort(0L, 0L))
        {
        if  (clipIO = (struct IOClipReq *)
            CreateExtIO (port, sizeof (struct IOClipReq)) )
            {
            unless( OpenDevice ("clipboard.device", 0, clipIO, 0) )
                return(clipIO);

            DeleteExtIO(clipIO);
            clipIO = NULL;
            }
        DeletePort(port);
        }

    return(0);
}

void clipClose()
{
        if( clipIO ) {
            CloseDevice(clipIO);
            DeletePort(clipIO->io_Message.mn_ReplyPort);
            DeleteExtIO(clipIO);
            }
}

LONG clipWrite(char * string)
{
    LONG length, slen = strlen (string);
    BOOL odd = (slen & 1);      /* pad byte flag */
    LONG error = 0L;

    /* Reset the clip id */
    clipIO->io_ClipID = 0;

    length = (odd) ? slen + 1 : slen;
    clipIO->io_Offset = 0;
    error = clipLongWrite ((LONG *) "FORM");/* "FORM" */

    length += 12;
    error = clipLongWrite (&length);    /* #  */
    error = clipLongWrite ((LONG *) "FTXT");/* "FTXT" for example */
    error = clipLongWrite ((LONG *) "CHRS");/* "CHRS" for example */
    error = clipLongWrite (&slen);              /* #  (length of string) */

    clipIO->io_Command = CMD_WRITE;
    clipIO->io_Error = 0;
    clipIO->io_Data = (char *) string;
    clipIO->io_Length = slen;           /* length of string */
    error = (LONG) DoIO (clipIO);       /* text string */

    /* Write the pad byte */
    if (odd)
    {
        clipIO->io_Command = CMD_WRITE;
        clipIO->io_Error = 0;
        clipIO->io_Data = NULL;
        clipIO->io_Length = 1;
        error = (LONG) DoIO (clipIO);
    }

    /* Indicate that we're done writing to the clipboard */
    clipIO->io_Command = CMD_UPDATE;
    clipIO->io_Error = 0;
    error = (LONG) DoIO (clipIO);

    return (error);
}

LONG clipLongWrite (LONG * ldata)
{
    clipIO->io_Command = CMD_WRITE;
    clipIO->io_Error = 0;
    clipIO->io_Data = (char *) ldata;
    clipIO->io_Length = 4L;
    return ( (LONG) DoIO (clipIO) );
}
#endif

/*******************************************************************************
**      Clip & Snip functions
*/
/*
**      searchline      - Search word, or raw line from editor.
**      curpos          - Cursor position on line, or zero for no parsing.
**      outmode         - Where to stuff it.
**
**      returns:
**              Filename clip is stored in, or name of entire file
**              Multiple references
**              Name of word searched for
**
**      errors:
**              Clip not found
**              Can't open file
**              Bad format
*/
char *getHelp(int outmode, char *searchline, int curpos)
{
struct SearchNode *sn=(struct SearchNode *)&SearchNodes; // Search
int                rc=ERROR_NOTFOUND;                    // Error code

                // searchline   // Passed: parameter, raw unprocessed line
char    *          searchword;  // Parsed: Word to search for, ie "OpenDevice"

char              *refline;     // Searched: [searchname] [offset] [lines]
char              *filename=0;  // Searched: Path name for refline

unsigned long   fh=0;
unsigned long   startpos;       // Calculated from refline
unsigned long   length;         // Calculated from refline

char    *       temp=0;         // Have bits, will travel.
char    *       refline2;       // Multi-reference buffer
char    *       filename2;      // Multi-reference buffer

char *  returnstring=0; // Indicates where to look for result.
                        // "CLIP"
                        // "STDOUT"
                        // "full:path"
                        // "t:DICEHelp.temp"

    /*
    **  Parse it.  Use Z mode if a cursor position is given.
    */
    if( curpos ) {
        searchline = StringScan( searchline );  // Null terminates searchline-
        }                                       // returns interesting part
    if( searchline == NULL || strlen( searchline ) < 2 )
        return( ErrorText( ERROR_NOTFOUND, "" ));

    /*
    **  Clip off "device:filename/word" to just "word"
    */
    searchword = tailpath_p( searchline );
    D(printf("\nWord=<%s> Line=<%s>\n",searchword, searchline ));
    if( !*searchword )          // Help item contained a trailing :
        return( ErrorText( ERROR_NOTFOUND, searchline ));

    /*
    **  Search all lists of words.  Reject refline's that don't match
    **  the expected file name.
    */
    refline = 0;
    while( !refline && (sn=(struct SearchNode *)sn->sn_Node.mln_Succ) &&
           (sn->sn_Node.mln_Succ)) {
        if( sn->sn_FileData ) {
            temp = sn->sn_FileData;
            while( temp = searchMe(temp, searchword, &filename) ) {
                D(printf("Search-Filename=%s res=%s cmp=%d\n",
                          filename, temp, strpathcmp( filename, searchline )));
                if( !strpathcmp( filename, searchline ) ) {
                    refline = temp;
                    break;
                }
                temp += strlen( temp )+1;
            }
        }
    }

    /*
    **  Basic parsing of refernce line.  Checks for bad format, gets length
    **  and offset.
    */
    unless( refline )
        return( ErrorText( ERROR_NOTFOUND, searchword ));
    unless( temp=strchr(refline,0x09) ) // Search for tab
        return( ErrorText( ERROR_BADFORMAT, 0 ));
    startpos=strtol(temp,&temp,16);
    length=strtol(temp,&temp,16);
    D(printf("Reading %s, startpos %lx, length %lx\nrefline=%s searchword=%s\n",
                filename,startpos,length,refline,searchword));

    /*
    **  If the index file specifies the *entire* file, and the mode is
    **  tempfile, just return the full path name of the file.  This saves
    **  reading the file twice, as well as memory.  Note: Multiple references
    **  won't be shown; life sucks.
    */
    if ( outmode==OUTMODE_CRUTCHES && length==0 )
        return( ErrorText( OK_FULLFILE, filename+1 ));

    if (fh=Open(filename+1, MODE_OLDFILE)) {
        Seek(fh, startpos, OFFSET_BEGINNING);
        unless( length ) {              // Zero length means read *all*
                Seek(fh, 0, OFFSET_END);
                length=Seek(fh, 0, OFFSET_BEGINNING);
                }
        unless( length )
                return( ErrorText( ERROR_READFILE, filename+1 ) );
        if (temp=AllocMem(length+1,0)) {
            if (length==Read(fh, temp, length)) {
                temp[length]=0;

                startOutput( outmode );
                sendOutput( outmode, temp, length );
                }
            FreeMem(temp,length+1);
            }
        else{
            return( ErrorText( ERROR_NOMEMORY, (void *)length ) );
            }
        Close(fh);
        }
    else{
        return( ErrorText( ERROR_READFILE, filename+1 ) );
        }

    /*
    **  Multiple references section.  Yes, I hacked it in.
    */
    sprintf(buf_string,"# %s\\%s\n", filename+1, refline);   /* First line */
    sendOutput( outmode, buf_string, strlen( buf_string ) );

    refline2  = refline;
    filename2 = filename;
    while( refline2 )
    {
        X(printf("Testing for more %s entries (%s)\n",searchword,refline2));

        refline2+=strlen( refline2 )+1;
        refline2 =searchMe(refline2,searchword,&temp);
        if( temp )
                filename2 = temp;       // If new filename, update

        if( refline2 )
                {
                X(printf("Got it: File %s refline=%lx/%s\n",
                filename2,refline2,refline2));

                strcpy (buf_string, "# ");
                strncat(buf_string, filename2+1,        STRING_MAX);
                strncat(buf_string, "\\",               STRING_MAX);
                strncat(buf_string, refline2,           STRING_MAX);
                strncat(buf_string, "\n",               STRING_MAX); // !!! ANSI Misfeature
                sendOutput( outmode, buf_string, strlen( buf_string ) );
                }
    }

    return( endOutput( outmode ) );
}

/*
**      Reverse case insensitive string compare specialized for paths
**      Compare:
**              Help Request    =       foo.doc/AllocMem
**              Full path       =       work:doc/foo.doc
**      If path part matches full path, return 0, else return 1.
*/
int strpathcmp( path, candidate)
const char *path;
const char *candidate;
{
const char *pathe;
const char *cande;
typedef unsigned char   ubyte;

        cande = tailpath_p( candidate );
        if( cande == candidate )
                return( 0 );            // Trivial case: No path to compare

        cande--;                        // foo.doc/AllocMem
        pathe = path+strlen(path);              //        ^

        while( (cande > candidate) && (pathe > path) ) {
                pathe--;
                cande--;
                if (tolower(*(ubyte *)pathe) != tolower(*(ubyte *)cande))
                        return( 1 );
                }
        return( 0 );
}


/*
**      Return just filename part of complete or incomplete path
*/
char *tailpath_p( char *pathname )
{
char *  temp;

    if( temp=strrchr( pathname, '\\' ) )
        return( ++temp );
    else if( temp=strrchr( pathname, ':' ) )
        return( ++temp );
    else
        return( pathname );
}


char *ErrorText( int error, void * auxdata )
{
char    * localstring;

    switch( error ) {
        case OK_TEMPFILE:
                localstring = "T %s";
                break;
        case OK_FULLFILE:
                localstring = "F %s";
                break;
        case ERROR_NOTOOLTYPE:
                localstring = "E No \"VIEWER\" tooltype in icon!";
                break;
        case ERROR_NOTFOUND:
                localstring = "E Item \"%s\" not found";
                break;
        case ERROR_BADFORMAT:
                localstring = "E Corrupt line in s:DICEHelp.index";
                break;
        case ERROR_BADCOMMAND:  // Handled as a rexx error number
        case ERROR_READFILE:
                localstring = "E Error reading \"%s\"";
                break;
        case ERROR_NOMEMORY:    // Out of memory error          !!!
                localstring = "E Out of memory - %d bytes needed";
                break;
        case ERROR_WRITEFILE:   // Error writing temporary file to T: !!!
                localstring = "E Error writing \"%s\"";
                break;
        default:
                localstring = "E Internal error!";
        }

    sprintf( buf_string, localstring, auxdata );  // !!! Length limit
    return( buf_string );
}

