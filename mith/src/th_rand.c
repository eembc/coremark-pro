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

#include "th_lib.h"
#include "th_rand.h"
#include "th_math.h"
#include "th_al.h"

typedef struct rand_state_s {
	e_u32 idx;
	e_u32 rsl[256];
    e_u32 mm[256];
    e_u32 aa, bb, cc;
	e_u32 initial_seed;
	e_u32 reseed;
	e_fp range;
	e_fp min;
	e_s32 mantsign; // negative mantsign, force negative. zero, force positive. +1, random. 
	e_s32 expsign;  // negative expsign, force negative. zero, force positive, +1, random.
	e_u16 exp_cut;
	e_u32 manthigh_cut;
	e_u32 mantlow_cut;

} rand_state;

static void isaac(void *pr)
{
   register e_u32 i,x,y;
   rand_state *r=(rand_state *)pr;
   e_u32 *mm=r->mm;
   e_u32 *randrsl=r->rsl;
   e_u32 aa=r->aa, bb=r->bb, cc=r->cc;

   cc = cc + 1;    /* cc just gets incremented once per reseed results */
   bb = bb + cc;   /* then combined with bb */

   for (i=0; i<256; ++i)
   {
     x = mm[i];
     switch (i%4)
     {
     case 0: aa = aa^(aa<<13); break;
     case 1: aa = aa^(aa>>6); break;
     case 2: aa = aa^(aa<<2); break;
     case 3: aa = aa^(aa>>16); break;
     }
     aa              = mm[(i+128)%256] + aa;
     mm[i]      = y  = mm[(x>>2)%256] + aa + bb;
     randrsl[i] = bb = mm[(y>>10)%256] + x;
   }
   r->aa=aa;r->bb=bb;r->cc=cc;
}

#define mix(a,b,c,d,e,f,g,h) \
{ \
   a^=b<<11; d+=a; b+=c; \
   b^=c>>2;  e+=b; c+=d; \
   c^=d<<8;  f+=c; d+=e; \
   d^=e>>16; g+=d; e+=f; \
   e^=f<<10; h+=e; f+=g; \
   f^=g>>4;  a+=f; g+=h; \
   g^=h<<8;  b+=g; h+=a; \
   h^=a>>9;  c+=h; a+=b; \
}

void rand_fini(void *r) {
	th_free(r);
}

void *rand_init(e_u32 seed, e_u32 reseed, e_f64 min, e_f64 max)
{
   int i;
   e_u32 a,b,c,d,e,f,g,h;
   rand_state *r=(rand_state *)th_malloc(sizeof(rand_state));
   e_u32 *mm=r->mm;

   r->reseed=0xff*reseed;
   r->range=max-min;
   r->min=min;
   if (r->reseed==0) r->reseed=0xffffffff;
   r->aa=r->bb=r->cc=0;
   r->initial_seed=a=b=c=d=e=f=g=h=seed;  
   r->idx=0;

   for (i=0; i<4; ++i)          /* scramble the seed */
   {
     mix(a,b,c,d,e,f,g,h);
   }

   for (i=0; i<256; i+=8)   /* fill in mm[] with messy stuff */
   {
     mix(a,b,c,d,e,f,g,h);
     mm[i  ]=a; mm[i+1]=b; mm[i+2]=c; mm[i+3]=d;
     mm[i+4]=e; mm[i+5]=f; mm[i+6]=g; mm[i+7]=h;
   }

   isaac(r);            /* fill in the first set of results */
   return r;
}

void *rand_intparts_init(e_u32 seed, e_u32 reseed, 	
	e_s32 mantsign,
	e_s32 expsign,
	e_u32 exp_cut,
	e_u32 manthigh_cut,
	e_u32 mantlow_cut) {
	rand_state *r=(rand_state *)rand_init(seed,reseed,0.0,0.0);
	r->mantsign=mantsign;
	r->expsign=expsign;
	r->exp_cut=(e_u16)exp_cut;
	r->manthigh_cut=manthigh_cut;
	r->mantlow_cut=mantlow_cut;
	return r;
}


e_u32 random_u32(void *pr) {
	rand_state *r=(rand_state *)pr;
	if (r==NULL)
		return 0;
	r->idx++;
	if (r->idx > r->reseed) { isaac(r); r->idx=0; }
	return r->rsl[r->idx&0xff];
}
e_u32 random_u32_inrange(void *pr, e_u32 min, e_u32 max) {
	rand_state *r=(rand_state *)pr;
	e_u32 ret=random_u32(r);
	if (min==max) return min;
	return ((ret % (max-min)) + min);
}
e_u8 random_u8(void *pr) {
	rand_state *r=(rand_state *)pr;
	return ((e_u8)random_u32(r));
}
e_u16 random_u16(void *pr) {
	rand_state *r=(rand_state *)pr;
	return ((e_u16)random_u32(r));
}
e_u32 *random_u32x256(void *pr) {
	rand_state *r=(rand_state *)pr;
	isaac(r); 
	return r->rsl;
}
e_u32 *random_u32_vector(e_u32 size, e_u32 seed) {
	void *r=rand_init(seed,256,-1e10,1e10);
	e_u32 N=size;
	e_u32 *p=(e_u32 *)th_malloc(sizeof(e_u32)*N);
	while (N-->0) {
		p[N]=random_u32(r);
	}
	rand_fini(r);
	return p;
}
e_u8 *random_u8_vector(e_u32 size, e_u32 seed) {
	void *r=rand_init(seed,256,-1e10,1e10);
	e_u32 N=size;
	e_u8 *p=(e_u8 *)th_malloc(sizeof(e_u8)*N);
	while (N-->0) {
		p[N]=random_u8(r);
	}
	rand_fini(r);
	return p;
}
e_f32 random_f32_01(void *pr) {
	e_f32 res=(e_f32)random_fp(pr);
	return res;
}
e_f64 random_f64_01(void *pr) {
	e_f64 res=random_fp(pr);
	return res;
}
e_f64 random_f64(void *pr) {
	rand_state *r=(rand_state *)pr;
	e_f64 res=r->min+random_f64_01(r)*r->range;
	return res;
}
e_f64 precise_random_f64(void *pr) {
	rand_state *r=(rand_state *)pr;
	e_f64 res;
	intparts num;
	num.sign=r->mantsign < 0 ?  1 : (r->mantsign  & (random_u32(pr)&1))==0 ? 0  :  1;  // negative mantsign, force negative. zero, force positive. +1, random. 
	num.exp= r->expsign  < 0 ? -1 : (r->expsign   & (random_u32(pr)&1))==0 ? 1  : -1;  // negative expsign, force negative. zero, force positive, +1, random.
	num.mant_high32=random_u32(pr);
	num.mant_low32=random_u32(pr);
	num.exp *= 1 + (random_u16(pr) & (r->exp_cut));
	num.mant_high32 &= r->manthigh_cut;
	num.mant_high32 |= (((e_u32)1)<<20);
	num.mant_low32 &= r->mantlow_cut;
	store_dp(&res,&num);
	return res;
}
e_f32 random_f32(void *pr) {
	rand_state *r=(rand_state *)pr;
	e_f32 res=(e_f32)r->min+random_f32_01(r)*(e_f32)(r->range);
	return res;
}
e_f32 precise_random_f32(void *pr) {
	rand_state *r=(rand_state *)pr;
	e_f32 res;
	intparts num;
	num.sign=r->mantsign < 0 ?  1 : (r->mantsign  & (random_u32(pr)&1))==0 ? 0  :  1;  // negative mantsign, force negative. zero, force positive. +1, random. 
	num.exp= r->expsign  < 0 ? -1 : (r->expsign   & (random_u32(pr)&1))==0 ? 1  : -1;  // negative expsign, force negative. zero, force positive, +1, random.
	num.mant_low32=random_u32(pr);
	num.exp *= 1 + (random_u16(pr) & (r->exp_cut));
	num.mant_low32 &= r->mantlow_cut;
	num.mant_low32 |= (((e_u32)1)<<23); 
	num.mant_high32=0;
	store_sp(&res,&num);
	return res;
}
e_f64 *fromint_f64_vector(int N, e_u32 seed) {
	e_f64 *v=(e_f64 *)th_malloc(sizeof(e_f64)*N);
	int i=0;
	void *R[4];
	if (seed==0) {							// m+-  ,e+-,ecut	,high		,low
		R[0]=rand_intparts_init(0x9e3779b9,256,0	,0	,0x7	,0			,0xffffffff);
		R[1]=rand_intparts_init(0x73686179,256,1	,1	,0x3	,0x0000ffff	,0xffffffff);
		R[2]=rand_intparts_init(0x656d6263,256,0	,1	,0x7	,0x0000ffff	,0x00000000);
		R[3]=rand_intparts_init(0xee6dbbcc,256,1	,0	,0xf	,0			,0x0000ffff);
	} else {
		R[0]=rand_intparts_init(seed,256	  ,0	,0	,0x7	,0			,0xffffffff);
		R[1]=rand_intparts_init(seed,256	  ,1	,1	,0x3	,0x0000ffff	,0xffffffff);
		R[2]=rand_intparts_init(seed,256	  ,0	,1	,0x7	,0x0000ffff	,0x00000000);
		R[3]=rand_intparts_init(seed,256	  ,1	,0	,0xf	,0			,0x0000ffff);
	}
	for (i=0 ; i<N ;i++) v[i]=precise_random_f64(R[i&3]);
	for (i=0 ; i<4 ;i++) rand_fini(R[i]);
	return v;
}

e_f32 *fromint_f32_vector(int N, e_u32 seed) {
	e_f32 *v=(e_f32 *)th_malloc(sizeof(e_f32)*N);
	int i=0;
	void *R[4];
	if (seed==0) {							// m+-  ,e+-,ecut	,high		,low
		R[0]=rand_intparts_init(0x9e3779b9,256,0	,0	,0x7	,0			,EE_LIMIT_DYNAMIC_RANGE(0x00ffffff));
		R[1]=rand_intparts_init(0x73686179,256,1	,1	,0x3	,0			,EE_LIMIT_DYNAMIC_RANGE(0x00ffffff));
		R[2]=rand_intparts_init(0x656d6263,256,0	,1	,0x7	,0			,EE_LIMIT_DYNAMIC_RANGE(0x000fffff));
		R[3]=rand_intparts_init(0xee6dbbcc,256,1	,0	,0xf	,0			,EE_LIMIT_DYNAMIC_RANGE(0x0000ffff));
	} else {
		R[0]=rand_intparts_init(seed,256	  ,0	,0	,0x7	,0			,EE_LIMIT_DYNAMIC_RANGE(0x00ffffff));
		R[1]=rand_intparts_init(seed,256	  ,1	,1	,0x3	,0			,EE_LIMIT_DYNAMIC_RANGE(0x00ffffff));
		R[2]=rand_intparts_init(seed,256	  ,0	,1	,0x7	,0			,EE_LIMIT_DYNAMIC_RANGE(0x000fffff));
		R[3]=rand_intparts_init(seed,256	  ,1	,0	,0xf	,0			,EE_LIMIT_DYNAMIC_RANGE(0x0000ffff));
	}
	for (i=0 ; i<N ;i++) v[i]=precise_random_f32(R[i&3]);
	for (i=0 ; i<4 ;i++) rand_fini(R[i]);
	return v;
}

e_f64 *fromint_f64_01_vector(int N, e_u32 seed) {
	e_f64 *v=(e_f64 *)th_malloc(sizeof(e_f64)*N);
	int i=0;
	void *R[4];
	if (seed==0) {							// m+-  ,e+-,ecut	,high		,low
		R[0]=rand_intparts_init(0x9e3779b9,256,0	,-1	,0x7	,0			,0xffffffff);
		R[1]=rand_intparts_init(0x73686179,256,0	,-1	,0x3	,0x0000ffff	,0xffffffff);
		R[2]=rand_intparts_init(0x656d6263,256,0	,-1	,0x7	,0x0000ffff	,0x00000000);
		R[3]=rand_intparts_init(0xee6dbbcc,256,0	,-1	,0xf	,0			,0x0000ffff);
	} else {
		R[0]=rand_intparts_init(seed,256	  ,0	,-1	,0x7	,0			,0xffffffff);
		R[1]=rand_intparts_init(seed,256	  ,0	,-1	,0x3	,0x0000ffff	,0xffffffff);
		R[2]=rand_intparts_init(seed,256	  ,0	,-1	,0x7	,0x0000ffff	,0x00000000);
		R[3]=rand_intparts_init(seed,256	  ,0	,-1	,0xf	,0			,0x0000ffff);
	}
	for (i=0 ; i<N ;i++) v[i]=precise_random_f64(R[i&3]);
	for (i=0 ; i<4 ;i++) rand_fini(R[i]);
	return v;
}

e_f32 *fromint_f32_01_vector(int N, e_u32 seed) {
	e_f32 *v=(e_f32 *)th_malloc(sizeof(e_f32)*N);
	int i=0;
	void *R[4];
	if (seed==0) {							// m+-  ,e+-,ecut	,high		,low
		R[0]=rand_intparts_init(0x9e3779b9,256,0	,-1	,0x7	,0	,EE_LIMIT_DYNAMIC_RANGE(0x00ffffff));
		R[1]=rand_intparts_init(0x73686179,256,0	,-1	,0x3	,0	,EE_LIMIT_DYNAMIC_RANGE(0x00ffffff));
		R[2]=rand_intparts_init(0x656d6263,256,0	,-1	,0x7	,0	,EE_LIMIT_DYNAMIC_RANGE(0x000f00ff));
		R[3]=rand_intparts_init(0xee6dbbcc,256,0	,-1	,0xf	,0	,EE_LIMIT_DYNAMIC_RANGE(0x0000ffff));
	} else {
		R[0]=rand_intparts_init(seed++,256	  ,0	,-1	,0x7	,0	,EE_LIMIT_DYNAMIC_RANGE(0x00ffffff));
		R[1]=rand_intparts_init(seed++,256	  ,0	,-1	,0x3	,0	,EE_LIMIT_DYNAMIC_RANGE(0x00ffffff));
		R[2]=rand_intparts_init(seed++,256	  ,0	,-1	,0x7	,0	,EE_LIMIT_DYNAMIC_RANGE(0x000f00ff));
		R[3]=rand_intparts_init(seed,256	  ,0	,-1	,0xf	,0	,EE_LIMIT_DYNAMIC_RANGE(0x0000ffff));
	}
	for (i=0 ; i<N ;i++) v[i]=precise_random_f32(R[i&3]);
	for (i=0 ; i<4 ;i++) rand_fini(R[i]);
	return v;
}

e_f64 *fromint_random_f64_vector(int N,void *pr) {
	e_f64 *v=(e_f64 *)th_malloc(sizeof(e_f64)*N);
	int i=0;
	for (i=0 ; i<N ;i++) v[i]=precise_random_f64(pr);
	return v;
}

e_f32 *fromint_random_f32_vector(int N,void *pr) {
	e_f32 *v=(e_f32 *)th_malloc(sizeof(e_f32)*N);
	int i=0;
	for (i=0 ; i<N ;i++) v[i]=precise_random_f32(pr);
	return v;
}

/* Create a vector based on RNG that was previously allocated */
e_f64 *random_f64_vector(int N,void *pr) {
	e_f64 *v=(e_f64 *)th_malloc(sizeof(e_f64)*N);
	int i=0;
	for (i=0 ; i<N ;i++) v[i]=random_f64(pr);
	return v;
}
/* Create a vector with random values in 4 ranges */
e_f64 *mixed_f64_vector(int N, e_u32 seed) {
	e_f64 *v=(e_f64 *)th_malloc(sizeof(e_f64)*N);
	int i=0;
	void *R[4];
	if (seed==0) {
		R[0]=rand_init(0x9e3779b9,256,-1e-10,1e-10);
		R[1]=rand_init(0x73686179,256,1.0e20,1.0e40);
		R[2]=rand_init(0x656d6263,256,-1.0e6,1.0e6);
		R[3]=rand_init(0xee6dbbcc,256,-1.0e40,1.0e40);
	} else {
		R[0]=rand_init(seed,256,-1e-10,1e-10);
		R[1]=rand_init(seed,256,1.0e20,1.0e40);
		R[2]=rand_init(seed,256,-1.0e6,1.0e6);
		R[3]=rand_init(seed,256,-1.0e40,1.0e40);
	}
	for (i=0 ; i<N ;i++) v[i]=random_f64(R[i&3]);
	for (i=0 ; i<4 ;i++) rand_fini(R[i]);
	return v;
}

#define BITS_IN_RAND (32)
#define SIZEOF_U32_IN_BITS (32)
#define MAX_MANT (53)


e_s32 range_random(e_s32 min, e_s32 max, void *pr)
{
	return (random_u32(pr) % (max - min)) + min;
}
/* deprecated */
e_fp th_exp2(e_fp x) {
	return FPCONST(0.0);
}
/* the exp2 function always works on doubles */
e_f64 th_exp2_64(e_f64 x)
{
	return th_pow_64(FPCONST(2.0),x);
}

e_fp complex_rand_fp(
	SIGNRAND sign_rand,
	e_s32 min_exp,
	e_s32 max_exp,
	e_u32 meaningfull_bits,
	void *pr)
{
	e_u32 mant[(MAX_MANT/SIZEOF_U32_IN_BITS) + 1];
	e_u32 mant_array_size = 0;
	e_u32 i;
	e_u32 processed_bits = 0;
	e_s32 iter_exp;
	e_fp res = FPCONST(0.0);

	do
	{
		mant[mant_array_size++] = random_u32(pr);
		processed_bits += SIZEOF_U32_IN_BITS;
	} while (processed_bits < meaningfull_bits);

	mant[mant_array_size-1] |= ((e_u32)1 << (SIZEOF_U32_IN_BITS - 1));

	for (i=0, iter_exp= 1-processed_bits ; i < mant_array_size ; i++, iter_exp += SIZEOF_U32_IN_BITS)
	{
		e_fp fp_mant = (e_fp)mant[i] * th_exp2_64((e_fp)iter_exp);
		res += fp_mant;
	}

	switch (sign_rand)
	{
	case RAND:
		res *= (random_u32(pr) & 1) ? FPCONST(-1.0) : FPCONST(1.0);
		break;
	case POS:
		break;
	case NEG:
		res = -res;
		break;
	}

	return res * th_exp2_64((e_fp)range_random(min_exp,max_exp,pr));
}

e_f64 random_f64_01_uniform(void *pr) {
	e_f64 res;
	intparts num;
	num.sign=0;
	num.exp= -(e_s16)((((e_u32)1)<<12)/(1+(random_u32(pr) & ((((e_u32)1)<<12)-1))));
	num.mant_high32=random_u32(pr);
	num.mant_low32=random_u32(pr);
	num.mant_high32 &= (((e_u32)1)<<20) - 1;
	num.mant_high32 |= (((e_u32)1)<<20);
	store_dp(&res,&num);
	return res;
}



e_f64 *fromint_f64_01_vector_uniform(int N, e_u32 seed) {
	e_f64 *v=(e_f64 *)th_malloc(sizeof(e_f64)*N);
	int i=0;
	void *R;
	if (seed==0) {							// m+-  ,e+-,ecut	,high		,low
		R=rand_intparts_init(0x9e3779b9,256,0	,-1	,0x7	,0	,0x00ffffff);
	} else {
		R=rand_intparts_init(seed,256,0	,-1	,0x7	,0	,0x00ffffff);
	}
	for (i=0 ; i<N ;i++) v[i]=random_f64_01_uniform(R);
	rand_fini(R);
	return v;
}

e_f32 random_f32_01_uniform(void *pr) {
	e_f32 res;
	intparts num;
	num.sign=0;
	num.exp= -(e_s16)((((e_u32)1)<<9)/(1+(random_u32(pr) & ((((e_u32)1)<<9)-1))));
	num.mant_low32=random_u32(pr);
	num.mant_low32 &= EE_LIMIT_DYNAMIC_RANGE((((e_u32)1)<<23) - 1);
	num.mant_low32 |= (((e_u32)1)<<23);
	store_sp(&res,&num);
	return res;
}



e_f32 *fromint_f32_01_vector_uniform(int N, e_u32 seed) {
	e_f32 *v=(e_f32 *)th_malloc(sizeof(e_f32)*N);
	int i=0;
	void *R;
	if (seed==0) {							// m+-  ,e+-,ecut	,high		,low
		R=rand_intparts_init(0x9e3779b9,256,0	,-1	,0x7	,0	,EE_LIMIT_DYNAMIC_RANGE(0x00ffffff));
	} else {
		R=rand_intparts_init(seed,256,0	,-1	,0x7	,0	,EE_LIMIT_DYNAMIC_RANGE(0x00ffffff));
	}
	for (i=0 ; i<N ;i++) v[i]=random_f32_01_uniform(R);
	rand_fini(R);
	return v;
}
