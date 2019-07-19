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
 *$RCSfile: jutils.c,v $
 *
 *   DESC : Jpeg Utilities
 *
 *  EEMBC : Consumer Subcommittee
 *
 * AUTHOR : 
 *          Modified for Multi Instance Test Harness by Ron Olson, 
 *          IBM Corporation
 *
 *    CVS : $Revision: 1.1 $
 *          $Date: 2004/05/10 22:36:05 $
 *          $Author: rick $
 *          $Source: D:/cvs/eembc2/consumer/cjpegv2/jutils.c,v $
 *          
 * NOTE   :
 *
 *------------------------------------------------------------------------------
 *
 * HISTORY :
 *
 * $Log: jutils.c,v $
 * Revision 1.1  2004/05/10 22:36:05  rick
 * Rename Consumer V1.1 benchmarks used in V2
 *
 * Revision 1.6  2004/04/29 17:23:58  rick
 * EEMBC Bug report fixes.
 *
 * Revision 1.5  2004/02/26 08:42:21  rick
 * mpeg 2 encoder psnr
 *
 * Revision 1.4  2004/02/21 00:05:51  rick
 * Upgrade cjpeg to V2
 *
 * Revision 1.3  2004/01/22 20:16:55  rick
 * Copyright update and cleanup
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
 * jutils.c
 *
 * Copyright (C) 1991-1996, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains tables and miscellaneous utility routines needed
 * for both compression and decompression.
 * Note we prefix all global names with "j" to minimize conflicts with
 * a surrounding application.
 */

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"
#include "algo.h"

/** Find and Open a file */
ee_FILE *pathfind_file_cjpeg(const char *filename, const char *filemode,
							char **actualname) {
	char	*path[] = {"../data/libbmp/","","../../../","../../libbmp/",
				"libbmp/","../../libjpeg/","libjpeg/",NULL};
	char	buf[FILENAME_MAX];
	int	i=0;
	ee_FILE	*found_file=NULL;

	if ((th_strcmp(filename,"stdin")) == 0) return th_fopen(filename,filemode);

	while (path[i] != NULL)
	{
		th_strcpy(buf,path[i]);
		th_strcat(buf,filename);
		if ( (found_file = th_fopen(buf,filemode)) != NULL ) 
		{
			if (actualname)
				*actualname = th_strdup(buf);
			return found_file;
		}
		i++;
	}
	return NULL;
}

/** Get file size using stat function */
size_t	getFilesize_cjpeg(const char *fname)
{
    size_t	length;
    struct	ee_stat st;

    if (!fname || !*fname) 
		length=0;
    else
        if ((th_stat(fname,&st))==-1)
			length=0;
        else
			length = st.st_size;
    return length;
}

/*
 * cjpeg_natural_order[i] is the natural-order position of the i'th element
 * of zigzag order.
 *
 * When reading corrupted data, the Huffman decoders could attempt
 * to reference an entry beyond the end of this array (if the decoded
 * zero run length reaches past the end of the block).  To prevent
 * wild stores without adding an inner-loop test, we put some extra
 * "63"s after the real entries.  This will cause the extra coefficient
 * to be stored in location 63 of the block, not somewhere random.
 * The worst case would be a run-length of 15, which means we need 16
 * fake entries.
 */

const int cjpeg_natural_order[DCTSIZE2+16] = {
  0,  1,  8, 16,  9,  2,  3, 10,
 17, 24, 32, 25, 18, 11,  4,  5,
 12, 19, 26, 33, 40, 48, 41, 34,
 27, 20, 13,  6,  7, 14, 21, 28,
 35, 42, 49, 56, 57, 50, 43, 36,
 29, 22, 15, 23, 30, 37, 44, 51,
 58, 59, 52, 45, 38, 31, 39, 46,
 53, 60, 61, 54, 47, 55, 62, 63,
 63, 63, 63, 63, 63, 63, 63, 63, /* extra entries for safety in decoder */
 63, 63, 63, 63, 63, 63, 63, 63
};


/*
 * Arithmetic utilities
 */

GLOBAL(long)
cjdiv_round_up (long a, long b)
/* Compute a/b rounded up to next integer, ie, ceil(a/b) */
/* Assumes a >= 0, b > 0 */
{
  return (a + b - 1L) / b;
}


GLOBAL(long)
cjround_up (long a, long b)
/* Compute a rounded up to next multiple of b, ie, ceil(a/b)*b */
/* Assumes a >= 0, b > 0 */
{
  a += b - 1L;
  return a - (a % b);
}


/* On normal machines we can apply MEMCOPY() and MEMZERO() to sample arrays
 * and coefficient-block arrays.  This won't work on 80x86 because the arrays
 * are FAR and we're assuming a small-pointer memory model.  However, some
 * DOS compilers provide far-pointer versions of memcpy() and memset() even
 * in the small-model libraries.  These will be used if USE_FMEM is defined.
 * Otherwise, the routines below do it the hard way.  (The performance cost
 * is not all that great, because these routines aren't very heavily used.)
 */

#ifndef NEED_FAR_POINTERS    /* normal case, same as regular macros */
#define FMEMCOPY(dest,src,size)    MEMCOPY(dest,src,size)
#define FMEMZERO(target,size)    MEMZERO(target,size)
#else                /* 80x86 case, define if we can */
#ifdef USE_FMEM
#define FMEMCOPY(dest,src,size)    _fmemcpy((void FAR *)(dest), (const void FAR *)(src), (size_t)(size))
#define FMEMZERO(target,size)    _fmemset((void FAR *)(target), 0, (size_t)(size))
#endif
#endif


GLOBAL(void)
cjcopy_sample_rows (JSAMPARRAY input_array, int source_row,
           JSAMPARRAY output_array, int dest_row,
           int num_rows, JDIMENSION num_cols)
/* Copy some rows of samples from one place to another.
 * num_rows rows are copied from input_array[source_row++]
 * to output_array[dest_row++]; these areas may overlap for duplication.
 * The source and destination arrays must be at least as wide as num_cols.
 */
{
  register JSAMPROW inptr, outptr;
#ifdef FMEMCOPY
  register size_t count = (size_t) (num_cols * SIZEOF(JSAMPLE));
#else
  register JDIMENSION count;
#endif
  register int row;

  input_array += source_row;
  output_array += dest_row;

  for (row = num_rows; row > 0; row--) {
    inptr = *input_array++;
    outptr = *output_array++;
#ifdef FMEMCOPY
    FMEMCOPY(outptr, inptr, count);
#else
    for (count = num_cols; count > 0; count--)
      *outptr++ = *inptr++;    /* needn't bother with GETJSAMPLE() here */
#endif
  }
}


GLOBAL(void)
cjcopy_block_row (JBLOCKROW input_row, JBLOCKROW output_row,
         JDIMENSION num_blocks)
/* Copy a row of coefficient blocks from one place to another. */
{
#ifdef FMEMCOPY
  FMEMCOPY(output_row, input_row, num_blocks * (DCTSIZE2 * SIZEOF(JCOEF)));
#else
  register JCOEFPTR inptr, outptr;
  register long count;

  inptr = (JCOEFPTR) input_row;
  outptr = (JCOEFPTR) output_row;
  for (count = (long) num_blocks * DCTSIZE2; count > 0; count--) {
    *outptr++ = *inptr++;
  }
#endif
}


GLOBAL(void)
cjzero_far (void FAR * target, size_t bytestozero)
/* Zero out a chunk of FAR memory. */
/* This might be sample-array data, block-array data, or alloc_large data. */
{
#ifdef FMEMZERO
  FMEMZERO(target, bytestozero);
#else
  register char FAR * ptr = (char FAR *) target;
  register size_t count;

  for (count = bytestozero; count > 0; count--) {
    *ptr++ = 0;
  }
#endif
}
