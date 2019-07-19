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

/* File: mith/src/mith_lib.c 
	Functions for processing MITH workloads
	This file provides the control for working with mith workloads.
 */

#include "th_lib.h"
#include "th_al.h"
#include "al_smp.h"
#include "mith_sync.h"
#include "mith_workload.h"

e_u32 verify_output=1;
e_u32 reporting_threshold=TH_INFO;

int mith_report_results(ee_workload *workload, size_t total_time, e_u32 num_contexts, unsigned int num_workers);
/* Struct : mith_context_t
	This structure contains data pertaining to an 
	execution context that can run benchmarks.
*/
typedef struct mith_context_s {
	al_thread_t thread;
	al_mutex_t mutex;
	ee_work_item_t *item;
	size_t time;
	Bool busy;
	Bool locked;
	e_u32 id;
	int inv;
	void *params;
	ee_workload *workload;
	/* future extensions: affinity? */
} mith_context_t;
/* DEBUG : This global is for debugging purposes only. */
mith_context_t *gcontext;

/* Variable: mith_mutex 
	global for sync on the next item to execute */
al_mutex_t mith_mutex;

/* Function: mith_wl_get_chain_mutex 
	Return a pointer to the start of the mutexes associated with this chain.

	Since each chain is going to have mcl items, we simply associate each chain with mcl cells
	in the array allocated for the specific iteration.
*/
al_mutex_t *mith_wl_get_chain_mutex(ee_workload *workload,ee_work_item_t *item) {
	al_mutex_t *mutex=&(workload->chain_mutexes[item->assigned][item->chain_uid*(workload->mcl+1)]);
	return mutex;
}
/* Function: mith_wl_get_connections 
	Fill up the connections related to a specific instantiation of a specific work item

	find the connections for this workload iteration (item->assigned).
	find the connections of this item using the connect_idx mapping.
*/
ee_connection **mith_wl_get_connections(ee_workload *workload,ee_work_item_t *item) {
	ee_connection **connect=(ee_connection **)th_malloc(sizeof(ee_connection *)*item->next_idx);
	int i;
	for (i=0; i<item->next_idx; i++) {
		connect[i]=(ee_connection *)&(workload->connections[item->assigned][item->connect_idx[i]]);
	}
	return connect;
}

#if REPORT_THMALLOC_STATS 
void print_memstat(char *s, char *m,float val) {
	th_printf(s);
	if (m) th_printf(" for %s",m); 
	if (val < 1024.0*1024.0) {
		th_printf(": %.02fKB\n",(float)val/(1024.0));
	} else {
		th_printf(":%.02fMB\n",(float)val/(1024.0*1024.0));
	}
	
}
#endif

/* Function: bench_thread 
	This is intended to be called as a thread.
	It will continously extract items to process from the workload,
	process and time them, and verify if needed.
	This thread will only exit when no more items remain for processing and
	last item aquired by this thread has finished.
*/
void *bench_thread(void *inarg) {
	mith_context_t *mycontext=(mith_context_t *)inarg;
	TCDef benchin;
	ee_work_item_t *item=NULL;
	ee_connection **connection=NULL;	/* point to connections for currrent item */
	al_mutex_t *chain_mutex=NULL;		/* point to chain mutexes for current item */
	void *retval=NULL; /* potentially return some info with this */
	Bool failed=0;
	int i;
#if THDEBUG
	char logbuf[256];
#endif
	while (1) {
		/* lock the workload and extract next item */
		al_mutex_lock(&mith_mutex); /* lock the workload and extract next item */
		/*P*/item=mith_wl_get_next(mycontext->workload);
		/*P*/if (item && item->chain_id>=0) {
		/*P*/	chain_mutex=mith_wl_get_chain_mutex(mycontext->workload,item); /* get address to start of chain mutex array (by item->assigned get specific chain instance) */
		/*P*/	if (item->chain_id==1) /* A special mutex to make sure all elements of a chain start together */
		/*P*/		al_mutex_lock(&chain_mutex[0]);
		/*P*/	al_mutex_lock(&(chain_mutex[item->chain_id])); /* lock the item specific mutex for this instance of this chain */
		/*P*/	connection=mith_wl_get_connections(mycontext->workload,item);
		/*P*/}
		/*P*/if (item!=NULL)
		/*P*/	item->assigned++;
		al_mutex_unlock(&mith_mutex);
		if (item==NULL)
			break; /* stop only when no more items to process */
		mycontext->inv=item->assigned;
		/* if this is part of a chain, make sure all chain items are active before continue */
		if (item->chain_id>=0) {
			/* release the special mutex if last chain item is assigned */
			if (item->chain_id == (mycontext->workload->chain_info[item->chain_uid].num_items)) {
				al_mutex_unlock(&chain_mutex[0]);
#if THDEBUG
				th_sprintf(logbuf," * last of of chain:%d [%d]%s[%d] in %d",item->chain_uid,item->uid,item->shortname,mycontext->inv,mycontext->id);
				th_log(TH_INFO,logbuf);
			}
			else {
				th_sprintf(logbuf," * waiting for rest of chain:%d [%d]%s[%d] in %d",item->chain_uid,item->uid,item->shortname,mycontext->inv,mycontext->id);
				th_log(TH_INFO,logbuf);
#endif
			}
			/* do not start execution until last item is assigned */
			al_mutex_lock(&chain_mutex[0]);
			al_mutex_unlock(&chain_mutex[0]);
		}
		if (item->init_func!=NULL) { /* some items may be fake, just for chains */
			al_item_setaffinity(item->kernel_id,item->instance_id,item->uid,mycontext->id);
#if REPORT_THMALLOC_STATS
			print_memstat("Total allocated memory before init",item->shortname,(float)th_malloc_total);
#endif
			mycontext->params=item->init_func(item->params); /* prepare for run */
#if REPORT_THMALLOC_STATS
			print_memstat("Total allocated memory after init",item->shortname,(float)th_malloc_total);
#endif
			th_memcpy(&benchin,item->tcdef,sizeof(TCDef));
			benchin.connection=connection;
			mycontext->time=al_signal_now(); 
			/* execute the work item - params are saved on a per context basis */
			item->bench_func(&benchin,mycontext->params);
			/* note time it took for item to execute */
			mycontext->time=al_signal_now()-mycontext->time;
#if THDEBUG
	th_sprintf(logbuf," * completed [%d]%s[%d] in %d",item->uid,item->shortname,mycontext->inv,mycontext->id);
	th_log(TH_INFO,logbuf);
#endif
			/* verify output */
			if (verify_output) {
				failed = (Bool)(item->veri_func(mycontext->params)<=0);
			} else {
				failed = (benchin.CRC==benchin.expected_CRC) ? 0 : 1;
			}
			/* cleanup and prepare for next run */
#if REPORT_THMALLOC_STATS
			print_memstat("Total allocated memory before fini",item->shortname,(float)th_malloc_total);
#endif
			item->fini_func(mycontext->params);
#if REPORT_THMALLOC_STATS
			print_memstat("Total allocated memory after fini",item->shortname,(float)th_malloc_total);
#endif
		}
		/* cleanup and wait for chain to end */
		if (item->chain_id>=0) { 
			al_mutex_unlock(&chain_mutex[item->chain_id]);
			/* to make sure all items in chain are done, lock the chain mutex for each */
			for (i=0; i<mycontext->workload->mcl; i++) {
				al_mutex_lock(&chain_mutex[i]);
				al_mutex_unlock(&chain_mutex[i]);
			}
			/* and clean up the connection block */
			th_free(connection);
		}
		mycontext->params = NULL;
		/* Record item time.
		if items end up sharing hardware contexts, total item time is irrelevant */
		al_mutex_lock(&mith_mutex); /* lock the workload to update item info */
		/*P*/item->finished++;
		/*P*/item->failed+=failed;
		/*P*/item->time+=mycontext->time;
		/*P*/item->tcdef->actual_iterations=benchin.actual_iterations;
		/*P*/item->tcdef->v1=benchin.v1;
		/*P*/item->tcdef->v2=benchin.v2;
		/*P*/item->tcdef->v3=benchin.v3;
		/*P*/item->tcdef->v4=benchin.v4;
#if FP_KERNELS_SUPPORT
		for (i=0; i<4; i++) 
			item->tcdef->dbl_data[i]=benchin.dbl_data[i];
#endif
		al_mutex_unlock(&mith_mutex);
		item = NULL;
	}
#if THDEBUG
	th_sprintf(logbuf," * all done in %d",mycontext->id);
	th_log(TH_INFO,logbuf);
#endif

	/* TODO: Is there anything that needs to be done with the bench thread output? */
	return retval;
}

static int RunBigEndian()
{
   short word = 0x4321;
   if(((*(char *)& word)&0xff) != 0x21 )
     return 1;
   else 
     return 0;
}

/* Function: mith_main
	Description: 
	main function that fires off work items. 

	Accepts a workload as a parameter, and number of iterations for the workload.

	Will then fire off items from the workload to available contexts as soon as they become available.
	Will stop once each item in the workload has iterated num_iterations, and report results.
	
	Parameters:
	workload - Actual workload to execute
	num_iterations - Number of times to execute each item of the workload
	num_contexts - number of execution contexts to execute on

	Returns:
	Instead of a return value, report results function is called to summarize results.
	*/
size_t mith_main_loop(ee_workload *workload, unsigned int num_iterations, unsigned int num_contexts, Bool oversubscribe_allowed) {
	size_t total_time;
	e_u32 i;
	mith_context_t *context;
	void *context_ret; /* potentially can be used to collect thread pool stats */
#if SINGLE_CONTEXT
	num_contexts=1;
#endif
	if (verify_output) /* SG: for validation run, there is no reason to run more then one iteration. */
		num_iterations=1;
	/* some sanity checks */
	if(sizeof(e_u32) < (32/CHAR_BIT))
		th_exit(TH_FATAL,"e_u32 type defined incorrectly. Please define e_u32 such that it will be a 32b type in th_types.h");
	
	if(sizeof(e_u32) != (32/CHAR_BIT))
		th_printf("WARNING: e_u32 type defined with more then 32 bits. Please define e_u32 such that it will be a 32b type in th_types.h to make sure all kernels execute correctly.");

	if ((EE_LITTLE_ENDIAN) && RunBigEndian()) 
		th_exit(TH_FATAL,"Big endian machine detected but little endian defined. Please define EE_LITTLE_ENDIAN in th_cfg.h");

	if ((EE_BIG_ENDIAN) && !RunBigEndian()) 
		th_exit(TH_FATAL,"Little endian machine detected but big endian defined. Please define EE_BIG_ENDIAN in th_cfg.h");

#if USE_FP32
	if(sizeof(e_f32) != (32/CHAR_BIT))
		th_exit(TH_FATAL,"e_f32 type defined incorrectly. Please define e_f32 such that it will be a 32b type in th_types.h");
#endif		
#if USE_FP64
	if(sizeof(e_f64) != (64/CHAR_BIT))
		th_exit(TH_FATAL,"e_f64 type defined incorrectly. Please define e_f64 such that it will be a 64b type in th_types.h");
#endif		
	/* First, do any initializations necessary */
	if (num_contexts<1) {
		th_log(TH_INFO,"No contexts to execute on!");
		return 0;
	}
	if (!mith_wl_reset(workload,num_iterations,num_contexts,oversubscribe_allowed)) {
		th_log(TH_INFO,"Initialization Failed!");
		return 0;
	}
	context=th_calloc(num_contexts,sizeof(mith_context_t));
	gcontext=context;
	al_mutex_init(&mith_mutex);
	for (i=0; (i<num_contexts); i++) {
		al_mutex_init(&context[i].mutex);
		context[i].id=i;
		context[i].workload=workload;
	}
	th_log(TH_INFO,"Starting Run...");

	/* now to start the timer, and get the first item */
	/* from here on we need to worry about thread safety */
	al_signal_start();
	/* create the thread pool */
	for (i=0; i<num_contexts; i++)
		al_thread_create(&(context[i].thread),bench_thread,(void *)(&context[i]));
	/* and wait until threads are all done executing */
	for (i=0; i<num_contexts; i++)
		al_thread_join((context[i].thread),&context_ret);
	total_time=al_signal_finished();
	/* workload is now done, report results */
	/* from here on, only one thread operates */

	al_mutex_destroy(&mith_mutex);
	for (i=0; (i<num_contexts); i++) {
		al_mutex_destroy(&context[i].mutex);
	}
	th_free(context);

	return total_time;
}


int mith_main(ee_workload *workload, unsigned int num_iterations, unsigned int num_contexts, Bool oversubscribe_allowed, unsigned int num_workers) {
	size_t total_time=mith_main_loop(workload, num_iterations, num_contexts, oversubscribe_allowed);
#if REPORT_THMALLOC_STATS 
	print_memstat("Max allocated memory",NULL,(float)th_malloc_max);
#endif
	return mith_report_results(workload, total_time,num_contexts, num_workers);
}

#if (FLOAT_SUPPORT && MITH_DEMO)
double mith_main_demorun(ee_workload *workload, unsigned int num_iterations, unsigned int num_contexts, Bool oversubscribe_allowed) {
	size_t total_time;
	double secs;
	double itps;
	unsigned int i;

	total_time=mith_main_loop(workload, num_iterations, num_contexts, oversubscribe_allowed);
	for (i=0; i<workload->max_idx ; i++)
	{
		ee_work_item_t *item=workload->load[i];
		if (item->failed>0)
			return 0.0;
	}
	secs=(double)total_time/(double)al_ticks_per_sec();
	itps= (double)workload->iterations/secs;
	return itps;
}
double mith_main_demorun2(ee_workload *workload, unsigned int num_iterations, unsigned int num_contexts, Bool oversubscribe_allowed, double *itpsret) {
	size_t total_time;
	double secs;
	double itps;
	unsigned int i;
	total_time=mith_main_loop(workload, num_iterations, num_contexts, oversubscribe_allowed);
	for (i=0; i<workload->max_idx ; i++)
	{
		ee_work_item_t *item=workload->load[i];
		if (item->failed>0)
			return 0.0;
	}
	secs=(double)total_time/(double)al_ticks_per_sec();
	itps= (double)workload->iterations/secs;
	*itpsret=itps;
	return itps;
}
#endif

/************************
 functions for logging results 
 */
/* log a double value */
#if FLOAT_SUPPORT
void mith_log_dbl(const char * component, const char *type, double val) {
	th_printf("-- %s:%s=%8g\n",component,type,val);
}
void mith_log_sci(const char * component, const char *type, double val) {
	th_printf("-- %s:%s=%le\n",component,type,val);
}
#endif
/* log an int value */
void mith_log(const char * component, const char *type, size_t val) {
	th_printf("-- %s:%s=%d\n",component,type,val);
}
/* log an unsigned int value  */
void mith_log_u(const char * component, const char *type, size_t val) {
	th_printf("-- %s:%s=%u\n",component,type,val);
}
void mith_log_iu(const char * component, const char *type, int index, size_t val) {
	th_printf("-- %s:%s[%d]=%u\n",component,type,index,val);
}
/* log a tcdef structure */
void mith_log_tcdef(const char * component, TCDef *tcdef, size_t total_item_time, e_u32 item_execs) {
#if FLOAT_SUPPORT
	double secs;
#endif
	mith_log(component,"repeats",tcdef->actual_iterations);
	mith_log(component,"v1",tcdef->v1);
	mith_log(component,"v2",tcdef->v2);
	mith_log(component,"v3",tcdef->v3);
	mith_log(component,"v4",tcdef->v4);
#if FP_KERNELS_SUPPORT
	mith_log_sci(component,"f1",tcdef->dbl_data[0]);
	mith_log_sci(component,"f2",tcdef->dbl_data[1]);
	mith_log_sci(component,"f3",tcdef->dbl_data[2]);
	mith_log_sci(component,"f4",tcdef->dbl_data[3]);
#endif
#if FLOAT_SUPPORT
	/* calculate based off average secs per item execution */
	secs=((double)total_item_time/(double)al_ticks_per_sec())/(double)item_execs; 
	mith_log_dbl(component,"secs/repeat",secs/(double)tcdef->actual_iterations);
	mith_log_dbl(component,"repeats/sec",(double)tcdef->actual_iterations/secs);
#endif
}
/* Function: mith_report_results
	Description:
	Log a full workload 

	Parameters:
	workload - workload that was executed
	total_time - total time taken for workload execution, 
	including overhead for verification and scheduling.
*/
int mith_report_results(ee_workload *workload, size_t total_time, e_u32 num_contexts, unsigned int num_workers) {
	e_u32 i;
	size_t total_item_time=0;
	int total_fails=0;
	char *wname=workload->shortname;
#if FLOAT_SUPPORT
	double secs,total_item_secs=0.0;
	#if OVERHEAD_REPORT	
	double total_sync=0.0,total_all;
	#endif
	if (reporting_threshold>TH_FATAL) {
		secs=(double)total_time/(double)al_ticks_per_sec();
		th_printf("%s,%lf\n",wname,(double)workload->iterations/secs);
		return 1;
	}
#endif
	mith_log_u("Workload",wname,workload->uid);
	mith_log_u(wname,"time(ns)",total_time);
	for (i=0; i<workload->max_idx ; i++)
	{
		ee_work_item_t *item=workload->load[i];
		total_fails+=item->failed;
	}
	if (total_fails>0)
		mith_log(wname,"ERRORS",total_fails);
	mith_log(wname,"contexts",num_contexts);
	if (num_workers>0)
		mith_log_u(wname,"workers",num_workers);
	mith_log(wname,"iterations",workload->iterations);
#if FLOAT_SUPPORT
	secs=(double)total_time/(double)al_ticks_per_sec();
	mith_log_dbl(wname,"time(secs)",secs);
	mith_log_dbl(wname,"secs/workload",secs/(double)workload->iterations);
	mith_log_dbl(wname,"workloads/sec",(double)workload->iterations/secs);
#endif
#if BMDEBUG
	th_printf("Info: This run was executed with kernel debug turned on! For performance results, define BMDEBUG to 0.\n");
#endif
#if THDEBUG
	th_printf("Info: This run was executed with harness debug turned on! For performance results, define THDEBUG to 0.\n");
#endif

	/* only output per item information if verification is turned on */
	if (verify_output) {
		th_printf("Info: This run was executed with verification turned on! For performance results, use -v0.\n");
		for (i=0; i<workload->max_idx ; i++)
		{
			ee_work_item_t *item=workload->load[i];
			char *name=item->shortname;
			mith_log(name,"UID",item->uid);
			mith_log(name,"fails",item->failed);
			mith_log(name,"time(ticks)",item->time);
			total_item_time+=item->time;
			mith_log(name,"count",item->finished);
			mith_log_tcdef(name,item->tcdef,item->time,item->finished);
	#if FLOAT_SUPPORT
			secs=(double)item->time/(double)al_ticks_per_sec();
			total_item_secs+=secs;
			mith_log_dbl(name,"time(secs)",secs);
			mith_log_dbl(name,"secs/item",secs/(double)item->finished);
			mith_log_dbl(name,"items/sec",(double)item->finished/secs);
	#endif
		}
		mith_log("Items","total(ticks)",total_item_time);
	#if FLOAT_SUPPORT
		/* To compute sync overhead, sum up item times from individual threads,
			subtract that from total time available to all contexts */
		mith_log_dbl("Items","total(secs)",total_item_secs);
		#if OVERHEAD_REPORT
		total_all=total_time*(double)num_contexts;
		total_sync=(double)(total_all-total_item_time)/(double)num_contexts;
		mith_log_dbl("Sync/Init/Verify Overhead","total(ticks)",total_sync);
		mith_log_dbl("Sync/Init/Verify Overhead","total(secs)",total_sync/(double)al_ticks_per_sec());
		mith_log_dbl("Sync/Init/Verify Overhead","(%%)",100.0*total_sync/(double)total_time);
		#endif
	#endif
	}
	#if DEBUG_ACCURATE_BITS
		mith_log("sig_exp","min",acc_summary.sig_exp.min);
		mith_log("sig_exp","max",acc_summary.sig_exp.max);
		mith_log("sig_exp","avg",acc_summary.sig_exp.avg);
		mith_log("ref_exp","min",acc_summary.ref_exp.min);
		mith_log("ref_exp","max",acc_summary.ref_exp.max);
		mith_log("ref_exp","avg",acc_summary.ref_exp.avg);
		{ 	e_u32 j=0;
			for (i=0; i<MAX_ACC_COUNTS; i++) {
				j+=acc_summary.counts[i];
				if (acc_summary.counts[i]>0)
					mith_log_iu("accbits","counts",i,acc_summary.counts[i]);
			}
			mith_log_u("accbits","counts[max]",acc_summary.counts[i]);
			mith_log_u("accbits","tracked",j);
		}
	#endif
	#if FP_KERNELS_SUPPORT
	if (((total_fails>0) && (acc_summary.bits.n>0)) || DEBUG_ACCURATE_BITS) {
		mith_log("accbits","min",acc_summary.bits.min);
		mith_log("accbits","max",acc_summary.bits.max);
		mith_log("accbits","avg",acc_summary.bits.avg);
	} /* Extra debug info for accurate bits */
	#endif
	mith_log_u("Done",wname,workload->uid);

	return 1;
}



int chain_init_ctrl(map_ent *map_out, ee_connection **connections, int channel) {
	int retval=1;
	int connection_index=map_out[channel].ctrl;
	ee_connection *connection=connections[connection_index];
	al_mutex_lock(&connection->mutex);
	if (connection->data==NULL) 
		connection->data=th_malloc(sizeof(signal_num));
	if (connection->data==NULL)
		retval=0;
	else
		*((signal_num *)(connection->data))=START;
	/* unlocking the mutex for this channel only happens on a wait! */
	return retval;
}

int chain_signal_in_ready(map_ent *map_in, ee_connection **connections, int channel) {
	int retval=0;
	int connection_index=map_in[channel].ctrl;
	ee_connection *connection=connections[connection_index];
	retval+=al_mutex_lock(&connection->mutex);
	*((signal_num *)(connection->data))=CONTINUE;
	retval+=al_cond_signal(&connection->signal);
	retval+=al_mutex_unlock(&connection->mutex);
	return retval;
}
/* This really serves 2 purposes:
	1. Ctrl channel message passing.
	2. Sender waits until message ACK, and only then continues.
*/
signal_num chain_signal_in(map_ent *map_in, ee_connection **connections, int channel) {
	signal_num retval=INVALID_SIG;
	int connection_index=map_in[channel].ctrl;
	ee_connection *connection=connections[connection_index];
	al_mutex_lock(&connection->mutex);
	retval=*((signal_num *)(connection->data));
	*((signal_num *)(connection->data))=ACK;
	al_cond_signal(&connection->signal);
	al_mutex_unlock(&connection->mutex);
	return retval;
}

int chain_ready_in(map_ent *map_in, ee_connection **connections, int channel) {
	int retval=0;
	int connection_index=map_in[channel].ctrl;
	ee_connection *connection=connections[connection_index];
	if (al_mutex_trylock(&connection->mutex)==0) {
		retval=1;
		al_mutex_unlock(&connection->mutex);
	}
	return retval;
}

int chain_signal_out(map_ent *map_out, ee_connection **connections, signal_num sendme, int channel) {
	int retval=0;
	int connection_index=map_out[channel].ctrl;
	ee_connection *connection=connections[connection_index];
	*((signal_num *)(connection->data))=sendme ;
	return retval;
}

int chain_wait_prev_ready(map_ent *map_in, ee_connection **connections, int channel) {
	int retval=0;
	int connection_index=map_in[channel].data;
	ee_connection *connection=connections[connection_index];
	retval+=al_cond_wait(&connection->signal,&connection->mutex);
	/* now the prev owner signals that the data is ready */
	return retval;
}
int chain_wait_prev_start(map_ent *map_in, ee_connection **connections, int channel) {
	int retval=0;
	int connection_index=map_in[channel].ctrl;
	ee_connection *connection=connections[connection_index];
	/* busy wait until ctrl channel of previous item in the chain is initialized 
		hopefully, the system is clever enough to yield when unlocking,
		since we do not have a yield primitive.
	*/
	while (connection->data==0) {
		al_mutex_lock(&connection->mutex);
		al_mutex_unlock(&connection->mutex);
	}
	return retval;
}
e_u8 *chain_get_next(map_ent *map_in, ee_connection **connections, int channel) {
	e_u8 *buf;
	int connection_index=map_in[channel].data;
	ee_connection *connection=connections[connection_index];
	buf=(e_u8 *)connection->data;
	return buf;
}
int chain_set_out(map_ent *map_out, ee_connection **connections, e_u8 *buf, int channel) {
	int retval=0;
	int connection_index=map_out[channel].data;
	ee_connection *connection=connections[connection_index];
	connection->data=(void *)buf;
	return retval;
}
int chain_signal_out_ready(map_ent *map_out, ee_connection **connections, int channel) {
	int retval=0;
	int connection_index=map_out[channel].data;
	ee_connection *connection=connections[connection_index];
	retval+=al_mutex_lock(&connection->mutex);
	retval+=al_cond_signal(&connection->signal);
	retval+=al_mutex_unlock(&connection->mutex);
	return retval;
}
int chain_lock_in(map_ent *map_in, ee_connection **connections, int channel, int lock_state) {
	int retval=0;
	int connection_index=map_in[channel].data;
	ee_connection *connection=connections[connection_index];
	if (lock_state)
		retval+=al_mutex_lock(&connection->mutex);
	else
		retval+=al_mutex_unlock(&connection->mutex);
	return retval;
}
int chain_lock_out(map_ent *map_out, ee_connection **connections, int channel, int lock_state) {
	int retval=0;
	int connection_index=map_out[channel].ctrl;
	ee_connection *connection=connections[connection_index];
	if (lock_state)
		retval+=al_mutex_lock(&connection->mutex);
	else
		retval+=al_mutex_unlock(&connection->mutex);
	return retval;
}

int chain_wait_next_ready(map_ent *map_out, ee_connection **connections, int channel) {
	int retval=0;
	int connection_index=map_out[channel].ctrl;
	ee_connection *connection=connections[connection_index];
	retval+=al_cond_wait(&connection->signal,&connection->mutex);
	/* at this point the data signal should be ACK */
	return retval;
}

