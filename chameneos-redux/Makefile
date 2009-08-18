# ex: set ts=8 noet:

CFLAGS = -W -Wall -std=c99 -pedantic -m32 -Os -DNDEBUG
LDFLAGS = -m32

test: c-r
	time ./c-r 6000

c-r: c-r.o

clean:
	$(RM) c-r *.o

