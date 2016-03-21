/*
 * main.c
 *
 *	Program: Router implication with distance vector algorithm
 *	File: main
 *
 *  Created on: Jan 8, 2015
 *      Author: Gili Mizrahi  302976840
 */

#include <errno.h>
#include "router.h"
#include "dvAlg.h"


int main(int argc, char** argv){

	FILE* file;
	router* router;


	if(argc != 4){
		printf("To run the program enter: %s file_path router_name num_of_loops\n", argv[0]);
		return 1;
	}

	if((file = fopen(argv[1], "r")) == NULL){
		printf("%s: %s\n", argv[1], strerror(errno));
		return 1;
	}

	if((router = initialize_router(file, argv[2])) == NULL){
		printf("%s\n", strerror(errno));
		return 1;
	}

	fclose(file);

	create_threads(router, atoi(argv[3]));

	print_Table(router);

	freeRouter(router);


	return 0;
}
