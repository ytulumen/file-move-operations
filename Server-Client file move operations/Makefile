CC = gcc

.c.o:
	$(CC) -c $< 

all: Server Client

Server: Server.o restart.o -lm
	$(CC) -o Server Server.o restart.o -lm

Client: Client.o restart.o
	$(CC) -o Client Client.o restart.o -lm


clean:
	rm -f server *.o

	
