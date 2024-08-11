#ifndef _OBJECT_H_
#define _OBJECT_H_

#include "value.h"
#include "function.h"
#include "klass.h"

#include <essentials/dynarr.h>
#include <essentials/lzhtable.h>

typedef enum _object_type_
{
    VALUE_OTYPE,
    STR_OTYPE,
    OBJ_ARR_OTYPE,
    FN_OTYPE,
    NATIVE_FN_OTYPE,
    METHOD_OTYPE,
    CLASS_OTYPE,
    INSTANCE_OTYPE
} ObjectType;

typedef struct _string_
{
    char core;     // 0 = need to be freeded
    char *buffer;  // NULL terminated
    size_t length; // without NULL
} String;

typedef struct _array_
{
    size_t length;
    struct _object_ **items;
} Array;

typedef struct _native_fn_
{
    char arity;
    char *name;
    void *raw_fn;
} NativeFn;

typedef struct _method_
{
    Fn *fn;
    void *instance;
} Method;

typedef struct _instance_
{
    LZHTable *attributes;
    struct _klass_ *klass;
} Instance;

typedef struct _object_
{
    char marked;
    enum _object_type_ type;
    struct _object_ *next;

    union
    {
        Value literal;
        String string;
        Array array;
        Fn *fn;
        NativeFn *native_fn;
        Method method;
        Klass *class;
        Instance instance;
    } value;

} Object;

#endif