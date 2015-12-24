
/*
 * $VER: lib/unix.h 1.0 (17.4.93)
 *
 * (c)Copyright 1992 Obvious Implementations Corp, All Rights Reserved
 */

#ifndef LIB_UNIX_H
#define LIB_UNIX_H

char *UnixToAmigaPath(char *);
char *AmigaToUnixPath(char *);
int sigcheckchld(void); 	    /*	private */
void amiga_vfork_sigall(long);	    /*	private */

int kill(int, int);
int wait3(union wait *, int, struct rusage *);
int wait(union wait *);
int amiga_dup2(int, int);
int amiga_execseg(char **, struct Segment *, long);
int amiga_execlp(char *, char *, ...);
int amiga_execvp(char *, char **);
int amiga_execl(char *, char *, ...);
int amiga_execv(char *, char **);
int _amiga_vforkc(void);

#endif

