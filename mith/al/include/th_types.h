/*
(C) 2014 EEMBC(R).  All rights reserved.                            

All EEMBC Benchmark Software are products of EEMBC 
and are provided under the terms of the EEMBC Benchmark License Agreements.  
The EEMBC Benchmark Software are proprietary intellectual properties of EEMBC and its Members 
and is protected under all applicable laws, including all applicable copyright laws.  
If you received this EEMBC Benchmark Software without having 
a currently effective EEMBC Benchmark License Agreement, you must discontinue use. 
Please refer to LICENSE.md for the specific license agreement that pertains to this Benchmark Software.
*/

/*==============================================================================
 * File: mith/include/th_types.h
 *
 *   DESC: 
 *	 Standard Include file, Defines all EEMBC common types and constants
 *          
 *   NOTE: 
 *			The typedefs in this file are typed to be the same size
 *          on all targets.  But note that this does not guarantee
 *          that structurs defined with these types will have the same
 *          size or that structure members will be at the same offset.
 *          This is due to alignment requirements of the target processor.
 */
 /*
 *------------------------------------------------------------------------------
 * Other Copyright Notice (if any): 
 * 
 * For conditions of distribution and use, see the accompanying README file.
 * ===========================================================================*/
#if	!defined(EEMBC_DT_H)
#define EEMBC_DT_H

#if _MSC_VER>=1500
/* visual studio 9 and above sal.h th_types.h to avoid conflicts */
/* Note : this is only for a non embedded system. Please test with gcc before comitting anything resolved with VS9 */
#include <sal.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define FILE_TYPE_DEFINED
#undef USE_CTYPE 
#define USE_CTYPE 0
#undef HAVE_ASSERT_H
#define HAVE_ASSERT_H 0
#endif

#include "th_cfg.h"

/*----------------------------------------------------------------------------
 * Some compilers do not define EOF in stdio.h, make sure it's defined.
 *----------------------------------------------------------------------------*/

#if	!defined(EOF)
#define EOF (-1)
#endif

/*------------------------------------------------------------------------------
 * The following data types are called the "Standard EEMBC Data Types".  Each
 * is defined appropriatly for the target exeuction environment and the
 * tool chain used to compile/link and possibly locate an instance of the
 * Test Harness and one or more benchmarks.
 *
 * Each of these data types MUST BE defined in a manner which gives
 * them a size of <<AT LEAST>> the number of bits indicated by its name.
 *
 * It is every important to note that a type MUST NOT be defined larger than
 * specified if the build and execution environments support the appropriatly
 * sized data type.
 *
 * For example, if a 'short' is 16bits on a target and an 'int' is 32bits,
 * then the 'e_u16' data type must be defined as 'unsigned short' and
 * not 'unsigned int'.
 *
 * The 8bit, 16bit, 24bit and 32bit integral data types are required.
 * The 48bit and 64bit integral types are optional.
 * The 32bit and 64bit floating point types are optional
 * ---------------------------------------------------------------------------*/
#include <stddef.h>
#include <stdarg.h>

#ifndef	HAVE_64
/** Integral data types at least 64 bits in size  */
#define HAVE_64    1
#endif

/** Integral data types at least 48 bits in size  */
#define HAVE_48    0

/** Floating Point data types */
#if		FLOAT_SUPPORT
#define HAVE_F32   1    /* at least 32 bits in size  */
#define HAVE_F64   1    /* at least 64 bits in size  */
#else
#define HAVE_F32   0    /* at least 32 bits in size  */
#define HAVE_F64   0    /* at least 64 bits in size  */
#endif

/**
 * @remark EEMBC Standard Data Types
 * Note that these all are prefixed with "e_".
 */

typedef unsigned char           e_u8;
typedef signed   char           e_s8;

typedef unsigned short          e_u16;
typedef signed   short          e_s16;
/* Types:
	On most platforms, long defaults to 32b.
	On some platforms, long defaults to 64b, and int to 32b.
	* In such cases, change the e_u32 and e_s32 types from long to int.
*/
#if EE_SIZEOF_LONG==4
typedef unsigned long           e_u24;
typedef signed   long           e_s24;

typedef unsigned long           e_u32;
typedef signed   long           e_s32;
#elif EE_SIZEOF_INT==4
typedef unsigned int           e_u24;
typedef signed   int           e_s24;

typedef unsigned int           e_u32;
typedef signed   int           e_s32;
#else
#error "Don't know how to define a 32b type"
#endif

#if HAVE_48
typedef unsigned long           e_u48;
typedef signed long             e_s48;
#endif

#if HAVE_64
#if defined(_MSC_VER)
typedef unsigned __int64		e_u64;
typedef          __int64		e_s64;
#else
typedef unsigned long long		e_u64;
typedef          long long		e_s64;
#endif
#endif

#if HAVE_F32
typedef float                   e_f32;
#endif

#if HAVE_F64
typedef double                  e_f64;
#endif

#if HAVE_F128
typedef long double             e_f128;
#endif

#if USE_FP32 && !HAVE_F32
#error "A floating point kernel is used, but platform does not support floating point. Please check th_cfg.g and/or kernels being used."
#endif
#if USE_FP32
typedef e_f32 e_fp;
#define FPCONST(_x) (_x##f)
#endif

#if USE_FP64 && !HAVE_F64
#error "A double precision floating point kernel is used, but platform does not support floating point. Please check th_cfg.g and/or kernels being used."
#endif
#if USE_FP64
typedef e_f64 e_fp;
#define FPCONST(_x) (_x)
#endif

#if USE_FP128
typedef e_f128 e_fp;
#define FPCONST(_x) (_x##L)
#endif

#if NEED_SIZE_T
typedef e_u32	size_t;
#endif

#if EE_SIZEOF_PTR==4
typedef e_s32 ee_iptr_t;
#elif EE_SIZEOF_PTR==8
typedef e_s64 ee_iptr_t;
#endif

/**
 * @remark Native Size Types.
 * Native types are used when you want to let the compiler
 *  figure out the native machine size (for optimizations)
 *  but should NOT be used when the sizeof() variable matters
 *  in the calculation.
 */

typedef char                    n_char;
typedef unsigned char           n_uchar;
typedef short                   n_short;
typedef unsigned short          n_ushort;
typedef int                     n_int;
typedef unsigned int            n_uint;
typedef long                    n_long;
typedef unsigned long           n_ulong;
typedef void					n_void; 
#if		FLOAT_SUPPORT
typedef float                   n_float;
typedef double                  n_double;
#endif
/**
 * This data type should be set to a type which will hold the larget
 * benchmark loop count used on your target system.  Generally, this will
 * be a 16bit or a 32bit unsigned type.
 *
 * By default, this type is typedefed as 32b unsigned.
 */
typedef e_u32 LoopCount;
/*------------------------------------------------------------------------------
 * Fixed Size Types
 *
 * These types are always set to be a specific size
 *
 * If the target does not support a type of the required size, then set the
 * type to the smallest size native data type that will hold the defined type.
 *
 * Note: These typedefs will all maintain there proper (indicatd) sizefor
 *       boht the 16 and 32 bit models for the 16/32 bit compilers listed
 *       in the module header.                                           
 *----------------------------------------------------------------------------*/

/*
 * Portable unsigned types
 */
/** Always  8 bits (unsigned) */
#if		USE_TH_BYTE_DEFINE
typedef          e_u8   Byte;     /* Always  8 bits (unsigned) */
#define TH_BYTE_DEFINED 1
#endif
/** Always 16 bits (unsigned) */
typedef          e_u16  Word;
/** Always 32 bits (unsigned) */
typedef          e_u32  Dword;

/*
 * Portable signed types
 */
/** Always  8 bits (signed)   */
typedef          e_s8   Char;
/** Always 16 bits (signed)   */
#if		USE_TH_SHORT_DEFINE
typedef          e_s16  Short;
#endif
/** Always 16 Bits (signed)   */
#if		USE_TH_BOOL_DEFINE
typedef          e_s16  Bool;
#endif
/** Always 32 bits (signed)   */
#if		USE_TH_LONG_DEFINE
typedef          e_s32  Long;
#endif

/**
 * The status type is used by functions which return 'Success' or 'Failure' 
 * where 'Success' is always zero and 'Failure' is defined as default value
 * of '1' but is also considered to be any non zero value.  This is defied
 * as a short because we usually don't need 32bit value here.
 * BUT! Status >does< need to be a signed!
 */
typedef          e_s16  Status;   /* Always 16 bits (signed)   */

/*------------------------------------------------------------------------------
 * Defines and Macros
 *----------------------------------------------------------------------------*/

#ifndef NULL
	#if	defined(__cplusplus)
	  #define NULL 0
	#else
	  #define NULL ((void*)0)
	#endif
#endif

#ifndef FALSE
#define FALSE   0                    
#endif
#ifndef TRUE
#define TRUE    1
#endif
#ifndef NO
#define NO      0
#endif
#ifndef YES
#define YES     1
#endif
#ifdef DEFINE_TH_BAD
#define BAD     0
#endif
#ifndef GOOD
#define GOOD    1
#endif
#ifndef STOP
#define STOP    0
#endif
#ifndef GO
#define GO      1
#endif
#ifndef CLEAR
#define CLEAR   0
#endif
#ifndef SET
#define SET     1
#endif
#ifdef DEFINE_TH_OFF
#define OFF     0
#endif
#ifdef DEFINE_TH_ON
#define ON      1
#endif
#ifndef OK
#define OK      0
#endif
#ifndef NOTOK
#define NOTOK   1
#endif
#if		USE_TH_SUCCESS_DEFINE
#undef  SUCCESS
#define SUCCESS 0
#endif
#if		USE_TH_FAILURE_DEFINE
#undef  FAILURE
#define FAILURE 1
#endif
#ifndef VALID
#define VALID   1
#endif
#ifndef INVALID
#define INVALID 0
#endif
#ifndef ENABLE
#define ENABLE  1
#endif
#ifndef DISABLE
#define DISABLE 0
#endif

/*------------------------------------------------------------------------------
 * Constants
 *----------------------------------------------------------------------------*/
#if	defined(__cplusplus)

   const Bool   False     =  0;         /* See FN#1 */
   const Bool   True      =  1;
   const Bool   No        =  0;
   const Bool   Yes       =  1;
   const Bool   Bad       =  0;
   const Bool   Good      =  1;
   const Bool   Stop      =  0;
   const Bool   Clear     =  0;
   const Bool   Set       =  1;
   const Bool   Off       =  0;
   const Bool   On        =  1;
   const Status Ok        =  0;
   const Status Success   =  0;
   const Status Valid     =  0;
   const Status Notok     =  1;
   const Status Failure   =  1;
   const Status Invalid   =  1;

#if		USE_TH_ENABLE_DEFINE
   const short  Enable    =  1;
#endif
   const short  Disable   =  0;

#else /* not c++ */

#define False     ((Bool)0)
#define True      ((Bool)1) 

#define No        ((Bool)0)
#define Yes       ((Bool)1)
#define Bad       ((Bool)0)
#define Good      ((Bool)1)
#define Stop      ((Bool)0)
#define Clear     ((Bool)0)
#define Set       ((Bool)1)
#define Off       ((Bool)0)
#define On        ((Bool)1)
#define Ok        ((Status)0)
#define Success   ((Status)0)
#define Valid     ((Status)0)
#define Notok     ((Status)1)
#define Failure   ((Status)1)
#define Invalid   ((Status)1)
#if		USE_TH_ENABLE_DEFINE
#define Enable    ((Status)1)
#endif
#define Disable   ((Status)0)

#endif /* if C++ */

/*---------------------------------------------------------------------------
 * Miscelaneous MACROS
 */

/** Round a value upward to the nearest multiple of 4 */
#define ROUNDUP4(x) (((x) + 3) & ~3)
#define ROUNDUP(x,y) ((((n_long)(x)) + (((n_long)(y))-1)) & (0-((n_long)(y))))
#if NEED_INT_LIMITS
#ifndef INT_MAX
#  define INT_MAX	2147483647
#endif
#ifndef INT_MIN
#  define INT_MIN	(-INT_MAX - 1)
#endif
#endif

#if FAKE_FILEIO
#define ee_FILE int
#endif

#endif /* EEMBC_DT_H */
