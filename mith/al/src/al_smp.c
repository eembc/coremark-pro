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

/* File: mith/al/src/al_smp.c
	Abstraction layer implementation for thread API

	This file contains implementation prototypes using pthreads for all MITH API.
	Change the implementation of the functions in this file to use primitives applicabale to your target.
	Also see pthread documentation for expected behavior of this API which is a subset of pthreads API.
*/

#if (HAVE_PTHREAD==1)
#include <pthread.h>
#endif
#if HAVE_PTHREAD_SETAFFINITY_NP==1
#include <sched.h>
#endif
#include "th_lib.h"
#include "al_smp.h"

#if (HAVE_PTHREAD==1) && (USE_SINGLE_CONTEXT!=1) && (USE_NATIVE_PTHREAD==0)
/* Function: al_mutex_init
Description: 
Initialize a mutex.
*/
int al_mutex_init(al_mutex_t *mutex) {
int retval=pthread_mutex_init((pthread_mutex_t *)mutex,NULL);
#if THDEBUG>2
	th_printf(" MInit[%d]:%08x,%08x\n",retval,mutex,*mutex);
#endif
	return retval;
}
/* Function: al_mutex_lock
Description: 
Lock a mutex.
*/
int al_mutex_lock(al_mutex_t *mutex) {
#if THDEBUG>2
	th_printf(" lock:%08x,%08x\n",mutex,*mutex);
#endif
	return pthread_mutex_lock((pthread_mutex_t *)mutex);
}
/* Function: al_mutex_trylock
Description:
Lock a mutex (non blocking), and return if mutex is already locked.
*/
int al_mutex_trylock(al_mutex_t *mutex) {
	return pthread_mutex_trylock((pthread_mutex_t *)mutex);
}
/* Function: al_mutex_unlock
Description: 
Unlock a mutex.
*/
int al_mutex_unlock(al_mutex_t *mutex) {
#if THDEBUG>2
	th_printf(" unlock:%08x,%08x\n",mutex,*mutex);
#endif
	return pthread_mutex_unlock((pthread_mutex_t *)mutex);
}
/* Function: al_mutex_destroy
Description:
Destroy a mutex previously initialized with <al_mutex_init>.
*/
int al_mutex_destroy(al_mutex_t *mutex) {
#if THDEBUG>2
	th_printf(" MDestroy:%08x,%08x\n",mutex,*mutex);
#endif
	return pthread_mutex_destroy((pthread_mutex_t *)mutex);
}
/* Function: al_cond_init
Description:
Initialize a condition.
*/
int al_cond_init(al_cond_t *cond) {
	return pthread_cond_init((pthread_cond_t *)cond,NULL);
}
/* Function: al_cond_signal
Description:
Signal a condition.
*/
int al_cond_signal(al_cond_t *cond) {
	return pthread_cond_signal((pthread_cond_t *)cond);
}
/* Function: al_cond_broadcast
Description:
Broadcast a condition.
*/
int al_cond_broadcast(al_cond_t *cond) {
	return pthread_cond_broadcast((pthread_cond_t *)cond);
}
/* Function: al_cond_wait
Description:
Wait for a signal based on condition.
*/
int al_cond_wait(al_cond_t *cond, al_mutex_t *mutex){
#if THDEBUG>2
	th_printf(" wait:%08x,%08x\n",mutex,*mutex);
#endif
	return pthread_cond_wait((pthread_cond_t *)cond,(pthread_mutex_t *)mutex);
}
/* Function: al_cond_destroy
Description:
Destroy a condition previously initialized by <al_cond_init>
*/
int al_cond_destroy(al_cond_t *cond) {
	return pthread_cond_destroy((pthread_cond_t *)cond);
}
/* Function: al_thread_create
Description:
Create a thread for execution.
*/
int al_thread_create(al_thread_t * thread, void *(*start_routine)(void *), void * arg) {
	return pthread_create((pthread_t *)thread,NULL,start_routine,arg);
}
/* Function: al_thread_join
Description:
Wait for a thread to complete.
*/
int al_thread_join(al_thread_t al_thread, void **thread_return) {
	pthread_t thread;
	thread=*((pthread_t *)&al_thread); /* this unsafe cast is ok in this case since we know they are the same */
	return pthread_join(thread,thread_return);
}
#endif


void al_set_hardware_info(char *pdescription) {
	e_s32 tmp;
	th_parse_buf_flag(pdescription,"cores=",&tmp);
	hardware_info.num_processors=tmp;
	hardware_info.description_string=pdescription;
}

hardware_info_t hardware_info={1,NULL};

/* Function: al_thread_create_ex
	Create a new thread, using extra information to set specific hardware related parameters.
	This function is used by some work items to allow affinity on sub threads created by the work item.

	Parameters:
	thread - pointer to a thread structure
	start_routine - pointer to the function to invoke for the new thread
	arg - argument to pass to the start_routine
	tex - extra information for thread create extensions.
	
	Default implementation:
	Use the extra information to set thread affinity such that each sub item with id>0 is assigned
	affinity to a specific processor.
	* Default implementation will only set affinity if PTHREAD_SETAFFINITY_NP is enabled.
*/
int al_thread_create_ex(al_thread_t * pThread, void *(*start_routine)(void *), void * arg, mith_textend *tex) {
int retval=al_thread_create(pThread,start_routine,arg);
#if HAVE_PTHREAD_SETAFFINITY_NP
	cpu_set_t mask;
	unsigned long masksum = 0;

	if (retval<0)
		return retval;

	CPU_ZERO(&mask);
	
	if (tex->sub_item_id>=0) {
		CPU_SET(tex->sub_item_id % hardware_info.num_processors, &mask);
		retval=pthread_setaffinity_np(*pThread, sizeof(mask),&mask);
	}
#else
	if (tex!=NULL) 
		retval*=1;
#endif
	return retval;
}

/* Function: al_setaffinity
	Description:
	If affinity is supported, can potentially use affinity to set specific work items to specific affinity.
	This function controls the way affinity is assigned to new work items being executed.

	Parameters:
	Item identifiers - kernel and instance (unique) as well as serial item id, 
	and context_id representing the harness view of items running in parallel.

	Default implementation:
	Assign different affinity to each context created by the harness up to num_processors.

	Porting:
	This function needs to be ported if affinity for work items is desired.
	This affects the work item affinity at the harness level, since this function is 
	called before each work item starts initialization and processing.
*/
int al_item_setaffinity(int kernel_id, int instance_id, int item_id, e_u32 context_id) {
	int retval=0;
#if HAVE_PTHREAD_SETAFFINITY_NP && HAVE_PTHREAD_SELF
/* sample implementation that puts each context in a different affinity */
	cpu_set_t mask;
	unsigned long masksum = 0;
	CPU_ZERO(&mask);
	
	CPU_SET(context_id % hardware_info.num_processors, &mask);
	/* or alternatively, make sure each item has an affinity to a specific core
	CPU_SET(item_id % hardware_info.num_processors, &mask);
	*/
	retval=pthread_setaffinity_np(pthread_self(), sizeof(mask),&mask);
#else
	retval=kernel_id+instance_id+item_id+context_id;
#endif
	return retval;
}

