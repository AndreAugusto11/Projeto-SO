#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "../lib/commandlinereader.h"
#include <sys/wait.h>
#include "errno.h"
#include "assert.h"

#define bufferSize 256
#define numberMaxArgs 5

typedef struct process{
	pid_t pid;
	int status;
} Process;


int main(int argc, char const *argv[])
{
	long MAXCHILDREN = -1;
	int i = 0, status = 0;
	int count = 1;						/* Total number of processes */
	int numberProcesses = 0;			/* Number of active child processes */
	long vectorCapacity = 10;			/* Vector Capacity */
	pid_t pid = 0;
	char line[bufferSize], *args[numberMaxArgs];

	Process *vectorProcess = (struct process*)malloc(sizeof(Process) * vectorCapacity);			/* Alloc memory for an array of Processes */
	assert(vectorProcess);

	if (argc == 2){
		if (argv[1] > 0)
			MAXCHILDREN = atoi(argv[1]);							/* Sets MAXCHILDREN to the argument read in the command line */

		else{
			free(vectorProcess);
			exit(1);
		}
	}

	else if (argc > 2){
		free(vectorProcess);
		exit(1);
	}

	while (1){
		while (readLineArguments(args, numberMaxArgs, line, bufferSize) == 0){}
		
		if (!strcmp(args[0], "run")){
			if (args[1] == NULL)
				continue;

			if (count == vectorCapacity - 1) {						/* If the array is almost full it's time to realoc to a new size */
				vectorCapacity *= 2;
				vectorProcess = realloc(vectorProcess, sizeof(Process*) * vectorCapacity);
				assert(vectorProcess);
			}

			if (MAXCHILDREN != -1){									/* If there's a limit to the number of active processes waits for a process to end */
				if (numberProcesses == MAXCHILDREN){
					pid = wait(&status);
					if ((pid == -1) && (errno == EINTR))
						while (errno == EINTR)
							pid = wait(&status);
					vectorProcess[i].pid = pid;
					vectorProcess[i++].status = status;
					numberProcesses--;
				}
			}

			pid = fork();											/* Creates a new child process */


			if (pid < 0){
				perror("fork");
				free(vectorProcess);
				exit(1);
			}

			if (pid == 0){
				execl("CircuitRouter-SeqSolver", "CircuitRouter-SeqSolver", args[1], NULL);
				perror("execl");
				free(vectorProcess);
				exit(1);
			}
			else {
				count++;
				numberProcesses++;
			}
		}

		if (!strcmp(args[0], "exit")){
			while (pid != -1){
				pid = wait(&status);							/* Waits for all the processes to terminate */
				if ((pid == -1) && (errno == EINTR))			/* Confirms if there was an error in wait() */
					continue;

				vectorProcess[i].pid = pid;						/* Add to the array the process id and status */
				vectorProcess[i++].status = status;
			}

			for(i = 0; vectorProcess[i].pid != -1; i++) {
				if (vectorProcess[i].status == 0)
					printf("CHILD EXITED (PID=%d; return OK)\n", vectorProcess[i].pid);
				else
					printf("CHILD EXITED (PID=%d; return NOK)\n", vectorProcess[i].pid);
			}

			free(vectorProcess);
			exit(0);
		}
	}

	return 0;
}