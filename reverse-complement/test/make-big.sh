#!/bin/sh
# multiply the normal input file to produce a larger test file

rm -f tmp
rm -f revcomp-big-input.txt
cat revcomp-input.txt >> revcomp-big-input.txt

for n in $(seq 1 8)
do
  cat revcomp-big-input.txt >> tmp
  cat tmp >> revcomp-big-input.txt
done

rm -f tmp

