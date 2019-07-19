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

/*************************
** MITH CoreMark version
** 
** Based on the version used for andebench.
** 
*************************/
#include "th_cfg.h"
#include "th_lib.h"
#include "th_rand.h" /* initialize a random data vector */
#include "coremark.h"
#include "core_mith.h"

core_params presets_core[] = {
	{0x3415,0x3415,0x66,31,2048,9,10000 ,0,NULL,0,
	{0x32cc,0xebc9,0x7e4d,0x535b}}
,
	{0x0,0x0,0x66,31,2048,9,1000 ,0,NULL,1,
	{0x33d,0xb5fb,0xb658,0x927b}}
,
	{0x33,0x33,0x33,10,512,4,100 ,0,NULL,2,
	{0x3d49,0x5b1,0x7eb,0xd135}}
,

};

/* ======================================================================== */
/*         Function Prototypes                            */
/* ======================================================================== */
/* file provides :
define - init base input params 
init -  allocate working memory
run - run the benchmark with defined params on working memory
fini - dealloc working memory
verify - Fully verify correct execution if needed
clean - deallocate output memory
*/
void *define_params_core(unsigned int idx, char *name, char *dataset);
void *bmark_init_core(void *);
void *t_run_test_core(struct TCDef *,void *);
int bmark_clean_core(void *);
int bmark_verify_core(void *in_params);
void *bmark_fini_core(void *in_params);


/* benchmark function declarations */
void *iterate(void *pres);

extern volatile e_s32 seed1_volatile;
extern volatile e_s32 seed2_volatile;
extern volatile e_s32 seed3_volatile;
extern volatile e_s32 seed4_volatile;
extern volatile e_s32 seed5_volatile;
volatile e_u32 list_items=0;
volatile e_u32 state_size=0;
volatile e_u32 matrix_n=0;
volatile e_u32 total_data_size=0;
volatile e_u32 mem_req[32];
volatile ee_u32 time_multiplier;

void *define_params_core(unsigned int idx, char *name, char *dataset) {
    core_params *params;
	e_s32 data_index=idx;

	/* parameter setup */
	params=(core_params *)th_malloc(sizeof(core_params));
	if ( params == NULL )
          th_exit( THE_OUT_OF_MEMORY, "Cannot Allocate Memory %s:%d", __FILE__,__LINE__ );

	th_memset(params,0,sizeof(core_params));
	th_memcpy(params,&(presets_core[0]),sizeof(core_params));
	
	if (pgo_training_run!=0) {
		data_index=2; 
	} else {
		th_parse_buf_flag(dataset,"-I",&data_index);
	}
	/* preset datasets */
	if ((data_index>=0) && (data_index<sizeof(presets_core)/sizeof(core_params))) {
		th_memcpy(params,&(presets_core[data_index]),sizeof(core_params));
	} 
	params->data_id=data_index;
	params->results=(core_results *)th_malloc(sizeof(core_results));
	/* command line overrides */
	if (pgo_training_run==0) {
		th_parse_buf_flag(dataset,"-g",&params->gen_ref);
		th_parse_buf_flag_unsigned(dataset,"-L",&params->li);
		th_parse_buf_flag_unsigned(dataset,"-S",&params->ss);
		th_parse_buf_flag_unsigned(dataset,"-M",&params->mn);
		th_parse_buf_flag_unsigned(dataset,"-a",&params->s1);
		th_parse_buf_flag_unsigned(dataset,"-b",&params->s2);
		th_parse_buf_flag_unsigned(dataset,"-c",&params->s3);
		th_parse_buf_flag_unsigned(dataset,"-i",&params->iterations);
	}
	/* setup data for memory alloc and init */
	params->results->iterations=params->iterations;
	params->results->seed1=params->s1;
	params->results->seed2=params->s2;
	params->results->seed3=params->s3;
	state_size=params->ss;
	list_items=params->li;
	matrix_n=params->mn;
	
	mem_req[ID_STATE]=state_size+PADDING/3;
	mem_req[ID_LIST]=((16+sizeof(struct list_data_s))*(list_items+2))+PADDING/3;
	mem_req[ID_MATRIX]=((matrix_n)*(matrix_n)*(2*sizeof(MATDAT) + sizeof(MATRES)))+PADDING/3;
	total_data_size=mem_req[ID_STATE] + mem_req[ID_LIST] + mem_req[ID_MATRIX];
	params->results->size=state_size;
	params->results->err=0;
	params->size=total_data_size;
	
	check_data_types();

	return params;
}


int bmark_clean_core(void *in_params) {
	core_params *params=(core_params *)in_params;
	if (in_params==NULL)
		th_exit( THE_BAD_PTR, "Invalid pointer %s:%d", __FILE__,__LINE__ );
	th_free(params->results);
	th_free(params);
	return 1;
}

void *bmark_init_core(void *in_params) {
	core_params *params=(core_params *)in_params;
    core_params *p;
	if (in_params==NULL)
		th_exit( THE_BAD_PTR, "Invalid pointer %s:%d", __FILE__,__LINE__ );
	p=(core_params *)th_malloc(sizeof(core_params));
	if ( p == NULL )
          th_exit( THE_OUT_OF_MEMORY, "Cannot Allocate Memory %s:%d", __FILE__,__LINE__ );
	th_memcpy(p,params,sizeof(core_params));
	/* alloc memory */
	p->results=(core_results *)th_malloc(sizeof(core_results));
	th_memcpy(p->results,params->results,sizeof(core_results));
	p->results->memblock[0]=th_malloc(params->size);
	if ( p->results->memblock[0] == NULL )
          th_exit( THE_OUT_OF_MEMORY, "Cannot Allocate Memory %s:%d", __FILE__,__LINE__ );
	/* assign memory */
	{
		ee_u32 reserved=0;
		int i;
		for (i=0; i<3; i++) {
			ee_u32 id=(1<<(ee_u32)i);
			p->results->memblock[i+1]=(char *)(p->results->memblock[0])+reserved;
			reserved+=mem_req[id];
		}
	}
	/* init data structures */
	{
		ee_u32 per_item=16+sizeof(struct list_data_s); /* to accomodate systems with 64b pointers, and make sure same code is executed, set max list elements */
		p->results->list=core_list_init(p->li,p->results->memblock[1],p->results->seed1);
		core_init_matrix(mem_req[ID_MATRIX], p->results->memblock[2], (ee_s32)p->results->seed1 | (((ee_s32)p->results->seed2) << 16), &(p->results->mat), p->mn );
		core_init_state(p->ss,p->results->seed1,p->results->memblock[3]);
	}
	
	return p;
}
void *bmark_fini_core(void *in_params) {
    core_params *params;
	if (in_params==NULL)
		th_exit( THE_BAD_PTR, "Invalid pointer %s:%d", __FILE__,__LINE__ );
	params=(core_params *)in_params;

	if ( params->results->memblock[0] != NULL )
		th_free(params->results->memblock[0]);
	th_free(params->results);
	th_free(params);

	return NULL;
}

void *t_run_test_core(struct TCDef *tcdef,void *in_params) {
	core_params *p=(core_params *)in_params;
	e_u32 test1=0, test2=0;
	int i,j;
	
	tcdef->expected_CRC=0;
	iterate(p->results);
		
	if (p->gen_ref)
		tcdef->CRC=0;
		
	return tcdef;
}

int bmark_verify_core(void *in_params) {
	int i;
	core_params *p=(core_params *)in_params;
	int err=0;

	e_u32 s1,s2,s3;
	e_u32 li,ss,mn;
	e_u32 seed;
	e_s32 gen_ref;
	core_results *results;
	int data_id;
	e_u16 crc[4];
	e_u16 seedcrc=crc16(p->results->seed1,0);
	seedcrc=crc16(p->results->seed2,seedcrc);
	seedcrc=crc16(p->results->seed3,seedcrc);
	seedcrc=crc16(p->size,seedcrc);

	if (p->gen_ref) {
		ee_printf("{0x%x,0x%x,0x%x,",		  p->s1,p->s2,p->s3);
		ee_printf("%d,%d,%d,%d ,0,NULL,%d,\n", p->li,p->ss,p->mn,p->iterations, 	p->data_id);
		ee_printf("{0x%x,0x%x,0x%x,0x%x}}\n",p->results->crclist,p->results->crcmatrix,p->results->crcstate,p->results->crc);
	} else {
		i=p->data_id;
		if ((p->results->crclist!=p->crc[0])) {
			ee_printf("[%u]ERROR! list crc 0x%04x - should be 0x%04x\n",i,p->results->crclist,p->crc[0]);
			p->results->err++;
		}
		if ((p->results->crcmatrix!=p->crc[1])) {
			ee_printf("[%u]ERROR! matrix crc 0x%04x - should be 0x%04x\n",i,p->results->crcmatrix,p->crc[1]);
			p->results->err++;
		}
		if ((p->results->crcstate!=p->crc[2])) {
			ee_printf("[%u]ERROR! state crc 0x%04x - should be 0x%04x\n",i,p->results->crcstate,p->crc[2]);
			p->results->err++;
		}
		if ((p->results->crc!=p->crc[3])) {
			ee_printf("[%u]ERROR! final crc 0x%04x - should be 0x%04x\n",i,p->results->crc,p->crc[3]);
			p->results->err++;
		}
	}
	if (p->results->err>0) {
		ee_printf(" - Sizes: %d/%d/%d:%d\n",p->li,p->mn,p->ss,p->results->size);
		ee_printf(" - Seeds: 0x%x/0x%x/0x%x\n",p->s1,p->s2,p->s3);
		return -1;
	}
	return 1;
}
void *iterate(void *pres) {
	ee_u32 i;
	ee_u16 crc;
	core_results *res=(core_results *)pres;
	ee_u32 iterations=res->iterations;
	res->crc=0;
	res->crclist=0;
	res->crcmatrix=0;
	res->crcstate=0;

	for (i=0; i<iterations; i++) {
		crc=core_bench_list(res,1);
		res->crc=crcu16(crc,res->crc);
		crc=core_bench_list(res,-1);
		res->crc=crcu16(crc,res->crc);
		if (i==0) res->crclist=res->crc;
	}
	return NULL;
}