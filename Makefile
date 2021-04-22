CC=gcc
CFLAGS=-std=gnu99 -Wall -Wextra -pedantic -g
LDFLAGS=-lrt -lpthread

.PHONY: clean

proj2: proj2.o parse_args.o
	$(CC) $^ -o $@ $(LDFLAGS)

%.c: %.o

clean:
	rm -rf *.o 

