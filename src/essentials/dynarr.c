#include "dynarr.h"

// private interface
static void *_alloc_(size_t bytes, struct _dynarr_allocator_ *allocator);
static void *_realloc_(void *ptr, size_t bytes, struct _dynarr_allocator_ *allocator);
static void _dealloc_(void *ptr, struct _dynarr_allocator_ *allocator);

#define DYNARR_DETERMINATE_GROW(count) (count == 0 ? DYNARR_DEFAULT_GROW_SIZE : count * 2)

size_t dynarr_padding(size_t item_size);
#define DYNARR_SIZE(padding, count, dynarr) ((dynarr->size + padding) * count)
#define DYNARR_POSITION(position, dynarr) (dynarr->items + DYNARR_SIZE(dynarr_padding(dynarr->size), position, dynarr))

int dynarr_resize(size_t padding, size_t new_count, struct _dynarr_ *dynarr);

// private implementation
void *_alloc_(size_t bytes, struct _dynarr_allocator_ *allocator)
{
    return allocator ? allocator->alloc(bytes) : malloc(bytes);
}

void *_realloc_(void *ptr, size_t bytes, struct _dynarr_allocator_ *allocator)
{
    return allocator ? allocator->realloc(ptr, bytes) : realloc(ptr, bytes);
}

void _dealloc_(void *ptr, struct _dynarr_allocator_ *allocator)
{
    if (!ptr)
        return;

    if (allocator)
        allocator->dealloc(ptr);
    else
        free(ptr);
}

size_t dynarr_padding(size_t item_size)
{
    size_t modulo = item_size & (DYNARR_ALIGNMENT - 1);
    return DYNARR_ALIGNMENT - modulo;
}

// public implementation
struct _dynarr_ *dynarr_create(size_t item_size, struct _dynarr_allocator_ *allocator)
{
    size_t bytes = sizeof(struct _dynarr_);
    struct _dynarr_ *vector = (struct _dynarr_ *)_alloc_(bytes, allocator);

    vector->used = 0;
    vector->count = 0;
    vector->size = item_size;
    vector->items = NULL;
    vector->allocator = allocator;

    return vector;
}

void dynarr_destroy(struct _dynarr_ *dynarr)
{
    if (!dynarr)
        return;

    struct _dynarr_allocator_ *allocator = dynarr->allocator;

    _dealloc_(dynarr->items, allocator);

    dynarr->used = 0;
    dynarr->count = 0;
    dynarr->items = NULL;
    dynarr->allocator = NULL;

    _dealloc_(dynarr, allocator);
}

int dynarr_resize(size_t padding, size_t new_count, struct _dynarr_ *dynarr)
{
    void *items = _realloc_(dynarr->items, DYNARR_SIZE(padding, new_count, dynarr), dynarr->allocator);

    if (!items)
        return 1;

    dynarr->items = items;
    dynarr->count = new_count;

    return 0;
}

void *dynarr_get(size_t index, struct _dynarr_ *dynarr)
{
    void *value = DYNARR_POSITION(index, dynarr);
    return value;
}

void dynarr_set(void *item, size_t index, struct _dynarr_ *dynarr)
{
    memcpy(DYNARR_POSITION(index, dynarr), item, dynarr->size);
}

int dynarr_insert(void *item, struct _dynarr_ *dynarr)
{
    if (dynarr->used >= dynarr->count)
    {
        size_t padding = dynarr_padding(dynarr->size);
        size_t count = DYNARR_DETERMINATE_GROW(dynarr->count);

        if (dynarr_resize(padding, count, dynarr))
            return 1;
    }

    void *slot = DYNARR_POSITION(dynarr->used++, dynarr);

    memcpy(slot, item, dynarr->size);

    return 0;
}

void dynarr_remove_index(size_t index, struct _dynarr_ *dynarr)
{
    if (index < dynarr->count - 1)
    {
        void *in = DYNARR_POSITION(index, dynarr);
        void *from = DYNARR_POSITION(index + 1, dynarr);
        size_t padding = dynarr_padding(dynarr->size);
        size_t count = dynarr->used - 1 - index;
        size_t size = DYNARR_SIZE(padding, count, dynarr);

        memmove(in, from, size);
    }

    if (dynarr->used - 1 > DYNARR_DEFAULT_GROW_SIZE && dynarr->used - 1 < dynarr->count / 2)
    {
        size_t padding = dynarr_padding(dynarr->size);
        size_t new_count = dynarr->count / 2;

        dynarr_resize(padding, new_count, dynarr);
    }

    dynarr->used--;
}

void dynarr_remove_all(struct _dynarr_ *dynarr)
{
    size_t padding = dynarr_padding(dynarr->size);
    dynarr_resize(padding, DYNARR_DEFAULT_GROW_SIZE, dynarr);
    dynarr->used = 0;
}

struct _dynarr_ptr_ *dynarr_ptr_create(struct _dynarr_allocator_ *allocator)
{
    struct _dynarr_ptr_ *dynarr = (struct _dynarr_ptr_ *)(allocator == NULL ? malloc(sizeof(struct _dynarr_ptr_)) : allocator->alloc(sizeof(struct _dynarr_ptr_)));

    if (!dynarr)
        return NULL;

    dynarr->used = 0;
    dynarr->count = 0;
    dynarr->items = NULL;
    dynarr->allocator = allocator;

    return dynarr;
}

void dynarr_ptr_destroy(struct _dynarr_ptr_ *dynarr)
{
    if (!dynarr)
        return;

    if (dynarr->allocator)
        dynarr->allocator->dealloc(dynarr->items);
    else
        free(dynarr->items);

    dynarr->used = 0;
    dynarr->count = 0;
    dynarr->items = NULL;

    if (dynarr->allocator)
        dynarr->allocator->dealloc(dynarr);
    else
        free(dynarr);
}

int dynarr_ptr_resize(size_t new_count, struct _dynarr_ptr_ *dynarr)
{
    void **items = NULL;

    if (dynarr->allocator)
        items = (void **)dynarr->allocator->realloc(dynarr->items, sizeof(void *) * new_count);
    else
        items = (void **)realloc(dynarr->items, sizeof(void *) * new_count);

    if (!items)
        return 1;

    for (size_t i = dynarr->count; i < new_count; i++)
    {
        items[i] = NULL;
    }

    dynarr->items = items;
    dynarr->count = new_count;

    return 0;
}

void dynarr_ptr_set(size_t index, void *value, struct _dynarr_ptr_ *dynarr)
{
    *DYNARR_PTR_POSITION(index, dynarr) = value;
}

int dynarr_ptr_insert(void *ptr, struct _dynarr_ptr_ *dynarr)
{
    if (dynarr->used >= dynarr->count && dynarr_ptr_resize(DYNARR_DETERMINATE_GROW(dynarr->count), dynarr))
        return 1;

    *(dynarr->items + (dynarr->used++)) = ptr;

    return 0;
}

void dynarr_ptr_move(size_t index, size_t from, size_t to, struct _dynarr_ptr_ *dynarr)
{
    for (size_t o = index, i = from; i < to; o++, i++)
        dynarr->items[o] = dynarr->items[i];
}

void dynarr_ptr_remove_index(size_t index, struct _dynarr_ptr_ *dynarr)
{
    if (index < dynarr->used - 1)
        dynarr_ptr_move(index, index + 1, dynarr->used, dynarr);

    if (dynarr->used - 1 > DYNARR_DEFAULT_GROW_SIZE && dynarr->used - 1 < dynarr->count / 2)
        dynarr_ptr_resize(dynarr->count / 2, dynarr);

    dynarr->used--;
}

void dynarr_ptr_remove_ptr(void *ptr, struct _dynarr_ptr_ *dynarr)
{
    for (size_t i = 0; i < dynarr->used; i++)
    {
        if (*(dynarr->items + i) == ptr)
        {
            dynarr_ptr_remove_index(i, dynarr);
            return;
        }
    }
}