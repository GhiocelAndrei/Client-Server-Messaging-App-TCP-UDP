CC = g++
CFLAGS = -Wall

# Port for the server
PORT = 12347

# Server's IP
IP = 127.0.0.1

all: server subscriber

server: server.cpp Server_utils.cpp
	$(CC) $(CFLAGS) $^ -o $@

subscriber: subscriber.cpp Client_utils.cpp
	$(CC) $(CFLAGS) $^ -o $@

.PHONY: clean run_server run_client

run_server: server
	./server $(PORT)

run_subscriber: subscriber
	./subscriber $(ID) $(IP) $(PORT)

# Command: make run_subscriber ID="..."

clean:
	$(RM) server subscriber