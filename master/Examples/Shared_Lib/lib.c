
/*
 *  LIB.C
 *
 *  Basic Library Resource Handling
 *
 *  NOTE: all data declarations should be initialized since we skip
 *        normal C startup code (unless initial value is don't care)
 *
 *  WARNING: arguments are passed in certain registers from the assembly
 *        tag file, matched to how they are declared below.  Do not change
 *        the argument declarations!
 */

#include "defs.h"

Prototype LibCall Library *LibInit(long);
Prototype LibCall Library *LibOpen(long, Library *);
Prototype LibCall long LibClose(long, Library *);
Prototype LibCall long LibExpunge(long, Library *);


Library *LibBase = NULL;        /*  Library Base pointer    */
long    SegList  = 0;
Library *SysBase  = NULL;       /*  EXEC calls              */
struct DosLibrary *DOSBase  = NULL;     /*  if we used it ...       */

/*
 *    The Initialization routine is given only a seglist pointer.  Since
 *    we are NOT AUTOINIT we must construct and add the library ourselves
 *    and return either NULL or the library pointer.  Exec has Forbid()
 *    for us during the call.
 *
 *    If you have an extended library structure you must specify the size
 *    of the extended structure in MakeLibrary().
 */

LibCall Library *
LibInit(segment)
long segment;
{
    Library *lib;
    static const long Vectors[] = {
        (long)ALibOpen,
        (long)ALibClose,
        (long)ALibExpunge,
        (long)ALibReserved,

        (long)LockTestLib,      /*  library dependant   */
        (long)UnLockTestLib,
        (long)PostString,
        (long)GetString,

        -1
    };
    SysBase = *(Library **)4;
    DOSBase = OpenLibrary("dos.library", 0);

    LibBase = lib = MakeLibrary((APTR)Vectors,NULL,NULL,sizeof(Library),NULL);
    lib->lib_Node.ln_Type = NT_LIBRARY;
    lib->lib_Node.ln_Name = LibName;
    lib->lib_Flags = LIBF_CHANGED|LIBF_SUMUSED;
    lib->lib_Version  = 36;
    lib->lib_Revision = 1;
    lib->lib_IdString = (APTR)LibId;
    SegList = segment;
    AddLibrary(lib);

    InitC();

    return(lib);
}

/*
 *    Open is given the library pointer and the version request.  Either
 *    return the library pointer or NULL.  Remove the DELAYED-EXPUNGE flag.
 *    Exec has Forbid() for us during the call.
 */

LibCall Library *
LibOpen(version, lib)
Library *lib;
long version;
{
    ++lib->lib_OpenCnt;
    lib->lib_Flags &= ~LIBF_DELEXP;
    return(lib);
}

/*
 *    Close is given the library pointer and the version request.  Be sure
 *    not to decrement the open count if already zero.  If the open count
 *    is or becomes zero AND there is a LIBF_DELEXP, we expunge the library
 *    and return the seglist.  Otherwise we return NULL.
 *
 *    Note that this routine never sets LIBF_DELEXP on its own.
 *
 *    Exec has Forbid() for us during the call.
 */

LibCall long
LibClose(dummy, lib)
long dummy;
Library *lib;
{
    if (lib->lib_OpenCnt && --lib->lib_OpenCnt)
        return(NULL);
    if (lib->lib_Flags & LIBF_DELEXP)
        return(LibExpunge(0, lib));
    return(NULL);
}

/*
 *    We expunge the library and return the Seglist ONLY if the open count
 *    is zero.  If the open count is not zero we set the DELAYED-EXPUNGE
 *    flag and return NULL.
 *
 *    Exec has Forbid() for us during the call.  NOTE ALSO that Expunge
 *    might be called from the memory allocator and thus we CANNOT DO A
 *    Wait() or otherwise take a long time to complete (straight from RKM).
 *
 *    Apparently RemLibrary(lib) calls our expunge routine and would
 *    therefore freeze if we called it ourselves.  As far as I can tell
 *    from RKM, LibExpunge(lib) must remove the library itself as shown
 *    below.
 */

LibCall long
LibExpunge(dummy, lib)
long dummy;
Library *lib;
{
    if (lib->lib_OpenCnt) {
        lib->lib_Flags |= LIBF_DELEXP;
        return(NULL);
    }
    Remove(&lib->lib_Node);
    FreeMem((char *)lib-lib->lib_NegSize, lib->lib_NegSize+lib->lib_PosSize);
    if (DOSBase) {
        CloseLibrary((Library *)DOSBase);
        DOSBase = NULL;
    }
    UnInitC();
    return((long)SegList);
}

