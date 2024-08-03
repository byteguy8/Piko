#ifndef _FRAME_H_
#define _FRAME_H_

#include "holder.h"
#include "value.h"

#include <essentials/dynarr.h>

#define FRAME_VALUES_LENGTH 255

typedef struct _frame_
{
    int ip;
    DynArr *chunks;
    Object *instance;
    char is_constructor;
    Holder values[FRAME_VALUES_LENGTH];
} Frame;

#endif
