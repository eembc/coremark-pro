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
** FFT radix 2 **
** Perform FFT with radix2.
*************************/
#include "th_cfg.h"
#include "th_math.h" /* for sin, cos and pow */
#include "th_lib.h"
#include "th_rand.h" /* initialize a random data vector */
#include "fft_radix2.h"

//PT: 4/4/2016: w/g decision (undoes th_lib.h for radix only)
#undef  MIN_ACC_BITS_FP64    
#define MIN_ACC_BITS_FP64 25

extern void init_preset_0();
extern void init_preset_1();
extern void init_preset_2();
extern void init_preset_3();
extern void init_preset_4();
extern void init_preset_5();
radix2_params presets_radix2[NUM_DATAS];

/* ======================================================================== */
/*         F U N C T I O N   P R O T O T Y P E S                            */
/* ======================================================================== */
/* file provides :
define - init base input params (segment [within 0..2 boundary], number of coefficients to calculate, number of integration steps)
init -  allocate working memory, assign a[0] and b[0] . 
run - calculate N coefficients in the segment
fini - calculate avg of coeffients then dealloc working memory
verify - SNR average of coefficients vs golden reference, must run at least base iterations for valid output.
clean - deallocate output memory
*/
void *define_params_radix2(unsigned int idx, char *name, char *dataset);
void *bmark_init_radix2(void *);
void *t_run_test_radix2(struct TCDef *,void *);
int bmark_clean_radix2(void *);
int bmark_verify_radix2(void *in_params);
void *bmark_fini_radix2(void *in_params);

/* benchmark function declarations */
static void FFT_transform_internal (int N, e_fp * RESTRICT data, int direction, e_fp *twp);
static void FFT_bitreverse(int N, e_fp * RESTRICT data);
static int int_log2 (int n)
{
    int k = 1;
    int log = 0;
    for(/*k=1*/; k < n; k *= 2, log++);
    if (n != (1 << log))
      th_exit(1,"ERROR: FFT radix2, Data length is not a power of 2! [%d]\n",n);
    return log; 
}

static e_fp *calculate_twiddles(int size, int direction) {
	int n=0,a;
	e_fp *twp=NULL;
    int bit = 0;
    int logn;
    int dual = 1;
	
    if (size == 1) return twp;         
    logn = int_log2(size/2);
	
    for (bit = 0; bit < logn; bit++, dual *= 2) 
      for (a = 1; a < dual; a++) 
	    n++;
	twp=(e_fp *)th_malloc(sizeof(e_fp)*2*n);
	n=0;
	bit=0;
	dual=1;
    for (bit = 0; bit < logn; bit++, dual *= 2) {
		e_fp w_real = FPCONST(1.0);
		e_fp w_imag = FPCONST(0.0);

		e_fp theta = FPCONST(2.0) * direction * EE_PI / (FPCONST(2.0) * (e_fp) dual);
		e_fp s = th_sin(theta);
		e_fp t = th_sin(theta / FPCONST(2.0));
		e_fp s2 = FPCONST(2.0) * t * t;

		for (a = 1; a < dual; a++) {
			e_fp tmp_real = w_real - s * w_imag - s2 * w_real;
			e_fp tmp_imag = w_imag + s * w_real - s2 * w_imag;
			w_real = tmp_real;
			w_imag = tmp_imag;
			twp[n++] = w_real;
			twp[n++] = w_imag;
		}
	}
	return twp;
}
void *define_params_radix2(unsigned int idx, char *name, char *dataset) {
    radix2_params *params;
	e_s32 data_index=idx;
	init_preset_0();
	init_preset_1();
	init_preset_2();
	init_preset_3();
	init_preset_4();
	init_preset_5();

	/* parameter setup */
	params=(radix2_params *)th_calloc(1,sizeof(radix2_params));
	if ( params == NULL )
          th_exit( THE_OUT_OF_MEMORY, "Cannot Allocate Memory %s:%d", __FILE__,__LINE__ );

	params->N=2048; /* default */
	params->gen_ref=0;
	params->ref_min=intparts_zero;
	params->ref_max=intparts_zero;
	params->ref_avg=intparts_zero;
	params->ref_data=NULL;
	params->seed=0;
#if (USE_FP32)
	params->minbits=MIN_ACC_BITS_FP32;
#elif (USE_FP64)
	params->minbits=MIN_ACC_BITS_FP64;
#else
	params->minbits=MIN_ACC_BITS_OTHER;
#endif
	th_parse_buf_flag(dataset,"-i",&data_index);
	if (pgo_training_run!=0) {
		data_index=0; 
	}
	/* preset datasets */
	if ((data_index>=0) && (data_index<NUM_DATAS)) {
		params->N=presets_radix2[data_index].N; /* default */
		params->ref_min=presets_radix2[data_index].ref_min;
		params->ref_max=presets_radix2[data_index].ref_max;
		params->ref_avg=presets_radix2[data_index].ref_avg;
		params->ref_data=presets_radix2[data_index].ref_data;
		params->seed=presets_radix2[data_index].seed;
	} 
	/* command line overrides */
	if (pgo_training_run==0) {
		th_parse_buf_flag_unsigned(dataset,"-s",&params->seed);
		th_parse_buf_flag_unsigned(dataset,"-n",&params->N);
		th_parse_buf_flag(dataset,"-g",&params->gen_ref);
	}
	/* generate the data */
	params->data=fromint_fp_vector(params->N,params->seed); /* default */
	params->twp=calculate_twiddles(params->N,-1);
	
	return params;
}

int bmark_clean_radix2(void *in_params) {
	radix2_params *params=(radix2_params *)in_params;
	if (params) {
		if (params->data)
			th_free(params->data);
		if (params->twp)
			th_free(params->twp);
		th_free(params);
	}
	return 1;
}

void *bmark_init_radix2(void *in_params) {
	radix2_params *params=(radix2_params *)in_params;
    radix2_params *myparams;
	if (in_params==NULL)
		th_exit( THE_BAD_PTR, "Invalid pointer %s:%d", __FILE__,__LINE__ );
	myparams=(radix2_params *)th_calloc(1,sizeof(radix2_params));
	if ( myparams == NULL )
          th_exit( THE_OUT_OF_MEMORY, "Cannot Allocate Memory %s:%d", __FILE__,__LINE__ );
	th_memcpy(myparams,params,sizeof(radix2_params));

	myparams->data = (e_fp *)th_aligned_malloc(params->N*sizeof(e_fp), ALIGN_BOUNDARY);
	if ( myparams->data == NULL) {
		th_printf("%s:%d - cannot allocate working array!", __FILE__, __LINE__ );
		return NULL;
	}

	th_memcpy(myparams->data,params->data,params->N*sizeof(e_fp));
	
	return myparams;
}
void *bmark_fini_radix2(void *in_params) {
    radix2_params *params;
	if (in_params==NULL)
		th_exit( THE_BAD_PTR, "Invalid pointer %s:%d", __FILE__,__LINE__ );
	params=(radix2_params *)in_params;

	if (params->data)
		th_aligned_free(params->data);
	th_free(params);

	return NULL;
}

#if BMDEBUG
static void dump_data(int iter, int N, e_fp * RESTRICT data)
{
   int i;
   for (i=0;i<N;i++)
      th_printf("\t%d, %d, %1.18le\n", iter, i, data[i]);
}
#endif
void *t_run_test_radix2(struct TCDef *tcdef,void *in_params) {
	int i;
	e_u32 test;
	e_fp min=EE_MININI,max=EE_MAXINI,avg=FPCONST(0.0);
	radix2_params *params=(radix2_params *)in_params;
	FFT_transform_internal(params->N, params->data, -1, params->twp);
	for (i=0; i<params->N ; i++) {
		min=(min>params->data[i])?params->data[i]:min;
		max=(max<params->data[i])?params->data[i]:max;
		avg+=params->data[i];
	}
	avg/=params->N;
	
	test=fp_iaccurate_bits(avg,&params->ref_avg);
	if (test>=params->minbits)
		tcdef->CRC=0;
	else 
		tcdef->CRC=1;
		
	tcdef->v1=test;
	tcdef->dbl_data[0]=avg;
	tcdef->dbl_data[1]=min;
	tcdef->dbl_data[2]=max;
	return tcdef;
}

int bmark_verify_radix2(void *in_params) {
	int i;
	radix2_params *params=(radix2_params *)in_params;
	if (params->gen_ref) {
		e_fp min=EE_MININI,max=EE_MAXINI,avg=FPCONST(0.0);
		char sbuf[256];
		th_printf("/**** START DATASET ****/\n#include \"th_lib.h\"\n#include \"../fft_radix2.h\"\n");
		th_printf("static const intparts ref_data_index[]={\n"); 
		for (i=0; i<params->N; i++) {
			th_printf("\t");
			th_print_fp(params->data[i]);
			if (i<params->N-1)
				th_printf(",\n");
			min=(min>params->data[i])?params->data[i]:min;
			max=(max<params->data[i])?params->data[i]:max;
			avg+=params->data[i];
		}
		th_printf("}; //ref_data\n\n"); 
		avg/=params->N;
		th_printf("static intparts ref_avg=%s;\n",th_sprint_fp(avg,sbuf));
		th_printf("static intparts ref_max=%s;\n",th_sprint_fp(max,sbuf));
		th_printf("static intparts ref_min=%s;\n",th_sprint_fp(min,sbuf));
		th_printf("void init_preset_index() {\n");
		th_printf("presets_radix2[index].ref_data=(intparts *)ref_data_index;\npresets_radix2[index].N=%d;\n",params->N);
		th_printf("presets_radix2[index].seed=%d;\n",params->seed);
		th_printf("presets_radix2[index].ref_avg=ref_avg;\npresets_radix2[index].ref_max=ref_max;\npresets_radix2[index].ref_min=ref_min;\n}\n",params->ref_avg,params->ref_max,params->ref_min);
		th_printf("/**** END DATASET ****/\n");
	} else {
		snr_result resa;
		if (params->ref_data==NULL) {
			th_printf("Cannot validate this dataset! Please generate reference data on reliable hardware and tools using -g!\n");
			return -1;
		}
		resa.bmin_ok=params->minbits;
		ee_ifpbits_buffer(params->data,params->ref_data,params->N,&resa);
	
		if (!resa.pass) return 0;
	}
	return 1;
}



static void FFT_transform_internal (int N, e_fp * RESTRICT data, int direction, e_fp *twp) {
    int n = N/2;
    int bit = 0;
    int logn;
    int dual = 1;

    if (n == 1) return;         /* Identity operation! */
    logn = int_log2(n);

    if (N == 0) return;    

#if BMDEBUG
    dump_data(-1, N, data);
#endif
    /* bit reverse the input data for decimation in time algorithm */
    FFT_bitreverse(N, data) ;

    /* apply fft recursion */
    /* this loop executed int_log2(N) times */
    for (bit = 0; bit < logn; bit++, dual *= 2) {
      int a;
      int b;
      e_fp w_real;
      e_fp w_imag;
#if (TWIDDLES_ON_THE_FLY) 
      e_fp theta = FPCONST(2.0) * direction * EE_PI / (FPCONST(2.0) * (e_fp) dual);
      e_fp s = th_sin(theta);
      e_fp t = th_sin(theta / FPCONST(2.0));
      e_fp s2 = FPCONST(2.0) * t * t;
	  w_real = FPCONST(1.0);
	  w_imag = FPCONST(0.0);
#endif

      for (a=0, b = 0; b < n; b += 2 * dual) {
        int i = 2*b ;
        int j = 2*(b + dual);

        e_fp wd_real = data[j] ;
        e_fp wd_imag = data[j+1] ;
          
        data[j]   = data[i]   - wd_real;
        data[j+1] = data[i+1] - wd_imag;
        data[i]  += wd_real;
        data[i+1]+= wd_imag;
      }
      
      /* a = 1 .. (dual-1) */
      for (a = 1; a < dual; a++) {
        /* trignometric recurrence for w-> exp(i theta) w */
#if (TWIDDLES_ON_THE_FLY) 
          e_fp tmp_real = w_real - s * w_imag - s2 * w_real;
          e_fp tmp_imag = w_imag + s * w_real - s2 * w_imag;
          w_real = tmp_real;
          w_imag = tmp_imag;
#else
			w_real=*twp++;
			w_imag=*twp++;
#endif

        for (b = 0; b < n; b += 2 * dual) {
          int i = 2*(b + a);
          int j = 2*(b + a + dual);

          e_fp z1_real = data[j];
          e_fp z1_imag = data[j+1];
              
          e_fp wd_real = w_real * z1_real - w_imag * z1_imag;
          e_fp wd_imag = w_real * z1_imag + w_imag * z1_real;

          data[j]   = data[i]   - wd_real;
          data[j+1] = data[i+1] - wd_imag;
          data[i]  += wd_real;
          data[i+1]+= wd_imag;
        }
      }
    }
#if BMDEBUG
    dump_data(bit, N, data);
#endif
}


static void FFT_bitreverse(int N, e_fp * RESTRICT data) {
    /* This is the Goldrader bit-reversal algorithm */
    int n=N/2;
    int nm1 = n-1;
    int i=0; 
    int j=0;
    for (; i < nm1; i++) {

      /*int ii = 2*i; */
      int ii = i << 1;

      /*int jj = 2*j; */
      int jj = j << 1;

      /* int k = n / 2 ; */
      int k = n >> 1;

      if (i < j) {
        e_fp tmp_real    = data[ii];
        e_fp tmp_imag    = data[ii+1];
        data[ii]   = data[jj];
        data[ii+1] = data[jj+1];
        data[jj]   = tmp_real;
        data[jj+1] = tmp_imag; }

      while (k <= j) 
      {
        /*j = j - k ; */
        j -= k;

        /*k = k / 2 ;  */
        k >>= 1 ; 
      }
      j += k ;
    }
  }

#if NEED_INVERSE
static void FFT_inverse(int N, e_fp * RESTRICT data)
{
    int n = N/2;
    e_fp norm = FPCONST(0.0);
    int i=0;
	e_fp *twp=calculate_twiddles(N,+1);
    FFT_transform_internal(N, data, +1,twp);
	th_free(twp);

    /* Normalize */
    norm=FPCONST(1.0)/((e_fp) n);
    for(i=0; i<N; i++)
      data[i] *= norm;
  
}
#endif

