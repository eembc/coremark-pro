/* $NetBSD: fpgetsticky.c,v 1.3 2008/04/28 20:23:00 martin Exp $ */

/*-
 * Copyright (c) 1997 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Neil A. Carson and Mark Brinicombe
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "softfloat-for-gcc.h"
#include "milieu.h"
#include "softfloat.h"

typedef union {
   float64 f64;
   unsigned long long ull;
   double d;
} convert_f64;

typedef union {
   float32 f32;
   float f;
} convert_f32;

int __eqdf2(double a, double b)
{
   convert_f64 _a, _b;
   _a.d = a;
   _b.d = b;
	/* libgcc1.c says !(a == b) */
	return !float64_eq(_a.f64, _b.f64);
}

int __eqsf2(float a, float b)
{
   convert_f32 _a, _b;
   _a.f = a;
   _b.f = b;
	/* libgcc1.c says !(a == b) */
	return !float32_eq(_a.f32, _b.f32);
}

int __gedf2(double a, double b)
{
   convert_f64 _a, _b;
   _a.d = a;
   _b.d = b;
	/* libgcc1.c says (a >= b) - 1 */
	return float64_le(_b.f64, _a.f64) - 1;
}

int __gesf2(float a, float b)
{
   convert_f32 _a, _b;
   _a.f = a;
   _b.f = b;
	/* libgcc1.c says (a >= b) - 1 */
	return float32_le(_b.f32, _a.f32) - 1;
}

int __gtdf2(double a, double b)
{
   convert_f64 _a, _b;
   _a.d = a;
   _b.d = b;
	/* libgcc1.c says a > b */
	return float64_lt(_b.f64, _a.f64);
}

int __gtsf2(float a, float b)
{
   convert_f32 _a, _b;
   _a.f = a;
   _b.f = b;
	/* libgcc1.c says a > b */
	return float32_lt(_b.f32, _a.f32);
}

int __ledf2(double a, double b)
{
   convert_f64 _a, _b;
   _a.d = a;
   _b.d = b;
	/* libgcc1.c says 1 - (a <= b) */
	return 1 - float64_le(_a.f64, _b.f64);
}

int __lesf2(float a, float b)
{
   convert_f32 _a, _b;
   _a.f = a;
   _b.f = b;
	/* libgcc1.c says 1 - (a <= b) */
	return 1 - float32_le(_a.f32, _b.f32);
}

int __ltdf2(double a, double b)
{
   convert_f64 _a, _b;
   _a.d = a;
   _b.d = b;
	/* libgcc1.c says -(a < b) */
	return -float64_lt(_a.f64, _b.f64);
}

int __ltsf2(float a, float b)
{
   convert_f32 _a, _b;
   _a.f = a;
   _b.f = b;
	/* libgcc1.c says -(a < b) */
	return -float32_lt(_a.f32, _b.f32);
}

int __nedf2(double a, double b)
{
   convert_f64 _a, _b;
   _a.d = a;
   _b.d = b;
	/* libgcc1.c says a != b */
	return !float64_eq(_a.f64, _b.f64);
}

double __negdf2(double a)
{
   convert_f64 _a, _b;
   _a.d = a;
	/* libgcc1.c says -a */
	_b.ull = _a.ull ^ 0x8000000000000000ULL;
   return _b.d;
}

float __negsf2(float a)
{
   convert_f32 _a, _b;
   _a.f = a;
	/* libgcc1.c says INTIFY(-a) */
   _b.f32 = _a.f32 ^ 0x80000000;
   return _b.f;
}

int __nesf2(float a, float b)
{
   convert_f32 _a, _b;
   _a.f = a;
   _b.f = b;
	/* libgcc1.c says a != b */
	return !float32_eq(_a.f32, _b.f32);
}

int __unorddf2(double a, double b)
{
   convert_f64 _a, _b;
   _a.d = a;
   _b.d = b;
	/*
	 * The comparison is unordered if either input is a NaN.
	 * Test for this by comparing each operand with itself.
	 * We must perform both comparisons to correctly check for
	 * signalling NaNs.
	 */
	return 1 ^ (float64_eq(_a.f64, _a.f64) & float64_eq(_b.f64, _b.f64));
}

int __unordsf2(float a, float b)
{
   convert_f32 _a, _b;
   _a.f = a;
   _b.f = b;
	/*
	 * The comparison is unordered if either input is a NaN.
	 * Test for this by comparing each operand with itself.
	 * We must perform both comparisons to correctly check for
	 * signalling NaNs.
	 */
	return 1 ^ (float32_eq(_a.f32, _a.f32) & float32_eq(_b.f32, _b.f32));
}


float __floatunsisf (unsigned int i)
{
   convert_f32 y;

   int64 x = (int64)i;
   y.f32 = int64_to_float32(x);
   return y.f;
}

double __floatunsidf (unsigned int i)
{
   convert_f64 y;

   int64 x = (int64)i;
   y.f64 = int64_to_float64(x);
   return y.d;
}

