VERSION         EQU     40
REVISION        EQU     3
DATE    MACRO
                dc.b    '28.4.93'
        ENDM
VERS    MACRO
                dc.b    'pools 40.3'
        ENDM
VSTRING MACRO
                dc.b    'pools 40.3 (28.4.93)',13,10,0
        ENDM
VERSTAG MACRO
                dc.b    0,'$VER: pools 40.3 (28.4.93)',0
        ENDM
