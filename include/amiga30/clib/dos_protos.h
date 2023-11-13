#ifndef  CLIB_DOS_PROTOS_H
#define  CLIB_DOS_PROTOS_H
/*
**      $VER: dos_protos.h 36.29 (31.05.92)
**      Includes Release 39.108
**
**      C prototypes. For use with 32 bit integers only.
**
**      (C) Copyright 1990-1992 Commodore-Amiga, Inc.
**          All Rights Reserved
*/
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
BPTR Open( STRPTR name, long accessMode );
LONG Close( BPTR file );
LONG Read( BPTR file, APTR buffer, long length );
LONG Write( BPTR file, APTR buffer, long length );
BPTR Input( void );
BPTR Output( void );
LONG Seek( BPTR file, long position, long offset );
LONG DeleteFile( STRPTR name );
LONG Rename( STRPTR oldName, STRPTR newName );
BPTR Lock( STRPTR name, long type );
void UnLock( BPTR lock );
BPTR DupLock( BPTR lock );
LONG Examine( BPTR lock, struct FileInfoBlock *fileInfoBlock );
LONG ExNext( BPTR lock, struct FileInfoBlock *fileInfoBlock );
LONG Info( BPTR lock, struct InfoData *parameterBlock );
BPTR CreateDir( STRPTR name );
BPTR CurrentDir( BPTR lock );
LONG IoErr( void );
struct MsgPort *CreateProc( STRPTR name, long pri, BPTR segList,
        long stackSize );
void Exit( long returnCode );
BPTR LoadSeg( STRPTR name );
void UnLoadSeg( BPTR seglist );
struct MsgPort *DeviceProc( STRPTR name );
LONG SetComment( STRPTR name, STRPTR comment );
LONG SetProtection( STRPTR name, long protect );
struct DateStamp *DateStamp( struct DateStamp *date );
void Delay( long timeout );
LONG WaitForChar( BPTR file, long timeout );
BPTR ParentDir( BPTR lock );
LONG IsInteractive( BPTR file );
LONG Execute( STRPTR string, BPTR file, BPTR file2 );
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
LONG FPutC( BPTR fh, long ch );
LONG UnGetC( BPTR fh, long character );
LONG FRead( BPTR fh, APTR block, unsigned long blocklen,
        unsigned long number );
LONG FWrite( BPTR fh, APTR block, unsigned long blocklen,
        unsigned long number );
STRPTR FGets( BPTR fh, STRPTR buf, unsigned long buflen );
LONG FPuts( BPTR fh, STRPTR str );
void VFWritef( BPTR fh, STRPTR format, LONG *argarray );
void FWritef( BPTR fh, STRPTR format, ... );
LONG VFPrintf( BPTR fh, STRPTR format, LONG *argarray );
LONG FPrintf( BPTR fh, STRPTR format, ... );
LONG Flush( BPTR fh );
LONG SetVBuf( BPTR fh, STRPTR buff, long type, long size );
/*      DOS Object Management */
BPTR DupLockFromFH( BPTR fh );
BPTR OpenFromLock( BPTR lock );
BPTR ParentOfFH( BPTR fh );
BOOL ExamineFH( BPTR fh, struct FileInfoBlock *fib );
LONG SetFileDate( STRPTR name, struct DateStamp *date );
LONG NameFromLock( BPTR lock, STRPTR buffer, long len );
LONG NameFromFH( BPTR fh, STRPTR buffer, long len );
WORD SplitName( STRPTR name, unsigned long seperator, STRPTR buf, long oldpos,
        long size );
LONG SameLock( BPTR lock1, BPTR lock2 );
LONG SetMode( BPTR fh, long mode );
LONG ExAll( BPTR lock, struct ExAllData *buffer, long size, long data,
        struct ExAllControl *control );
LONG ReadLink( struct MsgPort *port, BPTR lock, STRPTR path, STRPTR buffer,
        unsigned long size );
LONG MakeLink( STRPTR name, long dest, long soft );
LONG ChangeMode( long type, BPTR fh, long newmode );
LONG SetFileSize( BPTR fh, long pos, long mode );
/*      Error Handling */
LONG SetIoErr( long result );
BOOL Fault( long code, STRPTR header, STRPTR buffer, long len );
BOOL PrintFault( long code, STRPTR header );
LONG ErrorReport( long code, long type, unsigned long arg1,
        struct MsgPort *device );
/*      Process Management */
struct CommandLineInterface *Cli( void );
struct Process *CreateNewProc( struct TagItem *tags );
struct Process *CreateNewProcTagList( struct TagItem *tags );
struct Process *CreateNewProcTags( unsigned long tag1type, ... );
LONG RunCommand( BPTR seg, long stack, STRPTR paramptr, long paramlen );
struct MsgPort *GetConsoleTask( void );
struct MsgPort *SetConsoleTask( struct MsgPort *task );
struct MsgPort *GetFileSysTask( void );
struct MsgPort *SetFileSysTask( struct MsgPort *task );
STRPTR GetArgStr( void );
BOOL SetArgStr( STRPTR string );
struct Process *FindCliProc( unsigned long num );
ULONG MaxCli( void );
BOOL SetCurrentDirName( STRPTR name );
BOOL GetCurrentDirName( STRPTR buf, long len );
BOOL SetProgramName( STRPTR name );
BOOL GetProgramName( STRPTR buf, long len );
BOOL SetPrompt( STRPTR name );
BOOL GetPrompt( STRPTR buf, long len );
BPTR SetProgramDir( BPTR lock );
BPTR GetProgramDir( void );
/*      Device List Management */
LONG SystemTagList( STRPTR command, struct TagItem *tags );
LONG System( STRPTR command, struct TagItem *tags );
LONG SystemTags( STRPTR command, unsigned long tag1type, ... );
LONG AssignLock( STRPTR name, BPTR lock );
BOOL AssignLate( STRPTR name, STRPTR path );
BOOL AssignPath( STRPTR name, STRPTR path );
BOOL AssignAdd( STRPTR name, BPTR lock );
LONG RemAssignList( STRPTR name, BPTR lock );
struct DevProc *GetDeviceProc( STRPTR name, struct DevProc *dp );
void FreeDeviceProc( struct DevProc *dp );
struct DosList *LockDosList( unsigned long flags );
void UnLockDosList( unsigned long flags );
struct DosList *AttemptLockDosList( unsigned long flags );
BOOL RemDosEntry( struct DosList *dlist );
LONG AddDosEntry( struct DosList *dlist );
struct DosList *FindDosEntry( struct DosList *dlist, STRPTR name,
        unsigned long flags );
struct DosList *NextDosEntry( struct DosList *dlist, unsigned long flags );
struct DosList *MakeDosEntry( STRPTR name, long type );
void FreeDosEntry( struct DosList *dlist );
BOOL IsFileSystem( STRPTR name );
/*      Handler Interface */
BOOL Format( STRPTR filesystem, STRPTR volumename, unsigned long dostype );
LONG Relabel( STRPTR drive, STRPTR newname );
LONG Inhibit( STRPTR name, long onoff );
LONG AddBuffers( STRPTR name, long number );
/*      Date, Time Routines */
LONG CompareDates( struct DateStamp *date1, struct DateStamp *date2 );
LONG DateToStr( struct DateTime *datetime );
LONG StrToDate( struct DateTime *datetime );
/*      Image Management */
BPTR InternalLoadSeg( BPTR fh, BPTR table, LONG *funcarray, LONG *stack );
BOOL InternalUnLoadSeg( BPTR seglist, void (*freefunc)() );
BPTR NewLoadSeg( STRPTR file, struct TagItem *tags );
BPTR NewLoadSegTagList( STRPTR file, struct TagItem *tags );
BPTR NewLoadSegTags( STRPTR file, unsigned long tag1type, ... );
LONG AddSegment( STRPTR name, BPTR seg, long system );
struct Segment *FindSegment( STRPTR name, struct Segment *seg, long system );
LONG RemSegment( struct Segment *seg );
/*      Command Support */
LONG CheckSignal( long mask );
struct RDArgs *ReadArgs( STRPTR template, LONG *array, struct RDArgs *args );
LONG FindArg( STRPTR keyword, STRPTR template );
LONG ReadItem( STRPTR name, long maxchars, struct CSource *cSource );
LONG StrToLong( STRPTR string, LONG *value );
LONG MatchFirst( STRPTR pat, struct AnchorPath *anchor );
LONG MatchNext( struct AnchorPath *anchor );
void MatchEnd( struct AnchorPath *anchor );
LONG ParsePattern( STRPTR pat, STRPTR buf, long buflen );
BOOL MatchPattern( STRPTR pat, STRPTR str );
void FreeArgs( struct RDArgs *args );
STRPTR FilePart( STRPTR path );
STRPTR PathPart( STRPTR path );
BOOL AddPart( STRPTR dirname, STRPTR filename, unsigned long size );
/*      Notification */
BOOL StartNotify( struct NotifyRequest *notify );
void EndNotify( struct NotifyRequest *notify );
/*      Environment Variable functions */
BOOL SetVar( STRPTR name, STRPTR buffer, long size, long flags );
LONG GetVar( STRPTR name, STRPTR buffer, long size, long flags );
LONG DeleteVar( STRPTR name, unsigned long flags );
struct LocalVar *FindVar( STRPTR name, unsigned long type );
LONG CliInitNewcli( struct DosPacket *dp );
LONG CliInitRun( struct DosPacket *dp );
LONG WriteChars( STRPTR buf, unsigned long buflen );
LONG PutStr( STRPTR str );
LONG VPrintf( STRPTR format, LONG *argarray );
LONG Printf( STRPTR format, ... );
/* these were unimplemented until dos 36.147 */
LONG ParsePatternNoCase( STRPTR pat, STRPTR buf, long buflen );
BOOL MatchPatternNoCase( STRPTR pat, STRPTR str );
/* this was added for V37 dos, returned 0 before then. */
BOOL SameDevice( BPTR lock1, BPTR lock2 );

/* NOTE: the following entries did NOT exist before ks 36.303 (2.02) */
/* If you are going to use them, open dos.library with version 37 */

/* These calls were added for V39 dos: */
void ExAllEnd( BPTR lock, struct ExAllData *buffer, long size, long data,
        struct ExAllControl *control );
BOOL SetOwner( STRPTR name, long owner_info );
#endif   /* CLIB_DOS_PROTOS_H */
