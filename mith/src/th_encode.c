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
 *
 *   DESC : Functional Layer UUEncoding Routines
 *
 *
 *------------------------------------------------------------------------------
 * Other Copyright Notice (if any): 
 * 
 * For conditions of distribution and use, see the accompanying README file.
 * ===========================================================================*/

/**
@file
@brief Functional Layer UUEncoding Routines.

Simplified interface using UUENCODING (as specified in GNU) Writes it's
output to stdout

Adapted by Sergei Larin
*/ 

#include "th_cfg.h"
#include "th_encode.h" 
#include "th_types.h" /* for Char */
int th_printf( const char *fmt, ... );

/** Standard encoding table. Base64 is not used
@note Some versions include a '`'  termination character.
*/
const Char uu_std[64] =
{
	'`', '!', '"', '#', '$', '%', '&', '\'',
	'(', ')', '*', '+', ',', '-', '.', '/',
	'0', '1', '2', '3', '4', '5', '6', '7',
	'8', '9', ':', ';', '<', '=', '>', '?',
	'@', 'A', 'B', 'C', 'D', 'E', 'F', 'G',
	'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
	'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W',
	'X', 'Y', 'Z', '[', '\\', ']', '^', '_',
};

/** Pointer to the translation table we currently use.  */
const Char *trans_ptr = uu_std;

/** ENC is the basic 1 character encoding function to make a char printing.  */
#define ENC(Char) (trans_ptr[(Char) & 077])

/*
 *	Gnu style uuencoding routine
 *  encodes buffer raw_buffer of size raw_buf_len
 *
 */ 

void encode (const unsigned char *raw_buffer, size_t raw_buf_len)
{
  size_t			ch, n	= 0;
  size_t		    buf_cnt = 0;
  Char			    buf[80];
  register Char     *p		= buf;
  

  if(!raw_buf_len || !raw_buffer)
  {
	  th_printf("Uuencode buffer parameters error.\n");
	  return;
  } 

  while (1)
  {
	  for(n = 0; n<45;n++)
	  {
		  if(n+buf_cnt >= raw_buf_len) break; 
		  buf[n]	= raw_buffer[n+buf_cnt]; 
	  }
	  buf_cnt += 45; 

      if (n == 0)					break;
	  if (th_printf ("%c",ENC (n)) == EOF)	break;

	  for (p = buf; n > 2; n -= 3, p += 3)
	  {
			ch = *p >> 2;
			ch = ENC (ch);
			if (th_printf("%c",ch) == EOF)	break;
			ch = ((*p << 4) & 060) | ((p[1] >> 4) & 017);
			ch = ENC (ch);
			if (th_printf("%c",ch) == EOF)	break;
			ch = ((p[1] << 2) & 074) | ((p[2] >> 6) & 03);
			ch = ENC (ch);
			if (th_printf("%c",ch) == EOF)	break;
			ch = p[2] & 077;
			ch = ENC (ch);
			if (th_printf("%c",ch) == EOF)	break;
		}

		if (n != 0)						break;
		if (th_printf("\n") == EOF)		break;
    }

    while (n != 0)
	{
		Char c1 = *p;
		Char c2 = (Char) (n == 1 ?  0 :  p[1]);

		ch = c1 >> 2;
		ch = ENC (ch);
		if (th_printf("%c",ch) == EOF)		break;

		ch = ((c1 << 4) & 060) | ((c2 >> 4) & 017);
		ch = ENC (ch);
		if (th_printf("%c",ch) == EOF)		break;

		if (n == 1)
			ch = ENC ('\0');
		else{
			ch = (c2 << 2) & 074;
			ch = ENC (ch);
		}
		if (th_printf("%c",ch) == EOF)		break;
		ch = ENC ('\0');
		if (th_printf("%c",ch) == EOF)		break;
		th_printf("\n");
		break;
    }

	th_printf("\0\n");
}

/** Send buffer that uudecode will extract as a file.
@note Add spaces after file data to avoid short file warnings from uudecode.
@param buf data to encode.
@param length length of data
@param fn file name for uudecode.
@retval 0 Success (No user fail modes).
*/
int uu_send_buf( const unsigned char* buf, size_t length, const char* fn  )
{

   /* tell uudecode file mode (owner read/write), and file name. */ 
   th_printf ("begin 600 %s\n",fn);
   encode(buf,length);
   /* tell uudecode the file is done. */
   th_printf ("   \nend\n\n");
   return 0; /* Success; */ 
}
