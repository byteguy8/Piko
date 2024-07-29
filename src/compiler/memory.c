#include "memory.h"
#include <essentials/lzallocator.h>

#include <stdio.h>
#include <assert.h>

static int initialized = 0;
static LZAllocator *allocator = NULL;
static DynArrAllocator dynarr_allocator = {0};
static LZHTableAllocator lzhtable_allocator = {0};
static LZStackAllocator lzstack_allocator = {0};

// private interface
static void *_alloc_(size_t size);
static void *_realloc_(void *ptr, size_t size);
static void _dealloc_(void *ptr);

// private implementation
void *_alloc_(size_t size)
{
    return memory_alloc(size);
}

void *_realloc_(void *ptr, size_t size)
{
    return memory_realloc(ptr, size);
}

void _dealloc_(void *ptr)
{
    memory_dealloc(ptr);
}

// public implementation
int memory_init()
{
    if (initialized)
        return 0;

    allocator = lzallocator_create(lzallocator_mib(8));

    if (!allocator)
        return 1;

    initialized = 1;

    dynarr_allocator.alloc = _alloc_;
    dynarr_allocator.realloc = _realloc_;
    dynarr_allocator.dealloc = _dealloc_;

    lzhtable_allocator.alloc = _alloc_;
    lzhtable_allocator.realloc = _realloc_;
    lzhtable_allocator.dealloc = _dealloc_;

    lzstack_allocator.alloc = _alloc_;
    lzstack_allocator.dealloc = _dealloc_;

    return 0;
}

void memory_deinit()
{
    if (!initialized)
        return;

    lzallocator_destroy(allocator);

    initialized = 0;
}

void *memory_calloc(size_t bytes)
{
    assert(initialized && "You must call memory_init");

    void *ptr = lzallocator_calloc(bytes, allocator, NULL);

    if (!ptr)
        memory_deinit();

    assert(ptr && "Memory out of space");

    return ptr;
}

void *memory_alloc(size_t bytes)
{
    assert(initialized && "You must call memory_init");

    void *ptr = lzallocator_alloc(bytes, allocator, NULL);

    if (!ptr)
        memory_deinit();

    assert(ptr && "Memory out of space");

    return ptr;
}

void *memory_realloc(void *ptr, size_t bytes)
{
    assert(initialized && "You must call memory_init");

    void *new_ptr = lzallocator_realloc(bytes, ptr, allocator, NULL);

    if (!new_ptr)
        memory_deinit();

    assert(new_ptr && "Memory out of space");

    return new_ptr;
}

void memory_dealloc(void *ptr)
{
    assert(initialized && "You must call memory_init");

    if (!ptr)
        return;

    lzallocator_dealloc(ptr, allocator);
}

void memory_report()
{
    size_t total = allocator->bytes;
    size_t available = lzallocator_available_space(allocator);
    size_t used = total - available;

    printf("%ld/%ld\n", used, total);
}

char *memory_clone_raw_str(char *str)
{
    size_t length = strlen(str);
    char *new_str = (char *)memory_alloc(length + 1);

    memcpy(new_str, str, length);

    new_str[length] = 0;

    return new_str;
}

void memory_destroy_raw_str(char *str)
{
    if (!str)
        return;

    memory_dealloc(str);
}

StaticStr *memory_create_static_str(char *str, size_t len)
{
    char *raw = (char *)memory_alloc(len + 1);
    StaticStr *static_str = memory_alloc(sizeof(StaticStr));

    memcpy(raw, str, len);
    raw[len] = 0;

    static_str->raw = raw;
    static_str->len = len;

    return static_str;
}

void memory_destroy_static_str(StaticStr *str)
{
    if (!str)
        return;

    memory_dealloc(str->raw);

    str->len = 0;
    str->raw = NULL;

    memory_dealloc(str);
}

StaticStr *memory_read_source(char *path)
{
    FILE *source_stream = fopen(path, "r");
    fseek(source_stream, 0, SEEK_END);

    size_t len = ftell(source_stream);
    char *source = (char *)memory_alloc(len + 1);

    rewind(source_stream);
    fread(source, 1, len, source_stream);

    source[len] = '\0';

    fclose(source_stream);

    StaticStr *str = (StaticStr *)memory_alloc(sizeof(StaticStr));

    str->len = len;
    str->raw = source;

    return str;
}

DynArr *memory_create_dynarr(size_t bytes)
{
    return dynarr_create(bytes, &dynarr_allocator);
}

void memory_destroy_dynarr(DynArr *arr)
{
    if (!arr)
        return;

    dynarr_destroy(arr);
}

DynArrPtr *memory_create_dynarr_ptr()
{
    return dynarr_ptr_create(&dynarr_allocator);
}

void memory_destroy_dynarr_ptr(DynArrPtr *arr)
{
    dynarr_ptr_destroy(arr);
}

LZStack *memory_create_lzstack()
{
    return lzstack_create(&lzstack_allocator);
}

void memory_destroy_lzstack(LZStack *stack)
{
    lzstack_destroy(stack);
}

LZHTable *memory_create_lzhtable(size_t size)
{
    return lzhtable_create(size, &lzhtable_allocator);
}

void memory_destroy_lzhtable(LZHTable *table)
{
    lzhtable_destroy(table);
}

Token *memory_create_token(int line, char *lexeme, void *literal, size_t literal_size, TokenType type)
{
    Token *token = (Token *)memory_alloc(sizeof(Token));

    token->line = line;
    token->lexeme = lexeme;
    token->literal = literal;
    token->literal_size = literal_size;
    token->type = type;

    return token;
}

void memory_destroy_token(Token *token)
{
    if (!token)
        return;

    token->line = 0;
    token->lexeme = NULL;
    token->literal = NULL;
    token->literal_size = 0;
    token->type = 0;

    memory_dealloc(token);
}

Token *memory_clone_token(Token *token)
{
    void *literal = token->literal ? memory_alloc(sizeof(token->literal_size)) : NULL;
    Token *new_token = (Token *)memory_alloc(sizeof(Token));

    if (literal)
        memcpy(literal, token->literal, token->literal_size);

    new_token->line = token->line;
    new_token->lexeme = memory_clone_raw_str(token->lexeme);
    new_token->literal = literal;
    new_token->literal_size = token->literal_size;
    new_token->type = token->type;

    return new_token;
}

AssignExpr *memory_create_assign_expr(Expr *left, Token *equals_token, Expr *right)
{
    AssignExpr *expr = (AssignExpr *)memory_alloc(sizeof(AssignExpr));

    expr->left = left;
    expr->equals_token = equals_token;
    expr->right = right;

    return expr;
}

void memory_destroy_assign_expr(AssignExpr *expr)
{
    if (!expr)
        return;

    expr->left = NULL;
    expr->equals_token = NULL;
    expr->right = NULL;

    memory_dealloc(expr);
}

IsExpr *memory_create_is_expr(Expr *left, Token *is_token, Token *type_token)
{
    IsExpr *expr = (IsExpr *)memory_alloc(sizeof(IsExpr));

    expr->left = left;
    expr->is_token = is_token;
    expr->type_token = type_token;

    return expr;
}

void memory_destroy_is_expr(IsExpr *expr)
{
    if (!expr)
        return;

    expr->left = NULL;
    expr->is_token = NULL;
    expr->type_token = NULL;

    memory_dealloc(expr);
}

FromExpr *memory_create_from_expr(Expr *left, Token *from_token, Token *klass_identifier_token)
{
    FromExpr *expr = (FromExpr *)memory_alloc(sizeof(FromExpr *));

    expr->left = left;
    expr->from_token = from_token;
    expr->klass_identifier_token = klass_identifier_token;

    return expr;
}

void memory_destroy_from_expr(FromExpr *expr)
{
    if (!expr)
        return;

    expr->left = NULL;
    expr->from_token = NULL;
    expr->klass_identifier_token = NULL;

    memory_dealloc(expr);
}

ArrExpr *memory_create_arr_expr(Token *left_square_token, DynArrPtr *items, Expr *len_expr)
{
    ArrExpr *expr = (ArrExpr *)memory_alloc(sizeof(ArrExpr));

    expr->left_square_token = left_square_token;
    expr->items = items;
    expr->len_expr = len_expr;

    return expr;
}

void memory_destroy_arr_expr(ArrExpr *expr)
{
    if (!expr)
        return;

    memset(expr, 0, sizeof(ArrExpr));

    memory_dealloc(expr);
}

LogicalExpr *memory_create_logical_expr(Expr *left, Token *operator, Expr * right)
{
    LogicalExpr *expr = (LogicalExpr *)memory_alloc(sizeof(LogicalExpr));

    expr->left = left;
    expr->operator_token = operator;
    expr->right = right;

    return expr;
}

void memory_destroy_logical_expr(LogicalExpr *expr)
{
    if (!expr)
        return;

    expr->left = NULL;
    expr->operator_token = NULL;
    expr->right = NULL;

    memory_dealloc(expr);
}

ComparisonExpr *memory_create_comparison_expr(Expr *left, Token *operator, Expr * right)
{
    ComparisonExpr *expr = (ComparisonExpr *)memory_alloc(sizeof(ComparisonExpr));

    expr->left = left;
    expr->operator_token = operator;
    expr->right = right;

    return expr;
}

void memory_destroy_comparison_expr(ComparisonExpr *expr)
{
    if (!expr)
        return;

    expr->left = NULL;
    expr->operator_token = NULL;
    expr->right = NULL;

    memory_dealloc(expr);
}

BinaryExpr *memory_create_binary_expr(Expr *left, Token *operator, Expr * right)
{
    BinaryExpr *expr = (BinaryExpr *)memory_alloc(sizeof(BinaryExpr));

    expr->left = left;
    expr->operator_token = operator;
    expr->right = right;

    return expr;
}

void memory_destroy_binary_expr(BinaryExpr *expr)
{
    if (!expr)
        return;

    expr->left = NULL;
    expr->operator_token = NULL;
    expr->right = NULL;

    memory_dealloc(expr);
}

UnaryExpr *memory_create_unary_expr(Token *operator, Expr * right)
{
    UnaryExpr *expr = (UnaryExpr *)memory_alloc(sizeof(UnaryExpr));

    expr->operator_token = operator;
    expr->right = right;

    return expr;
}

void memory_destroy_unary_expr(UnaryExpr *expr)
{
    if (!expr)
        return;

    expr->operator_token = NULL;
    expr->right = NULL;

    memory_dealloc(expr);
}

ArrAccessExpr *memory_create_arr_access_expr(Expr *expr, Token *left_square_token, Expr *index_expr)
{
    ArrAccessExpr *e = (ArrAccessExpr *)memory_alloc(sizeof(ArrAccessExpr));

    e->expr = expr;
    e->left_square_token = left_square_token;
    e->index_expr = index_expr;

    return e;
}

void memory_destroy_arr_access_expr(ArrAccessExpr *expr)
{
    if (!expr)
        return;

    memset(expr, 0, sizeof(ArrAccessExpr));

    memory_dealloc(expr);
}

AccessExpr *memory_create_access_expr(Expr *left, Token *dot_token, Token *identifier)
{
    AccessExpr *expr = (AccessExpr *)memory_alloc(sizeof(AccessExpr));

    expr->left = left;
    expr->dot_token = dot_token;
    expr->identifier_token = identifier;

    return expr;
}

void memory_destroy_access_expr(AccessExpr *expr)
{
    if (!expr)
        return;

    memset(expr, 0, sizeof(AccessExpr));

    memory_dealloc(expr);
}

CallExpr *memory_create_call_expr(Expr *left, Token *left_parenthesis_token, DynArrPtr *args)
{
    CallExpr *expr = (CallExpr *)memory_alloc(sizeof(CallExpr));

    expr->left = left;
    expr->left_parenthesis_token = left_parenthesis_token;
    expr->args = args;

    return expr;
}

void memory_destroy_call_expr(CallExpr *expr)
{
    if (!expr)
        return;

    memset(expr, 0, sizeof(CallExpr));

    memory_dealloc(expr);
}

ThisExpr *memory_create_this_expr(Token *thisToken, Token *IdentifierExpr)
{
    ThisExpr *expr = (ThisExpr *)memory_alloc(sizeof(ThisExpr));

    expr->this_token = thisToken;
    expr->identifier_token = IdentifierExpr;

    return expr;
}

void memory_destroy_this_expr(ThisExpr *expr)
{
    if (!expr)
        return;

    expr->this_token = NULL;
    expr->identifier_token = NULL;

    memory_dealloc(expr);
}

IdentifierExpr *memory_create_identifier_expr(Token *identifier_token)
{
    IdentifierExpr *expr = (IdentifierExpr *)memory_alloc(sizeof(IdentifierExpr));

    expr->identifier_token = identifier_token;

    return expr;
}

void memory_destroy_identifier_expr(IdentifierExpr *expr)
{
    if (!expr)
        return;

    expr->identifier_token = NULL;

    memory_dealloc(expr);
}

GroupExpr *memory_create_group_expr(Token *left_paren_token, Expr *e)
{
    GroupExpr *expr = (GroupExpr *)memory_alloc(sizeof(GroupExpr));

    expr->left_paren_token = left_paren_token;
    expr->e = e;

    return expr;
}

void memory_destroy_group_expr(GroupExpr *expr)
{
    if (!expr)
        return;

    expr->left_paren_token = NULL;
    expr->e = NULL;

    memory_dealloc(expr);
}

LiteralExpr *memory_create_literal_expr(void *literal, size_t literal_size, Token *literal_token)
{
    LiteralExpr *expr = (LiteralExpr *)memory_alloc(sizeof(LiteralExpr));

    expr->literal = literal;
    expr->literal_size = literal_size;
    expr->literal_token = literal_token;

    return expr;
}

void memory_destroy_literal_expr(LiteralExpr *expr)
{
    if (!expr)
        return;

    memset(expr, 0, sizeof(LiteralExpr));

    memory_dealloc(expr);
}

Expr *memory_create_expr(void *e, ExprType type)
{
    Expr *expr = (Expr *)memory_alloc(sizeof(Expr));

    expr->e = e;
    expr->type = type;

    return expr;
}

void memory_destroy_expr(Expr *expr)
{
    if (!expr)
        return;

    expr->e = NULL;
    expr->type = 0;

    memory_dealloc(expr);
}

Stmt *memory_create_stmt(void *s, StmtType type)
{
    Stmt *stmt = (Stmt *)memory_alloc(sizeof(Stmt));

    stmt->s = s;
    stmt->type = type;

    return stmt;
}

void memory_destroy_stmt(Stmt *stmt)
{
    if (!stmt)
        return;

    memset(stmt, 0, sizeof(Stmt));

    memory_dealloc(stmt);
}

VarDeclStmt *memory_create_var_decl_stmt(Token *identifier, Expr *initializer)
{
    VarDeclStmt *stmt = (VarDeclStmt *)memory_alloc(sizeof(VarDeclStmt));

    stmt->identifier = identifier;
    stmt->initializer = initializer;

    return stmt;
}

void memory_destroy_var_decl_stmt(VarDeclStmt *stmt)
{
    if (!stmt)
        return;

    memset(stmt, 0, sizeof(VarDeclStmt));

    memory_dealloc(stmt);
}

BlockStmt *memory_create_block_stmt(DynArrPtr *stmts)
{
    BlockStmt *block = (BlockStmt *)memory_alloc(sizeof(BlockStmt));

    block->stmts = stmts;

    return block;
}
void memory_destroy_block_stmt(BlockStmt *stmt)
{
    if (!stmt)
        return;

    memset(stmt, 0, sizeof(BlockStmt));

    memory_dealloc(stmt);
}

IfStmtBranch *memory_create_if_stmt_branch(Expr *condition, DynArrPtr *stmts)
{
    IfStmtBranch *stmt = (IfStmtBranch *)memory_alloc(sizeof(IfStmtBranch));

    stmt->condition = condition;
    stmt->stmts = stmts;

    return stmt;
}

void memory_destroy_if_stmts_branch(IfStmtBranch *stmt)
{
    if (!stmt)
        return;

    memset(stmt, 0, sizeof(IfStmtBranch));

    memory_dealloc(stmt);
}

IfStmt *memory_create_if_stmt(IfStmtBranch *if_branch, DynArrPtr *elif_branches, DynArrPtr *else_stmts)
{
    IfStmt *stmt = (IfStmt *)memory_alloc(sizeof(IfStmt));

    stmt->if_branch = if_branch;
    stmt->elif_branches = elif_branches;
    stmt->else_stmts = else_stmts;

    return stmt;
}

void memory_destroy_if_stmt(IfStmt *stmt)
{
    if (!stmt)
        return;

    memset(stmt, 0, sizeof(IfStmt));

    memory_dealloc(stmt);
}

ContinueStmt *memory_create_continue_stmt(Token *continue_token)
{
    ContinueStmt *stmt = (ContinueStmt *)memory_alloc(sizeof(ContinueStmt));

    stmt->continue_token = continue_token;

    return stmt;
}

void memory_destroy_continue_stmt(ContinueStmt *stmt)
{
    if (!stmt)
        return;

    memset(stmt, 0, sizeof(ContinueStmt));

    memory_dealloc(stmt);
}

BreakStmt *memory_create_break_stmt(Token *break_token)
{
    BreakStmt *stmt = (BreakStmt *)memory_alloc(sizeof(BreakStmt));

    stmt->break_token = break_token;

    return stmt;
}

void memory_destroy_break_stmt(BreakStmt *stmt)
{
    if (!stmt)
        return;

    memset(stmt, 0, sizeof(BreakStmt));

    memory_dealloc(stmt);
}

WhileStmt *memory_create_while_stmt(Expr *condition, DynArrPtr *stmts)
{
    WhileStmt *stmt = (WhileStmt *)memory_alloc(sizeof(WhileStmt));

    stmt->condition = condition;
    stmt->stmts = stmts;

    return stmt;
}

void memory_destroy_whiles_stmt(WhileStmt *stmt)
{
    if (!stmt)
        return;

    memset(stmt, 0, sizeof(WhileStmt));

    memory_dealloc(stmt);
}

FnStmt *memory_create_fn_stmt(Token *identifier, DynArrPtr *params, DynArrPtr *stmts)
{
    FnStmt *stmt = (FnStmt *)memory_alloc(sizeof(FnStmt));

    stmt->identifier_token = identifier;
    stmt->params = params;
    stmt->stmts = stmts;

    return stmt;
}

void memory_destroy_fn_stmt(FnStmt *stmt)
{
    if (!stmt)
        return;

    memset(stmt, 0, sizeof(FnStmt));

    memory_dealloc(stmt);
}

ClassStmt *memory_create_class_stmt(Token *identifier, DynArrPtr *attributes, FnStmt *constructor, DynArrPtr *methods)
{
    ClassStmt *stmt = (ClassStmt *)memory_alloc(sizeof(ClassStmt));

    stmt->identifier = identifier;
    stmt->attributes = attributes;
    stmt->constructor = constructor;
    stmt->methods = methods;

    return stmt;
}

void memory_destroy_class(ClassStmt *stmt)
{
    if (!stmt)
        return;

    memset(stmt, 0, sizeof(ClassStmt));

    memory_dealloc(stmt);
}

PrintStmt *memory_create_print_stmt(Expr *expr, Token *print_token)
{
    PrintStmt *stmt = (PrintStmt *)memory_alloc(sizeof(PrintStmt));

    stmt->expr = expr;
    stmt->print_token = print_token;

    return stmt;
}

void memory_destroy_print_stmt(PrintStmt *stmt)
{
    if (!stmt)
        return;

    memset(stmt, 0, sizeof(PrintStmt));

    memory_dealloc(stmt);
}

ReturnStmt *memory_create_return_stmt(Expr *value, Token *return_token)
{
    ReturnStmt *stmt = (ReturnStmt *)memory_alloc(sizeof(ReturnStmt));

    stmt->value = value;
    stmt->return_token = return_token;

    return stmt;
}

void memory_destroy_return_stmt(ReturnStmt *stmt)
{
    if (!stmt)
        return;

    memset(stmt, 0, sizeof(ReturnStmt));

    memory_dealloc(stmt);
}

ExprStmt *memory_create_expr_stmt(Expr *expr)
{
    ExprStmt *stmt = (ExprStmt *)memory_alloc(sizeof(ExprStmt));

    stmt->expr = expr;

    return stmt;
}

void memory_destroy_expr_stmt(ExprStmt *stmt)
{
    if (!stmt)
        return;

    memset(stmt, 0, sizeof(ExprStmt));

    memory_dealloc(stmt);
}

Scanner *memory_create_scanner(DynArr *tokens, StaticStr *source, LZHTable *keywords)
{
    Scanner *scanner = (Scanner *)memory_alloc(sizeof(Scanner));

    scanner->line = 1;
    scanner->start = 0;
    scanner->current = 0;
    scanner->tokens = tokens;
    scanner->source = source;
    scanner->keywords = keywords;

    return scanner;
}

void memory_destroy_scanner(Scanner *scanner)
{
    if (!scanner)
        return;

    memset(scanner, 0, sizeof(Scanner));

    memory_dealloc(scanner);
}

Parser *memory_create_parser(DynArr *tokens, DynArrPtr *stmts)
{
    Parser *parser = (Parser *)memory_alloc(sizeof(Parser));

    parser->current = 0;
    parser->tokens = tokens;
    parser->stmts = stmts;

    return parser;
}

void memory_destroy_parser(Parser *parser)
{
    if (!parser)
        return;

    memset(parser, 0, sizeof(Parser));

    memory_dealloc(parser);
}

Symbol *memory_create_symbol(int global, int local, int depth, char *identifier, int is_entity, int class_bound)
{
    Symbol *symbol = (Symbol *)memory_alloc(sizeof(Symbol));

    symbol->global = global;
    symbol->local = local;
    symbol->depth = depth;
    symbol->identifier = identifier;
    symbol->is_entity = is_entity;
    symbol->class_bound = class_bound;
    symbol->next = NULL;

    return symbol;
}

void memory_destroy_symbol(Symbol *symbol)
{
    if (!symbol)
        return;

    memset(symbol, 0, sizeof(Symbol));

    memory_dealloc(symbol);
}

Compiler *memory_create_compiler()
{
    Compiler *compiler = (Compiler *)memory_calloc(sizeof(Compiler));
    return compiler;
}

void memory_destroy_compiler(Compiler *compiler)
{
    if (!compiler)
        return;

    memset(compiler, 0, sizeof(Compiler));

    memory_dealloc(compiler);
}