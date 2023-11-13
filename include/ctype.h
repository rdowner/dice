
/*
 * $VER: ctype.h 1.0 (17.4.93)
 *
 * (c)Copyright 1992 Obvious Implementations Corp, All Rights Reserved
 */

#ifndef CTYPE_H
#define CTYPE_H

#ifdef CTYPE_NEAR
#define _CTYPE_NEAR __near
#else
#define _CTYPE_NEAR
#endif

extern _CTYPE_NEAR const unsigned char __CArya[257];
extern _CTYPE_NEAR const unsigned char __CAryb[257];
extern _CTYPE_NEAR const unsigned char __CUToL[257];
extern _CTYPE_NEAR const unsigned char __CLToU[257];

#undef _CTYPE_NEAR

#define __CF_ISALNUM    0x01
#define __CF_ISALPHA    0x02
#define __CF_ISCNTRL    0x04
#define __CF_ISDIGIT    0x08
#define __CF_ISGRAPH    0x10
#define __CF_ISLOWER    0x20
#define __CF_ISSPACE    0x40
#define __CF_ISUPPER    0x80

#define __CF_ISPUNCT    0x01
#define __CF_ISXDIGIT   0x02
#define __CF_ISPRINT    0x04

extern int isalnum(int);
extern int isalpha(int);
extern int iscntrl(int);
extern int isdigit(int);
extern int isgraph(int);
extern int islower(int);
extern int isspace(int);
extern int isupper(int);

extern int ispunct(int);
extern int isxdigit(int);
extern int isprint(int);

extern int tolower(int);
extern int toupper(int);

#define isalnum(c)  ((__CArya+1)[c] & __CF_ISALNUM)
#define isalpha(c)  ((__CArya+1)[c] & __CF_ISALPHA)
#define iscntrl(c)  ((__CArya+1)[c] & __CF_ISCNTRL)
#define isdigit(c)  ((__CArya+1)[c] & __CF_ISDIGIT)
#define isgraph(c)  ((__CArya+1)[c] & __CF_ISGRAPH)
#define islower(c)  ((__CArya+1)[c] & __CF_ISLOWER)
#define isspace(c)  ((__CArya+1)[c] & __CF_ISSPACE)
#define isupper(c)  ((__CArya+1)[c] & __CF_ISUPPER)

#define ispunct(c)  ((__CAryb+1)[c] & __CF_ISPUNCT)
#define isxdigit(c) ((__CAryb+1)[c] & __CF_ISXDIGIT)
#define isprint(c)  ((__CAryb+1)[c] & __CF_ISPRINT)

#define tolower(c)  ((__CUToL+1)[c])
#define toupper(c)  ((__CLToU+1)[c])

#endif

