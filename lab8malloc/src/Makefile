#
# Makefile for the malloc lab driver
#
CC = gcc
CFLAGS = -Wall -O2 -m32 -g3

OBJS = mdriver.o mm.o memlib.o fsecs.o fcyc.o clock.o ftimer.o

all: mdriver checkalign

mdriver: $(OBJS)
	$(CC) $(CFLAGS) -o mdriver $(OBJS)

mdriver.o: mdriver.c fsecs.h fcyc.h clock.h memlib.h config.h mm.h
memlib.o: memlib.c memlib.h
mm.o: mm.c mm.h memlib.h
fsecs.o: fsecs.c fsecs.h config.h
fcyc.o: fcyc.c fcyc.h
ftimer.o: ftimer.c ftimer.h config.h
clock.o: clock.c clock.h

# Make it easy to switch between different malloc solution versions
naive: # version handed out to students
	rm -f mm.c mm.o; ln -s mm-naive.c mm.c

implicit: # implicit list version from text
	rm -f mm.c mm.o; ln -s mm-implicit.c mm.c

explicit: # explicit list allocator
	rm -f mm.c mm.o; ln -s mm-explicit.c mm.c

tree: # highly optimized red-black tree allocator
	rm -f mm.c mm.o; ln -s mm-tree.c mm.c

test: # buggy version with overlapping blocks
	rm -f mm.c mm.o; ln -s mm-test.c mm.c

clean:
	rm -f *~ *.o mdriver checkalign


