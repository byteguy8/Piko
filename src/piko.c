#include "compiler/memory.h"

#include "compiler/scanner.h"
#include "compiler/parser.h"
#include "compiler/compiler.h"

#include "vm/vm_memory.h"
#include "vm/vm.h"
#include "vm/dummper.h"

#include <stdio.h>

void clear_tokens(DynArr *tokens)
{
    for (size_t i = 0; i < tokens->used; i++)
    {
        Token *token = (Token *)dynarr_get(i, tokens);

        memory_dealloc(token->lexeme);
        memory_dealloc(token->literal);
    }
}

int main(int argc, char const *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "No input source file\n");
        exit(1);
    }

    memory_init();

    // scanner phase
    StaticStr *source = memory_read_source((char *)argv[1]);
    DynArr *tokens = memory_create_dynarr(sizeof(Token));
    LZHTable *keywords = memory_create_lzhtable(23);
    Scanner *scanner = memory_create_scanner(tokens, source, keywords);

    scanner_scan_tokens(scanner);

    memory_destroy_static_str(source);
    memory_destroy_lzhtable(keywords);
    memory_destroy_scanner(scanner);

    // parser phase
    DynArrPtr *stmts = memory_create_dynarr_ptr();
    Parser *parser = memory_create_parser(tokens, stmts);

    parser_parse(parser);

    clear_tokens(tokens);
    memory_destroy_dynarr(tokens);
    memory_destroy_parser(parser);

    //> compiler phase
    vm_memory_init();
    VM *vm = vm_create();

    compiler_compile(vm, stmts);
    // dumpper_execute(vm);
    int return_code = vm_execute(vm);
    // vm_print_stack(vm);

    vm_destroy(vm);
    vm_memory_deinit();
    memory_deinit();

    return return_code;
}
