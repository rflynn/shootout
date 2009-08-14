/* a minimal hash table in c */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "ht.h"

void htinit(struct ht *t, unsigned long long bumpsize)
{
  unsigned long long bytes = bumpsize * sizeof *t->bump;
  if (bytes > 1024UL * 1024 * 1024)
    bytes = 1024UL * 1024 * 1024; /* FIXME: too specific */
  t->bump = malloc((size_t)bytes);
  t->bumpcurr = t->bump;
  memset(t->bin, 0, sizeof t->bin);
}

void htfree(struct ht *t)
{
  free(t->bump);
}

struct htentry * htfind(struct ht *t, const char *key, size_t len, unsigned long idx)
{
  struct htentry *h = t->bin[idx];
  while (h && memcmp(h->key, key, len))
    h = h->next;
  return h;
}

void htincr(struct ht *t, const char *key, size_t len, unsigned long idx)
{
  struct htentry *e = htfind(t, key, len, idx);
  if (e) {
    e->cnt++;
  } else {
    e = t->bumpcurr++;
    e->next = t->bin[idx];
    e->key = key;
    e->len = len;
    e->cnt = 1;
    t->bin[idx] = e;
  }
}

static int hteach_find(struct hteach *e, const struct ht *t)
{
  unsigned idx = e->idx;
  do ++idx; while (HT_BINS != idx && !t->bin[idx]);
  if (HT_BINS == idx) return 0;
  e->idx = idx;
  e->curr = t->bin[idx];
  return 1;
}

int hteach_init(struct hteach *e, const struct ht *t)
{
  e->idx = 0;
  if (t->bin[0]) {
    e->curr = t->bin[0];
    return 1;
  }
  return hteach_find(e, t);
}

int hteach_next(struct hteach *e, const struct ht *t)
{
  if (!e->curr->next)
    return hteach_find(e, t);
  e->curr = e->curr->next;
  return 1;
}

