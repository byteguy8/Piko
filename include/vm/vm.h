#ifndef _VM_H_
#define _VM_H_

#include "opcode.h"
#include "frame.h"
#include "holder.h"
#include "function.h"
#include "klass.h"

#include <essentials/dynarr.h>
#include <essentials/lzstack.h>
#include <essentials/lzhtable.h>
#include <essentials/lzallocator.h>

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#define VM_STACK_LENGTH 255
#define VM_FRAME_LENGTH 255

typedef enum _entity_type_
{
    FUNCTION_SYMTYPE,
    NATIVE_SYMTYPE,
    CLASS_SYMTYPE
} SymbolType;

typedef struct _entity_
{
    enum _entity_type_ type;
    void *raw_symbol;
} Entity;

typedef struct _vm_
{
    char halt;
    char stop;
    int rtn_code;

    int stack_ptr;
    Holder stack[VM_STACK_LENGTH];

    int frame_ptr;
    Frame frames[VM_FRAME_LENGTH];

    DynArr *iconsts;
    DynArrPtr *strings;
    DynArr *entities;
    LZHTable *globals;

    LZStack *blocks_stack;
    LZStack *fn_def_stack;
    Klass *klass;

    size_t size;
    Object *head_object;
    Object *tail_object;
} VM;

VM *vm_create();
void vm_destroy(VM *vm);

size_t vm_block_length(VM *vm);
void vm_print_stack(VM *vm);

//> function
void vm_fn_start(char *name, VM *vm);
void vm_fn_end(VM *vm);

void vm_fn_add_param(char *name, VM *vm);
//< function

//> container
void vm_klass_start(char *name, VM *vm);
void vm_klass_end(VM *vm);

void vm_klass_constructor_start(VM *vm);
void vm_klass_constructor_end(VM *vm);
void vm_klass_constructor_add_param(char *name, VM *vm);

void vm_klass_fn_start(char *name, VM *vm);
void vm_klass_fn_end(VM *vm);
void vm_klass_fn_add_param(char *name, VM *vm);
//< container

size_t vm_write_chunk(uint8_t chunk, VM *vm);
size_t vm_write_i32(int32_t value, VM *vm);
void vm_update_i32(size_t index, int32_t value, VM *vm);

size_t vm_write_bool_const(uint8_t value, VM *vm);
size_t vm_write_i64_const(int64_t value, VM *vm);
size_t vm_write_str_const(char *value, VM *vm);

int vm_execute(VM *vm);

#endif