VERSION         EQU     2
REVISION        EQU     4
DATE    MACRO
                dc.b    '6.1.94'
        ENDM
VERS    MACRO
                dc.b    'dmake 2.4'
        ENDM
VSTRING MACRO
                dc.b    'dmake 2.4 (6.1.94)',13,10,0
        ENDM
VERSTAG MACRO
                dc.b    0,'$VER: dmake 2.4 (6.1.94)',0
        ENDM
