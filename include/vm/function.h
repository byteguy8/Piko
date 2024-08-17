#ifndef _FUNCTION_H_
#define _FUNCTION_H_

#include <essentials/dynarr.h>
#include <essentials/lzhtable.h>

#define CLOSURE_LOCALS_LENGTH 255

typedef struct _fn_
{
    char *name;
    DynArrPtr *params;
    DynArr *chunks;
} Fn;

#endif