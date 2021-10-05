#ifndef CSTR_INCLUDED
#define CSTR_INCLUDED

#include <assert.h>
#include <limits.h>
#include <stdbool.h>
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

#define CSTR_MALLOC_FUNC CSTR_FUNC_ATTR(malloc)
#define CSTR_PURE_FUNC CSTR_FUNC_ATTR(pure)

// Error handling, primitive as it is...
enum cstr_errcodes
{
    CSTR_NO_ERROR,
    CSTR_ALLOCATION_ERROR, // malloc or similar failed
    CSTR_SIZE_ERROR,       // if the size/length of something is too large
    CSTR_MAPPING_ERROR,    // mapping via an alphabet failed
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

// FIXME Hack to get a typeof()
#if 0
#define CSTR_SLICE_TYPE(x)         \
    _Generic((x),                  \
             struct cstr_sslice    \
             : struct cstr_sslice, \
               struct cstr_islice  \
             : struct cstr_islice)
#endif

#define CSTR_SLICE(TYPE, BUF, LEN) ((TYPE){.buf = (BUF), .len = (LEN)})

// get type from var (without non-standard typeof)
#define CSTR_SLICE_VAR(VAR, BUF, LEN)                              \
    .Generic((VAR),                                                \
             struct cstr_sslice                                    \
             : ((struct cstr_sslice){.buf = (BUF), .len = (LEN)}), \
               struct cstr_islice                                  \
             : ((struct cstr_islice){.buf = (BUF), .len = (LEN)}))

#define CSTR_SSLICE(BUF, LEN) CSTR_SLICE(struct cstr_sslice, BUF, LEN)
#define CSTR_ISLICE(BUF, LEN) CSTR_SLICE(struct cstr_islice, BUF, LEN)

#define CSTR_SSLICE_STRING(STR) CSTR_SSLICE(STR, strlen(STR))

// When we have slices we allocate, we want them zero-initialised. This macro can
// do that.
#define CSTR_NIL_SLICE(TYPE) CSTR_SLICE(TYPE, 0, 0)
#define CSTR_NIL_SSLICE CSTR_NIL_SLICE(struct cstr_sslice)
#define CSTR_NIL_ISLICE CSTR_NIL_SLICE(struct cstr_islice)

// Allocate a new buffer for a slice
INLINE bool cstr_alloc_slice_buffer(void **buf, size_t *len,
                                    size_t new_len, size_t elm_size)
{
    assert(*buf == 0); // safety check to avoid mem leak
    *buf = malloc(new_len * elm_size);
    *len = (*buf) ? new_len : 0;
    return !!*buf;
}
#define CSTR_ALLOC_SLICE_BUFFER(VAR, LEN) \
    cstr_alloc_slice_buffer((void **)(&(VAR).buf), &(VAR).len, LEN, sizeof(VAR).buf[0])

#define CSTR_FREE_SLICE_BUFFER(SLICE) \
    do                                \
    {                                 \
        free((SLICE).buf);            \
        (SLICE).buf = 0;              \
        (SLICE).len = 0;              \
    } while (0)

// Comparing string-slices
bool cstr_sslice_eq(struct cstr_sslice x,
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

// Write a mapped string into dst.
bool cstr_alphabet_map(struct cstr_sslice dst,
                       struct cstr_sslice src,
                       struct cstr_alphabet const *alpha,
                       enum cstr_errcodes *err);

// Map a slice into an integer slice.
bool cstr_alphabet_map_to_int(struct cstr_islice dst,
                              struct cstr_sslice src,
                              struct cstr_alphabet const *alpha,
                              enum cstr_errcodes *err);

// FIXME: probably remove this
// Allocate a new buffer to write the mapped array into.
int *cstr_alphabet_map_to_int_new(struct cstr_sslice src,
                                  struct cstr_alphabet const *alpha,
                                  enum cstr_errcodes *err) CSTR_MALLOC_FUNC;

// Map a string back into the dst slice.
bool cstr_alphabet_revmap(struct cstr_sslice dst,
                          struct cstr_sslice src,
                          struct cstr_alphabet const *alpha,
                          enum cstr_errcodes *err);

// FIXME: probably remove this
// Allocate new buffer and map a string back into it.
char *cstr_alphabet_revmap_new(struct cstr_sslice src,
                               struct cstr_alphabet const *alpha,
                               enum cstr_errcodes *err) CSTR_MALLOC_FUNC;

// == EXACT MATCHERS =============================
struct cstr_exact_matcher; // Opaque type

// returns -1 when there are no more matches
int cstr_exact_next_match(struct cstr_exact_matcher *matcher);
void cstr_free_exact_matcher(struct cstr_exact_matcher *matcher);

struct cstr_exact_matcher *
cstr_naive_matcher(struct cstr_sslice x, struct cstr_sslice p);
struct cstr_exact_matcher *
cstr_ba_matcher(struct cstr_sslice x, struct cstr_sslice p);
struct cstr_exact_matcher *
cstr_kmp_matcher(struct cstr_sslice x, struct cstr_sslice p);

// == SUFFIX ARRAYS =====================================================
// Suffix array construction
bool cstr_skew(struct cstr_islice sa, struct cstr_sslice x,
               struct cstr_alphabet *alpha, enum cstr_errcodes *err);

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
