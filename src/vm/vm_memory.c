#include "vm_memory.h"

static int initialized = 0;
static LZAllocator *allocator = NULL;
static DynArrAllocator dynarr_allocator = {0};
static LZStackAllocator lzstack_allocator = {0};
static LZHTableAllocator lzhtable_allocator = {0};

size_t vm_memory_all_space()
{
    assert(initialized && "You need to call vm_memory_init");

    return allocator->bytes;
}

size_t vm_memory_used_space()
{
    assert(initialized && "You need to call vm_memory_init");

    size_t available = lzallocator_available_space(allocator);

    return allocator->bytes - available;
}

void vm_memory_print_blocks()
{
    LZAllocatorHeader *header = allocator->blocks;

    while (header)
    {
        LZAllocatorHeader *next = header->next;

        printf("free: %d\n", header->free);
        printf("size: %ld\n", header->size);

        header = next;

        if (header)
            printf("\n");
    }
}

void vm_memory_report_space()
{
    size_t used = vm_memory_used_space();
    size_t all = vm_memory_all_space();

    printf("%ld/%ld bytes\n", used, all);
}

int vm_memory_init()
{
    if (initialized)
        return 0;

    dynarr_allocator.alloc = _alloc_;
    dynarr_allocator.realloc = _realloc_;
    dynarr_allocator.dealloc = _dealloc_;

    lzstack_allocator.alloc = _alloc_;
    lzstack_allocator.dealloc = _dealloc_;

    lzhtable_allocator.alloc = _alloc_;
    lzhtable_allocator.realloc = _realloc_;
    lzhtable_allocator.dealloc = _dealloc_;

    allocator = lzallocator_create(lzallocator_mib(8));

    if (!allocator)
        return 1;

    initialized = 1;

    return 0;
}

void vm_memory_deinit()
{
    if (!initialized)
        return;

    lzallocator_destroy(allocator);

    initialized = 0;
}

void *vm_memory_alloc(size_t bytes)
{
    assert(initialized && "You need to call vm_memory_init");

    void *ptr = lzallocator_alloc(bytes, allocator, NULL);

    if (!ptr)
        vm_memory_deinit();

    assert(ptr && "No space to allocate");

    return ptr;
}

void *vm_memory_calloc(size_t bytes)
{
    assert(initialized && "You need to call vm_memory_init");

    void *ptr = lzallocator_calloc(bytes, allocator, NULL);

    if (!ptr)
        vm_memory_deinit();

    assert(ptr && "No space to allocate");

    return ptr;
}

void *vm_memory_realloc(size_t bytes, void *ptr)
{
    assert(initialized && "You need to call vm_memory_init");

    void *new_ptr = lzallocator_realloc(bytes, ptr, allocator, NULL);

    if (!new_ptr)
        vm_memory_deinit();

    assert(new_ptr && "No space to allocate");

    return new_ptr;
}

void vm_memory_dealloc(void *ptr)
{
    if (!ptr)
        return;

    assert(initialized && "You need to call vm_memory_init");

    lzallocator_dealloc(ptr, allocator);
}

void *_alloc_(size_t size)
{
    return vm_memory_alloc(size);
}

void *_realloc_(void *ptr, size_t size)
{
    return vm_memory_realloc(size, ptr);
}

void _dealloc_(void *ptr)
{
    vm_memory_dealloc(ptr);
}

char *vm_memory_clone_string(char *string)
{
    size_t length = strlen(string);
    char *clone_string = vm_memory_alloc(length + 1);

    memcpy(clone_string, string, length);
    clone_string[length] = 0;

    return clone_string;
}

Fn *vm_memory_create_fn(char *name)
{
    Fn *fn = vm_memory_alloc(sizeof(Fn));

    fn->name = vm_memory_clone_string(name);
    fn->params = vm_memory_create_dynarr_ptr();
    fn->chunks = vm_memory_create_dynarr(sizeof(uint8_t));

    return fn;
}

void vm_memory_destroy_fn(Fn *fn)
{
    if (!fn)
        return;

    DynArrPtr *strs = fn->params;
    size_t strs_len = strs->used;

    for (size_t i = 0; i < strs_len; i++)
        vm_memory_dealloc(DYNARR_PTR_GET(i, strs));

    vm_memory_dealloc(fn->name);
    vm_memory_destroy_dynarr_ptr(fn->params);
    vm_memory_destroy_dynarr(fn->chunks);

    fn->name = NULL;
    fn->params = NULL;
    fn->chunks = NULL;

    vm_memory_dealloc(fn);
}

DynArr *vm_memory_create_dynarr(size_t bytes)
{
    return dynarr_create(bytes, &dynarr_allocator);
}

void vm_memory_destroy_dynarr(DynArr *array)
{
    if (!array)
        return;

    dynarr_destroy(array);
}

DynArrPtr *vm_memory_create_dynarr_ptr()
{
    return dynarr_ptr_create(&dynarr_allocator);
}

void vm_memory_destroy_dynarr_ptr(DynArrPtr *array)
{
    if (!array)
        return;

    dynarr_ptr_destroy(array);
}

LZStack *vm_memory_create_lzstack()
{
    return lzstack_create(&lzstack_allocator);
}

void vm_memory_destroy_lzstack(LZStack *stack)
{
    lzstack_destroy(stack);
}

LZHTable *vm_memory_create_lzhtable(size_t length)
{
    return lzhtable_create(length, &lzhtable_allocator);
}

void vm_memory_destroy_lzhtable(LZHTable *table)
{
    if (!table)
        return;

    lzhtable_destroy(table);
}

Klass *vm_memory_create_klass(char *name)
{
    Klass *klass = (Klass *)vm_memory_alloc(sizeof(Klass));

    klass->constructor = NULL;
    klass->name = vm_memory_clone_string(name);
    klass->methods = vm_memory_create_lzhtable(21);

    return klass;
}

void vm_memory_destroy_klass(Klass *klass)
{
    if (!klass)
        return;

    LZHTable *methods_table = klass->methods;
    LZHTableNode *node = methods_table->nodes;

    while (node)
    {
        LZHTableNode *prev = node->previous_table_node;
        Fn *fn = (Fn *)node->value;

        vm_memory_destroy_fn(fn);

        node = prev;
    }

    vm_memory_dealloc(klass->name);
    vm_memory_destroy_fn(klass->constructor);
    vm_memory_destroy_lzhtable(klass->methods);

    klass->name = NULL;
    klass->methods = NULL;

    vm_memory_dealloc(klass);
}

Instance *vm_memory_create_instance(Klass *container)
{
    Instance *instance = vm_memory_alloc(sizeof(Instance));

    instance->attributes = vm_memory_create_lzhtable(21);
    instance->klass = container;

    return instance;
}

void vm_memory_destroy_instance(Instance *instance)
{
    if (!instance)
        return;

    vm_memory_destroy_lzhtable(instance->attributes);

    instance->attributes = NULL;
    instance->klass = NULL;

    vm_memory_dealloc(instance);
}

Object *vm_memory_create_object(ObjectType type, VM *vm)
{
    Object *object = (Object *)vm_memory_alloc(sizeof(struct _object_));

    memset((void *)object, 0, sizeof(struct _object_));

    object->type = type;

    if (vm->head_object)
        vm->tail_object->next = object;
    else
        vm->head_object = object;

    vm->tail_object = object;

    vm->size += sizeof(Object);

    return object;
}

void vm_memory_destroy_object(Object *object)
{
    if (!object)
        return;

    vm_memory_dealloc(object);
}