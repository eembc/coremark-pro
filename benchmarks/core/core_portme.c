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

/* 
	File: core_portme.c
*/
/*
	Author : Shay Gal-On, EEMBC
*/ 
#include "th_cfg.h"
#include "th_lib.h"
#include "coremark.h"

#if (MEM_METHOD==MEM_MALLOC)
void *portable_malloc(size_t size) {
	return th_calloc(size,1);
}
void portable_free(void *p) {
	th_free(p);
}
#else
#error "MITH version requires MEM_MALLOC"
#endif

#if (SEED_METHOD==SEED_VOLATILE)
#if VALIDATION_RUN
	volatile ee_s32 seed1_volatile=0x3415;
	volatile ee_s32 seed2_volatile=0x3415;
	volatile ee_s32 seed3_volatile=0x66;
#endif
#if PERFORMANCE_RUN
	volatile ee_s32 seed1_volatile=0x0;
	volatile ee_s32 seed2_volatile=0x0;
	volatile ee_s32 seed3_volatile=0x66;
#endif
#if PROFILE_RUN
	volatile ee_s32 seed1_volatile=0x8;
	volatile ee_s32 seed2_volatile=0x8;
	volatile ee_s32 seed3_volatile=0x8;
#endif
	volatile ee_s32 seed4_volatile=ITERATIONS;
	volatile ee_s32 seed5_volatile=0;
#endif
#if USE_CLOCK
	#define NSECS_PER_SEC CLOCKS_PER_SEC
	#define EE_TIMER_TICKER_RATE 1000
	#define CORETIMETYPE clock_t 
	#define GETMYTIME(_t) (*_t=clock())
	#define MYTIMEDIFF(fin,ini) ((fin)-(ini))
	#define TIMER_RES_DIVIDER 1
	#define SAMPLE_TIME_IMPLEMENTATION 1
#elif defined(_MSC_VER)
	#define NSECS_PER_SEC 10000000
	#define EE_TIMER_TICKER_RATE 1000
	#define CORETIMETYPE FILETIME
	#define GETMYTIME(_t) GetSystemTimeAsFileTime(_t)
	#define MYTIMEDIFF(fin,ini) (((*(__int64*)&fin)-(*(__int64*)&ini))/TIMER_RES_DIVIDER)
	/* setting to millisces resolution by default with MSDEV */
	#ifndef TIMER_RES_DIVIDER
	#define TIMER_RES_DIVIDER 1000
	#endif
	#define SAMPLE_TIME_IMPLEMENTATION 1
#elif HAS_TIME_H
	#define NSECS_PER_SEC 1000000000
	#define EE_TIMER_TICKER_RATE 1000
	#define CORETIMETYPE struct timespec 
	#define GETMYTIME(_t) clock_gettime(CLOCK_REALTIME,_t)
	#define MYTIMEDIFF(fin,ini) ((fin.tv_sec-ini.tv_sec)*(NSECS_PER_SEC/TIMER_RES_DIVIDER)+(fin.tv_nsec-ini.tv_nsec)/TIMER_RES_DIVIDER)
	/* setting to 1/1000 of a second resolution by default with linux */
	#ifndef TIMER_RES_DIVIDER
	#define TIMER_RES_DIVIDER 1000000
	#endif
	#define SAMPLE_TIME_IMPLEMENTATION 1
#else
	#define SAMPLE_TIME_IMPLEMENTATION 0
#endif
#define EE_TICKS_PER_SEC (NSECS_PER_SEC / TIMER_RES_DIVIDER)

/** Using MITH for timing */
static char *mith_msg="NA - MITH VERSION!\n";

ee_u32 default_num_contexts=MULTITHREAD;

void portable_init(core_portable *p, int *argc, char *argv[])
{
	if (sizeof(ee_ptr_int) != sizeof(ee_u8 *)) {
		ee_printf("ERROR! Please define ee_ptr_int to a type that holds a pointer!\n");
	}
	if (sizeof(ee_u32) != 4) {
		ee_printf("ERROR! Please define ee_u32 to a 32b unsigned type!\n");
	}
	
	p->portable_id=1;
}
void portable_fini(core_portable *p)
{
	p->portable_id=0;
}

ee_u8 core_start_parallel(core_results *res) {
	th_printf(mith_msg);
	return 0;
}
ee_u8 core_stop_parallel(core_results *res) {
	th_printf(mith_msg);
	return 0;
}

