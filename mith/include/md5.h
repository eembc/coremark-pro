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

#ifndef MD5_H
#define MD5_H

#include "th_types.h"

struct MD5Context {
        e_u32 buf[4];
        e_u32 bits[2];
		union {
			unsigned char as_u8[64];
			e_u32 as_u32[16];
		} in ;
};

extern void MD5Init(struct MD5Context *ctx);
extern void MD5Update(struct MD5Context *ctx, unsigned char *buf, e_u32 len);
extern void MD5Final(unsigned char digest[16], struct MD5Context *ctx);
extern void MD5Transform(e_u32 buf[4], e_u32 in[16]);
extern e_u16 MD5CRC(unsigned char *buf, e_u32 len);
extern void DigestToBase16(unsigned char *digest, char *zBuf);

/*
 * This is needed to make RSAREF happy on some MS-DOS compilers.
 */
typedef struct MD5Context MD5_CTX;

#endif /* !MD5_H */
