#include "cachelab.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* indexing cache by macro */
#define IDX(m, n, E) m *E + n
#define MAXSIZE 30
char input[MAXSIZE]; /* store a input line */
int hit_count = 0, miss_count = 0, eviction_count = 0;
int debug = 0; /* flag -v*/

/* cache struct */
typedef struct sCache {
  int valid; /* valid bit */
  int tag;   /* tag bit */
  int count; /* number of access count */
} Cache;


/* Cache last_eviction; */

/* convert hex character to integer */
int hextodec(char c);

/* load cache */
void Load(int count, unsigned int setindex, unsigned int tag,
          unsigned int offset, unsigned int size, double s_pow, unsigned int E,
          double b_pow, Cache *cache);

int main(int argc, char *argv[]) {
  const char *str = "Usage: ./csim [-hv] -s <num> -E <num> -b <num> -t "
                    "<file>\nOptions:\n  -h Print this help message.\n  -v "
                    "Optional verbose flag.\n  -s <num> Number of set index "
                    "bits.\n  -E <num> Number of lines per set.\n  -b <num> "
                    "Number of block offset bits.\n  -t <file> Trace file.\n\n"
                    "Examples :\n linux> ./csim -s 4 -E 1 -b 4 -t "
                    "traces/yi.trace\n linux>  ./csim -v -s 8 -E 2 "
                    "-b 4 -t traces/yi.trace\n "; // help info
  int opt = 0;
  unsigned int s = 0, E = 0, b = 0; /* number of set index bits, number of lines per set, numer of blocks offset bits */
  double s_pow = 0, b_pow = 0; /* number of sets, number of blocks */
  char *t = "";                /* trace  file*/


  /* getopt: */
  while ((opt = getopt(argc, argv, "hvs:E:-b:-t:")) != -1) {
    switch (opt) {
    case 's':
      s = atoi(optarg);
      s_pow = 1 << s; /* num of sets */
      break;
    case 'E':
      E = atoi(optarg); /* num of lines per set */
      break;
    case 'b':
      b = atoi(optarg);
      b_pow = 1 << b; /* num of blocks per line */
      break;
    case 't':
      t = optarg; /* trace file*/
      break;
    case 'v':
      debug = 1; /* v flag */
      break;
    case 'h':
      printf("%s", str); /* help info*/
      return 0;
      break;
    default: /* '?' */
      fprintf(stderr, "Usage: %s [-hv] -s <num> -E <num> -b <num> -t <file>\n",
              argv[0]);
      exit(EXIT_FAILURE);
    }
  }


  Cache *cache = (Cache *)malloc(sizeof(Cache) * s_pow * E); /* cache array */
  for (int i = 0; i < s_pow * E; i++) { /* init */
    cache[i].valid = 0;
    cache[i].tag = 0;
    cache[i].count = 0;
  }
  FILE *fp = fopen(t, "r"); /* open trace file */
  int count = 0;            /* Cache access count updator */

  /* analyse trace file in line */
  /*e.g. format: 
    L 10,1 miss
    M 20,1 miss hit
    L 22,1 hit
    S 18,1 hit
    L 110,1 miss eviction
    L 210,1 miss eviction
    M 12,1 miss eviction hit
    hits:4 misses:5 evictions:3
  */
  while (fgets(input, MAXSIZE, fp)) {
    int op = 0; /* num of access in need */
    unsigned int offset = 0, tag = 0,
                 setindex = 0; /* block offset，tag ，set index */
    char c;
    int exist_comma = 0;                      /* comma flag */
    unsigned int address = 0, size = 0; /* cache address, size */
    count++;

    for (int i = 0; (c = input[i]) && (c != '\n'); i++) {
      if (c == ' ') { /* pass space */
        continue;
      } else if (c == 'I') {
        op = 0; /* I: access one time */
      } else if (c == 'L') {
        op = 1; /* L: access one time */
      } else if (c == 'S') {
        op = 1; /* S: access one time */
      } else if (c == 'M') {
        op = 2; /* M: access two times */
      } else if (c == ',') {
        exist_comma = 1; /* comma exist */
      } else {
        if (exist_comma) {          
          size = hextodec(c);
        } else {
          address =
              16 * address + hextodec(c);
        }
      }
    }

    /* get offset from address */
    for (int i = 0; i < b; i++) {
      offset = offset * 2 + address % 2;
      address >>= 1;
    }

    /* get set index from address */
    for (int i = 0; i < s; i++) {
      setindex = setindex * 2 + address % 2;
      address >>= 1;
    }

    /* get tag from address */
    tag = address;

    if (debug && op != 0) {
      printf("\n%s", input);
    }
    if (op == 1) {
      Load(count, setindex, tag, offset, size, s_pow, E, b_pow, cache);
    }
    /* M: access two times，at first load, at second hit straightly*/
    if (op == 2) {
      Load(count, setindex, tag, offset, size, s_pow, E, b_pow, cache);
      hit_count++;
      if (debug) {
        printf(" hit");
      }
    }

  }

  free(cache);
  fclose(fp);
  // optind: num of opt args
  if (optind > argc) {
    fprintf(stderr, "Expected argument after options\n");
    exit(EXIT_FAILURE);
  }

  if (debug) {
    printf("\n");
  }
  printSummary(hit_count, miss_count, eviction_count);
  return 0;
}

/* convert hex to dec */
int hextodec(char c) {
  if (c >= '0' && c <= '9') {
    return c - '0';
  }
  if (c >= 'A' && c <= 'F') {
    return c - 'A' + 10;
  }
  if (c >= 'a' && c <= 'f') {
    return c - 'a' + 10;
  }
  return 0;
}

/* load cache */
void Load(int count, unsigned int setindex, unsigned int tag,
          unsigned int offset, unsigned int size, double s_pow, unsigned int E,
          double b_pow, Cache *cache) {

  /* commare with tag, if exists, then hit */
  for (int i = 0; i < E; i++) {
    if (cache[IDX(setindex, i, E)].valid &&
        tag == cache[IDX(setindex, i, E)].tag) {
      cache[IDX(setindex, i, E)].count = count;
      hit_count++;
      if (debug) {
        printf(" hit");
      }
      return;
    }
  }

  /* if not hit, select a free cache*/
  miss_count++;
  if (debug) {
    printf(" miss");
  }
  for (int i = 0; i < E; i++) {
    if (!cache[IDX(setindex, i, E)].valid) {
      cache[IDX(setindex, i, E)].tag = tag;
      cache[IDX(setindex, i, E)].count = count;
      cache[IDX(setindex, i, E)].valid = 1;
      return;
    }
  }

  /* if full, knock out a cache*/
  int mix_index = 0, mix_count = 1000000000;
  for (int i = 0; i < E; i++) {
    if (cache[IDX(setindex, i, E)].count < mix_count) {
      mix_count = cache[IDX(setindex, i, E)].count;
      mix_index = i;
    }
  }

  eviction_count++;
  if (debug) {
    printf(" eviction");
  }

  cache[IDX(setindex, mix_index, E)].tag = tag;
  cache[IDX(setindex, mix_index, E)].count = count;
  cache[IDX(setindex, mix_index, E)].valid = 1;

  return;
}