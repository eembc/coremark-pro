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

#ifndef CORE_MITH_H
#define CORE_MITH_H

#define NUM_DATAS 1

typedef struct core_params_s {
	e_u32 s1,s2,s3;
	e_u32 li,ss,mn;
	e_u32 iterations;
	e_s32 gen_ref;
	core_results *results;
	int data_id;
	e_u16 crc[4];
	e_u32 size;
} core_params;

extern ee_u16 *list_known_crc;
extern ee_u16 *state_known_crc;
extern ee_u16 *matrix_known_crc;
#define PADDING (256*3)

#endif