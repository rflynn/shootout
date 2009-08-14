#!/usr/bin/perl -w
# generate minimal, large valid FASTA file for knucleotide benchmark
print ">THREE\n";
for (0..3*1024*1024) {
  print qw/A C G T/[rand 4] for 0..78;
  print "\n";
}
