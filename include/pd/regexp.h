/*
 * Definitions etc. for regexp(3) routines.
 *
 * Caveat:  this is V8 regexp(3) [actually, a reimplementation thereof],
 * not the System V one.
 */
#define NSUBEXP  10
typedef struct regexp {
	char *startp[NSUBEXP];
	char *endp[NSUBEXP];
	char regstart;		/* Internal use only. */
	char reganch;		/* Internal use only. */
	char *regmust;		/* Internal use only. */
	int regmlen;		/* Internal use only. */
	char program[1];	/* Unwarranted chumminess with compiler. */
} regexp;

#ifdef __STDC__
extern regexp *regcomp(char *);
extern int regexec(regexp *, char *);
extern char *regsub(regexp *, char *, char *);
extern void regerror(char *);
#else
extern regexp *regcomp();
extern int regexec();
extern char *regsub();
extern void regerror();
#endif



