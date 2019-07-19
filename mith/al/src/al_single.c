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

/* File: mith/al/src/al_single.c
	Abstraction layer implementation for thread API

	This file contains implementation prototypes that only allow a single context.
*/
#include "th_lib.h"
#include "al_smp.h"

#if USE_SINGLE_CONTEXT==1
/* Function: al_mutex_init
Description: 
Initialize a mutex - single context, mutexes irrelevant.
*/
int al_mutex_init(al_mutex_t *mutex) {
	return 0;
}
/* Function: al_mutex_lock
Description: 
Lock a mutex.
*/
int al_mutex_lock(al_mutex_t *mutex) {
	return 0;
}
/* Function: al_mutex_trylock
Description:
Lock a mutex (non blocking), and return if mutex is already locked - single context, mutexes irrelevant.
*/
int al_mutex_trylock(al_mutex_t *mutex) {
	return 0;
}
/* Function: al_mutex_unlock
Description: 
Unlock a mutex - single context, mutexes irrelevant.
*/
int al_mutex_unlock(al_mutex_t *mutex) {
	return 0;
}
/* Function: al_mutex_destroy
Description:
Destroy a mutex previously initialized with <al_mutex_init> - single context, mutexes irrelevant.
*/
int al_mutex_destroy(al_mutex_t *mutex) {
	return 0;
}
/* Function: al_cond_init
Description:
Initialize a condition - single context, mutexes irrelevant.
*/
int al_cond_init(al_cond_t *cond) {
	return 0;
}
/* Function: al_cond_signal
Description:
Signal a condition - single context, mutexes irrelevant.
*/
int al_cond_signal(al_cond_t *cond) {
	return 0;
}
/* Function: al_cond_broadcast
Description:
Broadcast a condition - single context, mutexes irrelevant.
*/
int al_cond_broadcast(al_cond_t *cond) {
	return 0;
}
/* Function: al_cond_wait
Description:
Wait for a signal based on condition - single context, mutexes irrelevant.
*/
int al_cond_wait(al_cond_t *cond, al_mutex_t *mutex){
	return 0;
}
/* Function: al_cond_destroy
Description:
Destroy a condition previously initialized by <al_cond_init> - single context, mutexes irrelevant.
*/
int al_cond_destroy(al_cond_t *cond) {
	return 0;
}
/* Function: al_thread_create
Description:
Create a thread for execution - single context, simply call the function, and store the return value.
*/
int al_thread_create(al_thread_t * thread, void *(*start_routine)(void *), void * arg) {
	*thread=(al_thread_t)start_routine(arg);
	return 0;
}
/* Function: al_thread_join
Description:
Wait for a thread to complete - single context, just pass the return value.
*/
int al_thread_join(al_thread_t al_thread, void **thread_return) {
	*thread_return=(void *)al_thread;
	return 0;
}
#endif

