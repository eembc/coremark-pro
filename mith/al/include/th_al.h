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
 *   DESC : Test Harness Adaptation Layer interface
 *
 *  EEMBC : EEMBC Technical Advisory Group (TechTAG)
 *
 * AUTHOR : Shay Gal-On, EEMBC
 *
 *  NOTE! : IMPORTANT! Do not include any host specific include files here
 *          like WINDOWS.H or SOLARIS.H.  This file is intended to be a host
 *          independent interface to the Test Harness Adaptation Layer used by
 *          the Test Harness Functional Layer.
 *
 *------------------------------------------------------------------------------
 * Other Copyright Notice (if any): 
 * 
 * For conditions of distribution and use, see the accompanying README file.
 * ===========================================================================*/

#ifndef   THAL_H       /* File Sentinal */
#define   THAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "th_cfg.h"
#include <stdarg.h> /* size_t */

#if (HAVE_FILEIO && !USE_TH_FILEIO && !FAKE_FILEIO) 
#include <stdio.h>
#endif

#if (!USE_TH_FILEIO) && (!defined(ee_FILE))
	#define ee_FILE FILE
	#ifndef FILE_TYPE_DEFINED
	typedef struct FILE FILE;
	#define FILE_TYPE_DEFINED 1
	#endif
#endif

   /*------------------------------------------------------------------------------
    * Global Defines
    */

#define FATAL_EXIT (-32766)

   /*------------------------------------------------------------------------------
    * Global data
    */


   /*------------------------------------------------------------------------------
    * The Test Harness Lite Application Layer API
    */

	/* Display Routines */
	int     al_sprintf			(char *str, const char *fmt, va_list args);
	int		al_printf			(const char *fmt, va_list	args);
#if	USE_TH_PRINTF
	int      al_write_con       (const char* tx_buf, size_t byte_count);
#endif

	/* Timer Routines */
	size_t   al_ticks_per_sec   (void);
	size_t   al_tick_granularity(void);
	void     al_signal_start    (void);
	size_t   al_signal_finished (void);
	size_t   al_signal_now		(void);

	/* System Routines */
	void     al_exit            (int exit_code);
	void     al_report_results  (void);
	void	 al_hardware_reset	(int ev);

	/* LIBC Routines */
	char	*al_getenv( const char *key );

	/* scanf input format conversion family (thal.c only sees vxxx functions) */

	int		al_vsscanf(const char *str, const char *format, va_list ap);
	int		al_vfscanf(ee_FILE *stream, const char *format, va_list ap);

	/* NON Standard routines */

	int		al_filecmp (const char *file1, const char *file2);
	size_t	al_fsize (const char *filename);
	void	*al_fcreate(const char *filename, const char *mode, char *data, size_t size) ;
	int 	al_unlink   (const char *filename);

	typedef struct intparts_s {
		e_s8 sign;
		e_s16 exp;
		e_u32 mant_high32;
		e_u32 mant_low32;
	} intparts;
	/* FP kernels support routines */
#if (FP_KERNELS_SUPPORT	)
	int store_dp(e_f64 *value, intparts *asint);
	int load_dp(e_f64 *value, intparts *asint);
	int store_sp(e_f32 *value, intparts *asint);
	int load_sp(e_f32 *value, intparts *asint);	

#if ( USE_FP64 )
#define load_fp load_dp
#define store_fp store_dp
#endif

#if ( USE_FP32 )
#define load_fp load_sp
#define store_fp store_sp
#endif

#endif /* of FP_KERNELS_SUPPORT */
   /*----------------------------------------------------------------------------*/

#ifdef __cplusplus    /* Take this out if you don't need it */
   }
#endif

#endif                /*  File Sentinal */

