/*
 * The Computer Language Benchmarks Game
 * http://shootout.alioth.debian.org
 *
 * contributed by Bob W
 * refactored by Ryan Flynn
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#define LINESZ    60
#define OUTBUFSZ  8 * 1024

/*
 *  _ _ _ _ _ _ _ _ _ _
 * |_|_|_|_|_|_|_|_|_|_|
 * ^head           <-- ^wr (moves backwards)
 * |<-- alloc bytes -->|
 */
struct revbuf {
  size_t alloc;
  char  *head,
        *wr;
};

#define buf_end(b)  ((b)->head + (b)->alloc)

/*
 * grow buffer or die; adjust members appropriately
 */
static inline void revbuf_grow(struct revbuf *b)
{
  const char *old = b->head;
  const size_t wrlen = buf_end(b) - b->wr;
  b->alloc *= 2;
  b->head = realloc(b->head, b->alloc);
  assert(b->head && "realloc");
  b->wr = memmove(buf_end(b) - wrlen,
                  b->head + (b->wr - old), wrlen);
}

/*
 * for each byte in rd
 *   if has a complement
 *     decrease b->wr and write complement
 */
static inline void revcomp(char *rd, struct revbuf *b)
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
  while (*rd) {
    char c = Rev[(unsigned char)(*rd++)];
    if (c)
      *--b->wr = c;
  }
}

/*
 * format and write each line of LINESZ or fewer bytes
 */
static inline void output(struct revbuf *b)
{
  char *pq = b->head;
  while (b->wr < buf_end(b)) {
    size_t len = LINESZ < buf_end(b) - b->wr ?
                 LINESZ : buf_end(b) - b->wr;
    memmove(pq, b->wr, len); /* move l to free space */
    b->wr += len;
    pq += len;
    *pq++ = '\n';
  }
  fwrite(b->head, 1, pq - b->head, stdout);
}

int main(void)
{
  struct revbuf b = { OUTBUFSZ, malloc(OUTBUFSZ), 0 };
  char l[LINESZ+1];
  char *rd = fgets(l, sizeof l, stdin);
  
  assert("Buffer allocation" && b.head);
  assert("No input data" && rd);
  assert("First char not '>'" && '>' == *l);
  b.wr = buf_end(&b);
  while (rd) {
    fputs(l, stdout); /* print id */
    while ((rd = fgets(l, sizeof l, stdin)) && '>' != *l) {
      if (b.wr <= b.head + sizeof l)
        revbuf_grow(&b);
      revcomp(l, &b);
    }
    output(&b);
  }
  return 0;
}
