#ifndef CSTR_INCLUDED
#define CSTR_INCLUDED

#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// The number of characters we have for char * strings.
// WARNING: if this is too large, the alphabet structure
// will suffer, but a char is likely to be 8 bits. You can
// check it with a static assert if you like.
#define CSTR_NO_CHARS (1 << CHAR_BIT)

// Many places, it is more convinient to work with bytes (where
// we know their size) than to work with char, so generally we use
// uint8_t instead of char. That requires a few casts here and there
// but only when we interact with the outer world.
// The only real consern is that functions such as strlen returns
// lengths in char and not bytes, if the two differs. So we need
// our own version of that. Be careful, however, if you want to use
// the nul-char ('\0') as a sentinel. It may not be the null byte!
size_t cstr_strlen(const char *x); // returns length of x in bytes
#define CSTR_STR_TO_BYTES(X) ((uint8_t *)(X))
#define CSTR_CONST_STR_TO_CONST_BYTES(X) ((const uint8_t *)(X))
// WARNING: only valid if nul-terminated!
#define CSTR_BYTES_TO_STR(X) ((char *)(X))
#define CSTR_CONST_BYTES_TO_CONST_STR(X) ((const char *)(X))

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
#define CSTR_OFFSETOF_INST(PTR, MEMBER) (size_t)(&(PTR = 0)->MEMBER)

// Macro for allocating a struct with a flexible array
// element. Gets the offset of the array from a varialble,
// which requires less redundancy and potential for errors
// than offsetof() which requires a type.
// VAR is the struct variable (must be a pointer), FLEX_ARRAY
// is the name of the flexible array member.
#define CSTR_MALLOC_FLEX_ARRAY(VAR, FLEX_ARRAY, LEN) \
  cstr_malloc_header_array(                          \
      CSTR_OFFSETOF_INST(VAR, FLEX_ARRAY), sizeof(VAR->FLEX_ARRAY[0]), LEN)

// Set a pointer to NULL when we free it
#define CSTR_FREE_NULL(P) \
  do                      \
  {                       \
    free(P);              \
    P = 0;                \
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
#define CSTR_SLICE_TYPE(TYPE) \
  struct                      \
  {                           \
    signed long long len;     \
    TYPE *buf;                \
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
#define CSTR_SLICE_NEW_GENERATOR(NAME, TYPE)                   \
  INLINE cstr_##NAME cstr_new_##NAME(TYPE *buf, long long len) \
  {                                                            \
    return (cstr_##NAME)CSTR_SLICE_INIT(buf, len);             \
  }

// Memory management
// -----------------
// Using inline functions for allocation so we don't risk
// evaluating the length expression twice.
#define CSTR_BUFFER_ALLOC_GENERATOR(TYPE)                           \
  INLINE cstr_##TYPE cstr_alloc_buffer_##TYPE(long long len)        \
  {                                                                 \
    cstr_##TYPE dummy; /* use dummy to get underlying type */       \
    return (cstr_##TYPE)CSTR_SLICE_INIT(                            \
        cstr_malloc_buffer(sizeof dummy.buf[0], (size_t)len), len); \
  }

#define CSTR_FREE_SLICE_BUFFER(SLICE) \
  do                                  \
  {                                   \
    free((SLICE).buf);                \
    (SLICE).buf = 0;                  \
    (SLICE).len = 0;                  \
  } while (0)

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
#define CSTR_DEFINE_SLICE(NAME, TYPE, ...)   \
  typedef CSTR_SLICE_TYPE(TYPE) cstr_##NAME; \
  CSTR_SLICE_NEW_GENERATOR(NAME, TYPE)       \
  CSTR_BUFFER_ALLOC_GENERATOR(NAME)          \
  CSTR_INDEX_AND_SLICING_GENERATOR(NAME, TYPE)

// Creating the concrete slice types. To add a new slice type
// you need to define it here. The map is used to generate
// types and do type-based dispatch. Add new types in calls to F
// following the pattern from the other types. To be able to dispatch
// functions based on static types, you must add the new type to the
// dispatch tables.

// clang-format off
CSTR_DEFINE_SLICE(sslice, uint8_t); // bytes for chars for convinience
CSTR_DEFINE_SLICE(islice, int);
CSTR_DEFINE_SLICE(uislice, unsigned int);
#define CSTR_SLICE_DISPATCH(X, FUNC)     \
  _Generic((X),                          \
           cstr_sslice                   \
           : cstr_##FUNC##_sslice,       \
           cstr_islice                   \
           : cstr_##FUNC##_islice,       \
           cstr_uislice                  \
           : cstr_##FUNC##_uislice)
#define CSTR_BASE_DISPATCH(B, FUNC)      \
  _Generic((B),                          \
           uint8_t *                     \
           : cstr_##FUNC##_sslice,       \
           int *                         \
           : cstr_##FUNC##_islice,       \
           unsigned int *                \
           : cstr_##FUNC##_uislice)

// If you have a variable you intend to assign a freshly allocated
// slice-buffer to, you can use this macro to automatically pick the
// right function from the type
#define CSTR_ALLOC_SLICE_BUFFER(S, LEN) CSTR_SLICE_DISPATCH(S, alloc_buffer)(LEN)

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
// With and without including the sentinel
#define CSTR_SLICE_STRING(STR) CSTR_SLICE((uint8_t*)STR, cstr_strlen(STR))
#define CSTR_SLICE_STRING0(STR) CSTR_SLICE((uint8_t*)STR, cstr_strlen(STR) + 1)

// Comparing slices
bool cstr_eq_sslice(cstr_sslice x, cstr_sslice y);
bool cstr_eq_islice(cstr_islice x, cstr_islice y);
bool cstr_eq_uislice(cstr_uislice x, cstr_uislice y);

#define CSTR_SLICE_EQ(A, B) CSTR_SLICE_DISPATCH(A, eq)(A, B)


// I/O
void cstr_fprint_sslice(FILE *f, cstr_sslice x);
#define cstr_print_sslice(X) cstr_fprint_sslice(stdout, X)
// clang-format on

// == ALPHABET =====================================================

// Alphabets, for when we remap strings to smaller alphabets
typedef struct cstr_alphabet
{
  unsigned int size;
  uint16_t map[CSTR_NO_CHARS];
  uint16_t revmap[CSTR_NO_CHARS];
} cstr_alphabet;

// Initialise an alphabet form a slice. Since the alphabet is already
// allocated, this function cannot fail.
void cstr_init_alphabet(cstr_alphabet *alpha, cstr_sslice slice);

// Write a mapped string into dst. dst.len must equal src.len
bool cstr_alphabet_map(cstr_sslice dst, cstr_sslice src, cstr_alphabet const *alpha);

// Map a slice into an integer slice. dst.len must match src.len + 1
// to make room for a sentinel.
bool cstr_alphabet_map_to_uint(cstr_uislice dst,
                               cstr_sslice src,
                               cstr_alphabet const *alpha);

// Map a string back into the dst slice. dst.len must equal src.len.
bool cstr_alphabet_revmap(cstr_sslice dst,
                          cstr_sslice src,
                          cstr_alphabet const *alpha);

// == EXACT MATCHERS =============================
// Opaque polymorphic type
typedef struct cstr_exact_matcher cstr_exact_matcher;

// returns -1 when there are no more matches, otherwise an index of a match
int cstr_exact_next_match(cstr_exact_matcher *matcher);
void cstr_free_exact_matcher(cstr_exact_matcher *matcher);

cstr_exact_matcher *
cstr_naive_matcher(cstr_sslice x, cstr_sslice p);
cstr_exact_matcher *
cstr_ba_matcher(cstr_sslice x, cstr_sslice p);
cstr_exact_matcher *
cstr_kmp_matcher(cstr_sslice x, cstr_sslice p);

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
// slice x must be mapped to alphabet and slice sa
// must be same length of x. The result will be put
// into sa.
void cstr_skew(cstr_suffix_array sa, cstr_uislice x, cstr_alphabet *alpha);

// ==== Burrows-Wheeler transform =================================

uint8_t *cstr_bwt(int n, uint8_t const *x, unsigned int sa[n]);

struct cstr_bwt_c_table
{
  int asize;
  int cumsum[];
};

struct cstr_bwt_c_table *cstr_compute_bwt_c_table(int n, uint8_t const *x, int asize);
void cstr_print_bwt_c_table(struct cstr_bwt_c_table const *ctab);
INLINE int cstr_bwt_c_tab_rank(struct cstr_bwt_c_table const *ctab, uint8_t i)
{
  return ctab->cumsum[i];
}

struct cstr_bwt_o_table;
struct cstr_bwt_o_table *cstr_compute_bwt_o_table(int n,
                                                  uint8_t const *bwt,
                                                  struct cstr_bwt_c_table const *ctab);
void cstr_print_bwt_o_table(struct cstr_bwt_o_table const *otab);
int cstr_bwt_o_tab_rank(struct cstr_bwt_o_table const *otab, uint8_t a, int i);

void cstr_bwt_search(int *left,
                     int *right,
                     uint8_t const *x,
                     uint8_t const *p,
                     struct cstr_bwt_c_table const *ctab,
                     struct cstr_bwt_o_table const *otab);

#undef INLINE
#endif // CSTR_INCLUDED
