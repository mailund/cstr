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

// We use continuations to avoid exhausting the call-stack. With an optimising
// compiler, all the calls are tail-calls, optimised into jump instructions,
// and the memory usage is all on the explicit stack. The stack contains callback functions
// (actions) and data for them (closures). When a function cannot do anything more, it should
// call the next continuation (call_continuation). It is a little more primitive than
// full continuation-passing-style, because we only need a stack, so we don't need to
// store continuations in the closures, but can always get the next from the stack
// instead. Thus, when we want to call with a continuation, we instead push the continuation
// to the stack and then call. The continuation will be called when the full processing
// of the called function is done.
typedef struct stack stack;
typedef struct stack_frame stack_frame;
typedef union closure closure;
typedef long long (*action)(cstr_li_durbin_preproc *preproc, stack **stack, closure *cl);

static void push_frame(stack **stack, action f, closure cl);
static stack_frame *pop_frame(stack **stack);

static long long call_continuation(cstr_li_durbin_preproc *preproc, stack **stack);

#define EMIT_CLOSURE(NEXT, END) (closure) { .emit_closure = { .next = (NEXT), .end = (END) } }
struct emit_closure
{
    long long next;
    long long end;
};

// clang-format off
union closure
{
    struct emit_closure emit_closure;
};
// clang-format on

struct stack_frame
{
    action f;
    closure closure;
};

static long long emit(cstr_li_durbin_preproc *preproc, stack **stack, closure *cl)
{
    struct emit_closure ecl = *(struct emit_closure *)cl;
    if (ecl.next < ecl.end)
    {
        push_frame(stack, emit, EMIT_CLOSURE(ecl.next + 1, ecl.end));
        return preproc->sa->buf[ecl.next];
    }
    return call_continuation(preproc, stack);
}

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

static void push_frame(stack **stack, action f, closure cl)
{
    *stack = resize_stack(*stack);
    (*stack)->frames[(*stack)->used++] =
        (stack_frame){.f = f, .closure = cl};
}

static stack_frame *pop_frame(stack **stack)
{
    *stack = resize_stack(*stack);
    return &(*stack)->frames[--(*stack)->used];
}

static long long call_continuation(cstr_li_durbin_preproc *preproc, stack **stack)
{
    if ((*stack)->used == 0)
    {
        return -1;
    }
    else
    {
        stack_frame *sf = pop_frame(stack);
        return sf->f(preproc, stack, &sf->closure);
    }
}

typedef struct iterator
{
    cstr_li_durbin_preproc *preproc; // Don't free
    stack *stack;
} iterator;

static iterator *new_iterator(cstr_li_durbin_preproc *preproc)
{
    iterator *itr = cstr_malloc(sizeof *itr);
    itr->preproc = preproc;
    itr->stack = new_stack();
    push_frame(&itr->stack, emit, EMIT_CLOSURE(0, 10));
    return itr;
}

static void free_iterator(iterator *itr)
{
    free(itr->stack);
    free(itr);
}

static long long iterator_next(iterator *itr)
{
    return call_continuation(itr->preproc, &itr->stack);
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
    cstr_li_durbin_preproc *preproc = cstr_li_durbin_preprocess(x);
    iterator *itr = new_iterator(preproc);
    
    for (long long i = iterator_next(itr); i != -1; i = iterator_next(itr))
    {
        printf("%lld\n", i);
    }

    free_iterator(itr);

    TL_END();
}

#endif // GEN_UNIT_TESTS
