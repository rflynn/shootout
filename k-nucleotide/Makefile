# ex: set ts=8 noet:

CFLAGS = -W -Wall -std=c99 -pedantic -fopenmp -m32 -Os -DNDEBUG
LDFLAGS = -lgomp -m32

test: kn testbig
	time ./kn < test/big

kn: kn.o
kn.o: ht.h

testbig:
	@chmod +x rand-dna.pl
	@if [ ! -e test/big ]; then ./rand-dna.pl > test/big; fi

clean:
	$(RM) kn *.o

