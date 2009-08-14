/* ex: set ts=2 et: */
/*
 * The Computer Language Benchmarks Game
 *  <URL: http://shootout.alioth.debian.org/>
 *
 * Contributed by Ryan Flynn
 *
 * Compile:
 *  CFLAGS = -W -Wall -std=c99 -fopenmp -m32 -Os -march=core2 -mtune=core2
 *  LDFLAGS = -lgomp -m32
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/time.h>

#define HT_BINS     (1024 * 1024 * 16UL)
#include "ht.h"

#define MAX_ENTRIES (1024 * 1024 * 1024UL)

#undef BUFSZ
#define BUFSZ       (1024 * 512)

struct str {
  char  *str;
  size_t len, alloc;
};

static struct str * str_new(void)
{
  struct str *s = calloc(sizeof *s, 1);
  if (!s)
    perror("malloc"), exit(1);
  s->alloc = BUFSZ;
  s->str = malloc(s->alloc);
  return s;
}

/*
 * 'len' bytes have been written, grow buffer if needed; there must
 * always be >= BUFSZ bytes free at the end
 */
static void str_grow(struct str *s, size_t len)
{
  size_t newlen = s->len + len + BUFSZ;
  if (newlen > s->alloc) {
    char *tmp = realloc(s->str, newlen * 2);
    if (!tmp)
      perror("malloc"), exit(1);
    s->alloc = newlen * 2;
    s->str = tmp;
  }
  while (len--)
    s->str[s->len] = (char)toupper((int)s->str[s->len]), s->len++;
}

/*
 * uniquely hash a DNA sequence
 */
inline unsigned long dna_hash(const char *dna, unsigned len)
{
  unsigned long h = 0;
  while (len--)
    h = (h << 2) + (*dna > 'A') +
                   (*dna > 'C') +
                   (*dna > 'G'), dna++;
  return h;
}

#define index(key, len) (dna_hash(key, len) & (HT_BINS - 1))

static unsigned long do_freq_build(struct ht *t, const struct str *seq, unsigned len) {
  const unsigned long total = seq->len - len + 1;
  const char *key = seq->str;
  unsigned long i;
  for (i = 0; i < total; i++)
    htincr(t, key, len, index(key, len)), key++;
  return total;
}

static ptrdiff_t do_freq_copy(const struct ht *t, struct htentry *e)
{
  struct htentry *c = e;
  struct hteach each;
  if (hteach_init(&each, t))
    do
      *c++ = *each.curr;
    while (hteach_next(&each, t));
  return c - e;
}

static int freqcmp(const void *va, const void *vb)
{
  const struct htentry *a = va, *b = vb;
  if (b->cnt != a->cnt)
    return (int)(b->cnt - a->cnt);
  return strcmp(b->key, a->key);
}

static void do_freq_print(const struct htentry *e, unsigned cnt,
                          unsigned long total, char *buf)
{
  while (cnt--)
    buf += sprintf(buf, "%.*s %5.3f\n",
      (unsigned)e->len, e->key, (double)e->cnt / total * 100.), e++;
  strcat(buf, "\n");
}

#define dna_combo(nth) (1ULL << (2 * (nth)))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

/*
 * count all the 1-nucleotide and 2-nucleotide sequences, and write the
 * code and percentage frequency, sorted by descending frequency and then
 * ascending k-nucleotide key
 */
static void do_frq(const struct str *seq, unsigned len, char *buf)
{
  struct ht *t = malloc(sizeof *t);
  unsigned long long cnt = dna_combo(len);
  htinit(t, MIN(cnt, MAX_ENTRIES));
  struct htentry *e = malloc(cnt * sizeof *e);
  unsigned long total = do_freq_build(t, seq, len);
  cnt = do_freq_copy(t, e);
  qsort(e, cnt, sizeof *e, freqcmp);
  do_freq_print(e, cnt, total, buf);
  free(e), htfree(t), free(t);
}

static void frq(const struct str *seq, char *out)
{
  char buf[2][512];
# pragma omp parallel for
  for (int i = 0; i < 2; i++)
    do_frq(seq, i+1, buf[i]);
  for (int i = 0; i < 2; i++)
    strcat(out, buf[i]);
}

/*
 * count all 'buf' substrings of length 'len', return count for buf[0..len-1]
 */
static void do_cnt(const struct str *seq, unsigned len, char *buf)
{
  static const char *Match = "GGTATTTTAATTTATAGT";
  struct htentry *e;
  struct ht *t = malloc(sizeof *t);
  unsigned long long bump = MIN(dna_combo(len), MAX_ENTRIES);
  htinit(t, bump);
  do_freq_build(t, seq, len);
  e = htfind(t, Match, len, index(Match, len));
  sprintf(buf, "%lu\t%.*s\n", e ? e->cnt : 0, len, Match);
  htfree(t);
  free(t);
}

/*
 * COUNT ALL THE 3- 4- 6- 12- AND 18-NUCLEOTIDE SEQUENCES, and write the
 * count and code for the specific sequences GGT GGTA GGTATT GGTATTTTAATT
 * GGTATTTTAATTTATAGT
 */
static void cnt(const struct str *seq, char *out) {
  struct {
    unsigned prefix;
    char result[64];
  } Cnt[] = {
    {  3, "" },
    {  4, "" },
    {  6, "" },
    { 12, "" },
    { 18, "" }
  };
  int i;
# pragma omp parallel for
  for (i = 0; i < 5; i++)
    do_cnt(seq, Cnt[i].prefix, Cnt[i].result);
  for (i = 0; i < 5; i++)
    strcat(out, Cnt[i].result);
}

/*
 * read line-by-line a redirected FASTA format file from stdin
 * extract DNA sequence THREE
 */
static const struct str * dna_seq3(void)
{
  struct str *s = str_new();
  while (NULL != fgets(s->str, BUFSZ, stdin))
    if ('>' == *s->str && !strncmp(">THREE", s->str, 6))
      break;
  char *curr = s->str;
  while (NULL != fgets(curr, BUFSZ, stdin)) {
    if ('>' == *curr)
      break;
    size_t len = strlen(curr);
    if ('\n' == curr[len-1])
      len--;
    str_grow(s, len);
    curr = s->str + s->len;
  }
  return s;
}

int main(void)
{
  static char buf[2][4096];
  const struct str *seq = dna_seq3();
# pragma omp sections
  {
    frq(seq, buf[0]);
    cnt(seq, buf[1]);
  }
# pragma omp barrier
  fputs(buf[0], stdout);
  fputs(buf[1], stdout);
  return 0;
}
