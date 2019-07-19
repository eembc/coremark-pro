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

/**** START DATASET ****/
#include "th_lib.h"
#include "../linpack.h"
static linpack_params in_data_3={
	1000,999,2,73686179,0,
	NULL,NULL,NULL,NULL,
	NULL,
	{0,-1,0x001fffff,0xffffffd5}/*9.999999999999952260e-01*/,
	{0,-1,0x001fffff,0xffffffe8}/*9.999999999999973355e-01*/,
	{0,-27,0x001bc1c4,0x00000000}/*1.292531237595540006e-08*/,
	{0,-52,0x00100000,0x00000000}/*2.220446049250313081e-16*/,
	{1,-46,0x00144000,0x00000000}/*-1.798561299892753595e-14*/,
	{0,-44,0x001f9000,0x00000000}/*1.121325254871408106e-13*/,
	{0,-1,0x001c7396,0x6c22417f}/*8.891098129261506555e-01*/,
	0,0,
	0x00000fff,NULL,MIN_ACC_BITS_FP,
	0.0,0.0,0.0,0.0,0.0,0.0,0.0}; //ref input data

void init_preset(int idx) {
th_memcpy(presets_linpack[idx],in_data_3,sizeof(linpack_params));
presets_linpack[idx].ref_data=in_data_3;
}/**** END DATASET ****/
