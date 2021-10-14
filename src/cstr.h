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
    struct                    \
    {                         \
        signed long long len; \
        TYPE *buf;            \
    }

// Creating slice instances
#define CSTR_SLICE_INIT(BUF, LEN)  \
    {                              \
        .buf = (BUF), .len = (LEN) \
    }

// We can create structs with macros, but we need to cast
// to the slice type for composite expressions, which requires
// some _Generic hacks. It is easier to generate inline functions
// and use the dispatch mechanism below.
#define CSTR_SLICE_NEW_GENERATOR(NAME, TYPE)           \
    INLINE cstr_##NAME                                 \
        cstr_new_##NAME(TYPE *buf, long long len)      \
    {                                                  \
        return (cstr_##NAME)CSTR_SLICE_INIT(buf, len); \
    }

// Memory management
// -----------------
// Using inline functions for allocation so we don't risk
// evaluating the length expression twice.
#define CSTR_BUFFER_ALLOC_GENERATOR(TYPE)                         \
    INLINE cstr_##TYPE cstr_alloc_buffer_##TYPE(long long len)    \
    {                                                             \
        cstr_##TYPE dummy; /* use dummy to get underlying type */ \
        return (cstr_##TYPE)CSTR_SLICE_INIT(                      \
            cstr_malloc_buffer(sizeof dummy.buf[0], (size_t)len), \
            len);                                                 \
    }

#define CSTR_FREE_SLICE_BUFFER(SLICE) \
    do                                \
    {                                 \
        free((SLICE).buf);            \
        (SLICE).buf = 0;              \
        (SLICE).len = 0;              \
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
INLINE long long cstr_idx(long long i, long long len)
{
    // When i is negative we add it to len to get len - abs(i).
    long long j = i >= 0 ? i : len + i;
    assert(0 <= j && j <= len); // rudementary check when DEBUG flag enabled
    return j;
}

#define CSTR_INDEX_AND_SLICING_GENERATOR(NAME, TYPE)                     \
    INLINE TYPE                                                          \
        cstr_idx_##NAME(cstr_##NAME x, long long i)                      \
    {                                                                    \
        return x.buf[cstr_idx(i, x.len)];                                \
    }                                                                    \
    INLINE cstr_##NAME                                                   \
        cstr_subslice_##NAME(cstr_##NAME x,                              \
                             long long i, long long j)                   \
    {                                                                    \
        i = cstr_idx(i, x.len);                                          \
        j = cstr_idx(j, x.len);                                          \
        assert(i <= j);                                                  \
        return (cstr_##NAME)CSTR_SLICE_INIT(x.buf + i, j - i);           \
    }                                                                    \
    INLINE cstr_##NAME                                                   \
        cstr_prefix_##NAME(cstr_##NAME x, long long i)                   \
    {                                                                    \
        return (cstr_##NAME)CSTR_SLICE_INIT(x.buf, cstr_idx(i, x.len));  \
    }                                                                    \
    INLINE cstr_##NAME                                                   \
        cstr_suffix_##NAME(cstr_##NAME x, long long i)                   \
    {                                                                    \
        return (cstr_##NAME)CSTR_SLICE_INIT(x.buf + cstr_idx(i, x.len),  \
                                            x.len - cstr_idx(i, x.len)); \
    }

// Define the slice types we need.
#define CSTR_DEFINE_SLICE(NAME, TYPE)          \
    typedef CSTR_SLICE_TYPE(TYPE) cstr_##NAME; \
    CSTR_SLICE_NEW_GENERATOR(NAME, TYPE)       \
    CSTR_BUFFER_ALLOC_GENERATOR(NAME)          \
    CSTR_INDEX_AND_SLICING_GENERATOR(NAME, TYPE)

// Creating the concrete slice types. To add a new slice type
// you need to define it here, add a "base macro", and add the
// type to the dispatch table. Unfortunately, we cannot do it
// all in the type definition macro, since macros cannot create
// new macros.
CSTR_DEFINE_SLICE(sslice, char)
CSTR_DEFINE_SLICE(islice, int)
CSTR_DEFINE_SLICE(uislice, unsigned int)

// Need these to map from a slice type to the underlying
// type when dispatching. We cannot define macros in macros,
// so this is, unfortunately, not something we can do with
// CSTTR_DEFINE_SLICE, which would otherwise be ideal.
#define CSTR_SLICE_BASE_sslice char
#define CSTR_SLICE_BASE_islice int
#define CSTR_SLICE_BASE_uislice unsigned int
#define CSTR_SLICE_BASE(TYPE) CSTR_SLICE_BASE_##TYPE

// This macro needs a dispatch map for each slice type
#define CSTR_DISPATCH_TABLE(FUNC, MTYPE)        \
    CSTR_DISPATCH_MAP(sslice, FUNC, MTYPE),     \
        CSTR_DISPATCH_MAP(islice, FUNC, MTYPE), \
        CSTR_DISPATCH_MAP(uislice, FUNC, MTYPE)

// Type-based static dispatch.
// ---------------------------
// Select a function based on the type of S and
// the dispatch table in, then call it with the
// remaining arguments.

// Maps a type to the corresponding function
#define CSTR_DISPATCH_MAP_(TYPE, FUNC) \
    cstr_##TYPE : cstr_##FUNC##_##TYPE
#define CSTR_DISPATCH_MAP_B(TYPE, FUNC) \
    CSTR_SLICE_BASE(TYPE) * : cstr_##FUNC##_##TYPE
#define CSTR_DISPATCH_MAP(TYPE, FUNC, MTYPE) \
    CSTR_DISPATCH_MAP_##MTYPE(TYPE, FUNC)

// Dispatch a function based on the type of X
#define CSTR_SLICE_DISPATCH(X, B, FUNC, ...) \
    _Generic((X), CSTR_DISPATCH_TABLE(FUNC, B))(__VA_ARGS__)

// ... returning from macro programming madness...
// Ph'nglui mglw'nafh Cthulhu R'lyeh wgah'nagl fhtagn

// If you have a variable you intend to assign a freshly allocated
// slice-buffer to, you can use this macro to automatically pick the
// right function from the type
#define CSTR_ALLOC_SLICE_BUFFER(S, LEN) \
    CSTR_SLICE_DISPATCH(S, , alloc_buffer, LEN)

// x[i] handling both positive and negative indices. Usually,
// x.buf[i] is more natural, if you only need to use positive
// indices.
#define CSTR_IDX(S, I) \
    CSTR_SLICE_DISPATCH(S, , idx, S, I)

// subslice: x => x[i:j]
#define CSTR_SUBSLICE(S, I, J) \
    CSTR_SLICE_DISPATCH(S, , subslice, S, I, J)

// prefix: x => x[0:i] (x[:i])
#define CSTR_PREFIX(S, I) \
    CSTR_SLICE_DISPATCH(S, , prefix, S, I)

// suffix: x => x[i:x.len] (x[i:])
#define CSTR_SUFFIX(S, I) \
    CSTR_SLICE_DISPATCH(S, , suffix, S, I)

#define CSTR_SLICE(BUF, LEN) \
    CSTR_SLICE_DISPATCH(BUF, B, new, BUF, LEN)

// Special constructor for C-strings to slices.
#define CSTR_SLICE_STRING(STR) \
    CSTR_SLICE(STR, strlen(STR))

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

cstr_exact_matcher *cstr_naive_matcher(cstr_sslice x, cstr_sslice p);
cstr_exact_matcher *cstr_ba_matcher(cstr_sslice x, cstr_sslice p);
cstr_exact_matcher *cstr_kmp_matcher(cstr_sslice x, cstr_sslice p);

// == SUFFIX ARRAYS =====================================================
// Suffix arrays stored in uislice objects can only handle lenghts
// up to x.len <= UINT_MAX, and the caller must ensure that.
typedef cstr_uislice cstr_suffix_array;

// Suffix array construction.
// slice x must be mapped to alphabet and slice sa
// must be same length of x. The result will be put
// into sa.
void cstr_skew(cstr_suffix_array sa,
               cstr_uislice x,
               cstr_alphabet *alpha);

// ==== Burrows-Wheeler transform =================================

char *cstr_bwt(int n, char const *x, unsigned int sa[n]);

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
