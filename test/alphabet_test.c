#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cstr.h>

#include "testlib.h"

TL_TEST(test_create_alphabet) {
  TL_BEGIN();

  enum cstr_errcodes err;

  char const *x = "foobar";
  struct cstr_alphabet *alpha = cstr_alphabet_from_string(x, &err);
  TL_FATAL_IF(NULL == alpha);
  TL_FATAL_IF(CSTR_NO_ERROR != err);

  TL_ERROR_IF(alpha->map[0] != 0);
  TL_ERROR_IF(alpha->map['a'] != 1);
  TL_ERROR_IF(alpha->map['b'] != 2);
  TL_ERROR_IF(alpha->map['f'] != 3);
  TL_ERROR_IF(alpha->map['o'] != 4);
  TL_ERROR_IF(alpha->map['r'] != 5);
  TL_ERROR_IF(6 != alpha->size);

  for (int i = 0; i < 256; i++) {
    if (alpha->map[i]) {
      TL_ERROR_IF(alpha->revmap[alpha->map[i]] != i);
    }
  }

  free(alpha);

  TL_END();
}

TL_TEST(test_mapping) {
  TL_BEGIN();

  enum cstr_errcodes err;

  char const *x = "foobar";
  struct cstr_alphabet *alpha = cstr_alphabet_from_string(x, &err);
  TL_FATAL_IF(CSTR_NO_ERROR != err);

  char *mapped =
      cstr_alphabet_map(alpha, cstr_slice_from_string((char *)x), &err);
  TL_FATAL_IF(mapped == NULL);
  TL_FATAL_IF(CSTR_NO_ERROR != err);

  TL_ERROR_IF(strcmp(mapped, "\3\4\4\2\1\5") != 0);

  free(mapped);
  mapped = 0;

  mapped = cstr_alphabet_map(alpha, cstr_slice_from_string("qux"), &err);
  TL_ERROR_IF(mapped != NULL);
  TL_ERROR_IF(CSTR_MAPPING_ERROR != err);

  free(alpha);

  TL_END();
}

TL_TEST(test_int_mapping) {
  TL_BEGIN();

  enum cstr_errcodes err;

  char const *x = "foobar";
  struct cstr_alphabet *alpha = cstr_alphabet_from_string(x, &err);
  TL_FATAL_IF(CSTR_NO_ERROR != err);

  unsigned int *mapped =
      cstr_alphabet_map_to_int(alpha, cstr_slice_from_string((char *)x), &err);
  TL_FATAL_IF(mapped == NULL);
  TL_FATAL_IF(CSTR_NO_ERROR != err);

  int expected[] = {3, 4, 4, 2, 1, 5, 0};
  TL_TEST_EQUAL_INT_ARRAYS(expected, mapped,
                           sizeof(expected) / sizeof(*expected));
  free(mapped);
  mapped = 0;

  mapped = cstr_alphabet_map_to_int(alpha, cstr_slice_from_string("qux"), &err);
  TL_ERROR_IF(mapped != NULL);
  TL_ERROR_IF(CSTR_MAPPING_ERROR != err);

  free(alpha);

  TL_END();
}

TL_TEST(test_revmapping) {
  TL_BEGIN();

  enum cstr_errcodes err;

  char const *x = "foobar";
  struct cstr_alphabet *alpha = cstr_alphabet_from_string(x, &err);
  TL_FATAL_IF(!alpha);

  char *mapped =
      cstr_alphabet_map(alpha, cstr_slice_from_string((char *)x), &err);
  char *rev = cstr_alphabet_revmap(alpha, cstr_slice_from_string(mapped), &err);

  TL_FATAL_IF(CSTR_NO_ERROR != err);
  TL_ERROR_IF(strcmp(x, rev) != 0);

  free(rev);
  free(mapped);
  free(alpha);

  TL_END();
}

int main(void) {
  TL_BEGIN_TEST_SUITE()
  TL_RUN_TEST(test_create_alphabet);
  TL_RUN_TEST(test_mapping);
  TL_RUN_TEST(test_int_mapping);
  TL_RUN_TEST(test_revmapping);
  TL_END_SUITE();
}
