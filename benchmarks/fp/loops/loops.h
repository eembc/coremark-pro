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

#ifndef _loops_H_
#define _loops_H_


typedef enum _test_type_idx {
	MONTE_CARLO_PI_IDX			=0,
	HYDRO_IDX					,
	CHOLESKY_IDX				,			
	INNER_PRODUCT_IDX			,	
	BANDED_LINEAR_IDX			,	
	TRI_DIAGONAL_IDX			,	
	LINEAR_RECURRENCE_IDX		,	
	STATE_FRAGMENT_IDX			,	
	ADI_INTEGRATION_IDX			,	
	INTEGRATE_PREDICTORS_IDX	,	
	DIFFERENCE_PREDICTORS_IDX	,	
	FIRST_SUM_IDX				,	
	FIRST_DIF_IDX				,	
	PIC_2D_IDX					,	
	PIC_1D_IDX					,	
	CASUAL_IDX					,	
	MONTE_CARLO_IDX				,	
	IMPLICIT_IDX				,	
	HYDRO_2D_IDX				,	
	LIN_RECURRENCE_IDX			,	
	ORDINATE_TRANSPORT_IDX		,	
	MATMUL_IDX					,	
	PLANCKIAN_IDX				,	
	HYDRO_2D_IMPLICIT_IDX		,	
	FIRSTMIN_IDX				,	
	LAST_TEST_IDX				
} test_type_idx;                  

typedef enum test_type_e {
	MONTE_CARLO_PI			= (e_u32)1<<MONTE_CARLO_PI_IDX	,
	HYDRO					= (e_u32)1<<HYDRO_IDX	,
	CHOLESKY				= (e_u32)1<<CHOLESKY_IDX	,			
	INNER_PRODUCT			= (e_u32)1<<INNER_PRODUCT_IDX	,	
	BANDED_LINEAR			= (e_u32)1<<BANDED_LINEAR_IDX	,	
	TRI_DIAGONAL			= (e_u32)1<<TRI_DIAGONAL_IDX	,	
	LINEAR_RECURRENCE		= (e_u32)1<<LINEAR_RECURRENCE_IDX	,	
	STATE_FRAGMENT			= (e_u32)1<<STATE_FRAGMENT_IDX	,	
	ADI_INTEGRATION			= (e_u32)1<<ADI_INTEGRATION_IDX	,	
	INTEGRATE_PREDICTORS	= (e_u32)1<<INTEGRATE_PREDICTORS_IDX	,	
	DIFFERENCE_PREDICTORS	= (e_u32)1<<DIFFERENCE_PREDICTORS_IDX	,	
	FIRST_SUM				= (e_u32)1<<FIRST_SUM_IDX	,	
	FIRST_DIF				= (e_u32)1<<FIRST_DIF_IDX	,	
	PIC_2D					= (e_u32)1<<PIC_2D_IDX	,	
	PIC_1D					= (e_u32)1<<PIC_1D_IDX	,	
	CASUAL					= (e_u32)1<<CASUAL_IDX	,	
	MONTE_CARLO				= (e_u32)1<<MONTE_CARLO_IDX	,	
	IMPLICIT				= (e_u32)1<<IMPLICIT_IDX	,	
	HYDRO_2D				= (e_u32)1<<HYDRO_2D_IDX	,	
	LIN_RECURRENCE			= (e_u32)1<<LIN_RECURRENCE_IDX	,	
	ORDINATE_TRANSPORT		= (e_u32)1<<ORDINATE_TRANSPORT_IDX	,	
	MATMUL					= (e_u32)1<<MATMUL_IDX	,	
	PLANCKIAN				= (e_u32)1<<PLANCKIAN_IDX	,	
	HYDRO_2D_IMPLICIT		= (e_u32)1<<HYDRO_2D_IMPLICIT_IDX	,	
	FIRSTMIN				= (e_u32)1<<FIRSTMIN_IDX	,	
	LAST_TEST				
} test_type;                  
                              
typedef struct loops_params_s {
	/* Benchmark setup */
	e_u32 tests;	//Type of tests to perform. Will be performed in enum order.
	int vsize;	 	//Data size to use for arrays
	int ivsize;
	int m2size;	 	//Data size to use for matrixes
	int m3size;	 	//Data size to use for 3D matrixes
	e_u32 N;	 		//Elements to process at each kernel
	int baseN;	 	//Elements to process, base initial value used.
	int nsize[32];
	int nruns[32];
	e_s32 Loop;	 	//Times to repeat loop
	e_u32 seed;
	e_s32 gen_ref;
	e_s32 reinit_step;
	int full_verify;
	char *err;
	e_u32 minbits;
	e_u32 rtype;

	/* Per init data */
	e_fp **v;
	e_u32 **iv;
	e_fp *v2;
	e_fp **m2;
	e_fp **m3;
	e_fp *rbank;
	e_u32 *irbank;
	void *r; //for random;
	
	/* Verification and return values */
	intparts *ref_data;
	e_fp val;
	e_fp *rx;
	e_fp *ry;
	e_u32 limit_int_input;
	/* random fp inputs that are based on integers */
	e_fp *lrbank;

} loops_params;

/* When adding new data, replace "index" with current NUM_DATAS in generated data,	*
 * add proto and call init_preset_NUM_DATAS in init, and increase NUM_DATAS by 1	*/
#define NUM_DATAS 8
extern loops_params presets_loops[NUM_DATAS];

#endif
