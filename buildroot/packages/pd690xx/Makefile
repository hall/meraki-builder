CC=gcc
PWD=$(shell pwd)

all: *.c
	$(CC) -I$(PWD)/package/postmerkos -Os pd690xx.c -o pd690xx \
	$(PWD)/package/pd690xx/libpd690xx.c \
	$(PWD)/package/postmerkos/libpostmerkos.c
	$(CC) -c -Wall -Werror -fpic libpd690xx.c
	$(CC) -shared -o libpd690xx.so libpd690xx.o
	$(CC) -L$(PWD) -Wall -o pd690xx pd690xx.c -lpd690xx
clean:
	rm -rf a.out pd690xx libpd690xx.0 libpd690xx.so
	rm -f *.o
