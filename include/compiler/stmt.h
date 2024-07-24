#ifndef _STMT_H_
#define _STMT_H_

#include "expr.h"

typedef enum _stmt_type_
{
    VAR_DECL_STMT_TYPE,
    BLOCK_STMT_TYPE,
    IF_STMT_TYPE,
    CONTINUE_STMT_TYPE,
    BREAK_STMT_TYPE,
    WHILE_STMT_TYPE,
    FN_STMT_TYPE,
    CLASS_STMT_TYPE,
    PRINT_STMT_TYPE,
    RETURN_STMT_TYPE,
    EXPR_STMT_TYPE,
} StmtType;

typedef struct stmt
{
    void *s;
    enum _stmt_type_ type;
} Stmt;

typedef struct _var_decl_stmt_
{
    Token *identifier;
    Expr *initializer;
} VarDeclStmt;

typedef struct _block_stmt_
{
    DynArrPtr *stmts;
} BlockStmt;

typedef struct _if_stmt_branch_
{
    Expr *condition;
    DynArrPtr *stmts;
} IfStmtBranch;

typedef struct _if_stmt_
{
    struct _if_stmt_branch_ *if_branch;
    DynArrPtr *elif_branches;
    DynArrPtr *else_stmts;
} IfStmt;

typedef struct _continue_stmt_
{
    Token *continue_token;
} ContinueStmt;

typedef struct _break_stmt_
{
    Token *break_token;
} BreakStmt;

typedef struct _while_stmt_
{
    Expr *condition;
    DynArrPtr *stmts;
} WhileStmt;

typedef struct _fn_stmt_
{
    Token *identifier_token;
    DynArrPtr *params;
    DynArrPtr *stmts;
} FnStmt;

typedef struct _klass_class_
{
    Token *identifier;
    DynArrPtr *attributes;
    FnStmt *constructor;
    DynArrPtr *methods;
} ClassStmt;

typedef struct _print_stmt_
{
    Expr *expr;
    Token *print_token;
} PrintStmt;

typedef struct _return_stmt_
{
    Expr *value;
    Token *return_token;
} ReturnStmt;

typedef struct _expr_stmt_
{
    Expr *expr;
} ExprStmt;

#endif