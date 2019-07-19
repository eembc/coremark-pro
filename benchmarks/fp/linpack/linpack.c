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
** Linpack **
** Solve linear equations.
** Adapted and modified from the code in linpackc.
*************************/
#include "th_cfg.h"
#include "th_types.h"
#include "th_math.h" /* for sin, cos and pow */
#include "th_lib.h"
#include "th_rand.h" /* initialize a random data vector */
#include "linpack.h"
#define DEBUG_LINPACK 0
#if DEBUG_LINPACK || BMDEBUG
static int dbgi=1;
#endif

linpack_params presets_linpack[NUM_DATAS];

/* ======================================================================== */
/*         FALSE U N C TRUE I O N   P R O TRUE O TRUE Y P E S                            */
/* ======================================================================== */
/* file provides :
define - init base input params (segment [within 0..2 boundary], number of coefficients to calculate, number of integration steps)
init -  allocate working memory, assign a[0] and b[0] . 
run - calculate N coefficients in the segment
fini - calculate avg of coeffients then dealloc working memory
verify - SNR average of coefficients vs golden reference, must run at least base iterations for valid output.
clean - deallocate output memory
*/
void *define_params_linpack(unsigned int idx, char *name, char *dataset);
void *bmark_init_linpack(void *);
void *t_run_test_linpack(struct TCDef *,void *);
int bmark_clean_linpack(void *);
int bmark_verify_linpack(void *in_params);
void *bmark_fini_linpack(void *in_params);

/* benchmark function declarations */
static void linpack_first(linpack_params *params);
static e_fp run_single_linpack(int outer,int inner,int lda,linpack_params *params);
static void run_linpack(linpack_params *params);
static void matgen(e_fp *a,int lda,int n,e_fp *b,e_fp *norma,e_u32 init, linpack_params *params);
static void dgefa(e_fp *a,int lda,int n,e_u16 ipvt[],int *info);
static void dgesl(e_fp * RESTRICT a,int lda,int n,e_u16 ipvt[],e_fp * RESTRICT b,int job);
static void daxpy(int n,e_fp da,e_fp * RESTRICT dx,int incx,e_fp * RESTRICT dy,int incy);
static e_fp ddot(int n,e_fp *dx,int incx,e_fp *dy,int incy);
static void dscal(int n,e_fp da,e_fp *dx,int incx);
static int idamax(int n,e_fp *dx,int incx);
static e_fp epslon (e_fp x);
static void dmxpy (int n1, e_fp * RESTRICT y, int n2, int ldm, e_fp * RESTRICT x, e_fp * RESTRICT m);

void *define_params_linpack(unsigned int idx, char *name, char *dataset) {
    linpack_params *params;
	e_u32 data_index=idx;
	init_presets_linpack();
	

	/* parameter setup */
	params=(linpack_params *)th_calloc(1,sizeof(linpack_params));
	if ( params == NULL )
          th_exit( THE_OUT_OF_MEMORY, "Cannot Allocate Memory %s:%d", __FILE__,__LINE__ );

	params->gen_ref=0;
	params->seed=73686179;
	params->ref_data=NULL;
	params->n=100;  /* original linpack value */
	params->ntimes=10; /* original linpack value */
	params->random_mask=0xff; /* 256 random values */
	
	if (pgo_training_run) {
		data_index=5; 
	} else {
		th_parse_buf_flag_unsigned(dataset,"-i",&data_index);
	}
	/* preset datasets */
	if (data_index<NUM_DATAS) {
		th_memcpy(params,&(presets_linpack[data_index]),sizeof(linpack_params));
	} 
	/* command line overrides */
	if (pgo_training_run==0) {
		th_parse_buf_flag_unsigned(dataset,"-s",&params->seed);
		th_parse_buf_flag_unsigned(dataset,"-n",&params->n);
		th_parse_buf_flag_unsigned(dataset,"-l",&params->ntimes);
		th_parse_buf_flag(dataset,"-g",&params->gen_ref);
	}
	params->lda=1+params->n;

#if USE_FP64
	params->random_values=fromint_fp_vector(1+params->random_mask,params->seed);
	params->minbits=MIN_ACC_BITS_FP64;
#elif USE_FP32
	params->random_values=fromint_fp_01_vector(1+params->random_mask,params->seed);
	params->minbits=11;
#else
	params->random_values=fromint_fp_01_vector(1+params->random_mask,params->seed);
	params->minbits=MIN_ACC_BITS_OTHER;
#endif

	linpack_first(params);
	
	return params;
}
int bmark_clean_linpack(void *in_params) {
	linpack_params *params=(linpack_params *)in_params;
	if (!params)
		return 0;
	if (params->random_values)
		th_free(params->random_values);
	th_free(params);
	return 1;
}

void *bmark_init_linpack(void *in_params) {
	/* Create a params for this invocation */
	linpack_params *params=(linpack_params *)in_params;
    linpack_params *myparams;
	int lda=params->lda;
	if (in_params==NULL)
		th_exit( THE_BAD_PTR, "Invalid pointer %s:%d", __FILE__,__LINE__ );
	myparams=(linpack_params *)th_calloc(1,sizeof(linpack_params));
	if ( myparams == NULL )
        th_exit( THE_OUT_OF_MEMORY, "Cannot Allocate Memory %s:%d", __FILE__,__LINE__ );
	th_memcpy(myparams,params,sizeof(linpack_params));
	/* Allocate working buffers */
	myparams->a=(e_fp *)th_malloc(sizeof(e_fp)*(lda-1)*lda);
	myparams->b=(e_fp *)th_malloc(sizeof(e_fp)*(lda-1));
	myparams->ipvt=(e_u16 *)th_malloc(sizeof(e_u16)*(lda-1));
	if (( myparams->a == NULL ) || ( myparams->b == NULL ) || ( myparams->ipvt == NULL ))
        th_exit( THE_OUT_OF_MEMORY, "Cannot Allocate Memory %s:%d", __FILE__,__LINE__ );

	return myparams;
}
void *bmark_fini_linpack(void *in_params) {
    linpack_params *params;
	if (in_params==NULL)
		th_exit( THE_BAD_PTR, "Invalid pointer %s:%d", __FILE__,__LINE__ );
	params=(linpack_params *)in_params;
	/* Cleanup working buffers */
	if (params->a!=NULL)
		th_free(params->a);
	if (params->b!=NULL)
		th_free(params->b);
	if (params->ipvt!=NULL)
		th_free(params->ipvt);
	th_free(params);

	return NULL;
}
int snr_set(linpack_params *params, e_u32 accbits)
{
	if (params->min_snr>accbits)
		params->min_snr=accbits;
	if (params->max_snr<accbits)
		params->max_snr=accbits;
	if (accbits>params->minbits)
		return 0;
	else 
		return 1;
}

void *t_run_test_linpack(struct TCDef *tcdef,void *in_params) {
	linpack_params *params=(linpack_params *)in_params;
	e_u32 test1=0,test2=0;
	
	tcdef->CRC=0;
	params->r=rand_init(params->seed,256,-1e10,1e10);
	run_linpack(params);
	rand_fini(params->r);
	
	/* If ref data available, do a quick check */
	if (!params->gen_ref) {
		if (params->ref_data) {
			test1=fp_iaccurate_bits(params->ret_lda,&params->ref_data->iret_lda);
			if (test1<params->minbits)
				tcdef->CRC++;
			test2=fp_iaccurate_bits(params->ret_ldaa,&params->ref_data->iret_ldaa);
			if (test2<params->minbits)
				tcdef->CRC++;
		}
	}
	
	/* Setup values */
	tcdef->v1=test1;
	tcdef->v2=test2;
	tcdef->v3=params->n;
	tcdef->v4=params->ntimes;
	/* Result values */
	tcdef->dbl_data[0]=params->ret_lda;
	tcdef->dbl_data[1]=params->ret_ldaa;
	
	return tcdef;
}

int bmark_verify_linpack(void *in_params) {
	int faults=0;
	linpack_params *params=(linpack_params *)in_params;
	e_u32 test;

	if (params->gen_ref) {
		char sbuf[256];
		th_printf("/**** START DATASET ****/\n#include \"th_lib.h\"\n#include \"../linpack.h\"\n");
		th_printf("static linpack_params in_data_index={\n\t"); 
		th_printf("%d,%d,%d,%u,0,\n\tNULL,NULL,NULL,NULL,\n\tNULL,\n\t",
		params->lda,params->n,params->ntimes,params->seed);
		th_printf("%s,\n\t",th_sprint_fp(params->ret_lda,sbuf));
		th_printf("%s,\n\t",th_sprint_fp(params->ret_ldaa,sbuf));
		th_printf("%s,\n\t",th_sprint_fp(params->resid,sbuf));
		th_printf("%s,\n\t",th_sprint_fp(params->eps,sbuf));
		th_printf("%s,\n\t",th_sprint_fp(FPCONST(1.0),sbuf));
		th_printf("%s,\n\t",th_sprint_fp(FPCONST(1.0),sbuf));
		th_printf("%s,\n\t0,0,\n\t0x%08x,NULL,MIN_ACC_BITS_FP,\n\t0.0,0.0,0.0,0.0,0.0,0.0,0.0",th_sprint_fp(params->residn,sbuf),params->random_mask);
		th_printf("}; //ref input data\n\n"); 
		th_printf("void init_preset(int idx) {\n");
		th_printf("th_memcpy(presets_linpack[idx],in_data_index,sizeof(linpack_params));\n");
		th_printf("presets_linpack[idx].ref_data=in_data_index;\n}");
		th_printf("/**** END DATASET ****/\n");
	} else {
		params->min_snr=999;
		params->max_snr=0;
		if (params->ref_data==NULL) {
			th_printf("Cannot validate this dataset! Please generate reference data on reliable hardware and tools using -g!\n");
			return -1;
		}
		test=fp_iaccurate_bits(params->ret_lda,&params->ref_data->iret_lda);
		faults += snr_set(params,test);
		test=fp_iaccurate_bits(params->ret_ldaa,&params->ref_data->iret_ldaa);
		faults += snr_set(params,test);
#ifdef VERIFY_LINEAR_RESIDUALS
		test=fp_iaccurate_bits(params->resid,&params->ref_data->iresid);
		faults += snr_set(params,test);
		test=fp_iaccurate_bits(params->residn,&params->ref_data->iresidn);
		faults += snr_set(params,test);
		test=fp_iaccurate_bits(params->eps,&params->ref_data->ieps);
		faults += snr_set(params,test);
#endif
		test=fp_iaccurate_bits(params->x0,&params->ref_data->ix0);
		faults += snr_set(params,test);
		test=fp_iaccurate_bits(params->xn,&params->ref_data->ixn);
		faults += snr_set(params,test);
	}
	return (faults==0);
}


static void linpack_first(linpack_params *params) {
	e_fp norma,normx;
	int lda=params->lda;
	int n=params->n;
	e_fp *a=(e_fp *)th_malloc(sizeof(e_fp)*(lda-1)*lda);
	e_fp *b=(e_fp *)th_malloc(sizeof(e_fp)*(lda-1));
	e_fp *x=(e_fp *)th_malloc(sizeof(e_fp)*(lda-1));
	e_u16 *ipvt=(e_u16 *)th_malloc(sizeof(e_u16)*(lda-1));
	int info,i;
/* initial run once to derive platform verification values - helps debug, does not affect timing */
	matgen(a,lda,n,b,&norma,1325,params);
#if BMDEBUG 
	th_printf("Input gen\n");
	for (info = 0; info < n; info++) {
		for (i = 0; i < n; i++) {
			th_printf("%1.18le,",a[info*lda+i]);
		}
		th_printf("\n");
	}
	th_printf("\n");
#endif
	dgefa(a,lda,n,ipvt,&info);
	dgesl(a,lda,n,ipvt,b,0);
/* compute a residual to verify results.  */ 
	for (i = 0; i < n; i++) {
		x[i] = b[i];
	}
	matgen(a,lda,n,b,&norma,1325,params); // seed must be the same as above for residual to verify.
	for (i = 0; i < n; i++) {
		b[i] = -b[i];
	}
	dmxpy(n,b,n,lda,x,a);
	params->resid = 0.0;
	normx = 0.0;
	for (i = 0; i < n; i++) {
			params->resid = (params->resid > th_fabs((e_fp)b[i])) 
		? params->resid : th_fabs((e_fp)b[i]);
			normx = (normx > th_fabs((e_fp)x[i])) 
		? normx : th_fabs((e_fp)x[i]);
#if BMDEBUG 
		th_printf("%d: %1.18le %1.18le %1.18le %1.18le\n",i, params->resid, b[i], normx, x[i]);
#endif
	}
	/* Save output values for verification for machine accuracy */
	params->eps = epslon((e_fp)ONE);
	params->residn = params->resid/( n*norma*normx*params->eps );
	params->x0=x[0];
	params->xn=x[n-1];
	th_free(a);
	th_free(b);
	th_free(x);
	th_free(ipvt);
}

static e_fp get_avg(e_fp *a, int maxidx) {
	int i;
	e_fp ret=0.0;
	for (i=0; i<maxidx; i++) {
		ret+=a[i];
#if BMDEBUG 
		th_printf("%d: %1.18le\n",dbgi++,a[i]);
#endif
	}
	return ret/(e_fp)maxidx;
}
static e_fp run_single_linpack(int outer, int inner, int lda, linpack_params *params) {
	e_fp *feedback=(e_fp *)th_malloc(sizeof(e_fp)*(outer+2*inner));
	e_fp ret=0.0,norma;
	e_fp *a=params->a,*b=params->b;
	int i,info;
	e_u16 *ipvt=params->ipvt,n=params->n;
	int j=0; /* feedback array index */

	for (i = 0; i < outer; i++) {
		/*  matgen generates a new input each time through. 
			make the input different each time so code does not get optimized away... */
		matgen(a,lda,n,b,&norma,random_u32(params->r),params);
		dgefa(a,lda,n,ipvt,&info); 
		dgesl(a,lda,n,ipvt,b,0);
		/* Now account for the final values calculated */
		feedback[j++]=get_avg(b,n);
	}
	for (i = 0; i < inner; i++) {
		matgen(a,lda,n,b,&norma,random_u32(params->r),params);
		dgefa(a,lda,n,ipvt,&info);
		/* Since only the last pivots will be used, make sure to collect something from all pivots, 
			otherwise a smart compiler can eliminate all but the last iteration */
		feedback[j++]=(e_fp)(th_crcbuffer(ipvt,sizeof(e_u16)*n,0)/32768)+norma; 
	}
	for (i = 0; i < inner; i++) {
		dgesl(a,lda,n,ipvt,b,0);
		feedback[j++]=get_avg(b,n);
	}
	/* now compute a value that is affected by all computed values */
	ret=get_avg(feedback,j);
	th_free(feedback);
	return ret;
}

static void run_linpack(linpack_params *params)
{
	params->ret_lda=run_single_linpack(params->ntimes,0,params->lda,params);
	params->ret_ldaa=run_single_linpack(params->ntimes+1,0,params->lda-1,params);
}

static void matgen(e_fp *a,int lda,int n,e_fp *b,e_fp *norma,e_u32 init, linpack_params *params)
/* In this function, references to a[i][j] are written a[lda*j+i].  Actual random generation taken out of the timing loop, just selecting precomputed values. */
{
	int i, j;
	*norma = 0.0;
	for (j = 0; j < n; j++) {
		for (i = 0; i < n; i++) {
			a[lda*j+i] = params->random_values[params->random_mask & init++];
			*norma = (a[lda*j+i] > *norma) ? a[lda*j+i] : *norma;
		}
	}
	for (i = 0; i < n; i++) {
          b[i] = 0.0;
	}
	for (j = 0; j < n; j++) {
		for (i = 0; i < n; i++) {
			b[i] = b[i] + a[lda*j+i];
		}
	}
}

/*----------------------*/ 
static void dgefa(e_fp *a,int lda,int n,e_u16 ipvt[],int *info)
/* In this function, references to a[i][j] are written a[lda*i+j].  */
/*
     dgefa factors a e_fp precision matrix by gaussian elimination.

     dgefa is usually called by dgeco, but it can be called
     directly with a saving in time if  rcond  is not needed.
     (time for dgeco) = (1 + 9/n)*(time for dgefa) .

     on entry

        a       e_fp precision[n][lda]
                the matrix to be factored.

        lda     integer
                the leading dimension of the array  a .

        n       integer
                the order of the matrix  a .

     on return

        a       an upper triangular matrix and the multipliers
                which were used to obtain it.
                the factorization can be written  a = l*u  where
                l  is a product of permutation and unit lower
                triangular matrices and  u  is upper triangular.

        ipvt    integer[n]
                an integer vector of pivot indices.

        info    integer
                = 0  normal value.
                = k  if  u[k][k] .eq. 0.0 .  this is not an error
                     condition for this subroutine, but it does
                     indicate that dgesl or dgedi will divide by zero
                     if called.  use  rcond  in dgeco for a reliable
                     indication of singularity.

     linpack. this version dated 08/14/78 .
     cleve moler, university of new mexico, argonne national lab.

     functions

     blas daxpy,dscal,idamax
*/

{
/*     internal variables	*/

e_fp t;
int idamax(),j,k,kp1,l,nm1;


/*     gaussian elimination with partial pivoting	*/

	*info = 0;
	nm1 = n - 1;
	if (nm1 >=  0) {
		for (k = 0; k < nm1; k++) {
			kp1 = k + 1;

          		/* find l = pivot index	*/

			l = idamax(n-k,&a[lda*k+k],1) + k;
			ipvt[k] = l;

			/* zero pivot implies this column already 
			   triangularized */

			if (a[lda*k+l] != ZERO) {

				/* interchange if necessary */

				if (l != k) {
					t = a[lda*k+l];
					a[lda*k+l] = a[lda*k+k];
					a[lda*k+k] = t; 
				}

				/* compute multipliers */

				t = -ONE/a[lda*k+k];
				dscal(n-(k+1),t,&a[lda*k+k+1],1);

				/* row elimination with column indexing */

				for (j = kp1; j < n; j++) {
					t = a[lda*j+l];
					if (l != k) {
						a[lda*j+l] = a[lda*j+k];
						a[lda*j+k] = t;
					}
					daxpy(n-(k+1),t,&a[lda*k+k+1],1,
					      &a[lda*j+k+1],1);
  				} 
  			}
			else { 
            			*info = k;
			}
		} 
	}
	ipvt[n-1] = n-1;
	if (a[lda*(n-1)+(n-1)] == ZERO) *info = n-1;
}

/*----------------------*/ 

static void dgesl(e_fp * RESTRICT a,int lda,int n,e_u16 ipvt[],e_fp * RESTRICT b,int job)
/* In this function, references to a[i][j] are written a[lda*i+j].  */
/*
     dgesl solves the e_fp precision system
     a * x = b  or  trans(a) * x = b
     using the factors computed by dgeco or dgefa.

     on entry

        a       e_fp precision[n][lda]
                the output from dgeco or dgefa.

        lda     integer
                the leading dimension of the array  a .

        n       integer
                the order of the matrix  a .

        ipvt    integer[n]
                the pivot vector from dgeco or dgefa.

        b       e_fp precision[n]
                the right hand side vector.

        job     integer
                = 0         to solve  a*x = b ,
                = nonzero   to solve  trans(a)*x = b  where
                            trans(a)  is the transpose.

    on return

        b       the solution vector  x .

     error condition

        a division by zero will occur if the input factor contains a
        zero on the diagonal.  technically this indicates singularity
        but it is often caused by improper arguments or improper
        setting of lda .  it will not occur if the subroutines are
        called correctly and if dgeco has set rcond .gt. 0.0
        or dgefa has set info .eq. 0 .

     to compute  inverse(a) * c  where  c  is a matrix
     with  p  columns
           dgeco(a,lda,n,ipvt,rcond,z)
           if (!rcond is too small){
           	for (j=0,j<p,j++)
              		dgesl(a,lda,n,ipvt,c[j][0],0);
	   }

     linpack. this version dated 08/14/78 .
     cleve moler, university of new mexico, argonne national lab.

     functions

     blas daxpy,ddot
*/
{
/*     internal variables	*/

	e_fp ddot(),t;
	int k,kb,l,nm1;

	nm1 = n - 1;
	if (job == 0) {

		/* job = 0 , solve  a * x = b
		   first solve  l*y = b    	*/

		if (nm1 >= 1) {
			for (k = 0; k < nm1; k++) {
				l = ipvt[k];
				t = b[l];
				if (l != k){ 
					b[l] = b[k];
					b[k] = t;
				}	
				daxpy(n-(k+1),t,&a[lda*k+k+1],1,&b[k+1],1);
			}
		} 

		/* now solve  u*x = y */

		for (kb = 0; kb < n; kb++) {
		    k = n - (kb + 1);
		    b[k] = b[k]/a[lda*k+k];
		    t = -b[k];
		    daxpy(k,t,&a[lda*k+0],1,&b[0],1);
		}
	}
	else { 

		/* job = nonzero, solve  trans(a) * x = b
		   first solve  trans(u)*y = b 			*/

		for (k = 0; k < n; k++) {
			t = ddot(k,&a[lda*k+0],1,&b[0],1);
			b[k] = (b[k] - t)/a[lda*k+k];
		}

		/* now solve trans(l)*x = y	*/

		if (nm1 >= 1) {
			for (kb = 1; kb < nm1; kb++) {
				k = n - (kb+1);
				b[k] = b[k] + ddot(n-(k+1),&a[lda*k+k+1],1,&b[k+1],1);
				l = ipvt[k];
				if (l != k) {
					t = b[l];
					b[l] = b[k];
					b[k] = t;
				}
			}
		}
	}
}

/*----------------------*/ 

static void daxpy(int n,e_fp da,e_fp * RESTRICT dx,int incx,e_fp * RESTRICT dy,int incy)
/*
     constant times a vector plus a vector.
     jack dongarra, linpack, 3/11/78.
*/
{
	int i,ix,iy;

	if(n <= 0) return;
	if (da == ZERO) return;

	if(incx != 1 || incy != 1) {

		/* code for unequal increments or equal increments
		   not equal to 1 					*/

		ix = 0;
		iy = 0;
		if(incx < 0) ix = (-n+1)*incx;
		if(incy < 0)iy = (-n+1)*incy;
		for (i = 0;i < n; i++) {
			dy[iy] = dy[iy] + da*dx[ix];
			ix = ix + incx;
			iy = iy + incy;
		}
      		return;
	}

	/* code for both increments equal to 1 */
	for (i = 0;i < n; i++) {
		dy[i] = dy[i] + da*dx[i];
	}
}
   
/*----------------------*/ 

static e_fp ddot(int n,e_fp *dx,int incx,e_fp *dy,int incy)
/*
     forms the dot product of two vectors.
     jack dongarra, linpack, 3/11/78.
*/
{
	e_fp dtemp;
	int i,ix,iy;

	dtemp = ZERO;

	if(n <= 0) return(ZERO);

	if(incx != 1 || incy != 1) {

		/* code for unequal increments or equal increments
		   not equal to 1					*/

		ix = 0;
		iy = 0;
		if (incx < 0) ix = (-n+1)*incx;
		if (incy < 0) iy = (-n+1)*incy;
		for (i = 0;i < n; i++) {
			dtemp = dtemp + dx[ix]*dy[iy];
			ix = ix + incx;
			iy = iy + incy;
		}
		return(dtemp);
	}

	/* code for both increments equal to 1 */

	for (i=0;i < n; i++)
		dtemp = dtemp + dx[i]*dy[i];
	return(dtemp);
}

/*----------------------*/ 
static void dscal(int n,e_fp da,e_fp *dx,int incx)
/*     scales a vector by a constant.
      jack dongarra, linpack, 3/11/78.
*/
{
	int i,nincx;

	if(n <= 0)return;
	if(incx != 1) {
		/* code for increment not equal to 1 */
		nincx = n*incx;
		for (i = 0; i < nincx; i = i + incx)
			dx[i] = da*dx[i];
		return;
	}
	/* code for increment equal to 1 */
	for (i = 0; i < n; i++)
		dx[i] = da*dx[i];
}

/*----------------------*/ 
static int idamax(int n,e_fp *dx,int incx)

/*
     finds the index of element having max. absolute value.
     jack dongarra, linpack, 3/11/78.
*/
{
	e_fp dmax;
	int i, ix, itemp;

	if( n < 1 ) return(-1);
	if(n ==1 ) return(0);
	if(incx != 1) {

		/* code for increment not equal to 1 */

		ix = 0;
		dmax = th_fabs((e_fp)dx[0]);
		ix = ix + incx;
		for (i = 1; i < n; i++) {
			if(th_fabs((e_fp)dx[ix]) > dmax)  {
				itemp = i;
				dmax = th_fabs((e_fp)dx[ix]);
			}
			ix = ix + incx;
		}
	}
	else {

		/* code for increment equal to 1 */

		itemp = 0;
		dmax = th_fabs((e_fp)dx[0]);
		for (i = 1; i < n; i++) {
			if(th_fabs((e_fp)dx[i]) > dmax) {
				itemp = i;
				dmax = th_fabs((e_fp)dx[i]);
			}
		}
	}
	return (itemp);
}

/*----------------------*/ 
static e_fp epslon (e_fp x)
/*
     estimate unit roundoff in quantities of size x.
*/

{
	e_fp a,b,c,eps;
/*
     this program should function properly on all systems
     satisfying the following two assumptions,
        1.  the base used in representing dfloating point
            numbers is not a power of three.
        2.  the quantity  a  in statement 10 is represented to 
            the accuracy used in dfloating point variables
            that are stored in memory.
     the statement number 10 and the go to 10 are intended to
     force optimizing compilers to generate code satisfying 
     assumption 2.
     under these assumptions, it should be true that,
            a  is not exactly equal to four-thirds,
            b  has a zero for its last bit or digit,
            c  is not exactly equal to one,
            eps  measures the separation of 1.0 from
                 the next larger dfloating point number.
     the developers of eispack would appreciate being informed
     about any systems where these assumptions do not hold.

     *****************************************************************
     this routine is one of the auxiliary routines used by eispack iii
     to avoid machine dependencies.
     *****************************************************************

     this version dated 4/6/83.
*/

	a = FPCONST(4.0e0)/FPCONST(3.0e0);
	eps = ZERO;
	while (eps == ZERO) {
		b = a - ONE;
		c = b + b + b;
		eps = th_fabs((e_fp)(c-ONE));
	}
	return(eps*th_fabs((e_fp)x));
}
 
/*----------------------*/ 
static void dmxpy (int n1, e_fp * RESTRICT y, int n2, int ldm, e_fp * RESTRICT x, e_fp * RESTRICT m)

/* In this function, references to m[i][j] are written m[ldm*i+j].  */

/*
   purpose:
     multiply matrix m times vector x and add the result to vector y.

   parameters:

     n1 integer, number of elements in vector y, and number of rows in
         matrix m

     y e_fp [n1], vector of length n1 to which is added 
         the product m*x

     n2 integer, number of elements in vector x, and number of columns
         in matrix m

     ldm integer, leading dimension of array m

     x e_fp [n2], vector of length n2

     m e_fp [ldm][n2], matrix of n1 rows and n2 columns

 ----------------------------------------------------------------------
*/
{
	int j,i,jmin;
	/* cleanup odd vector */

	j = n2 % 2;
	if (j >= 1) {
		j = j - 1;
		for (i = 0; i < n1; i++) 
            		y[i] = (y[i]) + x[j]*m[ldm*j+i];
	} 

	/* cleanup odd group of two vectors */

	j = n2 % 4;
	if (j >= 2) {
		j = j - 1;
		for (i = 0; i < n1; i++)
            		y[i] = ( (y[i])
                  	       + x[j-1]*m[ldm*(j-1)+i]) + x[j]*m[ldm*j+i];
	} 

	/* cleanup odd group of four vectors */

	j = n2 % 8;
	if (j >= 4) {
		j = j - 1;
		for (i = 0; i < n1; i++)
			y[i] = ((( (y[i])
			       + x[j-3]*m[ldm*(j-3)+i]) 
			       + x[j-2]*m[ldm*(j-2)+i])
			       + x[j-1]*m[ldm*(j-1)+i]) + x[j]*m[ldm*j+i];
	} 

	/* cleanup odd group of eight vectors */

	j = n2 % 16;
	if (j >= 8) {
		j = j - 1;
		for (i = 0; i < n1; i++)
			y[i] = ((((((( (y[i])
			       + x[j-7]*m[ldm*(j-7)+i]) + x[j-6]*m[ldm*(j-6)+i])
		  	       + x[j-5]*m[ldm*(j-5)+i]) + x[j-4]*m[ldm*(j-4)+i])
			       + x[j-3]*m[ldm*(j-3)+i]) + x[j-2]*m[ldm*(j-2)+i])
			       + x[j-1]*m[ldm*(j-1)+i]) + x[j]  *m[ldm*j+i];
	} 
	
	/* main loop - groups of sixteen vectors */

	jmin = (n2%16)+16;
	for (j = jmin-1; j < n2; j = j + 16) {
		for (i = 0; i < n1; i++) 
			y[i] = ((((((((((((((( (y[i])
			       	+ x[j-15]*m[ldm*(j-15)+i]) 
				+ x[j-14]*m[ldm*(j-14)+i])
			        + x[j-13]*m[ldm*(j-13)+i]) 
				+ x[j-12]*m[ldm*(j-12)+i])
			        + x[j-11]*m[ldm*(j-11)+i]) 
				+ x[j-10]*m[ldm*(j-10)+i])
			        + x[j- 9]*m[ldm*(j- 9)+i]) 
				+ x[j- 8]*m[ldm*(j- 8)+i])
			        + x[j- 7]*m[ldm*(j- 7)+i]) 
				+ x[j- 6]*m[ldm*(j- 6)+i])
			        + x[j- 5]*m[ldm*(j- 5)+i]) 
				+ x[j- 4]*m[ldm*(j- 4)+i])
			        + x[j- 3]*m[ldm*(j- 3)+i]) 
				+ x[j- 2]*m[ldm*(j- 2)+i])
			        + x[j- 1]*m[ldm*(j- 1)+i]) 
				+ x[j]   *m[ldm*j+i];
	}
} 




