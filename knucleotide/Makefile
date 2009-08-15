# ex: set ts=8 noet:

CFLAGS = -W -Wall -std=c99 -pedantic -fopenmp -m32 -Os -march=core2 -mtune=core2 -g
LDFLAGS = -lgomp -m32

kn: kn.o ht.o

clean:
	$(RM) kn *.o

