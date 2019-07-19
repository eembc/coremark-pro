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

/* Created with: ../xml/sha-test.xml */
/* common */
#include "th_lib.h"
#include "mith_workload.h"
#include "al_smp.h"

/* helper function to initialize a workload item */
ee_work_item_t *helper_shatest(ee_workload *workload, void *params, char *name, void * (*init_func)(void *), e_u32 repeats_override,
			void * (*bench_func)(struct TCDef *,void *), int (*cleanup)(void *), void * (*fini_func)(void *), int (*veri_func)(void *), int ncont,
			e_u32 kernel_id, e_u32 instance_id) {
	ee_work_item_t *item;
	if (params==NULL) {
		th_exit(1,"Error when trying to define benchmark params");
	}
	item=mith_item_init(repeats_override);
	item->params=params;
	if (th_strlen(name)>(MITH_MAX_NAME-1)) {
		th_strncpy(item->shortname,name,MITH_MAX_NAME-1);
		item->shortname[MITH_MAX_NAME-1]='\0';
	}
	else
		th_strcpy(item->shortname,name);
	item->init_func=init_func;
	item->fini_func=fini_func;
	item->veri_func=veri_func;
	item->bench_func=bench_func;
	item->cleanup=cleanup;
	item->num_contexts=ncont;
	item->kernel_id=kernel_id;
	item->instance_id=instance_id;
	mith_wl_add(workload,item);
	return item;
}
/* generated function types for each work item */
/* sha */
extern void *define_params_sha(unsigned int idx, char *name, char *dataset);
extern void *bmark_init_sha(void *);
extern void *bmark_fini_sha(void *);
extern void *t_run_test_sha(struct TCDef *,void *);
extern int bmark_verify_sha(void *);
extern int bmark_clean_sha(void *);

#include <jni.h>
char *al_get_android_buffer();

#define ANDEBENCH_JNI __attribute__ ((visibility ("default")))

static	ee_workload *workload=NULL;
jstring Java_com_eembc_coremark_Scenario1_shatest(JNIEnv* env, jobject thiz, jstring jcommand_line) ANDEBENCH_JNI;

/* main function to create the workload, run it, and report results */
jstring Java_com_eembc_coremark_Scenario1_shatest(JNIEnv* env, jobject thiz, jstring jcommand_line)
{
	char name[MITH_MAX_NAME];
	char dataname_buf[MITH_MAX_NAME];
	char *dataname;
	char *hardware_desc;
	int orig_dataname=1;
	void *retval;
	unsigned i;
	/* default values */
	e_u32 num_contexts=1;
	e_u32 num_workers=0;
	e_u32 bench_repeats=1;
	e_u32 oversubscribe_allowed=1;
	ee_work_item_t **real_items;
	e_u32 do_phase=0xff;

	const char *command_line=(*env)->GetStringUTFChars(env,jcommand_line,0);
	char *pcmd=th_strdup(command_line);
	char *p=pcmd;
	/* first do abstraction layer specific initalizations */
	int argc=0;
	char *argv[64];
	/* parse the command line into argc/argv */
	while (*p) {
		argv[argc++]=p;
		while (*p && *p != ' ') {
			if (*p=='"') while (*++p!='"');
			p++;
		}
		*p++=0;
	}
	al_main(argc,argv);
	th_parse_flag_unsigned(argc,argv,"-phase=",&do_phase);

	/* now prepare workload */
	if ((do_phase&1) != 0) {
		workload = mith_wl_init(1); /* num items extracted from xml: sum(item*instances) for all items */
		real_items = (ee_work_item_t **)th_malloc(sizeof(ee_work_item_t *)*1);
		th_strncpy(workload->shortname,"sha-test",MITH_MAX_NAME);
		workload->rev_M=1;
		workload->rev_m=1;
		workload->uid=1050863061;
		workload->iterations=10;
		th_parse_flag_unsigned(argc,argv,"-b",&bench_repeats);
		{ e_s32 stmp;
		  if (th_parse_flag(argc,argv,"-pgo=",&stmp)) 
			pgo_training_run=stmp;
		}
		if (th_get_flag(argc,argv,"-D=",&dataname))
			orig_dataname=0;
		else
			dataname=dataname_buf;
	}

	/* parse command line for overrides
	   overrides for num_iterations, num_contexts,
	   and bench iterations for all items */
	th_parse_flag_unsigned(argc,argv,"-i",&workload->iterations);
	th_parse_flag_unsigned(argc,argv,"-c",&num_contexts);
	th_parse_flag_unsigned(argc,argv,"-w",&num_workers);
	th_parse_flag_unsigned(argc,argv,"-o",&oversubscribe_allowed);
	th_parse_flag_unsigned(argc,argv,"-v",&verify_output);
	th_parse_flag_unsigned(argc,argv,"-V",&reporting_threshold);
	/* check command line for hardware specific information */
	if (th_get_flag(argc,argv,"-P=",&hardware_desc)) {
		al_set_hardware_info(hardware_desc);
	}
	/* SG/Test: make sure we have enough iterations to engage all contexts at least once */
	while (num_contexts > (workload->iterations * 1))
		workload->iterations++;
	
/* ITEM 0-0 [0]*/
	if ((do_phase&2) != 0) {		th_strncpy(name,"sha",MITH_MAX_NAME);
		if (orig_dataname) {
			th_strncpy(dataname,"NULL",MITH_MAX_NAME);
		}
		retval=define_params_sha(0,name,dataname);
		real_items[0]=helper_shatest(workload,retval,name,bmark_init_sha,bench_repeats,t_run_test_sha,bmark_clean_sha,bmark_fini_sha,bmark_verify_sha,1,(e_u32)560644875,(e_u32)709279032);
	}
	/* Run the workload */
	if ((do_phase&4) != 0) {
		mith_main(workload,workload->iterations,num_contexts,oversubscribe_allowed,num_workers);
	}
	/* And cleanup */
	if ((do_phase&8) != 0) {
		th_free(real_items);
		for (i=0; i<workload->max_idx ; i++) {
			ee_work_item_t *item=workload->load[i];
			item->cleanup(item->params);
		}
		mith_wl_destroy(workload);
		workload=NULL;
	}
	th_free(pcmd);
	(*env)->ReleaseStringUTFChars(env, jcommand_line, command_line);
    return (*env)->NewStringUTF(env, al_get_android_buffer());

}

