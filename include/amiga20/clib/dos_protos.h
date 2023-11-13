#ifndef  CLIB_DOS_PROTOS_H
#define  CLIB_DOS_PROTOS_H
/*
**      $Filename: clib/dos_protos.h $
**      $Release: 2.04 $
**      $Revision: 30.156 $
**      $Date: 1995/01/11 05:18:08 $
**
**      C prototypes. For use with 32 bit integers only.
**
**      (C) Copyright 1990 Commodore-Amiga, Inc.
**          All Rights Reserved
*/
/* "dos.library" */
#ifndef  DOS_DOS_H
#include <dos/dos.h>
#endif
#ifndef  DOS_DOSEXTENS_H
#include <dos/dosextens.h>
#endif
#ifndef  DOS_RECORD_H
#include <dos/record.h>
#endif
#ifndef  DOS_RDARGS_H
#include <dos/rdargs.h>
#endif
#ifndef  DOS_DOSASL_H
#include <dos/dosasl.h>
#endif
#ifndef  DOS_VAR_H
#include <dos/var.h>
#endif
#ifndef  DOS_NOTIFY_H
#include <dos/notify.h>
#endif
#ifndef  DOS_DATETIME_H
#include <dos/datetime.h>
#endif
BPTR Open( UBYTE *name, long accessMode );
LONG Close( BPTR file );
LONG Read( BPTR file, APTR buffer, long length );
LONG Write( BPTR file, APTR buffer, long length );
BPTR Input( void );
BPTR Output( void );
LONG Seek( BPTR file, long position, long offset );
LONG DeleteFile( UBYTE *name );
LONG Rename( UBYTE *oldName, UBYTE *newName );
BPTR Lock( UBYTE *name, long type );
void UnLock( BPTR lock );
BPTR DupLock( BPTR lock );
LONG Examine( BPTR lock, struct FileInfoBlock *fileInfoBlock );
LONG ExNext( BPTR lock, struct FileInfoBlock *fileInfoBlock );
LONG Info( BPTR lock, struct InfoData *parameterBlock );
BPTR CreateDir( UBYTE *name );
BPTR CurrentDir( BPTR lock );
LONG IoErr( void );
struct MsgPort *CreateProc( UBYTE *name, long pri, BPTR segList,
        long stackSize );
void Exit( long returnCode );
BPTR LoadSeg( UBYTE *name );
void UnLoadSeg( BPTR seglist );
struct MsgPort *DeviceProc( UBYTE *name );
LONG SetComment( UBYTE *name, UBYTE *comment );
LONG SetProtection( UBYTE *name, long protect );
struct DateStamp *DateStamp( struct DateStamp *date );
void Delay( long timeout );
LONG WaitForChar( BPTR file, long timeout );
BPTR ParentDir( BPTR lock );
LONG IsInteractive( BPTR file );
LONG Execute( UBYTE *string, BPTR file, BPTR file2 );
/*--- functions in V36 or higher (distributed as Release 2.0) ---*/
/*      DOS Object creation/deletion */
APTR AllocDosObject( unsigned long type, struct TagItem *tags );
APTR AllocDosObjectTagList( unsigned long type, struct TagItem *tags );
APTR AllocDosObjectTags( unsigned long type, unsigned long tag1type, ... );
void FreeDosObject( unsigned long type, APTR ptr );
/*      Packet Level routines */
LONG DoPkt( struct MsgPort *port, long action, long arg1, long arg2, long arg3,
        long arg4, long arg5 );
LONG DoPkt0( struct MsgPort *port, long action );
LONG DoPkt1( struct MsgPort *port, long action, long arg1 );
LONG DoPkt2( struct MsgPort *port, long action, long arg1, long arg2 );
LONG DoPkt3( struct MsgPort *port, long action, long arg1, long arg2,
        long arg3 );
LONG DoPkt4( struct MsgPort *port, long action, long arg1, long arg2,
        long arg3, long arg4 );
void SendPkt( struct DosPacket *dp, struct MsgPort *port,
        struct MsgPort *replyport );
struct DosPacket *WaitPkt( void );
void ReplyPkt( struct DosPacket *dp, long res1, long res2 );
void AbortPkt( struct MsgPort *port, struct DosPacket *pkt );
/*      Record Locking */
BOOL LockRecord( BPTR fh, unsigned long offset, unsigned long length,
        unsigned long mode, unsigned long timeout );
BOOL LockRecords( struct RecordLock *recArray, unsigned long timeout );
BOOL UnLockRecord( BPTR fh, unsigned long offset, unsigned long length );
BOOL UnLockRecords( struct RecordLock *recArray );
/*      Buffered File I/O */
BPTR SelectInput( BPTR fh );
BPTR SelectOutput( BPTR fh );
LONG FGetC( BPTR fh );
void FPutC( BPTR fh, unsigned long ch );
LONG UnGetC( BPTR fh, long character );
LONG FRead( BPTR fh, APTR block, unsigned long blocklen,
        unsigned long number );
LONG FWrite( BPTR fh, APTR block, unsigned long blocklen,
        unsigned long number );
UBYTE *FGets( BPTR fh, UBYTE *buf, unsigned long buflen );
LONG FPuts( BPTR fh, UBYTE *str );
void VFWritef( BPTR fh, UBYTE *format, LONG *argarray );
void FWritef( BPTR fh, UBYTE *format, long arg1, ... );
LONG VFPrintf( BPTR fh, UBYTE *format, LONG *argarray );
LONG FPrintf( BPTR fh, UBYTE *format, long arg1, ... );
void Flush( BPTR fh );
LONG SetVBuf( BPTR fh, UBYTE *buff, long type, long size );
/*      DOS Object Management */
BPTR DupLockFromFH( BPTR fh );
BPTR OpenFromLock( BPTR lock );
BPTR ParentOfFH( BPTR fh );
BOOL ExamineFH( BPTR fh, struct FileInfoBlock *fib );
LONG SetFileDate( UBYTE *name, struct DateStamp *date );
LONG NameFromLock( BPTR lock, UBYTE *buffer, long len );
LONG NameFromFH( BPTR fh, UBYTE *buffer, long len );
WORD SplitName( UBYTE *name, unsigned long seperator, UBYTE *buf, long oldpos,
        long size );
LONG SameLock( BPTR lock1, BPTR lock2 );
LONG SetMode( BPTR fh, long mode );
LONG ExAll( BPTR lock, struct ExAllData *buffer, long size, long data,
        struct ExAllControl *control );
LONG ReadLink( struct MsgPort *port, BPTR lock, UBYTE *path, UBYTE *buffer,
        unsigned long size );
LONG MakeLink( UBYTE *name, long dest, long soft );
LONG ChangeMode( long type, BPTR fh, long newmode );
LONG SetFileSize( BPTR fh, long pos, long mode );
/*      Error Handling */
LONG SetIoErr( long result );
BOOL Fault( long code, UBYTE *header, UBYTE *buffer, long len );
BOOL PrintFault( long code, UBYTE *header );
LONG ErrorReport( long code, long type, unsigned long arg1,
        struct MsgPort *device );
/*      Process Management */
struct CommandLineInterface *Cli( void );
struct Process *CreateNewProc( struct TagItem *tags );
struct Process *CreateNewProcTagList( struct TagItem *tags );
struct Process *CreateNewProcTags( unsigned long tag1type, ... );
LONG RunCommand( BPTR seg, long stack, UBYTE *paramptr, long paramlen );
struct MsgPort *GetConsoleTask( void );
struct MsgPort *SetConsoleTask( struct MsgPort *task );
struct MsgPort *GetFileSysTask( void );
struct MsgPort *SetFileSysTask( struct MsgPort *task );
UBYTE *GetArgStr( void );
BOOL SetArgStr( UBYTE *string );
struct Process *FindCliProc( unsigned long num );
ULONG MaxCli( void );
BOOL SetCurrentDirName( UBYTE *name );
BOOL GetCurrentDirName( UBYTE *buf, long len );
BOOL SetProgramName( UBYTE *name );
BOOL GetProgramName( UBYTE *buf, long len );
BOOL SetPrompt( UBYTE *name );
BOOL GetPrompt( UBYTE *buf, long len );
BPTR SetProgramDir( BPTR lock );
BPTR GetProgramDir( void );
/*      Device List Management */
LONG SystemTagList( UBYTE *command, struct TagItem *tags );
LONG System( UBYTE *command, struct TagItem *tags );
LONG SystemTags( UBYTE *command, unsigned long tag1type, ... );
LONG AssignLock( UBYTE *name, BPTR lock );
BOOL AssignLate( UBYTE *name, UBYTE *path );
BOOL AssignPath( UBYTE *name, UBYTE *path );
BOOL AssignAdd( UBYTE *name, BPTR lock );
LONG RemAssignList( UBYTE *name, BPTR lock );
struct DevProc *GetDeviceProc( UBYTE *name, struct DevProc *dp );
void FreeDeviceProc( struct DevProc *dp );
struct DosList *LockDosList( unsigned long flags );
void UnLockDosList( unsigned long flags );
struct DosList *AttemptLockDosList( unsigned long flags );
BOOL RemDosEntry( struct DosList *dlist );
struct DosList *AddDosEntry( struct DosList *dlist );
struct DosList *FindDosEntry( struct DosList *dlist, UBYTE *name,
        unsigned long flags );
struct DosList *NextDosEntry( struct DosList *dlist, unsigned long flags );
struct DosList *MakeDosEntry( UBYTE *name, long type );
void FreeDosEntry( struct DosList *dlist );
BOOL IsFileSystem( UBYTE *name );
/*      Handler Interface */
BOOL Format( UBYTE *filesystem, UBYTE *volumename, unsigned long dostype );
LONG Relabel( UBYTE *drive, UBYTE *newname );
LONG Inhibit( UBYTE *name, long onoff );
LONG AddBuffers( UBYTE *name, long number );
/*      Date, Time Routines */
LONG CompareDates( struct DateStamp *date1, struct DateStamp *date2 );
LONG DateToStr( struct DateTime *datetime );
LONG StrToDate( struct DateTime *datetime );
/*      Image Management */
BPTR InternalLoadSeg( BPTR fh, BPTR table, LONG *funcarray, LONG *stack );
void InternalUnLoadSeg( BPTR seglist, void (*freefunc)() );
BPTR NewLoadSeg( UBYTE *file, struct TagItem *tags );
BPTR NewLoadSegTagList( UBYTE *file, struct TagItem *tags );
BPTR NewLoadSegTags( UBYTE *file, unsigned long tag1type, ... );
LONG AddSegment( UBYTE *name, BPTR seg, long system );
struct Segment *FindSegment( UBYTE *name, struct Segment *seg, long system );
LONG RemSegment( struct Segment *seg );
/*      Command Support */
LONG CheckSignal( long mask );
struct RDArgs *ReadArgs( UBYTE *template, LONG *array, struct RDArgs *args );
LONG FindArg( UBYTE *keyword, UBYTE *template );
LONG ReadItem( UBYTE *name, long maxchars, struct CSource *cSource );
LONG StrToLong( UBYTE *string, LONG *value );
LONG MatchFirst( UBYTE *pat, struct AnchorPath *anchor );
LONG MatchNext( struct AnchorPath *anchor );
void MatchEnd( struct AnchorPath *anchor );
BOOL ParsePattern( UBYTE *pat, UBYTE *buf, long buflen );
BOOL MatchPattern( UBYTE *pat, UBYTE *str );
/* Not currently implemented. */
void FreeArgs( struct RDArgs *args );
UBYTE *FilePart( UBYTE *path );
UBYTE *PathPart( UBYTE *path );
BOOL AddPart( UBYTE *dirname, UBYTE *filename, unsigned long size );
/*      Notification */
BOOL StartNotify( struct NotifyRequest *notify );
void EndNotify( struct NotifyRequest *notify );
/*      Environment Variable functions */
BOOL SetVar( UBYTE *name, UBYTE *buffer, long size, long flags );
LONG GetVar( UBYTE *name, UBYTE *buffer, long size, long flags );
LONG DeleteVar( UBYTE *name, unsigned long flags );
struct LocalVar *FindVar( UBYTE *name, unsigned long type );
LONG CliInit( struct DosPacket *dp );
LONG CliInitNewcli( struct DosPacket *dp );
LONG CliInitRun( struct DosPacket *dp );
LONG WriteChars( UBYTE *buf, unsigned long buflen );
LONG PutStr( UBYTE *str );
LONG VPrintf( UBYTE *format, LONG *argarray );
LONG Printf( UBYTE *format, long arg1, ... );
/* these were unimplemented until dos 36.147 */
BOOL ParsePatternNoCase( UBYTE *pat, UBYTE *buf, long buflen );
BOOL MatchPatternNoCase( UBYTE *pat, UBYTE *str );
/* this was added for V37 dos, returned 0 before then. */
BOOL SameDevice( BPTR lock1, BPTR lock2 );
/* These were added in dos 36.147 */
/* these were added in dos 37.1 */
/* these were added in dos 37.8 */
#endif   /* CLIB_DOS_PROTOS_H */
