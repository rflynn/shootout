/* ex: set ts=2 et : */

#ifndef HT_H
#define HT_H

#include <stdlib.h>

struct htentry {
  const char     *key;
  size_t          len;
  struct htentry *nxt;
  unsigned long   cnt;
};

struct ht {
  struct htentry *nxt,
                 *bmp,
                **bin;
  unsigned long   bincnt;
};

void              htinit(struct ht *, unsigned long, unsigned long long);
void              htincr(struct ht *, const char *, size_t, unsigned long);
struct htentry *  htfind(struct ht *, const char *, size_t, unsigned long);
struct htentry *  ht2vec(const struct ht *t);
inline ptrdiff_t  htsize(const struct ht *t){ return t->nxt - t->bmp; }
inline void       htfree(struct ht *t){ free(t->bmp); }

#endif

