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
#include "th_al.h"
#define HIBITMASK 0x80000000UL
#define BITS_PER_DIGIT 32
#define BNC 8
typedef struct BIGNUM_S
{
	e_u32 n[BNC];
} BIGNUM;
void bignum_copy(BIGNUM *res, BIGNUM *val);
void bignum_zero(BIGNUM *res);
void bignum_add(BIGNUM *res, BIGNUM *a, BIGNUM *b);
e_u32 bignum_shl(BIGNUM *res, BIGNUM *a, e_u32 c);
void bignum_neg(BIGNUM *res, BIGNUM *a);
e_s32 bignum_cmp(BIGNUM *a, BIGNUM *b);
void bignum_sub(BIGNUM *res, BIGNUM *a, BIGNUM *b);
void bignum_abs(BIGNUM *res, BIGNUM *a);
void bignum_print(BIGNUM *a);
e_s32 bignum_diff(BIGNUM *diff, BIGNUM *a, BIGNUM *b);
e_u32 bignum_diff_dp(BIGNUM *diff, intparts *sig, intparts *ref);
e_u32 bignum_diff_sp(BIGNUM *diff, intparts *sig, intparts *ref);

e_u32 count_setbits(e_u32 v) {
	e_u32 c; // store the total here
	static const e_u32 B[] = {0x55555555, 0x33333333, 0x0F0F0F0F, 0x00FF00FF, 0x0000FFFF}; // Bit patterns

	c = v - ((v >> 1) & B[0]);
	c = ((c >> 2) & B[1]) + (c & B[1]);
	c = ((c >> 4) + c) & B[2];
	c = ((c >> 8) + c) & B[3];
	c = ((c >> 16) + c) & B[4];
	return c;
}
e_s32 count_msb(e_u32 v) {
	e_s32 c;
	for (c = 31; v; c--)
	{
		if (v&(1<<c)) return c;
	}
	return -1;
}
e_u32 bignum_setbits(BIGNUM *res)
{
	e_u32 i,ret=0;
	for (i=0;i<BNC;i++) ret+=count_setbits(res->n[i]);
	return ret;
}

e_s32 bignum_msb(BIGNUM *res)
{
	e_s32 i,ret=-1,tmp;
	for (i=BNC-1;i>=0;i--) {
		tmp=count_msb(res->n[i]);
		if (tmp>=0) {
			ret=tmp+32*i;
			break;
		}
	}
	return ret;
}

void bignum_zero(BIGNUM *res)
{
	e_u32 i;
	for (i=0;i<BNC;i++) res->n[i] = 0;
}
void bignum_copy(BIGNUM *res, BIGNUM *val)
{
	e_u32 i;
	for (i=0;i<BNC;i++) res->n[i] = val->n[i];
}

void bignum_add(BIGNUM *res, BIGNUM *a, BIGNUM *b)
{
	e_u32 i;
	e_u32 c = 0;

	if (!res || !a || !b)
		return;

	for (i=0;i<BNC;i++)
	{
		e_u32 n = a->n[i] + b->n[i] + c;
		c = (n < a->n[i] || n < b->n[i]) ? 1 : 0;
		res->n[i] = n;
	}
}

e_u32 bignum_shl(BIGNUM *res, BIGNUM *a, e_u32 c)
{
	e_u32 skip = c / (sizeof(e_u32) * CHAR_BIT);
	e_u32 rem = c % (sizeof(e_u32) * CHAR_BIT);
	e_u32 i,j;
	e_u32 prev = 0;

	for (i=skip,j=0;i<BNC;i++,j++)
	{
		e_u32 current = a->n[j];
		res->n[i] = (current << rem) | (prev >> ((sizeof(e_u32) * CHAR_BIT)-rem));
		prev = current;
	}

	for (i=0;i<skip;i++) 
	{
		res->n[i] = 0;
	}
	if (skip>=BNC)
		return 1;
	else 
		return (prev >> ((sizeof(e_u32) * CHAR_BIT)-rem));
}


e_s32 bignum_shl_with_carry(BIGNUM *a, BIGNUM *b,
	e_s32 shift)
{	/* Computes a = b << shift */
	/* [v2.1] Modified to cope with shift > BITS_PERDIGIT */
	e_s32 i, y, nw, bits;
	e_s32 mask, carry, nextcarry;

	/* Do we shift whole digits? */
	if (shift >= BITS_PER_DIGIT)
	{
		nw = shift / BITS_PER_DIGIT;
		i = BNC;
		while (i--)
		{
			if (i >= nw)
				a->n[i] = b->n[i-nw];
			else
				a->n[i] = 0;
		}
		/* Call again to shift bits inside digits */
		bits = shift % BITS_PER_DIGIT;
		carry = b->n[BNC-nw] << bits;
		if (bits) 
			carry |= bignum_shl_with_carry(a, a, bits);
		return carry;
	}
	else
	{
		bits = shift;
	}

	/* Construct mask = high bits set */
	mask = ~(~(e_s32)0 >> bits);
	
	y = BITS_PER_DIGIT - bits;
	carry = 0;
	for (i = 0; i < BNC; i++)
	{
		nextcarry = (b->n[i] & mask) >> y;
		a->n[i] = b->n[i] << bits | carry;
		carry = nextcarry;
	}

	return carry;
}

e_s32 bignum_shr_with_carry(BIGNUM *a, BIGNUM *b, e_s32 shift)
{	/* Computes a = b >> shift */
	/* [v2.1] Modified to cope with shift > BITS_PERDIGIT */
	e_s32 i, y, nw, bits;
	e_s32 mask, carry, nextcarry;

	/* Do we shift whole digits? */
	if (shift >= BITS_PER_DIGIT)
	{
		nw = shift / BITS_PER_DIGIT;
		for (i = 0; i < BNC; i++)
		{
			if ((i+nw) < BNC)
				a->n[i] = b->n[i+nw];
			else
				a->n[i] = 0;
		}
		/* Call again to shift bits inside digits */
		bits = shift % BITS_PER_DIGIT;
		carry = b->n[nw-1] >> bits;
		if (bits) 
			carry |= bignum_shr_with_carry(a, a, bits);
		return carry;
	}
	else
	{
		bits = shift;
	}

	/* Construct mask to set low bits */
	/* (thanks to Jesse Chisholm for suggesting this improved technique) */
	mask = ~(~(e_s32)0 << bits);
	
	y = BITS_PER_DIGIT - bits;
	carry = 0;
	i = BNC;
	while (i--)
	{
		nextcarry = (b->n[i] & mask) << y;
		a->n[i] = b->n[i] >> bits | carry;
		carry = nextcarry;
	}

	return carry;
}

void bignum_neg(BIGNUM *res, BIGNUM *a)
{
	BIGNUM t;
	e_u32 i;
	bignum_zero(&t);
	t.n[0] = 1;
	for (i=0;i<BNC;i++)
	{
		res->n[i] = ~a->n[i];
	}
	bignum_add(res,res,&t);
}

void bignum_sub(BIGNUM *res, BIGNUM *a, BIGNUM *b)
{
	BIGNUM t;
	bignum_neg(&t,b);
	bignum_add(res,a,&t);
}

void bignum_abs(BIGNUM *res, BIGNUM *a)
{
	if (a->n[BNC-1] & HIBITMASK)
	{
		bignum_neg(res,a);
	}
	else 
	{
		*res = *a;
	}
}

void bignum_print(BIGNUM *a)
{
	e_u32 i;
	for (i=BNC;i;i--)
	{
		th_printf("%x,",a->n[i-1]);
	}
}

BIGNUM * bignum_convert(BIGNUM *res,
		  e_u32 s_1, e_s32 e_1, e_u32 m1_1, e_u32 m0_1, e_s32 scale ) 
{
	bignum_zero(res);
	res->n[0] = m0_1;
	res->n[1] = m1_1;
	if (s_1) bignum_neg(res,res);
	if (scale>0)
		bignum_shl(res,res, scale); 
	return res;
}

e_s32 bignum_diff(BIGNUM *diff, BIGNUM *a, BIGNUM *b)
{
	bignum_sub(diff,a,b);
	bignum_abs(diff,diff);
	return bignum_msb(diff);
}

e_u32 bignum_diff_dp(BIGNUM *diff, intparts *sig, intparts *ref)
{
	BIGNUM _1,_2;
	e_s32 msb,ref_msb,sigscale=0,refscale=0,retval=52;
	if ((ref->mant_low32|ref->mant_high32)==0) { //special handling for zero, accept epsilon of 1e-200
		if (sig->exp<=-200 || (sig->exp==0 && (sig->mant_low32|sig->mant_high32)==0))
			return retval;
		else
			return 0;
	}
	if ((sig->mant_low32|sig->mant_high32)==0) { //special handling for zero, accept epsilon of 1e-200
		if (ref->exp<=-200 || (ref->exp==0 && (ref->mant_low32|ref->mant_high32)==0))
			return retval;
		else
			return 0;
	}
	if (th_abs(sig->exp - ref->exp) > 100) //special handling for cases where the difference in exponents is such that we can ignore the mantissa
		return 0;
	if (ref->exp < sig->exp)
		sigscale=th_abs(ref->exp-sig->exp);
	if (ref->exp > sig->exp)
		refscale=th_abs(sig->exp-ref->exp);
	bignum_convert(&_1,sig->sign,  sig->exp,  sig->mant_high32,  sig->mant_low32,sigscale);
	bignum_convert(&_2,ref->sign,  ref->exp,  ref->mant_high32,  ref->mant_low32,refscale);
	msb=bignum_diff(diff,&_1,&_2);
	if (msb<0) return retval; // no error
	if (ref->sign) 			//diff returns absolute value, so msb accordingly.
		bignum_neg(&_2,&_2);//need to abs the ref bignum to get correct diff for negative numbers
	ref_msb=bignum_msb(&_2);
	if (ref_msb<msb) 
		return 0;
	else {
		if (ref_msb-msb < retval) retval=ref_msb-msb;
		return retval;
	}
}

e_u32 bignum_diff_sp(BIGNUM *diff, intparts *sig, intparts *ref)
{
	BIGNUM _1,_2;
	e_s32 msb,ref_msb,sigscale=0,refscale=0,retval=23;
	if ((ref->mant_low32)==0) { //special handling for zero, accept epsilon of 1e-200
		if (sig->exp<=-100 || (sig->exp==0 && sig->mant_low32==0))
			return retval;
		else
			return 0;
	}
	if ((sig->mant_low32)==0) { //special handling for zero, accept epsilon of 1e-200
		if ((ref->exp<=-100) || (ref->exp==0 && ref->mant_low32==0))
			return retval;
		else
			return 0;
	}
	if (th_abs(sig->exp - ref->exp) > 40) //special handling for cases where the difference in exponents is such that we can ignore the mantissa
		return 0;
	if (ref->exp < sig->exp)
		sigscale=th_abs(ref->exp-sig->exp);
	if (ref->exp > sig->exp)
		refscale=th_abs(sig->exp-ref->exp);
	bignum_convert(&_1,sig->sign,  sig->exp,  0,  sig->mant_low32,sigscale);
	bignum_convert(&_2,ref->sign,  ref->exp,  0,  ref->mant_low32,refscale);
	msb=bignum_diff(diff,&_1,&_2);
	if (msb<0) return retval; // no error
	if (ref->sign) 			//diff returns absolute value, so msb accordingly.
		bignum_neg(&_2,&_2);//need to abs the ref bignum to get correct diff for negative numbers
	ref_msb=bignum_msb(&_2);
	if (ref_msb<msb) 
		return 0;
	else {
		if (ref_msb-msb < retval) retval=ref_msb-msb;
		return retval;
	}
}

e_s32 bignum_cmp(BIGNUM *a, BIGNUM *b)
{
	BIGNUM diff;
	bignum_sub(&diff,a,b);
	if (diff.n[BNC-1] & HIBITMASK)
		return -1;
	if (bignum_setbits(&diff)==0)
		return 0;
	return 1;
}
acc_bits_d acc_summary = {{0,9999,-9999,0,0},{0,9999,-9999,0,0},{0,9999,-9999,0,0},{0,}};

void svals(acc_bits_i *p, e_s32 val) {
	if (p->min > val) p->min=val;
	if (p->max < val) p->max=val;
	p->n++;
	p->sum+=val;
	p->avg=p->sum/p->n;
}
	
void acc_summary_info(intparts *sig, intparts *ref, e_u32 accbits) {
	svals(&acc_summary.sig_exp,sig->exp);
	svals(&acc_summary.ref_exp,ref->exp);
	svals(&acc_summary.bits,accbits);
	if (accbits<MIN_ACC_BITS_FP)
		acc_summary.counts[accbits]++;
	else
		acc_summary.counts[MAX_ACC_COUNTS]++;
}

e_u32 fp_accurate_bits_dp(e_f64 sig, e_f64 ref) {
	intparts sigbits,refbits;
	BIGNUM diff;
	e_u32 ret;
	load_dp(&sig,&sigbits);
	load_dp(&ref,&refbits);
	ret=bignum_diff_dp(&diff,&sigbits,&refbits);
	acc_summary_info(&sigbits,&refbits,ret);
#if DEBUG_ACCURATE_BITS
	th_printf("ACCBITS,%d,%1.18le,%1.18le,%d,%08x:%08x,%d,%08x:%08x\n",ret,sig,ref,refbits.exp,refbits.mant_high32,refbits.mant_low32,sigbits.exp,sigbits.mant_high32,sigbits.mant_low32);
#endif
	return ret;
}
e_u32 fp_accurate_bits_sp(e_f32 sig, e_f32 ref) {
	intparts sigbits,refbits;
	BIGNUM diff;
	e_u32 ret;
	load_sp(&sig,&sigbits);
	load_sp(&ref,&refbits);
	ret=bignum_diff_sp(&diff,&sigbits,&refbits);
	acc_summary_info(&sigbits,&refbits,ret);
#if DEBUG_ACCURATE_BITS
	th_printf("ACCBITS,%d,%1.18e,%1.18e\n",ret,sig,ref);
#endif
	return ret;
}
e_u32 fp_iaccurate_bits_dp(e_f64 sig, intparts *refbits) {
	intparts sigbits;
	BIGNUM diff;
	e_u32 ret;
#if DEBUG_ACCURATE_BITS
	e_f64 refval;
	store_dp(&refval,refbits);
#endif
	load_dp(&sig,&sigbits);
	ret=bignum_diff_dp(&diff,&sigbits,refbits);
	acc_summary_info(&sigbits,refbits,ret);
#if DEBUG_ACCURATE_BITS
	th_printf("ACCBITS,%d,%1.18le,%1.18le\n",ret,sig,refval);
#endif
	return ret;
}
e_u32 fp_iaccurate_bits_sp(e_f32 sig, intparts *refbits) {
	intparts sigbits;
	BIGNUM diff;
	e_u32 ret;
#if DEBUG_ACCURATE_BITS
	e_f32 refval;
	store_sp(&refval,refbits);
#endif
	load_sp(&sig,&sigbits);
	ret=bignum_diff_sp(&diff,&sigbits,refbits);
	acc_summary_info(&sigbits,refbits,ret);
#if DEBUG_ACCURATE_BITS
	th_printf("ACCBITS,%d,%1.18e,%1.18e\n",ret,sig,refval);
#endif
	return ret;
}
static e_s32 ilog2(e_u32 x)
{
       e_s32 i;

       for (i=31;i>= 0;i--)
       {
              if (x & (1 << i))
                     return i;
       }

       /* 0 */
       return -1;
}


static void ishld(e_u32 *l, e_u32 *h, e_u32 n)
{
       if (n <= 32)
       {
              *h = (*h << n) | (*l >> (32-n));
              *l <<= n;
       }
       else
       {
              *h = (*l << (n - 32));
              *l  = 0;
       }
}


int canonnical_form_dp(intparts *src)
{
       if (src->mant_high32)
       {
              e_s32 l2 = ilog2(src->mant_high32);

              if (l2 > 20) return 0;

              ishld(&src->mant_low32,&src->mant_high32,20 - l2);
              return 1;
       }
       else if (src->mant_low32)
       {
              e_s32 l2 = ilog2(src->mant_low32);
              ishld(&src->mant_low32,&src->mant_high32,52 - l2);
              return 1;
       }
       else
       {
       /* zero */
       src->exp = 0;
       }
	   return 0;
}
