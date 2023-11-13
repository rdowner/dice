#ifndef  CLIB_UTILITY_PROTOS_H
#define  CLIB_UTILITY_PROTOS_H
/*
**      $VER: utility_protos.h 39.11 (03.06.92)
**      Includes Release 39.108
**
**      C prototypes. For use with 32 bit integers only.
**
**      (C) Copyright 1990-1992 Commodore-Amiga, Inc.
**          All Rights Reserved
*/
#ifndef  EXEC_TYPES_H
#include <exec/types.h>
#endif
#ifndef  EXEC_PORTS_H
#include <exec/ports.h>
#endif
#ifndef  UTILITY_TAGITEM_H
#include <utility/tagitem.h>
#endif
#ifndef  UTILITY_DATE_H
#include <utility/date.h>
#endif
#ifndef  UTILITY_HOOKS_H
#include <utility/hooks.h>
#endif
#ifndef  UTILITY_NAME_H
#include <utility/name.h>
#endif
/*--- functions in V36 or higher (distributed as Release 2.0) ---*/

/* Tag item functions */

struct TagItem *FindTagItem( Tag tagVal, struct TagItem *tagList );
ULONG GetTagData( Tag tagValue, unsigned long defaultVal,
        struct TagItem *tagList );
ULONG PackBoolTags( unsigned long initialFlags, struct TagItem *tagList,
        struct TagItem *boolMap );
struct TagItem *NextTagItem( struct TagItem **tagListPtr );
void FilterTagChanges( struct TagItem *changeList,
        struct TagItem *originalList, unsigned long apply );
void MapTags( struct TagItem *tagList, struct TagItem *mapList,
        unsigned long mapType );
struct TagItem *AllocateTagItems( unsigned long numTags );
struct TagItem *CloneTagItems( struct TagItem *tagList );
void FreeTagItems( struct TagItem *tagList );
void RefreshTagItemClones( struct TagItem *clone, struct TagItem *original );
BOOL TagInArray( Tag tagValue, Tag *tagArray );
ULONG FilterTagItems( struct TagItem *tagList, Tag *filterArray,
        unsigned long logic );

/* Hook functions */

ULONG CallHookPkt( struct Hook *hook, APTR object, APTR paramPacket );

/* Date functions */

void Amiga2Date( unsigned long seconds, struct ClockData *result );
ULONG Date2Amiga( struct ClockData *date );
ULONG CheckDate( struct ClockData *date );

/* 32 bit integer muliply functions */

LONG SMult32( long arg1, long arg2 );
ULONG UMult32( unsigned long arg1, unsigned long arg2 );

/* 32 bit integer division funtions. The quotient and the remainder are */
/* returned respectively in d0 and d1 */

LONG SDivMod32( long dividend, long divisor );
ULONG UDivMod32( unsigned long dividend, unsigned long divisor );
/*--- functions in V37 or higher (distributed as Release 2.04) ---*/

/* International string routines */

LONG Stricmp( STRPTR string1, STRPTR string2 );
LONG Strnicmp( STRPTR string1, STRPTR string2, long length );
UBYTE ToUpper( unsigned long character );
UBYTE ToLower( unsigned long character );
/*--- functions in V39 or higher (beta release for developers only) ---*/

/* More tag Item functions */

void ApplyTagChanges( struct TagItem *list, struct TagItem *changeList );

/* 64 bit integer muliply functions. The results are 64 bit quantities */
/* returned in D0 and D1 */

LONG SMult64( long arg1, long arg2 );
ULONG UMult64( unsigned long arg1, unsigned long arg2 );

/* Structure to Tag and Tag to Structure support routines */

ULONG PackStructureTags( APTR pack, ULONG *packtable,
        struct TagItem *tagList );
ULONG UnpackStructureTags( APTR pack, ULONG *packtable,
        struct TagItem *tagList );

/* New, object-oriented NameSpaces */

BOOL AddNamedObject( struct NamedObject *namespace,
        struct NamedObject *object );
struct NamedObject *AllocNamedObjectA( STRPTR name, struct TagItem *tags );
struct NamedObject *AllocNamedObject( STRPTR name, Tag tag1, ... );
LONG AttemptRemNamedObject( struct NamedObject *object );
struct NamedObject *FindNamedObject( struct NamedObject *namespace,
        STRPTR name, struct NamedObject *lastobject );
void FreeNamedObject( struct NamedObject *object );
STRPTR NamedObjectName( struct NamedObject *object );
void ReleaseNamedObject( struct NamedObject *object );
void RemNamedObject( struct NamedObject *object, struct Message *message );

/* Unique ID generator */

ULONG GetUniqueID( void );
#endif   /* CLIB_UTILITY_PROTOS_H */
