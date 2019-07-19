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

#ifndef _MITH_WORKLOAD_H
#define _MITH_WORKLOAD_H
#include "al_smp.h"
#include "mith_sync.h"

/* File: mith/include/mith_workload.h
	Internal MITH functions and structures for creating and handling workloads.
	These should not be used outside of the scope of the test harness.
	These functions will be used by the workload created from the xml definition.
*/

/* Note! Each MITH capable benchmark needs to provide the following functions:
	void *define_params_<bench>(int idx, char *shortname, char *dataset); 
 */
#define MITH_MAX_NAME 256

/* Structure: ee_work_item_t
	This structure holds all of the information about a work item.
	
	Static:
	Information read in from the xml spec includes name, function pointers
	and number of contexts. 
	Parameters to pass to the benchmark are also part of this structure.

	Dynamic:
	Information updated while the item is executed includes timing, 
	number of executions, failures etc.
*/
typedef struct ee_work_item_s {
	char shortname[MITH_MAX_NAME];			/* a descriptor for the work item			*/
	void *params;							/* pointer to parameters, initialized by caller 		*/
	void * (*init_func)(void *params);		/* init function, can be timed separately 		*/
	void * (*fini_func)(void *params);		/* fini function, can be timed separately 		*/
	void * (*bench_func)(struct TCDef *,void *);	/* actual benchmark execution (was t_run_test) 	*/
	int (*veri_func)(void *params);	/* verify benchmark exec (return >0 if ok) 	*/
	int (*cleanup)(void *params);	/* actual benchmark execution (was t_run_test) 	*/
	unsigned int num_contexts;		/* number of contexts required				*/
	size_t time;					/* amount of time (cumulative)				*/
	unsigned int  finished;			/* number of times finished execution		*/
	unsigned int  assigned;			/* number of times assigned for execution	*/
	unsigned int  required;			/* number of times required for execution	*/
	unsigned int  failed;			/* number of times failed execution	*/
	e_u32  uid;				/* unique id for the work item in this workload (serial). */
	e_u32  kernel_id;				/* kernel id for the work item. common to all items using this kernel. */
	e_u32  instance_id;				/* instance id for the work item. common to all items with the same params. */
	struct TCDef *tcdef;		/* a way for the item to return info on run */
	/* Support for synchronization */
	e_s32 chain_id;				/* Index in the chain (DFS number). -1 if no chains in workload */
	e_s32 chain_uid;				/* Each item is associated with a specific chain, identified by chain_uid */
	e_u32 connect_idx[MAX_CONNECTIONS];		/* Mapping helpers to access actual connections */
	e_s32 next_idx;
} ee_work_item_t;

typedef struct chain_info_s {
	e_s32 id;
	int num_items;
	char *nodelist;
	char *name;
} chain_info_t;

typedef ee_work_item_t *ee_pitem;
/* Structure: ee_workload_s 
	This structure holds all the information about a workload
	A workload structure is created from an xml workload definition.
	The C file that creates and runs the workload is created using a perl
	script found under util/perl/workload_parser.pl
*/

typedef struct ee_workload_s {
	ee_pitem *load;				/* array of all items in the workload */
	unsigned int max_idx;		/* number of items in workload */
	unsigned int next_idx;		/* next item to try and assign */
	char shortname[MITH_MAX_NAME];	/* name for workload */
	e_u32 uid;			/* unique identifier for workload */
	e_u32 iterations;	/* number of iterations to run the workload */
	unsigned int rev_M;			/* revision of workload */
	unsigned int rev_m;			/* revision of workload */
	/* SYNC related info */
	struct ee_connection_s **connections;/* array of all connections active in the workload, by chain */
	struct ee_connection_s *connection_protos;/* prototypes for all connections */
	int num_connections;		/* total number of connection prototypes */
	int next_connect;			/* for building connection prototypes */
	unsigned int last_assign;	/* id of last assign */
	al_mutex_t **chain_mutexes;  /* array containing mutexes for all chains, by chain */
	chain_info_t *chain_info;  /* array containing info on each chain, by chain */
	int mcl;					/* max chain length */
	int num_chains;				/* number of chains in item list (==#items/mcl) */
	/* end of sync related info */
} ee_workload;

/* Function: mith_wl_init
	Allocate and initialize a workload that contains num_items */
ee_workload *mith_wl_init(int num_items);
/* Function: mith_item_init
	Allocate and initialize a specific work item */
ee_work_item_t *mith_item_init(unsigned int override_iterations);
/* Function: mith_wl_init
	Allocate and initialize chain info for workload with chains */
int mith_init_chains(ee_workload * workload,int numchains);
/* Function: mith_connect
	Create connection prototype.

	Actual connection will be created for each instance of the chain.
	2 items which are connected are parts of the same chain.

	Each time a chain is instantiated, new connections will be created.
*/
int mith_connect(ee_workload *workload, 
				 ee_work_item_t *item1, ee_work_item_t *item2,
				 char *name, ee_connect_type t) ;
/* Function: mith_wl_destroy
	Destroy a workload, and all attached items */
int mith_wl_destroy(ee_workload *workload);
/* Function: mith_wl_add
	Add an item to a workload */
int mith_wl_add(ee_workload *workload, ee_work_item_t *item);
/* Function: mith_wl_reset
	Reset a workload and prepare to run num_iterations of workload */
int mith_wl_reset(ee_workload *workload,unsigned int num_iterations, 
				  unsigned int num_contexts, unsigned int oversubscribe_allowed);
/* Function: mith_wl_get_next
	Helper to get the next item from a workload */
struct ee_work_item_s *mith_wl_get_next(ee_workload *workload);
/* Function: mith_main
	Description: 
	main function that fires off work items. 
	Resides in <mith_lib.c>

	Accepts a workload as a parameter, and number of iterations for the workload.

	Will then fire off items from the workload to available contexts as soon as they become available.
	Will stop once each item in the workload has iterated num_iterations, and report results.
	
	Parameters:
	workload - Actual workload to execute
	num_iterations - Number of times to execute each item of the workload
	num_contexts - number of execution contexts to execute on

	Returns:
	Instead of a return value, report results function is called to summarize results.
	*/
int mith_main(ee_workload *workload, unsigned int num_iterations, unsigned int num_contexts, Bool oversubscribe_allowed, unsigned int num_workers);

#endif /*_MITH_WORKLOAD_H*/
