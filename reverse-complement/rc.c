/*
 * The Computer Language Benchmarks Game
 * http://shootout.alioth.debian.org
 *
 * contributed by Ryan Flynn
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define BUFSZ   8192

struct buf {
  size_t len,
         alloc;
  char  *str;
};

/**
 * grow buffer (if necessary) to contain >= BUFSZ available space
 */
static inline void buf_grow(struct buf *b, size_t len)
{
  b->len += len;
  size_t newlen = b->len + BUFSZ;
  if (newlen > b->alloc) {
    b->alloc = newlen * 2;
    b->str = realloc(b->str, b->alloc);
    if (!b->str)
      perror("realloc"), exit(1);
  }
}

/**
 * reverse and complement the buffer contents in-place
 * one byte at a time
 */
static inline void complement(char *str, size_t len)
{
  static const char Rev[256] = {
    ['A'] = 'T', ['a'] = 'T',
    ['B'] = 'V', ['b'] = 'V', 
    ['C'] = 'G', ['c'] = 'G',
    ['D'] = 'H', ['d'] = 'H', 
    ['G'] = 'C', ['g'] = 'C', 
    ['H'] = 'D', ['h'] = 'D',
    ['K'] = 'M', ['k'] = 'M', 
    ['M'] = 'K', ['m'] = 'K', 
    ['N'] = 'N', ['n'] = 'N',
    ['R'] = 'Y', ['r'] = 'Y', 
    ['S'] = 'S', ['s'] = 'S', 
    ['T'] = 'A', ['t'] = 'A',
    ['V'] = 'B', ['v'] = 'B', 
    ['W'] = 'W', ['w'] = 'W', 
    ['Y'] = 'R', ['y'] = 'R'
  };
  #pragma omp parallel for
  for (unsigned i = 0; i < len; i++)
    str[i] = Rev[(unsigned char)str[i]];
}

/**
 * reverse string in-place
 */
static inline void reverse(char *str, size_t len)
{
  long i = 0,
       half = len / 2 + (len & 1);
  #pragma omp parallel for
  for (i = 0; i <= half - 8; i += 8) {
    /* 8 bytes at the same time */
    int64_t tmp = *(int64_t *)(str+i);
    *(int64_t *)(str+i) = __builtin_bswap64(*(int64_t *)(str-i+len-8));
    *(int64_t *)(str-i+len-8) = __builtin_bswap64(tmp);
  }
  while (i < half) {
    char tmp = str[len-i-1];
    str[len-i-1] = str[i];
    str[i++] = tmp;
  }
}

static inline void dumplines(struct buf *b)
{
# define LINE 60
  size_t outlen = b->len + ((b->len + LINE - 1) / LINE),
         left   = b->len,
         wr     = 0,
         rd     = 0,
         done   = left % LINE;
  char  *out    = malloc(outlen);
  //#pragma omp parallel for
  for (rd = wr = 0; left != done;
       rd += LINE, wr += LINE+1, left -= LINE)
  {
    memcpy(out+wr, b->str+rd, LINE);
    out[wr+LINE] = '\n';
  }
  if (left) {
    memcpy(out+wr, b->str+rd, left);
    out[wr+left] = '\n';
  }
  fwrite_unlocked(out, 1, outlen, stdout);
  free(out);
  b->len = 0;
}

static inline void dump(struct buf *b)
{
  if (!b->len)
    return;
  reverse(b->str, b->len);
  dumplines(b);
}

/*
 * act on a line of input
 */
static inline char * addline(char *curr, size_t len, struct buf *b)
{
  if ('>' != *curr) {
    complement(curr, len - 1);
    buf_grow(b, len - 1);
  } else {
    dump(b);
    fwrite_unlocked(curr, 1, len, stdout);
  }
  return b->str + b->len;
}

/*
 * read line-by-line a redirected FASTA format file from stdin
 * for each sequence:
 *   write the id, description, and the reverse-complement sequence
 *   in FASTA format to stdout
 */
int main(void)
{
  struct buf b = { 0, BUFSZ, malloc(BUFSZ) };
  char *curr = b.str;
  while (fgets_unlocked(curr, BUFSZ, stdin))
    curr = addline(curr, __builtin_strlen(curr), &b);
  dump(&b);
  return 0; 
}

