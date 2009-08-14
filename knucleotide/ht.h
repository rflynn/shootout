
#ifndef HT_H
#define HT_H

#include <stddef.h>

#ifndef HT_BINS
#define HT_BINS (64 * 1024)
#endif

struct htentry {
  struct htentry *next;
  const char *key;
  size_t len;
  unsigned long cnt;
};

struct ht {
  struct htentry *bumpcurr,
                 *bump,
                 *bin[HT_BINS];
};

void             htinit(struct ht *, unsigned long long bumpsize);
void             htfree(struct ht *);
struct htentry * htfind(struct ht *, const char *key, size_t len, unsigned long idx);
void             htincr(struct ht *, const char *key, size_t len, unsigned long idx);

struct hteach {
  struct htentry *curr;
  unsigned idx;
};

int hteach_init(struct hteach *, const struct ht *);
int hteach_next(struct hteach *, const struct ht *);

#endif

