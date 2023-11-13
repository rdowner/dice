######################################################################
MAKEMETA=       ../../tools/makemeta

SUBSYSNAME=     amiga_lib
SUBDIRS=        clib exec_support math_support timer_support \
                commodities_support graphics_support rexx_support \
                intuition_support network_support

all quick:      ${SUBDIRS} SFDs doc
                cat clib/clib_lib.lib \
                    exec_support/exec_support_lib.lib \
                    math_support/amiga.lib \
                    timer_support/timer_support_lib.lib \
                    commodities_support/commodities_support_lib.lib \
                    graphics_support/graphics_support_lib.lib \
                    rexx_support/rexx_support_lib.lib \
                    intuition_support/intuition_support_lib.lib \
                    network_support/network_support_lib.lib \
                    >amiga.lib
                cat clib/clib_lib.lib \
                    exec_support/exec_support_lib.lib \
                    math_support/rom.lib \
                    timer_support/timer_support_lib.lib \
                    commodities_support/commodities_support_lib.lib \
                    graphics_support/graphics_support_lib.lib \
                    rexx_support/rexx_support_lib.lib \
                    intuition_support/intuition_support_lib.lib \
                    network_support/network_support_lib.lib \
                    >rom.lib
                mv ${SUBSYSNAME}.doc ..

# This rule makes the alib_lib.sfd file out of sub-pieces, and
# also extracts the alib_stdio_lib.sfd file out of the clib
# directory, where it lives.
# The resulting prototypes get installed

SFDs::
                cat sfd_alib_header exec_support/exec.sfd \
                        clib/clib.sfd \
                        graphics_support/graphics.sfd \
                        math_support/math.sfd \
                        timer_support/timer.sfd \
                        commodities_support/commodities_support_lib.sfd \
                        rexx_support/rexx.sfd \
                        intuition_support/intuition_support_lib.sfd \
                        network_support/network_support_lib.sfd \
                    > alib_lib.sfd
                echo "==end" >>alib_lib.sfd
                rm -f alib_stdio_lib.sfd
                cp clib/clib_stdio.sfd alib_stdio_lib.sfd
                chmod 660 alib_stdio_lib.sfd
                ../../tools/bin/sfd alib_stdio_lib.sfd alib_lib.sfd
                cp PROTOS/* ../../include/clib


.INCLUDE=${MAKEMETA}
