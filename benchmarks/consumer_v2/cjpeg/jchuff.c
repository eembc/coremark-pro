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
 *$RCSfile: jchuff.c,v $
 *
 *   DESC : Jpeg Compression
 *
 *  EEMBC : Consumer Subcommittee
 *
 * AUTHOR : 
 *          Modified for Multi Instance Test Harness by Ron Olson, 
 *          IBM Corporation
 *
 *    CVS : $Revision: 1.1 $
 *          $Date: 2004/05/10 22:36:03 $
 *          $Author: rick $
 *          $Source: D:/cvs/eembc2/consumer/cjpegv2/jchuff.c,v $
 *          
 * NOTE   :
 *
 *------------------------------------------------------------------------------
 *
 * HISTORY :
 *
 * $Log: jchuff.c,v $
 * Revision 1.1  2004/05/10 22:36:03  rick
 * Rename Consumer V1.1 benchmarks used in V2
 *
 * Revision 1.3  2004/01/22 20:16:54  rick
 * Copyright update and cleanup
 *
 * Revision 1.2  2002/04/22 22:54:52  rick
 * Standard Comment blocks
 *
 *
 *------------------------------------------------------------------------------
 * Other Copyright Notice (if any): 
 * 
 * For conditions of distribution and use, see the accompanying README file.
 * ===========================================================================*/
/*
 * jchuff.c
 *
 * Copyright (C) 1991-1997, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains Huffman entropy encoding routines.
 *
 * Much of the complexity here has to do with supporting output suspension.
 * If the data destination module demands suspension, we want to be able to
 * back up to the start of the current MCU.  To do this, we copy state
 * variables into local working storage, and update them back to the
 * permanent JPEG objects only upon successful completion of an MCU.
 */

#define JPEG_INTERNALS
#include "jinclude.h"

#include <th_file.h>

#include "jpeglib.h"
#include "jchuff.h"        /* Declarations shared with jcphuff.c */


/* Expanded entropy encoder object for Huffman encoding.
 *
 * The savable_state subrecord contains fields that change within an MCU,
 * but must not be updated permanently until we complete the MCU.
 */

typedef struct {
  e_s32 put_buffer;        /* current bit-accumulation buffer */
  int put_bits;            /* # of bits now in it */
  int last_dc_val[MAX_COMPS_IN_SCAN]; /* last DC coef for each component */
} savable_state;

/* This macro is to work around compilers with missing or broken
 * structure assignment.  You'll need to fix this code if you have
 * such a compiler and you change MAX_COMPS_IN_SCAN.
 */

#ifndef NO_STRUCT_ASSIGN
#define ASSIGN_STATE(dest,src)  ((dest) = (src))
#else
#if MAX_COMPS_IN_SCAN == 4
#define ASSIGN_STATE(dest,src)  \
    ((dest).put_buffer = (src).put_buffer, \
     (dest).put_bits = (src).put_bits, \
     (dest).last_dc_val[0] = (src).last_dc_val[0], \
     (dest).last_dc_val[1] = (src).last_dc_val[1], \
     (dest).last_dc_val[2] = (src).last_dc_val[2], \
     (dest).last_dc_val[3] = (src).last_dc_val[3])
#endif
#endif


typedef struct {
  struct jpeg_entropy_encoder pub; /* public fields */

  savable_state saved;        /* Bit buffer & DC state at start of MCU */

  /* These fields are NOT loaded into local working state. */
  unsigned int restarts_to_go;    /* MCUs left in this restart interval */
  int next_restart_num;        /* next restart number to write (0-7) */

  /* Pointers to derived tables (these workspaces have image lifespan) */
  c_derived_tbl * dc_derived_tbls[NUM_HUFF_TBLS];
  c_derived_tbl * ac_derived_tbls[NUM_HUFF_TBLS];

} huff_entropy_encoder;

typedef huff_entropy_encoder * huff_entropy_ptr;

/* Working state while writing an MCU.
 * This struct contains all the fields that are needed by subroutines.
 */

typedef struct {
  JOCTET * next_output_byte;    /* => next byte to write in buffer */
  size_t free_in_buffer;    /* # of byte spaces remaining in buffer */
  savable_state cur;        /* Current bit buffer & DC state */
  j_compress_ptr cinfo;        /* dump_buffer needs access to this */
} working_state;


/* Forward declarations */
METHODDEF(boolean) encode_mcu_huff JPP((j_compress_ptr cinfo,
                    JBLOCKROW *MCU_data));
METHODDEF(void) finish_pass_huff JPP((j_compress_ptr cinfo));


/*
 * Initialize for a Huffman-compressed scan.
 * If gather_statistics is TRUE, we do not output anything during the scan,
 * just count the Huffman symbols used and generate Huffman code tables.
 */

METHODDEF(void)
start_pass_huff (j_compress_ptr cinfo, boolean gather_statistics)
{
  huff_entropy_ptr entropy = (huff_entropy_ptr) cinfo->entropy;
  int ci, dctbl, actbl;
  jpeg_component_info * compptr;

  if (gather_statistics) {
    ERREXIT(cinfo, JERR_NOT_COMPILED);
  } else {
    entropy->pub.encode_mcu = encode_mcu_huff;
    entropy->pub.finish_pass = finish_pass_huff;
  }

  for (ci = 0; ci < cinfo->comps_in_scan; ci++) {
    compptr = cinfo->cur_comp_info[ci];
    dctbl = compptr->dc_tbl_no;
    actbl = compptr->ac_tbl_no;
    if (gather_statistics) {
    /* no-op (not using ENTROPY option */
    } else {
      /* Compute derived values for Huffman tables */
      /* We may do this more than once for a table, but it's not expensive */
      jpeg_make_c_derived_tbl(cinfo, TRUE, dctbl,
                  & entropy->dc_derived_tbls[dctbl]);
      jpeg_make_c_derived_tbl(cinfo, FALSE, actbl,
                  & entropy->ac_derived_tbls[actbl]);
    }
    /* Initialize DC predictions to 0 */
    entropy->saved.last_dc_val[ci] = 0;
  }

  /* Initialize bit buffer to empty */
  entropy->saved.put_buffer = 0;
  entropy->saved.put_bits = 0;

  /* Initialize restart stuff */
  entropy->restarts_to_go = cinfo->restart_interval;
  entropy->next_restart_num = 0;
}


/*
 * Compute the derived values for a Huffman table.
 * This routine also performs some validation checks on the table.
 *
 * Note this is also used by jcphuff.c.
 */

GLOBAL(void)
jpeg_make_c_derived_tbl (j_compress_ptr cinfo, boolean isDC, int tblno,
             c_derived_tbl ** pdtbl)
{
  JHUFF_TBL *htbl;
  c_derived_tbl *dtbl;
  int p, i, l, lastp, si, maxsymbol;
  char huffsize[257];
  unsigned int huffcode[257];
  unsigned int code;

  /* Note that huffsize[] and huffcode[] are filled in code-length order,
   * paralleling the order of the symbols themselves in htbl->huffval[].
   */

  /* Find the input Huffman table */
  if (tblno < 0 || tblno >= NUM_HUFF_TBLS)
    ERREXIT1(cinfo, JERR_NO_HUFF_TABLE, tblno);
  htbl =
    isDC ? cinfo->dc_huff_tbl_ptrs[tblno] : cinfo->ac_huff_tbl_ptrs[tblno];
  if (htbl == NULL)
    ERREXIT1(cinfo, JERR_NO_HUFF_TABLE, tblno);

  /* Allocate a workspace if we haven't already done so. */
  if (*pdtbl == NULL)
    *pdtbl = (c_derived_tbl *)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
                  SIZEOF(c_derived_tbl));
  dtbl = *pdtbl;
  
  /* Figure C.1: make table of Huffman code length for each symbol */

  p = 0;
  for (l = 1; l <= 16; l++) {
    i = (int) htbl->bits[l];
    if (i < 0 || p + i > 256)    /* protect against table overrun */
      ERREXIT(cinfo, JERR_BAD_HUFF_TABLE);
    while (i--)
      huffsize[p++] = (char) l;
  }
  huffsize[p] = 0;
  lastp = p;
  
  /* Figure C.2: generate the codes themselves */
  /* We also validate that the counts represent a legal Huffman code tree. */

  code = 0;
  si = huffsize[0];
  p = 0;
  while (huffsize[p]) {
    while (((int) huffsize[p]) == si) {
      huffcode[p++] = code;
      code++;
    }
    /* code is now 1 more than the last code used for codelength si; but
     * it must still fit in si bits, since no code is allowed to be all ones.
     */
    if (((e_s32) code) >= (((e_s32) 1) << si))
      ERREXIT(cinfo, JERR_BAD_HUFF_TABLE);
    code <<= 1;
    si++;
  }
  
  /* Figure C.3: generate encoding tables */
  /* These are code and size indexed by symbol value */

  /* Set all codeless symbols to have code length 0;
   * this lets us detect duplicate VAL entries here, and later
   * allows emit_bits to detect any attempt to emit such symbols.
   */
  MEMZERO(dtbl->ehufsi, SIZEOF(dtbl->ehufsi));

  /* This is also a convenient place to check for out-of-range
   * and duplicated VAL entries.  We allow 0..255 for AC symbols
   * but only 0..15 for DC.  (We could constrain them further
   * based on data depth and mode, but this seems enough.)
   */
  maxsymbol = isDC ? 15 : 255;

  for (p = 0; p < lastp; p++) {
    i = htbl->huffval[p];
    if (i < 0 || i > maxsymbol || dtbl->ehufsi[i])
      ERREXIT(cinfo, JERR_BAD_HUFF_TABLE);
    dtbl->ehufco[i] = huffcode[p];
    dtbl->ehufsi[i] = huffsize[p];
  }
}


/* Outputting bytes to the file */

/* Emit a byte, taking 'action' if must suspend. */
#define emit_byte(state,val,action)  \
    { *(state)->next_output_byte++ = (JOCTET) (val);  \
      if (--(state)->free_in_buffer == 0)  \
        if (! dump_buffer(state))  \
          { action; } }


LOCAL(boolean)
dump_buffer (working_state * state)
/* Empty the output buffer; return TRUE if successful, FALSE if must suspend */
{
  struct jpeg_destination_mgr * dest = state->cinfo->dest;

  if (! (*dest->empty_output_buffer) (state->cinfo))
    return FALSE;
  /* After a successful buffer dump, must reset buffer pointers */
  state->next_output_byte = dest->next_output_byte;
  state->free_in_buffer = dest->free_in_buffer;
  return TRUE;
}


/* Outputting bits to the file */

/* Only the right 24 bits of put_buffer are used; the valid bits are
 * left-justified in this part.  At most 16 bits can be passed to emit_bits
 * in one call, and we never retain more than 7 bits in put_buffer
 * between calls, so 24 bits are sufficient.
 */

INLINE
LOCAL(boolean)
emit_bits (working_state * state, unsigned int code, int size)
/* Emit some bits; return TRUE if successful, FALSE if must suspend */
{
  /* This routine is heavily used, so it's worth coding tightly. */
  register e_s32 put_buffer = (e_s32) code;
  register int put_bits = state->cur.put_bits;

  /* if size is 0, caller used an invalid Huffman table entry */
  if (size == 0)
    ERREXIT(state->cinfo, JERR_HUFF_MISSING_CODE);

  put_buffer &= (((e_s32) 1)<<size) - 1; /* mask off any extra bits in code */
  
  put_bits += size;        /* new number of bits in buffer */
  
  put_buffer <<= 24 - put_bits; /* align incoming bits */

  put_buffer |= state->cur.put_buffer; /* and merge with old buffer contents */
  
  while (put_bits >= 8) {
    int c = (int) ((put_buffer >> 16) & 0xFF);
    
    emit_byte(state, c, return FALSE);
    if (c == 0xFF) {        /* need to stuff a zero byte? */
      emit_byte(state, 0, return FALSE);
    }
    put_buffer <<= 8;
    put_bits -= 8;
  }

  state->cur.put_buffer = put_buffer; /* update state variables */
  state->cur.put_bits = put_bits;

  return TRUE;
}


LOCAL(boolean)
flush_bits (working_state * state)
{
  if (! emit_bits(state, 0x7F, 7)) /* fill any partial byte with ones */
    return FALSE;
  state->cur.put_buffer = 0;    /* and reset bit-buffer to empty */
  state->cur.put_bits = 0;
  return TRUE;
}


/* Encode a single block's worth of coefficients */

LOCAL(boolean)
encode_one_block (working_state * state, JCOEFPTR block, int last_dc_val,
          c_derived_tbl *dctbl, c_derived_tbl *actbl)
{
  register int temp, temp2;
  register int nbits;
  register int k, r, i;
  
  /* Encode the DC coefficient difference per section F.1.2.1 */
  
  temp = temp2 = block[0] - last_dc_val;

  if (temp < 0) {
    temp = -temp;        /* temp is abs value of input */
    /* For a negative input, want temp2 = bitwise complement of abs(input) */
    /* This code assumes we are on a two's complement machine */
    temp2--;
  }
  
  /* Find the number of bits needed for the magnitude of the coefficient */
  nbits = 0;
  while (temp) {
    nbits++;
    temp >>= 1;
  }
  /* Check for out-of-range coefficient values.
   * Since we're encoding a difference, the range limit is twice as much.
   */
  if (nbits > MAX_COEF_BITS+1)
    ERREXIT(state->cinfo, JERR_BAD_DCT_COEF);
  
  /* Emit the Huffman-coded symbol for the number of bits */
  if (! emit_bits(state, dctbl->ehufco[nbits], dctbl->ehufsi[nbits]))
    return FALSE;

  /* Emit that number of bits of the value, if positive, */
  /* or the complement of its magnitude, if negative. */
  if (nbits)            /* emit_bits rejects calls with size 0 */
    if (! emit_bits(state, (unsigned int) temp2, nbits))
      return FALSE;

  /* Encode the AC coefficients per section F.1.2.2 */
  
  r = 0;            /* r = run length of zeros */
  
  for (k = 1; k < DCTSIZE2; k++) {
    if ((temp = block[cjpeg_natural_order[k]]) == 0) {
      r++;
    } else {
      /* if run length > 15, must emit special run-length-16 codes (0xF0) */
      while (r > 15) {
    if (! emit_bits(state, actbl->ehufco[0xF0], actbl->ehufsi[0xF0]))
      return FALSE;
    r -= 16;
      }

      temp2 = temp;
      if (temp < 0) {
    temp = -temp;        /* temp is abs value of input */
    /* This code assumes we are on a two's complement machine */
    temp2--;
      }
      
      /* Find the number of bits needed for the magnitude of the coefficient */
      nbits = 1;        /* there must be at least one 1 bit */
      while ((temp >>= 1))
    nbits++;
      /* Check for out-of-range coefficient values */
      if (nbits > MAX_COEF_BITS)
    ERREXIT(state->cinfo, JERR_BAD_DCT_COEF);
      
      /* Emit Huffman symbol for run length / number of bits */
      i = (r << 4) + nbits;
      if (! emit_bits(state, actbl->ehufco[i], actbl->ehufsi[i]))
    return FALSE;

      /* Emit that number of bits of the value, if positive, */
      /* or the complement of its magnitude, if negative. */
      if (! emit_bits(state, (unsigned int) temp2, nbits))
    return FALSE;
      
      r = 0;
    }
  }

  /* If the last coef(s) were zero, emit an end-of-block code */
  if (r > 0)
    if (! emit_bits(state, actbl->ehufco[0], actbl->ehufsi[0]))
      return FALSE;

  return TRUE;
}


/*
 * Emit a restart marker & resynchronize predictions.
 */

LOCAL(boolean)
emit_restart (working_state * state, int restart_num)
{
  int ci;

  if (! flush_bits(state))
    return FALSE;

  emit_byte(state, 0xFF, return FALSE);
  emit_byte(state, JPEG_RST0 + restart_num, return FALSE);

  /* Re-initialize DC predictions to 0 */
  for (ci = 0; ci < state->cinfo->comps_in_scan; ci++)
    state->cur.last_dc_val[ci] = 0;

  /* The restart counter is not updated until we successfully write the MCU. */

  return TRUE;
}


/*
 * Encode and output one MCU's worth of Huffman-compressed coefficients.
 */

METHODDEF(boolean)
encode_mcu_huff (j_compress_ptr cinfo, JBLOCKROW *MCU_data)
{
  huff_entropy_ptr entropy = (huff_entropy_ptr) cinfo->entropy;
  working_state state;
  int blkn, ci;
  jpeg_component_info * compptr;

  /* Load up working state */
  state.next_output_byte = cinfo->dest->next_output_byte;
  state.free_in_buffer = cinfo->dest->free_in_buffer;
  ASSIGN_STATE(state.cur, entropy->saved);
  state.cinfo = cinfo;

  /* Emit restart marker if needed */
  if (cinfo->restart_interval) {
    if (entropy->restarts_to_go == 0)
      if (! emit_restart(&state, entropy->next_restart_num))
    return FALSE;
  }

  /* Encode the MCU data blocks */
  for (blkn = 0; blkn < cinfo->blocks_in_MCU; blkn++) {
    ci = cinfo->MCU_membership[blkn];
    compptr = cinfo->cur_comp_info[ci];
    if (! encode_one_block(&state,
               MCU_data[blkn][0], state.cur.last_dc_val[ci],
               entropy->dc_derived_tbls[compptr->dc_tbl_no],
               entropy->ac_derived_tbls[compptr->ac_tbl_no]))
      return FALSE;
    /* Update last_dc_val */
    state.cur.last_dc_val[ci] = MCU_data[blkn][0][0];
  }

  /* Completed MCU, so update state */
  cinfo->dest->next_output_byte = state.next_output_byte;
  cinfo->dest->free_in_buffer = state.free_in_buffer;
  ASSIGN_STATE(entropy->saved, state.cur);

  /* Update restart-interval state too */
  if (cinfo->restart_interval) {
    if (entropy->restarts_to_go == 0) {
      entropy->restarts_to_go = cinfo->restart_interval;
      entropy->next_restart_num++;
      entropy->next_restart_num &= 7;
    }
    entropy->restarts_to_go--;
  }

  return TRUE;
}


/*
 * Finish up at the end of a Huffman-compressed scan.
 */

METHODDEF(void)
finish_pass_huff (j_compress_ptr cinfo)
{
  huff_entropy_ptr entropy = (huff_entropy_ptr) cinfo->entropy;
  working_state state;

  /* Load up working state ... flush_bits needs it */
  state.next_output_byte = cinfo->dest->next_output_byte;
  state.free_in_buffer = cinfo->dest->free_in_buffer;
  ASSIGN_STATE(state.cur, entropy->saved);
  state.cinfo = cinfo;

  /* Flush out the last data */
  if (! flush_bits(&state))
    ERREXIT(cinfo, JERR_CANT_SUSPEND);

  /* Update state */
  cinfo->dest->next_output_byte = state.next_output_byte;
  cinfo->dest->free_in_buffer = state.free_in_buffer;
  ASSIGN_STATE(entropy->saved, state.cur);
}


/*
 * Huffman coding optimization.
 *
 * We first scan the supplied data and count the number of uses of each symbol
 * that is to be Huffman-coded. (This process MUST agree with the code above.)
 * Then we build a Huffman coding tree for the observed counts.
 * Symbols which are not needed at all for the particular image are not
 * assigned any code, which saves space in the DHT marker as well as in
 * the compressed data.
 */


/*
 * Module initialization routine for Huffman entropy encoding.
 */

GLOBAL(void)
jinit_huff_encoder (j_compress_ptr cinfo)
{
  huff_entropy_ptr entropy;
  int i;

  entropy = (huff_entropy_ptr)
    (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
                SIZEOF(huff_entropy_encoder));
  cinfo->entropy = (struct jpeg_entropy_encoder *) entropy;
  entropy->pub.start_pass = start_pass_huff;

  /* Mark tables unallocated */
  for (i = 0; i < NUM_HUFF_TBLS; i++) {
    entropy->dc_derived_tbls[i] = entropy->ac_derived_tbls[i] = NULL;
  }
}
