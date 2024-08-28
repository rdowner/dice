dlink - linker
==============

DLINK <options> <files> <libraries>

Options may occur anywhere on the command lines.  Any file ending in
``.o`` or ``.obj`` is assumed to be an object file.  Any file ending in ``.l`` or ``.lib`` is
assumed to be a library.  Any file name beginning with ``@`` specifies a
text file containing a further list of files.

File ordering is maintained.  Section ordering is maintained.  All
sections of the same name are coagulated together with ordering
maintained.  **However,** inter-section ordering is not maintained
within a library since library modules are random included.
Ordering *is* maintained *between* libraries.

All object files specified are included in the final executable.  All
libraries specified are searched at the point they are specified (that is,
specifying an object file that references a symbol defined in a library
specified *before* the object file will cause an undefined symbol error).
Normally an object file is specified after a library to terminate an
``autoinit`` or ``autoexit`` section.

You do not have to order object files within a library - DLink will
automatically make as many passes as required to handle all internal
library references.  However, ordering object files will make DLink go
faster.

Symbols defined in object files overide symbols defined in libraries.
Symbols defined in libraries specified before later libraries overide
symbols defined in later libraries.  Symbols defined in a library and
also defined in a later specified object module causes an error.


Creating a library
------------------

DLink libraries are standard Amiga libraries - join one or more
object modules together and name the result with a ``.lib`` extension.


Linker symbols and A4-relative addressing
-----------------------------------------

DLink generates the following special symbols to aid in program
startup:

``__ABSOLUTE_BAS``
                base of data in volatile space.  This symbol is NOT
                defined for normal residentable programs since the
                base address is not known (must be allocated run-time)

``__DATA_BAS``
                base of data in non-volatile space.  This symbol points
                to a read-only copy of the initialized data for a
                program.
                
                For non-residentable programs this is the
                same as ``__ABSOLUTE_BAS``.

                For residentable programs, this points to a read-only
                copy of the initialized data that the program can
                duplicate on startup.

                For programs linked with an absolute base address for
                data, this points to the end of the CODE section.  The
                ``romable`` program always generates a ROM copy of the
                initialized data just after the CODE section, which
                startup code must copy into RAM.

``__DATA_LEN``
                Length of data space in *longwords*. ``__DATA_LEN * 4``
                yields the number of bytes of initialized data.  This
                is used by startup code to copy read-only initialized
                data to volatile space (residentable and data-absolute
                programs)

``__BSS_LEN``
                Length of bss space in *longwords*. ``__BSS_LEN * 4``
                yields the number of bytes of uninitialized (BSS) data.
                This is used in combination with ``__DATA_LEN`` to allocate
                the DATA and BSS space for residentable programs, and clear
                the BSS space for non-residentable and absolute-data-base
                programs.

                The BSS space occurs after the DATA space unless the
                ``-frag`` option is used.

``__RESIDENT``
                This symbol is set to 0 if the ``-r`` option was not used and
                1 if the ``-r`` option was used.  If set to 1 (``-r``
                option), then *no BSS space is preallocated when the
                program is run*.  The startup code must allocate DATA and BSS
                space, copy read-only initialized data over to this
                space, and clear out the BSS portion before it the
                program can run.

                Programs linked with the ``-mw`` or ``-ma`` options
                do not 'allocate' their data space since it is
                predefined.  Most Amiga programmers will never use
                the ``-mw`` or ``-ma`` options, by the way.


Small data model
----------------

The small data model uses A4 relative addressing.  The linker sets up
all relative offsets such that A4 must be initialized by startup code
the ``BaseOfInitializedData + 32766`` for A4-relative references to access
the appropriate address.


Resident
--------

If the ``-r`` options is given, no BSS space is allocated after the
data space. The startup code *must* allocate data and BSS space as shown
above.  DLink will give error messages for any absolute data references
that occur (except the ``__DATA_BAS`` symbol which must be used to copy the
static data to the newly allocated data and BSS memory on program startup).

DLink will give an error message if any data-data ``reloc32`` is encountered when
you specify the ``-r`` option as such relocations would be incorrect when
copied to the newly allocated data and BSS space.  DC1 understands this and
will produce ``autoinit`` code to handle any such static data relocations
that occur in your C code when the ``-r`` option is given to compile a C
program.

However, DLink does allow data-data relocations to occur if an absolute
data base is specified along with the ``-r`` option.  This is used only
when making ROMABLE code.


PC-relative
-----------

Because the linker will insert a jump table for PC-relative references
to different hunks (after coagulation) or where the range is larger
than +/-32K, data should not be placed into a code segment and be
referenced via an external label(pc) unless you are positive the
reference is within +/-32K.  This can only happen when referencing
between like-named code hunks.

.. note::

    The jump table is tagged
    onto the end of the section the jump(s) occur in and thus you do
    not want to have any ``autoinit``/``autoexit`` code that might possibly
    generate a jump table, since the whole idea with ``autoinit`` is that
    the code falls through to other ``autoinit`` code until the terminating
    section in ``x.o``'s RTS.

Currently dlink cannot handle inter-module PC-relative references
beyond +/-32K (i.e. when one object file has more than 32K of code).
An error will occur.

Note that if ``-frag`` is used you cannot make PC-relative calls
between sections of differing names ever, or make a program resident.
The ``-frag`` option is almost never used on untested.


Overlays
--------

Overlays are not supported as yet.

Options
-------

Options:

``-o execname``
    name of executable

``-s``
    Include symbolic information.

.. warning::

    If ``-r`` is used, symbolic info for the data sections will point to
    the statically initialized data, *not* the actual data
    space in BSS referenced by the code.  This is a bug.

``-frag``
    Fragment output file. Without this option, dlink will coagulate all
    hunks of the same type regardless of name.  If
    ``--frag`` is specified then only hunks of the same type
    and name are coagulated.

    See fragmentation note below.

``-r`` | ``-res``
    Resident link.

``-pi``
    Position independant non-residentable (i.e. only
    one copy of the data but also no relocation hunks)

``-pr``
    residentable position independant

``-P<postfix>``
    specify library name postfix.  If DLink cannot find
    the library as specified it will append the postfix
    and try again.  Used by DCC to specify the memory
    model.

    This allows you to specify ``-lc -Ps`` and DLink will
    automatically look for ``cs.lib``. You can specify a
    postfix that occurs before the ``.lib`` in the
    library name here.  If DLink cannot find the
    library as it is named by default it will try
    it with the postfix.

    DCC uses this to supply the memory model to DLINK
    also allowing the user to say -``lm`` in DCC and have
    it find ``MS.LIB`` if you are using the small-data
    model.

``-mw addr``
    specify absolute data base

``-ma addr``
    specify absolute data base

    Both options do exactly the same thing and are in
    duplicate to conform to DCC's options.

    ``-mw``/``-ma`` specify the base of data as a decimal,
    0octal, or 0xHEX address.  You must use the ``-r``
    option in conjunction with these options.

    DLink will resolve all absolute-word addresses
    but not all absolute-long addresses.  This is left
    up to the ROMABLE program which generates a raw
    binary image of the program that can then be
    transfered to an EPROM.

.. warning::

    Do not use when generating Amiga executables.

``-d[#]``
    debug mode (spews lots of debugging junk out)

``-F file``
    specify stderr file, any errors are appended to the file
    instead of to stdout.  Useful for batch compiles

``-chip``
    chip-only - forces all hunks into CHIP memory

``-L0``
    remove default library search path, including all
    explicitly specified (``-L dir``) directories up to
    this point.

``-L dir``
    add the specified directory to the library search
    path.  If the object module or library can not be
    found in the current directory, directories
    specified with ``-L`` are searched.  ``-L`` directories are
    searched before the default library directory
    (``DLIB:``), assuming it was not removed with ``-L0``.

    Note that the directory path specified by ``-L`` is used to
    search for libraries AND object modules.

    A trailing '/' is optional.

.. note::
    When ``-frag`` is specified, the linker will not create a special
    combined data+BSS hunk (so data and BSS can both be referenced with
    one base variable).

    However, when ``-frag`` is NOT specified, the linker will still not
    necessarily combine all data hunks into one big hunk and all BSS
    hunks into one big hunk.  Any data or BSS hunk with special upper
    bits set (e.g. to force it into chip) is not combined into these
    special hunks, and any data or BSS hunk whos NAME begins with
    ``far`` (upper or lower case) will also not be considered.


Examples
--------

This is what DCC gives the linker to link the program 'foo.c' :

.. code-block::

    dlink dlib:c.o @tmp dlib:x.o -o ram:foo

Contents of file @tmp:

.. code-block::

    foo.o
    dlib:c.lib dlib:amiga.lib dlib:auto.lib

Basically it tells dlink to link the startup code, ``c.o``, then the
program object module(s) (``foo.o``), then ``c.lib``, ``amiga.lib``, and ``auto.lib``,
then finally ``x.o``.

DCC handles all this for you

``auto.lib`` contains ``autoinit`` code for certain selected libraries,
including the ``dos.library``.  ``autoinit`` code is brought in whenever a
given library base symbol has been referenced but not defined. ``auto.lib``
defines the symbol and generates ``autoinit`` code to open the library and
``autoexit`` code to close the library.  To maintain portability you
probably do not want to use this automatic library-openning feature
yourself, it is really meant to support certain actions of the DICE
library (such as floating point support).

``x.o`` terminates the ``autoinit`` and ``autoexit`` sections with an ``rts`` instruction. The
``autoinit`` and ``autoexit`` sections are called from the startup code ``c.o``.
