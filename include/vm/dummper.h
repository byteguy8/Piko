#ifndef _DUMPPER_H_
#define _DUMPPER_H_

#include "vm.h"
#include "essentials/lzstack.h"

#define DUMPPER_SCOPE_LENGTH 255

typedef struct _dumpper_scope_
{
    size_t ip;
    DynArr *chunks;
} DumpperScope;

typedef struct _dumpper_
{
    VM *vm;
    int scope_ptr;
    struct _dumpper_scope_ scopes[DUMPPER_SCOPE_LENGTH];
} Dumpper;

void dumpper_execute(VM *vm);

#endif