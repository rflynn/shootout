/* ex: set ts=2 et : */

#ifndef HT_H
#define HT_H

#include <stdlib.h>

struct htentry {
  struct htentry *nxt;
  const char     *key;
  size_t          len;
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
void              htfree(struct ht *);

struct hteach {
  struct htentry *cur;
  unsigned long   idx;
};

int               hteach_init(struct hteach *, const struct ht *);
int               hteach_next(struct hteach *, const struct ht *);

#endif

