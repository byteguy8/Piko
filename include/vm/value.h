#ifndef _VALUE_H_
#define _VALUE_H_

#include <stdint.h>

typedef enum _value_type_
{
    BOOL_VTYPE,
    INT_VTYPE
} ValueType;

typedef struct _value_
{
    enum _value_type_ type;

    union
    {
        int64_t i64;
    };
} Value;

#endif