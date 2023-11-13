
/*
 *  DAS/ERROR.H
 *
 *    (c)Copyright 1992-1997 Obvious Implementations Corp.  Redistribution and
 *    use is allowed under the terms of the DICE-LICENSE FILE,
 *    DICE-LICENSE.TXT.
 */

#define EERROR      0
#define ESOFTWARN   1
#define EWARN       2
#define EVERBOSE    3

#define __FATALPT__ 4

#define EFATAL      4
#define ESOFT       5

#define EF_ERROR    (EERROR     << 12)
#define EF_SOFTWARN (ESOFTWARN  << 12)
#define EF_WARN     (EWARN      << 12)
#define EF_VERBOSE  (EVERBOSE   << 12)
#define EF_FATAL    (EFATAL     << 12)
#define EF_SOFT     (ESOFT      << 12)

#define EF_MASK     (0xF000)

#define EERROR_NO_INPUT_FILE            (EF_ERROR|1)
#define EFATAL_CANT_OPEN_FILE           (EF_FATAL|2)
#define EFATAL_CANT_SEEK_INPUT          (EF_FATAL|3)
#define EFATAL_NO_MEMORY                (EF_FATAL|4)
#define EFATAL_READ_ERROR               (EF_FATAL|5)
#define EFATAL_CANT_CREATE_FILE         (EF_FATAL|6)
#define EVERB_INTRO1                    (EF_VERBOSE|7)
#define EVERB_INTRO2                    (EF_VERBOSE|8)
#define EERROR_SYNTAX                   (EF_ERROR|9)
#define EFATAL_SYNTAX                   (EF_FATAL|9)
#define EERROR_ILLEGAL_LABEL            (EF_ERROR|10)
#define EERROR_EXPECTED_OPERATOR        (EF_ERROR|11)
#define EERROR_EXPECTED_EXP             (EF_ERROR|12)
#define EERROR_BAD_REG_SPEC             (EF_ERROR|13)
#define EERROR_TAKES_NO_OPS             (EF_ERROR|14)
#define ESOFT_BAD_DIRECTIVE             (EF_SOFT|15)
#define EERROR_ILLEGAL_SIZE             (EF_ERROR|16)
#define ESOFTWARN_BAD_OP_SIZE           (EF_SOFTWARN|17)
#define EVERBOSE_REMOVED_BRANCH_NEXT    (EF_VERBOSE|18)
#define ESOFT_BRANCH_OPT                (EF_SOFT|19)
#define ESOFT_BAD_MASK_REG              (EF_SOFT|20)
#define EVERBOSE_BRANCH_BRANCH          (EF_VERBOSE|21)
#define EERROR_BAD_RELOC                (EF_ERROR|22)
#define EFATAL_DUMPSECT_NEG             (EF_FATAL|23)
#define EERROR_BAD_SECTION_DIRECT       (EF_ERROR|24)
#define EERROR_BAD_SECTION_TYPE         (EF_ERROR|25)
#define EERROR_SECTION_NAME_CONFLICT    (EF_ERROR|26)
#define EERROR_NEG_LABEL_OFFSET         (EF_ERROR|27)
#define EERROR_OUTER_DISP_WL            (EF_ERROR|28)
#define EERROR_NO_BRACKET_NO_OD         (EF_ERROR|29)
#define EWARN_68020                     ( EF_WARN|30)
#define EERROR_UNDEFINED_LABEL          (EF_ERROR|31)
#define EERROR_EXPECTED_TERMINATING     (EF_ERROR|32)
#define EWARN_GARBAGE                   ( EF_WARN|33)
#define EERROR_ILLEGAL_ADDR_MODES       (EF_ERROR|34)
#define EERROR_DIRECTIVE_REQUIRES_LABEL (EF_ERROR|35)
#define EERROR_DIRECT_REQUIRES_SIZE     (EF_ERROR|36)
#define EERROR_EXPECTED_GOT             (EF_ERROR|37)
#define ESOFTWARN_DIRECT_BYTES_MISMATCH (EF_SOFTWARN|38)
#define ESOFT_DEBUG_INTERNAL_ERROR      ( EF_SOFT|39)
#define EFATAL_INTERNAL_ERROR           (EF_FATAL|39)
#define EERROR_LABEL_MULTIPLY_DEFINED   (EF_ERROR|40)
#define EERROR_UNKNOWN_DIRECTIVE        (EF_ERROR|41)
#define EERROR_LABEL_ADDR_MISMATCH      (EF_ERROR|42)
#define EERROR_ADDR_MISMATCH            (EF_ERROR|43)
#define EERROR_WORD_OFFSET              (EF_ERROR|44)
#define EERROR_BYTE_OFFSET              (EF_ERROR|45)
#define EERROR_7_OFFSET                 (EF_ERROR|46)
#define EERROR_8_OFFSET                 (EF_ERROR|47)
#define EERROR_15_OFFSET                (EF_ERROR|48)
#define EERROR_SHORT_BRANCH             (EF_ERROR|49)
#define EERROR_BYTE_BRANCH_OFFSET       (EF_ERROR|50)
#define ESOFT_BAD_SRCAB                 ( EF_SOFT|51)



