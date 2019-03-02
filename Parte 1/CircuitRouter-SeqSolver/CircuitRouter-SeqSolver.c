/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * This code is an adaptation of the Lee algorithm's implementation originally included in the STAMP Benchmark
 * by Stanford University.
 *
 * The original copyright notice is included below.
 *
  * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) Stanford University, 2006.  All Rights Reserved.
 * Author: Chi Cao Minh
 *
 * =============================================================================
 *
 * Unless otherwise noted, the following license applies to STAMP files:
 *
 * Copyright (c) 2007, Stanford University
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *
 *     * Neither the name of Stanford University nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY STANFORD UNIVERSITY ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL STANFORD UNIVERSITY BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 * =============================================================================
 *
 * CircuitRouter-SeqSolver.c
 *
 * =============================================================================
 */


#include <assert.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lib/list.h"
#include "maze.h"
#include "router.h"
#include "lib/timer.h"
#include "lib/types.h"

enum param_types {
	PARAM_BENDCOST = (unsigned char)'b',
	PARAM_XCOST    = (unsigned char)'x',
	PARAM_YCOST    = (unsigned char)'y',
	PARAM_ZCOST    = (unsigned char)'z',
};

enum param_defaults {
	PARAM_DEFAULT_BENDCOST = 1,
	PARAM_DEFAULT_XCOST    = 1,
	PARAM_DEFAULT_YCOST    = 1,
	PARAM_DEFAULT_ZCOST    = 2,
};

bool_t global_doPrint = FALSE;
char* global_inputFile = NULL;
long global_params[256]; /* 256 = ascii limit */


/* =============================================================================
 * displayUsage
 * =============================================================================
 */
static void displayUsage (const char* appName){
	printf("Usage: %s [options]\n", appName);
	puts("\nOptions:                            (defaults)\n");
	printf("    b <INT>    [b]end cost          (%i)\n", PARAM_DEFAULT_BENDCOST);
	printf("    x <UINT>   [x] movement cost    (%i)\n", PARAM_DEFAULT_XCOST);
	printf("    y <UINT>   [y] movement cost    (%i)\n", PARAM_DEFAULT_YCOST);
	printf("    z <UINT>   [z] movement cost    (%i)\n", PARAM_DEFAULT_ZCOST);
	printf("    h          [h]elp message       (false)\n");
	exit(1);
}


/* =============================================================================
 * setDefaultParams
 * =============================================================================
 */
static void setDefaultParams (){
	global_params[PARAM_BENDCOST] = PARAM_DEFAULT_BENDCOST;
	global_params[PARAM_XCOST]    = PARAM_DEFAULT_XCOST;
	global_params[PARAM_YCOST]    = PARAM_DEFAULT_YCOST;
	global_params[PARAM_ZCOST]    = PARAM_DEFAULT_ZCOST;
}


/* =============================================================================
 * doesFileExist
 * =============================================================================
 */
int doesFileExist(char *fileName){
	/* Returns 1 if there's a file it fileName, else returns 0 */

	FILE *fp;

	if ((fp = fopen(fileName, "r+")) == NULL)
		return 0;
	else {
		fclose(fp);
		return 1;
	}   
}


/* =============================================================================
 * parseArgs
 * =============================================================================
 */
static void parseArgs (long argc, char* const argv[]){
	long i;
	long opt;
	int countsFiles = 0;

	opterr = 0;

	setDefaultParams();

	while ((opt = getopt(argc, argv, "hb:x:y:z:")) != -1) {
		switch (opt) {
			case 'b':
			case 'x':
			case 'y':
			case 'z':
				global_params[(unsigned char)opt] = atol(optarg);
				break;
			case '?':
			case 'h':
			default:
				opterr++;
				break;
		}
	}

	for (i = optind; i < argc; i++) {
		if (!doesFileExist(argv[i])){
			fprintf(stderr, "Non-option argument: %s\n", argv[i]);
			opterr++;
		}
		else
			countsFiles++;

	}
		if (countsFiles > 1){					/* If there's more than one file in the arguments read in the command line */
			fprintf(stderr, "Non-option argument: %s\n", argv[i]);
			opterr++;
		}

	if (opterr) {
		displayUsage(argv[0]);
	}
}


/* =============================================================================
 * openFiles
 * =============================================================================
 */
void openFiles(FILE **fpIn, FILE **fpOut, char** argv, int argc){
	/* Opens the input file and generates the output file */
	char fileName1[50] = "";
	char fileName2[50] = "";
	char fileNameAux[50] = "";
	int i = 0;
	int indexFile = 0;

	for (i = 0; i < argc; i++){
		if (doesFileExist(argv[i]))
			indexFile = i;					/* Checks the index of the file in argv */
	}


	if ((*fpIn = fopen(argv[indexFile], "r+")) == NULL) {				/* Invalid input file */
		perror(argv[indexFile]);
		exit(1);
	}	

	/* Generating the name of the output file */
	strcpy(fileName1, argv[indexFile]);
	strcat(fileName1, ".res");

	if (!doesFileExist(fileName1)) {
		*fpOut = fopen(fileName1, "w");
		assert(fpOut);
		if (*fpOut == NULL){
			perror(fileName1);
			exit(1);
		}
	}

	else {
		strcat(fileName1, ".old");
		if (!doesFileExist(fileName1)) {
			*fpOut = fopen(fileName1, "w");
			assert(fpOut);
			if (*fpOut == NULL){
				perror(fileName1);
				exit(1);
			}
		}
		
		else {
			/* If the output files already exist rename ".res" to ".res.old" and opens the ".res" as the output file */
			strcpy(fileName1, argv[indexFile]);
			strcpy(fileName2, argv[indexFile]);

			strcat(fileName1, ".res");
			strcat(fileName2, ".res.old");

			if (rename(fileName1, fileName2) != 0){
				perror("rename");
				exit(1);
			}

			strcpy(fileNameAux, argv[indexFile]);
			strcat(fileNameAux, ".res");

			*fpOut = fopen(fileNameAux, "w");
			assert(fpOut);
			if (*fpOut == NULL){
				perror(fileName1);
				exit(1);
			}
		}
	}


}

/* =============================================================================
 * main
 * =============================================================================
 */
int main(int argc, char** argv){
	/*
	 * Initialization
	 */
	FILE *fpIn = NULL;
	FILE *fpOut = NULL;


	/* The filename is required */
	if (argc == 1)
		exit(1);

	parseArgs(argc, (char** const)argv);

	openFiles(&fpIn, &fpOut, argv, argc);

	maze_t* mazePtr = maze_alloc();
	assert(mazePtr);
	long numPathToRoute = maze_read(mazePtr, fpIn, fpOut);
	router_t* routerPtr = router_alloc(global_params[PARAM_XCOST],
									   global_params[PARAM_YCOST],
									   global_params[PARAM_ZCOST],
									   global_params[PARAM_BENDCOST]);
	assert(routerPtr);
	list_t* pathVectorListPtr = list_alloc(NULL);
	assert(pathVectorListPtr);

	router_solve_arg_t routerArg = {routerPtr, mazePtr, pathVectorListPtr};
	TIMER_T startTime;
	TIMER_READ(startTime);

	router_solve((void *)&routerArg);

	TIMER_T stopTime;
	TIMER_READ(stopTime);

	long numPathRouted = 0;
	list_iter_t it;
	list_iter_reset(&it, pathVectorListPtr);
	while (list_iter_hasNext(&it, pathVectorListPtr)) {
		vector_t* pathVectorPtr = (vector_t*)list_iter_next(&it, pathVectorListPtr);
		numPathRouted += vector_getSize(pathVectorPtr);
	}
	fprintf(fpOut, "Paths routed    = %li\n", numPathRouted);
	fprintf(fpOut, "Elapsed time    = %f seconds\n", TIMER_DIFF_SECONDS(startTime, stopTime));


	/*
	 * Check solution and clean up
	 */
	assert(numPathRouted <= numPathToRoute);
	bool_t status = maze_checkPaths(mazePtr, pathVectorListPtr, fpOut);
	assert(status == TRUE);
	fputs("Verification passed.\n", fpOut);

	maze_free(mazePtr);
	router_free(routerPtr);

	list_iter_reset(&it, pathVectorListPtr);
	while (list_iter_hasNext(&it, pathVectorListPtr)) {
		vector_t* pathVectorPtr = (vector_t*)list_iter_next(&it, pathVectorListPtr);
		vector_t* v;
		while((v = vector_popBack(pathVectorPtr))) {
			// v stores pointers to longs stored elsewhere; no need to free them here
			vector_free(v);
		}
		vector_free(pathVectorPtr);
	}
	list_free(pathVectorListPtr);

	fflush(fpOut);
	fclose(fpOut);

	exit(0);
}


/* =============================================================================
 *
 * End of CircuitRouter-SeqSolver.c
 *
 * =============================================================================
 */
