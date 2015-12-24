/*

    This program is an example that uses the capabilities of Multiple
    Stream C.  It uses the 'nextinput' function and reads its asynchronous
    events rather than using signals.

    The program simulates a one channel Citizens Band Radio (CB).  When
    someone logs-on, they are asked for a handle to identify themselves.
    Then everyone signed-on the CB is told that a new user is on.  When
    someone types a message, it is broadcast to everyone on the CB.  Each
    message that is sent is proceeded with the users identification.  There
    are several commands; each begins with a slash (/).  For a list of the
    available commands, type "/HELP".  To log-off, you type the command
    "/BYE".  If the processor is going to go down for maintenance, all the
    users are asked to relogon.

    This is basically the same program as in the Multi Stream C Document,
    modified by Dan Leichtenschlag to increase efficiency and to
    demonstrate GEnie specific routines used by Third party authors writing
    Multi stream C applications.  Most obvious is the table of users and
    using fileno(fp) to index into the table rather than searching for a
    file pointer each time.  Other modifications recommended for GEnie
    authors are setting the timeout for streams, and redirecting stdout and
    stderr to files rather than the bit bucket (default for stdout in Multi
    Stream mode).  It also uses the preferred style of having a simple
    processing loop and calling functions for each event rather than
    stuffing all the code into the switch statement like the example.

    There are 3 routines provided by GEnie that must be called so GEnie can
    keep track of who is in the game.  They are:

    getuser(fp) - called for each user that is allowed in the game.
    touser(fp)  - called to send user back to GEnie.
    delrt()     - called just before game terminates, Game is 'down'.

    The getuser and touser routines need the file pointer for that user
    passed so the user can be identified.  getuser returns a character
    pointer to the users U#.  This should be saved by the caller and
    used to uniquely identify users for a database key id needed.

    To Compile: cc +v l=game.lib o=cb

    GAME.LIB is a library provided to GEnie Authors.
*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ioctl.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <signal.h>
#include <nextin.h>
#include <mk3c.h>

#define BUFFER_SIZE 512
#define MAX_USERS 100			  /* Max number of users on CB */
#define HANDLE_LEN 16+1 		  /* Max length of handle      */
#define USERID_LEN 7+1			  /* 7 chars null terminated   */

/* Automata States */
#define LOGON	 0			  /* Ready to receive log-on   */
#define HANDLE	 1			  /* Requesting handle	       */
#define IDLE	 2			  /* Term ok to receive msgs.  */
#define LOGOFF	 3			  /* Term is logging off       */
#define NO_ECHO  4			  /* Term does not want echo   */

int number_of_users;			  /* number of users logged on */
int MAX_ON;				  /* highest index so far      */

void allow_logon();                       /* functions called in this  */
void broadcast();                         /* source module             */
void logoff();
void comm_down();
void udata();
void uconnect();
void ulinedrop();
void ubreak();
void uinitialize();
void udisconnect();
void ubreak();
void utimeout();
char *getuser();

extern void sig_handler();                /* GEnie provided routine    */

struct {				  /* user table 	       */
    FILE *fp;
    int  state;
    char handle[HANDLE_LEN];
    char uid[USERID_LEN];
    int echo;
} user[MAX_USERS];

char buffer[BUFFER_SIZE];		  /* input buffer	       */
char string[BUFFER_SIZE+81];		  /* output buffer	       */
char helpmsg[350];			  /* help message buffer       */

/************************************************************************
* main()                                                                *
*		 main program and main processing loop			*
************************************************************************/
main ()
{
  struct next *np;			  /* pointer for nextinput     */
  int index;				  /* index into user table     */

  uinitialize();                          /* initialization routine    */

  while (TRUE) {                          /* wait for 60 secs, 1 min   */
    np = nextinput(60, 0);                /* for nextimput to return   */

    if (np->file_pointer)
	index = fileno(np->file_pointer);     /* fileno is index into aray */
    else
	index = 0;
    /*
    printf("fp %08lx index %d status %d\n", np->file_pointer, index, np->status);
    */

    switch (np->status) {                 /* what did nextinput return?*/

      case S_DATA:			  /* there's data, call udata  */
	udata(index);                     /* pass index into table     */
      break;

      case S_CONNECT:			  /* someone connected	       */
	uconnect(np->file_pointer,index); /* pass file pointer & index */
      break;

      case S_BREAK:			  /* break, acknowledge it     */
	ubreak(index);                    /* pass index into table     */
      break;

      case S_DISC:			  /* disconnect 	       */
	udisconnect(index);               /* pass index into table     */
      break;

      case S_TIMEOUT:			  /* input time out	       */
      break;

      default:				  /* no activity	       */
	delrt();                          /* tell GEnie we're down     */
	comm_down();                      /* just in case, relog users */
	exit();                           /* and terminate             */
      break;

    } /* end of switch */

  } /* end of while(TRUE) */

} /* end of main() routine */

/************************************************************************
*  uinitialize()                                                        *
*   called once upon program startup; do any necessary initialization	*
************************************************************************/
void uinitialize()
{
  int i;


/*  signal(SIGTERM,comm_down);            /* look for comm shut down   */
/*  signal(SIGMEM,sig_handler);           /* abort and dump on MEM flt */
/*  signal(SIGOVF,sig_handler);           /* abort and dump on OVF flt */

  helpmsg[0] = '\0';                      /* build help message        */
  strcat(helpmsg,"These are the available commands:\n\n");
  strcat(helpmsg,"   /bye      - sign-off the CB\n");
  strcat(helpmsg,"   /echo     - toggle echo mode\n");
  strcat(helpmsg,"   /help     - see this list of commands\n");
  strcat(helpmsg,"   /status   - see all users on CB\n");
  strcat(helpmsg,"   /time     - date/time information\n\n");

  for (i=0;i<MAX_USERS;++i) {             /* initialize user table     */
    user[i].fp = (FILE *) 0;
    user[i].state = LOGON;
    user[i].echo = FALSE;
    }
  allow_logon();                          /* allow first user to logon */
}

/************************************************************************
*  udata()                                                              *
* called when we get a data event, index is pointer into table of users *
************************************************************************/
void udata(index)
int index;
{
  int i,dtnum;

  fgets(buffer,BUFFER_SIZE,user[index].fp);
  switch (user[index].state) {
    case IDLE:				  /* idle state 	       */
      if (*buffer == '/')  {              /* is this a command?        */
	strupper(buffer);
	if (!strncmp(buffer,"/HELP",4)) {
	  fputs(helpmsg,user[index].fp);
	  fflush(user[index].fp);
	  }
	else if (!strncmp(buffer,"/STATUS",4)) {    /* STATUS request? */
	  fprintf(user[index].fp,
		  "These are the users on the CB:\n\n");
	  fputs("   Handle            U#\n\n",user[index].fp);
	  for (i=1; i < MAX_USERS; i++)
	    if (user[i].state == IDLE)
	      fprintf(user[index].fp,"   %-16.16s  %s\n",
		      user[i].handle,user[i].uid);
	  fputc('\n',user[index].fp);
	  fflush(user[index].fp);
	  }
	else if (!strncmp(buffer,"/TIME",4)) {      /* TIME request?   */
	  fputs("Today is: ",user[index].fp);
	  time(&dtnum);
	  fputs(ctime(&dtnum),user[index].fp);
	  fflush(user[index].fp);
	  }
	else if (!strncmp(buffer,"/ECHO",4)) {      /* ECHO request?   */
	  if (user[index].echo)
	    user[index].echo = FALSE;
	  else
	    user[index].echo = TRUE;
	  if (user[index].echo)
	    fputs("Echo is on.\n",user[index].fp);
	  else
	    fputs("Echo off.\n",user[index].fp);
	  fflush(user[index].fp);
	 }
	else if (!strncmp(buffer,"/BYE",4)) {      /* LOG-OFF request? */
	  fputs("Logging off, have a nice day.\n",user[index].fp);
	  user[index].state = LOGOFF;	  /* place in log-off state    */
	  sprintf(string,"*<%s> is off\n",user[index].handle);
	  broadcast(string,index);        /* tell the world            */
	  logoff(index);                  /* log-off the user          */
	  }
	else {
	  fputs("Invalid command, type /HELP for list of valid commands.\n",
		user[index].fp);
	  fflush(user[index].fp);
	  }
	}
      else {				  /* send message to everyone  */
	sprintf(string,"<%s> %s",user[index].handle,buffer);
	broadcast(string,index);
      }
      break;

    case HANDLE:			  /* handle entered state      */
      if (*buffer == '\n') {
	fputs("Please enter a handle: ",user[index].fp);
	fflush(user[index].fp);
	}
      else {
	buffer[strlen(buffer)-1] = '\0';  /* get rid of newline        */
	strncpy(user[index].handle,buffer,HANDLE_LEN);
	sprintf(string,"%s, You are now logged on.  ",user[index].handle);
	strcat(string,"For a list of valid commands, type /HELP\n");
	fputs(string,user[index].fp);
	fflush(user[index].fp);
	sprintf(string,"*<%s> is on\n",user[index].handle);
	broadcast(string,index);          /* tell the world            */
	user[index].state = IDLE;	  /* place in idle mode        */
	}
      break;

    default:				  /* error, won't happen       */
      fprintf(user[index].fp,
      "Error, took default branch state is  %d\n",user[index].state);
      fflush(user[index].fp);
      break;
  }  /* end switch */
} /* udata() */

/************************************************************************
* uconnect()                                                            *
*	       called when a user attaches to the program		*
************************************************************************/
void uconnect(fp,index)
FILE *fp; int index;
{
struct streamio sio;			  /* will be filled via ioctl  */
char *uid_ptr;				  /* will point to u#	       */
int t;

  time(&t);
  fprintf(stdout, "logon\n");
  if (!strncmp("x","EMERGENCYSHUTDOWN",17)) {
    delrt();                              /* tell GEnie we're down     */
    comm_down();                          /* if ^ is PID, log every    */
    fclose(fp);                           /* user off, close this one, */
    exit();                               /* and terminate             */
    }
  if (number_of_users == MAX_USERS) {     /* are we at our limit?      */
    fprintf(fp,"\nSorry, user limit exceeded, try again later\n");
    fclose(fp);                           /* close stream              */
    }
  else {				  /* tell the world of log-on  */
    if (index > MAX_ON)                   /* MAX_ON is the highest     */
      MAX_ON = index;			  /* index since pgm startup   */
    user[index].fp = fp;		  /* put new user into table   */
    number_of_users++;			  /* increment number of users */
    user[index].state = HANDLE; 	  /* state is request handle   */
    fputs("\n*** Welcome to the mini-CB ***\n\nEnter your handle: ",fp);
    fflush(fp);
    ioctl(fileno(fp),O_GETP,&sio);        /* get I/O parameters        */
    sio.input_time_out = 8*60;		  /* input timeout = 8 minutes */
    sio.typeahead_size = 31;		  /* set to max buffers        */
    sio.writes_queued = 32;		  /* maximum queued writes     */
    ioctl(fileno(fp),O_SETP,&sio);        /* set I/O parameters        */
    uid_ptr = getuser(fp);                /* GEnie routine to log user */
    memcpy(user[index].uid,uid_ptr,8);    /* save users U#             */
    }
  allow_logon();                          /* allow another log-on      */
} /* end uconnect() */

/************************************************************************
*  ubreak()                                                             *
*    called when user hits the break key, just acknowledge we got it	*
************************************************************************/
void ubreak(index)
int index;
{
  fgets(buffer,BUFFER_SIZE,user[index].fp);         /* read the break  */
  fputs("\nBREAK Acknowledged\n",user[index].fp);
  fflush(user[index].fp);
}

/************************************************************************
*  disconnect()                                                         *
*	     called when we get a disconnect event			*
************************************************************************/
void udisconnect(index)
int index;
{
   logoff(index);                         /* log-off the user          */
   if (user[index].state != HANDLE) {     /* he never made it on       */
     sprintf(string,"*<%s> has disconnected\n",user[index].handle);
     broadcast(string,index);             /* tell the world            */
     }
}

/************************************************************************
*  timeout()                                                            *
*	    called when we get a timeout event, log off user		*
************************************************************************/
void utimeout(index)
int index;
{
   fputs("\nYou have been idle for too long\n",user[index].fp);
   fflush(user[index].fp);
   logoff(index);                         /* log-off the user          */
}

/************************************************************************
* allow_logon()                                                         *
*	allow another terminal to log on, terminate if we can't         *
************************************************************************/
void allow_logon()
  {
    int flags;				  /* flags for open statement  */
					  /* set up flags:	       */
    flags  = O_RDWR |			  /*	 read and write access */
	     O_NDELAY | 		  /*	 unroadblocked I/O     */
	     O_NO_SIGNAL |		  /*	 don't give me signals */
	     O_NO_DISC; 		  /*	 give U#= on close     */
    if (!(openport(flags, 0)))  {         /* open a port for a log-on  */
      comm_down();                        /* if error, boot everyone   */
      delrt();                            /* tell GEnie we're down     */
      exit();                             /* and terminate             */
      }
  } /* end allow_logon() */

/************************************************************************
* logoff()                                                              *
*	  relog the user, close the stream. and clear out array 	*
************************************************************************/
void logoff(index)
  int index;

  {
    touser(user[index].fp);               /* return user to GEnie      */
    fclose(user[index].fp);               /* close the stream          */
    number_of_users--;			  /* decrement number of users */
    user[index].fp = 0; 		  /* make table entry available*/
    user[index].state = LOGON;		  /* put in not connected state*/
  } /* end logoff() */

/************************************************************************
* broadcast()                                                           *
*		Broadcast a message to every user online		*
************************************************************************/
void broadcast(stringy,index)
  char *stringy;
  int index;

  {
    int i;
    int save;

    save = user[index].state;		  /* save this user's state    */
    if (!(user[index].echo))              /* does user want echo?      */
      user[index].state = NO_ECHO;	  /* no, change state	       */

    for( i = 0; i <= MAX_ON; i++)         /* for any users online      */
      if (user[i].state == IDLE)  {       /* if idle user, write msg   */
	fputs(stringy,user[i].fp);
	fflush(user[i].fp);
      }

    user[index].state = save;		  /* restore user state        */
    } /* end broadcast() */

/************************************************************************
* comm_down()                                                           *
* Log off all users.  Called if job is idle or processor is going down. *
************************************************************************/
void comm_down()
{
    int i;

    strcpy(string,
    "<PROCESSOR MESSAGE> Going down for maintainence.  Please relogon.");
    for( i = 0; i <= MAX_ON; i++)         /* for all users online      */
      if (user[i].state != LOGON)  {      /* send message and log-off  */
	fputs(string,user[i].fp);
	logoff(i);
      }
} /* end comm_dowm() */

