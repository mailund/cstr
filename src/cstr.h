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

// This is to provide inline functions without putting the
// static code in each output file...
#ifndef INLINE
#define INLINE inline
#endif

// We can give the compiler hints, if it understands them...
#if defined(__GNUC__) || defined(__clang__)
#define CSTR_FUNC_ATTR(a) __attribute__((a))
#else
#define CSTR_FUNC_ATTR(a) /* ignore */
#endif

// Tell the compiler that we return clean new memory
#define CSTR_MALLOC_FUNC CSTR_FUNC_ATTR(malloc)
// Tell the compiler that a function is pure (only depend
// on input to determine output, so completely deterministic).
#define CSTR_PURE_FUNC CSTR_FUNC_ATTR(pure)

// Allocation that cannot fail (except by terminating the program).
// With this, we don't need to test for allocation errors.
void *cstr_malloc(
    size_t size // number of chars to allocate
    ) CSTR_MALLOC_FUNC;
void *cstr_malloc_buffer(
    size_t obj_size, // size of objects
    size_t len       // how many of them
    ) CSTR_MALLOC_FUNC;
void *cstr_malloc_header_array(
    size_t base_size, // size of struct before array
    size_t elm_size,  // size of elements in array
    size_t len        // number of elements in array
    ) CSTR_MALLOC_FUNC;

// Macro for getting the offset of a flexible member array
// from an instance rather than a type (as for
// offsetof(type,member)). This ensures we get the right
// type to match the instance. The macro destroys the
// pointer variable by setting it to NULL, so use with care.
#define CSTR_OFFSETOF_INST(PTR, MEMBER) \
    (size_t)(&(PTR = 0)->MEMBER)

// Macro for allocating a struct with a flexible array
// element. Gets the offset of the array from a varialble,
// which requires less redundancy and potential for errors
// than offsetof() which requires a type.
// VAR is the struct variable (must be a pointer), FLEX_ARRAY
// is the name of the flexible array member.
#define CSTR_MALLOC_FLEX_ARRAY(VAR, FLEX_ARRAY, LEN) \
    cstr_malloc_header_array(                        \
        CSTR_OFFSETOF_INST(VAR, FLEX_ARRAY),         \
        sizeof(VAR->FLEX_ARRAY[0]), LEN)

// Set a pointer to NULL when we free it
#define CSTR_FREE_NULL(P) \
    do                    \
    {                     \
        free(P);          \
        P = 0;            \
    } while (0)

// ==== SLICES =====================================================

// Slices, for easier handling of sub-strings and sub-arrays.
// These should be passed by value and never dynamically allocated
// (although the underlying buffer can be).
// The most natural type for a length would be size_t, but we will
// allow indexing with negative numbers, so we use a signed long long.
// That will be at least 64 bits, so even if signed will be enough
// for any sane amount of data. Lengths should still be non-negative,
// of course
#define CSTR_SLICE_TYPE(MEMBER_TYPE) \
    struct                           \
    {                                \
        signed long long len;        \
        MEMBER_TYPE *buf;            \
    }

typedef CSTR_SLICE_TYPE(char) cstr_sslice;
typedef CSTR_SLICE_TYPE(int) cstr_islice;

// Creating slice instances
#define CSTR_SLICE_INIT(BUF, LEN)  \
    {                              \
        .buf = (BUF), .len = (LEN) \
    }

// Generic slice construction; the (void *) cast is necessary
// to get around the rules for _Generic.
#define CSTR_SLICE(BUF, LEN)                                   \
    _Generic((BUF),                                            \
             char *                                            \
             : (cstr_sslice)CSTR_SLICE_INIT((void *)BUF, LEN), \
               int *                                           \
             : (cstr_islice)CSTR_SLICE_INIT((void *)BUF, LEN))

#define CSTR_SLICE_STRING(STR) CSTR_SLICE(STR, strlen(STR))

// Getting sub-slices.
// Since standard C doesn't have statement-expressions, we are
// relying heavily on the compiler optimising away repeated calls
// to the index calculations, but this should not be a problem for
// it with a simple inlined function.

// When indexing x[i], if 0 <= i < x.len, we get x.buf[i], and
// if x.len < i <= -1 we get x.buf[x.len - abs(i)], i.e., we
// index from the back.
INLINE long long cstr_idx(long long i, long long len)
{
    // When i is negative we add it to len to get len - abs(i).
    long long j = i >= 0 ? i : len + i;
    assert(0 <= j && j <= len); // rudementary check when DEBUG flag enabled
    return j;
}

// x[i] handling both positive and negative indices. Usually,
// x.buf[i] is more natural, if you only need to use positive
// indices
#define CSTR_IDX(S, I) ((S).buf[cstr_idx(I, (S).len)])

// x => x[i:j].
#define CSTR_SUBSLICE(S, I, J)                             \
    (assert(cstr_idx(I, (S).len) <= cstr_idx(J, (S).len)), \
     CSTR_SLICE((S).buf + cstr_idx(I, (S).len),            \
                cstr_idx(J, (S).len) - cstr_idx(I, (S).len)))

// x => x[0:i] (x[:i])
#define CSTR_PREFIX(S, I) \
    CSTR_SLICE((S).buf, cstr_idx(I, (S).len))

// x => x[i:x.len] (x[i:])
#define CSTR_SUFFIX(S, I)                      \
    CSTR_SLICE((S).buf + cstr_idx(I, (S).len), \
               (S).len - cstr_idx(I, (S).len))

// Using inline functions for allocation so we don't risk
// evaluating the length expression twice.
#define CSTR_BUFFER_ALLOC_GENERATOR(TYPE)                         \
    INLINE cstr_##TYPE cstr_alloc_##TYPE##_buffer(long long len)  \
    {                                                             \
        cstr_##TYPE dummy; /* use dummy to get underlying type */ \
        return (cstr_##TYPE)CSTR_SLICE_INIT(                      \
            cstr_malloc_buffer(sizeof dummy.buf[0], (size_t)len), \
            len);                                                 \
    }

// for each type T we get cstr_alloc_T_buffer(size_t len)
CSTR_BUFFER_ALLOC_GENERATOR(sslice)
CSTR_BUFFER_ALLOC_GENERATOR(islice)

#define CSTR_FREE_SLICE_BUFFER(SLICE) \
    do                                \
    {                                 \
        free((SLICE).buf);            \
        (SLICE).buf = 0;              \
        (SLICE).len = 0;              \
    } while (0)

// Comparing string-slices
bool cstr_sslice_eq(cstr_sslice x, cstr_sslice y);

// I/O
void cstr_fprint_sslice(FILE *f, cstr_sslice x);
#define cstr_print_sslice(X) cstr_fprint_sslice(stdout, X)

// == ALPHABET =====================================================

// Alphabets, for when we remap strings to smaller alphabets
typedef struct cstr_alphabet
{
    unsigned int size;
    unsigned char map[CSTR_NO_CHARS];
    unsigned char revmap[CSTR_NO_CHARS];
} cstr_alphabet;

// Initialise an alphabet form a slice. Since the alphabet is already
// allocated, this function cannot fail.
void cstr_init_alphabet(cstr_alphabet *alpha,
                        cstr_sslice slice);

// Write a mapped string into dst. dst.len must equal src.len
bool cstr_alphabet_map(cstr_sslice dst,
                       cstr_sslice src,
                       cstr_alphabet const *alpha);

// Map a slice into an integer slice. dst.len must match src.len + 1
// to make room for a sentinel.
bool cstr_alphabet_map_to_int(cstr_islice dst,
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

cstr_exact_matcher *cstr_naive_matcher(cstr_sslice x, cstr_sslice p);
cstr_exact_matcher *cstr_ba_matcher(cstr_sslice x, cstr_sslice p);
cstr_exact_matcher *cstr_kmp_matcher(cstr_sslice x, cstr_sslice p);

// == SUFFIX ARRAYS =====================================================
// Suffix arrays stored in islice objects can only handle lenghts
// up to x.len > INT_MAX - 1, and the caller must ensure that.

// Suffix array construction.
// slice x must be mapped to alphabet and slice sa
// must be same length of x. The result will be put
// into sa.
void cstr_skew(cstr_islice sa,
               cstr_islice x,
               cstr_alphabet *alpha);

// ==== Burrows-Wheeler transform =================================

char *cstr_bwt(int n, char const *x, int sa[n]);

struct cstr_bwt_c_table
{
    int asize;
    int cumsum[];
};

struct cstr_bwt_c_table *cstr_compute_bwt_c_table(int n, char const *x,
                                                  int asize);
void cstr_print_bwt_c_table(struct cstr_bwt_c_table const *ctab);
INLINE int cstr_bwt_c_tab_rank(struct cstr_bwt_c_table const *ctab, char i)
{
    return ctab->cumsum[(int)i];
}

struct cstr_bwt_o_table;
struct cstr_bwt_o_table *
cstr_compute_bwt_o_table(int n, char const *bwt,
                         struct cstr_bwt_c_table const *ctab);
void cstr_print_bwt_o_table(struct cstr_bwt_o_table const *otab);
int cstr_bwt_o_tab_rank(struct cstr_bwt_o_table const *otab, char a, int i);

void cstr_bwt_search(int *left, int *right, char const *x, char const *p,
                     struct cstr_bwt_c_table const *ctab,
                     struct cstr_bwt_o_table const *otab);

#undef INLINE
#endif // CSTR_INCLUDED
