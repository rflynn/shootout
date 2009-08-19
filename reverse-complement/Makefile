# ex: set ts=8 noet:

CFLAGS = -W -Wall -std=c99 -pedantic -m32 -O3 -fomit-frame-pointer -mtune=native
LDFLAGS = -m32

correctness: rc
	./rc < test/revcomp-tiny-input.txt > out
	diff -u out test/revcomp-tiny-output.txt

speed: competition rc
	time ./competition < test/revcomp-big-input.txt > /dev/null
	time ./competition < test/revcomp-big-input.txt > /dev/null
	time ./competition < test/revcomp-big-input.txt > /dev/null
	time ./rc < test/revcomp-big-input.txt > /dev/null
	time ./rc < test/revcomp-big-input.txt > /dev/null
	time ./rc < test/revcomp-big-input.txt > /dev/null

rc: rc.o

competition: competition.o

clean:
	$(RM) rc *.o

