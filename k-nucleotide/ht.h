/*
 * a fast, simple, hash table
 * caller must
 *    know max keys in advance
 *    implement hash function
 */

#ifndef HT_H
#define HT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct htentry {
  const char     *key;
  size_t          len;
  struct htentry *nxt;
  union {
    unsigned long cnt;
    void         *ptr;
  } val;
};

struct ht {
  struct htentry *nxt,    /* next htentry */
                 *bmp,    /* vector of pre-allocated htentry */
                **bin;    /* list heads */
  unsigned long   bincnt;
};

static inline void htinit(struct ht *t, unsigned long bincnt,
                          unsigned long long bmp)
{
  t->nxt = t->bmp = malloc((size_t)bmp * sizeof *t->bmp);
  t->bin = calloc(sizeof *t->bin, bincnt);
  t->bincnt = bincnt;
}

static inline ptrdiff_t  htsize(const struct ht *t)
{
  return t->nxt - t->bmp;
}

static inline void htfree(struct ht *t){ free(t->bmp); }

static inline void htentrynew(struct ht *t, const char *key, size_t len,
                              unsigned long idx)
{
  struct htentry *e = t->nxt++;
  e->nxt = t->bin[idx];
  e->key = key;
  e->len = len;
  e->val.cnt = 1;
  t->bin[idx] = e;
}

static inline struct htentry * htfind(struct ht *t, const char *key,
                                      size_t len, unsigned long idx)
{
  struct htentry *h = t->bin[idx];
  while (h && (h->len != len || memcmp(h->key, key, len)))
    h = h->nxt;
  return h;
}

static inline void htincr(struct ht *t, const char *key, size_t len,
                          unsigned long idx)
{
  struct htentry *e = htfind(t, key, len, idx);
  if (e)
    e->val.cnt++;
  else
    htentrynew(t, key, len, idx);
}

/* allocate a vector and populate with contents of hash table */
static inline struct htentry * ht2vec(const struct ht *t)
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

#endif

