#pragma once

#include <stdint.h>
#include <stddef.h>
#include <SEGGER_SYSVIEW.h>

#ifndef true
    #define true         1
#endif
#ifndef false
    #define false        0
#endif
#define FOREVER          true
#define ALWAYS           true

#define ARRAY_LEN(arr)  (sizeof(arr) / sizeof((arr)[0]))

// Standard unsigned types
typedef uint8_t             u8;
typedef uint16_t            u16;
typedef uint32_t            u32;
typedef uint64_t            u64;

// Volatile unsigned types
// Standard unsigned types
typedef volatile uint8_t    vu8;
typedef volatile uint16_t   vu16;
typedef volatile uint32_t   vu32;
typedef volatile uint64_t   vu64;

// Best runtime performance - heavier on RAM 
typedef uint_fast8_t        fu8;
typedef uint_fast16_t       fu16;
typedef uint_fast32_t       fu32;
typedef uint_fast64_t       fu64;

// Easier on RAM - worse runtime performance
typedef uint_least8_t       lu8;
typedef uint_least16_t      lu16;
typedef uint_least32_t      lu32;
typedef uint_least64_t      lu64;   


// Standard signed types
typedef int8_t              i8;
typedef int16_t             i16;
typedef int32_t             i32;
typedef int64_t             i64;

// Volatile signed types
typedef volatile int8_t     vi8;
typedef volatile int16_t    vi16;
typedef volatile int32_t    vi32;
typedef volatile int64_t    vi64;

// Best runtime performance - heavier on RAM 
typedef int_fast8_t         fi8;
typedef int_fast16_t        fi16;
typedef int_fast32_t        fi32;
typedef int_fast64_t        fi64;

// Easier on RAM - worse runtime performance
typedef int_least8_t        li8;
typedef int_least16_t       li16;
typedef int_least32_t       li32;
typedef int_least64_t       li64;


typedef float               f32;
typedef double              f64;

typedef int8_t              b8;
typedef int32_t             b32;


#if defined(__clang__) || defined(__gcc__) || defined(__avr_gcc__) || defined(__arm_gcc__) || defined(__riscv_gcc__)
    #define STATIC_ASSERT _Static_assert
#else
    #define STATIC_ASSERT static_assert
#endif

STATIC_ASSERT(sizeof(u8)  == 1, "Expected u8 to be 1 byte");
STATIC_ASSERT(sizeof(u16) == 2, "Expected u16 to be 2 bytes");
STATIC_ASSERT(sizeof(u32) == 4, "Expected u32 to be 4 bytes");
STATIC_ASSERT(sizeof(u64) == 8, "Expected u64 to be 8 bytes");

STATIC_ASSERT(sizeof(i8)  == 1, "Expected i8 to be 1 byte");
STATIC_ASSERT(sizeof(i16) == 2, "Expected i16 to be 2 bytes");
STATIC_ASSERT(sizeof(i32) == 4, "Expected i32 to be 4 bytes");
STATIC_ASSERT(sizeof(i64) == 8, "Expected i64 to be 8 bytes");

STATIC_ASSERT(sizeof(f32) == 4, "Expected f32 to be 4 bytes");

#if __SIZEOF_DOUBLE__ != 8
    #ifdef _MSC_VER
        #pragma message "WARNING: `double` is less than 8 bytes!"
    #else
        #warning WARNING: `double` is less than 8 bytes!
    #endif
#else
    STATIC_ASSERT(sizeof(f64) == 8, "Expected f64 to be 8 bytes");
#endif


#if HPLATFORM_ARM
    #include "config/arm.h"
#endif

#if HPLATFORM_RISCV
    #include "config/riscv.h"
#endif