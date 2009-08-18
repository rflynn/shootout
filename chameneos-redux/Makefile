# ex: set ts=8 noet:

CFLAGS = -W -Wall -std=c99 -pedantic -m32 -Os -DNDEBUG
LDFLAGS = -m32 -lpthread

test: c-r
	time ./c-r

c-r: c-r.o

compete: competition
	time ./competition

competition: competition.o

clean:
	$(RM) c-r *.o

