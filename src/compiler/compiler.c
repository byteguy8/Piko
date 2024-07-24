#include "compiler.h"
#include "error_report.h"

#include <vm/opcode.h>
#include "memory.h"

#include <assert.h>
#include <stdint.h>

static Compiler *compiler = NULL;

#define COMPILER_VM compiler->vm

void _clear_lzhtable_(void *value)
{
    memory_destroy_symbol((Symbol *)value);
}

// private interface
void compiler_error_at(Token *token, char *msg, ...);

Symbol *compiler_declare_depth(int is_entity, int depth, Token *identifier_token);
Symbol *compiler_declare(int is_entity, Token *identifier_token);
Symbol *compiler_symbol_at(int depth, char *identifier);
Symbol *compiler_exists(Token *identifier_token);
Symbol *compiler_get(Token *identifier_token);

SymbolStack *compiler_scope_current();

int compiler_scope_inside_loop();
int compiler_scope_inside_fn();
int compiler_scope_inside_klass();

void compiler_scope_in(ScopeType type);
void compiler_scope_out();

void compiler_assign_expr(AssignExpr *expr);
void compiler_is_expr(IsExpr *expr);
void compiler_from_expr(FromExpr *expr);
void compiler_arr_expr(ArrExpr *expr);
void compiler_logical_expr(LogicalExpr *expr);
void compiler_comparison_expr(ComparisonExpr *expr);
void compiler_binary_expr(BinaryExpr *expr);
void compiler_unary_expr(UnaryExpr *expr);
void compiler_arr_access(ArrAccessExpr *expr);
void compiler_access(AccessExpr *expr);
void compiler_call_expr(CallExpr *expr);
void compiler_this_expr(ThisExpr *expr);
void compiler_nil_expr();
void compiler_bool_expr(LiteralExpr *expr);
void compiler_int_expr(LiteralExpr *expr);
void compiler_str_expr(LiteralExpr *expr);
void compiler_identifier_expr(IdentifierExpr *expr);
void compiler_group_expr(GroupExpr *expr);
void compiler_expr(Expr *expr);

void compiler_var_decl_stmt(VarDeclStmt *stmt);
void compiler_block_stmt(BlockStmt *stmt);
void compiler_if_stmt(IfStmt *stmt);
void compiler_continue_stmt(ContinueStmt *stmt);
void compiler_break_stmt(BreakStmt *stmt);
void compiler_while_stmt(WhileStmt *stmt);
void compiler_fn_stmt(FnStmt *stmt);
void compiler_klass_stmt(ClassStmt *stmt);
void compiler_print_stmt(PrintStmt *stmt);
void compiler_return_stmt(ReturnStmt *stmt);
void compiler_expr_stmt(ExprStmt *stmt);
void compiler_stmt(Stmt *stmt);

// private implementation
void compiler_error_at(Token *token, char *msg, ...)
{
    va_list args;
    va_start(args, msg);

    report_error_at(23, token->line, "Compiler", msg, args);

    va_end(args);
}

Symbol *compiler_declare_depth(int is_entity, int depth, Token *identifier_token)
{
    char *identifier = identifier_token->lexeme;

    SymbolStack *scope = &compiler->scope_stack[depth];
    LZHTable *symbols = scope->symbols;

    size_t key_size = strlen(identifier);

    if (lzhtable_contains((uint8_t *)identifier, key_size, symbols, NULL))
        compiler_error_at(identifier_token, "Already exists a symbol named as '%s'", identifier);

    int is_global = depth == 0;
    int local = is_entity ? ((int)compiler->natives->used + compiler->entity_counter++) : scope->local++;

    Symbol *symbol = memory_create_symbol(
        is_global,
        local,
        depth,
        identifier,
        is_entity,
        0);

    lzhtable_put((uint8_t *)identifier, key_size, symbol, symbols, NULL);

    return symbol;
}

Symbol *compiler_declare(int is_entity, Token *identifier_token)
{
    return compiler_declare_depth(is_entity, compiler->depth, identifier_token);
}

Symbol *compiler_symbol_at(int depth, char *identifier)
{
    SymbolStack *scope = &compiler->scope_stack[depth];
    return lzhtable_get((uint8_t *)identifier, strlen(identifier), scope->symbols);
}

Symbol *compiler_exists(Token *identifier_token)
{
    for (int i = compiler->depth; i >= 0; i--)
    {
        Symbol *symbol = compiler_symbol_at(i, identifier_token->lexeme);

        if (symbol)
            return symbol;
    }

    return NULL;
}

Symbol *compiler_get(Token *identifier_token)
{
    Symbol *symbol = compiler_exists(identifier_token);

    if (!symbol)
        compiler_error_at(identifier_token, "Do not exists a symbol named '%s'", identifier_token->lexeme);

    return symbol;
}

SymbolStack *compiler_scope_current()
{
    return &compiler->scope_stack[compiler->depth];
}

int compiler_scope_inside_loop()
{
    for (int i = compiler->depth; i >= 0; i--)
    {
        SymbolStack *scope = &compiler->scope_stack[i];

        if (scope->type == FN_SCOPE)
            return 0;

        if (scope->type == KLASS_SCOPE)
            return 0;

        if (scope->type == WHILE_SCOPE)
            return 1;
    }

    return 0;
}

int compiler_scope_inside_fn()
{
    for (int i = compiler->depth; i >= 0; i--)
    {
        SymbolStack *scope = &compiler->scope_stack[i];

        if (scope->type == FN_SCOPE)
            return 1;
    }

    return 0;
}

int compiler_scope_inside_klass()
{
    for (int i = compiler->depth; i >= 0; i--)
    {
        SymbolStack *scope = &compiler->scope_stack[i];

        if (scope->type == KLASS_SCOPE)
            return i;
    }

    return -1;
}

void compiler_scope_in(ScopeType type)
{
    SymbolStack *prev_scope = &compiler->scope_stack[compiler->depth];
    SymbolStack *scope = &compiler->scope_stack[++compiler->depth];

    if (type == FN_SCOPE || type == CONSTRUCTOR_SCOPE || type == KLASS_SCOPE)
        scope->local = 0;
    else
        scope->local = prev_scope->local;

    scope->type = type;

    if (!scope->symbols)
        scope->symbols = memory_create_lzhtable(17);
}

void compiler_scope_out()
{
    SymbolStack *scope = &compiler->scope_stack[compiler->depth--];

    lzhtable_clear(_clear_lzhtable_, scope->symbols);
}

void compiler_assign_expr(AssignExpr *expr)
{
    Expr *left = expr->left;
    Expr *right = expr->right;

    if (left->type == ACCESS_EXPR_TYPE)
    {
        AccessExpr *access_expr = (AccessExpr *)left->e;
        Token *identifier_token = access_expr->identifier;
        char *identifier = identifier_token->lexeme;

        compiler_expr(right);

        vm_write_chunk(THIS_OPC, COMPILER_VM);
        vm_write_chunk(SET_PROPERTY_OPC, COMPILER_VM);
        vm_write_str_const(identifier, COMPILER_VM);

        return;
    }

    if (left->type == THIS_EXPR_TYPE)
    {
        ThisExpr *this_expr = (ThisExpr *)left->e;
        Token *identifier_token = this_expr->identifier_token;
        int klass_scope = compiler_scope_inside_klass();

        if (identifier_token && klass_scope != -1)
        {
            char *identifier = identifier_token->lexeme;
            Symbol *symbol = compiler_symbol_at(klass_scope, identifier_token->lexeme);

            if (!symbol)
                compiler_declare_depth(0, klass_scope, identifier_token)->class_bound = 1;

            compiler_expr(right);

            vm_write_chunk(THIS_OPC, COMPILER_VM);
            vm_write_chunk(SET_PROPERTY_OPC, COMPILER_VM);
            vm_write_str_const(identifier, COMPILER_VM);

            return;
        }
    }

    if (left->type == ARR_ACCESS_EXPR_TYPE)
    {
        ArrAccessExpr *access_expr = (ArrAccessExpr *)left->e;

        compiler_expr(right);
        compiler_expr(access_expr->expr);
        compiler_expr(access_expr->index_expr);

        vm_write_chunk(ARR_SITM_OPC, COMPILER_VM);

        return;
    }

    if (left->type == IDENTIFIER_EXPR_TYPE)
    {
        compiler_expr(right);

        IdentifierExpr *identifier_expr = (IdentifierExpr *)left->e;
        Token *identifier_token = identifier_expr->identifier_token;
        char *identifier = identifier_token->lexeme;

        Symbol *symbol = compiler_get(identifier_token);

        if (symbol->class_bound)
        {
            vm_write_chunk(THIS_OPC, COMPILER_VM);
            vm_write_chunk(SET_PROPERTY_OPC, COMPILER_VM);
            vm_write_str_const(identifier, COMPILER_VM);

            return;
        }

        if (symbol->global)
        {
            vm_write_chunk(GWRITE_OPC, COMPILER_VM);
            vm_write_str_const(identifier, COMPILER_VM);
        }
        else
        {
            vm_write_chunk(LSET_OPC, COMPILER_VM);
            vm_write_chunk((uint8_t)symbol->local, COMPILER_VM);
        }

        return;
    }

    compiler_error_at(expr->equals_token, "Illegal assignment target.");
}

void compiler_is_expr(IsExpr *expr)
{
    compiler_expr(expr->left);

    vm_write_chunk(IS_OPC, COMPILER_VM);

    switch (expr->type_token->type)
    {
    case NIL_TOKTYPE:
        vm_write_chunk(0, COMPILER_VM);
        break;

    case BOOL_TOKTYPE:
        vm_write_chunk(1, COMPILER_VM);
        break;

    case INT_TOKTYPE:
        vm_write_chunk(2, COMPILER_VM);
        break;

    case STR_TOKTYPE:
        vm_write_chunk(3, COMPILER_VM);
        break;

    case ARR_TOKTYPE:
        vm_write_chunk(4, COMPILER_VM);
        break;

    case PROC_TOKTYPE:
        vm_write_chunk(5, COMPILER_VM);
        break;

    case CLASS_TOKTYPE:
        vm_write_chunk(6, COMPILER_VM);
        break;

    case INSTANCE_TOKTYPE:
        vm_write_chunk(7, COMPILER_VM);
        break;

    default:
        assert(0 && "Illegal type value.");
    }
}

void compiler_from_expr(FromExpr *expr)
{
    compiler_expr(expr->left);

    vm_write_chunk(FROM_OPC, COMPILER_VM);
    vm_write_str_const(expr->klass_identifier_token->lexeme, COMPILER_VM);
}

void compiler_arr_expr(ArrExpr *expr)
{
    Expr *length = expr->len_expr;
    DynArrPtr *exprs = expr->items;

    for (size_t i = 0; i < exprs->used; i++)
    {
        Expr *expr = (Expr *)DYNARR_PTR_GET(i, exprs);
        compiler_expr(expr);
    }

    if (length)
        compiler_expr(length);
    else
    {
        vm_write_chunk(ICONST_OPC, COMPILER_VM);
        vm_write_i64_const((int64_t)exprs->used, COMPILER_VM);
    }

    vm_write_chunk(OARR_OPC, COMPILER_VM);
    vm_write_chunk(exprs->used == 0, COMPILER_VM);
}

void compiler_logical_expr(LogicalExpr *expr)
{
    compiler_expr(expr->left);
    compiler_expr(expr->right);

    Token *operator= expr->operator;

    switch (operator->type)
    {
    case OR_TOKTYPE:
        vm_write_chunk(OR_OPC, COMPILER_VM);
        break;

    case AND_TOKTYPE:
        vm_write_chunk(AND_OPC, COMPILER_VM);
        break;

    default:
        assert(0 && "Illegal logical operator type value");
    }
}

void compiler_comparison_expr(ComparisonExpr *expr)
{
    compiler_expr(expr->left);
    compiler_expr(expr->right);

    Token *operator= expr->operator;

    switch (operator->type)
    {
    case LESS_TOKTYPE:
        vm_write_chunk(LT_OPC, COMPILER_VM);
        break;

    case GREATER_TOKTYPE:
        vm_write_chunk(GT_OPC, COMPILER_VM);
        break;

    case LESS_EQUALS_TOKTYPE:
        vm_write_chunk(LE_OPC, COMPILER_VM);
        break;

    case GREATER_EQUALS_TOKTYPE:
        vm_write_chunk(GE_OPC, COMPILER_VM);
        break;

    case EQUALS_EQUALS_TOKTYPE:
        vm_write_chunk(EQ_OPC, COMPILER_VM);
        break;

    case NOT_EQUALS_TOKTYPE:
        vm_write_chunk(NE_OPC, COMPILER_VM);
        break;

    default:
        assert(0 && "Illegal comparison operator type value");
    }
}

void compiler_binary_expr(BinaryExpr *expr)
{
    compiler_expr(expr->left);
    compiler_expr(expr->right);

    Token *operator= expr->operator;

    switch (operator->type)
    {
    case PLUS_TOKTYPE:
        vm_write_chunk(ADD_OPC, COMPILER_VM);
        break;

    case MINUS_TOKTYPE:
        vm_write_chunk(SUB_OPC, COMPILER_VM);
        break;

    case ASTERISK_TOKTYPE:
        vm_write_chunk(MUL_OPC, COMPILER_VM);
        break;

    case SLASH_TOKTYPE:
        vm_write_chunk(DIV_OPC, COMPILER_VM);
        break;

    case PERCENT_TOKTYPE:
        vm_write_chunk(MOD_OPC, COMPILER_VM);
        break;

    default:
        assert(0 && "Illegal binary operator type value");
    }
}

void compiler_unary_expr(UnaryExpr *expr)
{
    compiler_expr(expr->right);

    Token *operator= expr->operator;

    switch (operator->type)
    {
    case EXCLAMATION_TOKTYPE:
        vm_write_chunk(NOT_OPC, COMPILER_VM);
        break;

    case MINUS_TOKTYPE:
        vm_write_chunk(NNOT_OPC, COMPILER_VM);
        break;

    default:
        assert(0 && "Illegal unary operator value");
    }
}

void compiler_arr_access(ArrAccessExpr *expr)
{
    compiler_expr(expr->expr);
    compiler_expr(expr->index_expr);
    vm_write_chunk(ARR_ITM_OPC, COMPILER_VM);
}

void compiler_access(AccessExpr *expr)
{
    Expr *left = expr->left;
    Token *identifier = expr->identifier;

    compiler_expr(left);

    vm_write_chunk(GET_PROPERTY_OPC, COMPILER_VM);
    vm_write_str_const(identifier->lexeme, COMPILER_VM);
}

void compiler_call_expr(CallExpr *expr)
{
    Expr *left = expr->left;
    DynArrPtr *args = expr->args;

    compiler_expr(left);

    for (int i = args->used - 1; i >= 0; i--)
    {
        Expr *expr = (Expr *)DYNARR_PTR_GET(i, args);
        compiler_expr(expr);
    }

    vm_write_chunk(CALL_OPC, COMPILER_VM);
    vm_write_chunk((uint8_t)args->used, COMPILER_VM);
}

void compiler_this_expr(ThisExpr *expr)
{
    int klass_scope = compiler_scope_inside_klass();

    if (klass_scope == -1)
        compiler_error_at(expr->thisToken, "'this' expressions can only be used inside classes.");

    vm_write_chunk(THIS_OPC, COMPILER_VM);

    Token *identifier_token = expr->identifier_token;

    if (identifier_token)
    {
        char *identifier = identifier_token->lexeme;

        vm_write_chunk(GET_PROPERTY_OPC, COMPILER_VM);
        vm_write_str_const(identifier, COMPILER_VM);
    }
}

void compiler_nil_expr()
{
    vm_write_chunk(NIL_OPC, COMPILER_VM);
}

void compiler_bool_expr(LiteralExpr *expr)
{
    vm_write_chunk(BCONST_OPC, COMPILER_VM);
    vm_write_bool_const(*(int8_t *)expr->literal, COMPILER_VM);
}

void compiler_int_expr(LiteralExpr *expr)
{
    vm_write_chunk(ICONST_OPC, COMPILER_VM);
    vm_write_i64_const(*(int64_t *)expr->literal, COMPILER_VM);
}

void compiler_str_expr(LiteralExpr *expr)
{
    vm_write_chunk(SCONST_OPC, COMPILER_VM);
    vm_write_str_const((char *)expr->literal, COMPILER_VM);
}

void compiler_identifier_expr(IdentifierExpr *expr)
{
    Token *identifier_token = expr->identifier_token;
    char *identifier = identifier_token->lexeme;

    DynArrPtr *natives = compiler->natives;

    for (size_t i = 0; i < natives->used; i++)
    {
        char *native = DYNARR_PTR_GET(i, natives);

        if (strcmp(identifier, native) == 0)
        {
            vm_write_chunk(LOAD_OPC, COMPILER_VM);
            vm_write_i32(i, COMPILER_VM);

            return;
        }
    }

    Symbol *symbol = compiler_get(identifier_token);

    if (symbol->is_entity)
    {
        vm_write_chunk(LOAD_OPC, COMPILER_VM);
        vm_write_i32(symbol->local, COMPILER_VM);

        return;
    }

    if (symbol->class_bound)
    {
        vm_write_chunk(THIS_OPC, COMPILER_VM);
        vm_write_chunk(GET_PROPERTY_OPC, COMPILER_VM);
        vm_write_str_const(identifier, COMPILER_VM);

        return;
    }

    if (symbol->global)
    {
        vm_write_chunk(GREAD_OPC, COMPILER_VM);
        vm_write_str_const(identifier, COMPILER_VM);
    }
    else
    {
        vm_write_chunk(LREAD_OPC, COMPILER_VM);
        vm_write_chunk(symbol->local, COMPILER_VM);
    }
}

void compiler_group_expr(GroupExpr *expr)
{
    compiler_expr(expr->e);
}

void compiler_expr(Expr *expr)
{
    switch (expr->type)
    {
    case ASSIGN_EXPR_TYPE:
        compiler_assign_expr((AssignExpr *)expr->e);
        break;

    case IS_EXPR_TYPE:
        compiler_is_expr((IsExpr *)expr->e);
        break;

    case FROM_EXPR_TYPE:
        compiler_from_expr((FromExpr *)expr->e);
        break;

    case ARR_EXPR_TYPE:
        compiler_arr_expr((ArrExpr *)expr->e);
        break;

    case LOGICAL_EXPR_TYPE:
        compiler_logical_expr((LogicalExpr *)expr->e);
        break;

    case COMPARISON_EXPR_TYPE:
        compiler_comparison_expr((ComparisonExpr *)expr->e);
        break;

    case BINARY_EXPR_TYPE:
        compiler_binary_expr((BinaryExpr *)expr->e);
        break;

    case UNARY_EXPR_TYPE:
        compiler_unary_expr((UnaryExpr *)expr->e);
        break;

    case ARR_ACCESS_EXPR_TYPE:
        compiler_arr_access((ArrAccessExpr *)expr->e);
        break;

    case ACCESS_EXPR_TYPE:
        compiler_access((AccessExpr *)expr->e);
        break;

    case CALL_EXPR_TYPE:
        compiler_call_expr((CallExpr *)expr->e);
        break;

    case THIS_EXPR_TYPE:
        compiler_this_expr((ThisExpr *)expr->e);
        break;

    case NIL_EXPR_TYPE:
        compiler_nil_expr();
        break;

    case BOOL_EXPR_TYPE:
        compiler_bool_expr((LiteralExpr *)expr->e);
        break;

    case INT_EXPR_TYPE:
        compiler_int_expr((LiteralExpr *)expr->e);
        break;

    case STR_EXPR_TYPE:
        compiler_str_expr((LiteralExpr *)expr->e);
        break;

    case IDENTIFIER_EXPR_TYPE:
        compiler_identifier_expr((IdentifierExpr *)expr->e);
        break;

    case GROUP_EXPR_TYPE:
        compiler_group_expr((GroupExpr *)expr->e);
        break;

    default:
        assert(0 && "Illegal expression type value");
    }
}

void compiler_var_decl_stmt(VarDeclStmt *stmt)
{
    Token *identifier_token = stmt->identifier;
    char *identifier = identifier_token->lexeme;
    Expr *initializer = stmt->initializer;

    Symbol *symbol = compiler_declare(0, identifier_token);

    if (initializer)
        compiler_expr(initializer);
    else
        vm_write_chunk(NIL_OPC, COMPILER_VM);

    if (symbol->global)
    {
        vm_write_chunk(GWRITE_OPC, COMPILER_VM);
        vm_write_str_const(identifier, COMPILER_VM);
    }
    else
    {
        vm_write_chunk(LSET_OPC, COMPILER_VM);
        vm_write_chunk(symbol->local, COMPILER_VM);
    }

    vm_write_chunk(POP_OPC, COMPILER_VM);
}

void compiler_block_stmt(BlockStmt *stmt)
{
    DynArrPtr *stmts = stmt->stmts;

    compiler_scope_in(BLOCK_SCOPE);

    for (size_t i = 0; i < stmts->used; i++)
    {
        Stmt *stmt = (Stmt *)DYNARR_PTR_GET(i, stmts);
        compiler_stmt(stmt);
    }

    compiler_scope_out();
}

void compiler_if_stmt(IfStmt *stmt)
{
    DynArr *elif_lengths = memory_create_dynarr(sizeof(size_t) * 2);

    IfStmtBranch *if_branch = stmt->if_branch;
    Expr *if_condition = if_branch->condition;
    DynArrPtr *if_stmts = if_branch->stmts;

    DynArrPtr *elif_branches = stmt->elif_branches;

    DynArrPtr *else_stmts = stmt->else_stmts;

    compiler_expr(if_condition);

    vm_write_chunk(JIF_OPC, COMPILER_VM);
    size_t if_index_start = vm_write_i32(0, COMPILER_VM);
    size_t if_start_len = vm_block_length(COMPILER_VM);

    compiler_scope_in(IF_SCOPE);

    for (size_t i = 0; i < if_stmts->used; i++)
    {
        Stmt *stmt = (Stmt *)DYNARR_PTR_GET(i, if_stmts);
        compiler_stmt(stmt);
    }

    compiler_scope_out();

    vm_write_chunk(JMP_OPC, COMPILER_VM);
    size_t if_index_end = vm_write_i32(0, COMPILER_VM);

    size_t if_end_len = vm_block_length(COMPILER_VM);
    vm_update_i32(if_index_start, if_end_len - if_start_len, COMPILER_VM);

    if (elif_branches)
    {
        for (size_t i = 0; i < elif_branches->used; i++)
        {
            IfStmtBranch *branch = (IfStmtBranch *)DYNARR_PTR_GET(i, elif_branches);
            Expr *elif_condition = branch->condition;
            DynArrPtr *elif_stmts = branch->stmts;

            compiler_scope_in(ELIF_SCOPE);

            size_t len_before_elif = vm_block_length(COMPILER_VM);

            compiler_expr(elif_condition);

            vm_write_chunk(JIF_OPC, COMPILER_VM);
            size_t jif_index = vm_write_i32(0, COMPILER_VM);

            size_t len_before_elif_body = vm_block_length(COMPILER_VM);

            for (size_t i = 0; i < elif_stmts->used; i++)
            {
                Stmt *stmt = (Stmt *)DYNARR_PTR_GET(i, elif_stmts);
                compiler_stmt(stmt);
            }

            vm_write_chunk(JMP_OPC, COMPILER_VM);
            size_t jmp_index = vm_write_i32(0, COMPILER_VM);

            size_t len_after_elif = vm_block_length(COMPILER_VM);

            size_t len_elif = len_after_elif - len_before_elif;
            size_t len_elif_body = len_after_elif - len_before_elif_body;

            vm_update_i32(jif_index, len_elif_body, COMPILER_VM);

            size_t values[] = {len_elif, jmp_index};
            dynarr_insert((void *)values, elif_lengths);

            compiler_scope_out();
        }
    }

    size_t len_alt_branchs = 0;

    if (else_stmts)
    {
        compiler_scope_in(ELSE_SCOPE);

        size_t len_before = vm_block_length(COMPILER_VM);

        for (size_t i = 0; i < else_stmts->used; i++)
        {
            Stmt *stmt = (Stmt *)DYNARR_PTR_GET(i, else_stmts);
            compiler_stmt(stmt);
        }

        size_t len_after = vm_block_length(COMPILER_VM);

        len_alt_branchs = len_after - len_before;

        compiler_scope_out();
    }

    size_t len_after_if = vm_block_length(COMPILER_VM);
    vm_update_i32(if_index_end, len_after_if - if_end_len, COMPILER_VM);

    for (int i = elif_lengths->used - 1; i >= 0; i--)
    {
        size_t *current_len_index = (size_t *)dynarr_get(i, elif_lengths);

        size_t len = current_len_index[0];
        size_t index = current_len_index[1];

        vm_update_i32(index, len_alt_branchs, COMPILER_VM);

        len_alt_branchs += len;
    }

    memory_destroy_dynarr(elif_lengths);
}

void compiler_continue_stmt(ContinueStmt *stmt)
{
    if (!compiler_scope_inside_loop())
        compiler_error_at(stmt->continue_token, "Can only use 'continue' statement inside native loops.");

    vm_write_chunk(JMP_OPC, COMPILER_VM);
    size_t jmp_index = vm_write_i32(0, COMPILER_VM);

    size_t len = vm_block_length(COMPILER_VM);

    size_t *info = (size_t *)memory_alloc(sizeof(size_t) * 2);

    info[0] = len;
    info[1] = jmp_index;

    lzstack_push((void *)info, compiler->continues, NULL);
}

void compiler_break_stmt(BreakStmt *stmt)
{
    if (!compiler_scope_inside_loop())
        compiler_error_at(stmt->break_token, "Can only use 'break' statement inside native loops.");

    vm_write_chunk(JMP_OPC, COMPILER_VM);
    size_t jmp_index = vm_write_i32(0, COMPILER_VM);

    size_t len = vm_block_length(COMPILER_VM);

    size_t *info = (size_t *)memory_alloc(sizeof(size_t) * 2);

    info[0] = len;
    info[1] = jmp_index;

    lzstack_push((void *)info, compiler->breaks, NULL);
}

void compiler_while_stmt(WhileStmt *stmt)
{
    Expr *condition = stmt->condition;
    DynArrPtr *stmts = stmt->stmts;

    size_t bef_while_len = vm_block_length(COMPILER_VM);

    vm_write_chunk(JMP_OPC, COMPILER_VM);
    size_t jmp_index = vm_write_i32(0, COMPILER_VM);

    size_t bef_loop_body_len = vm_block_length(COMPILER_VM);

    compiler_scope_in(WHILE_SCOPE);

    for (size_t i = 0; i < stmts->used; i++)
    {
        Stmt *stmt = (Stmt *)DYNARR_PTR_GET(i, stmts);
        compiler_stmt(stmt);
    }

    compiler_scope_out();

    size_t after_loop_body_len = vm_block_length(COMPILER_VM);
    size_t loop_body_len = after_loop_body_len - bef_loop_body_len;

    vm_update_i32(jmp_index, loop_body_len, COMPILER_VM);

    compiler_expr(condition);

    size_t after_while_len = vm_block_length(COMPILER_VM);
    size_t while_len = after_while_len - bef_while_len;

    vm_write_chunk(JIT_OPC, COMPILER_VM);
    vm_write_i32(while_len * -1, COMPILER_VM);

    size_t after_whole_while_len = vm_block_length(COMPILER_VM);

    size_t *continue_info = (size_t *)lzstack_pop(compiler->continues);

    while (continue_info)
    {
        size_t len = continue_info[0];
        size_t index = continue_info[1];

        vm_update_i32(index, after_loop_body_len - len, COMPILER_VM);

        memory_dealloc(continue_info);
        continue_info = (size_t *)lzstack_pop(compiler->continues);
    }

    size_t *break_info = (size_t *)lzstack_pop(compiler->breaks);

    while (break_info)
    {
        size_t len = break_info[0];
        size_t index = break_info[1];

        vm_update_i32(index, after_whole_while_len - len, COMPILER_VM);

        memory_dealloc(break_info);
        break_info = (size_t *)lzstack_pop(compiler->breaks);
    }
}

void compiler_fn_stmt(FnStmt *stmt)
{
    if (compiler_scope_inside_fn())
        compiler_error_at(stmt->identifier_token, "Can not declare a function inside another one.");

    Token *identifier_token = stmt->identifier_token;
    DynArrPtr *params = stmt->params;
    DynArrPtr *stmts = stmt->stmts;

    compiler_declare(1, identifier_token);

    compiler_scope_in(FN_SCOPE);

    vm_fn_start(identifier_token->lexeme, COMPILER_VM);

    for (size_t i = 0; i < params->used; i++)
    {
        Token *param_token = (Token *)DYNARR_PTR_GET(i, params);

        compiler_declare(0, param_token);

        vm_fn_add_param(param_token->lexeme, COMPILER_VM);
    }

    for (size_t i = 0; i < stmts->used; i++)
    {
        Stmt *stmt = (Stmt *)DYNARR_PTR_GET(i, stmts);

        compiler_stmt(stmt);

        if (i + 1 >= stmts->used && stmt->type != RETURN_STMT_TYPE)
        {
            vm_write_chunk(NIL_OPC, COMPILER_VM);
            vm_write_chunk(RET_OPC, COMPILER_VM);
        }
    }

    vm_fn_end(COMPILER_VM);

    compiler_scope_out();
}

void compiler_klass_stmt(ClassStmt *stmt)
{
    if (compiler_scope_current()->type != GLOBAL_SCOPE)
        compiler_error_at(stmt->identifier, "Classes can only be declared at global scope.");

    Token *identifier_token = stmt->identifier;
    FnStmt *constructor = stmt->constructor;
    DynArrPtr *methods = stmt->methods;

    compiler_declare(1, identifier_token);

    compiler_scope_in(KLASS_SCOPE);

    vm_klass_start(identifier_token->lexeme, COMPILER_VM);

    // Declaring, before hand, the class methods. This is done in
    // order to methods can call each other
    for (size_t method_index = 0; method_index < methods->used; method_index++)
    {
        Stmt *raw_fn_stmt = (Stmt *)DYNARR_PTR_GET(method_index, methods);
        FnStmt *fn_stmt = (FnStmt *)raw_fn_stmt->s;

        Token *fn_identifier_token = fn_stmt->identifier_token;

        compiler_declare(0, fn_identifier_token)->class_bound = 1;
    }

    if (constructor)
    {
        DynArrPtr *params = constructor->params;
        DynArrPtr *body = constructor->stmts;

        compiler_scope_in(CONSTRUCTOR_SCOPE);

        vm_klass_constructor_start(COMPILER_VM);

        if (params)
        {
            for (size_t param_index = 0; param_index < params->used; param_index++)
            {
                Token *param_token = DYNARR_PTR_GET(param_index, params);
                char *param = param_token->lexeme;

                compiler_declare(0, param_token);

                vm_fn_add_param(param, COMPILER_VM);
            }
        }

        for (size_t stmt_index = 0; stmt_index < body->used; stmt_index++)
        {
            Stmt *stmt = DYNARR_PTR_GET(stmt_index, body);

            compiler_stmt(stmt);
        }

        vm_write_chunk(THIS_OPC, COMPILER_VM);
        vm_write_chunk(RET_OPC, COMPILER_VM);

        vm_klass_constructor_end(COMPILER_VM);

        compiler_scope_out();
    }

    for (size_t method_index = 0; method_index < methods->used; method_index++)
    {
        Stmt *raw_fn_stmt = (Stmt *)DYNARR_PTR_GET(method_index, methods);
        FnStmt *fn_stmt = (FnStmt *)raw_fn_stmt->s;

        Token *fn_identifier_token = fn_stmt->identifier_token;
        DynArrPtr *fn_params = fn_stmt->params;
        DynArrPtr *fn_stmts = fn_stmt->stmts;

        compiler_scope_in(FN_SCOPE);

        vm_klass_fn_start(fn_identifier_token->lexeme, COMPILER_VM);

        for (size_t param_index = 0; param_index < fn_params->used; param_index++)
        {
            Token *param_identifier_token = (Token *)DYNARR_PTR_GET(param_index, fn_params);
            char *param_identifier = param_identifier_token->lexeme;

            compiler_declare(0, param_identifier_token);

            vm_fn_add_param(param_identifier, COMPILER_VM);
        }

        for (size_t stmt_index = 0; stmt_index < fn_stmts->used; stmt_index++)
        {
            Stmt *stmt = (Stmt *)DYNARR_PTR_GET(stmt_index, fn_stmts);

            compiler_stmt(stmt);

            if (stmt_index + 1 >= fn_stmts->used && stmt->type != RETURN_STMT_TYPE)
            {
                vm_write_chunk(NIL_OPC, COMPILER_VM);
                vm_write_chunk(RET_OPC, COMPILER_VM);
            }
        }

        vm_klass_fn_end(COMPILER_VM);

        compiler_scope_out();
    }

    vm_klass_end(COMPILER_VM);

    compiler_scope_out();
}

void compiler_print_stmt(PrintStmt *stmt)
{
    compiler_expr(stmt->expr);
    vm_write_chunk(PRT_OPC, COMPILER_VM);
}

void compiler_return_stmt(ReturnStmt *stmt)
{
    if (compiler_scope_current()->type == CONSTRUCTOR_SCOPE)
        compiler_error_at(stmt->return_token, "Can not use 'return' statement inside constructors.");

    if (!compiler_scope_inside_fn())
        compiler_error_at(stmt->return_token, "Can only use 'return' statement inside functinos.");

    Expr *value = stmt->value;

    if (value)
        compiler_expr(value);
    else
        vm_write_chunk(NIL_OPC, COMPILER_VM);

    vm_write_chunk(RET_OPC, COMPILER_VM);
}

void compiler_expr_stmt(ExprStmt *stmt)
{
    compiler_expr(stmt->expr);
    vm_write_chunk(POP_OPC, COMPILER_VM);
}

void compiler_stmt(Stmt *stmt)
{
    switch (stmt->type)
    {
    case EXPR_STMT_TYPE:
        compiler_expr_stmt((ExprStmt *)stmt->s);
        break;

    case RETURN_STMT_TYPE:
        compiler_return_stmt((ReturnStmt *)stmt->s);
        break;

    case BLOCK_STMT_TYPE:
        compiler_block_stmt((BlockStmt *)stmt->s);
        break;

    case IF_STMT_TYPE:
        compiler_if_stmt((IfStmt *)stmt->s);
        break;

    case CONTINUE_STMT_TYPE:
        compiler_continue_stmt((ContinueStmt *)stmt->s);
        break;

    case BREAK_STMT_TYPE:
        compiler_break_stmt((BreakStmt *)stmt->s);
        break;

    case WHILE_STMT_TYPE:
        compiler_while_stmt((WhileStmt *)stmt->s);
        break;

    case FN_STMT_TYPE:
        compiler_fn_stmt((FnStmt *)stmt->s);
        break;

    case CLASS_STMT_TYPE:
        compiler_klass_stmt((ClassStmt *)stmt->s);
        break;

    case PRINT_STMT_TYPE:
        compiler_print_stmt((PrintStmt *)stmt->s);
        break;

    case VAR_DECL_STMT_TYPE:
        compiler_var_decl_stmt((VarDeclStmt *)stmt->s);
        break;

    default:
        assert(0 && "Illegal stmt type value");
    }
}

// public implementation
void compiler_compile(VM *vm, DynArrPtr *stmts)
{
    LZHTable *symbols = memory_create_lzhtable(1669);
    LZStack *continues = memory_create_lzstack();
    LZStack *breaks = memory_create_lzstack();
    DynArrPtr *natives = memory_create_dynarr_ptr();

    compiler = memory_create_compiler();

    memset(compiler->scope_stack, 0, sizeof(compiler->scope_stack));

    compiler->depth = 0;

    compiler->scope_stack[0].local = 0;
    compiler->scope_stack[0].type = GLOBAL_SCOPE;
    compiler->scope_stack[0].symbols = memory_create_lzhtable(17);

    compiler->vm = vm;
    compiler->stmts = stmts;
    compiler->entity_counter = 0;

    compiler->continues = continues;
    compiler->breaks = breaks;

    compiler->natives = natives;

    //> vm natives functions
    dynarr_ptr_insert((void *)memory_clone_raw_str("char_code"), natives);
    dynarr_ptr_insert((void *)memory_clone_raw_str("code_char"), natives);
    dynarr_ptr_insert((void *)memory_clone_raw_str("sub_str"), natives);
    dynarr_ptr_insert((void *)memory_clone_raw_str("str_lower"), natives);
    dynarr_ptr_insert((void *)memory_clone_raw_str("str_upper"), natives);
    dynarr_ptr_insert((void *)memory_clone_raw_str("str_title"), natives);
    dynarr_ptr_insert((void *)memory_clone_raw_str("cmp_str"), natives);
    dynarr_ptr_insert((void *)memory_clone_raw_str("cmp_ic_str"), natives);
    dynarr_ptr_insert((void *)memory_clone_raw_str("is_str_int"), natives);
    dynarr_ptr_insert((void *)memory_clone_raw_str("ascii_to_int"), natives);
    dynarr_ptr_insert((void *)memory_clone_raw_str("int_to_ascii"), natives);

    dynarr_ptr_insert((void *)memory_clone_raw_str("time"), natives);
    dynarr_ptr_insert((void *)memory_clone_raw_str("sleep"), natives);

    dynarr_ptr_insert((void *)memory_clone_raw_str("read_ln"), natives);
    dynarr_ptr_insert((void *)memory_clone_raw_str("read_file_bytes"), natives);

    dynarr_ptr_insert((void *)memory_clone_raw_str("panic"), natives);
    dynarr_ptr_insert((void *)memory_clone_raw_str("exit"), natives);
    //< vm natives functions

    //> language natives functions
    dynarr_ptr_insert((void *)memory_clone_raw_str("arr_len"), natives);
    dynarr_ptr_insert((void *)memory_clone_raw_str("str_len"), natives);
    dynarr_ptr_insert((void *)memory_clone_raw_str("str_char"), natives);
    dynarr_ptr_insert((void *)memory_clone_raw_str("concat"), natives);
    //< language natives functions

    //> arr_len function
    vm_fn_start("arr_len", COMPILER_VM);
    vm_fn_add_param("arr", COMPILER_VM);

    vm_write_chunk(LREAD_OPC, COMPILER_VM);
    vm_write_chunk(0, COMPILER_VM);

    vm_write_chunk(ARR_LEN_OPC, COMPILER_VM);

    vm_write_chunk(RET_OPC, COMPILER_VM);

    vm_fn_end(COMPILER_VM);
    //< arr_len function

    //> str_len function
    vm_fn_start("str_len", COMPILER_VM);
    vm_fn_add_param("str", COMPILER_VM);

    vm_write_chunk(LREAD_OPC, COMPILER_VM);
    vm_write_chunk(0, COMPILER_VM);

    vm_write_chunk(STR_LEN_OPC, COMPILER_VM);

    vm_write_chunk(RET_OPC, COMPILER_VM);

    vm_fn_end(COMPILER_VM);
    //< str_len function

    //> str_char function
    vm_fn_start("str_char", COMPILER_VM);
    vm_fn_add_param("str", COMPILER_VM);
    vm_fn_add_param("index", COMPILER_VM);

    vm_write_chunk(LREAD_OPC, COMPILER_VM);
    vm_write_chunk(1, COMPILER_VM);

    vm_write_chunk(LREAD_OPC, COMPILER_VM);
    vm_write_chunk(0, COMPILER_VM);

    vm_write_chunk(STR_ITM_OPC, COMPILER_VM);

    vm_write_chunk(RET_OPC, COMPILER_VM);

    vm_fn_end(COMPILER_VM);
    //< str_char function

    //> concat function
    vm_fn_start("concat", COMPILER_VM);

    vm_fn_add_param("str_0", COMPILER_VM);
    vm_fn_add_param("str_1", COMPILER_VM);

    vm_write_chunk(LREAD_OPC, COMPILER_VM);
    vm_write_chunk(0, COMPILER_VM);

    vm_write_chunk(LREAD_OPC, COMPILER_VM);
    vm_write_chunk(1, COMPILER_VM);

    vm_write_chunk(CONCAT_OPC, COMPILER_VM);

    vm_write_chunk(RET_OPC, COMPILER_VM);

    vm_fn_end(COMPILER_VM);
    //< concat function

    for (size_t i = 0; i < stmts->used; i++)
    {
        Stmt *stmt = DYNARR_PTR_GET(i, stmts);
        compiler_stmt(stmt);
    }

    memory_destroy_lzhtable(symbols);
    memory_destroy_lzstack(continues);
    memory_destroy_lzstack(breaks);
    memory_destroy_compiler(compiler);

    compiler = NULL;
}