#ifndef CSTR_INCLUDED
#define CSTR_INCLUDED

#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Many places, it is more convinient to work with bytes (where
// we know their size) than to work with char, so generally we use
// uint8_t instead of char. That requires a few casts here and there
// but only when we interact with the outer world.
// The only real consern is that functions such as strlen returns
// lengths in char and not bytes, if the two differs. So we need
// our own version of that. Be careful, however, if you want to use
// the nul-char ('\0') as a sentinel. It may not be the null byte!
long long cstr_strlen(const char *x); // returns length of x in bytes

// This is to provide inline functions without putting the
// static code in each output file...
#ifndef INLINE
#define INLINE inline
#endif

// Allocation that cannot fail (except by terminating the program).
// With this, we don't need to test for allocation errors.
void *cstr_malloc(size_t size);

void *cstr_malloc_buffer(size_t obj_size, // size of objects
                         size_t len);     // how many of them

void *cstr_malloc_header_array(size_t base_size, // size of struct before array
                               size_t elm_size,  // size of elements in array
                               size_t len);      // number of elements in array

// Macro for getting the offset of a flexible member array
// from an instance rather than a type (as for
// offsetof(type,member)). This ensures we get the right
// type to match the instance. The macro destroys the
// pointer variable by setting it to NULL, so use with care.
#define CSTR_OFFSETOF_INST(PTR, MEMBER) (size_t)(&((PTR) = 0)->MEMBER)

// Macro for allocating a struct with a flexible array
// element. Gets the offset of the array from a varialble,
// which requires less redundancy and potential for errors
// than offsetof() which requires a type.
// VAR is the struct variable (must be a pointer), FLEX_ARRAY
// is the name of the flexible array member.
#define CSTR_MALLOC_FLEX_ARRAY(VAR, FLEX_ARRAY, LEN) \
  cstr_malloc_header_array(                          \
      CSTR_OFFSETOF_INST(VAR, FLEX_ARRAY), sizeof((VAR)->FLEX_ARRAY[0]), LEN)

// Set a pointer to NULL when we free it
#define CSTR_FREE_NULL(P) \
  do                      \
  {                       \
    free(P);              \
    (P) = 0;              \
  } while (0)

// ==== SLICES =====================================================

// Slices, for easier handling of sub-strings and sub-arrays.
// These should be passed by value and never dynamically allocated
// (although the underlying buffer can be).
//
// The interface to slices is mostly generic, whenever that is possible,
// so they are implemented via macros that generate inline functions and
// then we dispatch to the right function via _Generic() calls.
//
// The most natural type for a length would be size_t, but we will
// allow indexing with negative numbers, so we use a signed long long.
// That will be at least 64 bits, so even if signed will be enough
// for any sane amount of data. Lengths should still be non-negative,
// of course
#define CSTR_SLICE_TYPE(TYPE)                           \
  struct                                                \
  {                                                     \
    signed long long len;                               \
    TYPE *buf; /* NOLINT -- ok not to put TYPE in () */ \
  }

// Creating slice instances
#define CSTR_SLICE_INIT(BUF, LEN) \
  {                               \
    .buf = (BUF), .len = (LEN)    \
  }

// We can create structs with macros, but we need to cast
// to the slice type for composite expressions, which requires
// some _Generic hacks. It is easier to generate inline functions
// and use the dispatch mechanism below.
#define CSTR_GEN_SLICE_NEW(NAME, QUAL, TYPE)                                     \
  INLINE cstr_##NAME cstr_new_##NAME(QUAL TYPE /* NOLINT */ *buf, long long len) \
  {                                                                              \
    return (cstr_##NAME)CSTR_SLICE_INIT(buf, len);                               \
  }

// Allocating slices containing their own buffers
#define CSTR_GEN_ALLOC_SLICE_PROTOTYPE(STYPE, QUAL, TYPE) \
  cstr_##STYPE *cstr_alloc_##STYPE(long long len);

// Getting sub-slices.
// -------------------
// Since standard C doesn't have statement-expressions, we have to
// jump through a few hoops to avoid evaluating the index calculations
// more than once (which would be disastrous if they are not
// deterministic, e.g. if we slice to random indices for testing).
// That means that we need to use generated functions, even though
// we do exactly the same computation for all types of slices.
// We can then use a _Generic expression to give the slices a
// uniform interface.

// When indexing x[i], if 0 <= i < x.len, we get x.buf[i], and
// if x.len < i <= -1 we get x.buf[x.len - abs(i)], i.e., we
// index from the back.
INLINE long long
cstr_idx(long long i, long long len)
{
  // When i is negative we add it to len to get len - abs(i).
  long long j = i >= 0 ? i : len + i;
  assert(0 <= j && j <= len); // rudementary check when DEBUG flag enabled
  return j;
}

#define CSTR_INDEX_AND_SLICING_GENERATOR(NAME, TYPE)                 \
  INLINE TYPE cstr_idx_##NAME(cstr_##NAME x, long long i)            \
  {                                                                  \
    return x.buf[cstr_idx(i, x.len)];                                \
  }                                                                  \
  INLINE cstr_##NAME cstr_subslice_##NAME(                           \
      cstr_##NAME x, long long i, long long j)                       \
  {                                                                  \
    i = cstr_idx(i, x.len);                                          \
    j = cstr_idx(j, x.len);                                          \
    assert(i <= j);                                                  \
    return (cstr_##NAME)CSTR_SLICE_INIT(x.buf + i, j - i);           \
  }                                                                  \
  INLINE cstr_##NAME cstr_prefix_##NAME(cstr_##NAME x, long long i)  \
  {                                                                  \
    return (cstr_##NAME)CSTR_SLICE_INIT(x.buf, cstr_idx(i, x.len));  \
  }                                                                  \
  INLINE cstr_##NAME cstr_suffix_##NAME(cstr_##NAME x, long long i)  \
  {                                                                  \
    return (cstr_##NAME)CSTR_SLICE_INIT(x.buf + cstr_idx(i, x.len),  \
                                        x.len - cstr_idx(i, x.len)); \
  }

// Define the slice types we need. Variadic to match with
// CSTR_MAP_SLICE_TYPES below.
#define CSTR_DEFINE_SLICE(NAME, QUAL, TYPE)        \
  typedef CSTR_SLICE_TYPE(QUAL TYPE) cstr_##NAME;  \
  CSTR_GEN_SLICE_NEW(NAME, QUAL, TYPE)             \
  CSTR_GEN_ALLOC_SLICE_PROTOTYPE(NAME, QUAL, TYPE) \
  CSTR_INDEX_AND_SLICING_GENERATOR(NAME, TYPE)

// Creating the concrete slice types. To add a new slice type
// you need to define it here. The map is used to generate
// types and do type-based dispatch. Add new types in calls to F
// following the pattern from the other types. To be able to dispatch
// functions based on static types, you must add the new type to the
// dispatch tables.

// clang-format off
CSTR_DEFINE_SLICE(sslice,             ,  uint8_t)
CSTR_DEFINE_SLICE(const_sslice,  const,  uint8_t)
CSTR_DEFINE_SLICE(islice,             ,  int)
CSTR_DEFINE_SLICE(const_islice,  const,  int)
CSTR_DEFINE_SLICE(uislice,            ,  unsigned int)
CSTR_DEFINE_SLICE(const_uislice, const,  unsigned int)
// clang-format on

// clang-format off
#define CSTR_SLICE_DISPATCH(X, FUNC)       \
  _Generic((X),                            \
           cstr_sslice                     \
           : cstr_##FUNC##_sslice,         \
           cstr_const_sslice               \
           : cstr_##FUNC##_const_sslice,   \
           cstr_islice                     \
           : cstr_##FUNC##_islice,         \
           cstr_const_islice               \
           : cstr_##FUNC##_const_islice,   \
           cstr_uislice                    \
           : cstr_##FUNC##_uislice,        \
           cstr_const_uislice              \
           : cstr_##FUNC##_const_uislice)

#define CSTR_BASE_DISPATCH(B, FUNC)        \
  _Generic((B),                            \
           uint8_t *                       \
           : cstr_##FUNC##_sslice,         \
           const uint8_t *                 \
           : cstr_##FUNC##_const_sslice,   \
           int *                           \
           : cstr_##FUNC##_islice,         \
           const int *                     \
           : cstr_##FUNC##_const_islice,   \
           unsigned int *                  \
           : cstr_##FUNC##_uislice,        \
           const unsigned int *            \
           : cstr_##FUNC##_const_uislice)

// The weird (void *) here are to silence the compiler who will warn about
// casting to incorrectly aligned sizes. It doesn't happen, but the compiler
// checks all branches in a _Generic
#define CSTR_SLICE_CONST_CAST(S)                                       \
  _Generic((S),                                                        \
           cstr_sslice                                                 \
           : CSTR_SLICE((const uint8_t *)(void *)(S).buf, (S).len),    \
           cstr_islice                                                 \
           : CSTR_SLICE((const int *)(void *)(S).buf, (S).len),        \
           cstr_uislice                                                \
           : CSTR_SLICE((const unsigned int *)(void *)(S).buf, (S).len))

// x[i] handling both positive and negative indices. Usually,
// x.buf[i] is more natural, if you only need to use positive
// indices.
#define CSTR_IDX(S, I) CSTR_SLICE_DISPATCH(S, idx)(S, I)

// subslice: x => x[i:j]
#define CSTR_SUBSLICE(S, I, J) CSTR_SLICE_DISPATCH(S, subslice)(S, I, J)

// prefix: x => x[0:i] (x[:i])
#define CSTR_PREFIX(S, I) CSTR_SLICE_DISPATCH(S, prefix)(S, I)

// suffix: x => x[i:x.len] (x[i:])
#define CSTR_SUFFIX(S, I) CSTR_SLICE_DISPATCH(S, suffix)(S, I)

#define CSTR_SLICE(BUF, LEN) CSTR_BASE_DISPATCH(BUF, new)(BUF, LEN)

// Special constructor for C-strings to slices.
// With and without including the sentinel (the -0 version includes the
// nul-char as a sentinel).

// These inline functions are needed to avoid _Generic type-checking
// of the non-const part when the input is const.
INLINE cstr_sslice 
cstr_new_sslice_from_string(char *x, long long len) {
  return (cstr_sslice)CSTR_SLICE_INIT((uint8_t *)x, len);
}
INLINE cstr_const_sslice 
cstr_new_const_sslice_from_string(char const *x, long long len) {
    return (cstr_const_sslice)CSTR_SLICE_INIT((const uint8_t *)x, len);
}
#define CSTR_SLICE_STRING_CONSTRUCTOR(STR)                         \
  _Generic((STR),                                                  \
           char *: cstr_new_sslice_from_string,                    \
           const char *: cstr_new_const_sslice_from_string)        \

#define CSTR_SLICE_STRING(STR)                                     \
  CSTR_SLICE_STRING_CONSTRUCTOR(STR)(STR, cstr_strlen(STR))
#define CSTR_SLICE_STRING0(STR)                                    \
  CSTR_SLICE_STRING_CONSTRUCTOR(STR)(STR, cstr_strlen(STR) + 1ll)

// Comparing slices
bool cstr_eq_sslice(cstr_sslice x, cstr_sslice y);
bool cstr_eq_const_sslice(cstr_const_sslice x, cstr_const_sslice y);
bool cstr_eq_islice(cstr_islice x, cstr_islice y);
bool cstr_eq_const_islice(cstr_const_islice x, cstr_const_islice y);
bool cstr_eq_uislice(cstr_uislice x, cstr_uislice y);
bool cstr_eq_const_uislice(cstr_const_uislice x, cstr_const_uislice y);
#define CSTR_SLICE_EQ(A, B) CSTR_SLICE_DISPATCH(A, eq)(A, B)

bool cstr_ge_sslice(cstr_sslice x, cstr_sslice y);
bool cstr_ge_const_sslice(cstr_const_sslice x, cstr_const_sslice y);
bool cstr_ge_islice(cstr_islice x, cstr_islice y);
bool cstr_ge_const_islice(cstr_const_islice x, cstr_const_islice y);
bool cstr_ge_uislice(cstr_uislice x, cstr_uislice y);
bool cstr_ge_const_uislice(cstr_const_uislice x, cstr_const_uislice y);
#define CSTR_SLICE_GE(A, B) CSTR_SLICE_DISPATCH(A, ge)(A, B)

bool cstr_le_sslice(cstr_sslice x, cstr_sslice y);
bool cstr_le_const_sslice(cstr_const_sslice x, cstr_const_sslice y);
bool cstr_le_islice(cstr_islice x, cstr_islice y);
bool cstr_le_const_islice(cstr_const_islice x, cstr_const_islice y);
bool cstr_le_uislice(cstr_uislice x, cstr_uislice y);
bool cstr_le_const_uislice(cstr_const_uislice x, cstr_const_uislice y);
#define CSTR_SLICE_LE(A, B) CSTR_SLICE_DISPATCH(A, le)(A, B)

long long cstr_lcp_sslice(cstr_sslice x, cstr_sslice y);
long long cstr_lcp_const_sslice(cstr_const_sslice x, cstr_const_sslice y);
long long cstr_lcp_islice(cstr_islice x, cstr_islice y);
long long cstr_lcp_const_islice(cstr_const_islice x, cstr_const_islice y);
long long cstr_lcp_uislice(cstr_uislice x, cstr_uislice y);
long long cstr_lcp_const_uislice(cstr_const_uislice x, cstr_const_uislice y);
#define CSTR_SLICE_LCP(A, B) CSTR_SLICE_DISPATCH(A, lcp)(A, B)

// I/O
void cstr_fprint_sslice(FILE *f, cstr_sslice x);
void cstr_fprint_islice(FILE *f, cstr_islice x);
void cstr_fprint_uislice(FILE *f, cstr_uislice x);
void cstr_fprint_const_sslice(FILE *f, cstr_const_sslice x);
void cstr_fprint_const_islice(FILE *f, cstr_const_islice x);
void cstr_fprint_const_uislice(FILE *f, cstr_const_uislice x);
#define CSTR_SLICE_FPRINT(F, S) CSTR_SLICE_DISPATCH(S, fprint)(F, S)
#define CSTR_SLICE_PRINT(S) CSTR_SLICE_DISPATCH(S, fprint)(stdout, S)

// clang-format on

// == BIT VECTOR ===================================================
typedef struct
{
  long long no_bits;
  long long no_words; // you can get this from no_bits, but no need for calculations each time
  uint64_t words[];
} cstr_bit_vector;

#define CSTR_BV_WORD_IDX(BIT_IDX) ((BIT_IDX) >> 6)                 // bit divided by 64
#define CSTR_BV_BIT_IDX(BIT_IDX) ((uint64_t)(BIT_IDX)&0x3f)        // bit % 64 (0x3f == 63)
#define CSTR_BV_WORD(BV, BIT) ((BV)->words[CSTR_BV_WORD_IDX(BIT)]) // pick the word in the vector
#define CSTR_BV_MASK(BIT) (1ull << CSTR_BV_BIT_IDX(BIT))           // mask for the right bit in the word

void cstr_bv_clear(cstr_bit_vector *bv); // Set all bits to 0

INLINE bool cstr_bv_get(cstr_bit_vector *bv, long long bit)
{
  return CSTR_BV_WORD(bv, bit) & CSTR_BV_MASK(bit);
}
INLINE void cstr_bv_set(cstr_bit_vector *bv, long long bit, bool val)
{
  uint64_t mask = CSTR_BV_MASK(bit);
  uint64_t *word = &CSTR_BV_WORD(bv, bit);
  *word = val ? (*word | mask) : (*word & ~mask);
}

cstr_bit_vector *cstr_new_bv(long long no_bits);
cstr_bit_vector *cstr_new_bv_init(long long no_bits);
cstr_bit_vector *cstr_new_bv_from_string(const char *bits);

bool cstr_bv_eq(cstr_bit_vector *a, cstr_bit_vector *b);

void cstr_bv_fprint(FILE *f, cstr_bit_vector *bv);
#define cstr_bv_print(BV) cstr_bv_fprint(stdout, BV)

// == ALPHABET =====================================================

// Alphabets, for when we remap strings to smaller alphabets
#define CSTR_MAX_ALPHABET_SIZE 256
typedef struct cstr_alphabet
{
  unsigned int size;
  uint16_t map[CSTR_MAX_ALPHABET_SIZE];
  uint16_t revmap[CSTR_MAX_ALPHABET_SIZE];
} cstr_alphabet;

// Initialise an alphabet form a slice. Since the alphabet is already
// allocated, this function cannot fail.
void cstr_init_alphabet(cstr_alphabet *alpha, cstr_const_sslice slice);

// Write a mapped string into dst. dst.len must equal src.len
bool cstr_alphabet_map(cstr_sslice dst, cstr_const_sslice src, cstr_alphabet const *alpha);

// Map a slice into an integer slice. dst.len must match src.len + 1
// to make room for a sentinel.
bool cstr_alphabet_map_to_uint(cstr_uislice dst,
                               cstr_const_sslice src,
                               cstr_alphabet const *alpha);

// Map a string back into the dst slice. dst.len must equal src.len.
bool cstr_alphabet_revmap(cstr_sslice dst,
                          cstr_const_sslice src,
                          cstr_alphabet const *alpha);

// == EXACT MATCHERS =============================
// Opaque polymorphic type
typedef struct cstr_exact_matcher cstr_exact_matcher;

// returns -1 when there are no more matches, otherwise an index of a match
int cstr_exact_next_match(cstr_exact_matcher *matcher);
void cstr_free_exact_matcher(cstr_exact_matcher *matcher);

cstr_exact_matcher *cstr_naive_matcher(cstr_sslice x, cstr_sslice p);
cstr_exact_matcher *cstr_ba_matcher(cstr_sslice x, cstr_sslice p);
cstr_exact_matcher *cstr_kmp_matcher(cstr_sslice x, cstr_sslice p);

// == SUFFIX ARRAYS =====================================================
// Suffix arrays stored in uislice objects can only handle lenghts
// up to x.len <= UINT_MAX, and the caller must ensure that. We limit
// ourselves to this size to save space. If we used size_t for the
// values we would likely end up using twice as much memory, and unsigned
// int is likely to be at least 32 bits which means that we can index
// strings of length above four billion. That's most strings we are likely
// to work with.
typedef cstr_uislice cstr_suffix_array;

// Suffix array construction.
// slice x must be mapped to alphabet alpha and slice sa
// must be same length of x. The result will be put
// into sa.
void cstr_skew(cstr_suffix_array sa, cstr_const_uislice x, cstr_alphabet *alpha);
void cstr_sais(cstr_suffix_array sa, cstr_const_uislice x, cstr_alphabet *alpha);

// ==== Suffix trees ==============================================

typedef struct cstr_suffix_tree cstr_suffix_tree;

cstr_suffix_tree *cstr_naive_suffix_tree(cstr_alphabet const *alpha,
                                         cstr_const_sslice x);
void cstr_free_suffix_tree(cstr_suffix_tree *st);

// ==== Burrows-Wheeler transform =================================

uint8_t *cstr_bwt(long long n, uint8_t const *x, unsigned int sa[n]);

struct cstr_bwt_c_table
{
  int asize;
  int cumsum[];
};

struct cstr_bwt_c_table *cstr_compute_bwt_c_table(long long n, uint8_t const *x, int asize);
void cstr_print_bwt_c_table(struct cstr_bwt_c_table const *ctab);
INLINE long long cstr_bwt_c_tab_rank(struct cstr_bwt_c_table const *ctab, uint8_t i)
{
  return ctab->cumsum[i];
}

struct cstr_bwt_o_table;
struct cstr_bwt_o_table *cstr_compute_bwt_o_table(long long n,
                                                  uint8_t const *bwt,
                                                  struct cstr_bwt_c_table const *ctab);
void cstr_print_bwt_o_table(struct cstr_bwt_o_table const *otab);
long long cstr_bwt_o_tab_rank(struct cstr_bwt_o_table const *otab, uint8_t a, long long i);

void cstr_bwt_search(long long *left,
                     long long *right,
                     uint8_t const *x,
                     uint8_t const *p,
                     struct cstr_bwt_c_table const *ctab,
                     struct cstr_bwt_o_table const *otab);

#undef INLINE
#endif // CSTR_INCLUDED
