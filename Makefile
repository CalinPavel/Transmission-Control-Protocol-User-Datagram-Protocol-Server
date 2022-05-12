CFLAGS = -Wall -g
CC = g++

PORT = 
IP = 

all: server subscriber

server: server.cpp
	$(CC) $(CFLAGS) -o server server.cpp

subscriber: subscriber.cpp
	$(CC) $(CFLAGS) -o subscriber subscriber.cpp

.PHONY: clean run_server run_client

run_server: server	
	./server ${PORT}

run_subscriber: subscriber
	./subscriber ${IP} ${PORT}

clean:
	-rm -f server
	-rm -f subscriber
