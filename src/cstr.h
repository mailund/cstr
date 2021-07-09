#ifndef CSTR_INCLUDED
#define CSTR_INCLUDED

#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <string.h>

// A few things we are simply going to assume is true...
static_assert(CHAR_BIT == 8, "Chars must be bytes (8-bit numbers)");

// Using some non-standard features (hoping that a future standard soon will
// give us this, since most compilers implement it anyway)
void cstr_auto_decref(void *);
#if defined(__clang__) || defined(__GNUC__) || defined(__GNUG__)
#  define CSTR_AUTO_DECREF __attribute__((cleanup(cstr_auto_decref)))
#elif defined(_MSC_VER)
#  error "I don't know the equivalent for MSVC"
#endif

// This is to provide inline functions without putting the
// static code in each output file...
#ifndef INLINE
#define INLINE inline
#endif

// ref-counting memory management
struct cstr_refcount_type {
    void (*cleanup)(void *); // callback to clean up memory
};
struct cstr_refcount_object { // embed in refcount objects
    struct cstr_refcount_type *type;
    int refcount;
};
void cstr_refcount_object_init(struct cstr_refcount_object *obj,
                               struct cstr_refcount_type *type);
void *cstr_refcount_incref(void *obj); // void to avoid casting
void *cstr_refcount_decref(void *obj); // void to avoid casting

// memory managed strings. When cstr allocate strings, this is the
// type it allocates. You can use the buf as a C type string (it is
// always zero-terminated, although it is not counted in the length).
struct cstr {
    struct cstr_refcount_object rc;
    size_t len;
    char buf[];
};
struct cstr *cstr_alloc_cstr(size_t size);
struct cstr *cstr_cstr_from_string(char *x);

// memory managed integer arrays. For when you want ref-counting and when
// you want to know how long an array is.
struct cstr_int_array {
    struct cstr_refcount_object rc;
    size_t len;
    int buf[];
};
struct cstr_int_array *cstr_alloc_int_array(size_t len);

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
INLINE struct cstr_str_slice cstr_slice_from_cstr(struct cstr *cstr) {
    return (struct cstr_str_slice){.buf = cstr->buf, .len = cstr->len};
}
INLINE struct cstr_str_slice cstr_slice_from_string(char const *x) {
    return (struct cstr_str_slice){.buf = x, .len = strlen(x)};
}
INLINE struct cstr_str_slice cstr_slice_from_buffer(char const *buf,
                                                    size_t len) {
    return (struct cstr_str_slice){.buf = buf, .len = len};
}

// Alphabets, for when we remap strings to smaller alphabets
struct cstr_alphabet {
    struct cstr_refcount_object rc;
    unsigned int size;
    char map[256];
    char revmap[256];
};
struct cstr_alphabet *cstr_alphabet_from_slice(struct cstr_str_slice slice);
INLINE struct cstr_alphabet *cstr_alphabet_from_cstr(struct cstr *x) {
    return cstr_alphabet_from_slice(cstr_slice_from_cstr(x));
}
INLINE struct cstr_alphabet *cstr_alphabet_from_string(char const *x) {
    return cstr_alphabet_from_slice(cstr_slice_from_string(x));
}

struct cstr *cstr_alphabet_map(struct cstr_alphabet const *alpha,
                               struct cstr_str_slice s);
struct cstr_int_array *
cstr_alphabet_map_to_int(struct cstr_alphabet const *alpha,
                         struct cstr_str_slice s);
struct cstr *cstr_alphabet_revmap(struct cstr_alphabet const *alpha,
                                  struct cstr_str_slice s);

// Suffix array construction
int *cstr_skew(char const *x);

// Burrows-Wheeler transform -----------------------------------
char *cstr_bwt(int n, char const *x, int sa[n]);

struct cstr_bwt_c_table {
    int asize;
    int cumsum[];
};

struct cstr_bwt_c_table *cstr_compute_bwt_c_table(int n, char const *x,
                                                  int asize);
void cstr_print_bwt_c_table(struct cstr_bwt_c_table const *ctab);
INLINE int cstr_bwt_c_tab_rank(struct cstr_bwt_c_table const *ctab, char i) {
    return ctab->cumsum[i];
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
