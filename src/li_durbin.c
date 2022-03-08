#include <stddef.h>

#include "bwt_internal.h"
#include "unittests.h"

struct cstr_li_durbin_preproc
{
    cstr_alphabet alpha;
    cstr_suffix_array *sa;
    struct c_table *ctab;
    struct o_table *otab;
    struct o_table *rotab;
};

void cstr_free_li_durbin_preproc(cstr_li_durbin_preproc *preproc)
{
    free(preproc->sa);
    free(preproc->ctab);
    free(preproc->otab);
    free(preproc->rotab);
    free(preproc);
}

cstr_li_durbin_preproc *
cstr_li_durbin_preprocess(cstr_const_sslice x)
{
    cstr_li_durbin_preproc *preproc = cstr_malloc(sizeof *preproc);

    cstr_init_alphabet(&preproc->alpha, x);

    // Map the string into an integer slice so we can build the suffix array.
    cstr_uislice *u_buf = cstr_alloc_uislice(x.len);
    cstr_alphabet_map_to_uint(*u_buf, x, &preproc->alpha);
    cstr_const_uislice u = CSTR_SLICE_CONST_CAST(*u_buf);

    cstr_sslice *w_buf = cstr_alloc_sslice(x.len); // Mapping x into w
    cstr_alphabet_map(*w_buf, x, &preproc->alpha);
    cstr_const_sslice w = CSTR_SLICE_CONST_CAST(*w_buf);

    preproc->sa = cstr_alloc_uislice(x.len); // Get the suffix array

    cstr_sslice *bwt_buf = cstr_alloc_sslice(x.len); // Get a buffer for the the bwt string
    cstr_const_sslice bwt = CSTR_SLICE_CONST_CAST(*bwt_buf);

    // Then build the suffix arrays. Build the reverse first,
    // so we don't need a copy for it later. We need to remember
    // the forward one, but not the backward one.
    // The PREFIX stuff is so we only reverse the prefix up to the sentinel,
    // but we do not include the sentinel in the reversal.
    CSTR_REV_SLICE(CSTR_PREFIX(*u_buf, -1)); // Both of these are representations of the input string.
    CSTR_REV_SLICE(CSTR_PREFIX(*w_buf, -1)); // Just with different types. We reverse to build RO

    cstr_sais(*preproc->sa, u, &preproc->alpha);
    cstr_bwt(*bwt_buf, w, *preproc->sa);
    preproc->ctab = cstr_build_c_table(bwt, preproc->alpha.size);
    preproc->rotab = cstr_build_o_table(bwt, preproc->ctab);

    // The C table is the same in either direction, but we need
    // to rebuild the suffix array and BWT to get the O table
    CSTR_REV_SLICE(CSTR_PREFIX(*u_buf, -1)); // Reverse the input back to the forward direction
    CSTR_REV_SLICE(CSTR_PREFIX(*w_buf, -1)); // so we can build the forward BWT and O table

    cstr_sais(*preproc->sa, u, &preproc->alpha);
    cstr_bwt(*bwt_buf, w, *preproc->sa);
    preproc->otab = cstr_build_o_table(bwt, preproc->ctab);

    // We don't need the mapped string nor the BWT any more.
    // The information we need is all in the tables in preproc.
    free(bwt_buf);
    free(w_buf);
    free(u_buf);

    return preproc;
}

// MARK: Continuation/closure boiler plate for continuation-passing-style recursion

// We use continuations to avoid exhausting the call-stack. With an optimising
// compiler, all the calls are tail-calls, optimised into jump instructions,
// and the memory usage is all on the explicit stack. The stack contains callback functions
// (continuation) and data for them (closures). When a function cannot do anything more, it should
// call the next continuation (call_continuation). It is a little more primitive than
// full continuation-passing-style, because we only need a stack, so we don't need to
// store continuations in the closures, but can always get the next from the stack
// instead. Thus, when we want to call with a continuation, we instead push the continuation
// to the stack and then call. The continuation will be called when the full processing
// of the called function is done.

typedef struct stack stack;
typedef struct stack_frame stack_frame;
typedef struct context context;
typedef union closure closure;
typedef long long (*continuation)(context *context, closure *cl);

static void push_frame(stack **stack, stack_frame frame);
static stack_frame *pop_frame(stack **stack);

// Data used for all search functions
typedef struct context
{
    cstr_li_durbin_preproc *preproc;
    cstr_sslice *p_buf;
    cstr_const_sslice p;
    stack *stack;
} context;

static long long call_next_continuation(context *context);
#define CALL_CONTINUATION() call_next_continuation(context)
#define K(CONT, CL) \
    (stack_frame) { .k = (CONT), .cl = (CL) }
#define CALL_WITH_CONTINUATION(CALL, K) \
    do                                  \
    {                                   \
        push_frame(&context->stack, K); \
        return CALL;                    \
    } while (0);

// clang-format off
struct emit_closure
{
    long long next;
    long long end;
};
#define EMIT_CLOSURE(NEXT, END)      \
    (closure) { .emit_closure = {    \
        .next = (NEXT), .end = (END) \
    } }

struct match_closure
{
    long long left;   // Range where we have matching 
    long long right;  // prefixes.
    long long i;      // Index into p
    long long d;      // Number of edits left
    uint8_t a;        // Current letter we are matching/mismatching
};
#define MATCH_CLOSURE(LEFT, RIGHT, I, D, A)  \
    (closure) { .match_closure = {           \
        .left = (LEFT), .right = (RIGHT),    \
        .i = (I), .d = (D), .a = (A)         \
    } }

struct rec_search_closure
{
    long long left;   // Range where we have matching 
    long long right;  // prefixes.
    long long i;      // Index into p
    long long d;      // Number of edits left
};
#define REC_SEARCH_CLOSURE(LEFT, RIGHT, I, D)  \
    (closure) { .rec_search_closure = {        \
        .left = (LEFT), .right = (RIGHT),      \
        .i = (I), .d = (D)                     \
    } }

union closure
{
    struct emit_closure emit_closure;
    struct match_closure match_closure;
    struct rec_search_closure rec_search_closure;
};
// clang-format on

struct stack_frame
{
    continuation k;
    closure cl;
};

// MARK: Stack used for CPS.
// Quite simple implementation, but it suffices for what we need here.

struct stack
{
    size_t size;
    size_t used;
    stack_frame frames[];
};

// We only shrink a stack when it is 1/4 used, and then only to 1/2 size,
// so memory we pop off is still available until the next stack action.
#define MIN_STACK_SIZE 128
static stack *new_stack(void)
{
    stack *stack = CSTR_MALLOC_FLEX_ARRAY(stack, frames, MIN_STACK_SIZE);
    stack->size = MIN_STACK_SIZE;
    stack->used = 0;
    return stack;
}

static inline stack *resize_stack(stack *stack)
{
    if (stack->size / 4 < stack->used && stack->used < stack->size)
    {
        return stack; // No resize necessary
    }
    if (stack->used == stack->size)
    {
        stack->size *= 2;
    }
    else
    {
        assert(stack->used <= stack->size / 4);
        stack->size /= 2;
        stack->size = (stack->size < MIN_STACK_SIZE) ? MIN_STACK_SIZE : stack->size;
    }
    return cstr_realloc_header_array(stack,
                                     offsetof(struct stack, frames),
                                     sizeof stack->frames[0],
                                     stack->size);
}

static inline void push_frame(stack **stack, stack_frame k)
{
    *stack = resize_stack(*stack);
    (*stack)->frames[(*stack)->used++] = k;
}

static inline stack_frame *pop_frame(stack **stack)
{
    *stack = resize_stack(*stack);
    return &(*stack)->frames[--(*stack)->used];
}

static inline long long call_next_continuation(context *context)
{
    if (context->stack->used == 0)
    {
        return -1;
    }
    else
    {
        stack_frame *sf = pop_frame(&context->stack);
        return sf->k(context, &sf->cl);
    }
}

// MARK: actions in the approximative search
static long long rec_search(
    long long i,
    long long left, long long right,
    long long d,
    context *context);

static long long emit_cont(context *context, closure *cl);
static inline long long emit(long long next, long long end, context *context)
{
    if (next < end)
    {
        CALL_WITH_CONTINUATION(
            context->preproc->sa->buf[next],
            K(emit_cont, EMIT_CLOSURE(next + 1, end)));
    }
    else
    {
        return CALL_CONTINUATION();
    }
}

static long long emit_cont(context *context, closure *cl)
{
    struct emit_closure *ecl = &cl->emit_closure;
    return emit(ecl->next, ecl->end, context);
}

static long long match_cont(context *context, closure *cl);
static inline long long match(
    long long left, long long right,
    long long i, long long d, uint8_t a,
    context *context)
{
    if (a == context->preproc->alpha.size)
    {
        // No more match operations.
        // FIXME: continue with insertions.
        // For now, just finish
        return CALL_CONTINUATION();
    }
    else
    {
        struct c_table *ctab = context->preproc->ctab;
        struct o_table *otab = context->preproc->otab;
        long long new_left = C(a) + O(a, left);
        long long new_right = C(a) + O(a, right);
        long long new_d = d - (context->p.buf[i] != a);
        // Recurse, then continue afterwards
        CALL_WITH_CONTINUATION(
            rec_search(new_left, new_right, i - 1, new_d, context),
            K(match_cont, MATCH_CLOSURE(left, right, i, d, a + 1)));
    }
}

static long long match_cont(context *context, closure *cl)
{
    struct match_closure *mcl = &cl->match_closure;
    return match(mcl->left, mcl->right, mcl->i, mcl->d, mcl->a, context);
}

static long long rec_search(long long left, long long right,
                            long long i, long long d,
                            context *context)
{
    if (left >= right || d < 0)
    {
        // Nothing to be found here, try the next continuation
        return CALL_CONTINUATION();
    }
    if (i < 0)
    {
        // We have a match, so emit it
        return emit(left, right, context);
    }

    // Otherwise, continue the recursion with the next match operation.
    // The match operation starts with a == 1 because we don't want
    // the sentinel.
    return match(left, right, i, d, 1, context);
}

static long long rec_search_cont(context *context, closure *cl)
{
    struct rec_search_closure *rscl = &cl->rec_search_closure;
    return rec_search(rscl->left, rscl->right, rscl->i, rscl->d, context);
}

typedef struct iterator
{
    cstr_sslice *p_buf;
    context context;
} iterator;

static iterator *new_iterator(cstr_li_durbin_preproc *preproc,
                              cstr_const_sslice p, long long d)
{
    iterator *itr = cstr_malloc(sizeof *itr);
    itr->context.preproc = preproc;
    
    itr->p_buf = cstr_alloc_sslice(p.len);
    itr->context.p = CSTR_SLICE_CONST_CAST(*itr->p_buf);
    bool map_ok = cstr_alphabet_map(*itr->p_buf, p, &preproc->alpha);
    itr->context.stack = new_stack();
    
    if (map_ok)
    {
        // If mapping failed, we leave the stack empty. Then the iterator will
        // return no matches, but it will still be in a state where we can free
        // it as per usual.
        push_frame(&itr->context.stack,
                   K(rec_search_cont, REC_SEARCH_CLOSURE(0, preproc->sa->len, p.len - 1, d)));
    }
    
    return itr;
}

static void free_iterator(iterator *itr)
{
    free(itr->p_buf);
    free(itr->context.stack);
    free(itr);
}

static long long iterator_next(iterator *itr)
{
    return call_next_continuation(&itr->context);
}

#ifdef GEN_UNIT_TESTS // unit testing of static functions...

static void print_c_table(struct c_table const *ctab)
{
    printf("[ ");
    for (long long i = 0; i < ctab->sigma; i++)
    {
        printf("%u ", ctab->cumsum[i]);
    }
    printf("]\n");
}

static void print_o_table(struct o_table const *otab)
{
    for (int a = 0; a < otab->sigma; a++)
    {
        for (int i = 0; i < otab->n; i++)
        {
            printf("%lld ", O_RAW(a, i));
        }
        printf("\n");
    }
}

TL_TEST(build_ld_tables)
{
    TL_BEGIN();

    cstr_const_sslice x = CSTR_SLICE_STRING0((const char *)"mississippi");
    cstr_li_durbin_preproc *preproc = cstr_li_durbin_preprocess(x);
    printf("SA: ");
    CSTR_SLICE_PRINT(*preproc->sa);
    printf("\n");
    printf("C: ");
    print_c_table(preproc->ctab);
    printf("\n");
    printf("O:\n");
    print_o_table(preproc->otab);
    printf("\n");
    printf("RO:\n");
    print_o_table(preproc->rotab);

    cstr_free_li_durbin_preproc(preproc);

    TL_END();
}

TL_TEST(ld_iterator)
{
    TL_BEGIN();

    cstr_const_sslice x = CSTR_SLICE_STRING0((const char *)"mississippi");
    cstr_const_sslice p = CSTR_SLICE_STRING((const char *)"is");
    cstr_li_durbin_preproc *preproc = cstr_li_durbin_preprocess(x);
    long long d = 1; // FIXME: FOR NOW
    iterator *itr = new_iterator(preproc, p, d);

    for (long long i = iterator_next(itr); i != -1; i = iterator_next(itr))
    {
        printf("%lld\n", i);
    }

    free_iterator(itr);

    TL_END();
}

#endif // GEN_UNIT_TESTS
