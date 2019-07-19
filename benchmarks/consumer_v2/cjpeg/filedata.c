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
 *$RCSfile: filedata.c,v $
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
 *          $Source: D:/cvs/eembc2/consumer/cjpegv2/filedata.c,v $
 *          
 * NOTE   :
 *
 *------------------------------------------------------------------------------
 *
 * HISTORY :
 *
 * $Log: filedata.c,v $
 * Revision 1.1  2004/05/10 22:36:03  rick
 * Rename Consumer V1.1 benchmarks used in V2
 *
 * Revision 1.9  2004/04/23 23:40:11  rick
 * PSNR Build updates
 *
 * Revision 1.8  2004/04/22 22:16:16  rick
 * Add New Datasets
 *
 * Revision 1.7  2004/03/23 21:05:16  rick
 * Add evaluation files to cjpeg/djpeg
 *
 * Revision 1.6  2004/02/21 00:05:50  rick
 * Upgrade cjpeg to V2
 *
 * Revision 1.5  2004/01/22 20:16:53  rick
 * Copyright update and cleanup
 *
 * Revision 1.4  2003/01/03 18:47:21  rick
 * Add ee fileio to harness
 *
 * Revision 1.3  2002/12/17 16:42:24  rick
 * Move fileio into Library
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
#include "algo.h"
#include <th_file.h>

/** Load a BMP file into memory */
e_u8 *loadBMPFile(char *fname, int *filesize) {
    ee_FILE	*fd;
    e_u8	*buf;
    char	*fileName;

    if ((fd=pathfind_file_cjpeg(fname,"rb",&fileName))==NULL)
        th_exit(THE_FAILURE,"Failure: Cannot find file '%s'\n",fname);

    *filesize=getFilesize_cjpeg(fileName);
    if(*filesize==0) 
        th_exit(THE_FAILURE,"Failure: Input file \"%s\" is zero bytes.",
							 fileName);

    buf = th_malloc(*filesize);
    if ((th_fread(buf,*filesize,1,fd))!=1)
        th_exit(THE_FAILURE,"Failure: Could not read %d bytes from image file.",
							*filesize);
    th_fclose(fd);
	th_free(fileName);
    return buf;    
}

/*
 * Initialize file used by cjpeg
 */
void init_files_cjpeg(cjpparam_t *params) {
    switch(params->idx) {
        case 0:
            params->outFile_crcsize=5900UL;
            params->default_out_name="Rose256.jpg";
            params->default_in_name="Rose256.bmp";
            break;
        case 1:
            params->outFile_crcsize=20000UL;
            params->default_out_name="goose.jpg";
            params->default_in_name ="goose.bmp";
            break;
        case 2:
            params->outFile_crcsize=85000UL;
            params->default_out_name="EEMBCGroupShotMiami.jpg";
            params->default_in_name ="EEMBCGroupShotMiami.bmp";
            break;
        case 3:
            params->outFile_crcsize=45000UL;
            params->default_out_name="DavidAndDogs.jpg";
            params->default_in_name ="DavidAndDogs.bmp";
            break;
        case 4:
            params->outFile_crcsize=121000UL;
            params->default_out_name="DragonFly.jpg";
            params->default_in_name ="DragonFly.bmp";
            break;
        case 5:
            params->outFile_crcsize=167000UL;
            params->default_out_name="MarsFormerLakes.jpg";
            params->default_in_name ="MarsFormerLakes.bmp";
            break;
        case 6:
            params->outFile_crcsize=18000UL;
            params->default_out_name="Galileo.jpg";
            params->default_in_name ="Galileo.bmp";
            break;
        default:
            params->outFile_crcsize=0;
            params->default_out_name="cjpegdefault.jpg";
            params->default_in_name = params->inFilename;
			th_printf("Unknown file provided!\n");
            break;
    }
    params->outFile_size=params->outFile_crcsize+1000UL;
	if (params->override_idx) {
		params->default_in_name = params->inFilename;
	}
	if (params->inFile_p==NULL)
		params->inFile_p=loadBMPFile(params->default_in_name,&params->inFile_size);
	else 
		th_printf("%s data supplied by C array\n",params->default_in_name);
    if(params->inFile_p==NULL) {
		params->outFile_size=8UL;
		th_printf(">> ERROR! Failed loading    : %s\n",params->default_in_name);
	}
    params->outFile_p=th_malloc(params->outFile_size);

    /* The format matches TH info. */
    th_printf(">> Data Set                 : %s\n",params->default_in_name);
    th_printf(">> Output File              : %s\n",params->default_out_name);
}

/*
 * Read input file
 */
size_t cjpeg_fread(void *buf, size_t sizeofbuf, cjpparam_t *params ) {
    size_t returnSize;

    if(params->inFile_size < (params->inFile_idx + sizeofbuf))
	returnSize = params->inFile_size - params->inFile_idx;
    else
	returnSize = sizeofbuf;

    th_memcpy(buf,&params->inFile_p[params->inFile_idx],returnSize);
    params->inFile_idx += returnSize;
    return returnSize;
}

/*
 * Write output file
 */
size_t cjpeg_fwrite(const void *buf, size_t sizeofbuf, cjpparam_t *params ) {
    size_t returnSize;

    if(params->outFile_size < (params->outFile_idx + sizeofbuf)) {
	returnSize = params->outFile_size - params->outFile_idx;
	th_printf("outFile_size=%d, outFile_idx=%d, sizeofbuf=%d\n",
		params->outFile_size, params->outFile_idx, sizeofbuf);
    }
    else
	returnSize = sizeofbuf;

    th_memcpy(&params->outFile_p[params->outFile_idx],buf,returnSize);
    params->outFile_idx += returnSize;
    return returnSize;
}
