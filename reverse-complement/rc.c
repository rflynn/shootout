/*
 * line-based i/o is the limiting factor here;
 * the only way to make it faster is to have
 * a line-reader thread and a worker thread.
 * i wonder if we can do multi-threaded i/o
 * to read multiple lines concurrently?
 * awful.
 */

#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#define BUFSZ   (1024 * 8UL)
#define BSWAP64 __builtin_bswap64

struct buf {
  char  *str;
  size_t len, alloc;
};

static inline void buf_init(struct buf *b)
{
  b->len = 0;
  b->alloc = BUFSZ;
  b->str = malloc(b->alloc);
}

/**
 * grow buffer (if necessary) to contain >= BUFSZ available space
 */
static inline void buf_grow(struct buf *b, size_t len)
{
  size_t newlen = b->len + len + BUFSZ;
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
  static const char Rev[128] = {
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
  while (len--)
    *str = Rev[(unsigned char)*str], str++;
}

/**
 * reverse string in-place
 */
static inline void reverse(char *str, size_t len)
{
  long i = 0,
       half = len / 2 + (len & 1);
  for ( ; i <= half - 8; i += 8) {
    /* 8 octets at a time */
    int64_t tmp = *(int64_t *)(str+i);
    *(int64_t *)(str+i) = BSWAP64(*(int64_t *)(str-i+len-8));
    *(int64_t *)(str-i+len-8) = BSWAP64(tmp);
  }
  while (i < half) {
    char tmp = str[len-i-1];
    str[len-i-1] = str[i];
    str[i++] = tmp;
  }
}

static inline void dumplines(struct buf *b)
{
  size_t left   = b->len,
         outlen = left + ((left + 59) / 60);
  char  *out    = malloc(outlen),
        *wr     = out,
        *rd     = b->str;
  /* format 60-char lines */
  while (left >= 60) {
    memcpy(wr, rd, 60);
    left -= 60, rd += 60, wr += 60;
    *wr++ = '\n';
  }
  if (left)
    memcpy(wr, rd, left), wr[left] = '\n';
  /* print */
  fwrite_unlocked(out, 1, outlen, stdout);
  free(out);
}

static inline void dump(struct buf *b, bool id)
{
  if (b->len) {
    reverse(b->str, b->len);
    dumplines(b);
  }
  if (id)
    fwrite_unlocked(b->str + b->len, 1,
                    strlen(b->str + b->len), stdout);
}

/*
 * act on a line of input
 */
static inline char * addline(char *curr, struct buf *b)
{
  if ('>' == *curr) {
    /* header line, dump contents of buffer */
    dump(b, true);
    b->len = 0;
  } else {
    /* sequence line, append and adjust buffer */
    size_t len = strlen(curr);
    if (len && curr[len-1] == '\n')
      --len;
    complement(curr, len);
    b->len += len;
    buf_grow(b, len);
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
  struct buf b;
  buf_init(&b);
  char *curr = b.str;
  while (fgets_unlocked(curr, BUFSZ, stdin))
    curr = addline(curr, &b);
  dump(&b, false);
  return 0; 
}

