// Humble implementation of a dynamic array

#ifndef _DYNARR_H_
#define _DYNARR_H_

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#ifndef DYNARR_ALIGNMENT
#define DYNARR_ALIGNMENT 8
#endif

#ifndef DYNARR_DEFAULT_GROW_SIZE
#define DYNARR_DEFAULT_GROW_SIZE 8
#endif

typedef struct _dynarr_allocator_
{
    void *(*alloc)(size_t size);
    void *(*realloc)(void *ptr, size_t size);
    void (*dealloc)(void *ptr);
} DynArrAllocator;

typedef struct _dynarr_
{
    size_t count;
    size_t used;
    size_t size;
    void *items;
    struct _dynarr_allocator_ *allocator;
} DynArr;

typedef struct _dynarr_ptr_
{
    size_t used;
    size_t count;
    void **items;
    struct _dynarr_allocator_ *allocator;
} DynArrPtr;

// interface

// DynArr
struct _dynarr_ *dynarr_create(size_t item_size, struct _dynarr_allocator_ *allocator);
void dynarr_destroy(struct _dynarr_ *dynarr);

#define DYNARR_LEN(dynarr) (dynarr->used)
#define DYNARR_AVAILABLE(dynarr) (dynarr->count - dynarr->used)

void *dynarr_get(size_t index, struct _dynarr_ *dynarr);
void dynarr_set(void *item, size_t index, struct _dynarr_ *dynarr);
int dynarr_insert(void *item, struct _dynarr_ *dynarr);
void dynarr_remove_index(size_t index, struct _dynarr_ *dynarr);
void dynarr_remove_all(struct _dynarr_ *dynarr);

// DynArrPtr
struct _dynarr_ptr_ *dynarr_ptr_create(struct _dynarr_allocator_ *allocator);
void dynarr_ptr_destroy(struct _dynarr_ptr_ *dynarr);

#define DYNARR_PTR_SIZE(count, dynarr) (sizeof(void *) * (count))
#define DYNARR_PTR_POSITION(position, dynarr) (dynarr->items + position)
#define DYNARR_PTR_GET(index, dynarr) *DYNARR_PTR_POSITION(index, dynarr)

int dynarr_ptr_resize(size_t new_count, struct _dynarr_ptr_ *dynarr);

void dynarr_ptr_set(size_t index, void *value, struct _dynarr_ptr_ *dynarr);
int dynarr_ptr_insert(void *ptr, struct _dynarr_ptr_ *dynarr);
void dynarr_ptr_move(size_t index, size_t from, size_t to, struct _dynarr_ptr_ *dynarr);
void dynarr_ptr_remove_index(size_t index, struct _dynarr_ptr_ *dynarr);
void dynarr_ptr_remove_ptr(void *ptr, struct _dynarr_ptr_ *dynarr);

#endif