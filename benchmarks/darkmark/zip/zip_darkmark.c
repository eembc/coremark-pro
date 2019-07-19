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

/*************************

*************************/
#include "th_cfg.h"
#include "th_file.h"
#include "th_math.h" /* for sin, cos and pow */
#include "th_lib.h"
#include "th_rand.h" /* initialize a random data vector */
#include "th_types.h"

#include "zlib.h"

typedef struct zip_params_s {
	e_u8 *zip_buf;
	e_u32 zip_buf_len;
	e_u8 *unz_buf;
	e_u32 unz_buf_len;
	e_s32 expected_result;
	e_s32 gen_ref;
	e_s32 seed; 
	e_u16 zip_result;
	char *filename;
	int debug;
	e_s32 unzip;
	int err;
	e_u32 buf_type;
	e_u16 expected_crc;
} zip_params;

/* file provides :
define - init base input params (segment [within 0..2 boundary], number of coefficients to calculate, number of integration steps)
init -  allocate working memory, assign a[0] and b[0] . 
run - calculate N coefficients in the segment
fini - calculate avg of coeffients then dealloc working memory
verify - SNR average of coefficients vs golden reference, must run at least base iterations for valid output.
clean - deallocate output memory
*/
void *define_params_zip(unsigned int idx, char *name, char *dataset);
void *bmark_init_zip(void *);
void *t_run_test_zip(struct TCDef *,void *);
int bmark_clean_zip(void *);
int bmark_verify_zip(void *in_params);
void *bmark_fini_zip(void *in_params);

/* benchmark function declarations */
e_u16 do_unz(zip_params *p);
e_u16 do_zip(zip_params *p);

#define MULTI_LINE_STRING(a) #a

static const char *default_start = MULTI_LINE_STRING(
<?xml version='1.0'?>
<html>
  <body scene='test'>
);

static const char *default_end = MULTI_LINE_STRING(
  </body>
</html>
<footer>(c) EEMBC</footer>
);

#define max_entry 100

static zip_params defaults[] = {
	{NULL,13560,NULL,1048113,0x34f8,0,8989,0,NULL,0,0,0,0,0x1764}, /* dataset 0 */
	{NULL,286,NULL,1000,0x011e,0,8989,0,NULL,0,0,0,99,0x6f87}	, /* dataset 1 */
	{NULL,286,NULL,1000,0x011e,0,8989,0,NULL,0,0,0,0,0xd371}	, /* dataset 2 */
	{NULL,377,NULL,1000,0x0179,0,8989,0,NULL,0,0,0,1,0xd853}	, /* dataset 3 */
	{NULL,509,NULL,1000,0x01fd,0,40,0,NULL,0,0,0,2,0xd55e}		, /* pgo gen - dataset 4 */
};

static char *cname[] = { "EEMBC" , "SAGESOFT", "INTEL" , "Lockheed Martin", "ST"   , "RENESAS" 	};
static char *pname[] = { "Shay"  ,  "Markus" , "Pierre", "Vader"          , "Skeet", "Boron" 	};
static e_u32 requested_size=0; 

static char *gen_parse_buf(e_u32 *psize, void *r, e_u32 record_type) {
	char lbuf[100];
	e_u32 size=*psize;
	char *buf=th_malloc(size);
	e_u32 start_size=th_strlen(default_start);
	e_u32 end_size=th_strlen(default_end);

	if (buf==NULL)
		return NULL;
	*buf=0;
	/* setup xml/html header */
	th_strcat(buf,default_start);
	size-=start_size;
	while (size > (end_size+max_entry)) 
	{
		/* add records until reaching required size */
		char *company=	cname[random_u32(r) % (sizeof(cname)/sizeof(char *))];
		char *name=		pname[random_u32(r) % (sizeof(pname)/sizeof(char *))];
		e_u32 id=random_u32_inrange(r,1,0xfff);
		switch (record_type) {
			case 1:
				th_sprintf(lbuf,"%s,%s,%d\n",company,name,id);
				break;
			case 2:
				th_sprintf(lbuf,"%d\n",id);
				break;
			default:
				th_sprintf(lbuf,"<p company='%s'><b>%s</b><data>%d</data></p>\n",company,name,id);
				break;
		}
		th_strcat(buf,lbuf);
		size-=th_strlen(lbuf);
	}
	/* setup xml/html footer */
	th_strcat(buf,default_end);
	size-=end_size;
	/* setup the actual size of the buffer being used */
	*psize-=size;
	return buf;
}

void *define_params_zip(unsigned int idx, char *name, char *dataset) {
    zip_params *params;
	e_s32 data_index=idx;
	e_u32 size=0;

	/* parameter setup */
	params=(zip_params *)th_malloc(sizeof(zip_params));
	if ( params == NULL )
          th_exit( THE_OUT_OF_MEMORY, "Cannot Allocate Memory %s:%d", __FILE__,__LINE__ );

	/* setup defaults */
	
	th_parse_buf_flag(dataset,"-i",&data_index);
	if (idx>=sizeof(defaults)/sizeof(zip_params))
		data_index=0;
	if (pgo_training_run!=0) {
		data_index=4; 
	} 
	th_memcpy(params,&(defaults[data_index]),sizeof(zip_params));
	/* command line overrides */
	if (pgo_training_run==0) {
		th_parse_buf_flag(dataset,"-d=",&params->unzip);
		th_parse_buf_flag_unsigned(dataset,"-n=",&size);
		th_parse_buf_flag_unsigned(dataset,"-t=",&params->buf_type);
		th_parse_buf_flag(dataset,"-s",&params->seed);
		th_parse_buf_flag(dataset,"-g",&params->gen_ref);
		th_get_buf_flag(dataset,"-f=",&params->filename);
	}
	if (params->filename) { /* file name defined, read into buf) */
		ee_FILE *f=th_fopen(params->filename,"rb");
		size_t fs=th_fsize(params->filename);
		if (params->unzip) {
			params->unz_buf=(e_u8 *)th_malloc(fs);
			params->unz_buf_len=fs;
			th_fread(params->unz_buf,fs,1,f);
		} else {
			params->zip_buf=(e_u8 *)th_malloc(fs);
			params->zip_buf_len=fs;
			th_fread(params->zip_buf,fs,1,f);
		}
		fclose(f);
	} else { /* generate sample data on the fly */
		if (size==0)
			size=params->unz_buf_len;
		if (size != 0) {
			requested_size=size;
			void *r=rand_init(params->seed, 0xff, 0, 0);
			switch (params->buf_type) {
				case 99:
					params->unz_buf=random_u8_vector(size,params->seed);
					break;
				default:
					params->unz_buf=(e_u8 *)gen_parse_buf(&size,r,params->buf_type);
					break;
			}
			params->unz_buf_len=size;
			rand_fini(r);
			if (params->unzip) { /* If we need to unzip create zipped version */
				do_zip(params);
			}
		}
	}
	
	return params;
}
#define CHECK_ERR(err, msg,p) { \
    if (err != Z_OK) { \
        th_printf("%s error: %d\n", msg, err); \
        p->err++; \
    } \
}

e_u16 do_zip(zip_params *p)
{
    int err;
    uLong ulen = p->unz_buf_len;
    uLong zlen = compressBound(ulen);
	if (p->zip_buf)
		th_free(p->zip_buf);
	p->zip_buf=(e_u8 *)th_malloc(zlen);
	if (p->zip_buf==NULL) {
		th_printf("Could not allocate memory!\n");
		return 0;
	}
    err = compress(p->zip_buf, &zlen, p->unz_buf, ulen);
    CHECK_ERR(err, "compress",p);
	p->zip_buf_len=zlen;
	/* TODO: subsample output to determine if ok */
	return zlen;
}
e_u16 do_unz(zip_params *p)
{
    int err;
    uLong ulen = p->unz_buf_len;
    uLong zlen = p->zip_buf_len;
	if (p->unz_buf)
		th_free(p->unz_buf);
	p->unz_buf=(e_u8 *)th_malloc(ulen);
	if (p->unz_buf==NULL) {
		th_printf("Could not allocate memory!\n");
		return 0;
	}
    err = uncompress(p->unz_buf, &ulen, p->zip_buf, zlen);
    CHECK_ERR(err, "compress",p);
	/* TODO: subsample output to determine if ok */
	return ulen;
}

int bmark_clean_zip(void *in_params) {
	zip_params *params=(zip_params *)in_params;
	if (in_params==NULL)
		th_exit( THE_BAD_PTR, "Invalid pointer %s:%d", __FILE__,__LINE__ );
	if (params->unz_buf!=NULL)
		th_free(params->unz_buf);
	if (params->zip_buf!=NULL)
		th_free(params->zip_buf);
	th_free(params);
	return 1;
}

void *bmark_init_zip(void *in_params) {
	zip_params *params=(zip_params *)in_params;
    zip_params *p;
	if (in_params==NULL)
		th_exit( THE_BAD_PTR, "Invalid pointer %s:%d", __FILE__,__LINE__ );
	p=(zip_params *)th_malloc(sizeof(zip_params));
	if ( p == NULL )
          th_exit( THE_OUT_OF_MEMORY, "Cannot Allocate Memory %s:%d", __FILE__,__LINE__ );
	th_memcpy(p,params,sizeof(zip_params));
	/* Copy the input */
	if (p->unzip) {
		p->zip_buf=(e_u8 *)th_malloc(params->zip_buf_len);
		th_memcpy(p->zip_buf,params->zip_buf,params->zip_buf_len);
	} else {
		p->unz_buf=(e_u8 *)th_malloc(params->unz_buf_len);
		th_memcpy(p->unz_buf,params->unz_buf,params->unz_buf_len);
	}

	return p;
}
void *bmark_fini_zip(void *in_params) {
    zip_params *params;
	if (in_params==NULL)
		th_exit( THE_BAD_PTR, "Invalid pointer %s:%d", __FILE__,__LINE__ );
	params=(zip_params *)in_params;
	if (params->zip_buf)
		th_free(params->zip_buf);
	if (params->unz_buf)
		th_free(params->unz_buf);
	
	th_free(params);

	return NULL;
}

void *t_run_test_zip(struct TCDef *tcdef,void *in_params) {
	zip_params *p=(zip_params *)in_params;
	int res=0, err;
	tcdef->expected_CRC=0;
	p->err=0;
	/* perform requested op on buffer */
	if (p->unzip)
		p->zip_result=do_unz(p);
	else 
		p->zip_result=do_zip(p);
	
	if ((p->zip_result != p->expected_result) || (p->err > 0))
		tcdef->CRC=1;
	
	if (p->gen_ref)
		tcdef->CRC=0;
		
	tcdef->v1=p->zip_result;
	return tcdef;
}

int bmark_verify_zip(void *in_params) {
	zip_params *params=(zip_params *)in_params;
	e_u8 *zip_buf;
	e_u32 zip_buf_len;
	e_u8 *unz_buf;
	e_u32 unz_buf_len;
	e_s32 expected_result;
	e_s32 gen_ref;
	e_s32 seed; 
	e_u16 crc=0;
	char *filename;
	int debug;
	e_s32 unzip;
	int err;
	
	if (params->unzip) {
		crc=th_crcbuffer(params->unz_buf,params->unz_buf_len,params->zip_result);
	} else {
		crc=th_crcbuffer(params->zip_buf,params->zip_buf_len,params->zip_result);
	}
	if (params->gen_ref) {
		th_printf("{NULL,%d,NULL,%d,0x%04x,0,%d,0,%s,0,%d,0,%d,0x%04x},\n",
			params->zip_buf_len,requested_size,params->zip_result,params->seed,
			params->filename==NULL?"NULL":params->filename,
			params->unzip,params->buf_type,crc);
	} else {
		if (crc != params->expected_crc) {
			th_printf("Error! Expected 0x%x got 0x%x\n",params->expected_crc,crc);
			return 0;
		}
	}
	return 1;
}

int ZEXPORT gzclose_r(gzFile file) {
	return Z_STREAM_ERROR;
}

int ZEXPORT gzclose_w(gzFile file) {
	return Z_STREAM_ERROR;
}
