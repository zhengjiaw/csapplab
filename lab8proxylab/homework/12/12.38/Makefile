CC = gcc
CFLAGS = -O2 -Wall -I .

# This flag includes the Pthreads library on a Linux box.
# Others systems will probably require something different.
LIB = -lpthread

all: tiny cgi 

tiny: tiny.c csapp.o thread_run.o
	$(CC) $(CFLAGS) -o tiny tiny.c csapp.o sbuf.o thread_run.o $(LIB)

csapp.o: csapp.c
	$(CC) $(CFLAGS) -c csapp.c
thread_run.o:thread_run.c thread_run.h sbuf.o
	$(CC) $(CFLAGS) -c thread_run.c
sbuf.o:sbuf.c sbuf.h
	$(CC) $(CFLAGS) -c sbuf.c

cgi:
	(cd cgi-bin; make)

clean:
	rm -f *.o tiny *~
	(cd cgi-bin; make clean)

