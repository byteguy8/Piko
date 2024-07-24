#ifndef _COMPILER_H_
#define _COMPILER_H_

#include "stmt.h"

#include <essentials/dynarr.h>
#include <essentials/lzhtable.h>

#include <vm/vm.h>

typedef struct _symbol_
{
    int global;
    int local;
    int depth;
    void *load;
    char *identifier;
    int is_entity;   // a function or a class
    int class_bound; // declared as member (function or attribute) of a class
    struct _symbol_ *next;
} Symbol;

typedef enum _scope_type_
{
    GLOBAL_SCOPE,
    BLOCK_SCOPE,
    IF_SCOPE,
    ELIF_SCOPE,
    ELSE_SCOPE,
    FN_SCOPE,
    CONSTRUCTOR_SCOPE,
    WHILE_SCOPE,
    KLASS_SCOPE
} ScopeType;

typedef struct _symbol_stack_
{
    int local;
    enum _scope_type_ type;
    LZHTable *symbols;
} SymbolStack;

typedef struct _compiler_
{
    int depth;
    int inside_constructor;
    int entity_counter;
    struct _symbol_stack_ scope_stack[255];

    VM *vm;

    DynArrPtr *stmts;

    LZStack *continues;
    LZStack *breaks;

    DynArrPtr *natives;
} Compiler;

void compiler_compile(VM *vm, DynArrPtr *stmts);

#endif