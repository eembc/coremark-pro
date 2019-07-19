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
 *$RCSfile: cdjpeg.c,v $
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
 *          $Source: D:/cvs/eembc2/consumer/cjpegv2/cdjpeg.c,v $
 *          
 * NOTE   :
 *
 *------------------------------------------------------------------------------
 *
 * HISTORY :
 *
 * $Log: cdjpeg.c,v $
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

/*
 * cdjpeg.c
 *
 * Copyright (C) 1991-1997, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains common support routines used by the IJG application
 * programs (cjpeg, djpeg, jpegtran).
 */
#include "th_lib.h"
#include "cdjpeg.h"        /* Common decls for cjpeg/djpeg applications */


/*
 * Case-insensitive matching of possibly-abbreviated keyword switches.
 * keyword is the constant keyword (must be lower case already),
 * minchars is length of minimum legal abbreviation.
 */

GLOBAL(boolean)
keymatch (char * arg, const char * keyword, int minchars)
{
  register int ca, ck;
  register int nmatched = 0;

  while ((ca = *arg++) != '\0') {
    if ((ck = *keyword++) == '\0')
      return FALSE;        /* arg longer than keyword, no good */
    if (th_isupper(ca))        /* force arg to lcase (assume ck is already) */
      ca = th_tolower(ca);
    if (ca != ck)
      return FALSE;        /* no good */
    nmatched++;            /* count matched characters */
  }
  /* reached end of argument; fail if it's too short for unique abbrev */
  if (nmatched < minchars)
    return FALSE;
  return TRUE;            /* A-OK */
}

