#ifndef CSTR_INCLUDED
#define CSTR_INCLUDED

#include <assert.h>
#include <limits.h>
#include <stdbool.h>
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
enum cstr_errcodes {
    CSTR_NO_ERROR,
    CSTR_ALLOCATION_ERROR, // malloc or similar failed
    CSTR_SIZE_ERROR,       // if the size/length of something is too large
    CSTR_MAPPING_ERROR,    // mapping via an alphabet failed
};

// ==== SLICES =====================================================

// slices, for easier handling of sub-strings and sub-arrays.
// These should be passed by value and never dynmaically allocated.
// They don't implement any kind of memory management, and the underlying
// buffer must be handled separately.
struct cstr_sslice {
    char *const buf;
    size_t const len;
};
struct cstr_const_sslice {
    char const *const buf;
    size_t const len;
};
struct cstr_islice {
    unsigned int *const buf;
    size_t const len;
};

#define CSTR_SLICE(TYPE, BUF, LEN) ((TYPE){.buf = (BUF), .len = (LEN)})

#define CSTR_SSLICE(BUF, LEN) CSTR_SLICE(struct cstr_sslice, BUF, LEN)
#define CSTR_SSLICE_STRING(STR) CSTR_SSLICE(STR, strlen(STR))

#define CSTR_CSSLICE(BUF, LEN) CSTR_SLICE(struct cstr_const_sslice, BUF, LEN)
#define CSTR_CSSLICE_STRING(STR) CSTR_CSSLICE(STR, strlen(STR))

#define CSTR_ISLICE(BUF, LEN) CSTR_SLICE(struct cstr_islice, BUF, LEN)

// WARNING: using the allocation and deallocation of slices is somewhat
// dangerious. We usually just want to pass slices as function arguments, but
// there are situations where we want to allocate memory for the underlying
// buffer as part of initialising a slice. The following code can do that, but
// the underlying buffer must be freed again. You can do that with the
// corresponding free methods. None of the functions allocate or deallocate the
// actual slice; they only touch the underlying buffer. To prevent leaks, the
// buf pointer must be null when you allocate a new buffer. It isn't perfect,
// but it is a slight consistency check.
bool cstr_alloc_sslice_buffer(struct cstr_sslice *slice, size_t len);
void cstr_free_sslice_buffer(struct cstr_sslice *slice);

bool cstr_alloc_islice_buffer(struct cstr_islice *slice, size_t len);
void cstr_free_islice_buffer(struct cstr_islice *slice);

// When we have slices we allocate, we want the zero-initialised. This macro can
// do that.
#define CSTR_NIL_SLICE                                                         \
    { .buf = 0, .len = 0 }

// == ALPHABET =====================================================

// Alphabets, for when we remap strings to smaller alphabets
struct cstr_alphabet {
    unsigned int size;
    unsigned char map[CSTR_NO_CHARS];
    unsigned char revmap[CSTR_NO_CHARS];
};

// Initialise an alphabet form a slice. Since the alphabet is already
// allocated, this function cannot fail.
void cstr_init_alphabet(struct cstr_alphabet *alpha,
                        struct cstr_const_sslice slice);

// Write a mapped string into dst.
bool cstr_alphabet_map(struct cstr_sslice dst, struct cstr_const_sslice src,
                       struct cstr_alphabet const *alpha,
                       enum cstr_errcodes *err);

// Allocate a new buffer and write the mapped string into it.
char *cstr_alphabet_map_new(struct cstr_const_sslice s,
                            struct cstr_alphabet const *alpha,
                            enum cstr_errcodes *err) CSTR_MALLOC_FUNC;

// Map a slice into an integer slice.
bool cstr_alphabet_map_to_int(struct cstr_islice dst,
                              struct cstr_const_sslice src,
                              struct cstr_alphabet const *alpha,
                              enum cstr_errcodes *err);

// Allocate a new buffer to write the mapped array into.
unsigned int *
cstr_alphabet_map_to_int_new(struct cstr_const_sslice src,
                             struct cstr_alphabet const *alpha,
                             enum cstr_errcodes *err) CSTR_MALLOC_FUNC;

// Map a string back into the dst slice.
bool cstr_alphabet_revmap(struct cstr_sslice dst, struct cstr_const_sslice src,
                          struct cstr_alphabet const *alpha,
                          enum cstr_errcodes *err);

// Allocate new buffer and map a string back into it.
char *cstr_alphabet_revmap_new(struct cstr_const_sslice src,
                               struct cstr_alphabet const *alpha,
                               enum cstr_errcodes *err) CSTR_MALLOC_FUNC;

// == EXACT MATCHERS =============================
struct cstr_exact_matcher; // Opaque type

// returns -1 when there are no more matches
int cstr_exact_next_match(struct cstr_exact_matcher *matcher);

void cstr_free_exact_matcher(struct cstr_exact_matcher *matcher);
struct cstr_exact_matcher *cstr_naive_matcher(const char *x, const char *p);
struct cstr_exact_matcher *cstr_ba_matcher(const char *x, const char *p);
struct cstr_exact_matcher *cstr_kmp_matcher(const char *x, const char *p);

// == SUFFIX ARRAYS =====================================================
// Suffix array construction
bool cstr_skew(struct cstr_islice sa, struct cstr_const_sslice x,
               struct cstr_alphabet *alpha, enum cstr_errcodes *err);

// ==== Burrows-Wheeler transform =================================

char *cstr_bwt(int n, char const *x, unsigned int sa[n]);

struct cstr_bwt_c_table {
    int asize;
    int cumsum[];
};

struct cstr_bwt_c_table *cstr_compute_bwt_c_table(int n, char const *x,
                                                  int asize);
void cstr_print_bwt_c_table(struct cstr_bwt_c_table const *ctab);
INLINE int cstr_bwt_c_tab_rank(struct cstr_bwt_c_table const *ctab, char i) {
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
