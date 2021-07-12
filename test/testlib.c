#include "testlib.h"

#include <stdlib.h>
#include <string.h>

void tl_error(struct tl_state *state, char *msg) {
  fputs(msg, stderr);
  state->no_errors++;
}

void tl_fatal(struct tl_state *state, char *msg) {
  tl_error(state, msg);
  longjmp(state->escape, 1);
}

int tl_test_array(void *restrict expected, void *restrict actual, size_t arrlen,
                  size_t objsize) {
  char *restrict x = expected, *restrict a = actual;
  for (int i = 0; i < arrlen; i++) {
    if (memcmp(x, a, objsize) != 0) {
      return i;
    }
    x += objsize;
    a += objsize;
  }
  return -1;
}