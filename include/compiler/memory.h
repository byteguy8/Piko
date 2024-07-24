#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "static_str.h"

#include "token.h"
#include "expr.h"
#include "stmt.h"

#include "scanner.h"
#include "parser.h"
#include "compiler.h"

#include <essentials/dynarr.h>
#include <essentials/lzhtable.h>

#include <stddef.h>

int memory_init();
void memory_deinit();

void *memory_calloc(size_t bytes);
void *memory_alloc(size_t bytes);
void *memory_realloc(void *ptr, size_t bytes);
void memory_dealloc(void *ptr);

void memory_report();

char *memory_clone_raw_str(char *str);
void memory_destroy_raw_str(char *str);

StaticStr *memory_create_static_str(char *str, size_t len);
void memory_destroy_static_str(StaticStr *str);

StaticStr *memory_read_source(char *path);

DynArr *memory_create_dynarr(size_t bytes);
void memory_destroy_dynarr(DynArr *arr);

DynArrPtr *memory_create_dynarr_ptr();
void memory_destroy_dynarr_ptr(DynArrPtr *arr);

LZStack *memory_create_lzstack();
void memory_destroy_lzstack(LZStack *stack);

LZHTable *memory_create_lzhtable(size_t size);
void memory_destroy_lzhtable(LZHTable *table);

Token *memory_create_token(int line, char *lexeme, void *literal, size_t literal_size, TokenType type);
void memory_destroy_token(Token *token);

Token *memory_clone_token(Token *token);

// ---> expressions
AssignExpr *memory_create_assign_expr(Expr *left, Token *equals_token, Expr *right);
void memory_destroy_assign_expr(AssignExpr *expr);

IsExpr *memory_create_is_expr(Expr *left, Token *is_token, Token *type_token);
void memory_destroy_is_expr(IsExpr * expr);

FromExpr *memory_create_from_expr(Expr *left, Token *from_token, Token *klass_identifier_token);
void memory_destroy_from_expr(FromExpr *expr);

ArrExpr *memory_create_arr_expr(Expr *len_expr, DynArrPtr *items);
void memory_destroy_arr_expr(ArrExpr *expr);

LogicalExpr *memory_create_logical_expr(Expr *left, Token *operator, Expr * right);
void memory_destroy_logical_expr(LogicalExpr *expr);

ComparisonExpr *memory_create_comparison_expr(Expr *left, Token *operator, Expr * right);
void memory_destroy_comparison_expr(ComparisonExpr *expr);

BinaryExpr *memory_create_binary_expr(Expr *left, Token *operator, Expr * right);
void memory_destroy_binary_expr(BinaryExpr *expr);

UnaryExpr *memory_create_unary_expr(Token *operator, Expr * right);
void memory_destroy_unary_expr(UnaryExpr *expr);

ArrAccessExpr *memory_create_arr_access_expr(Expr *expr, Expr *index_expr);
void memory_destroy_arr_access_expr(ArrAccessExpr *expr);

AccessExpr *memory_create_access_expr(Expr *left, Token *identifier);
void memory_destroy_access_expr(AccessExpr *expr);

CallExpr *memory_create_call_expr(Expr *left, DynArrPtr *args);
void memory_destroy_call_expr(CallExpr *expr);

ThisExpr *memory_create_this_expr(Token *thisToken, Token *IdentifierExpr);
void memory_destroy_this_expr(ThisExpr *expr);

IdentifierExpr *memory_create_identifier_expr(Token *identifier_token);
void memory_destroy_identifier_expr(IdentifierExpr *expr);

GroupExpr *memory_create_group_expr(Token *left_paren_token, Expr *e);
void memory_destroy_group_expr(GroupExpr *expr);

LiteralExpr *memory_create_literal_expr(void *literal, size_t literal_size);
void memory_destroy_literal_expr(LiteralExpr *expr);

Expr *memory_create_expr(void *e, ExprType type);
void memory_destroy_expr(Expr *expr);
// <--- expressions

// ---> statements
Stmt *memory_create_stmt(void *s, StmtType type);
void memory_destroy_stmt(Stmt *stmt);

VarDeclStmt *memory_create_var_decl_stmt(Token *identifier, Expr *initializer);
void memory_destroy_var_decl_stmt(VarDeclStmt *stmt);

BlockStmt *memory_create_block_stmt(DynArrPtr *stmts);
void memory_destroy_block_stmt(BlockStmt *stmt);

IfStmtBranch *memory_create_if_stmt_branch(Expr *condition, DynArrPtr *stmts);
void memory_destroy_if_stmts_branch(IfStmtBranch *stmt);

IfStmt *memory_create_if_stmt(IfStmtBranch *if_branch, DynArrPtr *elif_branches, DynArrPtr *elst_stmts);
void memory_destroy_if_stmt(IfStmt *stmt);

ContinueStmt *memory_create_continue_stmt(Token *continue_token);
void memory_destroy_continue_stmt(ContinueStmt *stmt);

BreakStmt *memory_create_break_stmt(Token *break_stmt);
void memory_destroy_break_stmt(BreakStmt *stmt);

WhileStmt *memory_create_while_stmt(Expr *condition, DynArrPtr *stmts);
void memory_destroy_whiles_stmt(WhileStmt *stmt);

FnStmt *memory_create_fn_stmt(Token *identifier, DynArrPtr *params, DynArrPtr *stmts);
void memory_destroy_fn_stmt(FnStmt *stmt);

ClassStmt *memory_create_class_stmt(Token *identifier, DynArrPtr *attributes, FnStmt *constructor, DynArrPtr *methods);
void memory_destroy_class(ClassStmt *stmt);

PrintStmt *memory_create_print_stmt(Expr *expr, Token *print_token);
void memory_destroy_print_stmt(PrintStmt *stmt);

ReturnStmt *memory_create_return_stmt(Expr *value, Token *return_token);
void memory_destroy_return_stmt(ReturnStmt *stmt);

ExprStmt *memory_create_expr_stmt(Expr *expr);
void memory_destroy_expr_stmt(ExprStmt *stmt);
// <--- statements

Scanner *memory_create_scanner(DynArr *tokens, StaticStr *source, LZHTable *keywords);
void memory_destroy_scanner(Scanner *scanner);

Parser *memory_create_parser(DynArr *tokens, DynArrPtr *stmts);
void memory_destroy_parser(Parser *parser);

Symbol *memory_create_symbol(int global, int index, int depth, char *identifier, int is_fn, int class_bound);
void memory_destroy_symbol(Symbol *symbol);

Compiler *memory_create_compiler();
void memory_destroy_compiler(Compiler *compiler);

#endif