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
Author : Shay Gal-On, EEMBC

*/ 
#include "coremark.h"
#if USE_CRC_TABLE
#define crcu8_bytable crcu8
#else
#define crcu8_byloop crcu8
#endif
/* Function: get_seed
	Get a values that cannot be determined at compile time.

	Since different embedded systems and compilers are used, 3 different methods are provided:
	1 - Using a volatile variable. This method is only valid if the compiler is forced to generate code that
	reads the value of a volatile variable from memory at run time. 
	Please note, if using this method, you would need to modify core_portme.c to generate training profile.
	2 - Command line arguments. This is the preferred method if command line arguments are supported.
	3 - System function. If none of the first 2 methods is available on the platform,
	a system function which is not a stub can be used.
	
	e.g. read the value on GPIO pins connected to switches, or invoke special simulator functions.
*/
extern volatile ee_s32 seed1_volatile;
extern volatile ee_s32 seed2_volatile;
extern volatile ee_s32 seed3_volatile;
extern volatile ee_s32 seed4_volatile;
extern volatile ee_s32 seed5_volatile;
ee_s32 get_seed_32(int i) {
	ee_s32 retval;
	switch (i) {
		case 1:
			retval=seed1_volatile;
			break;
		case 2:
			retval=seed2_volatile;
			break;
		case 3:
			retval=seed3_volatile;
			break;
		case 4:
			retval=seed4_volatile;
			break;
		case 5:
			retval=seed5_volatile;
			break;
		case 6:
			retval=0;
			break;
		case 7:
			retval=total_data_size;
			break;
		default:
			retval=0;
			break;
	}
	return retval;
}

static  ee_u16 crcu8_table[256] = {
        0x0,0xc0c1,0xc181,0x140,
        0xc301,0x3c0,0x280,0xc241,
        0xc601,0x6c0,0x780,0xc741,
        0x500,0xc5c1,0xc481,0x440,
        0xcc01,0xcc0,0xd80,0xcd41,
        0xf00,0xcfc1,0xce81,0xe40,
        0xa00,0xcac1,0xcb81,0xb40,
        0xc901,0x9c0,0x880,0xc841,
        0xd801,0x18c0,0x1980,0xd941,
        0x1b00,0xdbc1,0xda81,0x1a40,
        0x1e00,0xdec1,0xdf81,0x1f40,
        0xdd01,0x1dc0,0x1c80,0xdc41,
        0x1400,0xd4c1,0xd581,0x1540,
        0xd701,0x17c0,0x1680,0xd641,
        0xd201,0x12c0,0x1380,0xd341,
        0x1100,0xd1c1,0xd081,0x1040,
        0xf001,0x30c0,0x3180,0xf141,
        0x3300,0xf3c1,0xf281,0x3240,
        0x3600,0xf6c1,0xf781,0x3740,
        0xf501,0x35c0,0x3480,0xf441,
        0x3c00,0xfcc1,0xfd81,0x3d40,
        0xff01,0x3fc0,0x3e80,0xfe41,
        0xfa01,0x3ac0,0x3b80,0xfb41,
        0x3900,0xf9c1,0xf881,0x3840,
        0x2800,0xe8c1,0xe981,0x2940,
        0xeb01,0x2bc0,0x2a80,0xea41,
        0xee01,0x2ec0,0x2f80,0xef41,
        0x2d00,0xedc1,0xec81,0x2c40,
        0xe401,0x24c0,0x2580,0xe541,
        0x2700,0xe7c1,0xe681,0x2640,
        0x2200,0xe2c1,0xe381,0x2340,
        0xe101,0x21c0,0x2080,0xe041,
        0xa001,0x60c0,0x6180,0xa141,
        0x6300,0xa3c1,0xa281,0x6240,
        0x6600,0xa6c1,0xa781,0x6740,
        0xa501,0x65c0,0x6480,0xa441,
        0x6c00,0xacc1,0xad81,0x6d40,
        0xaf01,0x6fc0,0x6e80,0xae41,
        0xaa01,0x6ac0,0x6b80,0xab41,
        0x6900,0xa9c1,0xa881,0x6840,
        0x7800,0xb8c1,0xb981,0x7940,
        0xbb01,0x7bc0,0x7a80,0xba41,
        0xbe01,0x7ec0,0x7f80,0xbf41,
        0x7d00,0xbdc1,0xbc81,0x7c40,
        0xb401,0x74c0,0x7580,0xb541,
        0x7700,0xb7c1,0xb681,0x7640,
        0x7200,0xb2c1,0xb381,0x7340,
        0xb101,0x71c0,0x7080,0xb041,
        0x5000,0x90c1,0x9181,0x5140,
        0x9301,0x53c0,0x5280,0x9241,
        0x9601,0x56c0,0x5780,0x9741,
        0x5500,0x95c1,0x9481,0x5440,
        0x9c01,0x5cc0,0x5d80,0x9d41,
        0x5f00,0x9fc1,0x9e81,0x5e40,
        0x5a00,0x9ac1,0x9b81,0x5b40,
        0x9901,0x59c0,0x5880,0x9841,
        0x8801,0x48c0,0x4980,0x8941,
        0x4b00,0x8bc1,0x8a81,0x4a40,
        0x4e00,0x8ec1,0x8f81,0x4f40,
        0x8d01,0x4dc0,0x4c80,0x8c41,
        0x4400,0x84c1,0x8581,0x4540,
        0x8701,0x47c0,0x4680,0x8641,
        0x8201,0x42c0,0x4380,0x8341,
        0x4100,0x81c1,0x8081,0x4040
};


ee_u16 crcu8_bytable(ee_u8 data, ee_u16 crc )
{
        return (crc >> 8) ^ crcu8_table[(crc ^ data) & 0x00ff];
}
/* Function: crc*
	Service functions to calculate 16b CRC code.

*/
ee_u16 crcu8_byloop(ee_u8 data, ee_u16 crc )
{
	ee_u8 i=0,x16=0,carry=0;

	for (i = 0; i < 8; i++)
    {
		x16 = (ee_u8)((data & 1) ^ ((ee_u8)crc & 1));
		data >>= 1;

		if (x16 == 1)
		{
		   crc ^= 0x4002;
		   carry = 1;
		}
		else 
			carry = 0;
		crc >>= 1;
		if (carry)
		   crc |= 0x8000;
		else
		   crc &= 0x7fff;
    }
	return crc;
} 
ee_u16 crcu16(ee_u16 newval, ee_u16 crc) {
	crc=crcu8( (ee_u8) (newval)				,crc);
	crc=crcu8( (ee_u8) ((newval)>>8)	,crc);
	return crc;
}
ee_u16 crcu32(ee_u32 newval, ee_u16 crc) {
	crc=crc16((ee_s16) newval		,crc);
	crc=crc16((ee_s16) (newval>>16)	,crc);
	return crc;
}
ee_u16 crc16(ee_s16 newval, ee_u16 crc) {
	return crcu16((ee_u16)newval, crc);
}

ee_u8 check_data_types() {
	ee_u8 retval=0;
	if (sizeof(ee_u8) != 1) {
		ee_printf("ERROR: ee_u8 is not an 8b datatype!\n");
		retval++;
	}
	if (sizeof(ee_u16) != 2) {
		ee_printf("ERROR: ee_u16 is not a 16b datatype!\n");
		retval++;
	}
	if (sizeof(ee_s16) != 2) {
		ee_printf("ERROR: ee_s16 is not a 16b datatype!\n");
		retval++;
	}
	if (sizeof(ee_s32) != 4) {
		ee_printf("ERROR: ee_s32 is not a 32b datatype!\n");
		retval++;
	}
	if (sizeof(ee_u32) != 4) {
		ee_printf("ERROR: ee_u32 is not a 32b datatype!\n");
		retval++;
	}
	if (sizeof(ee_ptr_int) != sizeof(int *)) {
		ee_printf("ERROR: ee_ptr_int is not a datatype that holds an int pointer!\n");
		retval++;
	}
	if (retval>0) {
		ee_printf("ERROR: Please modify the datatypes in core_portme.h!\n");
	}
	return retval;
}
