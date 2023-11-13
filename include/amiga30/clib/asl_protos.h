#ifndef  CLIB_ASL_PROTOS_H
#define  CLIB_ASL_PROTOS_H
/*
**      $VER: asl_protos.h 38.3 (19.03.92)
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
#ifndef  UTILITY_TAGITEM_H
#include <utility/tagitem.h>
#endif
#ifndef  LIBRARIES_ASL_H
#include <libraries/asl.h>
#endif
/*--- functions in V36 or higher (distributed as Release 2.0) ---*/

/* OBSOLETE -- Please use the generic requester functions instead */

struct FileRequester *AllocFileRequest( void );
void FreeFileRequest( struct FileRequester *fileReq );
BOOL RequestFile( struct FileRequester *fileReq );
APTR AllocAslRequest( unsigned long reqType, struct TagItem *tagList );
APTR AllocAslRequestTags( unsigned long reqType, Tag Tag1, ... );
void FreeAslRequest( APTR requester );
BOOL AslRequest( APTR requester, struct TagItem *tagList );
BOOL AslRequestTags( APTR requester, Tag Tag1, ... );
#endif   /* CLIB_ASL_PROTOS_H */
