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

#ifndef _NNET_H_
#define _NNET_H_

#include "th_cfg.h"

#define IN_SIZE 35
#define IN_SIZE_FILLER (5)		/* filler for IN_SIZE array to make sure it is always divided by 8 elements */
#define IN_SIZE_ALLOC (IN_SIZE + IN_SIZE_FILLER) 
#define OUT_SIZE 8
#define MAX_PATTERNS 26
#define MID_SIZE 8              /* number of neurodes in middle layer */
#define OUT_SIZE 8              /* number of neurodes in output layer */
#define MARGIN FPCONST(0.1)              /* how near to 1,0 do we have to come to stop? */
#define BETA FPCONST(0.09)               /* beta learning constant */
#define ALPHA FPCONST(0.09)              /* momentum term constant */
#define NSTOP FPCONST(0.1)                /* when worst_error less than STOP, training is done */
#define TRUE 1
#define FALSE 0
#define ERR -1

typedef struct nnet_params_s {
	_ALIGN_VAR(e_fp, in_pats[MAX_PATTERNS][IN_SIZE_ALLOC] );      /* input patterns */
	_ALIGN_VAR(e_fp, out_pats[MAX_PATTERNS][OUT_SIZE] );    /* desired output patterns */

	_ALIGN_VAR(e_fp,  mid_wts[MID_SIZE][IN_SIZE_ALLOC] );     /* middle layer weights */
	_ALIGN_VAR(e_fp,  out_wts[OUT_SIZE][MID_SIZE] );    /* output layer weights */
	_ALIGN_VAR(e_fp,  mid_out[MID_SIZE] );              /* middle layer output */
	_ALIGN_VAR(e_fp,  out_out[OUT_SIZE] );              /* output layer output */
	_ALIGN_VAR(e_fp,  mid_error[MID_SIZE] );            /* middle layer errors */
	_ALIGN_VAR(e_fp,  out_error[OUT_SIZE] );            /* output layer errors */
	_ALIGN_VAR(e_fp,  mid_wt_change[MID_SIZE][IN_SIZE_ALLOC] ); /* storage for last wt change */
	_ALIGN_VAR(e_fp,  out_wt_change[OUT_SIZE][MID_SIZE] ); /* storage for last wt change */
	_ALIGN_VAR(e_fp,  out_wt_cum_change[OUT_SIZE][MID_SIZE] ); /* accumulated wt changes */
	_ALIGN_VAR(e_fp,  mid_wt_cum_change[MID_SIZE][IN_SIZE_ALLOC] );  /* accumulated wt changes */
	_ALIGN_VAR(e_fp,  tot_out_error[MAX_PATTERNS] );         /* measure of whether net is done */

	e_fp *random_values;	/* run dependent values	generated with int math */

	int d_x;
	int d_y;
	int d_out;
	int n_in;
	e_s32 gen_ref;
	e_s32 loops;
	int iterations; // net iterations required for net to learn
	int pass_count; // counts passes within current net iteration
	

	e_fp  worst_error; /* worst error each pass through the data */
	e_fp  average_error; /* average error each pass through the data */
	e_fp  avg_out_error[MAX_PATTERNS]; /* average error each pattern */
	
	int ref_iterations;
	int *ref_passes;
	intparts ref_average_error;
	intparts *ref_data;
	e_u32 minbits;
	e_u32 seed; /* Make sure network randomization is repeatable by passing the seed as a creation parameter */
	e_u32 next_ridx;
} nnet_params;

/* When adding new data, replace "index" with current NUM_DATAS in generated data,	*
 * add proto and call init_preset_NUM_DATAS in init, and increase NUM_DATAS by 1	*/
#define NUM_DATAS 2
#define EXTRA_PASSES 10
extern nnet_params presets_nnet[NUM_DATAS];
void fill_preset_nnet(e_u8 *in_data, nnet_params *params);
void init_preset_0();
void init_preset_1();
 
#endif