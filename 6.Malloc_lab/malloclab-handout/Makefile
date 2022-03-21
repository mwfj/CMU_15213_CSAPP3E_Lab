#
# Students' Makefile for the Malloc Lab
#
TEAM = Wufangjie Ma
VERSION = 1
HANDINDIR = /home/mwfj/cmu-15-213-CSAPP3E-lab/6.Malloc_lab/malloclab-handout/

CC = gcc
CFLAGS = -Wall -O2 -m32

OBJS =  memlib.o fsecs.o fcyc.o clock.o ftimer.o

OBJS_IMPLICIT = mdriver.o mm_implicit_list.o $(OBJS)

OBJS_EXPLICIT = mdriver.o mm_explicit_list.o $(OBJS)

OBJ_SEGREGATED = mdriver.o mm.o $(OBJS)

# Segregated Free List(Default)
mdriver: $(OBJ_SEGREGATED)
	$(CC) $(CFLAGS) -o mdriver $(OBJ_SEGREGATED)
# Implicit Free List
implicit: $(OBJS_IMPLICIT)
	$(CC) $(CFLAGS) -o mdriver $(OBJS_IMPLICIT)
# Explicit Free List
explicit: $(OBJS_EXPLICIT)
	$(CC) $(CFLAGS) -o mdriver $(OBJS_EXPLICIT)

mdriver.o: mdriver.c fsecs.h fcyc.h clock.h memlib.h config.h mm.h
memlib.o: memlib.c memlib.h
mm_explicit_list.o: mm_explicit_list.c mm.h memlib.h
mm_implicit_list.o: mm_implicit_list.c mm.h memlib.h
mm.o: mm.c mm.h memlib.h
fsecs.o: fsecs.c fsecs.h config.h
fcyc.o: fcyc.c fcyc.h
ftimer.o: ftimer.c ftimer.h config.h
clock.o: clock.c clock.h

handin:
	cp mm.c $(HANDINDIR)/$(TEAM)-$(VERSION)-mm.c

clean:
	rm -f *~ *.o mdriver explicit