#ifndef _VALUE_H_
#define _VALUE_H_

#include "primitive.h"
#include "object.h"

#include <stdint.h>

typedef enum _type_
{
    NIL_HTYPE,
    VALUE_HTYPE,
    OBJECT_HTYPE
} Type;

typedef struct _value_
{
    enum _type_ type;

    union
    {
        Primitive primitive;
        Object *object;
    } entity;
} Value;

#endif