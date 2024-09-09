dcc - the DICE compiler frontend
================================

.. note::

    new this version, use of single precision IEEE libraries for
    floating point.   Please read the note under the option ``-ffp`` for
    more information.

DCC is similar to the UNIX ``cc`` command and is the frontend for the
DICE compiler.  For best performance I strongly suggest that you make
``dcc``, ``dc1``, ``dcpp``, ``das``, and ``dlink`` resident.  If you do not have enough
memory to do so you at least want to leave the largest programs, ``dc1``
and ``das`` resident.   It is suggested that your machine have at least 1MB
of memory.  A compile or link can take as much as 400K of run-time
memory.

Options may occur anywhere on the command line but MUST occur singly.
That is, ``-c -a`` instead of ``-ca``.  file arguments to options may occur
with or without an intervening space.  ``-oFILE`` and ``-o FILE`` are both legal.

Files ending in ``.a`` or ``.asm`` are assumed to be assembly files.  Files ending
in ``.l`` or ``.lib`` are assumed to be library files.  Files ending in ``.o`` or ``.obj`` are
assumed to be object files.  All other files are assumed to be C source
files.

Normally DCC compiles all C source files, assembles all ``.a`` / ``.asm`` files,
and links the resulting object files with any specified ``.o`` / ``.obj`` files
together to produce an executable.  The output file may optionally be
specified with the ``-o`` option.  If not specified, a default output
filename based on the name of the input file is generated.  This
general action is modified by two options:

``-c``
    DCC does NOT link, ``-o`` specifies the output object file

``-a``
    DCC does NOT assemble (i.e. leaves the ``.a`` file resulting from
    a compile).  ``-o`` specifies the output assembly file

If neither option is given ``-o`` specifies the name of the resulting
executable.

The default object directory is ``T:`` and may be changed with the ``-O`` option.
The default temporary directory is also ``T:`` and may be changed with the ``-T``
option.  **If you have limited memory** you may have to specify that
temporary files not be placed in ``T:`` either by re-assigning ``T:`` or using
the ``-T`` option.  DICE goes much slower if temporary files must be written
to a floppy or even a hard disk.

.. warning::

    ``.a`` / ``.asm`` files are assembled with DAS, read DAS.DOC if you
    intend to assemble non-DC1 generated assembly.

Options
-------

file
^^^^

File to compile, assemble (.a), and/or link (.o, .lib)

@file
^^^^^

File containing further list of files, one per line.
(blank lines and lines beginning with ';' or '#' are
ignored.  File may NOT contain further options).

-F file
^^^^^^^

specify stderr file, any errors are appended to the file
instead of to stdout.  Useful for batch compiles

-c
^^

Compile C source files and assemble into OBJECT files
only (do not link).

-a
^^

Compile C source files into ASSEMBLY files (do not assemble
or link).

Keep in mind the DAS will do further optimizations on the
assembly file that you see.

-l0
^^^

Do not link default libraries (``DLIB:c.lib``, ``DLIB:amigas.lib``,
``DLIB:auto.lib``), or standard startup (``DLIB:c.o`` and
``DLIB:x.o``).

.. tip::

    Beginner's note:   Do not use this option

This option is used in special circumstances, such as when
generating .libraries or .devices.

WARNING: DICE is much more dependant on its startup code
(``c.o`` and ``x.o``) than other compilers, do not link without
the startup unless you know what you are doing.

-l lib
^^^^^^

When linking include this library. (space is optional)

Generally ``-l`` is used to include the math library (``-lm``)
when formatted floating point ``Xprintf()`` functions are required.

Refer to the section 'MODELS AND LIBRARIES' and 'CREATING
YOUR OWN LIBRARIES' below for more information on linking
in custom libraries.

-2.0
^^^^

Default amiga.lib is ``DLIB:amigas20.lib``
Default amiga include path is ``DINCLUDE:amiga20/``

-2.x where 'x' is the second digit replacing the '0' in the
above example.  This option is useful when compiling for
different versions of the operating system.

-1.3
^^^^

Like -2.0, but using ``DLIB:amigas13.lib`` and ``DINCLUDE:amiga13/``

Again, -1.x may be specified.

-L0
^^^

remove default library search path, including all explicitly
specified (``-L dir``) directories up to this point.

-L dir
^^^^^^

add the specified directory to the library search path.  If
the object module or library can not be found in the
current directory, directories specified with ``-L`` are
searched.  ``-L`` directories are searched before the default
library directory (``DLIB:``), assuming it was not removed
with ``-L0``.

Note that the directory path specified by ``-L`` is used to
search for libraries AND object modules.

A trailing '/' is optional

-I0
^^^

remove default include path from search list.  The default
include path is ``DINCLUDE:`` and ``DINCLUDE:amiga/`` (unless
modified by ``-1.x`` and ``-2.x`` options)

-I dir
^^^^^^

When compiling scan this include directory (space is
optional) The specified path takes precedence over defaults
but defaults are NOT removed.

-D define[=value]
^^^^^^^^^^^^^^^^^

Pre-define a symbol

-U
^^

undefine ``__STDC__``, ``mc68000``, ``_DCC``, and ``AMIGA``.

.. tip::

    Beginner's note:   Do not use this option

-H<outfile>=<hdrfile>
^^^^^^^^^^^^^^^^^^^^^

This option enables precompiled header file generation and
operation. You may specify any number of -H options.  Example
usage:

``-Ht:defs.m=defs.h``

When DICE encounters an ``#include <defs.h>`` this will cause it to
first check for the existance of ``T:DEFS.M`` ... if ``T:DEFS.M`` does
not exist DICE will generate it from ``defs.h``.  if ``T:DEFS.M``
does exist then DICE will use it directly and ignore ``defs.h``

You must specify the ``-H`` option both to have DICE create the
precompiled header file and to have DICE use the precompiled
header file.  Normally one makes operation as transparent as
possible so as not depend on the option existing when porting
to other enviroments.

.. warning::

    A precompiled header file contains
    the preprocessed header and preprocessor macros.  These are set in
    stone!  If you modify a ``#define`` that would normally effect
    preprocessing of a header file which is precompiled **the effect will
    not occurr**.  It is strongly suggested you use precompiled headers
    **only** with includes that are pretty much unchanging.  For example,
    the commodore includes or otherwise have an appropriate dependancy
    in your DMakefile or make script to delete the precompiled header
    file whenever any of your headers are modified.

Normally one has a single ``-H`` option that enables precompiling of a
local header file, say ``defs.h``, which contains ``#include``'s of all
other header files.  Source modules would then ``#include <defs.h>``

.. tip::

    Beginner's note:   Do not use this option

-o file
^^^^^^^

Specify output executable, object, or assembly file name
depending on what you are producing.  The space is optional

-020, -030, -881, -882
^^^^^^^^^^^^^^^^^^^^^^

+------+-------------------------------------------------------+
| -020 | generate code for the 68020 and later microprocessors |
+------+-------------------------------------------------------+
| -030 | generate code for the 68030 and later microprocessors |
+------+-------------------------------------------------------+
| -881 | generate inline FFP code for the 68881                |
+------+-------------------------------------------------------+
| -882 | generate inline FFP code for the 68882                |
+------+-------------------------------------------------------+

.. tip::

    Beginner's note:    Do not use any of these options

These options exist to produce 020 and 030 opcodes, and 881/882
inline assembly for floating point operations.  They are not
necessarily implemented yet.  The intent is to implement them
by V2.06 .

-md, -mD, -mc, -mC
^^^^^^^^^^^^^^^^^^

+-----+----------------------------+--------------------+
| -md | small data model (default) | uses A4-relative   |
+-----+----------------------------+--------------------+
| -mD | large data model           | uses absolute-long |
+-----+----------------------------+--------------------+
| -mc | small code model (default) | uses PC-relative   |
+-----+----------------------------+--------------------+
| -mC | large code model           | uses absolute-long |
+-----+----------------------------+--------------------+

.. tip::

    Beginner's note:    Use only ``-mD`` if you declare more than
    64KBytes of data.

These options specify the default data and code model to use.
The model may be overriden by use of the ``__near`` and ``__far``
type qualifiers (see EXTENSIONS.DOC) on a variable by variable
basis.

DICE defaults to the small data and small code model, and is able
to generate >32KBytes of code using the small code model so you
should never have to use ``-mC``.  Note that the DICE libraries have
all been compiled with the small-data model, and certain
applications may disrupt the base register, A4... in this case
use of the ``__geta4`` type qualifier should be of use.  If worse
comes to worse you can recompile a large-data model ``C.LIB``, but
I suggest you try other solutions first.

-ms0, -ms1, -ms, -ms2
^^^^^^^^^^^^^^^^^^^^^

+------+-----+-----------------------------------------------------------+
| -ms0 |     | (default), only const objects are put into a CODE hunk    |
+------+-----+-----------------------------------------------------------+
| -ms1 | -ms | string constants are put into the read-only code hunk     |
+------+-----+-----------------------------------------------------------+
| -ms2 |     | string constants are put into the read-only code hunk AND |
|      |     | all external const references use NEAR addressing         |
+------+-----+-----------------------------------------------------------+

.. tip::

    Beginner's note:    Use only ``-ms``

``-ms0`` turns off ``-ms1`` and ``-ms2`` in case you have it in your ``DCCOPTS``
enviroment variable and want to turn it off.

Default operation (no ``-ms1`` or ``-ms2``) puts ``const`` items into a
read-only CODE hunk.  Locally declared objects are referenced using
PC-REL while external objects (declared in some other module) are
referenced using 32-BIT ABSOLUTE addressing.

``-ms`` or ``-ms1`` will additionally make all string constants, such as ``"fubar"``,
const and referenced via PC-REL.  ``-ms`` is an extremely useful option
when you will never modify any of your string constants because the
strings are not copied for multiple running instances of the
program (if resident).

``-ms2`` works like ``-ms1``, but in addition forces all external const
references to use PC-REL addressing INSTEAD of 32-bit absolute
addressing.

.. caution::

    ``-ms2`` is a very dangerous option, do not use unless the
    final code size is less than 32 KBytes.

Using ``-ms`` along with ``-r`` can result in huge savings of memory due to
the string constants being moved out of the data segment (which
must be duplicated for each running instance of the program).

.. warning::

    In all cases if you declare an object as ``const``
    it must be ``extern``'d as ``const`` in other modules or incorrect code
    will be generated.  This is true whether you use ``-ms`` / ``--mS`` or not.

-mr - use registers instead of the stack for function calls
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Enable generation of registered functions and function calls.

When enabled, and as long as the conditions for registerization are met, DICE
will:

* Generate functions that accept their parameters through registers instead of
  the stack.
* Call functions by passing arguments through registers instead of the stack.

See the chapter ":ref:`ch2_using/registered-args:Registered Arguments`"
for full information on registerization.

.. note::

    Previous versions of DICE would also accept ``-mR`` and ``-mRR`` options,
    to alter some behaviour of registerization. These options now behave the
    identically to ``-mr``. They exist for backward compatibility only; please
    use only ``-mr`` instead.

-mi - Inline library calls
^^^^^^^^^^^^^^^^^^^^^^^^^^

Causes DICE to utilize inline library calls instead of library
tags.  DCC will pass ``-D__DICE_INLINE`` to the preprocessor which
causes inline stuff in ``<clib/*_protos.h>`` to be brought in.
Your programs must ``#include`` appropriate prototype headers
such as:

.. code-block:: c

    #include <clib/exec_protos.h>

These are Commodore standard prototype files which DICE postpends
with appropriate inline specifications when the ``-mi`` option is
used.  If the ``-mi`` option is not used the prototype files will
still work as expected but no inline calls will be made.

-mw <addr>
^^^^^^^^^^^

.. note::

    Used for making ROMable executables. Do not use to create
    Amiga executables.

.. tip::

    Beginner's note: Do not use this option

This option is another data model, called the ABSOLUTE-WORD data
model.  Source files compiled with this option generate
absolute-word data references to access data objects instead of
A4-relative or absolute-long.  The base of the data segment must be
specified as decimal, 0octal, or 0xHEX.

Since absolute-word is used exclusive of A4-relative, the compiler
will now use A4 for register variables.  You may NOT mix ``-mw``
modules with small-data models (do NOT use ``c.lib``, see ROMABLE.DOC)

The ``romable`` program is usually run on the executable generated by
DLink to generate a ROM.

-ma <addr>
^^^^^^^^^^

.. note::

    Used for making ROMable executables. Do not use to create
    Amiga executables.

.. tip::

    Beginner's note: Do not use this option

This option specifies to the compiler and linker that the resulting
code is intended to be relocated to a permanent data address, that
specified by <addr> in decimal, 0octal, of 0xHEX.

Unlike ``-mw``, ``-ma`` assumes that the data segment can be placed
anywhere.  The ``romable`` program is usually run on the executable
generated by DLink to generate a ROM.

You may still specify a data model, ``-md`` or ``-mD``, to use with this
option.  Unlike ``-mw``, ``-ma`` does NOT touch the A4 register and thus
may be mixed with the small-data model.  Again, see ROMABLE.DOC

-rom - set up options for generating ROMable code
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. tip::

    Beginner's note: Do not use this option

Like ``-l0``, ``-rom`` disables automatic inclusion of a startup file (you
must specify your own) and libraries.  However, ``x.o`` is still
included to sink any autoinit code.  Your startup code must handle
the appropriate model and call autoinit code before calling your
program main

This option is used to support ROMed firmware, i.e. non-Amiga
executables.  You should never link with ``c.lib``.  Instead, a new
library, ``rom.lib``, is available.

``rom.lib`` contains no static or global references and thus works
with any data model, and only completely self-contained routines
are included.  The only data ``rom.lib`` uses is stack-based.  All
``rom.lib`` routines are completely reentrant, including
``[v]sprintf()`` !

-proto - prototype checking and optimizations
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

When this option is used, an ERROR message will be generated for
any call that is not prototyped.  This option is useful to ensure
that you have properly prototyped routines (when you use
prototypes), especially when floats and doubles are passed and
indirect function pointers are used (they must be prototyped as
well!).

In the future this will enable stack-argument optimization.
Currently, ``char`` and ``short`` values are extended to ``long`` when pushed onto
the stack for a subroutine call.  In the future if the ``-proto``
option is used these objects will be pushed as shorts and not
extended.

-prof, -prof1, -prof2, -prof3 - profiling
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

+--------+-----------------------------------------------------------------------+
| -prof  | enable profiling for source modules                                   |
+--------+-----------------------------------------------------------------------+
| -prof1 | same as ``-prof``                                                     |
+--------+-----------------------------------------------------------------------+
| -prof2 | enable profiling for source modules and ``c*p.lib``                   |
+--------+-----------------------------------------------------------------------+
| -prof3 | enable profiling for source modules, ``c*p.lib``, and ``amiga*p.lib`` |
+--------+-----------------------------------------------------------------------+

Enable profiling.  You may compile some or all your source modules
with profiling enabled.  Any ``-prof*`` option will enable profiling
for compiled source modules.  ``-prof2`` will cause DCC to link
with a profiled ``c*p.lib`` while ``-prof3`` will cause DCC to link
with a profiled ``c*p.lib`` AND ``amiga*p.lib`` (the ultimate).

To profile ``c*.lib`` and/or ``amiga*.lib`` functions the equivalent
``c*p.lib`` and ``amiga*p.lib`` must exist.  These libraries are most
likely lharc'd in ``DCC2:dlib/`` or ``DCC3:dlib/`` but if not, registered
users may create any link library from the library source.

-r, -pr, -pi - make residentable executables
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

+-----+--------------------------------------------------------------+
| -r  | Make executable residentable with separate CODE & DATA hunks |
+-----+--------------------------------------------------------------+
| -pr | Make executable residentable with no relocation hunks        |
+-----+--------------------------------------------------------------+
| -pi | Make executable NON-residentable with no relocation hunks    |
+-----+--------------------------------------------------------------+

.. tip::

    Beginner's note: Just use ``-r`` to get residentable executables and
    do not worry about these other options.

``-pr`` and ``-pi`` generate *position independant* code also useful for ROMed
applications.  NOTE that ``-pi`` and ``-pr`` force const items to be
referenced pc-relative as well, causing ``-ms`` and ``-mS`` to do the same
thing (when combined with ``-pr`` / ``-pi``)

.. note::

    Code size is limited to 32Kbytes when using -pr or -pi

Refer to the section below called 'RESIDENTABILITY' for a
discussion of these options

.. note::

    You may not make data references within const
    declared objects when using the ``-r`` / ``-pr`` options.  This is because
    the CODE hunk is shared between running instances of the program
    and these address references would be different between the
    instances.

    However, if you are using the -ms option, string constants will
    be in the code section and thus no problem.

-O outdir
^^^^^^^^^

Specify directory that is to contain output executable,
object, or assembly files (used when specifying multiple
source files)

``-O`` is useful to tell the compiler where to put the objects
when you use dcc to compile and link a whole bunch of files
at once.  In this case, the ``-o`` option can still be used to
specify where to put the final executable.

.. note::
    The ``-O`` name is used as a prefix so if you are
    specifying a directory be sure it has a ``:`` or ``/`` on
    the end.

-T tmpdir
^^^^^^^^^

Specify the temporary directory used to hold preprocessed
source files and temporary assembly files... files that
will be deleted after use.

.. note::

    
    The ``-T`` name is used as a prefix so if you are
    specifying a directory be sure it has a ``:`` or ``/`` on
    the end.

The default is ``T:`` .. this option is useful in low-memory situations
where you may decide to put intermediate files elsewhere.  Putting
intermediate files on your HD or floppy slows down compilation by
an order of magnitude, but if you are running on a system with
little memory you may not have a choice.

-s
^^

Include symbolic debugging information in the executable.
(dlink opion)

This option includes the symbol table in the resulting executable
and is passed to dlink.  When using ``dobj`` to disassemble an
executable, ``dobj`` will use the symbol table to generate a more
symbolic dump.

-S - alternate section naming op for libraries
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

When making libraries: uses alternate section naming conventions so
that all initialized data in the module will be placed before any
initialized data in non -S modules (i.e. normal linked object
files).  Any library BSS will be placed before non-library BSS.
Thus, the final ordering in the final executable will be:

``[LIBDATA] [PROGRAMDATA] [LIBBSS] [PROGRAMBSS]``

Thus, if your program contains >64K Bytes of BSS you can still
link with a library that tries to reference its own BSS using
the small-data model.  If your library declares only initialized
data (i.e. ``int x = 0;`` ), then you can link with the library even
if your program declares >64KBytes of *initialized* data !

-v
^^

Display commands as DCC executes them.

-new
^^^^

Checks timestamps for source/destination and only
compiles/assembles if object is outdated or does not
exist.  Used to make DCC a standalone make.

-f - fast / ^C handling for 1.3
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

This option is used for 1.3 only.  You MUST be using the
commodore shell (``NewShell``) and if you make programs
resident you MUST use the commodore ``C:Resident`` command.
This option will probably not work if you use WShell or
ARPShell under 1.3.

This option allows DICE to take
short cuts to run sub-programs and allows ^C to be
propogated to said programs.  This option is useful
to set permanently in your ``DCCOPTS`` environment variable if
you run under 1.3

DICE under 2.0 has no such problems and will run sub
programs optimally, including propogation of ^C.

-frag - FRAGment (linker option)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Tell linker not to combine all code hunks together or combine all
data hunks together. Cannot be used if the ``-r`` or ``-mw`` options are
used. Generally only useful if the large-data model is used.  Not
entirely supported yet.

-ffp - Set fp library for floats
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. tip::

    Beginner's note: when using single precision floating point
    this option, use of the original ffp libraries, will make the
    program portable across all amigas.  Otherwise only amigas that
    have the commodore ``mathieeesing*.library`` libraries will be
    able to run the program.

If not specified, ``mathieeesingtrans.library`` and
``mathieeesingbas.library`` are used.  These are new 2.0 libraries
that may not exist on all machines yet.

If specified, ``mathtrans.library`` is used .. motorola's FFP float
library.

.. note::

    If ``-ffp`` is used, be warned that conversion from floats to
    doubles and back again is not entirely reliable.

-d#
^^^

Set debugging level (# = a number), used for compiler
diagnostics only.

-d<opts>
^^^^^^^^

Specify any combination of debugging options.  These options
may be combined in one -d option.

Currently no options are defined.

-gs
^^^

Generate Dynamic Stack Code.  This generates code on every
subroutine call to check available stack.  If available
stack falls below 2K a new stack frame is allocated which
will be deallocated when the subroutine returns.

If the allocation fails, ``stack_abort()`` is called.  If this
routine is not defined by you, the library ``stack_abort()``
will call ``abort()``.  REFER TO DOC/EXTENSIONS.DOC FOR
MORE INFO.

This option is extremely useful when compiling UNIX code
that expects infinite stack.

-chip - CHIP force (linker option)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Tell linker to force all hunks into CHIP memory.  You should
generally not use this option.  Instead, use the ``__chip``
keyword (see EXTENSIONS.DOC) for those specific data items
that need to be in CHIP memory.

NOTE:   CHIP data items are accessed using the large-data
model, thus you cannot create residentable executables that
contain ``__chip`` declarations *unless they are also* ``const``
objects -- read-only.

-CTOD dll_loadable_lib
^^^^^^^^^^^^^^^^^^^^^^

Causes DICE to generate a class (``.d``) file for the D language
with API interfacing declarations for procedures, globals, and
structures.  You must specify the DLL loadable object that
DICE will then create an import for within its generated ``.d`` file.
You generally also need to supply a ``-I`` option to direct DICE
to the C includes, e.g. ``-I /usr/include``, and a C source file
that ``#include`` 's the headers you wish to generate prototypes for.

Note that D includes ``/usr/lib``, ``/usr/local/lib``, and ``/usr/X11R6/lib``    
in its default search path, so it is usually sufficient to simply
specify the ``blah.so`` file, or ``blah.r`` file.

-unix
^^^^^

Causes DICE to use ``DLIB:uc*.lib`` instead of ``DLIB:c*.lib``  ... the
``uc*.lib`` is exactly the same as the normal ``c*.lib`` except that all
filenames are assumed to be UNIX names .. that is, a beginning
slash is converted to ``:`` (root of the current volume), ``./`` is
ignored, and ``../`` is converted to ``/`` for all file accesses.

This makes porting and usage of UNIX programs easier.

-aztec, -lattice, -sas
^^^^^^^^^^^^^^^^^^^^^^

+----------+---------------------------------------------------+
| -aztec   | The front end attempts to run Aztec executables   |
+----------+---------------------------------------------------+
| -lattice | The front end attempts to run Lattice executables |
+----------+---------------------------------------------------+
| -sas     | same as -lattice                                  |
+----------+---------------------------------------------------+

These options allow one to write a single DMakefile able to handle
compilation under any compiler, assuming the source is compilable
under any compiler.

These are very limited options and may not work across new
versions of Aztec or Sas/C

-//
^^^

This option enables C++ style // comments.  This form of commenting
begins with a // causing it and the remainder of the line to be
considered a comment.

-no-env
^^^^^^^

This option disables ``DCCOPTS``.  DCC will not process options in
the ``DCCOPTS`` enviroment variable.

The ``DCCOPTS`` enviroment variable may contain additional options.

``ENV:`` must exist for DCC to run, even if you do not have a ``DCCOPTS``
enviroment variable.  If you do not use ``ENV:`` then assign it to ``RAM:``

Examples
--------

Example #1.  Compile hello.c to executable.  The objects will be
left in T:

.. code-block:: none

    1> dcc hello.c -o ram:hello
    1> ram:hello

Example #2.  Compile hello.c to executable and put the object file
in X:

.. code-block:: none

    1> dcc hello.c -o ram:hello -TX:

Example #3.  Compile hello.c into object into RAM: then link with symbols

.. code-block:: none

    1> dcc -c hello.c -o ram:hello.o
    1> dcc ram:hello.o -o ram:hello -s

Example #4.  Compile foo.c and link with an already compiled object file
gar.o to produce an executable.  foo.o is placed in
T:

.. code-block:: none

    1> dcc foo.c gar.o -o ram:foogar

The preprocessor
----------------

Predefined Symbols:

+-------------------+----------------------------------------------------+
| ``__LINE__``      | current line number (integer constant)             |
+-------------------+----------------------------------------------------+
| ``__DATE__``      | current date (string)                              |
+-------------------+----------------------------------------------------+
| ``__TIME__``      | current time (string)                              |
+-------------------+----------------------------------------------------+
| ``__FILE__``      | current file (string)                              |
+-------------------+----------------------------------------------------+
| ``__BASE_FILE__`` | base source file (string), allows specification    |
|                   | of the actual name of the source file from         |
|                   | within an include file.                            |
+-------------------+----------------------------------------------------+
|                   | (-U undefines the below symbols)                   |
+-------------------+----------------------------------------------------+
| ``__STDC__``      |                                                    |
+-------------------+----------------------------------------------------+
| ``mc68000``       |                                                    |
+-------------------+----------------------------------------------------+
| ``_DCC``          |                                                    |
+-------------------+----------------------------------------------------+
| ``AMIGA``         |                                                    |
+-------------------+----------------------------------------------------+
| ``_FFP_FLOAT``    | set if single prec. floats are in FFP format       |
+-------------------+----------------------------------------------------+
| ``_SP_FLOAT``     | set if single prec. floats are in IEEE-SING format |
|                   | (default)                                          |
+-------------------+----------------------------------------------------+

NOTE:  There are no limits to symbol and macro lengths.

Models and libraries
--------------------

DICE supports various memory models and this is closely coupled with
the link libraries it picks.  There are two major memory models that
effect the link library selection:

+---------+------------------------------+
| -mRR    | registerized procedure calls |
+---------+------------------------------+
| -md/-mD | small data / large data      |
+---------+------------------------------+
| -prof1  | profiled user code           |
+---------+------------------------------+
| -prof2  | profiled c.lib               |
+---------+------------------------------+
| -prof3  | profiled c.lib and amiga.lib |
+---------+------------------------------+

Library naming convention works as follows:

    <libraryname>[srpl].lib

* l - large data model
* s - small data model
* r - registered arguments, else stack arguments
* p - profiled link library, else not profiled

One of `l` or `s` will always be present.

Keeping that in mind, these are the libraries that come with the
registered version of DICE:

* c.lib:

  * cs.lib          small data
  * csr.lib         small data + registered args
  * csp.lib         small data + profiled
  * csrp.lib        small data + registered args + profiled

* m.lib:

  * ms.lib          small data
  * msr.lib         small data + registered args
  * msp.lib         small data + profiled
  * msrp.lib        small data + registered args + profiled

* amiga.lib:

  * amigal13.lib    large data amiga.lib for 1.3 (commodore 1.3 amiga.lib)
  * amigas13.lib    small data amiga.lib for 1.3
  * amigasp13.lib   small data profiled amiga.lib for 1.3
  * amigal20.lib    large data amiga.lib for 2.0 (commodore 2.0 amiga.lib)
  * amigas20.lib    small data amiga.lib for 2.0
  * amigasp20.lib   small data profiled amiga.lib for 2.0
  * amigasr20.lib   small data registered-args amiga.lib for 2.0
  * amigasrp20.lib  small data registered-args + profiled amiga.lib for 2.0

Registered users can generate any combination of models they wish by
compiling the library source code.

DICE defaults to the small data model ('s' libraries).  Generally you
will never use the large-data model so these are not included in the
distribution (i.e. they have to be made).  Since there are a huge
number of model combinations possible the distribution itself contains
only those that are likely to be used --- to whit, small data model
libraries.  DICE has enough features in it that you should never have
to use the large-data model.

The large-data model suffix 'l' is new.  This was added to ensure that
all combinations of models generate at least a one character suffix.
This allows DICE to give the linker hints about where to find user
libraries --- if the linker cannot find a library name unadorned it
will append the suffix and try again.  Previously, the large-data model
had no suffix so this method would not work (the large data model
library would be found even though the small-data model library is what
we wanted).

Creating your own libraries
---------------------------

It is extremely easy to write your own link library.  There are two
ways to do it.  First, simply compile various source modules into
objects and ``JOIN`` them together into a ``.lib``.  Second, use the ``libmake``
utility (see LIBMAKE.DOC).  You should name the library according to
the data model, registered args, and profiling capability you
compiled the source code for.

For example, if you have compiled your library modules small-data
without registered arguments and no profiling and want to call your
library 'foo', you should append an 's' suffix to the library name
so the actual library file would be called ``foos.lib``.
If you compiled the library modules small-data with registered args it
would be called ``foosr.lib``.  If you compiled it small-data with normal
args and profiling it would be called ``foosp.lib``, etc... the ordering
of suffix characters is always reverse alphabetical.

When you finally specify your library linking some program with DCC you
have two choices:   (1) you can specify the full library name such as
``-lfoosr``, or (2) you can specify only the base name and let DICE pick
the suffixes for you: ``-lfoo``.  The latter is the most portable, DICE
will choose the suffixes according to the various options you give DCC
when linking.

.. warning::

    option (2), just specifying the base name, assumes that you do
    not have a library called ``foo.lib`` .. i.e. all your foo libraries have
    at least one suffix character.

Putting const data into the code segment
----------------------------------------

Two options are available to handle the ``const`` keyword: ``-ms`` and ``-mS``.  If
neither are used ``const`` is ignored.  The difference between the two
determines how external references (between modules) are handled.

With ``-ms``, all string constants ``"fubar"`` are placed into the code segment
and referenced according to the code model (pc-rel or absolute).  Any
variable declared ``const`` will also be put in the code segment and
referenced similary.  However, any ``extern`` variables referenced but
not defined in this module (i.e. defined in another module) will be
accessed using ABSOLUTE references, since you cannot make a jump table
for data variables.

The ``-mS`` option is much more dangerous.. it works the same as ``-ms`` except
that external references use pc-relative addressing as well. your final
executable **must be less than 32KBytes to ensure all code is reachable
via PC-relative addressing**.  If you have a choice, please use ``-ms``
instead of ``-mS``.

``const __chip`` variables will also be placed in the code section, but
these always use absolute references since the code section is not
combined together with other code sections.  You may also override
default access method (pc-rel/absolute) by using the ``__near`` and ``__far``
keywords, but this is not advised.

The ``-ms`` option affects the main compiler DC1 and can be specified
independently for each source module (i.e. some can have it while
others do not, though normally all will have it one way or the other)

When you use ``-pr`` OR ``-pi``, use of the ``-ms`` option is silently forced to
``-mS``.

Residentability
---------------

Two options are available to generate residentable executables and one
option is available to genreate a non-residentable executable with no
relocation information (loads quickly).

+-----+------+--------+---------+---------+--------+-----------------------------+
|     | RES? | RELOC? | DataLim | CodeLim | Alloc? | Hunks                       |
+=====+======+========+=========+=========+========+=============================+
| -md | no   | yes    | 64K(1)  | none    | no     | DATA, CODE                  |
+-----+------+--------+---------+---------+--------+-----------------------------+
| -mD | no   | yes    | none    | none    | no     | DATA, CODE, optionally more |
+-----+------+--------+---------+---------+--------+-----------------------------+
| -r  | yes  | yes    | 64K     | none    | yes    | DATA, CODE                  |
+-----+------+--------+---------+---------+--------+-----------------------------+
| -pr | yes  | no     | 64K     | 32K     | yes    | CODE (combined DATA+CODE)   |
+-----+------+--------+---------+---------+--------+-----------------------------+
| -pi | no   | no     | 64K     | 32K     | no     | CODE (combined DATA+CODE)   |
+-----+------+--------+---------+---------+--------+-----------------------------+

Key:

RES:
    Can this program be made resident (dlink sets Pure bit)

RELOC:
    Will this executable contain 32-bit relocations?

DATALIM:
    The maximum amount of DATA+BSS that may be declared

    (1) it is possible to declare more then 64KB worth of
    data when using the small-data model by selectively
    making large declarations __far.

CODELIM:
    The maximum size of the CODE

ALLOC:      
    Does the startup code allocate a run time DATA+BSS segment?
    (basically, resident programs must, non-resident programs
    do not)

HUNKS:
    Number and type of hunks that may exist in the executable

``-md`` is the default.

``-mD`` is used when final executable contains more than 64KBytes of data.

``-r`` is the most often used option.

``-pr`` is used instead of ``-r`` if there is only a little initialized data.
Can be used with most programs but requires one large chunk of
memory instead of the two smaller chunks that -r allocates.
Also used in ROMable applications

``-pi`` is used instead of default when there is only a little INITIALIZED+BSS.
Can be used with many small programs that you do not intend to
make residentable, to make them load extremely fast (since there
is no relocation information).
Also used in ROMable applications

The ``__geta4`` type qualifier will NOT work with residentable programs.

You cannot use ``__far`` when accessing non-``const`` (see ``-ms`` option) data
in residentable programs

Using the ``-ms`` option can drastically reduce the amount of data that
needs to be allocated/copied when a resident program starts up by
moving string constants into the code section (only the data section is
duplicated), reducing overhead in both time and space for each
invocation of the RESIDENT'd program.

To use one of the options ``-r``, ``-pi``, or ``-pr`` specify on the DCC line when
both compiling and linking your code.  The resulting executable will have
the pure bit set (for ``-r`` and ``-pr`` options) and be residentable.

``-r`` and ``-pr`` generate no BSS since the startup code is going to allocate
the DATA+BSS space. ``-pi`` generates BSS space in the code segment just
after the initialized data.

Using ``-pi`` switches out the ``c.o`` library for ``c_pi.o``. Likewise.
Using ``-pr`` switches out the ``c.o`` library for ``c_pr.o``.
