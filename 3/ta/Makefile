INCLUDE_DIRS = 
LIB_DIRS = 
CC=gcc

CDEFS=
CFLAGS= -O3 $(INCLUDE_DIRS) $(CDEFS)
LIBS= 

HFILES= 
CFILES= pthread_128.c

SRCS= ${HFILES} ${CFILES}
OBJS= ${CFILES:.c=.o}

all:	pthread_128

clean:
	-rm -f *.o *.d
	-rm -f perfmon pthread_128

distclean:
	-rm -f *.o *.d
	-rm -f pthread_128

pthread: pthread_128.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $@.o -lpthread_128

depend:

.c.o:
	$(CC) $(CFLAGS) -c $<
