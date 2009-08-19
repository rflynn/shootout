
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFSZ   (1024 * 512UL)

struct buf {
  char  *str;
  size_t len, alloc;
};

static void buf_init(struct buf *b)
{
  b->len = 0;
  b->alloc = BUFSZ;
  b->str = malloc(b->alloc);
}

static void buf_grow(struct buf *b, size_t len)
{
  size_t newlen = b->len + len + BUFSZ;
  if (newlen > b->alloc) {
    b->alloc = newlen * 2;
    b->str = realloc(b->str, b->alloc);
    if (!b->str)
      perror("realloc"), exit(1);
  }
}

static inline void revcomp(struct buf *b)
{
  static const unsigned char Rev[256] = {
    ['A'] = 'T', ['a'] = 'T',
    ['B'] = 'V', ['b'] = 'V', 
    ['C'] = 'G', ['c'] = 'G',
    ['D'] = 'H', ['d'] = 'H', 
    ['G'] = 'C', ['g'] = 'C', 
    ['H'] = 'D', ['h'] = 'D',
    ['K'] = 'M', ['k'] = 'M', 
    ['L'] = 'K', ['l'] = 'K', 
    ['N'] = 'N', ['n'] = 'N',
    ['R'] = 'Y', ['r'] = 'Y', 
    ['S'] = 'S', ['s'] = 'S', 
    ['T'] = 'A', ['t'] = 'A',
    ['V'] = 'B', ['v'] = 'B', 
    ['W'] = 'W', ['w'] = 'W', 
    ['Y'] = 'R', ['y'] = 'R'
  };
  size_t i = 0,
         len = b->len,
         halfway = len / 2;
  char *c = b->str;
  while (i <= halfway) {
    c[i] = c[len-1-i];
    c[len-1-i] = c[i];
    i++;
  }
}

static void dumplines(struct buf *b)
{
  const char *curr = b->str;
  size_t left = b->len;
  //revcomp(b);
  while (left >= 60) {
    (void)write(1, curr, 60);
    fputc('\n', stdout);
    curr += 60, left -= 60;
  }
  if (left) {
    (void)write(1, curr, left);
    fputc('\n', stdout);
  }
  b->len = 0;
}

static void dump(struct buf *b)
{
  if ('>' == *b->str)
    fputs(b->str, stdout);
  if (b->len)
    dumplines(b);
}

/* read line-by-line a redirected FASTA format file from stdin
 * for each sequence:
 *   write the id, description, and the reverse-complement sequence in FASTA
 *   format to stdout */
int main(void)
{
  struct buf b;
  buf_init(&b);
  char *curr = b.str;
  while (fgets(curr, BUFSZ, stdin)) {
    if ('>' == *curr) {
      dump(&b);
    } else {
      size_t len = strlen(curr);
      if (len && b.str[len-1] == '\n')
        --len;
      b.len += len;
      curr += len;
      buf_grow(&b, len);
    }
  }
  dump(&b);
  return 0; 
}
