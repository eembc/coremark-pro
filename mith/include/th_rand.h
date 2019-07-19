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

#ifndef _TH_RAND_H_
#define _TH_RAND_H_

#include "th_types.h"
/* Function: rand_init
	Call this function to initialize a random number generator.
*/
void *rand_init(e_u32 seed, e_u32 reseed, e_f64 min, e_f64 max);
void *rand_intparts_init(e_u32 seed, e_u32 reseed, 	
	e_s32 mantsign,
	e_s32 expsign,
	e_u32 exp_cut,
	e_u32 manthigh_cut,
	e_u32 mantlow_cut);

/* Function: rand_fini
	Call this function to destroy a random number generator.
*/
void rand_fini(void *r);
/* Function: random_u32
	Call this function to get a 32b random integer.
*/
e_u32 random_u32(void *pr);
e_u32 random_u32_inrange(void *pr, e_u32 min, e_u32 max);
e_u32 *random_u32_vector(e_u32 size, e_u32 seed);
e_u8 *random_u8_vector(e_u32 size, e_u32 seed);

/* Function: random_f64_01
	Call this function to get a floating point random integer in the range [0..1].
*/
e_f64 random_f64_01(void *pr);
/* Function: random_f64_01
	Call this function to get a floating point random integer 
	in the range min/max as initialized for the RNG.
*/
e_f64 random_f64(void *pr);
/* Function: random_f64_vector
	Create a vector based on RNG that was previously allocated 
 */
e_f64 *random_f64_vector(int N,void *pr);
/* Function: mixed_f64_vector
	Create a vector with random values in 4 ranges ([-1e-10..1e-10],[1e20..1e40],[-1e6..1e6],[-1e40..1e40])
*/
e_f64 *mixed_f64_vector(int N, e_u32 seed);
/* Function: fromint_f64_vector
	Create a vector with random values in 4 ranges using integer math only
*/
e_f64 *fromint_f64_vector(int N, e_u32 seed);
e_f32 *fromint_f32_vector(int N, e_u32 seed);
/* Function: fromint_f64_01_vector
	Create a vector with random values in 4 ranges using integer math only. All values are within [0,1] (positive mantissa, negative exponent)
*/
e_f64 *fromint_f64_01_vector(int N, e_u32 seed);
e_f32 *fromint_f32_01_vector(int N, e_u32 seed);

e_f64 *fromint_f64_01_vector_uniform(int N, e_u32 seed);
e_f32 *fromint_f32_01_vector_uniform(int N, e_u32 seed);
/* Function: fromint_random_f64_vector
	Create a vector with randoms based on RNG that was previously allocated for intparts
 */
e_f64 *fromint_random_f64_vector(int N,void *pr);
e_f32 *fromint_random_f32_vector(int N,void *pr);

e_f32 precise_random_f32(void *pr);
e_f64 precise_random_f64(void *pr);
#if ( USE_FP64 )
	#define precise_random_fp precise_random_f64
	#define fromint_random_fp_vector fromint_random_f64_vector
	#define fromint_fp_01_vector fromint_f64_01_vector
	#define fromint_fp_01_vector_uniform fromint_f64_01_vector_uniform
	#define fromint_fp_vector fromint_f64_vector
	#define EE_LIMIT_DYNAMIC_RANGE(x) (x)
#endif
#if ( USE_FP32 )
	#define precise_random_fp precise_random_f32
	#define fromint_random_fp_vector fromint_random_f32_vector
	#define fromint_fp_01_vector fromint_f32_01_vector
	#define fromint_fp_01_vector_uniform fromint_f32_01_vector_uniform
	#define fromint_fp_vector fromint_f32_vector
	#ifdef TEST_LIMITED_DYNAMIC_RANGE
		/* Limit to N uppermost bits for the mantissa */
		#ifndef EE_NUM_LIMIT_BITS
		#define EE_NUM_LIMIT_BITS 5
		#endif
		#define EE_LIMIT_DYNAMIC_RANGE(x) (((1<<EE_NUM_LIMIT_BITS) - 1) << (22-EE_NUM_LIMIT_BITS))
	#else
		#define EE_LIMIT_DYNAMIC_RANGE(x) (x)
	#endif
#endif


#define random_fp(R) (1.0/(65536.0*65536.0))*random_u32((rand_state *)R)
#define random_fp_inrange(R) (R)->min+random_fp(R)*(R)->range

typedef enum SIGNRAND_E
{
	RAND=0,
	POS=1,
	NEG=2
} SIGNRAND;

e_fp th_exp2(e_fp x);
e_fp complex_rand_fp(
	SIGNRAND sign_rand,
	e_s32 min_exp,
	e_s32 max_exp,
	e_u32 meaningfull_bits,
	void *pr);

#endif //_TH_RAND_H_

