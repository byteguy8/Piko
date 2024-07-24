#include "lzarea.h"

#include <string.h>
#include <stdio.h>
#include <assert.h>

// private interface
static void *_alloc_(size_t size, struct _lzarea_allocator_ *allocator);
static void *_realloc_(void *ptr, size_t size, struct _lzarea_allocator_ *allocator);
static void _dealloc_(void *ptr, struct _lzarea_allocator_ *allocator);

struct _lzlinear_ *lzlinear_create(size_t block_size, size_t block_count, struct _lzarea_allocator_ *allocator);
void lzlinear_destroy(struct _lzlinear_ *linear, struct _lzarea_allocator_ *allocator);

void lzlinear_print_slots(size_t index, struct _lzlinear_ *linear);
int lzlinear_is_slot(void *ptr, struct _lzlinear_ *linear);
#define LZLINEAR_USED_SPACE(linear) (linear->used_bytes)
#define LZLINEAR_AVAILABLE_SPACE(linear) (linear->slot_size * linear->slot_count - linear->used_bytes)

void *lzlinear_alloc(struct _lzlinear_ *linear);
void lzlinear_dealloc(void *ptr, struct _lzlinear_ *linear);

static void *_get_header_(size_t position, struct _lzlinear_ *linear_slots);
static struct _lzlinear_container_ *_has_container_slot_size_(size_t slot_size, struct _lzarea_ *area);
static struct _lzlinear_container_ *_has_container_slot_(void *ptr, struct _lzarea_ *area);

// private implementation
void *_alloc_(size_t size, struct _lzarea_allocator_ *allocator)
{
    return allocator ? allocator->alloc(size) : malloc(size);
}

void *_realloc_(void *ptr, size_t size, struct _lzarea_allocator_ *allocator)
{
    return allocator ? allocator->realloc(ptr, size) : realloc(ptr, size);
}

void _dealloc_(void *ptr, struct _lzarea_allocator_ *allocator)
{
    if (!ptr)
        return;

    if (allocator)
    {
        allocator->dealloc(ptr);
        return;
    }

    free(ptr);
}

struct _lzlinear_ *lzlinear_create(size_t slot_size, size_t slot_count, struct _lzarea_allocator_ *allocator)
{
    size_t size = (slot_size + 1) * slot_count;
    void *slots = _alloc_(size, allocator);
    struct _lzlinear_ *linear = (struct _lzlinear_ *)_alloc_(sizeof(struct _lzlinear_), allocator);

    if (!slots || !linear)
    {
        _dealloc_(slots, allocator);
        _dealloc_(linear, allocator);

        return NULL;
    }

    memset(slots, 0, size);

    linear->slot_size = slot_size;
    linear->slot_count = slot_count;
    linear->used_bytes = 0;
    linear->slots = slots;
    linear->next = NULL;

    return linear;
}

void lzlinear_destroy(struct _lzlinear_ *linear, struct _lzarea_allocator_ *allocator)
{
    if (!linear)
        return;

    _dealloc_(linear->slots, allocator);

    memset(linear, 0, sizeof(struct _lzlinear_));

    _dealloc_(linear, allocator);
}

void lzlinear_print_slots(size_t from, struct _lzlinear_ *linear)
{
    for (size_t i = 0; i < linear->slot_count; i++)
    {
        char *header = _get_header_(i, linear);
        printf("%ld: %s, %ld\n", from + i, *header == 0 ? "free" : "used", linear->slot_size);
    }
}

int lzlinear_is_slot(void *ptr, struct _lzlinear_ *linear)
{
    return ptr >= linear->slots && ptr <= _get_header_(linear->slot_count - 1, linear) + linear->slot_size;
}

void *lzlinear_alloc(struct _lzlinear_ *linear)
{
    // 0 = not used
    // 1 = used

    for (size_t i = 0; i < linear->slot_count; i++)
    {
        char *header = _get_header_(i, linear);

        if (*header == 0)
        {
            *header = 1;

            linear->used_bytes += linear->slot_size;

            return header + 1;
        }
    }

    return NULL;
}

void lzlinear_dealloc(void *ptr, struct _lzlinear_ *linear)
{
    if (!ptr)
        return;

    if (!lzlinear_is_slot(ptr, linear))
        return;

    char *header = ((char *)ptr) - 1;

    if (*header == 0)
        return;

    *header = 0;

    linear->used_bytes -= linear->slot_size;
}

void *_get_header_(size_t position, struct _lzlinear_ *linear)
{
    size_t skip_size = linear->slot_size + 1;
    char *slots = (char *)linear->slots;

    return (void *)(slots + skip_size * position);
}

struct _lzlinear_container_ *_has_container_slot_size_(size_t slot_size, struct _lzarea_ *area)
{
    struct _lzlinear_container_ *container = area->head;

    while (container)
    {
        struct _lzlinear_container_ *next = container->next;

        if (container->slot_size == slot_size)
            return container;

        container = next;
    }

    return NULL;
}

static struct _lzlinear_container_ *_has_container_slot_(void *ptr, struct _lzarea_ *area)
{
    struct _lzlinear_container_ *container = area->head;

    while (container)
    {
        struct _lzlinear_container_ *next = container->next;

        if (lzlinear_container_is_slot(ptr, container))
            return container;

        container = next;
    }

    return NULL;
}

// public implementation
struct _lzlinear_container_ *lzlinear_container_create(size_t slot_size, struct _lzarea_allocator_ *allocator)
{
    struct _lzlinear_container_ *container = (struct _lzlinear_container_ *)_alloc_(sizeof(struct _lzlinear_container_), allocator);

    if (!container)
    {
        _dealloc_(container, allocator);

        return NULL;
    }

    container->size = 0;

    container->slot_size = slot_size;

    container->bytes = 0;
    container->used_bytes = 0;

    container->head = NULL;
    container->tail = NULL;

    container->next = NULL;

    container->allocator = allocator;

    return container;
}

void lzlinear_container_destroy(struct _lzlinear_container_ *container)
{
    if (!container)
        return;

    struct _lzlinear_ *slots = container->head;
    struct _lzarea_allocator_ *allocator = container->allocator;

    while (slots)
    {
        struct _lzlinear_ *next = slots->next;

        lzlinear_destroy(slots, allocator);

        slots = next;
    }

    memset(container, 0, sizeof(struct _lzlinear_container_));

    _dealloc_(container, allocator);
}

void lzlinear_container_print_slots(struct _lzlinear_container_ *container)
{
    size_t index = 0;
    struct _lzlinear_ *linear = container->head;

    while (linear)
    {
        struct _lzlinear_ *next = linear->next;

        lzlinear_print_slots(index, linear);

        index += linear->slot_count;

        linear = next;
    }
}

int lzlinear_container_is_slot(void *ptr, struct _lzlinear_container_ *container)
{
    struct _lzlinear_ *linear = container->head;

    while (linear)
    {
        struct _lzlinear_ *next = linear->next;

        if (lzlinear_is_slot(ptr, linear))
            return 1;

        linear = next;
    }

    return 0;
}

int lzlinear_container_add_linear(size_t slot_count, struct _lzlinear_container_ *container)
{
    struct _lzarea_allocator_ *allocator = container->allocator;
    struct _lzlinear_ *linear = lzlinear_create(container->slot_size, slot_count, allocator);

    if (!linear)
        return 1;

    if (container->size == 0)
        container->head = linear;
    else
        container->tail->next = linear;

    container->size++;
    container->tail = linear;

    container->bytes += container->slot_size * slot_count;

    return 0;
}

void *lzlinear_container_alloc(size_t slot_count, struct _lzlinear_container_ *container)
{
    if (LZLINEAR_CONTAINER_AVAILABLE(container) == 0)
    {
        if (slot_count == 0)
            return NULL;

        if (lzlinear_container_add_linear(slot_count, container))
            return NULL;

        struct _lzlinear_ *tail = container->tail;

        container->used_bytes += container->slot_size;

        return lzlinear_alloc(tail);
    }

    struct _lzlinear_ *linear = container->head;

    while (linear)
    {
        struct _lzlinear_ *next = linear->next;
        size_t available = LZLINEAR_AVAILABLE_SPACE(linear);

        if (available > 0)
        {
            container->used_bytes += container->slot_size;

            return lzlinear_alloc(linear);
        }

        linear = next;
    }

    return NULL;
}

void lzlinear_container_dealloc(void *ptr, struct _lzlinear_container_ *container)
{
    if (!ptr)
        return;

    struct _lzlinear_ *linear = container->head;

    while (linear)
    {
        struct _lzlinear_ *next = linear->next;

        if (lzlinear_is_slot(ptr, linear))
        {
            container->used_bytes -= linear->slot_size;

            lzlinear_dealloc(ptr, linear);

            return;
        }

        linear = next;
    }
}

struct _lzarea_ *lzarea_create(struct _lzarea_allocator_ *allocator)
{
    struct _lzarea_ *area = (struct _lzarea_ *)_alloc_(sizeof(struct _lzarea_), allocator);

    if (!area)
        return NULL;

    area->size = 0;

    area->head = NULL;
    area->tail = NULL;

    area->allocator = allocator;

    return area;
}

void lzarea_destroy(struct _lzarea_ *area)
{
    if (!area)
        return;

    struct _lzlinear_container_ *container = area->head;
    struct _lzarea_allocator_ *allocator = area->allocator;

    while (container)
    {
        struct _lzlinear_container_ *next = container->next;

        lzlinear_container_destroy(container);

        container = next;
    }

    memset(area, 0, sizeof(struct _lzarea_));

    _dealloc_(area, allocator);
}

int lzarea_add_container(size_t slot_size, struct _lzarea_ *area, struct _lzlinear_container_ **out_container)
{
    struct _lzlinear_container_ *container = _has_container_slot_size_(slot_size, area);

    if (container)
    {
        if (out_container)
            *out_container = container;

        return 0;
    }

    container = lzlinear_container_create(slot_size, area->allocator);

    if (!container)
        return 1;

    if (area->size == 0)
        area->head = container;
    else
        area->tail->next = container;

    area->size++;
    area->tail = container;

    if (out_container)
        *out_container = container;

    return 0;
}

void *lzarea_alloc(size_t bytes, size_t slot_count, struct _lzarea_ *area)
{
    struct _lzlinear_container_ *container = NULL;

    if (lzarea_add_container(bytes, area, &container))
        return NULL;

    return lzlinear_container_alloc(slot_count, container);
}

void lzarea_dealloc(void *ptr, struct _lzarea_ *area)
{
    struct _lzlinear_container_ *container = _has_container_slot_(ptr, area);

    if(!container)
        return;

    lzlinear_container_dealloc(ptr, container);
}