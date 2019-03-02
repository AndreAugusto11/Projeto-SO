# Makefile CircuitRouterClient

CFLAGS =-Wall -g -std=gnu99 -I../

CircuitRouter-Client: CircuitRouter-Client.o ../lib/parseArgs.o
	gcc $(CFLAGS) CircuitRouter-Client.o ../lib/parseArgs.o -o CircuitRouter-Client

clean:
	rm -f CircuitRouter-Client CircuitRouter-Client.o

CircuitRouter-Client.o: CircuitRouter-Client.c ../lib/parseArgs.h
	gcc $(CFLAGS) -c CircuitRouter-Client.c -o CircuitRouter-Client.o

../lib/parseArgs.o: ../lib/parseArgs.c ../lib/parseArgs.h
	gcc $(CFLAGS) -c ../lib/parseArgs.c -o ../lib/parseArgs.o