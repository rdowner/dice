/*
 * stdint.h
 * 
 * Implements ISO C99 Standard: 7.18 Integer types <stdint.h> - at least in
 * part, as DICE does not support 64-bit integers.
 * 
 * TODO: 7.18.3 Limits of other integer types
 * TODO: 7.18.4 Macros for integer constants
 */

#ifndef _STDINT_H
#define _STDINT_H 1

typedef signed char     int8_t;
typedef unsigned char   uint8_t;
typedef signed short    int16_t;
typedef unsigned short  uint16_t;
typedef signed long     int32_t;
typedef unsigned long   uint32_t;

typedef signed char     int_least8_t;
typedef unsigned char   uint_least8_t;
typedef signed short    int_least16_t;
typedef unsigned short  uint_least16_t;
typedef signed long     int_least32_t;
typedef unsigned long   uint_least32_t;

typedef signed char     int_fast8_t;
typedef unsigned char   uint_fast8_t;
typedef signed short    int_fast16_t;
typedef unsigned short  uint_fast16_t;
typedef signed long     int_fast32_t;
typedef unsigned long   uint_fast32_t;

typedef signed long     intptr_t;
typedef unsigned long   uintptr_t;

typedef signed long     intmax_t;
typedef unsigned long   uintmax_t;

#define INT8_MIN        (-128)
#define INT8_MAX        127
#define UINT8_MAX       255
#define INT16_MIN       (-32768)
#define INT16_MAX       32767
#define UINT16_MAX      65535
#define INT32_MIN       (-2147483648)
#define INT32_MAX       2147483647
#define UINT32_MAX      4294967295

#define INT_LEAST8_MIN        (-128)
#define INT_LEAST8_MAX        127
#define UINT_LEAST8_MAX       255
#define INT_LEAST16_MIN       (-32768)
#define INT_LEAST16_MAX       32767
#define UIN_LEASTT16_MAX      65535
#define INT_LEAST32_MIN       (-2147483648)
#define IN_LEASTT32_MAX       2147483647
#define UINT_LEAST32_MAX      4294967295

#define INT_FAST8_MIN        (-128)
#define INT_FAST8_MAX        127
#define UINT_FAST8_MAX       255
#define INT_FAST16_MIN       (-32768)
#define INT_FAST16_MAX       32767
#define UINT_FAST16_MAX      65535
#define INT_FAST32_MIN       (-2147483648)
#define INT_FAST32_MAX       2147483647
#define UINT_FAST32_MAX      4294967295

#define INTPTR_MIN       (-2147483648)
#define INTPTR_MAX       2147483647
#define UINTPTR_MAX      4294967295

#define INTMAX_MIN       (-2147483648)
#define INTMAX_MAX       2147483647
#define UINTMAX_MAX      4294967295

#endif /* _STDINT_H */