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

/*==============================================================================
 * File: mith/include/th_version.h
 *
 *   DESC : Test Harness Version Information
 *
 *------------------------------------------------------------------------------
 * Other Copyright Notice (if any): 
 * 
 * For conditions of distribution and use, see the accompanying README file.
 * ===========================================================================*/
/**
@file
@brief Test Harness Version Information
*/
/** @addtogroup THFL TH Functional Layer
@{ */
/** @addtogroup thvinfo_h Test Harness Version Information
@{ */

/** File Sentinel */
#ifndef THVINFO_H
#define THVINFO_H

/** Version Text displayed to the user.

This is a human readable form of the version number. It is manually kept
in sync with major, minor, step, and revision numbers.
The revision minor, step, and revision are related to CVS tags.
*/

/** Major Release of Test Harness. */
#define EEMBC_TH_MAJOR         5
/** Minor Release of Test Harness. */
#define EEMBC_TH_MINOR         0
/** Step Version in Release. Describes Beta and Release. */
#define EEMBC_TH_STEP          'A'
/** Sub version within Step. */
#define EEMBC_TH_REVISION      0

#define MAKE_TH_VER(_name,_M,_m,_s,_r) _name"(##_M.##_m##_s##_r)"

#define EEMBC_TH_ID MAKE_TH_VER("EEMBC MITH ALPHA",EEMBC_TH_MAJOR,EEMBC_TH_MINOR ,EEMBC_TH_STEP,EEMBC_TH_REVISION)

#endif /* THVINFO_H */
/** @} */
/** @} */
