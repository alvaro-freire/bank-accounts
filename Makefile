CC=gcc
CFLAGS=-Wall -pthread -g

.SILENT:
all: copy ejercicios move

clear:
	for number in 1 2 3 4 ; do \
        rm ej$$number/* ; \
    done

ejercicios:
	for number in 1 2 3 4 ; do \
	$(CC) $(CFLAGS) ejercicio$$number.c options.c -o e$$number ; \
    done

move:
	for number in 1 2 3 4 ; do \
	mv e$$number ej$$number ; \
    done

copy:
	for number in 1 2 3 4 ; do \
	mkdir -p ej$$number ; \
	cp ejercicio$$number.c ej$$number ; \
    done
