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

/* Route to standard, and FPBench-specific, math functions */
#ifndef TH_MATH_H
#define TH_MATH_H

/* Standard math functions and constants */
#if USE_MATH_H
#include <math.h>
#ifdef  _MSC_VER
#include <float.h>
#endif
#endif

/* Configuration */
#include "th_cfg.h"
#include "th_types.h"

/* Route to math functions for DP configuration */
#if USE_FP64
#define th_cos cos
#define th_cosh cosh
#define th_exp exp
#define th_fabs fabs
#define th_floor floor
#define th_ln log
#define th_log10 log10
#define th_pow pow
#define th_sin sin
#define th_sinh sinh
#define th_sqrt sqrt
#define th_tan tan
#define th_tanh tanh
#define th_atan atan
#endif	

#define th_pow_64 pow

/* Route to math functions for SP configuration */
#if USE_FP32
#define th_cos cosf
#define th_cosh coshf
#define th_exp expf
#define th_fabs fabsf
#define th_floor floorf
#define th_ln logf
#define th_log10 log10f
#define th_pow powf
#define th_sin sinf
#define th_sinh sinhf
#define th_sqrt sqrtf
#define th_tan tanf
#define th_tanh tanhf
#define th_atan atanf
#endif	

/* FPBench-specific constants */
#define EE_PI FPCONST(3.1415926535897932)
#if USE_FP64
	#define EE_MININI FPCONST(1.0e100)
	#define EE_MAXINI FPCONST(-1.0e100)
	#define EE_EPSINI FPCONST(1.0e-100)
	#define EE_EPSILON FPCONST(1.0e-50)
#endif
#if USE_FP32
	#define EE_MININI FPCONST(1.0e38)
	#define EE_MAXINI FPCONST(-1.0e38)
	#define EE_EPSINI FPCONST(1.0e-37)
	#define EE_EPSILON EE_EPSINI
#endif

/* FPBench-specific functions */
/* The test harness is th_isfinite function needs to provide a method that will detect if a number is not infinite and not denormal */
/* implementation using fpclass(ify) for gcc and MSVC is provided.                                                                  */
/* Use AL_ISFINITE to provide your own implementation if fpclass(ify) is not available on your platform.                            */
#if AL_ISFINITE
  #if USE_FP64
     int al_isfinite_dp(e_f64 x);
     #define th_isfinite al_isfinite_dp
  #endif
  #if USE_FP32
     int al_isfinite_sp(e_f32 x);
     #define th_isfinite al_isfinite_sp
  #endif
#else
#ifdef  _MSC_VER
#define th_isfinite(x) \
    ((_fpclass(x) == _FPCLASS_PZ) || (_fpclass(x) == _FPCLASS_NZ) || \
     (_fpclass(x) == _FPCLASS_PN) || (_fpclass(x) == _FPCLASS_NN))
#else
#define th_isfinite(x) \
    ((fpclassify(x) == FP_NORMAL) || (fpclassify(x) == FP_ZERO))
#endif
#endif

#define th_log10_f64 log10
/* As per WG decision, do not allow non-std sincos builtin */
#define HAVE_SINCOS 0
void th_sincos(e_fp x,e_fp *s,e_fp *c);

#endif /*TH_MATH_H*/
