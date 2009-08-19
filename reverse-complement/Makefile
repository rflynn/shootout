# ex: set ts=8 noet:

CFLAGS = -W -Wall -std=c99 -pedantic -m32 -Os -DNDEBUG
LDFLAGS = -m32

test: rc
	time ./rc < test/revcomp-input.txt

rc: rc.o

compete: competition
	time ./competition > compete.out

competition: competition.o

clean:
	$(RM) rc *.o

