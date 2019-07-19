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
 *$RCSfile: algo.h,v $
 *
 *   DESC : TH_Lite Test Harness interface routines
 *
 *  EEMBC : EEMBC Technical Advisory Group (TechTAG)
 *          Modified for Multi Instance Test Harness by Ron Olson, 
 *          IBM Corporation
 *
 * AUTHOR : Rick Foos, ECL, LLC
 *
 *    CVS : $Revision: 1.1 $
 *          $Date: 2004/05/10 22:36:02 $
 *          $Author: rick $
 *          $Source: D:/cvs/eembc2/consumer/cjpegv2/algo.h,v $
 *          
 * NOTE   :
 *
 *------------------------------------------------------------------------------
 *
 * HISTORY :
 *
 * $Log: algo.h,v $
 * Revision 1.1  2004/05/10 22:36:02  rick
 * Rename Consumer V1.1 benchmarks used in V2
 *
 * Revision 1.13  2004/05/05 19:18:04  rick
 * Fix default DATA_1 ifdef
 *
 * Revision 1.12  2004/04/29 23:13:54  rick
 * Change Version to R1
 *
 * Revision 1.11  2004/04/29 17:23:58  rick
 * EEMBC Bug report fixes.
 *
 * Revision 1.10  2004/04/22 22:16:16  rick
 * Add New Datasets
 *
 * Revision 1.9  2004/03/24 02:11:09  rick
 * Add CRC's and fix builds for cjpeg/djpeg
 *
 * Revision 1.8  2004/03/23 21:05:16  rick
 * Add evaluation files to cjpeg/djpeg
 *
 * Revision 1.7  2004/02/26 08:42:21  rick
 * mpeg 2 encoder psnr
 *
 * Revision 1.6  2004/02/24 04:32:35  rick
 * MPEG2 checksums, psnr options
 *
 * Revision 1.5  2004/02/21 00:05:49  rick
 * Upgrade cjpeg to V2
 *
 * Revision 1.4  2004/01/22 20:16:53  rick
 * Copyright update and cleanup
 *
 * Revision 1.3  2002/12/17 16:42:24  rick
 * Move fileio into Library
 *
 * Revision 1.2  2002/02/27 00:22:10  rick
 * Add HAVE_MALLOC_H, al_printf to harness.
 *
 * Revision 1.4  2002/02/25 17:15:34  rick
 * Add comment blocks, fix atime th_report call.
 *
 *
 *------------------------------------------------------------------------------
 * Other Copyright Notice (if any): 
 * 
 * For conditions of distribution and use, see the accompanying README file.
 * ===========================================================================*/


/*******************************************************************************
    Includes (thlib.h provides eembc_dt.h for us)                                                                    
*******************************************************************************/

#ifndef _ALGO_H
#define _ALGO_H

#include <th_lib.h>
#include <th_file.h>

/* windef.h defines near and far, only in TH Regular */
#ifndef _WINDEF_
#define FAR 
#define NEAR 
#endif

/*******************************************************************************
    Defines                                                                     
*******************************************************************************/

/* Define iterations */
#if !defined(ITERATIONS) || CRC_CHECK || ITERATIONS==DEFAULT
#undef ITERATIONS
#if CRC_CHECK
#define ITERATIONS 1	/* required iterations for crc */
#else
#define ITERATIONS 10	/* recommended iterations for benchmark */
#endif
#endif

#define NUM_DATASETS 7

#define	BM_DESCRIPTION	"JPEG Compression Benchmark"
#define BM_VERSION		{ 2, 0, 'R', 2 }
#define	BM_ID			"CON cjpegv2    "

/*******************************************************************************
    Global Variables                                                            
*******************************************************************************/

#if BMDEBUG
extern void DebugInit( void );
extern void Debug_Write_Display( e_u16 *display );
extern void Debug_Write_Input(e_u8 Num, e_u8 Invert);
extern void Debug_Write_Loop( n_int Loop );
extern void Debug_Write_i( n_int i );
extern void DebugOutString( e_u8 *szSrc );
#endif

typedef struct {
    e_u32  idx;
    e_s32  do_uuencode;
    char	  inFilename[80];
    unsigned long output_file_size;
    char	  *default_out_name;
    char	  *default_in_name;
    e_u16	  cjpeg_CRC;
    e_u8	  *inFile_p;
    int 	  inFile_idx;
    int 	  inFile_size;
    e_u8	  *outFile_p;
    int 	  outFile_idx;
    int 	  outFile_size;
    int 	  outFile_crcsize;
    e_u32 	override_idx;
	int		use_c_buffer;
} cjpparam_t;

/*******************************************************************************
    Function Prototypes                                                         
*******************************************************************************/

int cjpeg_main ( char **output_fname, cjpparam_t *params );
void init_files(void);
ee_FILE *pathfind_file_cjpeg(const char *filename, const char *filemode,
							char **actualname);
size_t	getFilesize_cjpeg(const char *fname);

#endif /* File Sentinel */
