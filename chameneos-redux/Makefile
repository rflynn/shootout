# ex: set ts=8 noet:

CFLAGS = -W -Wall -std=c99 -pedantic -m32 -Os
LDFLAGS = -m32 -lpthread

test: cr
	time ./cr

cr: cr.o

compete: competition
	time ./competition

competition: competition.o

clean:
	$(RM) cr *.o

