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
 *$RCSfile: jinclude.h,v $
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
 *          $Source: D:/cvs/eembc2/consumer/cjpegv2/jinclude.h,v $
 *          
 * NOTE   :
 *
 *------------------------------------------------------------------------------
 *
 * HISTORY :
 *
 * $Log: jinclude.h,v $
 * Revision 1.1  2004/05/10 22:36:04  rick
 * Rename Consumer V1.1 benchmarks used in V2
 *
 * Revision 1.4  2004/02/21 00:05:51  rick
 * Upgrade cjpeg to V2
 *
 * Revision 1.3  2004/01/22 20:16:54  rick
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
 * jinclude.h
 *
 * Copyright (C) 1991-1994, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file exists to provide a single place to fix any problems with
 * including the wrong system include files.  (Common problems are taken
 * care of by the standard jconfig symbols, but on really weird systems
 * you may have to edit this file.)
 *
 * NOTE: this file is NOT intended to be included by applications using the
 * JPEG library.  Most applications need only include jpeglib.h.
 */
#include "th_lib.h"
#include <th_file.h>

/* Include auto-config file to find out which system include files we need. */
#include "algo.h"

#include "jconfig.h"        /* auto configuration options */
#define JCONFIG_INCLUDED    /* so that jpeglib.h doesn't do it again */

/*
 * We need the NULL macro and size_t typedef.
 * On an ANSI-conforming system it is sufficient to include <stddef.h>.
 * Otherwise, we get them from <stdlib.h> or <stdio.h>; we may have to
 * pull in <sys/types.h> as well.
 * Note that the core JPEG library does not require <stdio.h>;
 * only the default error handler and data source/destination modules do.
 * But we must pull it in because of the references to FILE in jpeglib.h.
 * You can remove those references if you want to compile without <stdio.h>.
 */

/*
 * We need memory copying and zeroing functions, plus strncpy().
 * ANSI and System V implementations declare these in <string.h>.
 * BSD doesn't have the mem() functions, but it does have bcopy()/bzero().
 * Some systems may declare memset and memcpy in <memory.h>.
 *
 * NOTE: we assume the size parameters to these functions are of type size_t.
 * Change the casts in these macros if not!
 */

#ifdef NEED_BSD_STRINGS

#include <strings.h>
#define MEMZERO(target,size)    bzero((void *)(target), (size_t)(size))
#define MEMCOPY(dest,src,size)    bcopy((const void *)(src), (void *)(dest), (size_t)(size))

#else /* not BSD, assume ANSI/SysV string lib */

#include <string.h>
#define MEMZERO(target,size)    memset((void *)(target), 0, (size_t)(size))
#define MEMCOPY(dest,src,size)    memcpy((void *)(dest), (const void *)(src), (size_t)(size))

#endif

/*
 * In ANSI C, and indeed any rational implementation, size_t is also the
 * type returned by sizeof().  However, it seems there are some irrational
 * implementations out there, in which sizeof() returns an int even though
 * size_t is defined as long or unsigned long.  To ensure consistent results
 * we always use this SIZEOF() macro in place of using sizeof() directly.
 */

#define SIZEOF(object)    ((size_t) sizeof(object))

/*
 * The modules that use fread() and fwrite() always invoke them through
 * these macros.  On some systems you may need to twiddle the argument casts.
 * CAUTION: argument order is different from underlying functions!
 */

size_t cjpeg_fread(void *buf, size_t sizeofbuf, cjpparam_t *params);
size_t cjpeg_fwrite(const void *buf, size_t sizeofbuf, cjpparam_t *params );

#define JFREAD(file,buf,sizeofbuf)  \
  ((size_t) cjpeg_fread((void *) (buf), (size_t) (sizeofbuf), (file)))
#define JFWRITE(file,buf,sizeofbuf)  \
  ((size_t) cjpeg_fwrite((const void *) (buf), (size_t) (sizeofbuf), (file)))

