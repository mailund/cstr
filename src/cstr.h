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
void *cstr_realloc(void *p, size_t size);

void *cstr_malloc_buffer(size_t obj_size,  // size of objects
                         size_t len);      // how many of them
void *cstr_realloc_buffer(void *p,         // existing memory
                          size_t obj_size, // size of objects
                          size_t len);     // how many of them

void *cstr_malloc_header_array(size_t base_size,  // size of struct before array
                               size_t elm_size,   // size of elements in array
                               size_t len);       // number of elements in array
void *cstr_realloc_header_array(void *p,          // existing memory,
                                size_t base_size, // size of struct before array
                                size_t elm_size,  // size of elements in array
                                size_t len);      // number of elements in array

// Macro for getting the offset of a flexible member array
// from an instance rather than a type (as for
// offsetof(type,member)). This ensures we get the right
// type to match the instance. The macro destroys the
// pointer variable by setting it to NULL, so use with care.
// You cannot, for example, use it with a realloc operation,
// since there you would lose the data you already have.
#define CSTR_OFFSETOF_INST(PTR, MEMBER) (size_t)(&((PTR) = 0)->MEMBER)

// Macro for allocating a struct with a flexible array
// element. Gets the offset of the array from a varialble,
// which requires less redundancy and potential for errors
// than offsetof() which requires a type.
// VAR is the struct variable (must be a pointer), FLEX_ARRAY
// is the name of the flexible array member.
#define CSTR_MALLOC_FLEX_ARRAY(VAR, FLEX_ARRAY, LEN) \
  cstr_malloc_header_array(                          \
      CSTR_OFFSETOF_INST(VAR, FLEX_ARRAY),           \
      sizeof((VAR)->FLEX_ARRAY[0]),                  \
      LEN)

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

// A buffer is essentially just a slice that is the owner of its own memory.
// However, we also add a capacity to it, so we can grow it with appending.
// This requires reallocation, which will potentially invalidate any pointers
// into a buffer, so if you store slices into a buffer that you append to,
// you need to re-create the slice whenever you use it. That is what the
// buffer-slice type is for; it uses a pointer to a pointer to a buffer, and that
// extra level of indirection means that we can always get a slice back from
// a buffer.

#define CSTR_BUF_SLICE(STYPE) \
  struct                      \
  {                           \
    cstr_##STYPE##_buf **buf; \
    long long from, to;       \
  }

#define CSTR_GEN_ALLOC_SLICE_PROTOTYPE(STYPE, QUAL, TYPE)                     \
  typedef struct cstr_##STYPE##_buf                                           \
  {                                                                           \
    cstr_##STYPE slice;                                                       \
    long long cap;                                                            \
    QUAL TYPE data[];                                                         \
  } cstr_##STYPE##_buf;                                                       \
                                                                              \
  cstr_##STYPE##_buf *cstr_alloc_##STYPE##_buf(long long len, long long cap); \
                                                                              \
  INLINE cstr_##STYPE *cstr_alloc_##STYPE(long long len)                      \
  {                                                                           \
    return (cstr_##STYPE *)cstr_alloc_##STYPE##_buf(len, len);                \
  }

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
#define CSTR_SLICE_DISPATCH(X, FUNC)         \
  _Generic((X),                              \
           cstr_sslice                       \
           : cstr_##FUNC##_sslice,           \
           cstr_const_sslice                 \
           : cstr_##FUNC##_const_sslice,     \
           cstr_islice                       \
           : cstr_##FUNC##_islice,           \
           cstr_const_islice                 \
           : cstr_##FUNC##_const_islice,     \
           cstr_uislice                      \
           : cstr_##FUNC##_uislice,          \
           cstr_const_uislice                \
           : cstr_##FUNC##_const_uislice)

#define CSTR_SLICE_DISPATCH_MUTABLE(X, FUNC) \
  _Generic((X),                              \
           cstr_sslice                       \
           : cstr_##FUNC##_sslice,           \
           cstr_islice                       \
           : cstr_##FUNC##_islice,           \
           cstr_uislice                      \
           : cstr_##FUNC##_uislice)

#define CSTR_BASE_DISPATCH(B, FUNC)          \
  _Generic((B),                              \
           uint8_t *                         \
           : cstr_##FUNC##_sslice,           \
           const uint8_t *                   \
           : cstr_##FUNC##_const_sslice,     \
           int *                             \
           : cstr_##FUNC##_islice,           \
           const int *                       \
           : cstr_##FUNC##_const_islice,     \
           unsigned int *                    \
           : cstr_##FUNC##_uislice,          \
           const unsigned int *              \
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

// Reversing slices
#define CSTR_GEN_REV_SLICE_PROTOTYPE(STYPE) \
  void cstr_rev_##STYPE(cstr_##STYPE);
CSTR_GEN_REV_SLICE_PROTOTYPE(sslice)
CSTR_GEN_REV_SLICE_PROTOTYPE(islice)
CSTR_GEN_REV_SLICE_PROTOTYPE(uislice)
#define CSTR_REV_SLICE(S) CSTR_SLICE_DISPATCH_MUTABLE(S, rev)(S)

// I/O
void cstr_fprint_sslice(FILE *f, cstr_sslice x);
void cstr_fprint_islice(FILE *f, cstr_islice x);
void cstr_fprint_uislice(FILE *f, cstr_uislice x);
void cstr_fprint_const_sslice(FILE *f, cstr_const_sslice x);
void cstr_fprint_const_islice(FILE *f, cstr_const_islice x);
void cstr_fprint_const_uislice(FILE *f, cstr_const_uislice x);
#define CSTR_SLICE_FPRINT(F, S) CSTR_SLICE_DISPATCH(S, fprint)(F, S)
#define CSTR_SLICE_PRINT(S) CSTR_SLICE_DISPATCH(S, fprint)(stdout, S)

#define CSTR_BUF_SLICE_DEREF_FUNC(STYPE, QUAL, TYPE)                                     \
  typedef CSTR_BUF_SLICE(STYPE) cstr_##STYPE##_buf_slice;                                \
  INLINE cstr_##STYPE cstr_deref_##STYPE##_buf_slice(cstr_##STYPE##_buf_slice buf_slice) \
  {                                                                                      \
    assert(buf_slice.to <= (*buf_slice.buf)->cap);                                       \
    return CSTR_SUBSLICE((*buf_slice.buf)->slice, buf_slice.from, buf_slice.to);         \
  }

// Both appending to a buffer or a buffer slice modifies the underlying buffer.
// If you append to a buffer, you append, well, to the buffer. If you append to a buffer
// slice, you reduce the buffer to the end point of the slice and then append.
#define CSTR_BUF_APPEND_PROTOTYPE(STYPE, QUAL, TYPE)                                      \
  cstr_##STYPE##_buf_slice cstr_append_##STYPE##_buf(cstr_##STYPE##_buf **buf, TYPE val); \
  cstr_##STYPE##_buf_slice cstr_append_##STYPE##_buf_slice(cstr_##STYPE##_buf_slice buf_slice, TYPE val);

CSTR_BUF_SLICE_DEREF_FUNC(sslice,             ,  uint8_t)
CSTR_BUF_SLICE_DEREF_FUNC(const_sslice,  const,  uint8_t)
CSTR_BUF_SLICE_DEREF_FUNC(islice,             ,  int)
CSTR_BUF_SLICE_DEREF_FUNC(const_islice,  const,  int)
CSTR_BUF_SLICE_DEREF_FUNC(uislice,            ,  unsigned int)
CSTR_BUF_SLICE_DEREF_FUNC(const_uislice, const,  unsigned int)

CSTR_BUF_APPEND_PROTOTYPE(sslice,  , uint8_t)
CSTR_BUF_APPEND_PROTOTYPE(islice,  , int)
CSTR_BUF_APPEND_PROTOTYPE(uislice, , unsigned int)


#define CSTR_BUF_DISPATCH_MUTABLE(X, FUNC)   \
  _Generic((X),                              \
           cstr_sslice_buf *                 \
           : cstr_##FUNC##_sslice_buf,       \
           cstr_islice_buf *                 \
           : cstr_##FUNC##_islice_buf,       \
           cstr_uislice_buf *                \
           : cstr_##FUNC##_uislice_buf)

#define CSTR_BUF_SLICE_DISPATCH(X, FUNC)       \
  _Generic((X),                                \
           cstr_sslice_buf_slice               \
           : cstr_##FUNC##_sslice_buf_slice,   \
           cstr_islice_buf_slice               \
           : cstr_##FUNC##_islice_buf_slice,   \
           cstr_uislice_buf_slice              \
           : cstr_##FUNC##_uislice_buf_slice)

#define CSTR_BUF_APPEND(B, V) CSTR_BUF_DISPATCH_MUTABLE(B, append)(&(B), V)
#define CSTR_BUF_SLICE_APPEND(BS, V) CSTR_BUF_SLICE_DISPATCH(BS, append)(BS, V)
#define CSTR_BUF_SLICE_DEREF(BS) CSTR_BUF_SLICE_DISPATCH(BS, deref)(BS)

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
// Polymorphic structure for exact matching
typedef struct
{ // Embed this in matchers (with the right v-table)
  struct cstr_exact_matcher_vtab *vtab;
} cstr_exact_matcher;

typedef struct cstr_exact_matcher_vtab
{ // Virtual table for dynamic dispatching
  // Implements iteration
  long long (*next)(cstr_exact_matcher *);
  // Implements destruction
  void (*free)(cstr_exact_matcher *);
} cstr_exact_matcher_vtab;

// clang-format off
// returns -1 when there are no more matches, otherwise an index of a match
INLINE long long cstr_exact_next_match(cstr_exact_matcher *self)   { return self->vtab->next(self); }
INLINE void      cstr_free_exact_matcher(cstr_exact_matcher *self) { self->vtab->free(self); }
// clang-format on

cstr_exact_matcher *cstr_naive_matcher(cstr_const_sslice x, cstr_const_sslice p);
cstr_exact_matcher *cstr_ba_matcher(cstr_const_sslice x, cstr_const_sslice p);
cstr_exact_matcher *cstr_kmp_matcher(cstr_const_sslice x, cstr_const_sslice p);

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

cstr_exact_matcher *cstr_sa_bsearch(cstr_suffix_array sa, cstr_const_sslice x, cstr_const_sslice p);

// ==== Suffix trees ==============================================

typedef struct cstr_suffix_tree cstr_suffix_tree;

cstr_suffix_tree *cstr_naive_suffix_tree(cstr_alphabet const *alpha,
                                         cstr_const_sslice x);
cstr_suffix_tree *cstr_mccreight_suffix_tree(cstr_alphabet const *alpha,
                                             cstr_const_sslice x);

void cstr_free_suffix_tree(cstr_suffix_tree *st);

typedef struct cstr_st_leaf_iter cstr_st_leaf_iter;
long long cstr_st_leaf_iter_next(cstr_st_leaf_iter *iter);
void cstr_free_st_leaf_iter(cstr_st_leaf_iter *iter);

// The first assumes that p is already mapped to the alphabet, the second will
// do the mapping itself.
cstr_exact_matcher *cstr_st_exact_search(cstr_suffix_tree *st, cstr_const_sslice p);
cstr_exact_matcher *cstr_st_exact_search_map(cstr_suffix_tree *st, cstr_const_sslice p);

// ==== Burrows-Wheeler transform =================================

void cstr_bwt(cstr_sslice bwt, cstr_const_sslice x, cstr_suffix_array sa);
void cstr_reverse_bwt(cstr_sslice rev, cstr_const_sslice bwt, cstr_suffix_array sa);

typedef struct cstr_bwt_preproc cstr_bwt_preproc; // Preprocessed tables for searching using FM-index
// Does all the preprocessing, including mapping the alphabet, building the suffix array,
// and building the tables for searching. You could save some time, if you already did
// some of the preprocessing elsewhere, by writing a function that does somewhat less.
cstr_bwt_preproc *cstr_bwt_preprocess(cstr_const_sslice x);

// This matcher does not assume that p is already mapped. It does assume that you have built
// the preproc tables.
cstr_exact_matcher *cstr_fmindex_search(cstr_bwt_preproc *preproc, cstr_const_sslice p);

void cstr_free_bwt_preproc(struct cstr_bwt_preproc *preproc);

// ==== Li-Durbin approximative matching ==========================
typedef struct cstr_li_durbin_preproc cstr_li_durbin_preproc;
cstr_li_durbin_preproc *cstr_li_durbin_preprocess(cstr_const_sslice x);
void cstr_free_li_durbin_preproc(cstr_li_durbin_preproc *preproc);

typedef struct cstr_approx_match
{
  long long pos;     // -1 if no more matches
  const char *cigar; // CIGAR, as a C string we can readily print.
} cstr_approx_match;

typedef struct cstr_approx_matcher cstr_approx_matcher;
cstr_approx_matcher *cstr_li_durbin_search(cstr_li_durbin_preproc *preproc,
                                           cstr_const_sslice p, long long d);
cstr_approx_match cstr_approx_next_match(cstr_approx_matcher *matcher);
void cstr_free_approx_matcher(cstr_approx_matcher *matcher);



#undef INLINE
#endif // CSTR_INCLUDED
