#ifndef  CLIB_LOCALE_PROTOS_H
#define  CLIB_LOCALE_PROTOS_H
/*
**	$VER: locale_protos.h 38.4 (23.06.92)
**	Includes Release 39.108
**
**	C prototypes. For use with 32 bit integers only.
**
**	(C) Copyright 1990-1992 Commodore-Amiga, Inc.
**	    All Rights Reserved
*/
#ifndef  EXEC_TYPES_H
#include <exec/types.h>
#endif
#ifndef  LIBRARIES_LOCALE_H
#include <libraries/locale.h>
#endif
#ifndef  DOS_DOS_H
#include <dos/dos.h>
#endif
#ifndef  UTILITY_HOOKS_H
#include <utility/hooks.h>
#endif
#ifndef  UTILITY_TAGITEM_H
#include <utility/tagitem.h>
#endif
#ifndef  REXX_STORAGE_H
#include <rexx/storage.h>
#endif
/*--- functions in V38 or higher (distributed as Release 2.1) ---*/

/* Public entries */

void CloseCatalog( struct Catalog *catalog );
void CloseLocale( struct Locale *locale );
ULONG ConvToLower( struct Locale *locale, unsigned long character );
ULONG ConvToUpper( struct Locale *locale, unsigned long character );
void FormatDate( struct Locale *locale, STRPTR template,
	struct DateStamp *date, struct Hook *putCharFunc );
APTR FormatString( struct Locale *locale, STRPTR string, APTR dataStream,
	struct Hook *putCharFunc );
STRPTR GetCatalogStr( struct Catalog *catalog, long stringNum,
	STRPTR defaultString );
STRPTR GetLocaleStr( struct Locale *locale, unsigned long stringNum );
BOOL IsAlNum( struct Locale *locale, unsigned long character );
BOOL IsAlpha( struct Locale *locale, unsigned long character );
BOOL IsCntrl( struct Locale *locale, unsigned long character );
BOOL IsDigit( struct Locale *locale, unsigned long character );
BOOL IsGraph( struct Locale *locale, unsigned long character );
BOOL IsLower( struct Locale *locale, unsigned long character );
BOOL IsPrint( struct Locale *locale, unsigned long character );
BOOL IsPunct( struct Locale *locale, unsigned long character );
BOOL IsSpace( struct Locale *locale, unsigned long character );
BOOL IsUpper( struct Locale *locale, unsigned long character );
BOOL IsXDigit( struct Locale *locale, unsigned long character );
struct Catalog *OpenCatalogA( struct Locale *locale, STRPTR name,
	struct TagItem *tags );
struct Catalog *OpenCatalog( struct Locale *locale, STRPTR name, Tag tag1,
	... );
struct Locale *OpenLocale( STRPTR name );
BOOL ParseDate( struct Locale *locale, struct DateStamp *date, STRPTR template,
	struct Hook *getCharFunc );
ULONG StrConvert( struct Locale *locale, STRPTR string, APTR buffer,
	unsigned long bufferSize, unsigned long type );
LONG StrnCmp( struct Locale *locale, STRPTR string1, STRPTR string2,
	long length, unsigned long type );
#endif	 /* CLIB_LOCALE_PROTOS_H */
