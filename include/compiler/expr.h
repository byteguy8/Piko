#ifndef _EXPR_H_
#define _EXPR_H_

#include "token.h"

#include <essentials/dynarr.h>

#include <stddef.h>

typedef enum _expr_type_
{
    ASSIGN_EXPR_TYPE,
    IS_EXPR_TYPE,
    FROM_EXPR_TYPE,
    ARR_EXPR_TYPE,
    LOGICAL_EXPR_TYPE,
    COMPARISON_EXPR_TYPE,
    BINARY_EXPR_TYPE,
    UNARY_EXPR_TYPE,
    ARR_ACCESS_EXPR_TYPE,
    ACCESS_EXPR_TYPE,
    CALL_EXPR_TYPE,
    THIS_EXPR_TYPE,
    GROUP_EXPR_TYPE,
    NIL_EXPR_TYPE,
    BOOL_EXPR_TYPE,
    INT_EXPR_TYPE,
    STR_EXPR_TYPE,
    IDENTIFIER_EXPR_TYPE
} ExprType;

typedef struct _expr_
{
    void *e;
    enum _expr_type_ type;
} Expr;

typedef struct _assign_expr_
{
    Expr *left;
    Token *equals_token;
    Expr *right;
} AssignExpr;

typedef struct _is_expr_
{
    Expr *left;
    Token *is_token;
    Token *type_token;
} IsExpr;

typedef struct _from_expr_
{
    Expr *left;
    Token *from_token;
    Token *klass_identifier_token;
} FromExpr;

typedef struct _arr_expr_
{
    Expr *len_expr;
    DynArrPtr *items;
} ArrExpr;

typedef struct _logical_expr_
{
    Expr *left;
    Token *operator;
    Expr *right;
} LogicalExpr;

typedef struct _comparison_expr_
{
    Expr *left;
    Token *operator;
    Expr *right;
} ComparisonExpr;

typedef struct _binary_expr_
{
    Expr *left;
    Token *operator;
    Expr *right;
} BinaryExpr;

typedef struct _unary_expr_
{
    Token *operator;
    Expr *right;
} UnaryExpr;

typedef struct _arr_access_expr_
{
    Expr *expr;
    Expr *index_expr;
} ArrAccessExpr;

typedef struct _access_expr_
{
    Expr *left;
    Token *identifier;
} AccessExpr;

typedef struct _call_expr_
{
    Expr *left;
    DynArrPtr *args;
} CallExpr;

typedef struct _this_expr_
{
    Token *thisToken;
    Token *identifier_token;
} ThisExpr;

typedef struct _identifier_expr_
{
    Token *identifier_token;
} IdentifierExpr;

typedef struct _group_expr_
{
    Token *left_paren_token;
    Expr *e;
} GroupExpr;

typedef struct _literal_expr_
{
    void *literal;
    size_t literal_size;
} LiteralExpr;

#endif