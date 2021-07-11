#ifndef CSTR_INCLUDED
#define CSTR_INCLUDED

#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <string.h>

// A few things we are simply going to assume is true...
static_assert(CHAR_BIT == 8, "Chars must be bytes (8-bit numbers)");

// This is to provide inline functions without putting the
// static code in each output file...
#ifndef INLINE
#define INLINE inline
#endif

// Error handling, primitive as it is...
enum cstr_errcodes {
    CSTR_NO_ERROR,
    CSTR_ALLOCATION_ERROR, // malloc or similar failed
    CSTR_MAPPING_ERROR,    // mapping via an alphabet failed
};

// slices, for easier handling of sub-strings. These should be
// passed by value and never dynmaically allocated. They don't
// implement any kind of memory management, and the underlying
// buffer must be handled separately.
struct cstr_str_slice {
    // The buffer points to char const, so it is safe to use with
    // C's literal strings that must be treated as such, but if
    // it is dynamically allocated, you can safely cast to modify
    // the buffer. You just have to do this explicitly, so you are
    // aware of the issue when you do this.
    char const *const buf;
    size_t len;
};
INLINE struct cstr_str_slice cstr_slice_from_string(char const *x) {
    return (struct cstr_str_slice){.buf = x, .len = strlen(x)};
}
INLINE struct cstr_str_slice cstr_slice_from_buffer(char const *buf,
                                                    size_t len) {
    return (struct cstr_str_slice){.buf = buf, .len = len};
}

// Alphabets, for when we remap strings to smaller alphabets
struct cstr_alphabet {
    unsigned int size;
    unsigned char map[256];
    unsigned char revmap[256];
};
struct cstr_alphabet *cstr_alphabet_from_slice(struct cstr_str_slice slice,
                                               enum cstr_errcodes *err);
INLINE struct cstr_alphabet *
cstr_alphabet_from_string(char const *x, enum cstr_errcodes *err) {
    return cstr_alphabet_from_slice(cstr_slice_from_string(x), err);
}

char *cstr_alphabet_map(struct cstr_alphabet const *alpha,
                        struct cstr_str_slice s, enum cstr_errcodes *err);
unsigned int *cstr_alphabet_map_to_int(struct cstr_alphabet const *alpha,
                              struct cstr_str_slice s, enum cstr_errcodes *err);
char *cstr_alphabet_revmap(struct cstr_alphabet const *alpha,
                           struct cstr_str_slice s, enum cstr_errcodes *err);

// Suffix array construction
unsigned int *cstr_skew(struct cstr_alphabet *alpha, struct cstr_str_slice slice, enum cstr_errcodes *err);
unsigned int *cstr_skew_from_string(char const *x, enum cstr_errcodes *err);

// Burrows-Wheeler transform -----------------------------------
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

struct range {
    int L, R;
};
struct range cstr_bwt_search(char const *x, char const *p,
                             struct cstr_bwt_c_table const *ctab,
                             struct cstr_bwt_o_table const *otab);

#undef INLINE
#endif // CSTR_INCLUDED
