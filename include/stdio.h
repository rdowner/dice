
/*
 * $VER: stdio.h 1.0 (17.4.93)
 *
 * (c)Copyright 1992 Obvious Implementations Corp, All Rights Reserved
 */

#ifndef STDIO_H
#define STDIO_H

#ifndef STDDEF_H
#include <stddef.h>
#endif

#ifndef STDARG_H
typedef void *va_list;
#endif

typedef long    fpos_t;

typedef struct FILE {
    struct FILE *sd_Next;
    struct FILE **sd_Prev;
    unsigned char   *sd_RBuf;
    unsigned char   *sd_RPtr;
    unsigned char   *sd_WBuf;
    unsigned char   *sd_WPtr;
    long    sd_BufSiz;
    int     sd_RLeft;
    int     sd_WLeft;
    int     sd_Flags;
    int     sd_Error;       /*  error: 0 / non-0    */
    fpos_t  sd_Offset;      /*  level 1 current off */
    int     sd_Fd;
    short   sd_UC;          /*  ungotten character or -1    */
    char    *sd_Name;       /*  use for tmpfile()   */
} FILE;

typedef struct _IOFDS {
    long    fd_Fh;
    long    fd_Flags;
    long    (*fd_Exec)(long, int, void *, void *);   /*  exec function       */
    char    *fd_FileName;   /*  file name for fstat() under 1.3 */
} _IOFDS;

extern FILE _Iob[3];                /*  fixed fps           */
extern FILE *_Iod;                  /*  dynamic fps         */

extern _IOFDS _IoStaticFD[3];       /*  0,1,2               */
extern _IOFDS *_IoFD;               /*  descriptors         */
extern short _IoFDLimit;
extern int _bufsiz;                 /*  default BUFSIZ      */

#define __SIF_ERROR     0x0001  /*  XXX */
#define __SIF_EOF       0x0002
#define __SIF_READ      0x0004
#define __SIF_WRITE     0x0008
#define __SIF_IOLBF     0x0010
#define __SIF_REMOVE    0x0020  /*  unlink file after closing   */
#define __SIF_APPEND    0x0040
#define __SIF_NOFREE    0x0080  /*  not a freeable descriptor   */
#define __SIF_OPEN      0x0100  /*  file is open                */
#define __SIF_MYBUF     0x0200  /*  I allocated the buffer(s)   */
#define __SIF_BINARY    0x0400
#define __SIF_RAW       0x0800  /*  RAW terminal , see setvbuf  */
#define __SIF_FILE      0x1000  /*  a seekable file             */

#define _IOFBF  -1
#define _IOLBF  __SIF_IOLBF
#define _IONBF  0

#define BUFSIZ  1024
#define EOF     -1
#define FILENAME_MAX    1024
#define FOPEN_MAX       20      /*  lets be reasonable          */
#define L_tmpnam        32
#define SEEK_SET        0
#define SEEK_CUR        1
#define SEEK_END        2
#define TMP_MAX         4096
#define stdin   (&_Iob[0])
#define stdout  (&_Iob[1])
#define stderr  (&_Iob[2])

#define clearerr(fi)    (((fi)->sd_Flags &= ~__SIF_EOF), ((fi)->sd_Error = 0))
#define feof(fi)        ((fi)->sd_Flags & __SIF_EOF)
#define ferror(fi)      ((fi)->sd_Error)
#define fileno(fi)      ((fi)->sd_Fd)

extern void perror(const char *);

extern int    fclose(FILE *);
extern int    fflush(FILE *);
extern FILE * fopen(const char *, const char *);
extern FILE * freopen(const char *, const char *, FILE *);
extern void   setbuf(FILE *, char *);
extern int    setvbuf(FILE *, char *, int, size_t);

extern int    remove(const char *);
extern int    rename(const char *, const char *);
extern FILE * tmpfile(void);
extern char * tmpnam(char *);

extern int    fgetpos(FILE *, fpos_t *);
extern int    fseek(FILE *, long, int);
extern int    fsetpos(FILE *, const fpos_t *);
extern long   ftell(FILE *);
extern void   rewind(FILE *);

extern int    fgetc(FILE *);
extern char * fgets(char *, int, FILE *);
extern int    fputc(unsigned char, FILE *);
extern int    fputs(const char *, FILE *);

#define getc(fi)   ( ((fi)->sd_UC >= 0) ? fgetc(fi) : ((fi)->sd_RLeft <= 0) ? fgetc(fi) : (--(fi)->sd_RLeft, (int)*(fi)->sd_RPtr++))

#ifdef NOTDEF   /* old putc() would reference 'c' twice */
#define putc(c,fi) (((c) == '\n' || (fi)->sd_WLeft <= 0) ? fputc((c),(fi)) : (*(fi)->sd_WPtr++ = (c), --(fi)->sd_WLeft, (c)))
#endif

/*
 *  new putc macro only references (c) once in the execution path,
 *  also fixed bug with '\n' (would previously fflush() even if not
 *  line buffered)
 */

#define putc(c,fi) (((fi)->sd_WLeft <= 0) ? fputc((c),(fi)) : \
         (((--(fi)->sd_WLeft, ((*(fi)->sd_WPtr++ = (c))) == '\n') && ((fi)->sd_Flags & _IOLBF)) ? (fflush(fi), '\n') : ((fi)->sd_WPtr)[-1]))


#define getchar()  fgetc(stdin)
#define putchar(c) fputc((c),stdout)

extern char * gets(char *);
extern int    puts(const char *);
extern int    ungetc(int, FILE *);

extern size_t fread(void *, size_t, size_t, FILE *);
extern size_t fwrite(const void *, size_t, size_t, FILE *);

extern int    fprintf(FILE *, const char *, ...);
extern int    fscanf(FILE *, const char *, ...);
extern int    printf(const char *, ...);
extern int    scanf(const char *, ...);
extern int    sprintf(char *, const char *, ...);
extern int    sscanf(char *, const char *, ...);

extern int    vfprintf(FILE *, const char *, va_list);
extern int    vprintf(const char *, va_list);
extern int    vsprintf(char *, const char *, va_list);

/*
 *  Standard, but not ANSI
 */

extern FILE * fdopen(int, const char *);
extern int    chdir(const char *);
extern char * getcwd(char *, int);
extern int    unlink(const char *);

/*
 *  Amiga specific
 */

extern int    fhprintf(long, const char *, ...);
extern int    vfhprintf(long, const char *, va_list);

#endif

