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
 *$RCSfile: jmemansi.c,v $
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
 *          $Date: 2004/05/10 22:36:04 $
 *          $Author: rick $
 *          $Source: D:/cvs/eembc2/consumer/cjpegv2/jmemansi.c,v $
 *          
 * NOTE   :
 *
 *------------------------------------------------------------------------------
 *
 * HISTORY :
 *
 * $Log: jmemansi.c,v $
 * Revision 1.1  2004/05/10 22:36:04  rick
 * Rename Consumer V1.1 benchmarks used in V2
 *
 * Revision 1.5  2004/02/21 00:05:51  rick
 * Upgrade cjpeg to V2
 *
 * Revision 1.4  2004/01/22 20:16:55  rick
 * Copyright update and cleanup
 *
 * Revision 1.3  2003/08/29 19:52:15  rick
 * th_malloc/th_free cleaning, update comment blocks
 *
 * Revision 1.2  2002/04/22 22:54:53  rick
 * Standard Comment blocks
 *
 *
 *------------------------------------------------------------------------------
 * Other Copyright Notice (if any): 
 * 
 * For conditions of distribution and use, see the accompanying README file.
 * ===========================================================================*/
/*
 * jmemansi.c
 *
 * Copyright (C) 1992-1996, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file provides a simple generic implementation of the system-
 * dependent portion of the JPEG memory manager.  This implementation
 * assumes that you have the ANSI-standard library routine tmpfile().
 * Also, the problem of determining the amount of memory available
 * is shoved onto the user.
 */

#define JPEG_INTERNALS
#include "jinclude.h"

#include "alloc.h"              /* for eembc_th_malloc & eembc_th_free */

#include <th_file.h>

#include "jpeglib.h"
#include "jmemsys.h"        /* import the system-dependent declarations */


#ifndef SEEK_SET        /* pre-ANSI systems may not define this; */
#define SEEK_SET  0        /* if not, assume 0 is correct */
#endif


/*
 * Memory allocation and freeing are controlled by the regular library
 * routines th_malloc() and th_free().
 */

GLOBAL(void *)
cjpeg_get_small (j_common_ptr cinfo, size_t sizeofobject)
{
  return (void *) eembc_th_malloc(sizeofobject);
}

GLOBAL(void)
cjpeg_free_small (j_common_ptr cinfo, void * object, size_t sizeofobject)
{
  eembc_th_free(object);
}


/*
 * "Large" objects are treated the same as "small" ones.
 * NB: although we include FAR keywords in the routine declarations,
 * this file won't actually work in 80x86 small/medium model; at least,
 * you probably won't be able to process useful-size images in only 64KB.
 */

GLOBAL(void FAR *)
cjpeg_get_large (j_common_ptr cinfo, size_t sizeofobject)
{
  return (void FAR *) eembc_th_malloc(sizeofobject);
}

GLOBAL(void)
cjpeg_free_large (j_common_ptr cinfo, void FAR * object, size_t sizeofobject)
{
  eembc_th_free(object);
}


/*
 * This routine computes the total memory space available for allocation.
 * It's impossible to do this in a portable way; our current solution is
 * to make the user tell us (with a default value set at compile time).
 * If you can actually get the available space, it's a good idea to subtract
 * a slop factor of 5% or so.
 */

#ifndef DEFAULT_MAX_MEM        /* so can override from makefile */
#define DEFAULT_MAX_MEM        1000000L /* default: one megabyte */
#endif

GLOBAL(long)
cjpeg_mem_available (j_common_ptr cinfo, long min_bytes_needed,
            long max_bytes_needed, long already_allocated)
{
  return cinfo->mem->max_memory_to_use - already_allocated;
}


/*
 * Backing store (temporary file) management.
 * Backing store objects are only used when the value returned by
 * cjpeg_mem_available is less than the total space needed.  You can dispense
 * with these routines if you have plenty of virtual memory; see jmemnobs.c.
 */


METHODDEF(void)
read_backing_store (j_common_ptr cinfo, backing_store_ptr info,
            void FAR * buffer_address,
            long file_offset, long byte_count)
{
  info->tempFile_idx=file_offset;
  if (info->tempFile_size < info->tempFile_idx + byte_count)
    ERREXIT(cinfo, JERR_TFILE_READ);
  memcpy(buffer_address,&info->tempFile_ptr[info->tempFile_idx],byte_count);
  info->tempFile_idx+=byte_count;
}


METHODDEF(void)
write_backing_store (j_common_ptr cinfo, backing_store_ptr info,
             void FAR * buffer_address,
             long file_offset, long byte_count)
{
  info->tempFile_idx=file_offset;
  if (info->tempFile_size < info->tempFile_idx + byte_count)
    ERREXIT(cinfo, JERR_TFILE_WRITE);
  memcpy(&info->tempFile_ptr[info->tempFile_idx],buffer_address,byte_count);
  info->tempFile_idx+=byte_count;
}


METHODDEF(void)
close_backing_store (j_common_ptr cinfo, backing_store_ptr info)
{
  th_free(info->tempFile_ptr);
  info->tempFile_idx=0;
  info->tempFile_size=0;
}


/*
 * Initial opening of a backing-store object.
 *
 * This version uses tmpfile(), which constructs a suitable file name
 * behind the scenes.  We don't have to use info->temp_name[] at all;
 * indeed, we can't even find out the actual name of the temp file.
 */

GLOBAL(void)
cjpeg_open_backing_store (j_common_ptr cinfo, backing_store_ptr info,
             long total_bytes_needed)
{
  if ((info->tempFile_ptr = th_malloc(total_bytes_needed)) == NULL)
    ERREXITS(cinfo, JERR_TFILE_CREATE, "");
  info->tempFile_idx=0;
  info->tempFile_size=total_bytes_needed;
  info->read_backing_store = read_backing_store;
  info->write_backing_store = write_backing_store;
  info->close_backing_store = close_backing_store;
}


/*
 * These routines take care of any system-dependent initialization and
 * cleanup required.
 */

GLOBAL(long)
cjpeg_mem_init (j_common_ptr cinfo)
{
  return DEFAULT_MAX_MEM;    /* default for max_memory_to_use */
}

GLOBAL(void)
cjpeg_mem_term (j_common_ptr cinfo)
{
  /* no work */
}
