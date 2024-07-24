#ifndef _STATIC_STR_H_
#define _STATIC_STR_H_

#include <stddef.h>

typedef struct _static_str_
{
    char *raw;
    size_t len;
} StaticStr;

#endif