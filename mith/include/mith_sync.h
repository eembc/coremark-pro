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

#ifndef MITH_SYNC_H
#define MITH_SYNC_H

#include "al_smp.h"
#define MAX_CONNECTIONS 32

typedef enum {
	UNTYPED,
	PRODUCER_CONSUMER,
	CONTROL,
	DATA
} ee_connect_type;

typedef struct ee_connect_requires_s
{
	unsigned int idx;	/* Index for the connection */
	char *type;			/* Arbitrary string defining connection type */
	int ifrom;			/* Min index in define params requiring/allowing this connection, -1 for always */
	int ito;			/* Max index in define params requiring/allowing this connection, -1 for always */
	char *dataset;		/* Regular expression on the dataset that would allow/require this connection , NULL for always*/
	int required;		/* Indicates weather connection is required if index/dataset allow */
	char *comment;		/* Comment about this type of connection, NULL if none */
} ee_connect_requires;

typedef unsigned int ee_connect_handle;

typedef struct ee_connection_s {
	ee_connect_type type;
	ee_connect_handle handle;
	char *name;
	unsigned int from;
	unsigned int to;
	al_mutex_t mutex;
	al_cond_t signal;
	void *data;
	int gid;
} ee_connection;

/* Work item connection related functions.
	These helper function are a way to use the connections to synchronize items.
	The protocol works like this:
	Item A  -->  Item B
	Two connections are needed. The first connection is used to control B, the second is used to throttle A and pass data.
	The first connection is referred to as CTRL.
	The second connection is referred to as DATA.
	The data on CTRL connection is initialized by A, and becomes "QUIT" when A is done working.
	A owns the mutex on CTRL, and B owns the mutex on DATA.

	Before starting work, A needs to init_ctrl() and B needs to lock_in().
	Then, B needs to:
		wait_prev_start(map,connections,channel); 
	and:
		while (signal_in(map,connections,channel) != QUIT) {
			wait_prev_ready(map,connections,channel);
			buf=get_next(map,connections,channel);
			.. do work on buf	
		}
	A needs to:
		... create a buffer
		wait_next_ready(map,connections,channel);
		set_out(map,connections,buf,channel);
		signal_out_ready(map,connections,channel);
	And when done:
		signal_out(map,connections,QUIT,channel);
		wait_next_ready(map,connections,channel);
*/
/* Connections come in pairs (ctrl and data), helper struct to make core more readable */
typedef struct map_ent_s
{
	int ctrl;
	int data;
} map_ent;

typedef enum {
	INVALID_SIG=0,
	START=1,
	CONTINUE=2,
	ACK=3,
	QUIT=5
} signal_num;

int chain_init_ctrl(map_ent *map_out, ee_connection **connections, int channel);
int chain_signal_in_ready(map_ent *map_in, ee_connection **connections, int channel);
signal_num chain_signal_in(map_ent *map_in, ee_connection **connections, int channel);
int chain_ready_in(map_ent *map_in, ee_connection **connections, int channel);
int chain_signal_out(map_ent *map_out, ee_connection **connections, signal_num sendme, int channel);
int chain_wait_prev_ready(map_ent *map_in, ee_connection **connections, int channel);
int chain_wait_prev_start(map_ent *map_in, ee_connection **connections, int channel);
e_u8 *chain_get_next(map_ent *map_in, ee_connection **connections, int channel);
int chain_set_out(map_ent *map_out, ee_connection **connections, e_u8 *buf, int channel);
int chain_signal_out_ready(map_ent *map_out, ee_connection **connections, int channel);
int chain_lock_in(map_ent *map_in, ee_connection **connections, int channel, int lock_state);
int chain_lock_out(map_ent *map_out, ee_connection **connections, int channel, int lock_state);
int chain_wait_next_ready(map_ent *map_out, ee_connection **connections, int channel);

#endif /*MITH_SYNC_H*/
