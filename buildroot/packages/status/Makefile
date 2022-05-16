CC=gcc

all: *.c
	$(CC) -I$(PWD)/package/pd690xx -I$(PWD)/package/postmerkos -Os status.c -o status -ljson-c
clean:
	rm -rf a.out status
	rm -f *.o
