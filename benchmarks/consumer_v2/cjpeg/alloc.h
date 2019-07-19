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
 *$RCSfile: alloc.h,v $
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
 *          $Date: 2004/05/10 22:36:02 $
 *          $Author: rick $
 *          $Source: D:/cvs/eembc2/consumer/cjpegv2/alloc.h,v $
 *          
 * NOTE   :
 *
 *------------------------------------------------------------------------------
 *
 * HISTORY :
 *
 * $Log: alloc.h,v $
 * Revision 1.1  2004/05/10 22:36:02  rick
 * Rename Consumer V1.1 benchmarks used in V2
 *
 * Revision 1.3  2004/01/22 20:16:53  rick
 * Copyright update and cleanup
 *
 * Revision 1.2  2002/04/22 22:54:51  rick
 * Standard Comment blocks
 *
 *
 *------------------------------------------------------------------------------
 * Other Copyright Notice (if any): 
 * 
 * For conditions of distribution and use, see the accompanying README file.
 * ===========================================================================*/

/*==============================================================================
 *   FILE : ALLOC.H
 *
 *   DESC : Test Harness Library Interface
 *
 *          This header file contains the interface fuctions and
 *          structures for the Test Harness Library, which impliments
 *          the API.
 *
 *    SEE : th.htm for more informat
 *
 *    MKS : $Revision: 1.1 $
 *              $Name: V2_0_0_B11C $
 *              $Date: 2004/05/10 22:36:02 $
 *            $Author: rick $
 *            $Locker:  $
 *
 *             $State: Exp $
 *
 *            $Source: D:/cvs/eembc2/consumer/cjpegv2/alloc.h,v $
 *           $RCSfile: alloc.h,v $
 *
 *       $ProjectName: M:\EEMBC\THV3\project.pj $
 *   $ProjectRevision: 1.3 $
 *
 * ===========================================================================*/

#ifndef   ALLOC_H        /* File Sentinal */
#define   ALLOC_H

#ifdef __cplusplus
extern "C" {
#endif

#define eembc_th_malloc( size ) th_malloc_x( size, __FILE__, __LINE__ )
extern void *th_malloc_x( size_t size, const char *file, int line );
#define eembc_th_free( blk ) th_free_x( blk, __FILE__, __LINE__ )
extern void    th_free_x( void *blk, const char *file, int line );

#ifdef __cplusplus
   }
#endif

#endif                /*  File Sentinal */


/*==============================================================================
 * ENDOF  : ALLOC.H
 * ===========================================================================*/
