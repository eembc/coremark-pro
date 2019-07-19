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
 * File: mith/include/th_lib.h
 *
 *  Description: 
 *		Test Harness Library Interface
 *
 *	Details:
 *		For more information about the functions in this file refer to <th_lib.c>
 */
 /*------------------------------------------------------------------------------
 * Other Copyright Notice (if any): 
 * 
 * For conditions of distribution and use, see the accompanying README file.
 * ===========================================================================*/

#ifdef THSTDIO_H
#error "You CANNOT include thlib.h with thstdio.h"
#endif

#ifndef THLIB_H
#define	THLIB_H

#if _MSC_VER>=1500
/* visual studio 9 and above needs all these before th_types.h to avoid conflicts */
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
#include "th_types.h"
#include "th_version.h"
#include "th_al.h"

#define		VN_BAD_PTR          0x0001
#define		VN_BAD_MAJOR        0x0002
#define		VN_BAD_MINOR        0x0004
#define		VN_BAD_STEP         0x0008
#define		VN_BAD_REVISION     0x0010
#define		VN_EMPTY_STR        0x0020
#define		VN_BAD_VER_IND      0x0040
#define		VN_MAJOR_TOO_BIG    0x0080
#define		VN_MINOR_TOO_BIG    0x0100
#define		VN_REVISION_TOO_BIG 0x0200
#define		VN_BAD_PERIOD       0x0400

/*----------------------------------------------------------------------------*/

typedef struct{
   unsigned char  major;
   unsigned char  minor;
   unsigned char  step;
   unsigned char  revision;
} version_number;

/*------------------------------------------------------------------------------
 * Test Component Defintion
 *
 * A const pointer to this sturcture is returned from test component's
 * entry point to the test harness.  It is used by the test harness to
 * control the test or benchmark.
 *----------------------------------------------------------------------------*/
#if FP_KERNELS_SUPPORT
typedef struct snr_result_s {
	e_f64	min;
	e_f64	max;
	e_f64	min_ok;
	e_u32	bmin;
	e_u32	bmax;
	e_u32	bmin_ok;
	e_f64	sum;
	e_f64	avg;
	e_u32	bsum;
	e_u32	bavg;
	e_f64	stdev;
	int pass;
} snr_result;
#endif
/* TCDef.revsion == 1  { this is revision 1 of this structure } */

#define TCDEF_REVISION (3)

typedef struct TCDef{
   /*------------------------------------
    * This section is the same 
    * for all versions of this structure
    */
   char				eembc_bm_id[ 16 ];/* id flag */
   char				member[ 16 ];     /* the member id */
   char				processor[ 16 ];  /* the processor id */
   char				platform[ 16 ];   /* the platform id */
   char				desc[ 64 ];       /* benchmark description */
   e_u16			revision;         /* The revision of this structure. */

   /*------------------------------------*/

   version_number	th_vnum_required;  /* TH Version Required        */
   version_number	target_vnum_required;  /* Target Hardware Version Required */
   version_number	bm_vnum;  /* the version of this bench mark */
   LoopCount  		rec_iterations;

   /*
    * Test Harness Results
    */
   e_u32			iterations;
   e_u32			actual_iterations;
   e_u32            duration;
   e_u16            CRC;
   size_t			v1;		/* Verification Data, can be double via union */
   size_t			v2;
   size_t			v3;
   size_t			v4;
   e_u16            expected_CRC;
#if FP_KERNELS_SUPPORT
	snr_result		SNR;
	e_f64			dbl_data[4];
#endif
   struct ee_connection_s **connection;
} TCDef;

#ifdef GCC_INLINE_MACRO
#define INLINE_KEYWORD __inline__
#endif

#ifdef INLINE_KEYWORD
#define inline INLINE_KEYWORD
#endif

#ifdef NO_INLINE_SUPPORT
#define inline 
#endif

/*------------------------------------------------------------------------------
 * The Test Harness Lite API
 *----------------------------------------------------------------------------*/

/* Display control */
int th_printf( const char *fmt, ... );
int th_vsprintf( char *str, const char *fmt, va_list args );
#if BMDEBUG
#define th_dprintf th_printf
#else
/* if BMDEBUG off, make this a local useless function to allow compiler to easily optimize it away */
static inline int th_dprintf( const char *fmt, ... ) { if (fmt) return 1; else return 0; }
#endif
int th_sprintf( char *str, const char *fmt, ... );
int th_snprintf(char *str, size_t size, const char *format, ...);

/* Memory Management */
#define th_malloc( size ) th_malloc_x( size, __FILE__, __LINE__ )
void *th_malloc_x( size_t size, const char *file, int line );
#define th_calloc( nmemb, size ) th_calloc_x( nmemb, size, __FILE__, __LINE__ )
void *th_calloc_x( size_t nmemb, size_t size, const char *file, int line );
#define th_realloc( blk, size ) th_realloc_x( blk, size, __FILE__, __LINE__ )
void *th_realloc_x( void *blk, size_t size, const char *file, int line );
#define th_strdup( string ) th_strdup_x( string, __FILE__, __LINE__ )
char *th_strdup_x( const char *string, const char *file, int line );
#define th_free( blk ) th_free_x( blk, __FILE__, __LINE__ )
void    th_free_x( void *blk, const char *file, int line );
void    th_heap_reset( void );
void mem_heap_initialize(void);

#define th_aligned_malloc( size, align ) th_aligned_malloc_x( size, align, __FILE__, __LINE__ )
void *th_aligned_malloc_x( size_t size, size_t align, const char *file, int line );

#define th_aligned_free( blk ) th_aligned_free_x( blk, __FILE__, __LINE__ )
void    th_aligned_free_x( void *blk, const char *file, int line );


/* memory related routines that reside in string.h */
char *th_strcpy(char *dest, const char *src);
char *th_strncpy(char *dest, const char *src, size_t n);
char *th_strchr(const char *s, int c);
char *th_strrchr(const char *s, int c);
char *th_strstr(const char *haystack, const char *needle);
int th_strcmp(const char *s1, const char *s2);
int th_strncmp(const char *s1, const char *s2, size_t n);
char *th_strpbrk(const char *s, const char *accept);
size_t th_strcspn(const char *s, const char *reject);
char *th_strcat(char *dest, const char *src);
void *th_memcpy(void *s1, const void *s2, size_t n);
void *th_memmove(void *s1, const void *s2, size_t n);
void *th_memset(void *s, int c, size_t n);
int th_memcmp(const void *s1, const void *s2, size_t n);
size_t th_strlen(const char *s);
size_t th_strspn(const char *s, const char *s2);
int th_isdigit(int c);
int th_isspace(int c);
long int th_strtol(const char *nptr, char **endptr, int base);
int th_atoi(const char *nptr);
int th_abs(int j);
#if FLOAT_SUPPORT
double th_atof(const char *nptr);
double th_strtod(const char *nptr, char ** RESTRICT endptr);
#endif

/* Endianess Stuff */
#define ee_32_swap(_x) ((((_x)&0x000000FF)<<24) | (((_x)&0x0000FF00)<<8) | (((_x)&0x00FF0000)>>8) | (((_x)&0xFF000000)>>24))
#define ee_16_swap(_x) ( (((_x)&0x00FF)<<8) | (((_x)&0xFF00)>>8) )

/** Test Harness Timer Routines */
int th_timer_available( void );
int th_timer_is_intrusive( void );
size_t th_ticks_per_sec( void );
size_t th_tick_granularity( void );
size_t th_signal_now( void );

/* Benchmark initiation and execution is managed by MITH
	these function should not be needed, and are obsolete.
void  th_signal_start( void );
e_u32 th_signal_finished( void );
*/

/** Test Harness System Routines */
void    al_main(int argc, char* argv[]); /* this function is in th_al.c, but accessed directly */
void	th_exit( int exit_code, const char *fmt, ... );
#ifdef DOUBLECHECK
#pragma double_check no_return 0 th_exit
#endif
int		th_report_results(TCDef *tcdef, e_u16 Expected_CRC );
int		th_harness_poll( void );

/** libc routines */
char	*th_getenv(const char *key);

/** UUencode buffer to stdout which can be uudecoded to filename on host. */
int th_send_buf_as_file(const unsigned char * buf, size_t length, const char * fn);

/** Use TH_RAND_MAX instead of stdlib.h RAND_MAX.
Test Harness Random Numbers (Uniform Distribution [0-128].
*/
#define TH_RAND_MAX	128
/* Function: th_rand
	A thread safe pseudo random number generator.
	Must initialize with th_srand.
	
	Params:
		id - Return value from a previous th_srand call.
			Each thread must initialize with th_srand.

	Returns:
		A pseudo random number, that depends on id, and is guaranteed
		to produce the same sequence at every run
*/
int th_rand(void * id);
/* Function: th_srand
	Initialize a thread safe pseudo random number generator.
	
	Params:
		seed - different seed numbers will lead to different 
			pseudo random number sequences.

	Returns:
		An id that should be passed to subsequent
		th_rand calls by that thread.

	Note: 
		Call th_frand before thread exits to clean up.
*/
void *th_srand(unsigned int seed);
/* Function: th_randseed
	Return a seed that can be used to recreate random sequence from current state.
*/
unsigned int th_randseed(void *curid);
/* Function: th_frand
	Clean up data that was previosuly allocated by a call to <th_srand>
*/
void th_frand(void *id);

/** Are CRC Utilities Needed? */
#if	CRC_CHECK || NON_INTRUSIVE_CRC_CHECK
e_u16 Calc_crc8(e_u8 data, e_u16 crc );
e_u16 Calc_crc16( e_u16 data, e_u16 crc );
e_u16 Calc_crc32( e_u32 data, e_u16 crc );
e_u16 th_crcbuffer(const void *inbuf, size_t size, e_u16 inputCRC);
#endif

#if FP_KERNELS_SUPPORT
#define ee_snr(sig,ref) ((e_f64)(sig)/((e_f64)(sig)-(e_f64)(ref)))*((e_f64)(sig)/((e_f64)(sig)-(e_f64)(ref)))
#define ee_snr_db(sig,ref) 10*th_log10_f64(ee_snr((sig),(ref)))
e_f64 ee_snr_buffer(e_f64 *signal, e_f64 *ref, int size, snr_result *res);
 #if ( USE_FP64 )
 #define th_sprint_fp th_sprint_dp
 #define th_print_fp th_print_dp
 #endif
 #if ( USE_FP32 )
 #define th_sprint_fp th_sprint_sp
 #define th_print_fp th_print_sp
 #endif
int th_print_fp(e_fp value);
char *th_sprint_fp(e_fp value, char *buf);
int th_fpprintf( const char *fmt, ... );
e_u32 fp_accurate_bits_dp(e_f64 sig, e_f64 ref);
e_u32 fp_accurate_bits_sp(e_f32 sig, e_f32 ref);
e_u32 ee_fpbits_buffer_dp(e_f64 *signal, e_f64 *ref, int size, snr_result *res);
e_u32 ee_fpbits_buffer_sp(e_f32 *signal, e_f32 *ref, int size, snr_result *res);
e_u32 fp_iaccurate_bits_dp(e_f64 sig, intparts *refbits);
e_u32 fp_iaccurate_bits_sp(e_f32 sig, intparts *refbits);
e_u32 ee_ifpbits_buffer_dp(e_f64 *signal, intparts *ref, int size, snr_result *res);
e_u32 ee_ifpbits_buffer_sp(e_f32 *signal, intparts *ref, int size, snr_result *res);
#define MIN_ACC_BITS_FP32 14
#define MIN_ACC_BITS_FP64 30
#define MIN_ACC_BITS_OTHER 42
extern intparts intparts_zero;
#if ( USE_FP64 )
#define fp_accurate_bits fp_accurate_bits_dp
#define fp_iaccurate_bits fp_iaccurate_bits_dp
#define ee_fpbits_buffer ee_fpbits_buffer_dp
#define ee_ifpbits_buffer ee_ifpbits_buffer_dp
#define MIN_ACC_BITS_FP MIN_ACC_BITS_FP64
#endif
#if ( USE_FP32 )
#define fp_accurate_bits fp_accurate_bits_sp
#define fp_iaccurate_bits fp_iaccurate_bits_sp
#define ee_fpbits_buffer ee_fpbits_buffer_sp
#define ee_ifpbits_buffer ee_ifpbits_buffer_sp
#define MIN_ACC_BITS_FP MIN_ACC_BITS_FP32
#endif
 #define MAX_ACC_COUNTS 128
 typedef struct acc_bits_i_s {
	e_s32 n;
	e_s32 min;
	e_s32 max;
	e_s32 sum;
	e_s32 avg;
 } acc_bits_i;
 typedef struct acc_bits_d_s {
	acc_bits_i sig_exp;
	acc_bits_i ref_exp;
	acc_bits_i bits;
	e_u32 counts[MAX_ACC_COUNTS+1];
 } acc_bits_d;
 extern acc_bits_d acc_summary;
#endif /* FP_KERNELS_SUPPORT */

/* scanf input format conversion family (thal.c only sees vxxx functions) */
/* TODO: Initial version of MITH relies on OS to provide thread safe scanf functions. 
No implementations due to some compilers not supporting vsscanf.
	*** scanf should not be used in benchmark context.
int 	th_scanf(const char *format, ...);
int		th_sscanf(const char *str, const char *format, ...);
int		th_vscanf(const char *format, va_list ap);
int		th_vsscanf(const char *str, const char *format, va_list ap);
*/
int		th_sscanf(const char *str, const char *format, ...);
/* Function: th_parse_flag
	parse argv looking for an int value from a flag, in the form of <flag><value>. */
#if USE_CTYPE
#include <ctype.h>
#define th_tolower tolower
#define th_islower islower
#define th_toupper toupper
#define th_isupper isupper
#else
/* ctype functionality */
#define th_islower(_c) (((_c) >= 'a') && ((_c) <= 'z'))
#define th_isupper(_c) (((_c) >= 'A') && ((_c) <= 'Z'))
#define th_tolower(_c) (th_isupper(_c) ? (_c) + ('a' - 'A') : (_c))
#define th_toupper(_c) (th_islower(_c) ? (_c) - ('a' - 'A') : (_c))
#endif
int th_parse_flag(int argc,char *argv[],char *flag,e_s32 *val);
int th_parse_flag_unsigned(int argc,char *argv[],char *flag,e_u32 *val);
/* Function: th_parse_buf_flag
	parse a string looking for an int value from a flag, in the form of <flag><value>.
*/
int th_parse_buf_flag(char *buf,char *flag, e_s32 *val);
int th_parse_buf_flag_unsigned(char *buf,char *flag,e_u32 *val);

#if FLOAT_SUPPORT
/* Function: th_parse_buf_fpflag
	parse a string looking for an floating point value from a flag, in the form of <flag><value>.
*/
int th_parse_buf_f64flag(char *buf,char *flag, e_f64 *val);
int th_parse_buf_f32flag(char *buf,char *flag, e_f32 *val);
 #if ( USE_FP64 )
 #define th_parse_buf_fpflag th_parse_buf_f64flag
 #endif
 #if ( USE_FP32 )
 #define th_parse_buf_fpflag th_parse_buf_f32flag
 #endif
#endif
/* Function: th_get_flag
	parse argv looking for a string flag from a flag, in the form of <flag><value>.
*/
int th_get_flag(int argc,char *argv[],char *flag,char **val);
/* Function: th_get_buf_flag
	parse a string looking for a string flag from a flag, in the form of <flag>...
*/
int th_get_buf_flag(char *buf,char *flag,char **val);
/* Function: th_dup_buf_flag
	Parse a string looking for a string flag from a flag, in the form of <flag>..., then strdup the string. 
	* When using this function, the memory returned in *val needs to be deallocated by the caller!
*/
int th_dup_buf_flag(char *buf,char *flag,char **val);
/* Function: th_parse_buf_flag_word
	parse a string looking for a string flag from a flag, in the form of <flag><value> .
*/
int th_parse_buf_flag_word(char *buf,char *flag,char *val[]);
/* Function: th_parse_val
	Parse a string to get an int. Accepts G/M/K etc qualifiers.
*/
e_s32 th_parse_val(char *valstring);

#ifdef USE_TH_BLOCKSIZE_DEFINE
typedef size_t BlockSize;
#endif

/* error reporting and handling aids */
enum TH_ERRORS
  {
  THE_SUCCESS,            /* Success! */
  THE_FAILURE,             /* Generic Failure... :( */

  THE_BAD_PTR,            /* a pointer parameter was NULL */
  THE_BAD_THDEF_VERSION,  /* the THDef pointer did not point to a valid structure */
  THE_BAD_TCDEF_VERSION,  /* the THDef pointer did not point to a valid structure */
  THE_TC_INIT_FAILED,     /* The test component intialization failed */
  THE_BAD_BASE_PTR,       /* Bad base pointer for Malloc init */
  THE_BAD_SIZE,           /* The size parameter is bad */
  THE_OUT_OF_MEMORY       /* The test ran out of memory */
  };

const char *th_error_str( int error );
typedef enum {
	TH_INVALID = 0,
	TH_INFO,
	TH_WARNING,
	TH_ERROR,
	TH_FATAL,
	TH_ERR_CODE_LAST
} th_err_codes;
/* Function: th_log
	Description:
	Log a message with test harness qualifiers

	Note:
		code must be one of the codes in th_err_codes
*/
void th_log(int code, char *msg);
extern e_u32 pgo_training_run;
extern e_u32 verify_output;
extern e_u32 reporting_threshold;

/* simple int list common code */
typedef struct ee_intlist_s
{
        void *data;
		unsigned int info;
        struct ee_intlist_s *next; /* pointer to next element in list */
} ee_intlist;
ee_intlist *ee_list_add(ee_intlist **p, void *data, unsigned int info);
void ee_list_remove(ee_intlist **p); /* remove head */
ee_intlist **ee_list_isearch(ee_intlist **n, unsigned int info);
ee_intlist **ee_list_dsearch(ee_intlist **n, void *data);
/* to report stats on malloc at various points */
#ifndef REPORT_THMALLOC_STATS
#define REPORT_THMALLOC_STATS 0
#endif
#if REPORT_THMALLOC_STATS
extern size_t th_malloc_total;
extern size_t th_malloc_max;
#endif

#endif /*THLIB_H_FILE*/ 
