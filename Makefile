
CC=gcc

CFLAGS=-O3 -Wall -g -pg

LIBS=-lm -lrados

all : rados_xdr show-stream

clean :
	-rm *.o rados_xdr

rados_xdr : rados_xdr.o
	$(CC) -o $@ $(CFLAGS) $^ $(LDFLAGS) $(LIBS)

show-stream: show-stream.o
	$(CC) -o $@ $(CFLAGS) $^ $(LDFLAGS) $(LIBS)
