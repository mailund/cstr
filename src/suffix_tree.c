#include "cstr.h"
#include "unittests.h"
#include <stdalign.h>
#include <stddef.h>
#include <stdlib.h>
#include <testlib.h>

// Positive nodes are inner nodes, negative nodes are
// leaves, and zero is NULL
typedef long long node_id;

// clang-format off
static inline bool is_leaf(node_id n)  { return n < 0;  }
static inline bool is_null(node_id n)  { return n == 0; }
static inline bool is_inner(node_id n) { return n > 0;  }
// clang-format on

// Mapping from a suffix to the corresponding leaf id.
static inline node_id leaf_id(long long suffix) { return -suffix - 1; }

typedef struct node_shared
{
    node_id parent;
} node_shared;

typedef struct leaf
{
    node_shared node;      // Embedding shared
    long long suffix;      // The suffix this leaf represents
    long long slice_start; // The end point of a leaf is always the end of x
} leaf;

typedef struct inner
{
    node_shared node; // Embedding shared
    cstr_const_sslice edge;
    node_id children[];
} inner;

// FIXME: Right now, my allocation pool for inner nodes always allocate the maximum
// number of nodes I could possibly need, n - 1, but this is wasteful and I should
// adjust it to grow, without growing too fast. Once I start growing the pool, however,
// pointers to inner nodes are never valid after an allocation, only their IDs are!
// It is better if the interface to the tree is always through IDs, then, even if there
// is a slight overhead because of it.
struct inner_node_pool
{
    size_t block_size;
    node_id next;
    alignas(struct inner) char *node_blocks[]; // type struct inner for align; use char * for addresses
};

static struct inner_node_pool *new_pool(long long sigma, long long n)
{
    // A node will take up the space for the edge and sigma children
    size_t node_size = offsetof(struct inner, children) + ((size_t)sigma * sizeof(node_id));
    // And a node block must have a size such that consequtive blocks are correctly aligned
    size_t align_constraint = alignof(struct inner);
    size_t block_size = align_constraint * (node_size + align_constraint - 1) / align_constraint;

    // Now, allocate a pool of n - 1 of those blocks. FIXME: change the pre-allocated size
    struct inner_node_pool *pool =
        cstr_malloc_header_array(offsetof(struct inner_node_pool, node_blocks), block_size, (size_t)(n - 1));

    pool->block_size = block_size;
    pool->next = 0; // start at 0 but inc *before* returning new

    return pool;
}

static inner *pool_get(struct inner_node_pool *pool, node_id n)
{
    size_t offset = (size_t)(n - 1) * pool->block_size;
    void *block = pool->node_blocks + offset;
    return (struct inner *)block;
}

static node_id pool_get_next(struct inner_node_pool *pool)
{
    return ++(pool->next);
}

struct cstr_suffix_tree
{
    cstr_alphabet const *alpha;
    cstr_const_sslice x;
    node_id root;
    struct inner_node_pool *pool;
    struct leaf leaves[];
};

static inline leaf *get_leaf(cstr_suffix_tree *st, node_id n)
{
    assert(is_leaf(n));
    return &st->leaves[-n - 1]; // change sign and adjust for NULL
}

static node_id new_inner(cstr_suffix_tree *st,
                         cstr_const_sslice edge)
{
    node_id node = pool_get_next(st->pool);
    inner *inner = pool_get(st->pool, node);

    inner->edge = edge;
    for (long long i = 0; i < st->alpha->size; i++)
    {
        inner->children[i] = 0; // NULL child
    }

    return node;
}

static inline inner *get_inner(cstr_suffix_tree *st, node_id n)
{
    assert(is_inner(n));
    return pool_get(st->pool, n);
}

// clang-format off
static inline node_shared *get_node(cstr_suffix_tree *st, node_id n)
{
    return (n < 0) ? (node_shared *)get_leaf(st, n) :
           (n > 0) ? (node_shared *)get_inner(st, n) : 
           0;
}
// clang-format on

static cstr_const_sslice get_edge(cstr_suffix_tree *st, node_id n)
{
    assert(!is_null(n));
    if (is_leaf(n))
    {
        return CSTR_SUFFIX(st->x, get_leaf(st, n)->slice_start);
    }
    else
    {
        return get_inner(st, n)->edge;
    }
}

static void set_edge(cstr_suffix_tree *st, node_id n, cstr_const_sslice edge)
{
    assert(!is_null(n));
    if (is_leaf(n))
    {
        // edge must be a suffix of st->x
        assert(st->x.buf <= edge.buf && edge.buf + edge.len == st->x.buf + st->x.len);
        get_leaf(st, n)->slice_start = (edge.buf - st->x.buf);
    }
    else
    {
        get_inner(st, n)->edge = edge;
    }
}

static void set_child(cstr_suffix_tree *st, node_id parent_id, node_id child_id)
{
    assert(is_inner(parent_id));
    inner *parent = get_inner(st, parent_id);
    node_shared *child = get_node(st, child_id);
    cstr_const_sslice child_edge = get_edge(st, child_id);
    parent->children[child_edge.buf[0]] = child_id;
    child->parent = parent_id;
}

static node_id break_edge(cstr_suffix_tree *st, node_id to, long long len)
{
    // Split the edge into two slices at offset len
    cstr_const_sslice edge = get_edge(st, to);
    cstr_const_sslice prefix = CSTR_PREFIX(edge, len);
    cstr_const_sslice suffix = CSTR_SUFFIX(edge, len);

    // Create a new node and get the parent of 'to'.
    // The edge to the new node is the prefix of the current edge.
    node_id new_node = new_inner(st, prefix);
    node_id parent_id = get_node(st, to)->parent;

    // Change the edge to `to` so it is the suffix of the current
    // edge, then connect the parent to the new node and the new node to to
    set_edge(st, to, suffix);
    set_child(st, parent_id, new_node);
    set_child(st, new_node, to);

    return new_node;
}

static void get_children(cstr_suffix_tree *st, node_id n,
                         node_id **beg, node_id **end)
{
    if (n <= 0)
    {
        *beg = *end = 0; // Empty interval
    }
    else
    {
        inner *node = get_inner(st, n);
        *beg = node->children;
        *end = node->children + st->alpha->size;
    }
}

static node_id get_child(cstr_suffix_tree *st, node_id n, uint8_t a)
{
    assert(is_inner(n));
    inner *node = get_inner(st, n);
    return node->children[a];
}

static long long shared_prefix(cstr_suffix_tree *st,
                               node_id node, cstr_const_sslice p)
{
    return CSTR_SLICE_LCP(get_edge(st, node), p);
}

// clang-format off
typedef struct
{
    enum
    {
        NODE_MATCH, NODE_MISMATCH,
        EDGE_MATCH, EDGE_MISMATCH,
    } result;
    union
    {
        struct { node_id n; }                                 node_match;
        struct { node_id n; cstr_const_sslice final_string; } node_mismatch;
        struct { node_id n; long long shared; }               edge_match;
        struct { 
            node_id n; 
            cstr_const_sslice final_string; 
            long long shared;
        } edge_mismatch;
    };
} scan_res;
// clang-format on

static inline scan_res node_match(node_id n)
{
    return (scan_res){.result = NODE_MATCH, .node_match = {.n = n}};
}

static inline scan_res node_mismatch(node_id n, cstr_const_sslice final_string)
{
    return (scan_res){
        .result = NODE_MISMATCH,
        .node_mismatch = {.n = n, .final_string = final_string}};
}

static inline scan_res edge_match(node_id n, long long shared)
{
    return (scan_res){
        .result = EDGE_MATCH,
        .edge_match = {.n = n, .shared = shared}};
}

static inline scan_res edge_mismatch(node_id n,
                                     cstr_const_sslice final_string,
                                     long long shared)
{
    return (scan_res){
        .result = EDGE_MISMATCH,
        .edge_mismatch = {
            .n = n,
            .final_string = final_string,
            .shared = shared}};
}

static scan_res
slow_scan(cstr_suffix_tree *st,
          node_id from, cstr_const_sslice p)
{
    for (;;)
    {
        if (p.len == 0)
        {
            // A complete match (just because p is empty)
            return node_match(from);
        }

        node_id to = get_child(st, from, p.buf[0]);
        if (is_null(to))
        {
            // We cannot continue, because there is no where to continue to
            // A complete match
            return node_mismatch(from, p);
        }

        long long shared = shared_prefix(st, to, p);
        long long edge_len = get_edge(st, to).len;
        if (shared == p.len)
        {
            // We matched all of p, so we have a match. It's on a
            // node if we also made it through the edge, otherwise
            // it is an edge match
            return (edge_len == shared) ? node_match(to) : edge_match(to, shared);
        }

        if (shared < edge_len)
        {
            // We have a mismatch on the edge because we didn't make it
            // through it (checked for above)
            return edge_mismatch(to, p, shared);
        }

        // Continue recursion, chop off what we already matched
        // and continue from the node we reached
        p = CSTR_SUFFIX(p, shared);
        from = to;
    }
}

static cstr_suffix_tree *
new_suffix_tree(cstr_alphabet const *alpha, cstr_const_sslice x)
{
    cstr_suffix_tree *st = CSTR_MALLOC_FLEX_ARRAY(st, leaves, (size_t)x.len);
    st->pool = new_pool(alpha->size, x.len);

    st->alpha = alpha;
    st->x = x;
    st->root = new_inner(st, CSTR_SUBSLICE(x, 0, 0));
    for (long long i = 0; i < x.len; i++)
    {
        // The slice_start is wrong here, but will be updated in
        // the construction algorithms.
        st->leaves[i] = (leaf){.suffix = i, .slice_start = i};
    }

    return st;
}

static void naive_insert(cstr_suffix_tree *st, long long i)
{
    cstr_const_sslice suffix = CSTR_SUFFIX(st->x, i);
    scan_res res = slow_scan(st, st->root, suffix);
    switch (res.result)
    {
    case NODE_MISMATCH:
        set_edge(st, leaf_id(i), res.node_mismatch.final_string);
        set_child(st, res.node_mismatch.n, leaf_id(i));
        break;

    case EDGE_MISMATCH:
    {
        node_id breakpoint = break_edge(st, res.edge_mismatch.n, res.edge_mismatch.shared);
        set_edge(st, leaf_id(i), CSTR_SUFFIX(res.edge_mismatch.final_string, res.edge_mismatch.shared));
        set_child(st, breakpoint, leaf_id(i));
    }
    break;

    default:
        assert(false); // There should never be complete matches here.
    }
}

cstr_suffix_tree *
cstr_naive_suffix_tree(cstr_alphabet const *alpha, cstr_const_sslice x)
{
    cstr_suffix_tree *st = new_suffix_tree(alpha, x);
    for (long long i = 0; i < x.len; i++)
    {
        naive_insert(st, i);
    }
    return st;
}

void cstr_free_suffix_tree(cstr_suffix_tree *st)
{
    free(st->pool);
    free(st);
}

#ifdef GEN_UNIT_TESTS // unit testing of static functions...

TL_TEST(st_constructing_leaves)
{
    TL_BEGIN();

    cstr_const_sslice u = CSTR_SLICE_STRING0((const char *)"mississippi");
    cstr_alphabet alpha;
    cstr_init_alphabet(&alpha, u);

    cstr_sslice *x_buf = cstr_alloc_sslice(u.len);
    bool ok = cstr_alphabet_map(*x_buf, u, &alpha);
    TL_FATAL_IF(!ok);
    cstr_const_sslice x = CSTR_SLICE_CONST_CAST(*x_buf);

    cstr_suffix_tree *st = new_suffix_tree(&alpha, x);

    for (long long i = 0; i < x.len; i++)
    {
        node_id n = -(i + 1); // Turning the index into a leaf id
        TL_FATAL_IF_NEQ_LL(leaf_id(i), n);
        leaf *l = get_leaf(st, n);
        TL_FATAL_IF_NEQ_LL(l->suffix, i);
        TL_FATAL_IF_NEQ_LL(l->slice_start, i);
        TL_FATAL_IF_NEQ_SLICE(CSTR_SUFFIX(x, i), get_edge(st, n));
    }

    cstr_free_suffix_tree(st);
    free(x_buf);

    TL_END();
}

TL_TEST(st_constructing_inner_nodes)
{
    TL_BEGIN();

    cstr_const_sslice u = CSTR_SLICE_STRING0((const char *)"mississippi");
    cstr_alphabet alpha;
    cstr_init_alphabet(&alpha, u);

    cstr_sslice *x_buf = cstr_alloc_sslice(u.len);
    bool ok = cstr_alphabet_map(*x_buf, u, &alpha);
    TL_FATAL_IF(!ok);
    cstr_const_sslice x = CSTR_SLICE_CONST_CAST(*x_buf);

    cstr_suffix_tree *st = new_suffix_tree(&alpha, x);

    TL_FATAL_IF_NEQ_LL(st->root, 1LL); // We expect the root to be the first inner node and have id 1

    // Let's try adding the first leaf as a child to the root
    set_child(st, st->root, leaf_id(0));

    // The root should have sigma children
    node_id *beg, *end;
    get_children(st, st->root, &beg, &end);
    assert(end - beg == st->alpha->size);

    int no_children = 0;
    for (node_id *n = beg; n != end; n++)
    {
        if (!is_null(*n))
        {
            no_children++;
        }
    }
    // We should have one child of the root now
    TL_FATAL_IF_NEQ_INT(no_children, 1);

    // The child we inserted should be the one we got if we asked for that out edge
    node_id child = get_child(st, st->root, st->x.buf[0]);
    TL_FATAL_IF_NEQ_LL(child, leaf_id(0));

    // A leaf should have zero children
    get_children(st, leaf_id(0), &beg, &end);
    assert(end == beg);

    // Try adding "ississippi"
    set_child(st, st->root, leaf_id(1));
    // and "ssissippi"
    set_child(st, st->root, leaf_id(2));
    // but not more for now, we don't want to hit used slots...

    get_children(st, st->root, &beg, &end);
    no_children = 0;
    for (node_id *n = beg; n != end; n++)
    {
        if (!is_null(*n))
        {
            no_children++;
        }
    }
    // We should have three children of the root now
    TL_FATAL_IF_NEQ_INT(no_children, 3);

    cstr_free_suffix_tree(st);
    free(x_buf);

    TL_END();
}

TL_TEST(st_attempted_scans)
{
    TL_BEGIN();

    cstr_const_sslice u = CSTR_SLICE_STRING0((const char *)"mississippi");
    cstr_alphabet alpha;
    cstr_init_alphabet(&alpha, u);

    cstr_sslice *x_buf = cstr_alloc_sslice(u.len);
    bool ok = cstr_alphabet_map(*x_buf, u, &alpha);
    TL_FATAL_IF(!ok);
    cstr_const_sslice x = CSTR_SLICE_CONST_CAST(*x_buf);

    cstr_suffix_tree *st = new_suffix_tree(&alpha, x);

    TL_FATAL_IF_NEQ_LL(st->root, 1LL); // We expect the root to be the first inner node and have id 1

    // Adding "mississippi"
    set_child(st, st->root, leaf_id(0));
    // "ississippi"
    set_child(st, st->root, leaf_id(1));
    // and "ssissippi"
    set_child(st, st->root, leaf_id(2));

    cstr_const_sslice p = CSTR_SUFFIX(x, 0);
    scan_res res = slow_scan(st, st->root, p);
    assert(res.result == NODE_MATCH);
    assert(res.node_match.n == leaf_id(0));

    p = CSTR_SUFFIX(x, 1);
    res = slow_scan(st, st->root, p);
    assert(res.result == NODE_MATCH);
    assert(res.node_match.n == leaf_id(1));

    p = CSTR_SUFFIX(x, 2);
    res = slow_scan(st, st->root, p);
    assert(res.result == NODE_MATCH);
    assert(res.node_match.n == leaf_id(2));

    /*
     x = [2, 1, 4, 4, 1, 4, 4, 2, 2, 0]
     */
    cstr_const_sslice w = CSTR_SLICE_STRING((const char *)"\2\1\4\1"); // should mismatch on 3
    res = slow_scan(st, st->root, w);
    assert(res.result == EDGE_MISMATCH);
    assert(res.edge_mismatch.n == leaf_id(0));
    assert(res.edge_mismatch.shared == 3);

    w = CSTR_SLICE_STRING((const char *)"\2\1\4"); // should match on 3
    res = slow_scan(st, st->root, w);
    assert(res.result == EDGE_MATCH);
    assert(res.edge_match.n == leaf_id(0));
    assert(res.edge_match.shared == 3);

    // Try breaking the edge at this location. It won't be a suffix tree now,
    // it has a node with only one child, but its something we need later
    node_id new_node = break_edge(st, leaf_id(0), 3LL);
    cstr_const_sslice new_edge = get_edge(st, new_node);
    assert(new_edge.len = 3LL);
    // leaf_id(0) should now have the new node as its parent
    // and a shorter edge.
    assert(get_node(st, leaf_id(0))->parent == new_node);
    TL_FATAL_IF_NEQ_LL(get_edge(st, leaf_id(0)).len, x.len - 3LL);

    cstr_free_suffix_tree(st);
    free(x_buf);

    TL_END();
}

#endif // GEN_UNIT_TESTS
