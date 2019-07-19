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

#ifndef _LINPACK_H_
#define _LINPACK_H_


#define ZERO FPCONST(0.0)
#define ONE FPCONST(1.0)

typedef struct linpack_params_s {
	/* Benchmark setup */
	e_u32 lda; 	//Leading dimension
	e_u32 n;	 	//Matrix size
	e_u32 ntimes; //Number of matrixes to solve
	e_u32 seed;
	e_s32 gen_ref;

	/* Per init data */
	e_fp *a;
	e_fp *b; 
	e_u16 *ipvt;
	void *r; //for random;
	
	/* Verification and return values */
	struct linpack_params_s *ref_data;
	intparts iret_lda,iret_ldaa; /* return value so computation cannot be optimized away */
	intparts iresid,ieps,ix0,ixn,iresidn; /* for verification */
	e_u32 min_snr;
	e_u32 max_snr;
	
	e_u32 random_mask;
	e_fp *random_values;
	e_u32 minbits;

	e_fp ret_lda,ret_ldaa; /* return value so computation cannot be optimized away */
	e_fp resid,eps,x0,xn,residn; /* for verification */
} linpack_params;

/* When adding new data, replace "index" with current NUM_DATAS in generated data,	*
 * add proto and call init_preset_NUM_DATAS in init, and increase NUM_DATAS by 1	*/
#define NUM_DATAS 6
extern linpack_params presets_linpack[NUM_DATAS];
void init_presets_linpack();

#endif