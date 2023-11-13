
        INCLUDE         'hardware/custom.i'

REGDEF          MACRO
                XDEF    _\1
_\1             EQU     _custom+\1
                ENDM

        DATA

                XDEF    _custom
_custom         EQU     $dff000

        REGDEF  bltddat
        REGDEF  dmaconr
        REGDEF  vposr
        REGDEF  vhposr
        REGDEF  dskdatr
        REGDEF  joy0dat
        REGDEF  joy1dat
        REGDEF  clxdat
        REGDEF  adkconr
        REGDEF  pot0dat
        REGDEF  pot1dat
        REGDEF  potinp
        REGDEF  serdatr
        REGDEF  dskbytr
        REGDEF  intenar
        REGDEF  intreqr
        REGDEF  dskpt
        REGDEF  dsklen
        REGDEF  dskdat
        REGDEF  refptr
        REGDEF  vposw
        REGDEF  vhposw
        REGDEF  copcon
        REGDEF  serdat
        REGDEF  serper
        REGDEF  potgo
        REGDEF  joytest
        REGDEF  bltcon0
        REGDEF  bltcon1
        REGDEF  bltafwm
        REGDEF  bltalwm
        REGDEF  bltcpt
        REGDEF  bltbpt
        REGDEF  bltapt
        REGDEF  bltdpt
        REGDEF  bltsize
        REGDEF  bltcmod
        REGDEF  bltbmod
        REGDEF  bltamod
        REGDEF  bltdmod
        REGDEF  bltcdat
        REGDEF  bltbdat
        REGDEF  bltadat
        REGDEF  cop1lc
        REGDEF  cop2lc
        REGDEF  copjmp1
        REGDEF  copjmp2
        REGDEF  copins
        REGDEF  diwstrt
        REGDEF  diwstop
        REGDEF  ddfstrt
        REGDEF  ddfstop
        REGDEF  dmacon
        REGDEF  clxcon
        REGDEF  intena
        REGDEF  intreq
        REGDEF  adkcon
        REGDEF  aud
        REGDEF  bplpt
        REGDEF  bplcon0
        REGDEF  bplcon1
        REGDEF  bplcon2
        REGDEF  bpl1mod
        REGDEF  bpl2mod
        REGDEF  bpldat
        REGDEF  sprpt
        REGDEF  spr
        REGDEF  color

        END
