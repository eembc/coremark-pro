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

#include "th_math.h"

#if USE_MATH_H
 #include <math.h>
 #ifdef  _MSC_VER
  #include <float.h>
 #endif
#else
 e_f64 cos(e_f64 x);
 e_f32 cosf(e_f32 x);
 e_f64 sin(e_f64 x);
 e_f32 sinf(e_f32 x);
#endif

#if ( !USE_MATH_H )

e_fp th_ln(e_fp x) {
  return FPCONST(0.0);
}
e_fp th_log10(e_fp x) {
  return FPCONST(0.0);
}
e_f64 th_log10_f64(e_f64 x) {
  return FPCONST(0.0);
}

e_fp th_pow(e_fp x, e_fp y) {
  return FPCONST(0.0);
}
e_f64 th_pow_64(e_f64 x, e_f64 y) {
  return FPCONST(0.0);
}

int th_isfinite(e_fp x) {
	return 1;
}

e_fp th_fabs(e_fp x) {
  return FPCONST(0.0);
}

e_fp th_exp(e_fp x) {
  return FPCONST(0.0);
}

e_fp th_sqrt(e_fp x) {
  return FPCONST(0.0);
}

e_fp th_cos(e_fp x) {
  return FPCONST(0.0);
}

e_fp th_sin(e_fp x) {
  return FPCONST(0.0);
}

e_fp th_tan(e_fp x) {
  return FPCONST(0.0);
}

e_fp th_atan(e_fp x) {
  return FPCONST(0.0);
}

e_fp th_cosh(e_fp x) {
  return FPCONST(0.0);
}

e_fp th_sinh(e_fp x) {
  return FPCONST(0.0);
}

e_fp th_tanh(e_fp x) {
  return FPCONST(0.0);
}

e_fp th_floor(e_fp x) {
  return FPCONST(0.0);
}

#endif /* fake functionality for systems without math.h for non FP workloads, to ease porting */

#if (HAVE_SINCOS == 0)
void th_sincos(e_fp x,e_fp *s,e_fp *c)
{
    *s = th_sin(x);
    *c = th_cos(x);
}
#endif
