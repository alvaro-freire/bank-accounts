CC=gcc
CFLAGS=-Wall -pthread -g
LIBS=
OBJS=bank.o options.o
OBJS1=ejercicio1.c options.c
OBJS2=ejercicio1.c options.c
OBJS3=ejercicio1.c options.c
OBJS4=ejercicio1.c options.c

PROGS= bank 

all: $(PROGS)

.SILENT:
all2:
	for number in 1 2 3 4 ; do \
        make -s ejercicio$$number ; \
        make -s move$$number ; \
    done

clear:
	for number in 1 2 3 4 ; do \
        rm ej$$number/* ; \
    done

ejercicio1: copy1
	$(CC) $(CFLAGS) $(OBJS1) -o e1
ejercicio2: copy2
	$(CC) $(CFLAGS) $(OBJS2) -o e2
ejercicio3: copy3
	$(CC) $(CFLAGS) $(OBJS3) -o e3
ejercicio4: copy4
	$(CC) $(CFLAGS) $(OBJS4) -o e4

move1:
	mv e1 ej1
move2:
	mv e2 ej2
move3:
	mv e3 ej3
move4:
	mv e4 ej4

copy1:
	cp ejercicio1.c ej1
copy2:
	cp ejercicio1.c ej2
copy3:
	cp ejercicio1.c ej3
copy4:
	cp ejercicio1.c ej4


%.o : %.c
	$(CC) $(CFLAGS) -c $<

bank: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

clean:
	rm -f $(PROGS) *.o *~

