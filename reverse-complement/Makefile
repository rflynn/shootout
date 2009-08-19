# ex: set ts=8 noet:

CFLAGS = -W -Wall -std=c99 -pedantic -fopenmp -m32 -O3
LDFLAGS = -m32 -lgomp

correctness: rc
	./rc < test/revcomp-tiny-input.txt > out
	diff -u out test/revcomp-tiny-output.txt

speed: competition rc
	time ./big-test.sh | ./competition > /dev/null
	time ./big-test.sh | ./competition > /dev/null
	time ./big-test.sh | ./competition > /dev/null
	time ./big-test.sh | ./rc > /dev/null
	time ./big-test.sh | ./rc > /dev/null
	time ./big-test.sh | ./rc > /dev/null

rc: rc.o

competition: competition.o

clean:
	$(RM) rc *.o

