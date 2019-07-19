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
	51,50,10,73686179,0,
	NULL,NULL,NULL,NULL,
	NULL,
	{0,0,0x00100000,0x00000001}/*1.000000000000000222e+00*/,
	{0,0,0x00100000,0x00000001}/*1.000000000000000222e+00*/,
	{0,-34,0x001f9000,0x00000000}/*1.148237060988321900e-10*/,
	{0,-52,0x00100000,0x00000000}/*2.220446049250313081e-16*/,
	{0,-1023,0x00000000,0x00000000}/*0.000000000000000000e+00*/,
	{1,-50,0x00140000,0x00000000}/*-1.110223024625156540e-15*/,
	{0,-3,0x00143333,0x333214cc}/*1.578124999979649834e-01*/,
	0,0,
	0x00000fff,NULL,MIN_ACC_BITS_FP,
	0.0,0.0,0.0,0.0,0.0,0.0,0.0}; //ref input data

void init_preset(int idx) {
th_memcpy(presets_linpack[idx],in_data_3,sizeof(linpack_params));
presets_linpack[idx].ref_data=in_data_3;
}/**** END DATASET ****/
