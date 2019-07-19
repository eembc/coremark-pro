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
** Neural net **
** Perform a neural net search for patterns.
** Adapted and modified from the code in nbench.
*************************/
#include "th_cfg.h"
#include "th_math.h" /* for sin, cos and pow */
#include "th_lib.h"
#include "th_rand.h" /* initialize a random data vector */
#include "nnet.h"

extern void init_preset_0();
nnet_params presets_nnet[NUM_DATAS];

/* ======================================================================== */
/* ======================================================================== */
/* file provides :
define - init base input params (segment [within 0..2 boundary], number of coefficients to calculate, number of integration steps)
init -  allocate working memory, assign a[0] and b[0] . 
run - calculate N coefficients in the segment
fini - calculate avg of coeffients then dealloc working memory
verify - SNR average of coefficients vs golden reference, must run at least base iterations for valid output.
clean - deallocate output memory
*/
void *define_params_nnet(unsigned int idx, char *name, char *dataset);
void *bmark_init_nnet(void *);
void *t_run_test_nnet(struct TCDef *,void *);
int bmark_clean_nnet(void *);
int bmark_verify_nnet(void *in_params);
void *bmark_fini_nnet(void *in_params);

/* benchmark function declarations */
static e_u32 DoNNetIteration(nnet_params *params);
static void  do_mid_forward(int patt,nnet_params *params);
static void  do_out_forward(nnet_params *params);
static void  do_forward_pass(int patt,nnet_params *params);
static void do_out_error(int patt,nnet_params *params);
static void  worst_pass_error(nnet_params *params);
static void do_mid_error(nnet_params *params);
static void adjust_out_wts(nnet_params *params);
static void adjust_mid_wts(int patt,nnet_params *params);
static void  do_back_pass(int patt,nnet_params *params);
static void move_wt_changes(nnet_params *params);
static int check_out_error(nnet_params *params);
static void zero_changes(nnet_params *params);
static void randomize_wts(nnet_params *params);

void init_random_patterns(nnet_params *params) {
	int i,m,k=0;
	int inpat_size=params->d_y * params->d_x;
	e_fp *rvals=fromint_fp_01_vector(1+0x0fff,params->seed);
	e_fp *ivals=fromint_fp_01_vector(params->n_in,params->seed);
	
	for (i=0; i<params->n_in; i++) {
		for (m=0; m<inpat_size; m++) {
			params->in_pats[i][m] = (rvals[k++ & 0xfff]>ivals[i]) ? FPCONST(0.9):FPCONST(0.1);
		}
		for (m=0; m<params->d_out; m++) {
			params->out_pats[i][m] = (((i>>m) & 1) == 1) ? FPCONST(1.0) : FPCONST(0.0);
		}
	}
	th_free(ivals);
	th_free(rvals);
}

void *define_params_nnet(unsigned int idx, char *name, char *dataset) {
    nnet_params *params;
	e_s32 data_index=idx;
	init_preset_0();
	init_preset_1();

	/* parameter setup */
	params=(nnet_params *)th_aligned_malloc(sizeof(nnet_params),ALIGN_BOUNDARY);
	if ( params == NULL )
          th_exit( THE_OUT_OF_MEMORY, "Cannot Allocate Memory %s:%d", __FILE__,__LINE__ );

	th_memset(params,0,sizeof(nnet_params));

	params->gen_ref=0;
	params->seed=0;
	params->ref_data=NULL;
	params->next_ridx=0;
	params->n_in=0;
	
	if (pgo_training_run) {
		data_index=1; 
	} else {
		th_parse_buf_flag(dataset,"-i",&data_index);
	}
	/* preset datasets */
	if ((data_index>=0) && (data_index<NUM_DATAS)) {
		th_memcpy(params,&(presets_nnet[data_index]),sizeof(nnet_params));
	} 
	/* command line overrides */
	if (pgo_training_run==0) {
		th_parse_buf_flag_unsigned(dataset,"-s",&params->seed);
		th_parse_buf_flag(dataset,"-n",&params->loops);
		th_parse_buf_flag(dataset,"-g",&params->gen_ref);
	}
	if (params->n_in > MAX_PATTERNS) {
		th_printf("ERROR: MAX_PATTERNS is set to 26. Must be modified in the header file for this dataset!\n");
		th_free(params);
		return NULL;
	}

	params->random_values=fromint_fp_01_vector(1+0x0ff,params->seed);
#if (USE_FP32)
	params->minbits=MIN_ACC_BITS_FP32;
#elif (USE_FP64)
	params->minbits=MIN_ACC_BITS_FP64;
#else
	params->minbits=MIN_ACC_BITS_OTHER;
#endif
	
	return params;
}


int bmark_clean_nnet(void *in_params) {
	nnet_params *params=(nnet_params *)in_params;
	if (in_params==NULL)
		th_exit( THE_BAD_PTR, "Invalid pointer %s:%d", __FILE__,__LINE__ );
	if (params->random_values)
		th_free(params->random_values);
	th_aligned_free(params);
	return 1;
}

void *bmark_init_nnet(void *in_params) {
	nnet_params *params=(nnet_params *)in_params;
    nnet_params *myparams;
	if (in_params==NULL)
		th_exit( THE_BAD_PTR, "Invalid pointer %s:%d", __FILE__,__LINE__ );
	myparams=(nnet_params *)th_aligned_malloc(sizeof(nnet_params),ALIGN_BOUNDARY);
	if ( myparams == NULL )
          th_exit( THE_OUT_OF_MEMORY, "Cannot Allocate Memory %s:%d", __FILE__,__LINE__ );
	th_memcpy(myparams,params,sizeof(nnet_params));

	return myparams;
}
void *bmark_fini_nnet(void *in_params) {
    nnet_params *params;
	if (in_params==NULL)
		th_exit( THE_BAD_PTR, "Invalid pointer %s:%d", __FILE__,__LINE__ );
	params=(nnet_params *)in_params;

	th_aligned_free(params);

	return NULL;
}

void *t_run_test_nnet(struct TCDef *tcdef,void *in_params) {
	nnet_params *params=(nnet_params *)in_params;
	e_u32 test;
	
	tcdef->CRC=0;
	tcdef->expected_CRC=0;
	params->iterations=DoNNetIteration(params);
	
	if (!params->gen_ref && (params->iterations != params->ref_iterations)) 
		tcdef->CRC|=1;

	test=fp_iaccurate_bits(params->average_error,&params->ref_average_error);
	if (test>=params->minbits)
		tcdef->CRC=0;
	else 
		tcdef->CRC=1;

	tcdef->v1=test;
	tcdef->v2=params->iterations;
	tcdef->dbl_data[0]=params->average_error;
	tcdef->dbl_data[1]=params->worst_error;
	
	return tcdef;
}

int bmark_verify_nnet(void *in_params) {
	int i,m,val;
	nnet_params *params=(nnet_params *)in_params;
	int	inpat_size=params->d_y * params->d_x;

	if (params->gen_ref) {
		char sbuf[256];
		th_printf("/**** START DATASET ****/\n#include \"th_lib.h\"\n#include \"../nnet.h\"\n");
		th_printf("static e_u8 in_data_index[]={\n"); 
		for (i=0; i<params->n_in; i++) {
			for (m=0; m<inpat_size; m++) {
				val=params->in_pats[i][m]>FPCONST(0.5);
				th_printf("%d,",val);
				if (((m+1)%params->d_x)==0) th_printf("\n");
			}
			for (m=0; m<params->d_out; m++) {
				val=params->out_pats[i][m]>FPCONST(0.5);
				th_printf("%d,",val);
			}
			th_printf("\n");
		}
		th_printf("}; //ref input data\n\n"); 
		th_printf("static intparts ref_err_index[]={\n"); 
		for (i=0; i<params->n_in; i++) {
			th_printf("\t%s",th_sprint_fp(params->avg_out_error[i],sbuf));
			if (i<params->n_in-1)
				th_printf(",\n");
		}
		th_printf("}; /* ref err */\n\n"); 
		th_fpprintf("static intparts ref_average_error_index=%s;\n",th_sprint_fp(params->average_error,sbuf));
		th_printf("static int ref_passes[] = {\n");
		for (i=0; i<params->loops; i++) {
			th_printf("\t%d",params->ref_passes[i]);
			if (i<params->loops-1)
				th_printf(",\n");
		}
		th_printf("}; /* ref passes */\n\n"); 
		th_printf("void init_preset_index() {\n");
		th_printf("presets_nnet[index].seed=%d;\n",params->seed);
		th_printf("presets_nnet[index].ref_data=ref_err_index;\n");
		th_printf("presets_nnet[index].d_x=%d;\n",params->d_x);
		th_printf("presets_nnet[index].d_y=%d;\n",params->d_y);
		th_printf("presets_nnet[index].d_out=%d;\n",params->d_out);
		th_printf("presets_nnet[index].n_in=%d;\n",params->n_in);
		th_printf("presets_nnet[index].loops=%d;\n",params->loops);
		th_printf("fill_preset_nnet(in_data_index,&(presets_nnet[index]));\npresets_nnet[index].ref_passes=ref_passes;\npresets_nnet[index].ref_average_error=ref_average_error_index;\n");
		th_printf("presets_nnet[index].ref_iterations=%d;\npresets_nnet[index].next_ridx=0;\n}\n",params->iterations);

		th_printf("/**** END DATASET ****/\n");
	} else {
		snr_result resa;
		if (params->ref_data==NULL) {
			th_printf("Cannot validate this dataset! Please generate reference data on reliable hardware and tools using -g!\n");
			return -1;
		}
		resa.bmin_ok=params->minbits;
		ee_ifpbits_buffer(params->avg_out_error,params->ref_data,params->n_in,&resa);
	
		if (!resa.pass) return 0;
	}
	return 1;
}

/********************
** DoNNetIteration **
*********************
** Do a single iteration of the neural net benchmark.
** By iteration, we mean a "learning" pass.
*/
static e_u32 DoNNetIteration(nnet_params *params)
{
int patt;
int nloops=params->loops;
int numpasses=0;
int curloop=0;

if (params->gen_ref)  
	params->ref_passes = (int *)th_malloc(sizeof(int) * nloops);

/*
** Run nloops learning cycles.  Notice that, counted with
** the learning cycle is the weight randomization and
** zeroing of changes.  
*/
while(nloops--)
{
	int learned = FALSE;
	int req_passes=999999;
	if (!params->gen_ref) req_passes = params->ref_passes[curloop++];
	randomize_wts(params);
	zero_changes(params);
	params->pass_count=1;
	numpasses = 0;
	while (learned == FALSE)
	{
		for (patt=0; patt<params->n_in; patt++)
		{
			params->worst_error = FPCONST(0.0);      /* reset this every pass through data */
			move_wt_changes(params);      /* move last pass's wt changes to momentum array */
			do_forward_pass(patt,params);
			do_back_pass(patt,params);
			params->pass_count++;
		}
		numpasses ++;
		if (params->gen_ref) 
			learned = check_out_error(params);
		else {
			learned = (numpasses==req_passes) ? TRUE : FALSE;
			check_out_error(params);
		}
	}
	if (params->gen_ref)  params->ref_passes[curloop++] = numpasses;
#if BMDEBUG
th_printf("Learned in %d passes\n",numpasses);
#endif
}
return numpasses;
}

/*************************
** do_mid_forward(patt) **
**************************
** Process the middle layer's forward pass
** The activation of middle layer's neurode is the weighted
** sum of the inputs from the input pattern, with sigmoid
** function applied to the inputs.
**/
static void  do_mid_forward(int patt,nnet_params *params)
{
e_fp  sum;
int     neurode, i;

for (neurode=0;neurode<MID_SIZE; neurode++)
{
	sum = FPCONST(0.0);
	for (i=0; i<IN_SIZE; i++)
	{       /* compute weighted sum of input signals */
		sum += params->mid_wts[neurode][i]*params->in_pats[patt][i];
	}
	/*
	** apply sigmoid function f(x) = 1/(1+exp(-x)) to weighted sum
	*/
	sum = FPCONST(1.0)/(FPCONST(1.0)+th_exp(-sum));
	params->mid_out[neurode] = sum;
}
return;
}

/*********************
** do_out_forward() **
**********************
** process the forward pass through the output layer
** The activation of the output layer is the weighted sum of
** the inputs (outputs from middle layer), modified by the
** sigmoid function.
**/
static void  do_out_forward(nnet_params *params)
{
e_fp sum;
int neurode, i;

for (neurode=0; neurode<OUT_SIZE; neurode++)
{
	sum = FPCONST(0.0);
	for (i=0; i<MID_SIZE; i++)
	{       /*
		** compute weighted sum of input signals
		** from middle layer
		*/
		sum += params->out_wts[neurode][i]*params->mid_out[i];
	}
	/*
	** Apply f(x) = 1/(1+exp(-x)) to weighted input
	*/
	sum = FPCONST(1.0)/(FPCONST(1.0)+th_exp(-sum));
	params->out_out[neurode] = sum;
}
return;
}

/*************************
** display_output(patt) **
**************************
** Display the actual output vs. the desired output of the
** network.
** Once the training is complete, and the "learned" flag set
** to TRUE, then display_output sends its output to both
** the screen and to a text output file.
**
** NOTE: This routine has been disabled in the benchmark
** version. -- RG
**/
/*
void  display_output(int patt)
{
int             i;

	fprintf(outfile,"\n Iteration # %d",pass_count);
	fprintf(outfile,"\n Desired Output:  ");

	for (i=0; i<OUT_SIZE; i++)
	{
		fprintf(outfile,"%6.3f  ",params->out_pats[patt][i]);
	}
	fprintf(outfile,"\n Actual Output:   ");

	for (i=0; i<OUT_SIZE; i++)
	{
		fprintf(outfile,"%6.3f  ",params->out_out[i]);
	}
	fprintf(outfile,"\n");
	return;
}
*/

/**********************
** do_forward_pass() **
***********************
** control function for the forward pass through the network
** NOTE: I have disabled the call to display_output() in
**  the benchmark version -- RG.
**/
static void  do_forward_pass(int patt,nnet_params *params)
{
do_mid_forward(patt,params);   /* process forward pass, middle layer */
do_out_forward(params);       /* process forward pass, output layer */
/* display_output(patt);        ** display results of forward pass */
return;
}

/***********************
** do_out_error(patt) **
************************
** Compute the error for the output layer neurodes.
** This is simply Desired - Actual.
**/
static void do_out_error(int patt,nnet_params *params)
{
int neurode;
e_fp error,tot_error, sum;

tot_error = FPCONST(0.0);
sum = FPCONST(0.0);
for (neurode=0; neurode<OUT_SIZE; neurode++)
{
	params->out_error[neurode] = params->out_pats[patt][neurode] - params->out_out[neurode];
	/*
	** while we're here, also compute magnitude
	** of total error and worst error in this pass.
	** We use these to decide if we are done yet.
	*/
	error = params->out_error[neurode];
	if (error <FPCONST(0.0))
	{
		sum += -error;
		if (-error > tot_error)
			tot_error = -error; /* worst error this pattern */
	}
	else
	{
		sum += error;
		if (error > tot_error)
			tot_error = error; /* worst error this pattern */
	}
}
params->avg_out_error[patt] = sum/OUT_SIZE;
params->tot_out_error[patt] = tot_error;
return;
}

/***********************
** worst_pass_error() **
************************
** Find the worst and average error in the pass and save it
**/
static void  worst_pass_error(nnet_params *params)
{
e_fp error,sum;

int i;

error = FPCONST(0.0);
sum = FPCONST(0.0);
for (i=0; i<params->n_in; i++)
{
	if (params->tot_out_error[i] > error) error = params->tot_out_error[i];
	sum += params->avg_out_error[i];
}
params->worst_error = error;
params->average_error = sum/params->n_in;
return;
}

/*******************
** do_mid_error() **
********************
** Compute the error for the middle layer neurodes
** This is based on the output errors computed above.
** Note that the derivative of the sigmoid f(x) is
**        f'(x) = f(x)(1 - f(x))
** Recall that f(x) is merely the output of the middle
** layer neurode on the forward pass.
**/
static void do_mid_error(nnet_params *params)
{
e_fp sum;
int neurode, i;

for (neurode=0; neurode<MID_SIZE; neurode++)
{
	sum = FPCONST(0.0);
	for (i=0; i<OUT_SIZE; i++)
		sum += params->out_wts[i][neurode]*params->out_error[i];

	/*
	** apply the derivative of the sigmoid here
	** Because of the choice of sigmoid f(I), the derivative
	** of the sigmoid is f'(I) = f(I)(1 - f(I))
	*/
	params->mid_error[neurode] = params->mid_out[neurode]*(1-params->mid_out[neurode])*sum;
}
return;
}

/*********************
** adjust_out_wts() **
**********************
** Adjust the weights of the output layer.  The error for
** the output layer has been previously propagated back to
** the middle layer.
** Use the Delta Rule with momentum term to adjust the weights.
**/
static void adjust_out_wts(nnet_params *params)
{
int weight, neurode;
e_fp learn,delta,alph;

learn = BETA;
alph  = ALPHA;
for (neurode=0; neurode<OUT_SIZE; neurode++)
{
	for (weight=0; weight<MID_SIZE; weight++)
	{
		/* standard delta rule */
		delta = learn * params->out_error[neurode] * params->mid_out[weight];

		/* now the momentum term */
		delta += alph * params->out_wt_change[neurode][weight];
		params->out_wts[neurode][weight] += delta;

		/* keep track of this pass's cum wt changes for next pass's momentum */
		params->out_wt_cum_change[neurode][weight] += delta;
	}
}
return;
}

/*************************
** adjust_mid_wts(patt) **
**************************
** Adjust the middle layer weights using the previously computed
** errors.
** We use the Generalized Delta Rule with momentum term
**/
static void adjust_mid_wts(int patt,nnet_params *params)
{
int weight, neurode;
e_fp learn,alph,delta;

learn = BETA;
alph  = ALPHA;
for (neurode=0; neurode<MID_SIZE; neurode++)
{
	for (weight=0; weight<IN_SIZE; weight++)
	{
		/* first the basic delta rule */
		delta = learn * params->mid_error[neurode] * params->in_pats[patt][weight];

		/* with the momentum term */
		delta += alph * params->mid_wt_change[neurode][weight];
		params->mid_wts[neurode][weight] += delta;

		/* keep track of this pass's cum wt changes for next pass's momentum */
		params->mid_wt_cum_change[neurode][weight] += delta;
	}
}
return;
}

/*******************
** do_back_pass() **
********************
** Process the backward propagation of error through network.
**/
static void  do_back_pass(int patt,nnet_params *params)
{

do_out_error(patt,params);
do_mid_error(params);
adjust_out_wts(params);
adjust_mid_wts(patt,params);

return;
}


/**********************
** move_wt_changes() **
***********************
** Move the weight changes accumulated last pass into the wt-change
** array for use by the momentum term in this pass. Also zero out
** the accumulating arrays after the move.
**/
static void move_wt_changes(nnet_params *params)
{
int i,j;

for (i = 0; i<MID_SIZE; i++)
	for (j = 0; j<IN_SIZE; j++)
	{
		params->mid_wt_change[i][j] = params->mid_wt_cum_change[i][j];
		/*
		** Zero it out for next pass accumulation.
		*/
		params->mid_wt_cum_change[i][j] = FPCONST(0.0);
	}

for (i = 0; i<OUT_SIZE; i++)
	for (j=0; j<MID_SIZE; j++)
	{
		params->out_wt_change[i][j] = params->out_wt_cum_change[i][j];
		params->out_wt_cum_change[i][j] = FPCONST(0.0);
	}

return;
}

/**********************
** check_out_error() **
***********************
** Check to see if the error in the output layer is below
** MARGIN*OUT_SIZE for all output patterns.  If so, then
** assume the network has learned acceptably well.  This
** is simply an arbitrary measure of how well the network
** has learned -- many other standards are possible.
**/
static int check_out_error(nnet_params *params)
{
int result,i,error;

result  = TRUE;
error   = FALSE;
worst_pass_error(params);     /* identify the worst error in this pass */

/*
#ifdef DEBUG
printf("\n Iteration # %d",pass_count);
#endif
*/
for (i=0; i<params->n_in; i++)
{
/*      printf("\n Error pattern %d:   Worst: %8.3f; Average: %8.3f",
	  i+1,params->tot_out_error[i], params->avg_out_error[i]);
	fprintf(outfile,
	 "\n Error pattern %d:   Worst: %8.3f; Average: %8.3f",
	 i+1,params->tot_out_error[i]);
*/

	if (params->worst_error >= NSTOP) result = FALSE;
	if (params->tot_out_error[i] >= FPCONST(16.0)) error = TRUE;
}

if (error == TRUE) result = ERR;


#ifdef DEBUG
/* printf("\n Error this pass thru data:   Worst: %8.3f; Average: %8.3f",
 params->worst_error,params->average_error);
*/
/* fprintf(outfile,
 "\n Error this pass thru data:   Worst: %8.3f; Average: %8.3f",
  params->worst_error, params->average_error); */
#endif

return(result);
}


/*******************
** zero_changes() **
********************
** Zero out all the wt change arrays
**/
static void zero_changes(nnet_params *params)
{
int i,j;

for (i = 0; i<MID_SIZE; i++)
{
	for (j=0; j<IN_SIZE; j++)
	{
		params->mid_wt_change[i][j] = FPCONST(0.0);
		params->mid_wt_cum_change[i][j] = FPCONST(0.0);
	}
}

for (i = 0; i< OUT_SIZE; i++)
{
	for (j=0; j<MID_SIZE; j++)
	{
		params->out_wt_change[i][j] = FPCONST(0.0);
		params->out_wt_cum_change[i][j] = FPCONST(0.0);
	}
}
return;
}


/********************
** randomize_wts() **
*********************
** Intialize the weights in the middle and output layers to
** random values between -0.25..+0.25
**
** NOTE: Had to make alterations to how the random numbers were
** created.  -- SG.
**/
static void randomize_wts(nnet_params *params)
{
int neurode,i;
e_fp value;

for (neurode = 0; neurode<MID_SIZE; neurode++)
{
	for(i=0; i<IN_SIZE; i++)
	{
	        /* value=(e_fp)abs_randwc(100000L); */
		value=params->random_values[params->next_ridx++ & 0xff];
		params->mid_wts[neurode][i] = value;
	}
}
for (neurode=0; neurode<OUT_SIZE; neurode++)
{
	for(i=0; i<MID_SIZE; i++)
	{
		value=params->random_values[params->next_ridx++ & 0xff];
			/* original code had a bug in nnet init:	value=value/(e_fp)10000.0 - (e_fp) 0.5; */
		params->out_wts[neurode][i] = value;
	}
}

return;
}

/*********************
** initialize_net() **
**********************
** Do all the initialization stuff before beginning
*/
/*
static int initialize_net()
{
int err_code;

randomize_wts();
zero_changes();
err_code = read_data_file();
pass_count = 1;
return(err_code);
}
*/

/**********************
** display_mid_wts() **
***********************
** Display the weights on the middle layer neurodes
** NOTE: This routine is not used in the benchmark
**  test -- RG
**/
/* static void display_mid_wts()
{
int             neurode, weight, row, col;

fprintf(outfile,"\n Weights of Middle Layer neurodes:");

for (neurode=0; neurode<MID_SIZE; neurode++)
{
	fprintf(outfile,"\n  Mid Neurode # %d",neurode);
	for (row=0; row<IN_Y_SIZE; row++)
	{
		fprintf(outfile,"\n ");
		for (col=0; col<IN_X_SIZE; col++)
		{
			weight = IN_X_SIZE * row + col;
			fprintf(outfile," %8.3f ", params->mid_wts[neurode][weight]);
		}
	}
}
return;
}
*/
/**********************
** display_out_wts() **
***********************
** Display the weights on the output layer neurodes
** NOTE: This code is not used in the benchmark
**  test -- RG
*/
/* void  display_out_wts()
{
int             neurode, weight;

	fprintf(outfile,"\n Weights of Output Layer neurodes:");

	for (neurode=0; neurode<OUT_SIZE; neurode++)
	{
		fprintf(outfile,"\n  Out Neurode # %d \n",neurode);
		for (weight=0; weight<MID_SIZE; weight++)
		{
			fprintf(outfile," %8.3f ", params->out_wts[neurode][weight]);
		}
	}
	return;
}
*/

void fill_preset_nnet(e_u8 *in_data, nnet_params *params) {
	int in_idx=0,i,m;
	int inpat_size=params->d_y * params->d_x;
	for (i=0; i<params->n_in; i++) {
		for (m=0; m<inpat_size; m++) {
			params->in_pats[i][m] = in_data[in_idx++] ? FPCONST(0.9):FPCONST(0.1);
		}
		for (m=0; m<params->d_out; m++) {
			params->out_pats[i][m] = in_data[in_idx++] ? FPCONST(1.0):FPCONST(0.0);
		}
	}
}
