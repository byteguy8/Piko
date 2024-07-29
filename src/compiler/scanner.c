#include "token.h"
#include "scanner.h"
#include "memory.h"
#include "error_report.h"

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

void scanner_error_at(Scanner *scanner, char *msg, ...);
void scanner_add_keyword(char *keyword, TokenType type, LZHTable *table);
void scanner_clear_keywords(LZHTable *table);

int scanner_is_at_end(Scanner *scanner);
int scanner_is_digit(char c);
int scanner_is_alpha(char c);
int scanner_is_alpha_numeric(char c);

char scanner_advance(Scanner *scanner);
char scanner_peek(Scanner *scanner);
int scanner_match(char c, Scanner *scanner);

char *scanner_lexeme_range(int from, int to, Scanner *scanner);
char *scanner_lexeme(Scanner *scanner);
void scanner_add_token_literal(void *literal, size_t literal_size, TokenType type, Scanner *scanner);
void scanner_add_token(TokenType type, Scanner *scanner);

int64_t scanner_lexeme_to_i64(char *lexeme, Scanner *scanner);
void scanner_number(Scanner *scanner);
void scanner_string(Scanner *scanner);
void scanner_identifier(Scanner *scanner);

void scanner_scan_token(Scanner *scanner);

// private implementation
void scanner_error_at(Scanner *scanner, char *msg, ...)
{
    va_list args;
    va_start(args, msg);

    report_error_at(11, scanner->line, "Scanner", msg, args);

    va_end(args);
}

void scanner_add_keyword(char *keyword, TokenType type, LZHTable *table)
{
    TokenType *mem_type = (TokenType *)memory_alloc(sizeof(TokenType));

    *mem_type = type;

    lzhtable_put((uint8_t *)keyword, strlen(keyword), mem_type, table, NULL);
}

void scanner_clear_keywords(LZHTable *table)
{
    LZHTableNode *node = table->nodes;

    while (node)
    {
        LZHTableNode *before = node->previous_table_node;
        memory_dealloc(node->value);
        node = before;
    }
}

int scanner_is_at_end(Scanner *scanner)
{
    return (size_t)scanner->current >= scanner->source->len;
}

int scanner_is_digit(char c)
{
    return c >= '0' && c <= '9';
}

int scanner_is_alpha(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

int scanner_is_alpha_numeric(char c)
{
    return scanner_is_alpha(c) || scanner_is_digit(c) || c == '_';
}

char scanner_advance(Scanner *scanner)
{
    char *raw_source = scanner->source->raw;
    return raw_source[scanner->current++];
}

char scanner_peek(Scanner *scanner)
{
    if (scanner_is_at_end(scanner))
        return '\0';

    char *raw_source = scanner->source->raw;

    return raw_source[scanner->current];
}

int scanner_match(char c, Scanner *scanner)
{
    char source_c = scanner_peek(scanner);

    if (source_c != c)
        return 0;

    scanner_advance(scanner);

    return 1;
}

char *scanner_lexeme_range(int from, int to, Scanner *scanner)
{
    char *raw_source = scanner->source->raw;

    size_t len = to - from;
    char *lexeme = (char *)memory_alloc(len + 1);

    memcpy(lexeme, raw_source + from, len);
    lexeme[len] = 0;

    return lexeme;
}

char *scanner_lexeme(Scanner *scanner)
{
    return scanner_lexeme_range(scanner->start, scanner->current, scanner);
}

void scanner_add_token_literal(void *literal, size_t literal_size, TokenType type, Scanner *scanner)
{
    char *lexeme = scanner_lexeme(scanner);

    Token token = {0};

    token.line = scanner->line;
    token.lexeme = lexeme;
    token.literal = literal;
    token.literal_size = literal_size;
    token.type = type;

    dynarr_insert((void *)&token, scanner->tokens);
}

void scanner_add_token(TokenType type, Scanner *scanner)
{
    scanner_add_token_literal(NULL, 0, type, scanner);
}

int64_t scanner_lexeme_to_i64(char *lexeme, Scanner *scanner)
{
    int64_t value = 0;
    size_t len = strlen(lexeme);

    for (size_t i = 0; i < len; i++)
    {
        char c = lexeme[i];

        value *= 10;
        value += c - 48;
    }

    return value;
}

void scanner_number(Scanner *scanner)
{
    while (!scanner_is_at_end(scanner) && scanner_is_digit(scanner_peek(scanner)))
        scanner_advance(scanner);

    char *lexeme = scanner_lexeme(scanner);

    size_t value_size = sizeof(int64_t);
    int64_t *value = (int64_t *)memory_alloc(value_size);

    *value = scanner_lexeme_to_i64(lexeme, scanner);

    memory_destroy_raw_str(lexeme);

    scanner_add_token_literal((void *)value, value_size, INTEGER_TOKTYPE, scanner);
}

void scanner_string(Scanner *scanner)
{
    int line = scanner->line;

    while (!scanner_is_at_end(scanner) && scanner_peek(scanner) != '"')
        if (scanner_advance(scanner) == '\n')
            line++;

    if (scanner_peek(scanner) != '"')
        scanner_error_at(scanner, "Unterminated string. Expect '\"' at end of string");

    scanner_advance(scanner);

    char *literal = scanner_lexeme_range(scanner->start + 1, scanner->current - 1, scanner);
    size_t len = strlen(literal);

    scanner_add_token_literal((void *)literal, len, STRING_TOKTYPE, scanner);

    scanner->line = line;
}

void scanner_identifier(Scanner *scanner)
{
    while (!scanner_is_at_end(scanner) && scanner_is_alpha_numeric(scanner_peek(scanner)))
        scanner_advance(scanner);

    char *lexeme = scanner_lexeme(scanner);
    size_t len = strlen(lexeme);

    TokenType *type = lzhtable_get((uint8_t *)lexeme, len, scanner->keywords);

    if (type)
        scanner_add_token(*type, scanner);
    else
        scanner_add_token(IDENTIFIER_TOKTYPE, scanner);

    memory_destroy_raw_str(lexeme);
}

void scanner_scan_token(Scanner *scanner)
{
    char c = scanner_advance(scanner);

    switch (c)
    {
    case '+':
        scanner_add_token(PLUS_TOKTYPE, scanner);
        break;

    case '-':
        scanner_add_token(MINUS_TOKTYPE, scanner);
        break;

    case '*':
        scanner_add_token(ASTERISK_TOKTYPE, scanner);
        break;

    case '/':
        scanner_add_token(SLASH_TOKTYPE, scanner);
        break;

    case '%':
        scanner_add_token(PERCENT_TOKTYPE, scanner);
        break;

    case '.':
        scanner_add_token(DOT_TOKTYPE, scanner);
        break;

    case '(':
        scanner_add_token(LEFT_PARENTHESIS_TOKTYPE, scanner);
        break;

    case ')':
        scanner_add_token(RIGHT_PARENTHESIS_TOKTYPE, scanner);
        break;

    case '[':
        scanner_add_token(LEFT_SQUARE_TOKTYPE, scanner);
        break;

    case ']':
        scanner_add_token(RIGHT_SQUARE_TOKTYPE, scanner);
        break;

    case '{':
        scanner_add_token(LEFT_BRACKET_TOKTYPE, scanner);
        break;

    case '}':
        scanner_add_token(RIGHT_BRACKET_TOKTYPE, scanner);
        break;

    case ',':
        scanner_add_token(COMMA_TOKTYPE, scanner);
        break;

    case ':':
        scanner_add_token(COLON_TOKTYPE, scanner);
        break;

    case ';':
        scanner_add_token(SEMICOLON_TOKTYPE, scanner);
        break;

    case '|':
        if (scanner_match('|', scanner))
            scanner_add_token(OR_TOKTYPE, scanner);
        break;

    case '&':
        if (scanner_match('&', scanner))
            scanner_add_token(AND_TOKTYPE, scanner);
        break;

    case '<':
        if (scanner_match('=', scanner))
            scanner_add_token(LESS_EQUALS_TOKTYPE, scanner);
        else
            scanner_add_token(LESS_TOKTYPE, scanner);
        break;

    case '>':
        if (scanner_match('=', scanner))
            scanner_add_token(GREATER_EQUALS_TOKTYPE, scanner);
        else
            scanner_add_token(GREATER_TOKTYPE, scanner);
        break;

    case '=':
        if (scanner_match('=', scanner))
            scanner_add_token(EQUALS_EQUALS_TOKTYPE, scanner);
        else
            scanner_add_token(EQUALS_TOKTYPE, scanner);
        break;

    case '!':
        if (scanner_match('=', scanner))
            scanner_add_token(NOT_EQUALS_TOKTYPE, scanner);
        else
            scanner_add_token(EXCLAMATION_TOKTYPE, scanner);
        break;

    case '\n':
        scanner->line++;
        break;

    case ' ':
    case '\t':
        break;

    default:
        if (scanner_is_digit(c))
            scanner_number(scanner);
        else if (c == '"')
            scanner_string(scanner);
        else if (scanner_is_alpha_numeric(c))
            scanner_identifier(scanner);
        else
            scanner_error_at(scanner, "Unknown token '%c'", c);

        break;
    }
}

// public implementation
void scanner_print_tokens(DynArr *tokens)
{
    for (size_t i = 0; i < tokens->used; i++)
    {
        Token *token = (Token *)dynarr_get(i, tokens);
        printf("token %32.32s\tat line %7d\n", token->lexeme, token->line + 1);
    }
}

int scanner_scan_tokens(Scanner *scanner)
{
    LZHTable *keywords = scanner->keywords;

    scanner_add_keyword("nil", NIL_TOKTYPE, keywords);
    scanner_add_keyword("true", TRUE_TOKTYPE, keywords);
    scanner_add_keyword("false", FALSE_TOKTYPE, keywords);
    scanner_add_keyword("cl", CL_TOKTYPE, keywords);
    scanner_add_keyword("if", IF_TOKTYPE, keywords);
    scanner_add_keyword("elif", ELIF_TOKTYPE, keywords);
    scanner_add_keyword("else", ELSE_TOKTYPE, keywords);
    scanner_add_keyword("while", WHILE_TOKTYPE, keywords);
    scanner_add_keyword("break", BREAK_TOKTYPE, keywords);
    scanner_add_keyword("continue", CONTINUE_TOKTYPE, keywords);
    scanner_add_keyword("print", PRINT_TOKTYPE, keywords);
    scanner_add_keyword("proc", PROC_TOKTYPE, keywords);
    scanner_add_keyword("ret", RETURN_TOKTYPE, keywords);

    scanner_add_keyword("is", IS_TOKTYPE, keywords);
    scanner_add_keyword("from", FROM_TOKTYPE, keywords);
    scanner_add_keyword("bool", BOOL_TOKTYPE, keywords);
    scanner_add_keyword("int", INT_TOKTYPE, keywords);
    scanner_add_keyword("str", STR_TOKTYPE, keywords);
    scanner_add_keyword("arr", ARR_TOKTYPE, keywords);
    scanner_add_keyword("instance", INSTANCE_TOKTYPE, keywords);

    scanner_add_keyword("klass", CLASS_TOKTYPE, keywords);
    scanner_add_keyword("init", INIT_TOKTYPE, keywords);
    scanner_add_keyword("this", THIS_TOKTYPE, keywords);

    while (!scanner_is_at_end(scanner))
    {
        scanner_scan_token(scanner);
        scanner->start = scanner->current;
    }

    scanner_add_token(EOF_TOKTYPE, scanner);

    scanner_clear_keywords(keywords);

    return 0;
}