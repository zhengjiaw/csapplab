CC = gcc
CFLAGS   =
 
EXECUTABLE = tsh
SOURCES    = $(wildcard *.c)
OBJECTS = $(SOURCES:.c=.o)
INCLUDE    = 
release: CFLAGS += -O3 -DNDEBUG 
release: $(EXECUTABLE)      

debug: CFLAGS += -g3 -DDEBUG 
debug: $(EXECUTABLE)
	

all: release debug 
$(EXECUTABLE): $(OBJECTS)
ifeq ($(EXECUTABLE), executable)
	@echo Edit EXECUTABLE variable in Makefile.
	@echo Using default a.out.
	$(CC) $(CFLAGS) $(OBJECTS) 
else
	$(CC) $(CFLAGS) $(OBJECTS)  -o $(EXECUTABLE) 
endif

.PHONY : clean 
clean : 
	-rm -f $(OBJECTS) $(EXECUTABLE) $(EXECUTABLE)_debug


base.o: base.c base.h
get_num.o: get_num.c get_num.h
jobs.o: jobs.c jobs.h base.h get_num.h sio.h
signalHandle.o: signalHandle.c signalHandle.h base.h jobs.h get_num.h \
 sio.h
