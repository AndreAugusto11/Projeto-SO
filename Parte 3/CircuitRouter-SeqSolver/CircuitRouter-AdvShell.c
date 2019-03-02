#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <signal.h>
#include "../lib/parseArgs.h"
#include <sys/wait.h>
#include "lib/timer.h"
#include "errno.h"
#include "assert.h"
#include <fcntl.h>

#define bufferSize 256
#define numberMaxArgs 5

#define SERVER_PATH "/tmp/so_base/CircuitRouter-SeqSolver/CircuitRouter-SeqSolver.pipe"

typedef struct process{
	pid_t pid;
	int status;
	TIMER_T startTime;
	TIMER_T stopTime;
} Process;

Process *vectorProcess;
int iter = 0;

void sigchildHandler();


int main(int argc, char const *argv[])
{
	long MAXCHILDREN = -1;
	int i = 0, status = 0;
	int count = 1;													/* Total number of processes */
	int numberProcesses = 0;										/* Number of active child processes */
	long vectorCapacity = 1000;										/* Vector Capacity */
	pid_t pid = 0;
	char buffer[bufferSize], *args[numberMaxArgs], pipeOut[256];
	int fserver, fout = 1;
	fd_set mask, testmask;

	sigset_t sigSet;
	sigemptyset(&sigSet);
	sigaddset(&sigSet, SIGCHLD);

	unlink(SERVER_PATH);
	vectorProcess = (struct process*)malloc(sizeof(Process) * vectorCapacity);			/* Alloc memory for an array of Processes */
	assert(vectorProcess);

	struct sigaction act;

	act.sa_handler = sigchildHandler;
	act.sa_mask = sigSet;
	act.sa_flags = SA_NOCLDSTOP | SA_RESTART;

	if (sigaction(SIGCHLD, &act, NULL) < 0){
		perror("sigaction");
		exit(1);
	}

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

	/* Creates the server's pipe */
	if (mkfifo(SERVER_PATH, 0777) < 0){
		perror("mkfifo");
		exit(1);
	}

	/* Opens the server's pipe */
	if ((fserver = open(SERVER_PATH, O_RDONLY)) < 0){
		perror("open");
		exit(1);
	}


	while (1){

		memset(buffer, 0, sizeof(buffer));

		mask = testmask;

		FD_ZERO(&mask);
		FD_SET(0, &mask);
		FD_SET(fserver, &mask);

		/* Waiting for input from the stdin or from the pipe */
		if (select(fserver+1, &mask, 0, 0, 0) == -1){
			if (errno == EINTR)
				continue;

			perror("select");
			exit(1);
		}

		if (FD_ISSET(0, &mask)){
			/* Input from stdin */

			fout = 1;

			if (fgets(buffer, bufferSize, stdin) == NULL) {
    			return -1;
  			}

  			if (!strcmp(buffer, "\n"))
  				continue;

  			/* Parsing the input */
  			if (parseArgs(args, numberMaxArgs, buffer, bufferSize) == 0)
				perror("parseArgs");
		}

		else if(FD_ISSET(fserver, &mask)){
			/* Input from the pipe */
			if (read(fserver, buffer, bufferSize) < 0)
				perror("read");
			
			/* Parsing the input */
			if (parseArgs(args, numberMaxArgs, buffer, bufferSize) == 0)
				perror("parseArgs");
			
			i = 0;

			/* Checking pipe's name position */
			while(args[i] != NULL){i++;}

			strcpy(pipeOut, args[i-1]);

			if (strcmp(args[0], "run") != 0){
				/* Input received from the client is not supported */
				if ((fout = open(pipeOut, O_WRONLY)) < 0){
					perror("open");
					exit(-1);
				}

				if (write(fout, "Command not supported", 30) < 0)
					perror("write");

				if (close(fout) < 0)
					perror("close");

				continue;
			}
  			
			/* Opens the client pipe */
			if ((fout = open(pipeOut, O_WRONLY)) < 0){
				perror("open");
				exit(1);
			}
		}

		if (!strcmp(args[0], "run")){
			if (args[1] == NULL)
				continue;

			if (count == vectorCapacity - 1) {						/* If the array is almost full it's time to realoc to a new size */
				vectorCapacity *= 2;
				vectorProcess = realloc(vectorProcess, sizeof(Process*) * vectorCapacity);
				assert(vectorProcess);
			}

			if (MAXCHILDREN != -1){									/* If there's a limit to the number of active processes, waits for a process to end */
				if (numberProcesses == MAXCHILDREN){
					/* Blocking any signals */
					sigprocmask(SIG_BLOCK, &sigSet, NULL);
					pid = wait(&status);
					if ((pid == -1) && (errno == EINTR))
						while (errno == EINTR)
							pid = wait(&status);
					
					vectorProcess[iter].pid = pid;
					vectorProcess[iter].status = status;
					TIMER_READ(vectorProcess[iter++].stopTime);
					sigprocmask(SIG_UNBLOCK, &sigSet, NULL);
					/* Unblocking signals */
				}
			}

			pid = fork();											/* Creates a new child process */

			if (pid < 0){
				perror("fork");
				free(vectorProcess);
				exit(1);
			}

			if (pid == 0){
				if (fout != 1){
					char seqArgPipe[100];
					char fileDescriptorChar[10];
					sprintf(fileDescriptorChar, "%d", fout);

					strcat(seqArgPipe, "-f ");
					strcat(seqArgPipe, fileDescriptorChar);
					/* Runs CircuitRouter-SeqSolver with the filename and a flag passing the file descriptor that correspond to the client's pipe */
					execl("CircuitRouter-SeqSolver", "CircuitRouter-SeqSolver", args[1], seqArgPipe, NULL);
				}
				else{
					/* Runs CircuitRouter-SeqSolver only with the filename*/
					execl("CircuitRouter-SeqSolver", "CircuitRouter-SeqSolver", args[1], NULL);
				}
				

				perror("execl");
				free(vectorProcess);
				exit(1);
			}

			else {
				/* Reads the time */
				vectorProcess[iter].pid = pid;
				TIMER_READ(vectorProcess[iter++].startTime);

				/* Closes the client's pipe */
				if (fout != 1){
					if (close(fout) < 0)
						perror("close");
				}

				count++;
				numberProcesses++;
			}
		}

		if (!strcmp(args[0], "exit")){
			while (pid != -1){
				pid = wait(&status);								/* Waits for all the processes to terminate one by one */
				if ((pid == -1) && (errno == EINTR))				/* Confirms if there was an error in wait() */
					continue;
			}

			/* Printing out the pid's of the childs, their status and the time they were alive */
			for(i = 0; i < iter; i++) {
				if (vectorProcess[i].pid != 0){
					if (vectorProcess[i].status == 0){
						printf("CHILD EXITED (PID=%d; return OK; %f s)\n", vectorProcess[i].pid, TIMER_DIFF_SECONDS(vectorProcess[i].startTime, vectorProcess[i].stopTime));
					}
					else
						printf("CHILD EXITED (PID=%d; return NOK; %f s)\n", vectorProcess[i].pid, TIMER_DIFF_SECONDS(vectorProcess[i].startTime, vectorProcess[i].stopTime));
				}
			}
			free(vectorProcess);
			exit(0);
		}
	}

	unlink(SERVER_PATH);
	return 0;
}

void sigchildHandler(){
	int i, status, pid = 1;

	pid = waitpid(-1, &status, 0);
	
	if ((pid == -1) && (errno != ECHILD)){
		perror("waitpid");
		exit(1);
	}

	for (i = 0; i < iter; i++){
		if (pid == vectorProcess[i].pid){
			/* Saves the current time */
			TIMER_READ(vectorProcess[i].stopTime);
			vectorProcess[i].status = status;
			break;
		}
	}
}