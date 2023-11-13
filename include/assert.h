
/*
 * $VER: assert.h 1.0 (17.4.93)
 *
 * (c)Copyright 1992 Obvious Implementations Corp, All Rights Reserved
 *
 *  relatively optimized, takes advantage of GNU-cpp __BASE_FILE__ macro
 *  allowing us to store the filename string once in a static decl.
 */

#ifndef ASSERT_H
#define ASSERT_H

static char *__BaseFile = __BASE_FILE__;

extern void __FailedAssert(char *, int);

#endif

/*
 *  if user has not overriden our assert, we define it.  We must handle
 *  the possibility of redefinition.
 */

#ifdef _ASSERT_IS_LOCAL
#undef assert
#endif

#ifndef assert
#define _ASSERT_IS_LOCAL
#ifdef NDEBUG
#define assert(ignore)
#else
#define assert(exp)     if (!(exp)) __FailedAssert( __BaseFile, __LINE__);
#endif
#endif

