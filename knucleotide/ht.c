/* ex: set ts=2 et: */

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

void htfree(struct ht *t)
{
  free(t->bmp);
}

struct htentry * htfind(struct ht *t, const char *key, size_t len, unsigned long idx)
{
  struct htentry *h = t->bin[idx];
  while (h && (h->len != len || memcmp(h->key, key, len)))
    h = h->nxt;
  return h;
}

static void htentrynew(struct ht *t, const char *key, size_t len, unsigned long idx)
{
  struct htentry *e = t->nxt++;
  e->nxt = t->bin[idx];
  e->key = key;
  e->len = len;
  e->cnt = 1;
  t->bin[idx] = e;
}

void htincr(struct ht *t, const char *key, size_t len, unsigned long idx)
{
  struct htentry *e = htfind(t, key, len, idx);
  if (e)
    e->cnt++;
  else
    htentrynew(t, key, len, idx);
}

static int hteach_find(struct hteach *e, const struct ht *t)
{
  unsigned long idx = e->idx;
  do
    ++idx;
  while (t->bincnt != idx && !t->bin[idx]);
  if (t->bincnt == idx)
    return 0;
  e->idx = idx;
  e->cur = t->bin[idx];
  return 1;
}

int hteach_init(struct hteach *e, const struct ht *t)
{
  e->idx = 0;
  if (!t->bin[0])
    return hteach_find(e, t);
  e->cur = t->bin[0];
  return 1;
}

int hteach_next(struct hteach *e, const struct ht *t)
{
  if (!e->cur->nxt)
    return hteach_find(e, t);
  e->cur = e->cur->nxt;
  return 1;
}

