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
** Black Scholes .
** Calculate the theta of black scholes for stocks.
** Based on the equation in wikipedia (see http://en.wikipedia.org/wiki/Black%E2%80%93Scholes).
** 
*************************/
#include "th_cfg.h"
#include "th_file.h"
#include "th_math.h" /* for th_sin, th_cos and th_pow */
#include "th_lib.h"
#include "th_rand.h" /* initialize a random data vector */
#include "th_types.h"

#include "ezxml.h"

typedef struct parser_params_s {
	char *xml_buf;
	e_u32 buf_len;
	e_s32 expected_result;
	e_s32 gen_ref;
	e_s32 seed; 
	e_u16 parser_result;
	char *filename;
	e_s32 debug;
} parser_params;

//parser_params presets_parser[NUM_DATAS];

/* file provides :
define - init base input params (segment [within 0..2 boundary], number of coefficients to calculate, number of integration steps)
init -  allocate working memory, assign a[0] and b[0] . 
run - calculate N coefficients in the segment
fini - calculate avg of coeffients then dealloc working memory
verify - SNR average of coefficients vs golden reference, must run at least base iterations for valid output.
clean - deallocate output memory
*/
void *define_params_parser(unsigned int idx, char *name, char *dataset);
void *bmark_init_parser(void *);
void *t_run_test_parser(struct TCDef *,void *);
int bmark_clean_parser(void *);
int bmark_verify_parser(void *in_params);
void *bmark_fini_parser(void *in_params);

/* benchmark function declarations */

#define MULTI_LINE_STRING(a) #a
static const char *default_xml = MULTI_LINE_STRING(
<?xml version='1.0'?>
<html>
  <body scene='test'>
    <p company='EEMBC'>
      <b>Shay Gal-On</b>
      <data>42</data>
    </p>
    <p company='EEMBC'>
      <b>Markus Levy</b>
      <data>66</data>
    </p>
  </body>
</html>
<footer>(c) EEMBC</footer>
);

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

static parser_params defaults[] = {
	{"",125000,0x56f8,0,0,0,NULL,0} , /*~125k*/
	{"",511943,0x2d85,0,0,0,NULL,0} , /*~500K*/
	{"",414,0x50cd,0,0,0,NULL,0} /* ~500 : pgo training input */
};

static char *cname[] = { "EEMBC" , "SAGESOFT", "INTEL" , "Lockheed Martin", "ST"   , "RENESAS" 	};
static char *pname[] = { "Shay"  ,  "Markus" , "Pierre", "Vader"          , "Skeet", "Boron" 	};
static e_u32 requested_size;

static char *gen_parse_buf(e_u32 *psize, void *r) {
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
		th_sprintf(lbuf,"<p company='%s'><b>%s</b><data>%d</data></p>\n",company,name,id);
		th_strcat(buf,lbuf);
		size-=th_strlen(lbuf);
	}
	/* setup xml/html footer */
	th_strcat(buf,default_end);
	size-=end_size;
#if BMDEBUG
	th_printf("Req\t%06d\nleft\t%06d\nactual\t%06d\n",*psize,size,*psize-size);
#endif
	/* setup the actual size of the buffer being used */
	*psize-=size;
	return buf;
}

void *define_params_parser(unsigned int idx, char *name, char *dataset) {
    parser_params *params;
	e_s32 data_index=idx;
	e_u32 size=0;

	/* parameter setup */
	params=(parser_params *)th_malloc(sizeof(parser_params));
	if ( params == NULL )
          th_exit( THE_OUT_OF_MEMORY, "Cannot Allocate Memory %s:%d", __FILE__,__LINE__ );

	/* setup defaults */
	defaults[0].xml_buf=(char *)default_xml;
	
	th_parse_buf_flag(dataset,"-i",&data_index);
	if (data_index>=sizeof(defaults)/sizeof(parser_params))
		data_index=0;
	if (pgo_training_run!=0) {
		data_index=2; 
	}
	th_memcpy(params,&(defaults[data_index]),sizeof(parser_params));
	size=params->buf_len;
	/* command line overrides */
	if (pgo_training_run==0) {
		th_parse_buf_flag_unsigned(dataset,"-n=",&size);
		th_parse_buf_flag(dataset,"-s",&params->seed);
		th_parse_buf_flag(dataset,"-g",&params->gen_ref);
		th_parse_buf_flag(dataset,"-d",&params->debug);
		th_get_buf_flag(dataset,"-f=",&params->filename);
	}
	if (params->filename) { /* file name defined, read into buf) */
		ee_FILE *f=th_fopen(params->filename,"rb");
		size_t fs=th_fsize(params->filename);
		params->xml_buf=(char *)th_malloc(fs);
		params->buf_len=fs;
		th_fread(params->xml_buf,fs,1,f);
		th_fclose(f);
	} else { /* generate sample data on the fly */
		if (size != 0) {
			requested_size=size;
			void *r=rand_init(params->seed, 0xff, 0, 0);
			params->xml_buf=gen_parse_buf(&size,r);
			params->buf_len=size;
			rand_fini(r);
		}
	}
	
	return params;
}


int bmark_clean_parser(void *in_params) {
	parser_params *params=(parser_params *)in_params;
	if (in_params==NULL)
		th_exit( THE_BAD_PTR, "Invalid pointer %s:%d", __FILE__,__LINE__ );
	if(params->xml_buf!=NULL)
		th_free(params->xml_buf);
	th_free(params);
	return 1;
}

void *bmark_init_parser(void *in_params) {
	parser_params *params=(parser_params *)in_params;
    parser_params *myparams;
	if (in_params==NULL)
		th_exit( THE_BAD_PTR, "Invalid pointer %s:%d", __FILE__,__LINE__ );
	myparams=(parser_params *)th_malloc(sizeof(parser_params));
	if ( myparams == NULL )
          th_exit( THE_OUT_OF_MEMORY, "Cannot Allocate Memory %s:%d", __FILE__,__LINE__ );
	th_memcpy(myparams,params,sizeof(parser_params));
	myparams->xml_buf=(char *)th_malloc(params->buf_len);
	if (myparams->xml_buf==NULL)
          th_exit( THE_OUT_OF_MEMORY, "Cannot Allocate Memory %s:%d", __FILE__,__LINE__ );
		
	th_memcpy(myparams->xml_buf,params->xml_buf,params->buf_len);

	return myparams;
}
void *bmark_fini_parser(void *in_params) {
    parser_params *params;
	if (in_params==NULL)
		th_exit( THE_BAD_PTR, "Invalid pointer %s:%d", __FILE__,__LINE__ );
	params=(parser_params *)in_params;
	if (params->xml_buf)
		th_free(params->xml_buf);
	
	th_free(params);

	return NULL;
}

void *t_run_test_parser(struct TCDef *tcdef,void *in_params) {
	parser_params *params=(parser_params *)in_params;
	int res=0, err;
	ezxml_t top,body,person,tag;
	const char *scene;
	tcdef->expected_CRC=0;
	/* initial parser into tree */
	top=ezxml_parse_str(params->xml_buf,params->buf_len);
	if (!top)
		th_exit(THE_BAD_PTR,"Invalid XML\n");
	/* get body element */
	body=ezxml_child(top, "body");
	if (!body)
		th_exit(THE_BAD_PTR,"No body TAG\n");
	scene=ezxml_attr(body,"scene");
	if (!scene || (th_strcmp(scene,"test")!=0) )
		th_exit(THE_BAD_PTR,"Invalid scene in XML\n");
	params->parser_result=0;
	/* Work with body element, sample */
	for (person = ezxml_child(body, "p"); person; person = person->next) {
		const char *company, *name;
		int data;
		company=ezxml_attr(person,"company");
		if (!company)
			th_exit(THE_BAD_PTR,"Bad company name\n");
		tag=ezxml_child(person,"b");
		if (!tag)
			th_exit(THE_BAD_PTR,"Bad person name\n");
		name=tag->txt;
		tag=ezxml_child(person,"data");
		if (!tag)
			th_exit(THE_BAD_PTR,"Bad data\n");
		data=th_atoi(ezxml_child(person,"data")->txt);
		
		params->parser_result=th_crcbuffer(company,th_strlen(company),params->parser_result);
		params->parser_result=th_crcbuffer(name,th_strlen(name),params->parser_result);
		params->parser_result=Calc_crc32(data,params->parser_result);
		if (params->debug)
			th_printf("%d:%s,%s\n",data,name,company);
	}
	ezxml_free(top);
	
	
	if (params->parser_result!=params->expected_result) {
		tcdef->CRC=1;
		th_printf("ERROR: res=%x exp=%x\n", params->parser_result,params->expected_result);
	}
	
	if (params->gen_ref)
		tcdef->CRC=0;
		
	tcdef->v1=params->parser_result;
	return tcdef;
}

int bmark_verify_parser(void *in_params) {
	parser_params *params=(parser_params *)in_params;
	
	if (params->gen_ref) {
		char *n="NULL";
		if (params->filename)
			n=params->filename;
		th_printf("{\"\",%d,0x%04x,0,0,0,%s,0}\n",requested_size,params->parser_result,n);
	}
	
	return 1;
}
