#include "cstr.h"
#include "unittests.h"
#include <stdalign.h>
#include <stddef.h>
#include <stdlib.h>
#include <testlib.h>

// We could put slices on the edges, but if we use ranges instead, we can encode
// a leaf tag in the encoding of the range. Proper ranges will always have beg < end,
// since we don't have empty edge labels outside of the root, and leaves will always
// have edges that end at the end of the underlying string, so we can put the leaf label
// in 'end' and recognise that we have a leaf if end >= beg, and in that case, the real
// `end` should be st->x.len
// clang-format off
typedef struct
{
    // Always the beginning of the range
    long long beg;
    // But the end for internal nodes or the suffix index for leaves
    union { long long end, leaf; };
} range;

static inline bool is_leaf_range(range r) { return r.end <= r.beg; }
// clang-format on

// Putting SHARED at the top of both 'node' and 'inner_node' makes the
// memory layout the same, so we can use them interchangable, except that
// only inner_node will have the flexible array of children.
// The parent/next union is used such that we have a parent when constructing
// the tree (needed for McCreight) and a threaded tree when we want to
// traverse the tree.
#define SHARED                     \
    range range;                   \
    union                          \
    {                              \
        struct inner_node *parent; \
        struct node *next;         \
    };

// clang-format off
typedef struct node       { SHARED } node;
typedef struct inner_node { SHARED 
    struct inner_node *slink; // suffix link (needed by McCreight) FIXME: union with the other pointers?
    struct node *children[]; // children will be in a block of memory following the struct.
} inner_node;

static inline bool is_leaf(node const *n)  { return is_leaf_range(n->range); }
static inline bool is_inner(node const *n) { return !is_leaf(n); }
// clang-format on

// We use a pool (of sub-pools) to allocate inner nodes. We use sub-pools so we can grow
// the pool without reallocating (and then moving) inner nodes. That way, we can have stable
// pointers to our nodes. The sub-pools are a linked lists of blocks of nodes, where each
// block is a contiguous chunk of memory storing the nodes.

static const size_t sub_pool_size = 256; // Number of nodes in a sub-pool
struct sub_pool
{
    // Link to next sub pool in the list.
    struct sub_pool *next;

    // The block of raw memory where we store the nodes.
    // Alignned as an inner_node but of type char * so we can manipulate
    // it as raw memory. We know how many there are (sub_pool_size), but
    // their size depends on the alphabet, so we cannot allocate them
    // statically.
    alignas(struct inner_node) char node_blocks[];
};

struct inner_node_pool
{
    struct sub_pool *sub_pool; // Linked list of sub-pools
    size_t block_size;         // The run-time size of an inner node
    char *next;                // Pointer to the next free node
    char *end;                 // End of allocated memory
};

static void new_sub_pool(struct inner_node_pool *pool)
{
    struct sub_pool *sub_pool =
        cstr_malloc_header_array(offsetof(struct sub_pool, node_blocks),
                                 pool->block_size, sub_pool_size);
    sub_pool->next = pool->sub_pool;
    pool->sub_pool = sub_pool;
    pool->next = &sub_pool->node_blocks[0];
    pool->end = pool->next + pool->block_size * sub_pool_size;
}

static void init_pool(struct inner_node_pool *pool, long long sigma, long long n)
{
    (void)n; // unused parameter

    // First, figure out the size of memory blocks we need to store inner nodes.
    // A node will take up the space for the shared struct and sigma children
    size_t node_size = offsetof(struct inner_node, children) + ((size_t)sigma * sizeof(node *));
    // And a node block must have a size such that consequtive blocks are correctly aligned,
    // so the block size is the node size rounded up to a full number of alignment blocks.
    size_t align_constraint = alignof(struct inner_node);

    pool->block_size = align_constraint * ((node_size + align_constraint - 1) / align_constraint);
    pool->sub_pool = 0;
    new_sub_pool(pool);
}

// A suffix tree contains the alphabet and string x, the root
// of the tree, then a pointer to a pool of inner nodes and embedded
// in the same memory block, as a flexible array, the leaves. We
// know the size of a leaf and the number of them at compile time,
// so it is easier to put those at the flexible array than it is to
// put the inner nodes there.
struct cstr_suffix_tree
{
    cstr_alphabet const *alpha;
    cstr_const_sslice x;

    inner_node *root;

    struct inner_node_pool pool; // inner nodes are allocated from a pool
    node leaves[];               // leaves are allocated together with the tree
};

// Translating between ranges and slices
static inline cstr_const_sslice range_to_slice(cstr_suffix_tree *st, range r)
{
    return is_leaf_range(r)
               // A leaf goes to the end
               ? CSTR_SUFFIX(st->x, r.beg)
               // An inner node is just the range
               : CSTR_SUBSLICE(st->x, r.beg, r.end);
}
static inline range slice_to_range(cstr_suffix_tree *st, cstr_const_sslice s)
{
    // We cannot recover the leaf information from a slice (even though we can)
    // see if it ends at the end of st->x. That is because the leaf could be an
    // index much earlier in the search. Therefore, we always return the range
    // we would get if we had an internal node.
    long long offset = s.buf - st->x.buf;
    return (range){.beg = offset, .end = offset + s.len};
}

static inline cstr_const_sslice get_edge(cstr_suffix_tree *st, node *n)
{
    assert(n != NULL);
    return range_to_slice(st, n->range);
}

static void set_edge(cstr_suffix_tree *st, node *n, cstr_const_sslice edge)
{
    range r = slice_to_range(st, edge);
    if (is_leaf(n))
    {
        // edge must be a suffix of st->x
        assert(st->x.buf <= edge.buf && edge.buf + edge.len == st->x.buf + st->x.len);
        // Changing the edge to a leaf is only changing the beginning of the range
        n->range.beg = r.beg;
    }
    else
    {
        // For inner nodes, we update the entire range.
        n->range = r;
    }
}

static inner_node *pool_get_next(struct inner_node_pool *pool)
{
    if (pool->next == pool->end)
    {
        new_sub_pool(pool);
    }
    inner_node *next = (inner_node *)(void *)pool->next;
    pool->next += pool->block_size;
    return next;
}

static inline node *get_suffix_leaf(cstr_suffix_tree *st, long long suffix)
{
    return &st->leaves[suffix]; // The leaves are allocated in an array, so its just the offset
}

static inner_node *new_inner(cstr_suffix_tree *st,
                             cstr_const_sslice edge)
{
    inner_node *n = pool_get_next(&st->pool);
    n->range = slice_to_range(st, edge);
    n->slink = n->parent = 0;
    for (long long i = 0; i < st->alpha->size; i++)
    {
        n->children[i] = 0;
    }

    return n;
}

static void set_child(cstr_suffix_tree *st, inner_node *parent, node *child)
{
    cstr_const_sslice child_edge = get_edge(st, child);
    parent->children[child_edge.buf[0]] = child;
    child->parent = parent;
}

static inner_node *break_edge(cstr_suffix_tree *st, node *to, long long len)
{
    // Split the edge into two slices at offset len
    cstr_const_sslice edge = get_edge(st, to);
    cstr_const_sslice prefix = CSTR_PREFIX(edge, len);
    cstr_const_sslice suffix = CSTR_SUFFIX(edge, len);

    // Create a new node and get the parent of 'to'.
    // The edge to the new node is the prefix of the current edge.
    inner_node *new_node = new_inner(st, prefix);

    // Change the edge to `to` so it is the suffix of the current
    // edge, then connect the parent to the new node and the new node to to
    set_edge(st, to, suffix);
    set_child(st, to->parent, (node *)new_node);
    set_child(st, new_node, to);

    return new_node;
}

static inline node **get_children_begin(cstr_suffix_tree *st, node *n)
{
    (void)st; // Unused parameter

    return (n == 0 || is_leaf(n))
               // Leaves and NULL do not have any children
               ? 0
               // Inner nodes have the first here
               : &((inner_node *)n)->children[0];
}
static inline node **get_children_end(cstr_suffix_tree *st, node *n)
{
    return (n == 0 || is_leaf(n))
               // Leaves and NULL do not have any children
               ? 0
               // Inner nodes have (one past the last) here
               : &((inner_node *)n)->children[st->alpha->size];
}

static void get_children(cstr_suffix_tree *st, node *n,
                         node ***beg, node ***end)
{
    *beg = get_children_begin(st, n);
    *end = get_children_end(st, n);
}

static inline node *get_child(inner_node *n, uint8_t a)
{
    return n->children[a];
}

static node *first_child(cstr_suffix_tree *st, node *n)
{
    assert(is_inner(n));
    node **beg, **end, **child;
    get_children(st, n, &beg, &end);
    for (child = beg; child != end; child++)
    {
        if (*child)
        {
            return *child;
        }
    }
    assert(false); // Inner nodes must have a first child
}

// Get the address where a node sits in its parent
static node **node_addr(cstr_suffix_tree *st, node *n)
{
    // The root is a special case, since it sits in the tree and not
    // in another inner node.
    if (n == (node *)st->root)
    {
        return (node **)&st->root;
    }
    uint8_t a = get_edge(st, n).buf[0]; // Out edge to node
    return &n->parent->children[a];
}

static node *next_node(cstr_suffix_tree *st, node *n)
{
    if (is_inner(n))
    {
        // inner node, so move to first child for the recursion...
        return first_child(st, n);
    }

    // Leaf -- we must search for the next node
    for (;;)
    {
        node *p = (node *)n->parent;
        node **sib = node_addr(st, n) + 1; // Start searching one past current
        node **end = get_children_end(st, p);

        // We don't need n's parent any more, so we can change it to its
        // next pointer. We know what next will be, if v is an inner node.
        // If it is a leaf, we have to find the next node first, so we handle
        // that in the node iterator.
        if (is_inner(n))
        {
            n->next = first_child(st, n);
        }

        // Search for the next sibling of n
        for (; sib < end; sib++)
        {
            if (*sib)
            {
                return *sib;
            }
        }

        // If there are no siblings and we are at the root,
        // we can find no more nodes at all.
        if (p == (node *)st->root)
        {
            return 0;
        }

        // Otherwise, we move to our parent's sibling.
        // This is a tail-recursive call.
        n = p;
    }
}

// Connects the `next` pointers in the nodes, destroying `parent` in the
// process (of course, since they share the same memory).
static void thread_nodes(cstr_suffix_tree *st)
{
    node *prev = (node *)st->root;
    do
    {
        node *n = next_node(st, prev);
        if (is_leaf(prev))
        {
            // Connect the leaves here, when we know what the next node
            // will be. The inner nodes (where next has to be the first child)
            // are connected in `next_node`.
            prev->next = n;
        }
        prev = n;
    } while (prev);
    st->root->next = first_child(st, (node *)st->root);
}

static inline long long lcp(cstr_suffix_tree *st, node *n, cstr_const_sslice p)
{
    return CSTR_SLICE_LCP(get_edge(st, n), p);
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
        struct { node *n; }                                 node_match;
        struct { node *n; cstr_const_sslice final_string; } node_mismatch;
        struct { node *n; long long shared; }               edge_match;
        struct { 
            node *n; 
            cstr_const_sslice final_string; 
            long long shared;
        } edge_mismatch;
    };
} scan_res;
// clang-format on

static inline scan_res node_match(node *n)
{
    return (scan_res){.result = NODE_MATCH, .node_match = {.n = n}};
}

static inline scan_res node_mismatch(node *n, cstr_const_sslice final_string)
{
    return (scan_res){
        .result = NODE_MISMATCH,
        .node_mismatch = {.n = n, .final_string = final_string}};
}

static inline scan_res edge_match(node *n, long long shared)
{
    return (scan_res){
        .result = EDGE_MATCH,
        .edge_match = {.n = n, .shared = shared}};
}

static inline scan_res edge_mismatch(node *n,
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
slow_scan(cstr_suffix_tree *st, inner_node *from, cstr_const_sslice p)
{
    for (;;)
    {
        if (p.len == 0)
        {
            // A complete match (just because p is empty)
            return node_match((node *)from);
        }

        node *to = get_child(from, p.buf[0]);
        if (to == NULL)
        {
            // We cannot continue, because there is no where to continue to
            // A complete match
            return node_mismatch((node *)from, p);
        }

        long long shared = lcp(st, to, p);
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
        from = (inner_node *)to;
    }
}

static scan_res
fast_scan(cstr_suffix_tree *st, inner_node *from, cstr_const_sslice p)
{
    for (;;)
    {
        if (p.len == 0)
        {
            // A complete match (just because p is empty)
            return node_match((node *)from);
        }

        node *to = get_child(from, p.buf[0]);
        assert(to); // If we search on a node in fast_scan, the child is there

        long long edge_len = get_edge(st, to).len;
        if (p.len == edge_len)
        {
            return node_match(to);
        }
        if (p.len < edge_len)
        {
            return edge_match(to, p.len);
        }

        // Continue recursion, chop off what we already matched
        // and continue from the node we reached
        p = CSTR_SUFFIX(p, edge_len);
        from = (inner_node *)to;
    }
}

static cstr_suffix_tree *
new_suffix_tree(cstr_alphabet const *alpha, cstr_const_sslice x)
{
    cstr_suffix_tree *st = CSTR_MALLOC_FLEX_ARRAY(st, leaves, (size_t)x.len);

    st->alpha = alpha;
    st->x = x;

    init_pool(&st->pool, alpha->size, x.len);

    // It doesn't matter what edge we put on the root, we are never going to
    // look at it, but we want beg < end on the range so we don't confuse
    // it with a leaf. Here, we just use the entire string.
    st->root = new_inner(st, x);

    // Initialising the leaves (initially, they all just hook up
    // to the root, but this is somewhat arbitrary).
    for (long long i = 0; i < x.len; i++)
    {
        // The slice_start is wrong here, but will be updated in
        // the construction algorithms.
        st->leaves[i] = (node){
            .range = {.beg = i, .leaf = i}, .parent = st->root};
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
    {
        node *leaf = get_suffix_leaf(st, i);
        // res must be inner node; we don't mismatch on leaves
        inner_node *v = (inner_node *)res.node_mismatch.n;
        set_edge(st, leaf, res.node_mismatch.final_string);
        set_child(st, v, leaf);
    }
    break;

    case EDGE_MISMATCH:
    {
        inner_node *breakpoint = break_edge(st, res.edge_mismatch.n, res.edge_mismatch.shared);
        node *leaf = get_suffix_leaf(st, i);
        set_edge(st, leaf,
                 CSTR_SUFFIX(res.edge_mismatch.final_string, res.edge_mismatch.shared));
        set_child(st, breakpoint, leaf);
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
    thread_nodes(st); // Need this to get an efficient traversal
    return st;
}

// Get the suffix of a path, which is either the last edge in the path
// or the last edge minus the first character if the last edge starts
// in the root.
// It boils down to this: chop off the first character from the edge
//  if the parent is the root, and otherwise, just give us the edge.
static inline cstr_const_sslice suffix(cstr_suffix_tree *st, node *n)
{
    cstr_const_sslice y = get_edge(st, n);
    if (n->parent == st->root)
    {
        y.buf++;
        y.len--;
    }
    return y;
}

cstr_suffix_tree *
cstr_mccreight_suffix_tree(cstr_alphabet const *alpha, cstr_const_sslice x)
{
    cstr_suffix_tree *st = new_suffix_tree(alpha, x);

    // The previous suffix we inserted had the form "a|y|z|w" where
    // a is a character, ayz is head(i) and ay is parent(head(i)).
    // We can use the suffix links to get to either ayz (when head(i)
    // has a suffix link) or ay, and if we only get to ay we can fast-scan
    // through z to get to ayz. From there, we find head(i+1) by scanning
    // through w as far as we can.

    cstr_const_sslice z, w;
    inner_node *y_node;  // node where string y ends == s(ay)
    inner_node *yz_node; // node where string yz ends == s(ayz)

    node *leaf = get_suffix_leaf(st, 0);
    set_edge(st, leaf, x);
    set_child(st, st->root, leaf);

    // Avoid some special cases by making the root its own parent and suffix
    st->root->parent = st->root;
    st->root->slink = st->root;

    for (long long i = 1; i < x.len; i++)
    {
        w = suffix(st, leaf);                // leaf is ayzw, we get the last bit, w
        inner_node *ayz_node = leaf->parent; // and the parent of leaf is ayz

        if (ayz_node->slink)
        {
            // We have the link, so we jump directly from ayz to yz
            yz_node = ayz_node->slink;
        }
        else
        {
            inner_node *ay_node = ayz_node->parent;
            assert(ay_node);
            y_node = ay_node->slink;
            assert(y_node);

            z = suffix(st, (node *)ayz_node); // get z as the last edge on ayz
            scan_res res = fast_scan(st, y_node, z);
            switch (res.result)
            {
            case NODE_MATCH:
            {
                ayz_node->slink = yz_node = (inner_node *)res.node_match.n;
                break;
            }

            case EDGE_MATCH:
            {
                ayz_node->slink = break_edge(st, res.edge_match.n, res.edge_match.shared);
                leaf = get_suffix_leaf(st, i);
                set_edge(st, leaf, w);
                set_child(st, ayz_node->slink, leaf);
                continue; // We don't need the rest of the loop, we can short circuit here
            }

            default:
                assert(false); // We must have a match on fast-scan.
            }
        }

        // We have yz_node now, and to find head(i+1) we need to search for yzw, i.e. to search
        // for w from yz_node until we get a mismatch.
        scan_res res = slow_scan(st, yz_node, w);
        switch (res.result)
        {
        case NODE_MISMATCH:
        {
            leaf = get_suffix_leaf(st, i);
            set_edge(st, leaf, res.node_mismatch.final_string);
            set_child(st, (inner_node *)res.node_mismatch.n, leaf);
            break;
        }

        case EDGE_MISMATCH:
        {
            inner_node *head = break_edge(st, res.edge_mismatch.n, res.edge_mismatch.shared);
            cstr_const_sslice tail = CSTR_SUFFIX(res.edge_mismatch.final_string, res.edge_mismatch.shared);
            leaf = get_suffix_leaf(st, i);
            set_edge(st, leaf, tail);
            set_child(st, head, leaf);
            break;
        }

        default:
            assert(false); // We must have a mismatch on slow-scan.
        }
    }

    thread_nodes(st); // Need this to get an efficient traversal

    return st;
}

void cstr_free_suffix_tree(cstr_suffix_tree *st)
{
    struct sub_pool *spool = st->pool.sub_pool, *next;
    for (; spool; spool = next)
    {
        next = spool->next;
        free(spool);
    }
    free(st);
}

struct st_matcher
{
    cstr_exact_matcher matcher;
    node *n, *sentinel;
};

static inline void inc(struct st_matcher *iter)
{
    // If we are at the sentinel, the next is null, otherwise
    // we take whatever next is
    iter->n = (iter->n == iter->sentinel) ? 0 : iter->n->next;
}
static long long next_match(struct st_matcher *iter)
{
    for (; iter->n; inc(iter))
    {
        if (is_leaf(iter->n))
        {
            long long suf = iter->n->range.leaf;
            inc(iter);
            return suf;
        }
    }

    return -1; // Done
}

typedef long long (*next_f)(cstr_exact_matcher *);
typedef void (*free_f)(cstr_exact_matcher *);
static cstr_exact_matcher_vtab st_matcher_vtab = {.next = (next_f)next_match, .free = (free_f)free};

// Get the rightmost leaf in a sub-tree. We use it as a sentinel in a threaded
// traversal.
static node *rightmost_leaf(cstr_suffix_tree *st, node *n)
{
    for (;;)
    {
        if (is_leaf(n))
        {
            return n;
        }
        node **v = get_children_end(st, n);
        for (v--; *v == 0; v--)
            ;
        n = *v;
    }
}

static inline cstr_exact_matcher *matcher_from_node(cstr_suffix_tree *st, node *n)
{
    struct st_matcher *m = cstr_malloc(sizeof *m);
    m->matcher.vtab = &st_matcher_vtab;
    m->n = n;
    m->sentinel = n ? rightmost_leaf(st, n) : 0;
    return (cstr_exact_matcher *)m;
}

cstr_exact_matcher *cstr_st_exact_search(cstr_suffix_tree *st, cstr_const_sslice p)
{
    node *n = 0;
    scan_res res = slow_scan(st, st->root, p);
    switch (res.result)
    {
    case NODE_MATCH:
        n = res.node_match.n;
        break;

    case EDGE_MATCH:
        n = res.edge_match.n;
        break;

    case NODE_MISMATCH:
        // [[fallthrough]];
    case EDGE_MISMATCH:
        n = 0;
        break;
    }

    return matcher_from_node(st, n);
}

cstr_exact_matcher *cstr_st_exact_search_map(cstr_suffix_tree *st, cstr_const_sslice p)
{
    cstr_exact_matcher *m = 0;
    cstr_sslice *p_buf = cstr_alloc_sslice(p.len);
    bool ok = cstr_alphabet_map(*p_buf, p, st->alpha);
    if (ok)
    {
        m = cstr_st_exact_search(st, CSTR_SLICE_CONST_CAST(*p_buf));
    }
    else
    {
        m = matcher_from_node(st, 0); // no map means no match
    }
    free(p_buf);
    return m;
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
        node *leaf = get_suffix_leaf(st, i);
        TL_FATAL_IF_NEQ_LL(leaf->range.leaf, i);
        TL_FATAL_IF_NEQ_LL(leaf->range.beg, i);
        TL_FATAL_IF_NEQ_SLICE(CSTR_SUFFIX(x, i), get_edge(st, leaf));
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

    // Let's try adding the first leaf as a child to the root
    set_child(st, st->root, get_suffix_leaf(st, 0));

    // The root should have sigma children
    node **beg, **end;
    get_children(st, (node *)st->root, &beg, &end);
    assert(end - beg == st->alpha->size);

    int no_children = 0;
    for (node **n = beg; n != end; n++)
    {
        if (*n)
        {
            no_children++;
        }
    }
    // We should have one child of the root now
    TL_FATAL_IF_NEQ_INT(no_children, 1);

    // The child we inserted should be the one we got if we asked for that out edge
    node *child = get_child(st->root, st->x.buf[0]);
    assert(child == get_suffix_leaf(st, 0));

    // A leaf should have zero children
    get_children(st, get_suffix_leaf(st, 0), &beg, &end);
    assert(end == beg);

    // Try adding "ississippi"
    set_child(st, st->root, get_suffix_leaf(st, 1));
    // and "ssissippi"
    set_child(st, st->root, get_suffix_leaf(st, 2));
    // but not more for now, we don't want to hit used slots...

    get_children(st, (node *)st->root, &beg, &end);
    no_children = 0;
    for (node **n = beg; n != end; n++)
    {
        if (*n)
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

    // Adding "mississippi"
    set_child(st, st->root, get_suffix_leaf(st, 0));
    // "ississippi"
    set_child(st, st->root, get_suffix_leaf(st, 1));
    // and "ssissippi"
    set_child(st, st->root, get_suffix_leaf(st, 2));

    cstr_const_sslice p = CSTR_SUFFIX(x, 0);
    scan_res res = slow_scan(st, st->root, p);
    assert(res.result == NODE_MATCH);
    assert(res.node_match.n == get_suffix_leaf(st, 0));

    res = fast_scan(st, st->root, p);
    assert(res.result == NODE_MATCH);
    assert(res.node_match.n == get_suffix_leaf(st, 0));

    p = CSTR_SUFFIX(x, 1);
    res = slow_scan(st, st->root, p);
    assert(res.result == NODE_MATCH);
    assert(res.node_match.n == get_suffix_leaf(st, 1));

    res = fast_scan(st, st->root, p);
    assert(res.result == NODE_MATCH);
    assert(res.node_match.n == get_suffix_leaf(st, 1));

    p = CSTR_SUFFIX(x, 2);
    res = slow_scan(st, st->root, p);
    assert(res.result == NODE_MATCH);
    assert(res.node_match.n == get_suffix_leaf(st, 2));

    res = fast_scan(st, st->root, p);
    assert(res.result == NODE_MATCH);
    assert(res.node_match.n == get_suffix_leaf(st, 2));

    /*
     x = [2, 1, 4, 4, 1, 4, 4, 2, 2, 0]
     */
    cstr_const_sslice w = CSTR_SLICE_STRING((const char *)"\2\1\4\1"); // should mismatch on 3
    res = slow_scan(st, st->root, w);
    assert(res.result == EDGE_MISMATCH);
    assert(res.edge_mismatch.n == get_suffix_leaf(st, 0));
    assert(res.edge_mismatch.shared == 3);

    w = CSTR_SLICE_STRING((const char *)"\2\1\4"); // should match on 3
    res = slow_scan(st, st->root, w);
    assert(res.result == EDGE_MATCH);
    assert(res.edge_match.n == get_suffix_leaf(st, 0));
    assert(res.edge_match.shared == 3);

    // Try breaking the edge at this location. It won't be a suffix tree now,
    // it has a node with only one child, but its something we need later
    inner_node *new_node = break_edge(st, get_suffix_leaf(st, 0), 3LL);
    cstr_const_sslice new_edge = get_edge(st, (node *)new_node);
    assert(new_edge.len = 3LL);
    // leaf_id(0) should now have the new node as its parent
    // and a shorter edge.
    assert(get_suffix_leaf(st, 0)->parent == new_node);
    TL_FATAL_IF_NEQ_LL(get_edge(st, get_suffix_leaf(st, 0)).len, x.len - 3LL);

    cstr_free_suffix_tree(st);
    free(x_buf);

    TL_END();
}

static void thread_traverse(cstr_suffix_tree *st)
{
    node *r = (node *)st->root;
    printf("r is %p\n", (void *)r);
    node *n = r->next;
    while (n)
    {
        if (is_leaf(n))
        {
            printf("%lld\n", n->range.leaf);
        }
        n = n->next;
    }
}

TL_TEST(st_dft)
{
    TL_BEGIN();

    cstr_const_sslice u = CSTR_SLICE_STRING0((const char *)"mississippi");
    cstr_alphabet alpha;
    cstr_init_alphabet(&alpha, u);

    cstr_sslice *x_buf = cstr_alloc_sslice(u.len);
    bool ok = cstr_alphabet_map(*x_buf, u, &alpha);
    TL_FATAL_IF(!ok);
    cstr_const_sslice x = CSTR_SLICE_CONST_CAST(*x_buf);

    cstr_suffix_tree *st = cstr_naive_suffix_tree(&alpha, x);

    node *sentinel_leaf = get_child(st->root, 0);
    assert(is_leaf(sentinel_leaf));
    assert(sentinel_leaf->range.leaf == 11LL);

    node *i_node = get_child(st->root, 1);
    assert(!is_leaf(i_node));

    node *mississippi_leaf = get_child(st->root, 2);
    assert(is_leaf(mississippi_leaf));
    assert(mississippi_leaf->range.leaf == 0LL);

    node *p_node = get_child(st->root, 3);
    assert(!is_leaf(p_node));
    node *s_node = get_child(st->root, 4);
    assert(!is_leaf(s_node));

    printf("Threaded depth-first traversal\n");
    thread_traverse(st);
    printf("Done\n\n");

    printf("Iterator traversal\n");
    cstr_exact_matcher *iter = cstr_st_exact_search(st, CSTR_SLICE_STRING((const char *)""));
    for (long long suf = cstr_exact_next_match(iter);
         suf != -1;
         suf = cstr_exact_next_match(iter))
    {
        printf("%lld\n", suf);
    }
    cstr_free_exact_matcher(iter);
    printf("Done\n\n");

    cstr_free_suffix_tree(st);
    free(x_buf);

    TL_END();
}

TL_TEST(st_search)
{
    TL_BEGIN();

    cstr_const_sslice u = CSTR_SLICE_STRING0((const char *)"mississippi");
    cstr_alphabet alpha;
    cstr_init_alphabet(&alpha, u);

    cstr_sslice *x_buf = cstr_alloc_sslice(u.len);
    bool ok = cstr_alphabet_map(*x_buf, u, &alpha);
    TL_FATAL_IF(!ok);
    cstr_const_sslice x = CSTR_SLICE_CONST_CAST(*x_buf);

    cstr_suffix_tree *st = cstr_naive_suffix_tree(&alpha, x);

    printf("Search for empty string -- should match entire tree.\n");
    cstr_exact_matcher *m = cstr_st_exact_search_map(st, CSTR_SLICE_STRING((const char *)""));
    for (long long i = cstr_exact_next_match(m); i != -1; i = cstr_exact_next_match(m))
    {
        printf("%lld\n", i);
    }
    cstr_free_exact_matcher(m);

    printf("Search for `missi` -- should suffix 0 only.\n");
    m = cstr_st_exact_search_map(st, CSTR_SLICE_STRING((const char *)"missi"));
    for (long long i = cstr_exact_next_match(m); i != -1; i = cstr_exact_next_match(m))
    {
        printf("%lld\n", i);
    }
    cstr_free_exact_matcher(m);
    m = cstr_st_exact_search_map(st, CSTR_SLICE_STRING((const char *)"missi"));
    TL_FATAL_IF_NEQ_LL(cstr_exact_next_match(m), 0LL);
    TL_FATAL_IF_NEQ_LL(cstr_exact_next_match(m), -1LL);
    cstr_free_exact_matcher(m);

    printf("Search for `i` -- should get 10, 7, 4, and 1.\n");
    m = cstr_st_exact_search_map(st, CSTR_SLICE_STRING((const char *)"i"));
    for (long long i = cstr_exact_next_match(m); i != -1; i = cstr_exact_next_match(m))
    {
        printf("%lld\n", i);
    }
    cstr_free_exact_matcher(m);
    m = cstr_st_exact_search_map(st, CSTR_SLICE_STRING((const char *)"i"));
    TL_FATAL_IF_NEQ_LL(cstr_exact_next_match(m), 10LL);
    TL_FATAL_IF_NEQ_LL(cstr_exact_next_match(m), 7LL);
    TL_FATAL_IF_NEQ_LL(cstr_exact_next_match(m), 4LL);
    TL_FATAL_IF_NEQ_LL(cstr_exact_next_match(m), 1LL);
    TL_FATAL_IF_NEQ_LL(cstr_exact_next_match(m), -1LL);
    cstr_free_exact_matcher(m);

    cstr_free_suffix_tree(st);
    free(x_buf);

    TL_END();
}

#endif // GEN_UNIT_TESTS
