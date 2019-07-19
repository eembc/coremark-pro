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

#ifndef _FFTR2_H_
#define _FFTR2_H_

typedef struct radix2_params_s {
	e_fp *data;
	e_fp *twp;
	e_u32 N;
	e_s32 gen_ref;
	intparts *ref_data;
	intparts ref_min;
	intparts ref_max;
	intparts ref_avg;
	e_u32 minbits;
	e_u32 seed;
} radix2_params;

/* When adding new data, replace "index" with current NUM_DATAS in generated data,	*
 * add proto and call init_preset_NUM_DATAS in init, and increase NUM_DATAS by 1	*/
#define NUM_DATAS 6
extern radix2_params presets_radix2[NUM_DATAS];

#endif