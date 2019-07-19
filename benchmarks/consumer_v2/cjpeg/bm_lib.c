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
 *$RCSfile: bm_lib.c,v $
 *
 *   DESC : Jpeg Compression
 *
 *  EEMBC : Consumer Subcommittee
 *
 * AUTHOR : 
 *          Modified for Multi Instance Test Harness by Ron Olson, 
 *          IBM Corporation
 *
 *    CVS : $Revision: 1.1 $
 *          $Date: 2004/05/10 22:36:03 $
 *          $Author: rick $
 *          $Source: D:/cvs/eembc2/consumer/cjpegv2/bm_lib.c,v $
 *          
 * NOTE   :
 *
 *------------------------------------------------------------------------------
 *
 * HISTORY :
 *
 * $Log: bm_lib.c,v $
 * Revision 1.1  2004/05/10 22:36:03  rick
 * Rename Consumer V1.1 benchmarks used in V2
 *
 * Revision 1.3  2004/01/22 20:16:53  rick
 * Copyright update and cleanup
 *
 * Revision 1.2  2002/04/19 22:17:02  rick
 * Code cleanup for Linux build
 *
 *
 *------------------------------------------------------------------------------
 * Other Copyright Notice (if any): 
 * 
 * For conditions of distribution and use, see the accompanying README file.
 * ===========================================================================*/

#include "validate.h"
#include "bm_lib.h"

#ifndef VALIDATE_BENCHMARK 
#error  "Please make sure VALIDATE_BENCHMARK is defined."
#endif

#if VALIDATE_BENCHMARK 

/*
 * Called whenever a benchmark runs to completion and the output
 * is exactly what is expected.
 */
void eembc_benchmark_passed ( void )
{
    int i;
    i = 1;
    return;
}

/*
 * Called whenever a benchmark runs to completion but the output
 * is not what is expected.
 */
void eembc_benchmark_failed ( void )
{
    int i;
    i = 1;
    return;
}


/*
 * Called whenever a fatal (unexpected, unrecoverable) error is encountered
 */
void eembc_fatal_error ( char *error_msg )
{
    int i;
    i = 1;
    return;
}
#endif   /* #if VALIDATE_BENCHMARK */ 
