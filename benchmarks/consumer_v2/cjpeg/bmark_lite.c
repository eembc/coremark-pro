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
 *$RCSfile: bmark_lite.c,v $
 *
 *   DESC : This file contains the Test Main and other TH support functions
 *
 * AUTHOR : Rick Foos, ECL, LLC
 *          Modified for Multi Instance Test Harness by Ron Olson, 
 *          IBM Corporation
 *
 *  EEMBC : Consumer Subcommittee 
 *
 *    CVS : $Revision: 1.1 $
 *          $Date: 2004/05/10 22:36:03 $
 *          $Author: rick $
 *          $Source: D:/cvs/eembc2/consumer/cjpegv2/bmark_lite.c,v $
 *          
 * NOTE   :
 *
 *------------------------------------------------------------------------------
 * HISTORY :
 *
 * $Log: bmark_lite.c,v $
 * Revision 1.1  2004/05/10 22:36:03  rick
 * Rename Consumer V1.1 benchmarks used in V2
 *
 * Revision 1.24  2004/05/05 18:20:27  rick
 * Fix default ifdef for DATA_1
 *
 * Revision 1.23  2004/04/29 17:23:58  rick
 * EEMBC Bug report fixes.
 *
 * Revision 1.22  2004/02/21 00:05:49  rick
 * Upgrade cjpeg to V2
 *
 * Revision 1.21  2004/01/22 20:16:53  rick
 * Copyright update and cleanup
 *
 * Revision 1.20  2003/05/19 22:01:12  rick
 * Add al_hardware_reset to harness
 *
 * Revision 1.19  2003/05/02 20:36:45  mike
 * Undef FILENAME_MAX before include stdio.h
 *
 * Revision 1.18  2003/04/09 23:17:45  rick
 * frame timing, output streaming
 *
 * Revision 1.17  2003/01/03 18:47:21  rick
 * Add ee fileio to harness
 *
 * Revision 1.16  2002/12/17 22:58:39  rick
 * Fix prototype to cjpeg_main
 *
 * Revision 1.15  2002/09/26 21:54:48  rick
 * Remove unused debug defines
 *
 * Revision 1.14  2002/08/09 00:01:50  rick
 * Add NI CRC to TH Regular
 *
 * Revision 1.13  2002/07/22 21:59:38  rick
 * General cleanup Beta 2b
 *
 * Revision 1.12  2002/07/19 15:53:22  rick
 * Rollback cjpeg/djpeg checksum fix, change checksum to crc8 with e_u8 cast.
 *
 * Revision 1.11  2002/07/17 17:46:31  rick
 * Fix checksum data and results
 *
 * Revision 1.10  2002/07/11 22:19:36  rick
 * Initialize tcdef results
 *
 * Revision 1.9  2002/07/10 19:01:44  rick
 * Always initialize tcdef->CRC
 *
 * Revision 1.8  2002/05/29 22:25:49  rick
 * Set recommended iterations with make
 *
 * Revision 1.7  2002/05/11 00:10:24  rick
 * Fix missing shifts in 16, and 32 bit CRC calculations
 *
 * Revision 1.6  2002/05/10 17:20:38  rick
 * Add al_main to API
 *
 * Revision 1.5  2002/03/12 13:43:42  rick
 * ITERATIONS, CRC_CHECK, and NON_INTRUSIVE_CRC_CHECK
 *
 * Revision 1.4  2002/03/11 22:11:49  rick
 * ITERATIONS, CRC_CHECK, NON_INTRUSIVE TCDef Usage
 *
 *
 *------------------------------------------------------------------------------
 * Other Copyright Notice (if any): 
 * 
 * For conditions of distribution and use, see the accompanying README file.
 * ===========================================================================*/

#include <stdlib.h> /* size_t */
#include <string.h> /* strcmp, strcpy */
#include "algo.h"
#if USE_PRESET
/* Data files */
#include "DavidAndDogs_bmp.h"
#include "door_bmp.h"
#include "DragonFly_bmp.h"
#include "EEMBCGroupShotMiami_bmp.h"
#include "eye_bmp.h"
#include "Galileo_bmp.h"
#include "goose_bmp.h"
#include "hall_bmp.h"
#include "Mandrake_bmp.h"
#include "MarsFormerLakes_bmp.h"
#include "Rose256_bmp.h"
#include "window_bmp.h"
/* End of data files */
#endif

static const version_number bm_ver=BM_VERSION;

static const e_u16 expected_CRC_cjpeg[NUM_DATASETS+1] =
	{ 0x625C, 0xf1fa, 0x00da, 0xd800, 0xdadf, 0x3f50, 0xc551, 0 };

/*
 * Test defintion Structure
 */ 
void init_files_cjpeg(cjpparam_t *params);

/*
 * fill in TCDEF info
 */
static void fill_tcdef_cjpeg(TCDef *tcdef, unsigned int idx) {
    th_strcpy(tcdef->eembc_bm_id, BM_ID);
    th_strcpy(tcdef->member, EEMBC_MEMBER_COMPANY);
    th_strcpy(tcdef->processor, EEMBC_PROCESSOR);
    th_strcpy(tcdef->platform, EEMBC_TARGET);
    th_strcpy(tcdef->desc, BM_DESCRIPTION);

    tcdef->revision = TCDEF_REVISION;
 
    /* Fill the Version of this bench mark */
    tcdef->bm_vnum = bm_ver;

    tcdef->rec_iterations = ITERATIONS;
	if (idx<NUM_DATASETS)
	    tcdef->expected_CRC = expected_CRC_cjpeg[idx];
}


/*
 * Local Declarations
 */

/*
 * Parse command line for input parameters
 */
static void parse_dataset_cjpeg(char *dataset, cjpparam_t *params) {
    int index = 0;
    char para[20];
    e_s32 idx;

    /* Set default values */
    idx = -1;
	params->override_idx=0;

    if(params->idx>=NUM_DATASETS)
		params->idx = NUM_DATASETS;
		
	if (dataset!=NULL)
	{
		char *p=NULL;
		th_get_buf_flag(dataset,"-dataname=",&p);
		if (p) 
			th_sscanf(p,"%s",params->inFilename);
		th_parse_buf_flag(dataset,"-dataindex=",&idx);
		th_parse_buf_flag_unsigned(dataset,"-xdataindex=",&params->override_idx);
		th_parse_buf_flag(dataset,"-do_uuencode=",&params->do_uuencode);
	}
	if (idx >=0)
		params->idx=idx;
#ifdef SELECT_PRESET_ID
	params->use_c_buffer=1;
	switch (params->idx) {
#if ((SELECT_PRESET_ID==1) || (SELECT_PRESET_ID==-1))
	case 0:
	params->inFile_p=get_Rose256_bmp();
	params->inFile_size=Rose256_bmp_length;
	break;
#endif
#if ((SELECT_PRESET_ID==2) || (SELECT_PRESET_ID==-1))
	case 1:
	params->inFile_p=get_goose_bmp();
	params->inFile_size=goose_bmp_length;
	break;
#endif
	default:
	break;
	}
#else
	params->use_c_buffer=0;
#endif /* SELECT_PRESET_ID */
}

/*
 * Define parameters for cjpeg and read input files.
 */
void *define_params_cjpeg(unsigned int idx, char *name, char *dataset) {
    cjpparam_t *params;

    params=(cjpparam_t *)th_calloc(1,sizeof(cjpparam_t));

#if defined(DO_UUENCODE)
    params->do_uuencode=1;
#else
    params->do_uuencode=0;
#endif

    params->idx=idx;
	if (pgo_training_run!=0) {/* goose reserved for pgo training */
		params->idx=1;
	}
	
    parse_dataset_cjpeg(dataset, params);
    init_files_cjpeg(params);	/* Create input and output RAM file */
    return params;
}

/*
 * initialize cjpeg instance
 */
void *bmark_init_cjpeg(void *in_params) {
    cjpparam_t *p,*params=in_params;

    if (in_params == NULL)
        return NULL;

    /* Make a copy of the params for the current instance */
    p = (cjpparam_t *)th_malloc(sizeof(cjpparam_t));
	th_memcpy(p,params,sizeof(cjpparam_t));
	/* Get your own area for output */
	p->outFile_p=th_malloc(p->outFile_size);
	if (p->outFile_p==NULL)
		th_printf("ERROR: Failed allocating output buffer for cjpeg!\n");
    return p;
}
void *bmark_fini_cjpeg(void *in_params) {
    cjpparam_t *p=in_params;
    if (in_params == NULL)
		return NULL;
	if (p->outFile_p!=NULL)
		th_free(p->outFile_p);
    th_free(p);
    return NULL;
}


/*
 * Run an instance of cjpeg test
 */
void *t_run_test_cjpeg( struct TCDef *tcdef, void *in_params ) {
    cjpparam_t *params = (cjpparam_t *)in_params;
    LoopCount	loop_cnt;
    int		rv=0;
    char	*outname;

	e_u32 overide;

	tcdef->expected_CRC=0;
	overide=tcdef->iterations;
	fill_tcdef_cjpeg(tcdef, params->idx);
	if (overide != 0) /* override detected */
		tcdef->iterations=overide;
	else
		tcdef->iterations=tcdef->rec_iterations;

    for(loop_cnt=0;(loop_cnt<tcdef->iterations)&&(rv==0);loop_cnt++)
    {
		rv+=cjpeg_main(&outname, params);
    }
	
	if (verify_output==0)
		tcdef->CRC=tcdef->expected_CRC;

    tcdef->actual_iterations = loop_cnt ;
    tcdef->v1         = rv;
    tcdef->v2         = 0;
    tcdef->v3         = 0;
    tcdef->v4         = 0;
    return NULL;
}

/*
 * Verify cjpeg by calculating CRC over entire output and matching with 
 * expected value.
 */
int bmark_verify_cjpeg(void *in_params) {
    cjpparam_t	*params = (cjpparam_t *)in_params;
    int		i;
    int		img_size=0;
    e_u8	*img_start=NULL;

    if (params == NULL)
        return 0;
    
    img_size = params->outFile_size;
    img_start = params->outFile_p;

    params->cjpeg_CRC=0;
    for(i=0;i<img_size;i++)
	params->cjpeg_CRC = Calc_crc8((e_u8)img_start[i],
							params->cjpeg_CRC);

    /* send the output image buffer back to the host */
    if (params->do_uuencode)
	th_send_buf_as_file((const char *)img_start, img_size,
						params->default_out_name);

    return params->cjpeg_CRC == expected_CRC_cjpeg[params->idx];
}

/*
 * Clean cjpeg instance
 */
/*
 * Clean up cjpeg
 */
int bmark_clean_cjpeg (void *in_params) {
    cjpparam_t	*params = (cjpparam_t *)in_params;

    if (params != NULL) {
        if(!params->use_c_buffer && (params->inFile_p!=NULL)) th_free(params->inFile_p);
        if(params->outFile_p!=NULL) th_free(params->outFile_p);
        th_free(params);
    }
    return 0;
}

