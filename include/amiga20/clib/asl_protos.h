#ifndef  CLIB_ASL_PROTOS_H
#define  CLIB_ASL_PROTOS_H
/*
**	$Filename: clib/asl_protos.h $
**	$Release: 2.04 $
**	$Revision: 30.156 $
**	$Date: 1995/01/11 05:18:07 $
**
**	C prototypes. For use with 32 bit integers only.
**
**	(C) Copyright 1990 Commodore-Amiga, Inc.
**	    All Rights Reserved
*/
/* "asl.library" */
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
/**/
struct FileRequester *AllocFileRequest( void );
void FreeFileRequest( struct FileRequester *fileReq );
BOOL RequestFile( struct FileRequester *fileReq );
APTR AllocAslRequest( unsigned long type, struct TagItem *tagList );
APTR AllocAslRequestTags( unsigned long type, Tag Tag1, ... );
void FreeAslRequest( APTR request );
BOOL AslRequest( APTR request, struct TagItem *tagList );
BOOL AslRequestTags( APTR request, Tag Tag1, ... );
#endif	 /* CLIB_ASL_PROTOS_H */
