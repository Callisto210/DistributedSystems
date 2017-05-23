CC=gcc
CFLAGS=-Wall -Wextra -pedantic

default: client
	
client: client.c
	$(CC) $< -o $@ $(CFLAGS) -g
	
clean:
	rm server 
	rm client
