/*
**      $Id: alib_stdio_pragmas.h,v 30.0 1994/06/10 18:10:33 dice Exp $
**
**      SAS/C format pragma files.
**
**      Contains private definitions. COMMODORE INTERNAL USE ONLY!
*/


/* stdio functions that duplicate those in a typical C library */

#ifdef __SASC_60
#pragma tagcall None printf 18 801
#endif
#ifdef __SASC_60
#pragma tagcall None sprintf 18 9802
#endif
#pragma libcall None fclose 1e 001
#pragma libcall None fgetc 24 001
#ifdef __SASC_60
#pragma tagcall None fprintf 24 8002
#endif
#pragma libcall None fputc 2a 1002
#pragma libcall None fputs 30 0802
#pragma libcall None getchar 36 00
#pragma libcall None putchar 3c 001
#pragma libcall None puts 42 801
