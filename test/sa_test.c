#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cstr.h>

void check_suffix_ordered(char const *x, unsigned int n, unsigned int sa[n]) {
  for (int i = 1; i < n; i++) {
    assert(strcmp(x + sa[i - 1], x + sa[i]) < 0);
  }
}

void test_mississippi() {
  char const *x = "mississippi";
  enum cstr_errcodes err;

  unsigned int *sa = cstr_skew_from_string(x, &err);
  assert(sa != NULL);
  assert(CSTR_NO_ERROR == err);

  int n = strlen(x);
  check_suffix_ordered(x, n + 1, sa);
  free(sa);
}

int main(void) {
  test_mississippi();
  return 0;
}
