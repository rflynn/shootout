
#include <stdio.h>
#include <stdlib.h>

#define LINEBUF 82

static const unsigned char XLate[256] = {
  ['A'] = 'T', ['B'] = 'V', ['C'] = 'G',
  ['D'] = 'H', ['G'] = 'C', ['H'] = 'D',
  ['K'] = 'M', ['L'] = 'K', ['N'] = 'N',
  ['R'] = 'Y', ['S'] = 'S', ['T'] = 'A',
  ['V'] = 'B', ['W'] = 'W', ['Y'] = 'R',
  ['\r'] = '\r', ['\n'] = '\n', [' '] = ' '
};

static inline void rev(const unsigned char *line)
{
  while (*line)
    fputc(XLate[*line++], stdout);
}

/* read line-by-line a redirected FASTA format file from stdin
 * extract DNA sequence THREE */
static void rev_cmp(void)
{
  unsigned char line[LINEBUF];
  while (NULL != fgets(line, sizeof line, stdin))
    if ('>' == *line)
      break;
  fputs(line, stdout);
  while (NULL != fgets(line, sizeof line, stdin)) {
    if ('>' == *line)
      break;
    rev(line);
  }
}

int main(void)
{
  rev_cmp();
  return 0; 
}
