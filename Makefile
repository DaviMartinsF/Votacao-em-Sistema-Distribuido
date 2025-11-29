#Davi Martins Figueiredo - 1037487

CC = gcc
CFLAGS = -Wall -pthread

all: server client

server: server.c protocol.h
	$(CC) $(CFLAGS) -o server server.c

client: client.c protocol.h
	$(CC) $(CFLAGS) -o client client.c

clean:
	rm -f server client eleicao.log resultado_final.txt