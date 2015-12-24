
/*
 * $VER: string.h 1.0 (17.4.93)
 *
 * (c)Copyright 1992 Obvious Implementations Corp, All Rights Reserved
 */

#ifndef STRING_H
#define STRING_H

#ifndef STDDEF_H
#include <stddef.h>
#endif


extern int memcmp(const void *, const void *, size_t);
extern int strcmp(const char *, const char *);
extern int strcoll(const char *, const char *);
extern int strncmp(const char *, const char *, size_t);
extern size_t strxfrm(char *, const char *, size_t);

extern char *strcat(char *, const char *);
extern char *strncat(char *, const char *, size_t);

extern void *memcpy(void *, const void *, size_t);
extern void *memmove(void *, const void *, size_t);
extern char *strcpy(char *, const char *);
extern char *stpcpy(char *, const char *);
extern char *strncpy(char *, const char *, size_t);
extern void *memset(void *, int, size_t);
extern char *strerror(int);
extern size_t strlen(const char *);

extern void *memchr(const void *, int, size_t);
extern char *strchr(const char *, int);
extern size_t strcspn(const char *, const char *);
extern char *strpbrk(const char *, const char *);
extern char *strrchr(const char *, int);
extern size_t strspn(const char *, const char *);
extern char *strstr(const char *, const char *);
extern char *strtok(char *, const char *);

extern double strtod(const char *, char **);
extern long   strtol(const char *, char **, int);
extern unsigned long strtoul(const char *, char **, int);

/*
 *  Standard, but not ANSI
 */

extern void *movmem(const void *, const void *, size_t);
extern void *setmem(void *, size_t, int);
extern void *cmpmem(const void *, const void *, size_t);
extern void *clrmem(void *, size_t);
extern void *bzero(void *, size_t);
extern void *bcopy(const void *, void *, size_t);
extern void *bcmp(const void *, const void *, size_t);
extern char *strupper(char *);

/*
 *  Other string routines
 */

extern int stricmp(const char *, const char *);
extern int strnicmp(const char *, const char *, int);
extern int strbpl(char **, int, const char *);
extern void strins(char *, const char *);
extern char *strdup(const char *);
extern char *stpbrk(const char *, const char *);
extern const char *stpchr(const char *, char);


#endif

