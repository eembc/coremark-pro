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
 *$RCSfile: jcdctmgr.c,v $
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
 *          $Source: D:/cvs/eembc2/consumer/cjpegv2/jcdctmgr.c,v $
 *          
 * NOTE   :
 *
 *------------------------------------------------------------------------------
 *
 * HISTORY :
 *
 * $Log: jcdctmgr.c,v $
 * Revision 1.1  2004/05/10 22:36:03  rick
 * Rename Consumer V1.1 benchmarks used in V2
 *
 * Revision 1.3  2004/01/22 20:16:54  rick
 * Copyright update and cleanup
 *
 * Revision 1.2  2002/04/22 22:54:52  rick
 * Standard Comment blocks
 *
 *
 *------------------------------------------------------------------------------
 * Other Copyright Notice (if any): 
 * 
 * For conditions of distribution and use, see the accompanying README file.
 * ===========================================================================*/
/*
 * jcdctmgr.c
 *
 * Copyright (C) 1994-1996, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains the forward-DCT management logic.
 * This code selects a particular DCT implementation to be used,
 * and it performs related housekeeping chores including coefficient
 * quantization.
 */

#define JPEG_INTERNALS
#include "jinclude.h"

#include <th_file.h>

#include "jpeglib.h"
#include "jdct.h"        /* Private declarations for DCT subsystem */


/* Private subobject for this module */

typedef struct {
  struct jpeg_forward_dct pub;    /* public fields */

  /* Pointer to the DCT routine actually in use */
  forward_DCT_method_ptr do_dct;

  /* The actual post-DCT divisors --- not identical to the quant table
   * entries, because of scaling (especially for an unnormalized DCT).
   * Each table is given in normal array order.
   */
  DCTELEM * divisors[NUM_QUANT_TBLS];

} my_fdct_controller;

typedef my_fdct_controller * my_fdct_ptr;


/*
 * Initialize for a processing pass.
 * Verify that all referenced Q-tables are present, and set up
 * the divisor table for each one.
 * In the current implementation, DCT of all components is done during
 * the first pass, even if only some components will be output in the
 * first scan.  Hence all components should be examined here.
 */

METHODDEF(void)
start_pass_fdctmgr (j_compress_ptr cinfo)
{
  my_fdct_ptr fdct = (my_fdct_ptr) cinfo->fdct;
  int ci, qtblno, i;
  jpeg_component_info *compptr;
  JQUANT_TBL * qtbl;
  DCTELEM * dtbl;

  for (ci = 0, compptr = cinfo->comp_info; ci < cinfo->num_components;
       ci++, compptr++) {
    qtblno = compptr->quant_tbl_no;
    /* Make sure specified quantization table is present */
    if (qtblno < 0 || qtblno >= NUM_QUANT_TBLS ||
    cinfo->quant_tbl_ptrs[qtblno] == NULL)
      ERREXIT1(cinfo, JERR_NO_QUANT_TABLE, qtblno);
    qtbl = cinfo->quant_tbl_ptrs[qtblno];
    /* Compute divisors for this quant table */
    /* We may do this more than once for same table, but it's not a big deal */
    switch (cinfo->dct_method) {
#ifdef DCT_ISLOW_SUPPORTED
    case JDCT_ISLOW:
      /* For LL&M IDCT method, divisors are equal to raw quantization
       * coefficients multiplied by 8 (to counteract scaling).
       */
      if (fdct->divisors[qtblno] == NULL) {
    fdct->divisors[qtblno] = (DCTELEM *)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
                      DCTSIZE2 * SIZEOF(DCTELEM));
      }
      dtbl = fdct->divisors[qtblno];
      for (i = 0; i < DCTSIZE2; i++) {
    dtbl[i] = ((DCTELEM) qtbl->quantval[i]) << 3;
      }
      break;
#endif
    default:
      ERREXIT(cinfo, JERR_NOT_COMPILED);
      break;
    }
  }
}


/*
 * Perform forward DCT on one or more blocks of a component.
 *
 * The input samples are taken from the sample_data[] array starting at
 * position start_row/start_col, and moving to the right for any additional
 * blocks. The quantized coefficients are returned in coef_blocks[].
 */

METHODDEF(void)
forward_DCT (j_compress_ptr cinfo, jpeg_component_info * compptr,
         JSAMPARRAY sample_data, JBLOCKROW coef_blocks,
         JDIMENSION start_row, JDIMENSION start_col,
         JDIMENSION num_blocks)
/* This version is used for integer DCT implementations. */
{
  /* This routine is heavily used, so it's worth coding it tightly. */
  my_fdct_ptr fdct = (my_fdct_ptr) cinfo->fdct;
  forward_DCT_method_ptr do_dct = fdct->do_dct;
  DCTELEM * divisors = fdct->divisors[compptr->quant_tbl_no];
  DCTELEM workspace[DCTSIZE2];    /* work area for FDCT subroutine */
  JDIMENSION bi;

  sample_data += start_row;    /* fold in the vertical offset once */

  for (bi = 0; bi < num_blocks; bi++, start_col += DCTSIZE) {
    /* Load data into workspace, applying unsigned->signed conversion */
    { register DCTELEM *workspaceptr;
      register JSAMPROW elemptr;
      register int elemr;

      workspaceptr = workspace;
      for (elemr = 0; elemr < DCTSIZE; elemr++) {
    elemptr = sample_data[elemr] + start_col;
#if DCTSIZE == 8        /* unroll the inner loop */
    *workspaceptr++ = GETJSAMPLE(*elemptr++) - CENTERJSAMPLE;
    *workspaceptr++ = GETJSAMPLE(*elemptr++) - CENTERJSAMPLE;
    *workspaceptr++ = GETJSAMPLE(*elemptr++) - CENTERJSAMPLE;
    *workspaceptr++ = GETJSAMPLE(*elemptr++) - CENTERJSAMPLE;
    *workspaceptr++ = GETJSAMPLE(*elemptr++) - CENTERJSAMPLE;
    *workspaceptr++ = GETJSAMPLE(*elemptr++) - CENTERJSAMPLE;
    *workspaceptr++ = GETJSAMPLE(*elemptr++) - CENTERJSAMPLE;
    *workspaceptr++ = GETJSAMPLE(*elemptr++) - CENTERJSAMPLE;
#else
    { register int elemc;
      for (elemc = DCTSIZE; elemc > 0; elemc--) {
        *workspaceptr++ = GETJSAMPLE(*elemptr++) - CENTERJSAMPLE;
      }
    }
#endif
      }
    }

    /* Perform the DCT */
    (*do_dct) (workspace);

    /* Quantize/descale the coefficients, and store into coef_blocks[] */
    { register DCTELEM temp, qval;
      register int i;
      register JCOEFPTR output_ptr = coef_blocks[bi];

      for (i = 0; i < DCTSIZE2; i++) {
    qval = divisors[i];
    temp = workspace[i];
    /* Divide the coefficient value by qval, ensuring proper rounding.
     * Since C does not specify the direction of rounding for negative
     * quotients, we have to force the dividend positive for portability.
     *
     * In most files, at least half of the output values will be zero
     * (at default quantization settings, more like three-quarters...)
     * so we should ensure that this case is fast.  On many machines,
     * a comparison is enough cheaper than a divide to make a special test
     * a win.  Since both inputs will be nonnegative, we need only test
     * for a < b to discover whether a/b is 0.
     * If your machine's division is fast enough, define FAST_DIVIDE.
     */
#ifdef FAST_DIVIDE
#define DIVIDE_BY(a,b)    a /= b
#else
#define DIVIDE_BY(a,b)    if (a >= b) a /= b; else a = 0
#endif
    if (temp < 0) {
      temp = -temp;
      temp += qval>>1;    /* for rounding */
      DIVIDE_BY(temp, qval);
      temp = -temp;
    } else {
      temp += qval>>1;    /* for rounding */
      DIVIDE_BY(temp, qval);
    }
    output_ptr[i] = (JCOEF) temp;
      }
    }
  }
}



/*
 * Initialize FDCT manager.
 */

GLOBAL(void)
jinit_forward_dct (j_compress_ptr cinfo)
{
  my_fdct_ptr fdct;
  int i;

  fdct = (my_fdct_ptr)
    (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
                SIZEOF(my_fdct_controller));
  cinfo->fdct = (struct jpeg_forward_dct *) fdct;
  fdct->pub.start_pass = start_pass_fdctmgr;

  switch (cinfo->dct_method) {
#ifdef DCT_ISLOW_SUPPORTED
  case JDCT_ISLOW:
    fdct->pub.forward_DCT = forward_DCT;
    fdct->do_dct = jpeg_fdct_islow;
    break;
#endif
  default:
    ERREXIT(cinfo, JERR_NOT_COMPILED);
    break;
  }

  /* Mark divisor tables unallocated */
  for (i = 0; i < NUM_QUANT_TBLS; i++) {
    fdct->divisors[i] = NULL;
  }
}
