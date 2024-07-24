#ifndef _LZPOOL_H_
#define _LZPOOL_H_

#include <stdlib.h>

typedef struct _lzarea_allocator_
{
    void *(*alloc)(size_t size);
    void *(*realloc)(void *ptr, size_t size);
    void (*dealloc)(void *ptr);
} LZAreaAllocator;

typedef struct _lzlinear_
{
    size_t slot_size;
    size_t slot_count;
    size_t used_bytes;
    void *slots;
    struct _lzlinear_ *next;
} LZLinear;

typedef struct _lzlinear_container_
{
    size_t size;

    size_t slot_size;

    size_t bytes;
    size_t used_bytes;

    struct _lzlinear_ *head;
    struct _lzlinear_ *tail;

    struct _lzlinear_container_ *next;

    struct _lzarea_allocator_ *allocator;
} LZLinearContainer;

typedef struct _lzarea_
{
    size_t size;

    struct _lzlinear_container_ *head;
    struct _lzlinear_container_ *tail;

    struct _lzarea_allocator_ *allocator;
} LZArea;

struct _lzlinear_container_ *lzlinear_container_create(size_t slot_size, struct _lzarea_allocator_ *allocator);
void lzlinear_container_destroy(struct _lzlinear_container_ *linear);

void lzlinear_container_print_slots(struct _lzlinear_container_ *linear);
int lzlinear_container_is_slot(void *ptr, struct _lzlinear_container_ *linear);
int lzlinear_container_add_linear(size_t slot_count, struct _lzlinear_container_ *container);
#define LZLINEAR_CONTAINER_AVAILABLE(container) (container->bytes - container->used_bytes)

void *lzlinear_container_alloc(size_t slot_count, struct _lzlinear_container_ *container);
void lzlinear_container_dealloc(void *ptr, struct _lzlinear_container_ *container);

struct _lzarea_ *lzarea_create(struct _lzarea_allocator_ *allocator);
void lzarea_destroy(struct _lzarea_ *area);

int lzarea_add_container(size_t slot_size, struct _lzarea_ *area, struct _lzlinear_container_ **out_container);

void *lzarea_alloc(size_t bytes, size_t slot_count, struct _lzarea_ * area);
void lzarea_dealloc(void *ptr, struct _lzarea_ * area);

#endif