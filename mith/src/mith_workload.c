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

/* File: mith/src/mith_workload.c
	This file provides the data structures for holding mith workloads.
 */

#include "th_lib.h"
#include "al_smp.h"
#include "mith_sync.h"
#include "mith_workload.h"

static unsigned int next_uid=10000;
/*Functions that do not need to worry about thread safety*/
ee_workload *mith_wl_init(int num_items) {
	ee_workload *workload=th_calloc(1,sizeof(ee_workload));
	workload->load=th_calloc(num_items,sizeof(ee_pitem));
	workload->max_idx=num_items;
	workload->next_connect=1;
	return workload;
}
ee_work_item_t *mith_item_init(unsigned int override_iterations) {
	ee_work_item_t *item=th_calloc(1,sizeof(ee_work_item_t));
	item->tcdef=(TCDef *)th_calloc(1,sizeof(TCDef));
	if (override_iterations>0)
		item->tcdef->iterations=override_iterations;
	item->num_contexts=1;
	item->chain_id=-1;
	item->next_idx=0;
	if (item->uid==0)
		item->uid=next_uid++;
	return item;
}
int mith_init_chains(ee_workload * workload,int numchains) {
	workload->chain_info=(chain_info_t *)th_malloc(numchains*sizeof(chain_info_t));
	workload->num_chains=numchains;
	return 1;
}
int mith_wl_destroy(ee_workload *workload) {
	e_u32 i;
	for (i=0; i<workload->max_idx; i++) {
		if (workload->load[i] != NULL) {
			th_free(workload->load[i]->tcdef);
			th_free(workload->load[i]);
			workload->load[i]=NULL;
		}
	}
	/* TODO: destroy all connections */
	if (workload->load) {
		th_free(workload->load);
		workload->load=NULL;
	}
	if (workload->chain_mutexes) {
		for (i=0; i<workload->iterations; i++) {
			th_free(workload->chain_mutexes[i]);
			workload->chain_mutexes[i]=NULL;
		}
		th_free(workload->chain_mutexes);
		workload->chain_mutexes=NULL;
	}
	if (workload->connections) {
		for (i=0; i<workload->iterations; i++) {
			th_free(workload->connections[i]);
			workload->connections[i]=NULL;
		}
		th_free(workload->connections);
		workload->connections=NULL;
	}
	if (workload->chain_info) {
		th_free(workload->chain_info);
		workload->chain_info=NULL;
	}
	if (workload->connection_protos) {
		th_free(workload->connection_protos);
		workload->connection_protos=NULL;
	}
	th_free(workload);
	return 1;
}
int mith_wl_add(ee_workload *workload, ee_work_item_t *item) {
	if (workload->next_idx>workload->max_idx)
		return 0;
	workload->load[workload->next_idx]=item;
	workload->next_idx++;
	return 1;
}

/* Function: mith_connect
	Create connection prototype.

	Actual connection will be created for each instance of the chain.
	2 items which are connected are parts of the same chain.
	This function is called before multiple threads are active and is not thread safe.

	Each time a chain is instantiated, new connections will be created.
*/
static int ee_connect_gid=0;

int mith_connect(ee_workload *workload, 
				 ee_work_item_t *item1, ee_work_item_t *item2,
				 char *name, ee_connect_type t) 
{
	ee_connection *pc;
	/* create storage if not yet allocated */
	if (workload->connection_protos==NULL) {
		workload->connection_protos=(ee_connection *)th_malloc(sizeof(ee_connection)*(1+workload->num_connections));
		workload->next_connect=0;
	}
	/* initialize prototype */
	pc=&(workload->connection_protos[workload->next_connect]);
	pc->from=item1->uid;
	pc->to=item2->uid;
	pc->name=name;
	pc->type=t;
	pc->gid=ee_connect_gid++;
	/* create mapping */
	item1->connect_idx[item1->next_idx++]=workload->next_connect;
	item2->connect_idx[item2->next_idx++]=workload->next_connect;
	/* advance index */
	workload->next_connect++;
	return 1;
}
/* Function: mith_wl_reset
	Reset all workload structures and indexes.

	This function will also initialize connections and mutexes for synchronized items.
	This is done outside of the timing loop, and final destruction of sync support is 
	done only at workload destroy.
*/
int mith_wl_reset(ee_workload *workload,unsigned int num_iterations, 
				  unsigned int num_contexts, unsigned int oversubscribe_allowed) {
	e_u32 i;
	workload->next_idx=0;
	workload->iterations=num_iterations;
	if ((int)num_contexts < workload->mcl) {
		th_exit(TH_ERROR,"This workload contains a chain that cannot be instantiated with currently defined number of contexts!");
		return 0;
	}
	for (i=0; i<workload->max_idx; i++) {
		if (workload->load[i] != NULL) {
			workload->load[i]->required=num_iterations;
			workload->load[i]->assigned=0;
			workload->load[i]->finished=0;
			workload->load[i]->time=0;
			/* Currently MITH is ignoring number of contexts actually consumed by each item.
			if (workload->load[i]->num_contexts > num_contexts) {
				if (oversubscribe_allowed)
					th_log(TH_WARNING,"Workload contains an item that oversubscribes available contexts!");
				else {
					th_log(TH_ERROR,"Workload contains an item that oversubscribes available contexts!");
					return 0;
				}
			}
			*/
		}
	}
	/* for workloads containing connected items, initialize all connections and chain structures */
	if (workload->mcl>1) {
		int mcl=workload->mcl;
		int CID=0;
		e_u32 total_mutexes=(mcl+1)*workload->num_chains;
		e_u32 total_connections=workload->num_connections;

		/* initialize all connections and mutexes */
		/* TODO: We only really need connections for the active iterations.
			That means we can reuse connections after all items from a specific iteration are done.
			That would require keeping track of which iterations are still executing, 
			and estimate upper bound on number of iterations that can execute in parallel.
			Then the <mith_wl_get_connections> function could assign new iterations to
			a connection based on a rotating buffer instead of keeping connections
			live for all iterations until all iterations are done.

			Also, currently initializing too many mutexes for chain startup/finish
			int chain_mutexes. Use chain_info struct to hold these mutexes instead?
		*/
		/* initialize all mutexes */
		if (workload->chain_mutexes) {
			th_free(workload->chain_mutexes);
			workload->chain_mutexes=NULL;
		}
		if (workload->connections) {
			th_free(workload->connections);
			workload->connections=NULL;
		}

		workload->chain_mutexes=(al_mutex_t **)th_malloc(sizeof(al_mutex_t *)*num_iterations);
		workload->connections=(ee_connection **)th_malloc(sizeof(ee_connection *)*num_iterations);
		for (i=0; i<num_iterations; i++) {
			e_u32 j;
			ee_connection *connect;
			/* mutexes */
			workload->chain_mutexes[i]=(al_mutex_t *)th_malloc(sizeof(al_mutex_t)*total_mutexes);
			for (j=0; j<total_mutexes; j++) 
				al_mutex_init(&(workload->chain_mutexes[i][j]));
			/* connections */
			workload->connections[i]=(ee_connection *)th_malloc(sizeof(ee_connection)*total_connections);
			connect=workload->connections[i];
			for (j=0; j<total_connections; j++) {
				th_memcpy(&connect[j],&workload->connection_protos[j],sizeof(ee_connection));
				/* the mutex and signal are specific for each chain, as is the handle */
				al_mutex_init(&connect[j].mutex);
				al_cond_init(&connect[j].signal);
				connect[j].data=NULL;
				connect[j].handle = CID++; /* dummy value until MPP implementation */
			}
		}
	}
	return 1;
}
/*Functions that can happen while multiple threads are running*/

/* Function: mith_wl_get_next
	Description:
	This function detects the next item that still has executions required.

	Thread Safety:
	Calls to this function should be guarded by a mutex on the workload.
	Since there are likely item specific actions that need to also be protected
	(e.g increasing the "assigned" value if accepting the workload) the protection
	is left to the caller.
*/
ee_work_item_t *mith_wl_get_next(ee_workload *workload) {
	ee_work_item_t *item=NULL;
	e_u32 start_search=workload->next_idx;
	e_u32 next_idx;
	e_u32 max_idx=workload->max_idx;
	next_idx=start_search;
	/******************** protection not needed **************************/
		while (item==NULL) { /* look for the next item we can assign */
			item=workload->load[next_idx];
			if (item->assigned >= item->required)
				item=NULL; 
			next_idx++;
			if (next_idx==max_idx)
				next_idx=0;
			if (start_search==next_idx)
				break; /* no more items we can assign for execution */
		}
		workload->next_idx=next_idx;
	/*********************************************************************/
	return item;
}
