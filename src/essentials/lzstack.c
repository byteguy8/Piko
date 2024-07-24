#include "lzstack.h"

// private interface
static void *_alloc_(size_t bytes, struct _lzstack_allocator_ *allocator);
static void _dealloc_(void *ptr, struct _lzstack_allocator_ *allocator);

// private implementation
static void *_alloc_(size_t bytes, struct _lzstack_allocator_ *allocator)
{
    return allocator ? allocator->alloc(bytes) : malloc(bytes);
}

static void _dealloc_(void *ptr, struct _lzstack_allocator_ *allocator)
{
    if (!ptr)
        return;

    if (allocator)
        allocator->dealloc(ptr);
    else
        free(ptr);
}

// public implementation
struct _lzstack_ *lzstack_create(struct _lzstack_allocator_ *allocator)
{
    struct _lzstack_ *stack = (struct _lzstack_ *)_alloc_(sizeof(struct _lzstack_), allocator);

    if (!stack)
        return NULL;

    stack->nodes = NULL;
    stack->allocator = allocator;

    return stack;
}

void lzstack_destroy(struct _lzstack_ *stack)
{
    if (!stack)
        return;

    struct _lzstack_node_ *node = stack->nodes;

    while (node)
    {
        struct _lzstack_node_ *previous = node->previous;

        node->value = NULL;
        node->previous = NULL;

        _dealloc_(node, stack->allocator);

        node = previous;
    }

    struct _lzstack_allocator_ *allocator = stack->allocator;

    _dealloc_(stack, allocator);
}

void *lzstack_peek(struct _lzstack_ *stack, struct _lzstack_node_ **out_node)
{
    struct _lzstack_node_ *node = stack->nodes;

    if (!node)
        return NULL;

    if (out_node)
        *out_node = node;

    return node->value;
}

int lzstack_push(void *value, struct _lzstack_ *stack, struct _lzstack_node_ **out_node)
{
    struct _lzstack_node_ *node = (struct _lzstack_node_ *)_alloc_(sizeof(struct _lzstack_node_), stack->allocator);

    if (!node)
        return 1;

    node->value = value;
    node->previous = stack->nodes;

    stack->nodes = node;

    if (out_node)
        *out_node = node;

    return 0;
}

void *lzstack_pop(struct _lzstack_ *stack)
{
    struct _lzstack_node_ *node = stack->nodes;

    if (!node)
        return NULL;

    stack->nodes = node->previous;

    void *value = node->value;

    node->value = NULL;
    node->previous = NULL;

    _dealloc_(node, stack->allocator);

    return value;
}

int lzstack_count(struct _lzstack_ *stack)
{
    int count = 0;

    struct _lzstack_node_ *node = stack->nodes;

    while (node)
    {
        struct _lzstack_node_ *previous = node->previous;

        count++;

        node = previous;
    }

    return count;
}