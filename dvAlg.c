/*
 * dvAlg.h
 *
 *	Program: Router implication with distance vector algorithm
 *	File: Distance Vector Algorithm: implication file
 *
 *  Created on: Jan 13, 2015
 *      Author: Gili Mizrahi  302976840
 */

#include "dvAlg.h"

//initialize the condition variable and mutex
void initialize_condVar(){


	pthread_mutex_init(&count_mutex, NULL);
	pthread_cond_init (&sender_receiver, NULL);
	pthread_cond_init (&receiver_computing, NULL);
	pthread_cond_init (&sender_computing, NULL);
}

//destroy the condition variable and mutex
void destroy_condVar(){

	pthread_mutex_destroy(&count_mutex);
	pthread_cond_destroy(&sender_receiver);
	pthread_cond_destroy(&receiver_computing);
	pthread_cond_destroy(&sender_computing);
}

//create client socket if succeed return the socket key else return -1
int create_client_socket(char* addr, int portNum){

	int sock = 0;
	int i = 0;
	struct sockaddr_in  serv_addr;

	if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		printf("Error with create socket: %s\n", strerror(errno));
		pthread_exit(NULL);
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port  = htons(portNum);
	serv_addr.sin_addr.s_addr = inet_addr(addr);

	while(((connect(sock,(struct sockaddr *)&serv_addr,sizeof(serv_addr))) < 0) && (i < loops)){
		i++;
		sleep(1);
	}
	i = 0;

	return sock;
}

//create structure data for sender thread and return its reference, if there is a problem with allocate memory return null
sender* createSenderStruct(router* rou, neighbor* neig){

	sender* newSender;

	if((newSender = (sender*)malloc(sizeof(sender))) == NULL)
		return NULL;

	newSender->myName = rou->name;
	newSender->addr = neig->address;
	newSender->port = neig->port;
	newSender->dv = rou->dv;
	newSender->n_routers = rou->n_routers;
	newSender->n_neighbors = rou->n_neighbor;
	newSender->change = rou->change;


	return newSender;
}

//create structure data for receiver thread and return its reference, if there is a problem with allocate memory return null
receiver* createReceiverStruct(router* rou, char* neig, neighbor** neighbors){

	receiver* newReciever;

	if((newReciever = (receiver*)malloc(sizeof(receiver))) == NULL)
		return NULL;

	newReciever->addr = rou->address;
	newReciever->neighbor = neig;
	newReciever->port = rou->port;
	newReciever->n_routers = rou->n_routers;
	newReciever->neighbors = neighbors;
	newReciever->n_neighbors = rou->n_neighbor;

	return newReciever;
}

//send the distance vector to all neighbors
void *send_DV(void* send_data){

	int i, notChange = 0;
	int send_buf[((sender*)send_data)->n_routers +1];


	((sender*)send_data)->port = calculate_port(((sender*)send_data)->port, ((sender*)send_data)->myName);
	((sender*)send_data)->sock = create_client_socket(((sender*)send_data)->addr, ((sender*)send_data)->port);

	while(1){

		if(!done){

			bzero(send_buf, sizeof(send_buf));

			pthread_mutex_lock(&count_mutex);

			if(*(((sender*)send_data)->change) == TRUE){                    //if the distance vector was changed

				send_buf[0] = 1;

				for(i = 1; i < ((sender*)send_data)->n_routers +1; i++)
					if(((sender*)send_data)->change)
						send_buf[i] = ((sender*)send_data)->dv[i-1]->estimate;

				if((write(((sender*)send_data)->sock, (void*)&send_buf, sizeof(send_buf))) < 0){
					printf("Error could not write to socket: %s\n", strerror(errno));
					pthread_exit(NULL);
				}
			}

			else{                                                           //if the distance vector was't changed
				if((write(((sender*)send_data)->sock, (void*)&notChange, sizeof(notChange))) < 0){
					printf("Error could not write to socket: %s\n", strerror(errno));
					pthread_exit(NULL);
				}
			}

			send_count++;

			if(send_count == ((sender*)send_data)->n_neighbors){
				for(i = 0; i < ((sender*)send_data)->n_neighbors; i++)
					pthread_cond_signal(&sender_receiver);                      //if all threads send the the dv then send signal to receivers
			}


			if(send_count > 0)
				pthread_cond_wait(&sender_computing, &count_mutex);          //wait to calculator signal


			pthread_mutex_unlock(&count_mutex);

		}

		if(done)
			break;
	}

	return (void*)0;
}

//receive from all neighbors there's distance vector
void *receive_DV(void* receive){

	int receive_buf[((receiver*)receive)->n_routers +1];
	struct sockaddr_in serv_addr, cli_addr;

	socklen_t clilen;


	((receiver*)receive)->port = calculate_port(((receiver*)receive)->port, ((receiver*)receive)->neighbor);

	if((((receiver*)receive)->sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		printf("Error with create socket: %s\n", strerror(errno));
		pthread_exit(NULL);
	}


	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(((receiver*)receive)->addr);
	serv_addr.sin_port = htons(((receiver*)receive)->port);

	if((bind(((receiver*)receive)->sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr))) < 0){
		printf("Error with bind: %s\n", strerror(errno));
		pthread_exit(NULL);
	}

	listen(((receiver*)receive)->sock, 7);
	clilen = sizeof(cli_addr);
	((receiver*)receive)->newSock = accept(((receiver*)receive)->sock, (struct sockaddr *)&cli_addr, &clilen);

	if(((receiver*)receive)->newSock < 0){
		printf("Error with accept: %s\n", strerror(errno));
		pthread_exit(NULL);
	}


	while(1){

		if(!done){

			pthread_mutex_lock(&count_mutex);

			while(send_count < ((receiver*)receive)->n_neighbors)      //while not all sender threads send the dv -> wait
				pthread_cond_wait(&sender_receiver, &count_mutex);

			bzero(receive_buf, sizeof(receive_buf));

			if((read(((receiver*)receive)->newSock, (void*)&receive_buf, sizeof(receive_buf))) < 0){
				printf("Error could not read from socket: %s\n", strerror(errno));
				pthread_exit(NULL);
			}


			if(receive_buf[0] == 0)          //if receive 0
				zero++;


			else if(receive_buf[0] == 1)     //if receive distance vector
				update_neighbor_DV(((receiver*)receive)->neighbor, ((receiver*)receive)->neighbors, ((receiver*)receive)->n_neighbors, receive_buf);


			receive_count++;

			if(receive_count == ((receiver*)receive)->n_neighbors){
				pthread_cond_signal(&receiver_computing);              //if all threads receive distance vector or 0 wake the calculator
			}

			pthread_cond_wait(&receiver_computing ,&count_mutex);

			pthread_mutex_unlock(&count_mutex);
		}

		if(done)
			break;
	}

	return (void*)0;
}

//calculate the portNum + name - sum of ascii and return the result
int calculate_port(int portNum, char* name){

	int port = portNum;
	int i = 0;

	while(i < strlen(name)){
		port += (int)name[i];
		i++;
	}

	return port;
}

//create for calculate and for each neighbor sender and receiver threads
void create_threads(router* router, int num_of_loops){

	int i, j;

	sender* senders_struct[router->n_neighbor];
	receiver* receivers_struct[router->n_neighbor];
	pthread_t senders[router->n_neighbor];
	pthread_t receivers[router->n_neighbor];
	pthread_t computing;
	loops = num_of_loops;

	done = FALSE;


	initialize_condVar();

	pthread_create(&computing, NULL, (void*)Relax, (void*)router);

	for(i = 0; i < router->n_neighbor; i++){
		receivers_struct[i] = createReceiverStruct(router, router->neighbors[i]->name, router->neighbors);
		pthread_create(&receivers[i], NULL, receive_DV, (void*)receivers_struct[i]);
		senders_struct[i] = createSenderStruct(router, router->neighbors[i]);
		pthread_create(&senders[i], NULL, send_DV, (void*)senders_struct[i]);
	}

	/*********wait for all threads*********/

	for(j = 0; j < router->n_neighbor; j++){
		pthread_join(receivers[j], NULL);
		pthread_join(senders[j], NULL);
		if(j == 0)
			pthread_join(computing, NULL);
	}


	destroy_condVar();

	/***********close all sockets***********/

	for(i = 0; i < router->n_neighbor; i++){
		close(senders_struct[i]->sock);
		close(receivers_struct[i]->sock);
		close(receivers_struct[i]->newSock);
	}


	/*********free sender and receiver structures**********/

	for(i = 0; i < router->n_neighbor; i++){

		free(receivers_struct[i]);
		free(senders_struct[i]);
	}
}

//update the distance vector of the specific neighbor - name
void update_neighbor_DV(char* name, neighbor** neighbors, int n_neighbors, int buf[]){

	int index, i = 0, j = 1;
	DV* tmpDv;

	index = is_neighbor(neighbors, name, n_neighbors);

	while((tmpDv = neighbors[index]->dv[i]) != NULL){
		neighbors[index]->dv[i]->estimate = buf[j];
		j++;
		i++;
	}
}

//update the distance vector with the bellman-ford equation
void *Relax(void* rou){

	int i, j;
	int change;


	while(1){

		pthread_mutex_lock(&count_mutex);

		while(receive_count < ((router*)rou)->n_neighbor)
			pthread_cond_wait(&receiver_computing, &count_mutex);        //if not all receiver threads are receive dv or 0 -> wait


		if(zero == ((router*)rou)->n_neighbor){
			done = TRUE;
			break;
		}


		else
			zero = 0;

		change = FALSE;

		/*********************************Relax the distance vector************************************/

		for(i = 0; i < ((router*)rou)->n_routers; i++){

			for(j = 0; j < ((router*)rou)->n_neighbor; j++){

				if(((router*)rou)->dv[i]->estimate > ((router*)rou)->neighbors[j]->dv[i]->estimate + ((router*)rou)->neighbors[j]->cost){
					((router*)rou)->dv[i]->estimate = ((router*)rou)->neighbors[j]->dv[i]->estimate + ((router*)rou)->neighbors[j]->cost;
					((router*)rou)->via[i]->via_router = (char*)realloc(((router*)rou)->via[i]->via_router, strlen((((router*)rou)->neighbors[j]->name)) +1);
					strcpy(((router*)rou)->via[i]->via_router, ((router*)rou)->neighbors[j]->name);
					change = TRUE;
				}
			}
		}

		if(change == TRUE)
			*(((router*)rou)->change) = TRUE;
		else
			*(((router*)rou)->change) = FALSE;


		for(i = 0; i < receive_count; i++)
			pthread_cond_signal(&receiver_computing);    //wake up receiver threads

		for(i = 0; i < send_count; i++)
			pthread_cond_signal(&sender_computing);      //wake up sender threads


		receive_count = 0;
		send_count = 0;


		pthread_mutex_unlock(&count_mutex);
	}

	for(i = 0; i < receive_count; i++)
		pthread_cond_signal(&receiver_computing);        //wake up receiver threads if done

	for(i = 0; i < send_count; i++){
		pthread_cond_signal(&sender_computing);          //wake up sender threads if done
	}

	send_count = 0;

	pthread_mutex_unlock(&count_mutex);


	return (void*)0;
}
