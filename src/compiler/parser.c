#include "token.h"

#include "expr.h"
#include "stmt.h"
#include "parser.h"
#include "error_report.h"

// private interface
void parser_error_at(Token *token, char *msg, ...);

int parser_is_at_end(Parser *parser);

Token *parser_advance(Parser *parser);
Token *parser_peek(Parser *parser);
Token *parser_peek_next(Parser *parser);
Token *parser_previous(Parser *parser);

int parser_check(Parser *parser, TokenType type);
int parser_check_next(Parser *parser, TokenType type);
int parser_match(Parser *parser, size_t count, ...);
Token *parser_consume(Parser *parser, TokenType type, char *err_msg, ...);

Expr *parser_expr(Parser *parser);
Expr *parser_assign_expr(Parser *parser);
Expr *parser_arr_expr(Parser *parser);
Expr *parser_type_expr(Parser *parser);
Expr *parser_bit_or(Parser *parser);
Expr *parser_bit_xor(Parser *parser);
Expr *parser_bit_and(Parser *parser);
Expr *parser_bit_not(Parser *parser);
Expr *parser_or_expr(Parser *parser);
Expr *parser_and_expr(Parser *parser);
Expr *parser_comparison_expr(Parser *parser);
Expr *parser_shift_expr(Parser *parser);
Expr *parser_term_expr(Parser *parser);
Expr *parser_factor_expr(Parser *parser);
Expr *parser_unary_expr(Parser *parser);
Expr *parser_access_expr(Parser *parser);
Expr *parser_this_expr(Parser *parser);
Expr *parser_literal_expr(Parser *parser);

Stmt *parser_stmt(Parser *parser);
Stmt *parser_var_decl_stmt(Parser *parser);
DynArrPtr *parser_block_stmt(Parser *parser);
IfStmtBranch *parser_if_branch(Parser *parser);
Stmt *parser_if_stmt(Parser *parser);
Stmt *parser_continue_stmt(Parser *parser);
Stmt *parser_break_stmt(Parser *parser);
Stmt *parser_while_stmt(Parser *parser);
Stmt *parser_for_stmt(Parser *parser);
Stmt *parser_fn_stmt(Parser *parser);
Stmt *parser_class_stmt(Parser *parser);
Stmt *parser_print_stmt(Parser *parser);
Stmt *parser_return_stmt(Parser *parser);
Stmt *parser_expr_stmt(Parser *parser);

// private implementation
void parser_error_at(Token *token, char *msg, ...)
{
    va_list args;
    va_start(args, msg);

    report_error_at(17, token->line, "Parser", msg, args);

    va_end(args);
}

int parser_is_at_end(Parser *parser)
{
    size_t current = (size_t)parser->current;
    Token *token = (Token *)dynarr_get(current, parser->tokens);

    return token->type == EOF_TOKTYPE;
}

Token *parser_advance(Parser *parser)
{
    return (Token *)dynarr_get((size_t)parser->current++, parser->tokens);
}

Token *parser_peek(Parser *parser)
{
    return (Token *)dynarr_get((size_t)parser->current, parser->tokens);
}

Token *parser_peek_next(Parser *parser)
{
    if ((size_t)parser->current + 1 >= parser->tokens->used)
        return NULL;

    return (Token *)dynarr_get(parser->current + 1, parser->tokens);
}

Token *parser_previous(Parser *parser)
{
    return (Token *)dynarr_get(parser->current - 1, parser->tokens);
}

int parser_check(Parser *parser, TokenType type)
{
    Token *token = parser_peek(parser);
    return token->type == type;
}

int parser_check_next(Parser *parser, TokenType type)
{
    Token *token = parser_peek_next(parser);

    if (token)
        return token->type == type;

    return 0;
}

int parser_match(Parser *parser, size_t count, ...)
{
    va_list argp;
    va_start(argp, count);

    Token *token = parser_peek(parser);

    for (size_t i = 0; i < count; i++)
    {
        TokenType type = va_arg(argp, TokenType);

        if (type == token->type)
        {
            parser->current++;
            va_end(argp);
            return 1;
        }
    }

    va_end(argp);

    return 0;
}

Token *parser_consume(Parser *parser, TokenType type, char *err_msg, ...)
{
    Token *token = parser_peek(parser);

    if (token->type == type)
    {
        parser->current++;
        return token;
    }

    va_list args;
    va_start(args, err_msg);

    report_error_at(17, token->line, "Parser", err_msg, args);

    va_end(args);

    return NULL;
}

Expr *parser_expr(Parser *parser)
{
    return parser_assign_expr(parser);
}

Expr *parser_assign_expr(Parser *parser)
{
    Expr *left = parser_arr_expr(parser);

    if (parser_match(parser, 1, EQUALS_TOKTYPE))
    {
        Token *equals_token = memory_clone_token(parser_previous(parser));
        Expr *right = parser_assign_expr(parser);

        AssignExpr *expr = memory_create_assign_expr(left, equals_token, right);

        return memory_create_expr(expr, ASSIGN_EXPR_TYPE);
    }

    return left;
}

Expr *parser_is_expr(Expr *left, Token *is_token, Parser *parser)
{
    if (parser_match(parser, 7,
                     NIL_TOKTYPE,
                     BOOL_TOKTYPE,
                     INT_TOKTYPE,
                     STR_TOKTYPE,
                     ARR_TOKTYPE,
                     PROC_TOKTYPE,
                     CLASS_TOKTYPE,
                     INSTANCE_TOKTYPE))
    {
        Token *type_token = memory_clone_token(parser_previous(parser));

        IsExpr *expr = memory_create_is_expr(
            left,
            memory_clone_token(is_token),
            type_token);

        return memory_create_expr(expr, IS_EXPR_TYPE);
    }

    parser_error_at(parser_peek(parser), "Expect 'nil', 'bool', 'int', 'str', 'klass' or 'instance', but got something else.");

    return NULL;
}

Expr *parser_from_expr(Expr *left, Token *from_token, Parser *parser)
{
    Token *klass_name_token = parser_consume(
        parser,
        IDENTIFIER_TOKTYPE,
        "Expect klass identifier at right side of from expression.");

    FromExpr *expr = memory_create_from_expr(
        left,
        memory_clone_token(from_token),
        memory_clone_token(klass_name_token));

    return memory_create_expr(expr, FROM_EXPR_TYPE);
}

Expr *parser_arr_expr(Parser *parser)
{
    if (parser_match(parser, 1, LEFT_SQUARE_TOKTYPE))
    {
        Token *left_square_token = memory_clone_token(parser_previous(parser));
        DynArrPtr *items = memory_create_dynarr_ptr();
        Expr *len_expr = NULL;

        if (!parser_check(parser, RIGHT_SQUARE_TOKTYPE))
        {
            do
            {
                dynarr_ptr_insert(parser_expr(parser), items);
            } while (parser_match(parser, 1, COMMA_TOKTYPE));
        }

        parser_consume(parser, RIGHT_SQUARE_TOKTYPE, "Expect ']' at end or array creation expression.");

        if (parser_match(parser, 1, COLON_TOKTYPE))
            len_expr = parser_expr(parser);

        ArrExpr *expr = memory_create_arr_expr(left_square_token, items, len_expr);

        return memory_create_expr(expr, ARR_EXPR_TYPE);
    }

    return parser_type_expr(parser);
}

Expr *parser_type_expr(Parser *parser)
{
    Expr *left = parser_bit_or(parser);

    if (parser_match(parser, 2, IS_TOKTYPE, FROM_TOKTYPE))
    {
        Token *previous_token = parser_previous(parser);

        if (previous_token->type == IS_TOKTYPE)
            return parser_is_expr(left, previous_token, parser);
        else
            return parser_from_expr(left, previous_token, parser);
    }

    return left;
}

Expr *parser_bit_or(Parser *parser)
{
    Expr *left = parser_bit_xor(parser);

    while (parser_match(parser, 1, BITWISE_OR_TOKTYPE))
    {
        Token *operator_token = memory_clone_token(parser_previous(parser));
        Expr *right = parser_bit_xor(parser);

        BinaryExpr *expr = memory_create_binary_expr(left, operator_token, right);

        left = memory_create_expr(expr, BINARY_EXPR_TYPE);
    }

    return left;
}

Expr *parser_bit_xor(Parser *parser)
{
    Expr *left = parser_bit_and(parser);

    while (parser_match(parser, 1, BITWISE_XOR_TOKTYPE))
    {
        Token *operator_token = memory_clone_token(parser_previous(parser));
        Expr *right = parser_bit_and(parser);

        BinaryExpr *expr = memory_create_binary_expr(left, operator_token, right);

        left = memory_create_expr(expr, BINARY_EXPR_TYPE);
    }

    return left;
}

Expr *parser_bit_and(Parser *parser)
{
    Expr *left = parser_bit_not(parser);

    while (parser_match(parser, 1, BITWISE_AND_TOKTYPE))
    {
        Token *operator_token = memory_clone_token(parser_previous(parser));
        Expr *right = parser_bit_not(parser);

        BinaryExpr *expr = memory_create_binary_expr(left, operator_token, right);

        left = memory_create_expr(expr, BINARY_EXPR_TYPE);
    }

    return left;
}

Expr *parser_bit_not(Parser *parser)
{
    if (parser_match(parser, 1, BITWISE_NOT_TOKTYPE))
    {
        Token *operator_token = memory_clone_token(parser_previous(parser));
        Expr *right = parser_bit_not(parser);

        UnaryExpr *expr = memory_create_unary_expr(operator_token, right);

        return memory_create_expr(expr, UNARY_EXPR_TYPE);
    }

    return parser_or_expr(parser);
}

Expr *parser_or_expr(Parser *parser)
{
    Expr *left = parser_and_expr(parser);

    while (parser_match(parser, 1, OR_TOKTYPE))
    {
        Token *operator= memory_clone_token(parser_previous(parser));
        Expr *right = parser_and_expr(parser);

        LogicalExpr *expr = memory_create_logical_expr(left, operator, right);

        left = memory_create_expr(expr, LOGICAL_EXPR_TYPE);
    }

    return left;
}

Expr *parser_and_expr(Parser *parser)
{
    Expr *left = parser_comparison_expr(parser);

    while (parser_match(parser, 1, AND_TOKTYPE))
    {
        Token *operator_token = memory_clone_token(parser_previous(parser));
        Expr *right = parser_comparison_expr(parser);

        LogicalExpr *expr = memory_create_logical_expr(left, operator_token, right);

        left = memory_create_expr(expr, LOGICAL_EXPR_TYPE);
    }

    return left;
}

Expr *parser_comparison_expr(Parser *parser)
{
    Expr *left = parser_shift_expr(parser);

    while (parser_match(parser, 6,
                        LESS_TOKTYPE,
                        GREATER_TOKTYPE,
                        LESS_EQUALS_TOKTYPE,
                        GREATER_EQUALS_TOKTYPE,
                        EQUALS_EQUALS_TOKTYPE,
                        NOT_EQUALS_TOKTYPE))
    {
        Token *operator_token = memory_clone_token(parser_previous(parser));
        Expr *right = parser_shift_expr(parser);

        ComparisonExpr *expr = memory_create_comparison_expr(left, operator_token, right);

        left = memory_create_expr(expr, COMPARISON_EXPR_TYPE);
    }

    return left;
}

Expr *parser_shift_expr(Parser *parser)
{
    Expr *left = parser_term_expr(parser);

    while (parser_match(parser, 2, SHIFT_LEFT, SHIFT_RIGHT))
    {
        Token *operator_token = memory_clone_token(parser_previous(parser));
        Expr *right = parser_term_expr(parser);

        BinaryExpr *expr = memory_create_binary_expr(left, operator_token, right);

        left = memory_create_expr(expr, BINARY_EXPR_TYPE);
    }

    return left;
}

Expr *parser_term_expr(Parser *parser)
{
    Expr *left = parser_factor_expr(parser);

    while (parser_match(parser, 2, PLUS_TOKTYPE, MINUS_TOKTYPE))
    {
        Token *operator_token = memory_clone_token(parser_previous(parser));
        Expr *right = parser_factor_expr(parser);

        BinaryExpr *expr = memory_create_binary_expr(left, operator_token, right);

        left = memory_create_expr(expr, BINARY_EXPR_TYPE);
    }

    return left;
}

Expr *parser_factor_expr(Parser *parser)
{
    Expr *left = parser_unary_expr(parser);

    while (parser_match(parser, 3, ASTERISK_TOKTYPE, SLASH_TOKTYPE, PERCENT_TOKTYPE))
    {
        Token *operator_token = memory_clone_token(parser_previous(parser));
        Expr *right = parser_unary_expr(parser);

        BinaryExpr *expr = memory_create_binary_expr(left, operator_token, right);

        left = memory_create_expr(expr, BINARY_EXPR_TYPE);
    }

    return left;
}

Expr *parser_unary_expr(Parser *parser)
{
    if (parser_match(parser, 2, MINUS_TOKTYPE, EXCLAMATION_TOKTYPE))
    {
        Token *operator_token = memory_clone_token(parser_previous(parser));
        Expr *right = parser_unary_expr(parser);

        UnaryExpr *expr = memory_create_unary_expr(operator_token, right);

        return memory_create_expr(expr, UNARY_EXPR_TYPE);
    }

    return parser_access_expr(parser);
}

Expr *parser_access_expr(Parser *parser)
{
    Expr *left = parser_this_expr(parser);

    while (parser_match(parser, 3,
                        LEFT_SQUARE_TOKTYPE,
                        DOT_TOKTYPE,
                        LEFT_PARENTHESIS_TOKTYPE))
    {
        Token *previous = parser_previous(parser);

        switch (previous->type)
        {
        case LEFT_SQUARE_TOKTYPE:
        {
            do
            {
                Expr *index_expr = parser_term_expr(parser);
                Token *left_square_index = memory_clone_token(previous);
                ArrAccessExpr *expr = memory_create_arr_access_expr(left, left_square_index, index_expr);

                left = memory_create_expr(expr, ARR_ACCESS_EXPR_TYPE);

                parser_consume(parser, RIGHT_SQUARE_TOKTYPE, "Expect ']' at end of array access expression.");

            } while (parser_match(parser, 1, LEFT_SQUARE_TOKTYPE));

            break;
        }

        case DOT_TOKTYPE:
        {
            Token *dot_token = memory_clone_token(previous);
            Token *raw_identifier = parser_consume(parser, IDENTIFIER_TOKTYPE, "Expect identifier after '.'.");
            Token *identifier = memory_clone_token(raw_identifier);

            AccessExpr *expr = memory_create_access_expr(left, dot_token, identifier);

            left = memory_create_expr(expr, ACCESS_EXPR_TYPE);

            break;
        }

        case LEFT_PARENTHESIS_TOKTYPE:
        {
            Token *left_parenthesis_token = memory_clone_token(previous);
            DynArrPtr *args = memory_create_dynarr_ptr();

            if (!parser_check(parser, RIGHT_PARENTHESIS_TOKTYPE))
            {
                do
                {
                    Expr *expr = parser_expr(parser);
                    dynarr_ptr_insert((void *)expr, args);
                } while (parser_match(parser, 1, COMMA_TOKTYPE));
            }

            parser_consume(parser, RIGHT_PARENTHESIS_TOKTYPE, "Expect ')' at end of call expression argument list.");

            CallExpr *expr = memory_create_call_expr(left, left_parenthesis_token, args);

            left = memory_create_expr(expr, CALL_EXPR_TYPE);

            break;
        }

        default:
            assert(0 && "Illegal TokenType value");
        }
    }

    return left;
}

Expr *parser_this_expr(Parser *parser)
{
    if (parser_match(parser, 1, THIS_TOKTYPE))
    {
        Token *raw_this_token = parser_previous(parser);
        Token *this_token = memory_clone_token(raw_this_token);
        Token *identifier_token = NULL;

        if (parser_match(parser, 1, DOT_TOKTYPE))
            identifier_token = memory_clone_token(parser_consume(parser, IDENTIFIER_TOKTYPE, "Expect identifier after '.'"));

        ThisExpr *expr = memory_create_this_expr(this_token, identifier_token);

        return memory_create_expr(expr, THIS_EXPR_TYPE);
    }

    return parser_literal_expr(parser);
}

Expr *parser_literal_expr(Parser *parser)
{
    if (parser_match(parser, 1, NIL_TOKTYPE))
    {
        Token *literal_token = memory_clone_token(parser_previous(parser));

        LiteralExpr *expr = memory_create_literal_expr(NULL, 0, literal_token);

        return memory_create_expr(expr, NIL_EXPR_TYPE);
    }

    if (parser_match(parser, 2, TRUE_TOKTYPE, FALSE_TOKTYPE))
    {
        Token *literal_token = memory_clone_token(parser_previous(parser));
        int8_t *literal = (int8_t *)memory_alloc(sizeof(int8_t));

        *literal = literal_token->type == TRUE_TOKTYPE ? 1 : 0;

        LiteralExpr *expr = memory_create_literal_expr((void *)literal, sizeof(int8_t), literal_token);

        return memory_create_expr(expr, BOOL_EXPR_TYPE);
    }

    if (parser_match(parser, 1, INTEGER_TOKTYPE))
    {
        Token *literal_token = memory_clone_token(parser_previous(parser));
        int64_t *literal = (int64_t *)memory_alloc(sizeof(int64_t));

        *literal = *(int64_t *)literal_token->literal;

        LiteralExpr *expr = memory_create_literal_expr(literal, literal_token->literal_size, literal_token);

        return memory_create_expr(expr, INT_EXPR_TYPE);
    }

    if (parser_match(parser, 1, STRING_TOKTYPE))
    {
        Token *raw_literal_token = parser_previous(parser);
        Token *literal_token = memory_clone_token(raw_literal_token);
        char *literal = memory_clone_raw_str((char *)literal_token->literal);

        LiteralExpr *expr = memory_create_literal_expr(literal, literal_token->literal_size, literal_token);

        return memory_create_expr(expr, STR_EXPR_TYPE);
    }

    if (parser_match(parser, 1, IDENTIFIER_TOKTYPE))
    {
        Token *identifier_token = memory_clone_token(parser_previous(parser));
        IdentifierExpr *expr = memory_create_identifier_expr(identifier_token);

        return memory_create_expr(expr, IDENTIFIER_EXPR_TYPE);
    }

    if (parser_match(parser, 1, LEFT_PARENTHESIS_TOKTYPE))
    {
        Token *left_paren_token = memory_clone_token(parser_previous(parser));
        Expr *e = parser_expr(parser);

        parser_consume(parser, RIGHT_PARENTHESIS_TOKTYPE, "Expect ')' at end of group expression.");

        GroupExpr *expr = memory_create_group_expr(left_paren_token, e);

        return memory_create_expr(expr, GROUP_EXPR_TYPE);
    }

    Token *token = parser_peek(parser);

    parser_error_at(parser_peek(parser), "Expected something, but got '%s'", token->lexeme);

    return NULL;
}

Stmt *parser_stmt(Parser *parser)
{
    if (parser_match(parser, 1, CL_TOKTYPE))
        return parser_var_decl_stmt(parser);

    if (parser_match(parser, 1, LEFT_BRACKET_TOKTYPE))
    {
        DynArrPtr *stmts = parser_block_stmt(parser);
        BlockStmt *stmt = memory_create_block_stmt(stmts);

        return memory_create_stmt(stmt, BLOCK_STMT_TYPE);
    }

    if (parser_match(parser, 1, IF_TOKTYPE))
        return parser_if_stmt(parser);

    if (parser_match(parser, 1, CONTINUE_TOKTYPE))
        return parser_continue_stmt(parser);

    if (parser_match(parser, 1, BREAK_TOKTYPE))
        return parser_break_stmt(parser);

    if (parser_match(parser, 1, WHILE_TOKTYPE))
        return parser_while_stmt(parser);

    if (parser_match(parser, 1, FOR_TOKTYPE))
        return parser_for_stmt(parser);

    if (parser_match(parser, 1, PROC_TOKTYPE))
        return parser_fn_stmt(parser);

    if (parser_match(parser, 1, CLASS_TOKTYPE))
        return parser_class_stmt(parser);

    if (parser_match(parser, 1, PRINT_TOKTYPE))
        return parser_print_stmt(parser);

    if (parser_match(parser, 1, RETURN_TOKTYPE))
        return parser_return_stmt(parser);

    return parser_expr_stmt(parser);
}

Stmt *parser_var_decl_stmt(Parser *parser)
{
    Token *raw_identifier = parser_consume(parser, IDENTIFIER_TOKTYPE, "Expect identifier after 'cl' keyword");
    Token *identifier = memory_clone_token(raw_identifier);
    Expr *initializer = NULL;

    if (parser_match(parser, 1, EQUALS_TOKTYPE))
        initializer = parser_expr(parser);

    parser_consume(parser, SEMICOLON_TOKTYPE, "Expect ';' at end of var declaration statement.");

    VarDeclStmt *stmt = memory_create_var_decl_stmt(identifier, initializer);

    return memory_create_stmt(stmt, VAR_DECL_STMT_TYPE);
}

DynArrPtr *parser_block_stmt(Parser *parser)
{
    DynArrPtr *stmts = memory_create_dynarr_ptr();

    while (!parser_check(parser, RIGHT_BRACKET_TOKTYPE))
    {
        Stmt *stmt = parser_stmt(parser);
        dynarr_ptr_insert(stmt, stmts);
    }

    parser_consume(parser, RIGHT_BRACKET_TOKTYPE, "Expect '}' at end of body statement.");

    return stmts;
}

IfStmtBranch *parser_if_branch(Parser *parser)
{
    parser_consume(parser, LEFT_PARENTHESIS_TOKTYPE, "Expect '(' at start of branch condition.");

    Expr *condition = parser_type_expr(parser);

    parser_consume(parser, RIGHT_PARENTHESIS_TOKTYPE, "Expect ')' at end of branch condition.");
    parser_consume(parser, LEFT_BRACKET_TOKTYPE, "Expect '{' at start of branch body.");

    DynArrPtr *stmts = parser_block_stmt(parser);

    return memory_create_if_stmt_branch(condition, stmts);
}

Stmt *parser_if_stmt(Parser *parser)
{
    IfStmtBranch *if_branch = parser_if_branch(parser);
    DynArrPtr *elif_branches = NULL;
    DynArrPtr *else_stmts = NULL;

    if (parser_check(parser, ELIF_TOKTYPE))
    {
        elif_branches = memory_create_dynarr_ptr();

        while (parser_match(parser, 1, ELIF_TOKTYPE))
            dynarr_ptr_insert((void *)parser_if_branch(parser), elif_branches);
    }

    if (parser_match(parser, 1, ELSE_TOKTYPE))
    {
        parser_consume(parser, LEFT_BRACKET_TOKTYPE, "Expect '{' at start of else block.");
        else_stmts = parser_block_stmt(parser);
    }

    IfStmt *stmt = memory_create_if_stmt(if_branch, elif_branches, else_stmts);

    return memory_create_stmt(stmt, IF_STMT_TYPE);
}

Stmt *parser_continue_stmt(Parser *parser)
{
    Token *continue_token = memory_clone_token(parser_previous(parser));

    parser_consume(parser, SEMICOLON_TOKTYPE, "Expect ';' at end of continue statement.");

    ContinueStmt *stmt = memory_create_continue_stmt(continue_token);

    return memory_create_stmt(stmt, CONTINUE_STMT_TYPE);
}

Stmt *parser_break_stmt(Parser *parser)
{
    Token *break_token = memory_clone_token(parser_previous(parser));

    parser_consume(parser, SEMICOLON_TOKTYPE, "Expect ';' at end of break statement.");

    BreakStmt *stmt = memory_create_break_stmt(break_token);

    return memory_create_stmt(stmt, BREAK_STMT_TYPE);
}

Stmt *parser_while_stmt(Parser *parser)
{
    Expr *condition = NULL;
    DynArrPtr *stmts = NULL;

    parser_consume(parser, LEFT_PARENTHESIS_TOKTYPE, "Expect '(' after 'while' keyword.");

    condition = parser_type_expr(parser);

    parser_consume(parser, RIGHT_PARENTHESIS_TOKTYPE, "Expect ')' at end of while statement condition.");
    parser_consume(parser, LEFT_BRACKET_TOKTYPE, "Expect '{' at start of while body.");

    stmts = parser_block_stmt(parser);

    WhileStmt *stmt = memory_create_while_stmt(condition, stmts);

    return memory_create_stmt(stmt, WHILE_STMT_TYPE);
}

DynArrPtr *fn_params(Parser *parser)
{
    DynArrPtr *params = memory_create_dynarr_ptr();

    do
    {
        Token *raw_param_token = parser_consume(parser, IDENTIFIER_TOKTYPE, "Expect identifier as parameter.");
        Token *param_token = memory_clone_token(raw_param_token);

        dynarr_ptr_insert((void *)param_token, params);
    } while (parser_match(parser, 1, COMMA_TOKTYPE));

    return params;
}

Stmt *parser_for_stmt(Parser *parser)
{
    Token *identifier_token = NULL;
    Expr *left_expr = NULL;
    Token *operator_token = NULL;
    Expr *right_expr = NULL;
    DynArrPtr *stmts = NULL;

    parser_consume(parser, LEFT_PARENTHESIS_TOKTYPE, "Expect '(' at start of for header.");

    identifier_token = parser_consume(parser, IDENTIFIER_TOKTYPE, "Expect identifier.");
    identifier_token = memory_clone_token(identifier_token);

    parser_consume(parser, IN_TOKTYPE, "Expect 'in' keyword after identifier.");
    left_expr = parser_term_expr(parser);

    if (parser_match(parser, 2, DOWN_TOKTYPE, UP_TOKTYPE))
        operator_token = memory_clone_token(parser_previous(parser));

    if (!operator_token)
        parser_error_at(parser_peek(parser), "Expect 'down' or 'up', but got something else.");

    right_expr = parser_term_expr(parser);
    parser_consume(parser, RIGHT_PARENTHESIS_TOKTYPE, "Expect ')' at end of for header.");

    parser_consume(parser, LEFT_BRACKET_TOKTYPE, "Expect '{' at start of for body.");
    stmts = parser_block_stmt(parser);

    ForStmt *stmt = memory_create_for_stmt(identifier_token, left_expr, operator_token, right_expr, stmts);

    return memory_create_stmt(stmt, FOR_STMT_TYPE);
}

Stmt *parser_fn_stmt(Parser *parser)
{
    Token *raw_identifier = parser_consume(parser, IDENTIFIER_TOKTYPE, "Expect identifier after 'proc' keyword.");
    Token *identifier = memory_clone_token(raw_identifier);
    DynArrPtr *params = memory_create_dynarr_ptr();
    DynArrPtr *stmts = NULL;

    parser_consume(parser, LEFT_PARENTHESIS_TOKTYPE, "Expect '(' after function name.");

    if (!parser_check(parser, RIGHT_PARENTHESIS_TOKTYPE))
        params = fn_params(parser);

    parser_consume(parser, RIGHT_PARENTHESIS_TOKTYPE, "Expect ')' at end of function parameter list.");
    parser_consume(parser, LEFT_BRACKET_TOKTYPE, "Expect '{' at start of function body.");

    stmts = parser_block_stmt(parser);
    FnStmt *stmt = memory_create_fn_stmt(identifier, params, stmts);

    return memory_create_stmt(stmt, FN_STMT_TYPE);
}

FnStmt *class_constructor(Parser *parser, Token *init_token)
{
    DynArrPtr *params = NULL;
    DynArrPtr *body = NULL;

    parser_consume(parser, LEFT_PARENTHESIS_TOKTYPE, "Expect '(' after 'init' keyword.");

    if (!parser_check(parser, RIGHT_PARENTHESIS_TOKTYPE))
        params = fn_params(parser);

    parser_consume(parser, RIGHT_PARENTHESIS_TOKTYPE, "Expect '(' after 'init' keyword.");
    parser_consume(parser, LEFT_BRACKET_TOKTYPE, "Expect '{' at start of init body.");

    body = parser_block_stmt(parser);

    return memory_create_fn_stmt(init_token, params, body);
}

Stmt *parser_class_stmt(Parser *parser)
{
    Token *raw_identifier = parser_consume(parser, IDENTIFIER_TOKTYPE, "Expect class name after 'class' keyword.");
    Token *identifier = memory_clone_token(raw_identifier);
    DynArrPtr *attributes = memory_create_dynarr_ptr();
    FnStmt *constructor = NULL;
    DynArrPtr *methods = memory_create_dynarr_ptr();

    parser_consume(parser, LEFT_BRACKET_TOKTYPE, "Expect '{' at start of class body.");

    if (!parser_check(parser, RIGHT_BRACKET_TOKTYPE))
    {
        while (parser_match(parser, 2, PROC_TOKTYPE, INIT_TOKTYPE))
        {
            Token *previous_token = parser_previous(parser);

            if (previous_token->type == INIT_TOKTYPE)
            {
                if (constructor)
                    parser_error_at(raw_identifier, "Classes can't have more than one constructor.");

                constructor = class_constructor(parser, previous_token);

                continue;
            }

            dynarr_ptr_insert((void *)parser_fn_stmt(parser), methods);
        }
    }

    parser_consume(parser, RIGHT_BRACKET_TOKTYPE, "Expect '}' at end of class body.");

    ClassStmt *stmt = memory_create_class_stmt(identifier, attributes, constructor, methods);

    return memory_create_stmt(stmt, CLASS_STMT_TYPE);
}

Stmt *parser_print_stmt(Parser *parser)
{
    Token *print_token = memory_clone_token(parser_previous(parser));
    Expr *expr = parser_expr(parser);
    PrintStmt *stmt = memory_create_print_stmt(expr, print_token);

    parser_consume(parser, SEMICOLON_TOKTYPE, "Expect ';' at end of print statement");

    return memory_create_stmt(stmt, PRINT_STMT_TYPE);
}

Stmt *parser_return_stmt(Parser *parser)
{
    Expr *value = NULL;
    Token *return_token = memory_clone_token(parser_previous(parser));

    if (!parser_check(parser, SEMICOLON_TOKTYPE))
        value = parser_expr(parser);

    parser_consume(parser, SEMICOLON_TOKTYPE, "Expect ';' at end of return statement.");

    ReturnStmt *stmt = memory_create_return_stmt(value, return_token);

    return memory_create_stmt(stmt, RETURN_STMT_TYPE);
}

Stmt *parser_expr_stmt(Parser *parser)
{
    Expr *expr = parser_expr(parser);

    Token *token = parser_peek(parser);

    parser_consume(parser, SEMICOLON_TOKTYPE, "Expect ';' at end of expression statement, but got '%s'.", token->lexeme);

    ExprStmt *stmt = memory_create_expr_stmt(expr);

    return memory_create_stmt(stmt, EXPR_STMT_TYPE);
}

// public implementation
void parser_parse(Parser *parser)
{
    while (!parser_is_at_end(parser))
        dynarr_ptr_insert(parser_stmt(parser), parser->stmts);
}