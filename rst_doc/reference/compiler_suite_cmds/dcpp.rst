dcpp - the C Preprocessor
=========================

.. code-block:: text

    dcpp sourcefile [-o outfile] [-I includedir ...] <options>

``dcpp`` automatically scans ``DINCLUDE:``, ``DINCLUDE:pd/``, and ``DINCLUDE:amiga/``.
Any ``-I`` option directories are searched in sequence *before* ``dcpp`` 's
default search path.  The last default directory, ``DINCLUDE:amiga/``, may
be modified with the ``-1.x``, ``-2.x`` and ``3.x`` options, see below.

Note that ``DINCLUDE:pd/`` (added after version X.11) is meant to be a
place to put public domain header files so as not to clutter the
top level ``DINCLUDE:`` directory.

As with all DICE commands, the space between the option and an
associated file/dir argument is optional.

``dcpp`` probably does not implement everything perfectly and might fail on
some constructions.

``dcc`` normally runs ``dcpp`` before ``dc1``.

The following symbols are defined by default

+--------------+--------------------+
| ``mc68000``  | running on a 68000 |
+--------------+--------------------+
| ``_DCC``     | 'DCC' compiler     |
+--------------+--------------------+
| ``__STDC__`` | ANSI __STDC__      |
+--------------+--------------------+
| ``AMIGA``    | AMIGA computer     |
+--------------+--------------------+


Options
-------

-1.N, -2.N, -3.N
^^^^^^^^^^^^^^^^

This option selects the OS.  If not specified, ``dcpp`` searches
``DINCLUDE:amiga`` for amiga includes.  If specified, ``dcpp`` searches
``DINCLUDE:amiga1N``, ``DINCLUDE:amiga2N`` or ``DINCLUDE:amiga3N`` for amiga includes
instead.

``dcc`` supports this option and passes it along to ``dcpp``.  This
allows developers to compile under either 1.3 or 2.0 (or
whatever) with the flick of an option.  ``dcc`` also uses a
different ``amiga.lib`` (see DCC docs).

-d[#]
^^^^^

This option turns on ``dcpp`` debugging

-o[ ]file
^^^^^^^^^

This option sets the output file, otherwise stdout is used.
the space is optional

-ffp
^^^^

Passed from DCC, tells preprocessor to define _FFP_FLOAT.
If not specified, preprocessor defines _SP_FLOAT.  This
exists to better support alternate floating point models
in header files.

-D[ ]define[=stuff]
^^^^^^^^^^^^^^^^^^^

This option predefines a symbol or macro.

-F file
^^^^^^^

specify stderr file, any errors are appended to the file
instead of to stdout.  Useful for batch compiles

-U
^^

This option undefines the following symbols:

* ``__STDC__``
* ``mc68000``
* ``_DCC``
* ``AMIGA``

-H<precompoutfile>=<headerfile>
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Enable use/creation of precompiled header files.  Example:

.. code-block:: text

    -Ht:defs.m=defs.h

see DCC.DOC for more information.

-I0
^^^

This option causes ``dcpp`` to *not* include any default
directories in the include search list.

-I[ ]dir
^^^^^^^^

This option adds the specified directory to the include search
list.  A hanging slash on the end of the path is not required.
The space is optional.

-//
^^^

Enable C++ style ``//`` comments.  The remainder of the line after
``//`` is encountered is interpreted as a comment.  This differs
from ``/*`` style commenting in that no explicit comment-terminator
is required.

-notri
^^^^^^

Disable tri-graph scan pass.  Note that the tri-graph pass is
implemented in assembly and does not slow down preprocessing in
any noticable fashion, you should not disable tri-graphs unless
you need to.
