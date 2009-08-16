
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "ht.h"

void htinit(struct ht *t, unsigned long bincnt, unsigned long long bmp)
{
  t->nxt = t->bmp = malloc((size_t)bmp * sizeof *t->bmp);
  t->bin = calloc(sizeof *t->bin, bincnt);
  t->bincnt = bincnt;
}

struct htentry * htfind(struct ht *t, const char *key, size_t len, unsigned long idx)
{
  struct htentry *h = t->bin[idx];
  while (h && (h->len != len || memcmp(h->key, key, len)))
    h = h->nxt;
  return h;
}

static inline void htentrynew(struct ht *t, const char *key, size_t len, unsigned long idx)
{
  struct htentry *e = t->nxt++;
  e->nxt = t->bin[idx];
  e->key = key;
  e->len = len;
  e->val.cnt = 1;
  t->bin[idx] = e;
}

void htincr(struct ht *t, const char *key, size_t len, unsigned long idx)
{
  struct htentry *e = htfind(t, key, len, idx);
  if (e)
    e->val.cnt++;
  else
    htentrynew(t, key, len, idx);
}

/*
 * allocate a vector and populate with contents of hash table
 */
struct htentry * ht2vec(const struct ht *t)
{
  if (!htsize(t))
    return NULL;
  struct htentry *v = malloc(htsize(t) * sizeof *v);
  struct htentry *w = v;
  unsigned long idx = 0;
  do {
    while (idx < t->bincnt && !t->bin[idx])
      ++idx;
    if (idx < t->bincnt) {
      const struct htentry *r = t->bin[idx];
      do
        *w++ = *r;
      while ((r = r->nxt));
      ++idx;
    }
  } while (idx < t->bincnt);
  return v;
}

