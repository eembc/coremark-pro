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

#ifndef _AL_SMP_H
#define _AL_SMP_H

#include "th_types.h"
/* File: al_smp.h
 Abstractions of thread functions 
 EEMBC AL defines 3 types: thread, mutex and condition, 
 and functions to create and use them. 
 These functions are used by MITH to run and manage workloads.
 They should also be used by benchmarks that implement concurrency 
 within the scope of the benchmark.
*/

 /* Basic primitives */
#if USE_NATIVE_PTHREAD==1
	#include <pthread.h>
	#define al_thread_t pthread_t
	#define al_mutex_t pthread_mutex_t
	#define al_cond_t pthread_cond_t

	#define al_mutex_init(_mutex) pthread_mutex_init(_mutex,NULL)
	#define al_mutex_lock(a) pthread_mutex_lock(a)
	#define al_mutex_trylock(a) pthread_mutex_trylock(a)
	#define al_mutex_unlock(a) pthread_mutex_unlock(a)
	#define al_mutex_destroy(a) pthread_mutex_destroy(a)

	#define al_cond_init(_cond) pthread_cond_init(_cond,NULL)
	#define al_cond_signal(a) pthread_cond_signal(a)
	#define al_cond_broadcast(a) pthread_cond_broadcast(a)
	#define al_cond_wait(a,b) pthread_cond_wait((a),(b))
	#define al_cond_destroy(a) pthread_cond_destroy(a)

	#define al_thread_create(_thread, _start_r, _arg) pthread_create(_thread,NULL,_start_r,_arg)
	#define al_thread_join(a,b) pthread_join(a,b)

#else /* pthread library not availble */
#if USE_SINGLE_CONTEXT
	typedef void *al_thread_t;		/* dummy */
	typedef void *al_mutex_t;		/* dummy */
	typedef void *al_cond_t;		/* dummy */
#elif AL_THREAD_U32
/* thread type is a u32 */
	typedef e_u32 al_thread_t;		/* identify a thread */
	typedef e_u32 al_mutex_t;		/* identify a mutex */
	typedef e_u32 al_cond_t;		/* identify a condition variable */
#elif AL_THREAD_VC
/* thread type from the win32 pthread library */
	typedef struct {
		void * p;                   /* Pointer to actual object */
		unsigned int x;             /* Extra information - reuse count etc */
	} al_thread_t;		/* identify a thread */
	typedef void *al_mutex_t;		/* identify a mutex */
	typedef void *al_cond_t;		/* identify a condition variable */
#else
/* other? */
 #error "What is the type of a thread handle?"
#endif
/* Function: al_mutex_init
	Initialize a mutex.
*/
int al_mutex_init(al_mutex_t *mutex);
/* Function: al_mutex_lock
	Lock a mutex.
*/
int al_mutex_lock(al_mutex_t *mutex);
/* Function: al_mutex_trylock
	Non-blocking mutex lock. 
*/
int al_mutex_trylock(al_mutex_t *mutex);
/* Function: al_mutex_unlock
	Unlock a mutex. 
*/
int al_mutex_unlock(al_mutex_t *mutex);
/* Function: al_mutex_lock
	Destroy a mutex.
*/
int al_mutex_destroy(al_mutex_t *mutex);

/* Function: al_cond_init
	Initialize a cond variable.
*/
int al_cond_init(al_cond_t *cond);
/* Function: al_cond_signal
	Signal on a cond variable.
*/
int al_cond_signal(al_cond_t *cond);
/* Function: al_cond_broadcast
	Broadcast on a cond variable.
*/
int al_cond_broadcast(al_cond_t *cond);
/* Function: al_cond_wait
	Wait on a cond variable, releasing an associated mutex.
*/
int al_cond_wait(al_cond_t *cond, al_mutex_t *mutex);
/* Function: al_cond_destroy
	Destroy a cond variable.
*/
int al_cond_destroy(al_cond_t *cond);

/* Function: al_thread_create
	Create a new thread

	Parameters:
	thread - pointer to a thread structure
	start_routine - pointer to the function to invoke for the new thread
	arg - argument to pass to the start_routine
*/
int al_thread_create(al_thread_t * thread, void *(*start_routine)(void *), void * arg);
/* Function: al_thread_join
	Wait for a thread to complete.
*/
int al_thread_join(al_thread_t thread, void **thread_return);
#endif /* of not using native pthread */

/* Function : al_timer_create
	TODO : Create a new timer.
	The timer will signal after _millisecs_ passed.  
*/
void al_timer_create(e_u32 millisecs); 
/* Function : al_timer_destroy
	TODO : Destroy a timer created with <al_timer_create>.
*/
void al_timer_destroy(void);

/* Structure: mith_iextend
	Extended information on work items
*/
typedef struct mith_iextend_s {
	e_u32	item_id;
	e_u32	kernel_id;
	e_u32	verify;
} mith_iextend;
/* Structure: mith_textend
	Extended information for new threads created
*/
typedef struct mith_textend_s {
	/* item, kernel and sub_item ids, currently to be used for affinity */
	e_u32	item_id;
	e_u32	kernel_id;
	e_u32	sub_item_id;
} mith_textend;
/* Function: al_thread_create_ex
	Create a new thread, potentially setting affinity.
	This function is used by some work items to allow affinity on sub threads created by the work item.

	Parameters:
	thread - pointer to a thread structure
	start_routine - pointer to the function to invoke for the new thread
	arg - argument to pass to the start_routine
	tex - extra information for thread create extensions.
*/
int al_thread_create_ex(al_thread_t * thread, void *(*start_routine)(void *), void * arg, mith_textend *tex);

/* Function: al_setaffinity
	Description:
	If affinity is supported, can potentially use affinity to set specific work items to specific affinity.
	This function controls the way affinity is assigned to new work items being executed.

	Parameters:
	Item identifiers - kernel and instance (unique) as well as serial item id, 
	and context_id representing the harness view of items running in parallel.

	Porting:
	This function needs to be ported if affinity for work items is desired.
	This affects the work item affinity at the harness level, since this function is 
	called before each work item starts initialization and processing.
*/
int al_item_setaffinity(int kernel_id, int instance_id, int item_id, e_u32 context_id);

/* Structure: hardware_info_t
	Structure containing hardware specific information

	Porting:
	This structure can be changed when porting to different platforms, even for OTB cerstifications 
*/
typedef struct hardware_info_s {
	int num_processors;
	char *description_string;
	/* this structure may be extended with info regarding specific processor capabilities,
	   and other platform specific information */
} hardware_info_t;

/* Variable: hardware_info
	Global value, set before any threads are created.
	Once actual execution starts, this structure must be read only 
*/
extern hardware_info_t hardware_info;

/* Function: al_set_hardware_info
	Set hardware specific information from a string.

	Porting:
	This function must be ported to specific hardware if intending to use 
	information more then just number of processors.
*/
void al_set_hardware_info(char *pdescription);

#endif /*_AL_SMP_H*/
