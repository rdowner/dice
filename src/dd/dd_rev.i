VERSION         EQU     1
REVISION        EQU     19
DATE    MACRO
                dc.b    '6.2.94'
        ENDM
VERS    MACRO
                dc.b    'dd 1.19'
        ENDM
VSTRING MACRO
                dc.b    'dd 1.19 (6.2.94)',13,10,0
        ENDM
VERSTAG MACRO
                dc.b    0,'$VER: dd 1.19 (6.2.94)',0
        ENDM
