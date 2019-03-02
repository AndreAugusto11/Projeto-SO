#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#define bufferSize 256
#define numberMaxArgs 5

/*#define SERVER_PATH "/tmp/so_base/CircuitRouter-SeqSolver.pipe"*/
#define TEMP "/tmp/so_base/pipes/pipeXXXXXX"

int main(int argc, char const *argv[])
{
	char buffer[bufferSize], name[bufferSize], server_path[bufferSize];
	int fserver, fclient;

	strcpy(server_path, argv[1]);

	if ((fserver = open(server_path, O_WRONLY)) < 0){
		perror("open");
		exit(-1);
	}

	strcpy(name,TEMP);
	mkstemp(name);	
	unlink(name);
	strcat(name,".pipe");
	mkfifo(name, 0777);


	while (1){

		memset(buffer, 0, bufferSize);

		if (fgets(buffer, bufferSize, stdin) == NULL) {
    		continue;
  		}

  		strcat(buffer, "#");
  		strcat(buffer, name);

		if (write(fserver, buffer, strlen(buffer)+1) < 0)
			perror("write");

		if ((fclient = open(name, O_RDONLY)) < 0){
			perror("open");
			exit(-1);
		}

		memset(buffer, 0, bufferSize);

		if (read(fclient, buffer, bufferSize) < 0)
			perror("read");
		

		printf("%s\n", buffer);
	}

	unlink(name);
	return 0;
}