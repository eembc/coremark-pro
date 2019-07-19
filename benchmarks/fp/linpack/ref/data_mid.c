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
	101,100,10,73686179,0,
	NULL,NULL,NULL,NULL,
	NULL,
	{0,0,0x00100000,0x0000000d}/*1.000000000000002887e+00*/,
	{0,0,0x00100000,0x00000003}/*1.000000000000000666e+00*/,
	{0,-33,0x001b5180,0x00000000}/*1.987672249015304260e-10*/,
	{0,-52,0x00100000,0x00000000}/*2.220446049250313081e-16*/,
	{1,-45,0x001d8000,0x00000000}/*-5.240252676230738871e-14*/,
	{0,-45,0x001d8000,0x00000000}/*5.240252676230738871e-14*/,
	{0,-3,0x00117bd7,0x0a3c77a7}/*1.365917968732308363e-01*/,
	0,0,
	0x00000fff,NULL,MIN_ACC_BITS_FP,
	0.0,0.0,0.0,0.0,0.0,0.0,0.0}; //ref input data

void init_preset(int idx) {
th_memcpy(presets_linpack[idx],in_data_3,sizeof(linpack_params));
presets_linpack[idx].ref_data=in_data_3;
}/**** END DATASET ****/
