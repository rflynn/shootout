# ex: set ts=8 noet:

CC = gcc
CFLAGS = -W -Wall -std=c99 -fopenmp -m32 -Os -fomit-frame-pointer -march=core2 -mtune=core2
LDFLAGS = -lgomp -m32

kn: kn.o ht.o

clean:
	$(RM) kn *.o

