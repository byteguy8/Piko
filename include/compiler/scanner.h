#ifndef _SCANNER_H_
#define _SCANNER_H_

#include "static_str.h"

#include <essentials/dynarr.h>
#include <essentials/lzhtable.h>

typedef struct _scanner_
{
    int line;
    int start;
    int current;
    DynArr *tokens;
    StaticStr *source;
    LZHTable *keywords;
} Scanner;

void scanner_print_tokens(DynArr *tokens);
int scanner_scan_tokens(Scanner *scanner);

#endif