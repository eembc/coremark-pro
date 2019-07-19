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
** Black Scholes .
** Calculate the theta of black scholes for stocks.
** Based on the equation in wikipedia (see http://en.wikipedia.org/wiki/Black%E2%80%93Scholes).
** 
*************************/
#include "th_cfg.h"
#include "th_lib.h"
#include "th_rand.h" /* initialize a random data vector */
#include "shabench.h"

sha_params presets_sha[NUM_DATAS] = {
	{1048576,NULL,
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0x15,0xa2,0x8b,0xdd,0x4c,0x67,0x87,0xec,0xb6,0xba,0x11,0x77,0x22,0xc2,0xf1,0x56,0xc0,0x93,0x4a,0x9f,0x53,0xd1,0x2d,0xaa,0xb8,0x03,0xf2,0xb6,0x38,0xc2,0x89,0xef},
		66,0},
	{33554432,NULL,
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0xea,0xb6,0xae,0xe7,0xa7,0x59,0xc2,0xfc,0x3e,0xa5,0xa0,0x67,0xa9,0xc9,0xf6,0xdc,0x53,0x6f,0x33,0xb8,0x54,0x15,0x53,0xec,0x08,0x95,0x97,0x94,0x57,0x82,0xf1,0x1f},
		66,0},
	{128,NULL,
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0x23,0x38,0x74,0xbb,0x06,0xc3,0x21,0x91,0xcd,0xc8,0x1a,0x6d,0xbf,0x44,0x14,0x9a,0x86,0xfc,0x6b,0xed,0x04,0xbf,0x4f,0x22,0xc7,0xfd,0xb3,0xb6,0x83,0x50,0x79,0xfc},
		66,0}
};

/* ======================================================================== */
/*         Function Prototypes                            */
/* ======================================================================== */
/* file provides :
define - init base input params (segment [within 0..2 boundary], number of coefficients to calculate, number of integration steps)
init -  allocate working memory, assign a[0] and b[0] . 
run - calculate N coefficients in the segment
fini - calculate avg of coeffients then dealloc working memory
verify - SNR average of coefficients vs golden reference, must run at least base iterations for valid output.
clean - deallocate output memory
*/
void *define_params_sha(unsigned int idx, char *name, char *dataset);
void *bmark_init_sha(void *);
void *t_run_test_sha(struct TCDef *,void *);
int bmark_clean_sha(void *);
int bmark_verify_sha(void *in_params);
void *bmark_fini_sha(void *in_params);

/* benchmark function declarations */
void sha2(e_u8* data, e_u32 length, e_u8* digest);


void *define_params_sha(unsigned int idx, char *name, char *dataset) {
    sha_params *params;
	e_s32 data_index=idx;

	/* parameter setup */
	params=(sha_params *)th_malloc(sizeof(sha_params));
	if ( params == NULL )
          th_exit( THE_OUT_OF_MEMORY, "Cannot Allocate Memory %s:%d", __FILE__,__LINE__ );

	th_memset(params,0,sizeof(sha_params));

	params->gen_ref=0;
	params->seed=1835102827;
	params->size=10;
	th_memset(params->expected_digest,0,32);
	
	if (pgo_training_run!=0) {
		data_index=2; 
	}
	th_parse_buf_flag(dataset,"-i",&data_index);
	/* preset datasets */
	if ((data_index>=0) && (data_index<NUM_DATAS)) {
		th_memcpy(params,&(presets_sha[data_index]),sizeof(sha_params));
	} 
	/* command line overrides */
	if (pgo_training_run==0) {
		th_parse_buf_flag_unsigned(dataset,"-s",&params->seed);
		th_parse_buf_flag(dataset,"-g",&params->gen_ref);
		th_parse_buf_flag_unsigned(dataset,"-N",&params->size);
	}
	/* setup the input data */
	params->data=random_u8_vector(params->size,params->seed);
	
	return params;
}


int bmark_clean_sha(void *in_params) {
	sha_params *params=(sha_params *)in_params;
	if (in_params==NULL)
		th_exit( THE_BAD_PTR, "Invalid pointer %s:%d", __FILE__,__LINE__ );
	if (params->data)
		th_free(params->data);
	th_free(params);
	return 1;
}

void *bmark_init_sha(void *in_params) {
	sha_params *params=(sha_params *)in_params;
    sha_params *myparams;
	if (in_params==NULL)
		th_exit( THE_BAD_PTR, "Invalid pointer %s:%d", __FILE__,__LINE__ );
	myparams=(sha_params *)th_malloc(sizeof(sha_params));
	if ( myparams == NULL )
          th_exit( THE_OUT_OF_MEMORY, "Cannot Allocate Memory %s:%d", __FILE__,__LINE__ );
	th_memcpy(myparams,params,sizeof(sha_params));
	
	return myparams;
}
void *bmark_fini_sha(void *in_params) {
    sha_params *params;
	if (in_params==NULL)
		th_exit( THE_BAD_PTR, "Invalid pointer %s:%d", __FILE__,__LINE__ );
	params=(sha_params *)in_params;
	
	th_free(params);

	return NULL;
}

void *t_run_test_sha(struct TCDef *tcdef,void *in_params) {
	sha_params *p=(sha_params *)in_params;
	e_u32 test1=0, test2=0;
	int i,j;
	
	tcdef->expected_CRC=0;
	sha2(p->data, p->size, p->digest);
	for (i=0; i<32; i++)
		if (p->digest[i]!=p->expected_digest[i])
			tcdef->CRC++;
		
	if (p->gen_ref)
		tcdef->CRC=0;
		
	return tcdef;
}

int bmark_verify_sha(void *in_params) {
	int i;
	char digest_string[5*32];
	char buf[5];
	sha_params *p=(sha_params *)in_params;
	int err=0;

	if (p->gen_ref) {
		*digest_string=0;
		for (i=0; i<32; i++) {
			th_sprintf(buf,"0x%02x",p->digest[i]);
			if (i>0)
				th_strcat(digest_string,",");
			th_strcat(digest_string,buf);
		}
		th_printf("\t{%d,NULL,\n\t\t{0,},\n\t\t{%s},\n\t%d,0},\n",
						p->size,	digest_string,p->seed);			
	} else {
		for (i=0; i<32; i++)
			if (p->digest[i]!=p->expected_digest[i])
				err++;
		
		if (err==0)
			return 1;
			
		th_printf("SHA256 ERROR in validation!\n");
		*digest_string=0;
		for (i=0; i<32; i++) {
			th_sprintf(buf,"0x%02x",p->digest[i]);
			th_strcat(digest_string,buf);
		}
		th_printf("digest,%s\n",digest_string);
		*digest_string=0;
		for (i=0; i<32; i++) {
			th_sprintf(buf,"0x%02x",p->expected_digest[i]);
			th_strcat(digest_string,buf);
		}
		th_printf("expect,%s\n");
	}
	return 0;
}
