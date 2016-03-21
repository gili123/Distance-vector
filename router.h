/*
 * main.c
 *
 *	Program: Router implication with distance vector algorithm
 *	File: router: h file
 *
 *  Created on: Jan 8, 2015
 *      Author: Gili Mizrahi  302976840
 */

#ifndef ROUTER_H_
#define ROUTER_H_

#define INF 1000000
#define TRUE 1
#define FALSE 0

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct router{
	char* name;
	int port;
	char* address;
	int n_neighbor;
	int n_routers;
    struct neighbor** neighbors;
    struct DV** dv;
    int* change;
    struct VIA** via;
}router;

typedef struct neighbor{
	char* name;
	int cost;
	int port;
	char* address;
	struct DV** dv;
}neighbor;


typedef struct DV{
	char* name;
	int estimate;
}DV;


typedef struct VIA{
	char* dest;
	char* via_router;
}VIA;


//create router, if succeed return the reference else return null
router* craete_router(char* name, int n_DV);

//create neighbor, if succeed return the reference els return null
neighbor* create_neighbor(char* name, int cost,int  n_DV);

//create DV, if succeed return the reference else return null
DV* create_DV(char* name, int estimate_cost);

//initialize the router at the first time, if succeed return the reference else return null
router* initialize_router(FILE* file, char* name);

//find and add the neighbors of the router, if there is such problem return 1 else return 0
int find_neighbors(FILE* file, char* name, router* rou, int n_DV);

//check if exists at dv a specific node, if yes return TRUE=1 else return FALSE=0
int is_node_exists(DV** dv, char* node);

//initialize the dv and all neighbors's dv at the first time
void initialize_DV(router* rou);

//initialize via at the first time
int initialize_via(router* rou, int n_DV);

//create VIA, if succeed return the reference else return null
VIA* create_via(char* dest, char* via);

//check if name is neighbor of this router, if yes return the index of him at the neighbors array else return -1
int is_neighbor(neighbor** neig, char* name, int n_neghbors);

//initialize the port and address for this router and all neighbors, if succeed return 0, else return 1
int finds_port_address(FILE* file, router* rou);

//print the way for each router (destination  via  cost)
void print_Table(router* rou);

//free router
void freeRouter();


#endif /* ROUTER_H_ */
