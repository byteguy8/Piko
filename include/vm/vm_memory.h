#ifndef _VM_MEMORY_H_
#define _VM_MEMORY_H_

#include "essentials/dynarr.h"
#include "essentials/lzallocator.h"
#include "essentials/lzhtable.h"
#include "essentials/lzstack.h"

#include "function.h"
#include "klass.h"
#include "object.h"
#include "vm.h"

#include <stddef.h>

size_t vm_memory_all_space();
size_t vm_memory_used_space();
void vm_memory_print_blocks();
void vm_memory_report_space();

int vm_memory_init();
void vm_memory_deinit();

void *vm_memory_alloc(size_t bytes);
void *vm_memory_calloc(size_t bytes);
void *vm_memory_realloc(size_t bytes, void *ptr);
void vm_memory_dealloc(void *ptr);

void *_alloc_(size_t size);
void *_realloc_(void *ptr, size_t size);
void _dealloc_(void *ptr);

char *vm_memory_clone_string(char *string);

Fn *vm_memory_create_fn(char *name);
void vm_memory_destroy_fn(Fn *fn);

DynArr *vm_memory_create_dynarr(size_t bytes);
void vm_memory_destroy_dynarr(DynArr *array);

DynArrPtr *vm_memory_create_dynarr_ptr();
void vm_memory_destroy_dynarr_ptr(DynArrPtr *array);

LZStack *vm_memory_create_lzstack();
void vm_memory_destroy_lzstack(LZStack *stack);

LZHTable *vm_memory_create_lzhtable(size_t length);
void vm_memory_destroy_lzhtable(LZHTable *table);

Klass *vm_memory_create_klass(char *name);
void vm_memory_destroy_klass(Klass *klass);

Instance *vm_memory_create_instance(Klass *container);
void vm_memory_destroy_instance(Instance *instance);

Object *vm_memory_create_object(ObjectType type, VM *vm);
void vm_memory_destroy_object(Object *object);

#endif