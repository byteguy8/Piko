#ifndef _HOLDER_H_
#define _HOLDER_H_

#include "value.h"
#include "object.h"

#include <stdint.h>

typedef enum _type_
{
    NIL_HTYPE,
    VALUE_HTYPE,
    OBJECT_HTYPE
} Type;

typedef struct _holder_
{
    enum _type_ type;

    union
    {
        Value literal;
        Object *object;
    } entity;
} Holder;

#endif