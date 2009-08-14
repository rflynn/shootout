/* ex: set ts=2 et: */
/*
 * The Computer Language Benchmarks Game
 *  <URL: http://shootout.alioth.debian.org/>
 *
 * Contributed by Ryan Flynn
 *
 * Compile:
 *  CFLAGS = -fopenmp -std=c99 -pedantic -W -Wall -O3
 *  LDFLAGS = -lgomp
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <unistd.h>
#include <sys/time.h>

#undef BUFSZ
#define BUFSZ (512 * 1024)

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

/* uniquely hash a DNA sequence */
inline unsigned dna_hash(const char *dna, unsigned len)
{
  unsigned h = 0;
  while (len--)
    h = (h << 2) + (*dna > 'A') + (*dna > 'C') + (*dna > 'G'), dna++;
  return h;
}

struct freq {
  unsigned    cnt,
              len;
  const char *str;
};

static int freqcmp(const void *va, const void *vb)
{
  const struct freq *a = va, *b = vb;
  if (b->cnt != a->cnt)
    return (int)(b->cnt - a->cnt);
  if (!a->str || !b->str)
    return 0;
  return strncmp(b->str, a->str, b->len);
}

/*
 * count all the 1-nucleotide and 2-nucleotide sequences, and write the
 * code and percentage frequency, sorted by descending frequency and then
 * ascending k-nucleotide key
 */
static void do_frq(const struct str *seq, unsigned len, char *buf)
{
# define dna_combo(nth) (1 << (2 * (nth)))
  struct freq Freq[16];
  const long total = seq->len - len + 1;
  long i, combos = dna_combo(len);
  
  memset(Freq, 0, sizeof Freq);
  
  for (i = 0; i < combos; i++)
    Freq[i].len = len;
  
  for (i = 0; i < total; i++) {
    unsigned h = dna_hash(seq->str + i, len);
    Freq[h].cnt++;
    if (!Freq[h].str)
      Freq[h].str = (char*)seq->str + i;
  }
  
  qsort(Freq, combos, sizeof *Freq, freqcmp);
  
  for (i = 0; i < combos; i++)
    buf += sprintf(buf, "%.*s %5.3f\n",
      len, Freq[i].str, (double)Freq[i].cnt / total * 100.);
  strcat(buf, "\n");
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
 * initialize Boyer-Moore-Horspool skip table
 */
static void bmh_init(const char * restrict needle,
                          ssize_t nlen, size_t skip[])
{
  size_t i, last = nlen - 1;
  for (i = 0; i <= UCHAR_MAX; i++)
    skip[i] = nlen;
  for (i = 0; i < last; i++)
    skip[(unsigned char)needle[i]] = last - i;
}

/*
 * Boyer-Moore-Horspool string search
 */
static const char * bmh(const char * restrict haystack, size_t hlen,
                        const char * restrict needle,   size_t nlen,
                        const size_t skip[])
{
  const size_t last = nlen - 1;
  while (hlen >= nlen) {
    for (size_t i = last; haystack[i] == needle[i]; i--)
      if (!i) return haystack;
    hlen     -= skip[(unsigned char)haystack[last]];
    haystack += skip[(unsigned char)haystack[last]];
  }
  return NULL;
}

/*
 * count instances of substring Match[0..len-1] in string buf
 */
static void do_cnt(const struct str *seq, unsigned len, char *buf)
{
  static const char *Match = "GGTATTTTAATTTATAGT";
  size_t skip[UCHAR_MAX + 1];
  const char *next = seq->str - 1;
  size_t left = seq->len;
  unsigned long c = 0;
  bmh_init(Match, len, skip);
  while ((next = bmh(next + 1, left, Match, len, skip)))
    c++, left = seq->len - (next - seq->str) - 1;
  sprintf(buf, "%lu\t%.*s\n", c, len, Match);
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
  } Cnt[5] = { { 3, "" }, { 4, "" }, { 6, "" }, { 12, "" }, { 18, "" } };
# pragma omp parallel for
  for (int i = 0; i < 5; i++)
    do_cnt(seq, Cnt[i].prefix, Cnt[i].result);
  for (int i = 0; i < 5; i++)
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
