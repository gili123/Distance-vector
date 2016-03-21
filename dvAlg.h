/*
 * dvAlg.h
 *
 *	Program: Router implication with distance vector algorithm
 *	File: Distance Vector Algorithm: h file
 *
 *  Created on: Jan 13, 2015
 *      Author: Gili Mizrahi  302976840
 */

#ifndef DVALG_H_
#define DVALG_H_

#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "router.h"
#include <errno.h>

static int loops;
int receive_count;
int send_count;
int zero;
int done;

pthread_mutex_t receive_mutex;
pthread_mutex_t count_mutex;
pthread_cond_t sender_receiver;
pthread_cond_t receiver_computing;
pthread_cond_t sender_computing;

//structure data for each sender thread
typedef struct sender{
	char* myName;
	char* addr;
	int port;
	int sock;
	int n_routers;
	int n_neighbors;
	int* change;
	DV** dv;
}sender;

//structure data for each receiver thread
typedef struct receiver{
	int port;
	int n_routers;
	int n_neighbors;
	int sock;
	int newSock;
	char* neighbor;
	char* addr;
	neighbor** neighbors;
}receiver;

//create structure data for sender thread and return its reference, if there is a problem with allocate memory return null
sender* createSenderStruct(router* rou, neighbor* neig);

//create structure data for receiver thread and return its reference, if there is a problem with allocate memory return null
receiver* createReceiverStruct(router* rou, char* neig, neighbor** neghbors);

//create client socket if succeed return the socket key else return -1
int create_client_socket(char* addr, int portNum);

//send the distance vector to all neighbors
void *send_DV(void* send_data);

//receive from all neighbors there's distance vector
void *receive_DV(void* receive);

//calculate the portNum + name - sum of ascii and return the result
int calculate_port(int portNum, char* name);

//create for calculate and for each neighbor sender and receiver threads
void create_threads(router* router, int num_of_loops);

//update the distance vector of the specific neighbor - name
void update_neighbor_DV(char* name, neighbor** neighbors, int n_neighbors, int buf[]);

//initialize the condition variable and mutex
void initialize_condVar();

//destroy the condition variable and mutex
void destroy_condVar();

//update the distance vector with the bellman-ford equation
void *Relax(void* rou);


#endif /* DVALG_H_ */
