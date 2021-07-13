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
    CSTR_SIZE_ERROR, // if the size/length of something is too large
    CSTR_MAPPING_ERROR, // mapping via an alphabet failed
};

// slices, for easier handling of sub-strings and sub-arrays.
// These should be passed by value and never dynmaically allocated.
// They don't implement any kind of memory management, and the underlying
// buffer must be handled separately.
struct cstr_sslice {
    char* const buf;
    size_t len;
};
struct cstr_const_sslice {
    char const* const buf;
    size_t len;
};
struct cstr_islice {
    unsigned int* const buf;
    size_t len;
};

#define CSTR_SLICE(TYPE, BUF, LEN) \
    ((TYPE) { .buf = (BUF), .len = (LEN) })

#define CSTR_SSLICE(BUF, LEN) \
    CSTR_SLICE(struct cstr_sslice, BUF, LEN)
#define CSTR_SSLICE_STRING(STR) \
    CSTR_SSLICE(STR, strlen(STR))

#define CSTR_CSSLICE(BUF, LEN) \
    CSTR_SLICE(struct cstr_const_sslice, BUF, LEN)
#define CSTR_CSSLICE_STRING(STR) \
    CSTR_CSSLICE(STR, strlen(STR))

#define CSTR_ISLICE(BUF, LEN) \
    CSTR_SLICE(struct cstr_islice, BUF, LEN)

// Alphabets, for when we remap strings to smaller alphabets
struct cstr_alphabet {
    unsigned int size;
    unsigned char map[CSTR_NO_CHARS];
    unsigned char revmap[CSTR_NO_CHARS];
};

// Initialise an alphabet form a slice. Since the alphabet is already
// allocated, this function cannot fail.
void cstr_init_alphabet(
    struct cstr_alphabet* alpha,
    struct cstr_const_sslice slice);

// Write a mapped string into dst.
bool cstr_alphabet_map(
    struct cstr_sslice dst,
    struct cstr_const_sslice src,
    struct cstr_alphabet const* alpha,
    enum cstr_errcodes* err);

// Allocate a new buffer and write the mapped string into it.
char* cstr_alphabet_map_new(
    struct cstr_const_sslice s,
    struct cstr_alphabet const* alpha,
    enum cstr_errcodes* err)
    CSTR_MALLOC_FUNC;

// Map a slice into an integer slice.
bool cstr_alphabet_map_to_int(
    struct cstr_islice dst,
    struct cstr_const_sslice src,
    struct cstr_alphabet const* alpha,
    enum cstr_errcodes* err);

// Allocate a new buffer to write the mapped array into.
unsigned int* cstr_alphabet_map_to_int_new(
    struct cstr_const_sslice src,
    struct cstr_alphabet const* alpha,
    enum cstr_errcodes* err)
    CSTR_MALLOC_FUNC;

bool cstr_alphabet_revmap(
    struct cstr_sslice dst,
    struct cstr_const_sslice src,
    struct cstr_alphabet const* alpha,
    enum cstr_errcodes* err);

char* cstr_alphabet_revmap_new(
    struct cstr_const_sslice src,
    struct cstr_alphabet const* alpha,
    enum cstr_errcodes* err)
    CSTR_MALLOC_FUNC;

// Suffix array construction
unsigned int* cstr_skew(
    struct cstr_alphabet* alpha,
    struct cstr_const_sslice slice,
    enum cstr_errcodes* err);
unsigned int* cstr_skew_from_string(
    char const* x,
    enum cstr_errcodes* err);

// Burrows-Wheeler transform -----------------------------------
char* cstr_bwt(int n, char const* x, unsigned int sa[n]);

struct cstr_bwt_c_table {
    int asize;
    int cumsum[];
};

struct cstr_bwt_c_table* cstr_compute_bwt_c_table(
    int n,
    char const* x,
    int asize);
void cstr_print_bwt_c_table(struct cstr_bwt_c_table const* ctab);
INLINE int cstr_bwt_c_tab_rank(struct cstr_bwt_c_table const* ctab, char i)
{
    return ctab->cumsum[(int)i];
}

struct cstr_bwt_o_table;
struct cstr_bwt_o_table*
cstr_compute_bwt_o_table(int n, char const* bwt,
    struct cstr_bwt_c_table const* ctab);
void cstr_print_bwt_o_table(struct cstr_bwt_o_table const* otab);
int cstr_bwt_o_tab_rank(struct cstr_bwt_o_table const* otab, char a, int i);

void cstr_bwt_search(
    int* left, int* right,
    char const* x, char const* p,
    struct cstr_bwt_c_table const* ctab,
    struct cstr_bwt_o_table const* otab);

#undef INLINE
#endif // CSTR_INCLUDED
