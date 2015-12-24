
/*
 *  SUPLIB/STRING.H
 */

#if 0
extern char *strdup(const char *);
#endif
extern int stricmp(const char *, const char *);
extern int strnicmp(const char *, const char *, int);
extern void strins(char *, const char *);
extern char *LocatePath(const char *envvar, const char *fname);

