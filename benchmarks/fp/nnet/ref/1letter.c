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
#include "../nnet.h"
static e_u8 in_data_1[]={
0,0,1,0,0,
0,1,0,1,0,
1,0,0,0,1,
1,0,0,0,1,
1,1,1,1,1,
1,0,0,0,1,
1,0,0,0,1,
0,1,0,0,0,0,0,1,
}; //ref input data

static intparts ref_err_1[]={
	{0,-4,0x0014fbfc,0xbfcdaab7}/*8.197002108287675115e-02*/}; /* ref err */

static intparts ref_average_error_1={0,-4,0x0014fbfc,0xbfcdaab7}/*8.197002108287675115e-02*/;
static int ref_passes[] = {
	21,
	20,
	20,
	21,
	19,
	19,
	22,
	19,
	20}; /* ref passes */

void init_preset_1() {
presets_nnet[1].seed=1936220537;
presets_nnet[1].ref_data=ref_err_1;
presets_nnet[1].d_x=5;
presets_nnet[1].d_y=7;
presets_nnet[1].d_out=8;
presets_nnet[1].n_in=1;
presets_nnet[1].loops=9;
fill_preset_nnet(in_data_1,&(presets_nnet[1]));
presets_nnet[1].ref_passes=ref_passes;
presets_nnet[1].ref_average_error=ref_average_error_1;
presets_nnet[1].ref_iterations=20;
presets_nnet[1].next_ridx=0;
}
/**** END DATASET ****/
