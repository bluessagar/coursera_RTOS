
INCLUDE_DIRS = 
LIB_DIRS = 
CC=gcc

CDEFS=
CFLAGS= -O0 -g $(INCLUDE_DIRS) $(CDEFS)
LIBS= 

HFILES= 
CFILES= pthread31-2.c

SRCS= ${HFILES} ${CFILES}
OBJS= ${CFILES:.c=.o}

all:	pthread31-2

clean:
	-rm -f *.o *.d
	-rm -f pthread31-2

pthread: pthread31-2.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $@.o -lpthread -g -O0

depend:

.c.o:
	$(CC) $(CFLAGS) -c $<
