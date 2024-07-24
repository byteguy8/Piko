#ifndef _PARSER_H_
#define _PARSER_H_

#include <essentials/dynarr.h>

typedef struct _parser_
{
    int current;
    DynArr *tokens;
    DynArrPtr *stmts;
} Parser;

void parser_parse(Parser *parser);

#endif