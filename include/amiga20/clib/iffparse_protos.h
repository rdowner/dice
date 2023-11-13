#ifndef  CLIB_IFFPARSE_PROTOS_H
#define  CLIB_IFFPARSE_PROTOS_H
/*
**      $Filename: clib/iffparse_protos.h $
**      $Release: 2.04 $
**      $Revision: 30.156 $
**      $Date: 1995/01/11 05:18:09 $
**
**      C prototypes. For use with 32 bit integers only.
**
**      (C) Copyright 1990 Commodore-Amiga, Inc.
**          All Rights Reserved
*/
/* "iffparse.library" */
#ifndef  EXEC_TYPES_H
#include <exec/types.h>
#endif
/*--- functions in V33 or higher (distributed as Release 1.2) ---*/
/*------ Basic functions ------*/
struct IFFHandle *AllocIFF( void );
LONG OpenIFF( struct IFFHandle *iff, long rwMode );
LONG ParseIFF( struct IFFHandle *iff, long control );
void CloseIFF( struct IFFHandle *iff );
void FreeIFF( struct IFFHandle *iff );
/*------ Read/Write functions ------*/
LONG ReadChunkBytes( struct IFFHandle *iff, APTR buf, long size );
LONG WriteChunkBytes( struct IFFHandle *iff, APTR buf, long size );
LONG ReadChunkRecords( struct IFFHandle *iff, APTR buf, long bytesPerRecord,
        long nRecords );
LONG WriteChunkRecords( struct IFFHandle *iff, APTR buf, long bytesPerRecord,
        long nRecords );
/*------ Context entry/exit ------*/
LONG PushChunk( struct IFFHandle *iff, long type, long id, long size );
LONG PopChunk( struct IFFHandle *iff );
/*------ Low-level handler installation ------*/
LONG EntryHandler( struct IFFHandle *iff, long type, long id, long position,
        struct Hook *handler, APTR object );
LONG ExitHandler( struct IFFHandle *iff, long type, long id, long position,
        struct Hook *handler, APTR object );
/*------ Built-in chunk/property handlers ------*/
LONG PropChunk( struct IFFHandle *iff, long type, long id );
LONG PropChunks( struct IFFHandle *iff, LONG *propArray, long nProps );
LONG StopChunk( struct IFFHandle *iff, long type, long id );
LONG StopChunks( struct IFFHandle *iff, LONG *propArray, long nProps );
LONG CollectionChunk( struct IFFHandle *iff, long type, long id );
LONG CollectionChunks( struct IFFHandle *iff, LONG *propArray, long nProps );
LONG StopOnExit( struct IFFHandle *iff, long type, long id );
/*------ Context utilities ------*/
struct StoredProperty *FindProp( struct IFFHandle *iff, long type, long id );
struct CollectionItem *FindCollection( struct IFFHandle *iff, long type,
        long id );
struct ContextNode *FindPropContext( struct IFFHandle *iff );
struct ContextNode *CurrentChunk( struct IFFHandle *iff );
struct ContextNode *ParentChunk( struct ContextNode *contextNode );
/*------ LocalContextItem support functions ------*/
struct LocalContextItem *AllocLocalItem( long type, long id, long ident,
        long dataSize );
APTR LocalItemData( struct LocalContextItem *localItem );
void SetLocalItemPurge( struct LocalContextItem *localItem,
        struct Hook *purgeHook );
void FreeLocalItem( struct LocalContextItem *localItem );
struct LocalContextItem *FindLocalItem( struct IFFHandle *iff, long type,
        long id, long ident );
LONG StoreLocalItem( struct IFFHandle *iff, struct LocalContextItem *localItem,
        long position );
void StoreItemInContext( struct IFFHandle *iff,
        struct LocalContextItem *localItem,
        struct ContextNode *contextNode );
/*------ IFFHandle initialization ------*/
void InitIFF( struct IFFHandle *iff, long flags, struct Hook *streamHook );
void InitIFFasDOS( struct IFFHandle *iff );
void InitIFFasClip( struct IFFHandle *iff );
/*------ Internal clipboard support ------*/
struct ClipboardHandle *OpenClipboard( long unitNum );
void CloseClipboard( struct ClipboardHandle *clipboard );
/*------ Miscellaneous ------*/
LONG GoodID( long id );
LONG GoodType( long type );
STRPTR IDtoStr( long id, STRPTR buf );
#endif   /* CLIB_IFFPARSE_PROTOS_H */
