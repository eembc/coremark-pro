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
 *$RCSfile: cjpeg.c,v $
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
 *          $Source: D:/cvs/eembc2/consumer/cjpegv2/cjpeg.c,v $
 *          
 * NOTE   :
 *
 *------------------------------------------------------------------------------
 *
 * HISTORY :
 *
 * $Log: cjpeg.c,v $
 * Revision 1.1  2004/05/10 22:36:03  rick
 * Rename Consumer V1.1 benchmarks used in V2
 *
 * Revision 1.9  2004/02/26 08:42:21  rick
 * mpeg 2 encoder psnr
 *
 * Revision 1.8  2004/02/21 00:05:49  rick
 * Upgrade cjpeg to V2
 *
 * Revision 1.7  2004/01/22 20:16:53  rick
 * Copyright update and cleanup
 *
 * Revision 1.6  2002/09/04 18:48:51  rick
 * Replace exit with th_exit
 *
 * Revision 1.5  2002/07/19 15:53:22  rick
 * Rollback cjpeg/djpeg checksum fix, change checksum to crc8 with e_u8 cast.
 *
 * Revision 1.4  2002/07/17 17:46:31  rick
 * Fix checksum data and results
 *
 * Revision 1.3  2002/04/19 22:17:02  rick
 * Code cleanup for Linux build
 *
 *
 *------------------------------------------------------------------------------
 * Other Copyright Notice (if any): 
 * 
 * Copyright (C) 1991-1998, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains a command-line user interface for the JPEG compressor.
 * It should work on any system with Unix- or MS-DOS-style command lines.
 *
 * Two different command line styles are permitted, depending on the
 * compile-time switch TWO_FILE_COMMANDLINE:
 *    cjpeg [options]  inputfile outputfile
 *    cjpeg [options]  [inputfile]
 * In the second style, output is always to standard output, which you'd
 * normally redirect to a file or pipe to some other program.  Input is
 * either from a named file or from standard input (typically redirected).
 * The second style is convenient on Unix but is unhelpful on systems that
 * don't support pipes.  Also, you MUST use the first style if your system
 * doesn't do binary I/O to stdin/stdout.
 * To simplify script writing, the "-outfile" switch is provided.  The syntax
 *    cjpeg [options]  -outfile outputfile  inputfile
 * works regardless of which command line style is used.
 *
 * For conditions of distribution and use, see the accompanying README file.
 * ===========================================================================*/


#include "cdjpeg.h"          /* Common decls for cjpeg/djpeg applications */
#include "jversion.h"        /* for version message */
#include "algo.h"

/* Create the add-on message string table. */

#define JMESSAGE(code,string)    string ,

static const char * const cdjpeg_message_table[] = {
#include "cderror.h"
  NULL
};

#include "validate.h"
/* for EEMBC library */
#include "bm_lib.h"

/* "hard coded" option: -maxmemory 100  ==> 100kbytes */
#define MAXMEMORY   100

#ifndef VALIDATE_BENCHMARK
#error  "Please make sure VALIDATE_BENCHMARK is defined."
#endif

/** This routine forces the input file format to be BMP. */
LOCAL(cjpeg_source_ptr)
select_file_type (j_compress_ptr cinfo, ee_FILE * infile)
{
    return jinit_read_bmp(cinfo);
}


/*
 * Input and output filenames.
 * Input  file should be a binary bitmap (BMP) file.
 * Output file will be a JPEG (JPG) file.
 */

/* const char *infilename   = "testimg.bmp";    input  filename */
/* const char *outfilename  = "outfile.jpg";    output filename */


/** The main program. */
int cjpeg_main ( char **output_fname, cjpparam_t *params)
{
  struct jpeg_compress_struct   cinfo;
  struct jpeg_error_mgr         jerr;
  cjpeg_source_ptr              src_mgr;
  ee_FILE *                     input_file;
  ee_FILE *                     output_file;
  JDIMENSION                    num_scanlines;

  /* Initialize the JPEG compression object with default error handling. */
  cinfo.err = cjpeg_std_error(&jerr);
  jpeg_create_compress(&cinfo);

  /* Add some application-specific error messages (from cderror.h) */
  jerr.addon_message_table = cdjpeg_message_table;
  jerr.first_addon_message = JMSG_FIRSTADDONCODE;
  jerr.last_addon_message  = JMSG_LASTADDONCODE;

  /* Initialize JPEG parameters.
   * Much of this may be overridden later.
   * In particular, we don't yet know the input file's color space,
   * but we need to provide some value for jpeg_set_defaults() to work.
   */

  cinfo.in_color_space = JCS_RGB; /* arbitrary guess */
  jpeg_set_defaults(&cinfo);

  /* Used to be done in parse_switches() */
  cinfo.mem->max_memory_to_use = MAXMEMORY * 1000L;
  cinfo.err->trace_level       = 0;

  /* Open the input file. */
  input_file = (ee_FILE *)params;

  /* Open the output file. */
  output_file = (ee_FILE *)params;

  /**
   * Figure out the input file format, and set up to read it.
   * 
   * @NOTE: we are only using BMP format so this should result in a
   * call to jinit_read_bmp(). Therefore function pointers will be 
   * start_input_bmp() and finish_input_bmp()
   * 
   */
  src_mgr = select_file_type(&cinfo, input_file);
  src_mgr->input_file = ( cjpparam_t *) input_file;

  /* Read the input file header to obtain file size & colorspace. */
  (*src_mgr->start_input) (&cinfo, src_mgr);

  /* Now that we know input colorspace, fix colorspace-dependent defaults */
  jpeg_default_colorspace(&cinfo);

  /* Adjust default compression parameters by re-parsing the options */
  jpeg_set_quality(&cinfo, 75, FALSE);

  /* Specify data destination for compression */
  jpeg_stdio_dest(&cinfo, output_file);

  /* Start compressor */
  jpeg_start_compress(&cinfo, TRUE);

  /* Process data */
  while (cinfo.next_scanline < cinfo.image_height)
  {
    num_scanlines = (*src_mgr->get_pixel_rows) (&cinfo, src_mgr);
    (void) jpeg_write_scanlines(&cinfo, src_mgr->buffer, num_scanlines);
  }

  /* Finish compression and release memory */
  (*src_mgr->finish_input) (&cinfo, src_mgr);
  jpeg_finish_compress(&cinfo);
  jpeg_destroy_compress(&cinfo);

  /* return output file name to caller */
  /* Handled in pathfind_file *output_fname = CJPEG_OUTPUT_FILE; */

  /* Close input and output files */
  params->inFile_idx=0;
  params->outFile_size=params->outFile_idx;
  params->outFile_idx=0;

#if VALIDATE_BENCHMARK
  /* Verify the actual output against the expected output */
  /* We do this in bmark now... */
  if ( filecmp("testout.jpg", "outfile.jpg") == 0 )
  {
    eembc_benchmark_passed();
  }
  else
  {
    eembc_benchmark_failed();
  }
#endif

  /* All done. */
  return 0;            /* suppress no-return-value warnings */
}
