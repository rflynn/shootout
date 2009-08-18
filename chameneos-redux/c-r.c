/* The Computer Language Benchmarks Game
   http://shootout.alioth.debian.org/

   contributed by Michael Barker
   based on a Java contribution by Luzius Meisser

   convert to C by dualamd
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/select.h>

enum Color {
  blue, red, yellow, Color_CNT
};

static const char* ColorName[COLOR_CNT] = {
  "blue", "red", "yellow"
};

static const enum Color Compliment[COLOR_CNT][COLOR_CNT] = {
               /* blue    red     yellow */
  /* blue   */ {  blue,   yellow, red    },
  /* red    */ {  yellow, red,    blue   },
  /* yellow */ {  red,    blue,   yellow }
};

static void formatNumber(unsigned n, char *outbuf)
{
  static const char *Digits[] = {
    "zero ", "one ", "two ",   "three ", "four ",
    "five ", "six ", "seven ", "eight ", "nine "
  };
  *outbuf = '\0';
  do
    strcat(outbuf, Digits[n % 10]);
  while ((n /= 10));
  outbuf[strlen(outbuf)-1] = '\0';
}

static void printColors(void)
{
  for (enum Color x = blue; x <= yellow; x++)
    for (enum Color y = blue; y <= yellow; y++)
      printf(" %s + %s -> %s\n",
        ColorName[x], ColorName[y], ColorName[Compliment[x][y]]);
}

struct Creature
{
  int        id;
  /*
   * this is what the parent/creatures read/write back and forth
   * the rest
   */
  struct Meet {
    enum Color colour;
    unsigned   cnt, sameCnt;
    bool       two_met, sameid;
  } meet;
  int        from[2],
             to[2];
             /*
              * from[0]: creature read
              * from[1]: master write
              * to[0]: master read
              * to[1]: creature write
              */
  struct Meet rd,
              wr;
};

/* format meeting times of each creature to string */
static char * Creature_getResult(struct Creature* cr, char* str)
{
  formatNumber(cr->sameCount, str + sprintf(str, "%u", cr->cnt));
  return str;
}

static void Meet_merge(const struct Meet *src, struct Meet *dst)
{
  dst->color = src->color;
  dst->cnt++;
  dst->sameCnt += src->sameCnt;
  dst->two_met |= src->two_met;
  dst->same_id |= src->same_id;
}

static void Creature_Init(struct Creature *cr, struct MeetingPlace* place, enum Colour color)
{
  static int CreatureID = 0;
  cr->place = place;
  cr->cnt = cr->sameCnt = 0;
  cr->id = ++CreatureID;
  cr->color = color;
  cr->two_met = false;
  pipe(cr->from);
  pipe(cr->to);
}

/*
 * run in the context of a forked "creature" process.
 * "meet" with other creatures, update our own state
 * based on the results.
 */
static void Meet(struct Creature *c)
{
  struct Meet m;
  do {
    write(c->to[1], "\0", 1);
    read(c->from[0], &m, sizeof m);
    Creature_merge(&m, &c->meet);
  } while (tmp.cnt);
  write(c->to[1], &c->meet, sizeof c->meet);
}


BOOL Meet( struct Creature* cr)
{
   BOOL retval = TRUE;
   struct MeetingPlace* mp = cr->place;
   pthread_mutex_lock( &(mp->mutex) );
   if ( mp->meetingsLeft > 0 )
   {
      if ( mp->firstCreature == 0 )
      {
         cr->two_met = FALSE;
         mp->firstCreature = cr;
      }
      else
      {
         struct Creature* first;
         enum Colour newColour;
         first = mp->firstCreature;
         newColour = doCompliment( cr->colour, first->colour );
         cr->sameid = cr->id == first->id;
         cr->colour = newColour;
         cr->two_met = TRUE;
         first->sameid = cr->sameid;
         first->colour = newColour;
         first->two_met = TRUE;
         mp->firstCreature = 0;
         mp->meetingsLeft--;
      }
   }
   else
      retval = FALSE;
   pthread_mutex_unlock( &(mp->mutex) );
   return retval;
}

/*
 * context: run by single master thread
 */
static void runGame(unsigned meetings, const unsigned n, struct Creature *c)
{
  const int maxfd = c[n-1]->to[0];
  while (meetings) {
#if 0
    int select(int nfds, fd_set *readfds, fd_set *writefds,
               fd_set *exceptfds, struct timeval *timeout);
    void FD_CLR(int fd, fd_set *set);
    int  FD_ISSET(int fd, fd_set *set);
    void FD_SET(int fd, fd_set *set);
    void FD_ZERO(fd_set *set);
#endif
    fd_set rd;
    FD_ZERO(&rd);
    for (unsigned i = 0; i < n; i++)
      FD_SET(c->to[0], &rd);
              /*
               * from[0]: creature read
               * from[1]: master write
               * to[0]: master read
               * to[1]: creature write
               */
    int sel = select(maxfd, &rd, NULL, NULL, NULL);
    printf("sel=%d\n", sel);
    if (sel <= 0)
      continue;
    for (unsigned i = 0; i < cnt; i++) {
      if (FD_ISSET(c[i]->to[0], &rd)) {
        if (read(c[i]->to[0], 
      }
    }
    
  }
}

void runGame(int n_meeting, int ncolor, const enum Colour* colours)
{
   int i;
   int total = 0;
   char str[256];
   struct MeetingPlace place;
   struct Creature *creatures = (struct Creature*) calloc( ncolor, sizeof(struct Creature) );
   MeetingPlace_Init( &place, n_meeting );
   /* print initial color of each creature */
   for (i = 0; i < ncolor; i++)
   {
      printf( "%s ", ColourName[ colours[i] ] );
      Creature_Init( &(creatures[i]), &place, colours[i] );
   }
   printf("\n");
   /* wait for them to meet */
   for (i = 0; i < ncolor; i++)
      pthread_join( creatures[i].ht, 0 );
   /* print meeting times of each creature */
   for (i = 0; i < ncolor; i++)
   {
      printf( "%s\n", Creature_getResult(&(creatures[i]), str) );
      total += creatures[i].count;
   }
   /* print total meeting times, should equal n_meeting */
   printf( "%s\n\n", formatNumber(total, str) );
   /* cleaup & quit */
   pthread_mutex_destroy( &place.mutex );
   free(creatures);
}



int main(int argc, char** argv)
{
   int n = (argc == 2) ? atoi(argv[1]) : 600;

   printColoursTable();
   printf("\n");

   const enum Colour r1[] = {   blue, red, yellow   };
   const enum Colour r2[] = {   blue, red, yellow,
                                red, yellow, blue,
                                red, yellow, red, blue   };

   runGame( n, sizeof(r1) / sizeof(r1[0]), r1 );
   runGame( n, sizeof(r2) / sizeof(r2[0]), r2 );

   return 0;
}

