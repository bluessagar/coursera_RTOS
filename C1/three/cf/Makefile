INCLUDE_DIRS = 
LIB_DIRS = 
CC=gcc

CDEFS=
CFLAGS= -O0 -g $(INCLUDE_DIRS) $(CDEFS)
LIBS= 

HFILES= 
CFILES= pthread31.c

SRCS= ${HFILES} ${CFILES}
OBJS= ${CFILES:.c=.o}

all:	pthread31

clean:
	-rm -f *.o *.d
	-rm -f pthread31

pthread: pthread31.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $@.o -lpthread -g -O0

depend:

.c.o:
	$(CC) $(CFLAGS) -c $<
