/*
 * The Computer Language Benchmarks Game
 * http://shootout.alioth.debian.org/
 *
 * contributed by Ryan Flynn
 */

#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ht.h"

#define BUFSZ          (1024 * 512UL)
#define HT_BINS        (1024 * 1024 * 32UL) /* NOTE: power of 2 */
#define MAX_ENTRIES    (HT_BINS * 2)
#define dna_combo(nth) (1ULL << (2 * (nth)))
#define MIN(a, b)      ((a) < (b) ? (a) : (b))

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
  while (len--) /* uppercase */
    b->str[b->len] = (char)toupper((int)b->str[b->len]), b->len++;
}

inline unsigned long dna_hash(const char *dna, unsigned len)
{
  static const unsigned char X[UCHAR_MAX + 1] =
  {
    ['A'] = 0,
    ['C'] = 1,
    ['G'] = 2,
    ['T'] = 3,
  };
  unsigned long h = 0;
  while (len--)
    h = (h << 2) | X[(unsigned char)*dna++];
  return h;
}

#define index(key, len) (dna_hash(key, len) & (HT_BINS - 1))

static unsigned long freq_build(struct ht *t, const struct buf *seq, unsigned len) {
  const unsigned long total = seq->len - len + 1;
  const char *key = seq->str;
  for (unsigned long i = 0; i < total; i++)
    htincr(t, key, len, index(key, len)), key++;
  return total;
}

/*
 * sorted by descending frequency and then ascending k-nucleotide key
 */
static int freq_cmp(const void *va, const void *vb)
{
  const struct htentry *a = va, *b = vb;
  if (a->val.cnt != b->val.cnt)
    return (int)(b->val.cnt - a->val.cnt);
  return strcmp(b->key, a->key);
}

static void freq_print(const struct htentry *e, unsigned cnt,
                       unsigned long total, char *dst)
{
  while (cnt--)
    dst += sprintf(dst, "%.*s %5.3f\n",
      (unsigned)e->len, e->key, 100. * e->val.cnt / total), e++;
  strcat(dst, "\n");
}

/* count all the 1-nucleotide and 2-nucleotide sequences, and write the
 * code and percentage frequency, sorted */
static void do_freq(const struct buf *seq, unsigned len, char *dst)
{
  struct ht t;
  htinit(&t, HT_BINS, MIN(dna_combo(len), MAX_ENTRIES));
  unsigned long total = freq_build(&t, seq, len);
  {
    struct htentry *e = ht2vec(&t);
    qsort(e, htsize(&t), sizeof *e, freq_cmp);
    freq_print(e, htsize(&t), total, dst);
    free(e);
  }
  htfree(&t);
}

static void frq(const struct buf *seq, char *dst)
{
  char buf[2][512];
  #pragma omp parallel for
  for (int i = 0; i < 2; i++)
    do_freq(seq, i+1, buf[i]);
  for (int i = 0; i < 2; i++)
    strcat(dst, buf[i]);
}

/* count all 'buf' substrings of length 'len', return count for buf[0..len-1] */
static void do_cnt(const struct buf *seq, unsigned len, char *buf)
{
  const char *Match = "GGTATTTTAATTTATAGT";
  unsigned long cnt = 0;
  struct ht t;
  htinit(&t, HT_BINS, MIN(dna_combo(len), MAX_ENTRIES));
  freq_build(&t, seq, len);
  struct htentry *e = htfind(&t, Match, len, index(Match, len));
  if (e)
    cnt = e->val.cnt;
  htfree(&t);
  sprintf(buf, "%lu\t%.*s\n", cnt, len, Match);
}

/* COUNT ALL THE 3- 4- 6- 12- AND 18-NUCLEOTIDE SEQUENCES, and write the
 * count and code for the specific sequences GGT GGTA GGTATT GGTATTTTAATT
 * GGTATTTTAATTTATAGT */
#define CNT (int)(sizeof Cnt / sizeof Cnt[0])
static void cnt(const struct buf *seq, char *out) {
  static struct {
    unsigned len;
    char res[64];
  } Cnt[] = {
    {  3, "" },
    {  4, "" },
    {  6, "" },
    { 12, "" },
    { 18, "" },
  };
  #pragma omp parallel for schedule(static,1)
  for (int i = CNT - 1; i >= 0; i--)
    if (seq->len >= Cnt[i].len)
      do_cnt(seq, Cnt[i].len, Cnt[i].res);
  for (int i = 0; i < CNT; i++)
    strcat(out, Cnt[i].res);
}

/* read line-by-line a redirected FASTA format file from stdin
 * extract DNA sequence THREE */
static size_t dna_seq3(struct buf *b)
{
  buf_init(b);
  while (NULL != fgets(b->str, BUFSZ, stdin))
    if ('>' == *b->str && !strncmp(">THREE", b->str, 6))
      break;
  char *curr = b->str;
  while (NULL != fgets(curr, BUFSZ, stdin)) {
    if ('>' == *curr)
      break;
    size_t len = strlen(curr);
    if ('\n' == curr[len-1])
      len--;
    buf_grow(b, len);
    curr = b->str + b->len;
  }
  return b->len;
}

int main(void)
{
  static char buf[2][1024];
  struct buf seq;
  if (dna_seq3(&seq))
  #pragma omp sections
  {
    frq(&seq, buf[0]);
    cnt(&seq, buf[1]);
  }
  #pragma omp barrier
  for (int i = 0; i < 2; i++)
    fputs(buf[i], stdout);
  return 0;
}
