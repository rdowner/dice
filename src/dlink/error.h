
/*
 *  ERROR.H
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

/*
**      $Filename: error.h $
**      $Author: dice $
**      $Revision: 30.156 $
**      $Date: 1995/01/11 05:06:09 $
**      $Log: error.h,v $
 * Revision 30.156  1995/01/11  05:06:09  dice
 * .
 *
 * Revision 30.0  1994/06/10  18:05:38  dice
 * .
 *
 * Revision 1.2  1993/10/13  19:47:37  jtoebes
 * Fix BUG06025 - Error when linking with Amiga.lib.
 *
**/

#define EMSG    1
#define EWARN   2
#define EERROR  3
#define ESOFT   4
#define EFATAL  5

#define EF_MASK     (0xF000)

#define EF_MSG      (EMSG << 12)
#define EF_WARN     (EWARN << 12)
#define EF_ERROR    (EERROR << 12)
#define EF_SOFT     (ESOFT << 12)
#define EF_FATAL    (EFATAL << 12)

#define EFATAL_PC_REL_RANGE             (EF_FATAL|1)
#define EFATAL_RANGE_HUNK               (EF_FATAL|2)
#define EERROR_RANGE_HUNK               (EF_ERROR|2)
#define EFATAL_NO_MEMORY                (EF_FATAL|3)
#define EFATAL_RELOC_NOEXIST_HUNK       (EF_FATAL|4)
#define EFATAL_RELOC_BSS_ILLEGAL        (EF_FATAL|5)
#define EFATAL_PC_REL_ILLEGAL_SYMBOL    (EF_FATAL|6)
#define ESOFT_JUMP_TABLE                (EF_SOFT |7)
#define EFATAL_EXTSYM_RELOC             (EF_FATAL|8)
#define EFATAL_RELOC_WORD_ALIGN         (EF_FATAL|9)
#define EFATAL_RELOC_ILLEGAL_SYM        (EF_FATAL|10)
#define EERROR_RELOC_ILLEGAL_SYM        (EF_ERROR|10)
#define EFATAL_OBJECT_PARSE             (EF_FATAL|11)
#define EFATAL_UNKNOWN_SYMBOL_TYPE      (EF_FATAL|12)
#define EFATAL_UNKNOWN_HUNK_TYPE        (EF_FATAL|13)
#define EFATAL_146                      (EF_FATAL|14)
#define EFATAL_TOO_MANY_HUNKS           (EF_FATAL|15)
#define EFATAL_TOO_FEW_HUNKS            (EF_FATAL|16)
#define EFATAL_556                      (EF_FATAL|17)
#define EFATAL_562                      (EF_FATAL|18)
#define EFATAL_571                      (EF_FATAL|19)
#define EFATAL_DATA_HUNK_EMPTY          (EF_FATAL|20)
#define EFATAL_FINAL_SIZE_MISMATCH      (EF_FATAL|21)
#define EFATAL_RELOC_ARRAY_MISMATCH     (EF_FATAL|22)
#define EFATAL_ASSERT                   (EF_FATAL|23)
#define EFATAL_MEMORY_FREE_ERROR        (EF_FATAL|24)
#define EERROR_PCREL_RELOC_TO_DATA      (EF_FATAL|25)
#define EERROR_RELOC32_ILLEGAL_PI       (EF_ERROR|26)
#define EERROR_RELOC32_DATABSS_RES      (EF_ERROR|27)
#define EFATAL_RELOCSCAN_HUNK_RANGE     (EF_FATAL|28)
#define EERROR_RELOCPCREL_BSS_ILLEGAL   (EF_ERROR|29)
#define EERROR_RELOCPCREL_ILLEGAL       (EF_ERROR|30)
#define EERROR_RELOC_LABREL_CODE_ILL    (EF_ERROR|31)
#define EERROR_RELOC8_RANGE             (EF_ERROR|32)
#define EERROR_RELOC16_RANGE            (EF_ERROR|33)
#define EERROR_MULT_DEF_SYM             (EF_ERROR|34)
#define  EWARN_MULT_DEF_SYM             (EF_WARN |34)
#define EERROR_MULT_DEF_SYM_LIB         (EF_ERROR|35)
#define  EWARN_MULT_DEF_SYM_LIB         (EF_WARN |35)
#define EERROR_UNDEF_SYM                (EF_ERROR|36)
#define EERROR_A4REL_TO_FAR_OBJECT      (EF_ERROR|37)
#define EERROR_ABS_REFS_DATA_BSS        (EF_ERROR|38)
#define EERROR_NO_CODE_DATA_BSS         (EF_ERROR|39)
#define EERROR_RELOC32_DATA_DATA_RES    (EF_ERROR|40)
#define EERROR_CANT_FIND_LIB            (EF_ERROR|41)
#define EERROR_CANT_FRAG_RES            (EF_ERROR|42)
#define EERROR_BAD_OPTION               (EF_ERROR|43)
#define EERROR_CANT_OPEN_FILE           (EF_ERROR|44)
#define EERROR_253                      (EF_ERROR|45)
#define EERROR_CANT_CREATE_FILE         (EF_ERROR|46)
#define EFATAL_ERROR_READING_FILE       (EF_FATAL|47)
#define EILLEGAL_HUNK                   (         51)
#define EFATAL_OBJECT_SIZE_MISMATCH     (EF_FATAL|52)
#define EERROR_UNKNOWN_SYM_TYPE         (EF_ERROR|53)
#define EERROR_RELOC16DATA_SYM_RANGE    (EF_ERROR|54)

#define EERROR_PI_EXPECTED_CODE         (EF_ERROR|55)
#define EERROR_PI_EXPECTED_ONE_CODE     (EF_ERROR|56)
#define EWARN_CANNOT_COAG_DATABSS       (EF_WARN |57)
#define EERROR_RELOC32_ILLEGAL_PI_NH    (EF_ERROR|58)
#define EFATAL_EXECUTABLE_TOO_LARGE     (EF_ERROR|59)

