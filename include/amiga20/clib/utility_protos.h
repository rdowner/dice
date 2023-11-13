#ifndef  CLIB_UTILITY_PROTOS_H
#define  CLIB_UTILITY_PROTOS_H
/*
**      $Filename: clib/utility_protos.h $
**      $Release: 2.04 $
**      $Revision: 30.156 $
**      $Date: 1995/01/11 05:18:13 $
**
**      C prototypes. For use with 32 bit integers only.
**
**      (C) Copyright 1990 Commodore-Amiga, Inc.
**          All Rights Reserved
*/
/* "utility.library" */
#ifndef  UTILITY_TAGITEM_H
#include <utility/tagitem.h>
#endif
#ifndef  UTILITY_DATE_H
#include <utility/date.h>
#endif
#ifndef  UTILITY_HOOKS_H
#include <utility/hooks.h>
#endif
/* *** TagItem FUNCTIONS *** */
struct TagItem *FindTagItem( Tag tagVal, struct TagItem *tagList );
ULONG GetTagData( Tag tagVal, unsigned long defaultVal,
        struct TagItem *tagList );
ULONG PackBoolTags( unsigned long initialFlags, struct TagItem *tagList,
        struct TagItem *boolMap );
struct TagItem *NextTagItem( struct TagItem **tagListPtr );
void FilterTagChanges( struct TagItem *newTagList, struct TagItem *oldTagList,
        long apply );
void MapTags( struct TagItem *tagList, struct TagItem *mapList,
        long includeMiss );
struct TagItem *AllocateTagItems( unsigned long numItems );
struct TagItem *CloneTagItems( struct TagItem *tagList );
void FreeTagItems( struct TagItem *tagList );
void RefreshTagItemClones( struct TagItem *cloneList,
        struct TagItem *origList );
BOOL TagInArray( Tag tagVal, Tag *tagArray );
LONG FilterTagItems( struct TagItem *tagList, Tag *filterArray, long logic );
/**/
/* *** HOOK FUNCTIONS *** * */
ULONG CallHookPkt( struct Hook *hook, APTR object, APTR paramPacket );
/**/
/* *** DATE FUNCTIONS *** * */
void Amiga2Date( unsigned long amigaTime, struct ClockData *date );
ULONG Date2Amiga( struct ClockData *date );
ULONG CheckDate( struct ClockData *date );
/**/
/* *** 32 BIT MATH FUNCTIONS *** * */
LONG SMult32( long factor1, long factor2 );
ULONG UMult32( unsigned long factor1, unsigned long factor2 );
/* NOTE: Quotient:Remainder returned in d0:d1 */
LONG SDivMod32( long dividend, long divisor );
ULONG UDivMod32( unsigned long dividend, unsigned long divisor );
/**/
/* *** International string routines *** */
LONG Stricmp( UBYTE *string1, UBYTE *string2 );
LONG Strnicmp( UBYTE *string1, UBYTE *string2, long length );
UBYTE ToUpper( unsigned long character );
UBYTE ToLower( unsigned long character );
#endif   /* CLIB_UTILITY_PROTOS_H */
