#ifndef _CONTAINER_H_
#define _CONTAINER_H_

#include <essentials/dynarr.h>
#include <essentials/lzhtable.h>

typedef struct _klass_
{
    char *name;
    Fn *constructor;
    LZHTable *methods;
} Klass;

#endif