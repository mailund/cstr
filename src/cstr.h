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
void *cstr_malloc(size_t size) CSTR_MALLOC_FUNC;
// Allocatte a buffer. Terminates if len * sizeof type exceeds
// SIZE_MAX
void *cstr_malloc_buffer(size_t obj_size, size_t len) CSTR_MALLOC_FUNC;
// Allocate a flexible array as part of a struct
void *cstr_malloc_flex_array(size_t base_size, // size of struct before array
                             size_t elm_size,  // size of elements in array
                             size_t len        // number of elements in array
                             ) CSTR_MALLOC_FUNC;

// Error handling, primitive as it is...
enum cstr_errcodes
{
    CSTR_NO_ERROR,
    CSTR_MAPPING_ERROR, // mapping via an alphabet failed
};

// ==== SLICES =====================================================

// slices, for easier handling of sub-strings and sub-arrays.
// These should be passed by value and never dynamically allocated
// (although the underlying buffer can be).
struct cstr_sslice
{
    char *buf;
    size_t len;
};
struct cstr_islice
{
    int *buf;
    size_t len;
};

#define CSTR_SLICE_STRUCT(TYPE, BUF, LEN) ((TYPE){.buf = (BUF), .len = (LEN)})
#define CSTR_SSLICE(BUF, LEN) CSTR_SLICE_STRUCT(struct cstr_sslice, BUF, LEN)
#define CSTR_ISLICE(BUF, LEN) CSTR_SLICE_STRUCT(struct cstr_islice, BUF, LEN)

#define CSTR_SSLICE_STRING(STR) CSTR_SSLICE(STR, strlen(STR))

INLINE struct cstr_sslice
CSTR_ALLOC_SSLICE(size_t len)
{
    struct cstr_sslice dummy; // for size calculation
    return CSTR_SSLICE(cstr_malloc_buffer(sizeof dummy.buf[0], len), len);
}

INLINE struct cstr_islice
CSTR_ALLOC_ISLICE(size_t len)
{
    struct cstr_islice dummy; // for size calculation
    return CSTR_ISLICE(cstr_malloc_buffer(sizeof dummy.buf[0], len), len);
}

#define CSTR_FREE_SLICE_BUFFER(SLICE) \
    do                                \
    {                                 \
        free((SLICE).buf);            \
        (SLICE).buf = 0;              \
        (SLICE).len = 0;              \
    } while (0)

// Comparing string-slices
bool
cstr_sslice_eq(struct cstr_sslice x,
               struct cstr_sslice y);

// == ALPHABET =====================================================

// Alphabets, for when we remap strings to smaller alphabets
struct cstr_alphabet
{
    unsigned int size;
    unsigned char map[CSTR_NO_CHARS];
    unsigned char revmap[CSTR_NO_CHARS];
};

// Initialise an alphabet form a slice. Since the alphabet is already
// allocated, this function cannot fail.
void cstr_init_alphabet(struct cstr_alphabet *alpha,
                        struct cstr_sslice slice);

// Write a mapped string into dst. dst.len must equal src.len
bool cstr_alphabet_map(struct cstr_sslice dst,
                       struct cstr_sslice src,
                       struct cstr_alphabet const *alpha,
                       enum cstr_errcodes *err);

// Map a slice into an integer slice. dst.len must match src.len + 1
// to make room for a sentinel.
bool cstr_alphabet_map_to_int(struct cstr_islice dst,
                              struct cstr_sslice src,
                              struct cstr_alphabet const *alpha,
                              enum cstr_errcodes *err);

// Map a string back into the dst slice. dst.len must equal src.len.
bool cstr_alphabet_revmap(struct cstr_sslice dst,
                          struct cstr_sslice src,
                          struct cstr_alphabet const *alpha,
                          enum cstr_errcodes *err);

// == EXACT MATCHERS =============================
struct cstr_exact_matcher; // Opaque polymorphic type

// returns -1 when there are no more matches, otherwise an index of a match
int cstr_exact_next_match(struct cstr_exact_matcher *matcher);
void cstr_free_exact_matcher(struct cstr_exact_matcher *matcher);

struct cstr_exact_matcher *
cstr_naive_matcher(struct cstr_sslice x, struct cstr_sslice p);
struct cstr_exact_matcher *
cstr_ba_matcher(struct cstr_sslice x, struct cstr_sslice p);
struct cstr_exact_matcher *
cstr_kmp_matcher(struct cstr_sslice x, struct cstr_sslice p);

// == SUFFIX ARRAYS =====================================================
// Suffix arrays stored in islice objects can only handle lenghts up to
// x.len > INT_MAX - 1, and the caller must ensure that.

// Suffix array construction.
// slice x must be mapped to alphabet and slice sa
// must be same length of x. The result will be put
// into sa.
void cstr_skew(struct cstr_islice sa,
               struct cstr_islice x,
               struct cstr_alphabet *alpha);

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
