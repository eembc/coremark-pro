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

/* File: core_portme.h */

/*
	Author : Shay Gal-On, EEMBC
*/ 
/* Topic: Description
	This file contains configuration constants required to execute on different platforms
*/
#ifndef CORE_PORTME_H
#define CORE_PORTME_H
#include "th_cfg.h"
#include "th_types.h"
#include "th_lib.h"
#include "al_smp.h"
/************************/
/* Data types and settings */
/************************/
/* Configuration: HAS_FLOAT 
	Define to 1 if the platform supports floating point.
*/
#ifndef HAS_FLOAT 
#define HAS_FLOAT 1
#endif
/* Configuration: HAS_TIME_H
	Define to 1 if platform has the time.h header file,
	and implementation of functions thereof.
*/
#ifndef HAS_TIME_H
#define HAS_TIME_H 1
#endif
/* Configuration: USE_CLOCK
	Define to 1 if platform has the time.h header file,
	and implementation of functions thereof.
*/
#ifndef USE_CLOCK
#define USE_CLOCK 0
#endif
/* Configuration: HAS_STDIO
	Define to 1 if the platform has stdio.h.
*/
#ifndef HAS_STDIO
#define HAS_STDIO 1
#endif
/* Configuration: HAS_PRINTF
	Define to 1 if the platform has stdio.h and implements the printf function.
*/
#ifndef HAS_PRINTF
#define HAS_PRINTF 0
#endif

/* Configuration: CORE_TICKS
	Define type of return from the timing functions.
 */
typedef size_t CORE_TICKS;

/* Definitions: COMPILER_VERSION, COMPILER_FLAGS, MEM_LOCATION
	Initialize these strings per platform
*/
#ifndef COMPILER_VERSION 
 #ifdef __GNUC__
 #define COMPILER_VERSION "GCC"__VERSION__
 #else
 #define COMPILER_VERSION "MITH HARNESS"
 #endif
#endif
#ifndef COMPILER_FLAGS 
 #define COMPILER_FLAGS "MITH HARNESS"
#endif
#ifndef MEM_LOCATION 
 #define MEM_LOCATION "HEAP"
#endif

/* Data Types:
	To avoid compiler issues, define the data types that need ot be used for 8b, 16b and 32b in <core_portme.h>.
	
	*Imprtant*:
	ee_ptr_int needs to be the data type used to hold pointers, otherwise coremark may fail!!!
*/
typedef e_s16 ee_s16;
typedef e_u16 ee_u16;
typedef e_s32 ee_s32;
typedef e_f32 ee_f32;
typedef e_u8 ee_u8;
typedef e_u32 ee_u32;
typedef ee_iptr_t ee_ptr_int;
typedef size_t ee_size_t;
/* align_mem:
	This macro is used to align an offset to point to a 32b value. It is used in the Matrix algorithm to initialize the input memory blocks.
*/
#define align_mem(x) (void *)(4 + (((ee_ptr_int)(x) - 1) & ~3))

/* Configuration: SEED_METHOD
	Defines method to get seed values that cannot be computed at compile time.
	
	Valid values:
	SEED_ARG - from command line.
	SEED_FUNC - from a system function.
	SEED_VOLATILE - from volatile variables.
*/
#ifndef SEED_METHOD
#define SEED_METHOD SEED_VOLATILE
#endif

/* Configuration: MEM_METHOD
	Defines method to get a block of memry.
	
	Valid values:
	MEM_MALLOC - for platforms that implement malloc and have malloc.h.
	MEM_STATIC - to use a static memory array.
	MEM_STACK - to allocate the data block on the stack (NYI).
*/
#ifndef MEM_METHOD
#define MEM_METHOD MEM_MALLOC
#endif

/* Configuration: MULTITHREAD
	Define for parallel execution 
	
	Valid values:
	1 - only one context (default).
	N>1 - will execute N copies in parallel.
	
	Note: 
	If this flag is defined to more then 1, an implementation for launching parallel contexts must be defined.
	
	Two sample implementations are provided. Use <USE_PTHREAD> or <USE_FORK> to enable them.
	
	It is valid to have a different implementation of <core_start_parallel> and <core_end_parallel> in <core_portme.c>,
	to fit a particular architecture. 
*/
#ifndef MULTITHREAD
#define MULTITHREAD 1
#endif

/* Configuration: USE_PTHREAD
	Sample implementation for launching parallel contexts 
	This implementation uses pthread_thread_create and pthread_join.
	
	Valid values:
	0 - Do not use pthreads API.
	1 - Use pthreads API
	
	Note: 
	This flag only matters if MULTITHREAD has been defined to a value greater then 1.
*/
#ifndef USE_PTHREAD
#define USE_PTHREAD 0
#endif

/* Configuration: USE_FORK
	Sample implementation for launching parallel contexts 
	This implementation uses fork, waitpid, shmget,shmat and shmdt.
	
	Valid values:
	0 - Do not use fork API.
	1 - Use fork API
	
	Note: 
	This flag only matters if MULTITHREAD has been defined to a value greater then 1.
*/
#ifndef USE_FORK
#define USE_FORK 0
#endif

/* Configuration: USE_SOCKET
	Sample implementation for launching parallel contexts 
	This implementation uses fork, socket, sendto and recvfrom
	
	Valid values:
	0 - Do not use fork and sockets API.
	1 - Use fork and sockets API
	
	Note: 
	This flag only matters if MULTITHREAD has been defined to a value greater then 1.
*/
#ifndef USE_SOCKET
#define USE_SOCKET 0
#endif

/* Configuration: MAIN_HAS_NOARGC
	Needed if platform does not support getting arguments to main. 
	
	Valid values:
	0 - argc/argv to main is supported
	1 - argc/argv to main is not supported
*/
#ifndef MAIN_HAS_NOARGC 
#define MAIN_HAS_NOARGC 1
#endif

/* Configuration: MAIN_HAS_NORETURN
	Needed if platform does not support returning a value from main. 
	
	Valid values:
	0 - main returns an int, and return value will be 0.
	1 - platform does not support returning a value from main
*/
#ifndef MAIN_HAS_NORETURN
#define MAIN_HAS_NORETURN 1
#endif

/* Variable: default_num_contexts
	Number of contexts to spawn in multicore context.
	Override this global value to change number of contexts used.
	
	Note:
	This value may not be set higher then the <MULTITHREAD> define.
	
	To experiment, you can set the <MULTITHREAD> define to the highest value expected, and use argc/argv in the <portable_init> to set this value from the command line.
*/
extern ee_u32 default_num_contexts;

#if (MULTITHREAD>1)
	#include "al_smp.h"
	#define USE_MITH 1
#endif /* MULTITHREAD > 1 */

typedef struct CORE_PORTABLE_S {
#if (MULTITHREAD>1)
	al_thread_t thread;
#endif /* MULTITHREAD>1 */
	ee_u8	portable_id;
} core_portable;

/* target specific init/fini */
void portable_init(core_portable *p, int *argc, char *argv[]);
void portable_fini(core_portable *p);

#if (SEED_METHOD==SEED_VOLATILE)
 #if (VALIDATION_RUN || PERFORMANCE_RUN || PROFILE_RUN)
  #define RUN_TYPE_FLAG 1
 #else
  #if (TOTAL_DATA_SIZE==1200)
   #define PROFILE_RUN 1
  #else
   #define PERFORMANCE_RUN 1
  #endif
 #endif
#endif /* SEED_METHOD==SEED_VOLATILE */

#define ee_printf th_printf
#define ITERATIONS 1

#endif /* CORE_PORTME_H */
