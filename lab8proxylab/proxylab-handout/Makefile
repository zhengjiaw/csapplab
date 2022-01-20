# Makefile for Proxy Lab 
#
# You may modify this file any way you like (except for the handin
# rule). You instructor will type "make" on your specific Makefile to
# build your proxy from sources.

CC = gcc
CFLAGS = -g -Wall 
LDFLAGS = -lpthread
SOURCES    = $(wildcard *.c)
OBJECTS = $(SOURCES:.c=.o)
all: proxy 

csapp.o: csapp.c csapp.h 
	$(CC) $(CFLAGS) -c csapp.c

proxy.o: proxy.c cache.h request.h csapp.h thread_run.h sbuf.h
	$(CC) $(CFLAGS) -c proxy.c
thread_run.o:thread_run.c thread_run.h sbuf.h csapp.h 
	$(CC) $(CFLAGS) -c thread_run.c 
sbuf.o:sbuf.c sbuf.h
	$(CC) $(CFLAGS) -c sbuf.c
requset.o: requset.c csapp.h request.h
	$(CC) $(CFLAGS) -c  requset.c
cache.o: cache.c cache.h request.h csapp.h wr.h
	$(CC) $(CFLAGS) -c  cache.c
wr.o: wr.c wr.h csapp.h
	$(CC) $(CFLAGS) -c  wr.c

proxy:$(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o proxy $(LDFLAGS)
# Creates a tarball in ../proxylab-handin.tar that you can then
# hand in. DO NOT MODIFY THIS!
handin:
	(make clean; cd ..; tar cvf $(USER)-proxylab-handin.tar proxylab-handout --exclude tiny --exclude nop-server.py --exclude proxy --exclude driver.sh --exclude port-for-user.pl --exclude free-port.sh --exclude ".*")

clean:
	rm -f *~ *.o proxy core *.tar *.zip *.gzip *.bzip *.gz

