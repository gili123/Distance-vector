/*
 * main.c
 *
 *	Program: Router implication with distance vector algorithm
 *	File: router: implication file
 *
 *  Created on: Jan 8, 2015
 *      Author: Gili Mizrahi  302976840
 */

#include "router.h"

//create router, if succeed return the reference else return null
router* craete_router(char* name, int n_DV){

	router* newRouter;

	if((newRouter = (router*)malloc(sizeof(router))) == NULL)
		return NULL;

	if((newRouter->name = (char*)malloc(strlen(name) +1)) == NULL)
		return NULL;

	if((newRouter->dv = (DV**)calloc(n_DV, n_DV * sizeof(DV*))) == NULL)
		return NULL;

	if((newRouter->neighbors = (neighbor**)calloc(1, sizeof(neighbor*))) == NULL)
		return NULL;

	if((newRouter->via = (VIA**)calloc(n_DV, n_DV * sizeof(VIA*))) == NULL)
		return NULL;

	if((newRouter->change = (int*)malloc(sizeof(int))) == NULL)
		return NULL;

	strcpy(newRouter->name, name);
	newRouter->n_neighbor = 0;
	newRouter->n_routers = n_DV;
	*(newRouter->change) = TRUE;

	return newRouter;
}

//initialize the router at the first time, if succeed return the reference else return null
router* initialize_router(FILE* file, char* name){

	int n_DV;
	router* new;

	fscanf(file, "%d", &n_DV);

	if((new = craete_router(name, n_DV)) == NULL)
		return NULL;

	if((find_neighbors(file, name, new, n_DV)) == 1)
		return NULL;

	initialize_DV(new);

	if((initialize_via(new, n_DV)) == 1)
		return NULL;

	if((finds_port_address(file, new)) == 1)
		return NULL;


	return new;
}

//find and add the neighbors of the router, if there is some problem return 1 else return 0
int find_neighbors(FILE* file, char* name, router* rou, int n_DV){

	int i, j, n_node, n_nodes, cost;
	int dvCounter = 0;
	char c;
	char* node1;
	char* node2;


	fseek(file, 0, SEEK_SET);

	for(i = 0; i < n_DV+1; i++)
		while((c = fgetc(file)) != '\n');


	do{

		n_nodes = 0;
		for(i = 0; i < 2; i++){

			n_node = 0;

			while((c = fgetc(file)) != ' '){
				if(c == EOF){
					fseek(file, 0, SEEK_SET);
					return 0;
				}
				n_node++;
				n_nodes++;
			}

			if(i == 0){
				if((node1 = (char*)malloc(n_node * sizeof(char) +1)) == NULL)
					return 1;
			}

			else if(i == 1){
				if((node2 = (char*)malloc(n_node * sizeof(char) +1)) == NULL)
					return 1;
			}
		}

		fseek(file, -1 * (n_nodes + 2), SEEK_CUR);
		fscanf(file, "%s %s %d", node1, node2, &cost);


		if(((strcmp(node1, name)) == 0) || ((strcmp(node2, name)) == 0)){

			rou->n_neighbor++;

			if(rou->n_neighbor > 1){
				if((rou->neighbors = (neighbor**)realloc(rou->neighbors, rou->n_neighbor * sizeof(neighbor*))) == NULL)
					return 1;
			}

			if((strcmp(node1, name)) == 0)
				rou->neighbors[rou->n_neighbor -1] = create_neighbor(node2, cost, n_DV);

			else if((strcmp(node2, name)) == 0)
				rou->neighbors[rou->n_neighbor -1] = create_neighbor(node1, cost, n_DV);
		}

		if(!is_node_exists(rou->dv, node1)){
			rou->dv[dvCounter] = create_DV(node1, INF);
			dvCounter++;
		}

		if(!is_node_exists(rou->dv, node2)){
			rou->dv[dvCounter] = create_DV(node2, INF);
			dvCounter++;
		}

		free(node1);
		free(node2);

	}while(1);


	return 1;
}

//create neighbor, if succeed return the reference els return null
neighbor* create_neighbor(char* name, int cost, int n_DV){

	neighbor* new_neighbor;

	if((new_neighbor = (neighbor*)malloc(sizeof(neighbor))) == NULL)
		return NULL;

	if((new_neighbor->name = (char*)malloc(strlen(name) +1)) == NULL)
		return NULL;

	if((new_neighbor->dv = (DV**)calloc(n_DV, n_DV * sizeof(DV*))) == NULL)
		return NULL;

	strcpy(new_neighbor->name, name);
	new_neighbor->cost = cost;

	return new_neighbor;
}

//create DV, if succeed return the reference else return null
DV* create_DV(char* name, int estimate_cost){

	DV* new_DV;

	if((new_DV = (DV*)malloc(sizeof(DV))) == NULL)
		return NULL;

	if((new_DV->name = (char*)malloc(strlen(name) +1)) == NULL)
		return NULL;

	strcpy(new_DV->name, name);
	new_DV->estimate = estimate_cost;

	return new_DV;
}

//check if exists at dv a specific node, if yes return TRUE=1 else return FALSE=0
int is_node_exists(DV** dv, char* node){

	DV* tmp = NULL;
	int i = 0;

	while((tmp = dv[i]) != NULL){
		if((strcmp(tmp->name, node)) == 0)
			return TRUE;
		i++;
	}

	return FALSE;
}

//initialize the dv and all neighbors's dv at the first time
void initialize_DV(router* rou){

	int j, i;


	for(i = 0; i < rou->n_neighbor; i++){

		j = 0;
		while(j < rou->n_routers){

			rou->neighbors[i]->dv[j] = create_DV(rou->dv[j]->name, INF);

			if(((strcmp(rou->neighbors[i]->name, rou->dv[j]->name)) == 0) &&((strcmp(rou->dv[j]->name, rou->name)) != 0))
				rou->dv[j]->estimate = rou->neighbors[i]->cost;

			else if((strcmp(rou->name, rou->dv[j]->name)) == 0)
				rou->dv[j]->estimate = 0;
			j++;
		}
	}
}

//initialize via at the first time
int initialize_via(router* rou, int n_DV){

	int i;

	for(i = 0; i < n_DV; i++){

		if((is_neighbor(rou->neighbors, rou->dv[i]->name, rou->n_neighbor) >= 0)){
			if((rou->via[i] = create_via(rou->dv[i]->name, rou->dv[i]->name)) == NULL)
				return 1;
		}

		else if((strcmp(rou->name, rou->dv[i]->name)) == 0){
			if((rou->via[i] = create_via(rou->dv[i]->name, rou->name)) == NULL)
				return 1;
		}

		else{
			if((rou->via[i] = create_via(rou->dv[i]->name, "null")) == NULL)
				return 1;
		}
	}

	return 0;
}

//create VIA, if succeed return the reference else return null
VIA* create_via(char* dest, char* via){

	VIA* new_via;

	if((new_via = (VIA*)malloc(sizeof(VIA))) == NULL)
		return NULL;

	if((new_via->dest = (char*)malloc(strlen(dest) +1)) == NULL)
		return NULL;

	if((new_via->via_router = (char*)malloc(strlen(via) +1)) == NULL)
		return NULL;

	strcpy(new_via->dest, dest);
	strcpy(new_via->via_router, via);

	return new_via;
}

//check if name is neighbor of this router, if yes return the index of him at the neighbors array else return -1
int is_neighbor(neighbor** neig, char* name, int n_neghbors){

	int i;


	for(i = 0; i < n_neghbors; i++){
		if((strcmp(neig[i]->name, name)) == 0)
			return i;
	}

	return -1;
}

//initialize the port and address for this router and all neighbors, if succeed return 0, else return 1
int finds_port_address(FILE* file, router* rou){

	int n_routers, i, index, portNum, n_lenght;
	char c;
	char* addr;
	char* node;

	fscanf(file, "%d", &n_routers);

	for(i = 0; i < n_routers; i++){

		n_lenght = 0;
		while((c = fgetc(file)) != ' ')
			n_lenght++;


		if((node = (char*)malloc(n_lenght * sizeof(char) +1)) == NULL)
			return 1;

		if((addr = (char*)malloc(16 * sizeof(char))) == NULL)
			return 1;

		fseek(file, -1 * (n_lenght + 1), SEEK_CUR);
		fscanf(file, "%s %s %d", node, addr, &portNum);

		if((strcmp(node, rou->name)) == 0){
			if((rou->address = (char*)malloc(strlen(addr) * sizeof(char) +1)) == NULL)
				return 1;
			strcpy(rou->address, addr);
			rou->port = portNum;
		}

		else if((index = is_neighbor(rou->neighbors, node, rou->n_neighbor)) >= 0){
			if((rou->neighbors[index]->address = (char*)malloc(strlen(addr) + sizeof(char) +1)) == NULL)
				return 1;
			strcpy(rou->neighbors[index]->address, addr);
			rou->neighbors[index]->port = portNum;
		}

		free(node);
		free(addr);
	}

	return 0;
}

//print the way for each router (destination  via  cost)
void print_Table(router* rou){

	int i;

	for(i = 0 ; i < rou->n_routers; i++){
		if(rou->dv[i]->estimate == INF)
			printf("%s\t%s\tinfinity\n", rou->dv[i]->name, rou->via[i]->via_router);

		else
			printf("%s\t%s\t%d\n", rou->dv[i]->name, rou->via[i]->via_router, rou->dv[i]->estimate);

	}
}

//free router
void freeRouter(router* rou){

	int i, j;

	free(rou->name);
	free(rou->address);
	free(rou->change);

	/***********free neighbors***********/

	for(i = 0; i < rou->n_neighbor; i++){

		free(rou->neighbors[i]->address);
		free(rou->neighbors[i]->name);

		for(j = 0; j < rou->n_routers; j++){

			free(rou->neighbors[i]->dv[j]->name);
			free(rou->neighbors[i]->dv[j]);
		}
		free(rou->neighbors[i]->dv);
		free(rou->neighbors[i]);
	}

	free(rou->neighbors);

	/************free DV*************/

	for(i = 0; i < rou->n_routers; i++){

		free(rou->dv[i]->name);
		free(rou->dv[i]);
	}
	free(rou->dv);

	/***********free Via************/

	for(i = 0; i < rou->n_routers; i++){

		free(rou->via[i]->dest);
		free(rou->via[i]->via_router);
		free(rou->via[i]);
	}
	free(rou->via);

	free(rou);
}
