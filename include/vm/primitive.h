#ifndef _PRIMITIVE_H_
#define _PRIMITIVE_H_

#include <stdint.h>

typedef enum _value_type_
{
    BOOL_VTYPE,
    INT_VTYPE
} ValueType;

typedef struct _primitive_
{
    enum _value_type_ type;

    union
    {
        int64_t i64;
    };
} Primitive;

#endif