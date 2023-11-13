
        INCLUDE 'hardware/cia.i'

REGDEF          MACRO   name
        XDEF    _ciaa\1
        XDEF    _ciab\1
_ciaa\1         EQU     _ciaa+cia\1
_ciab\1         EQU     _ciab+cia\1
                ENDM

        DATA

        XDEF    _ciaa
        XDEF    _ciab
_ciaa           EQU     $bfe001
_ciab           EQU     $bfd000

        REGDEF  pra
        REGDEF  prb
        REGDEF  ddra
        REGDEF  ddrb
        REGDEF  talo
        REGDEF  tahi
        REGDEF  tblo
        REGDEF  tbhi
        REGDEF  todlow
        REGDEF  todmid
        REGDEF  todhi
        REGDEF  sdr
        REGDEF  icr
        REGDEF  cra
        REGDEF  crb

                END
