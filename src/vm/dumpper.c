#include "dummper.h"
#include "vm_memory.h"

static Dumpper dumpper = {0};

// private interface
DumpperScope *dumpper_scope_current();
uint8_t dumpper_advance();
void dumpper_scope_up(DynArr *chunks);
void dumpper_scope_down();

int32_t dumpper_compose_i32(uint8_t *bytes);
int32_t dumpper_read_i32();
uint8_t dumpper_read_bool_const();
int64_t dummper_read_i64_const();
char *dumpper_read_str_const();

void dumpper_execute_nil();
void dumpper_execute_bool();
void dumpper_execute_int();
void dumpper_execute_string();
void dumpper_execute_object_array();
void dumpper_execute_array_length();
void dumpper_execute_get_array_item();
void dumpper_execute_set_array_item();
void dumpper_execute_arithmetic(int type);
void dumpper_execute_comparison(int type);
void dumpper_execute_argjmp(int type);
void dumpper_execute_logical(int type);
void dumpper_execute_negation(int type);
void dumpper_execute_concat();
void dumpper_execute_str_len();
void dumpper_execute_str_itm();
void dumpper_execute_klass();
void dumpper_execute_get_property();
void dumpper_execute_set_property();
void dumpper_execute_is();
void dumpper_execute_from();
void dumpper_execute_this();
void dumpper_execute_jmp();
void dumpper_execute_create_local();
void dumpper_execute_get_local();
void dumpper_execute_set_local();
void dumpper_execute_set_global();
void dumpper_execute_get_global();
void dumpper_execute_load_function();
void dumpper_execute_print();
void dumpper_execute_pop();
void dumpper_execute_call();
void dumpper_execute_garbage();
void dumpper_execute_return();
void dumpper_execute_raw_instruction(uint8_t instruction);
void dumpper_execute_instruction();
void dumpper_execute_chunks(DynArr *chunks);

void dumpper_dump_functions(VM *vm);

#define DUMPPER_VM dumpper.vm

// private implementation
DumpperScope *dumpper_scope_current()
{
    DumpperScope *scope = &dumpper.scopes[dumpper.scope_ptr];

    if (!scope->chunks)
        assert(0 && "No valid chunks");

    return scope;
}

uint8_t dumpper_advance()
{
    DumpperScope *scope = dumpper_scope_current();
    return *(uint8_t *)dynarr_get(scope->ip++, scope->chunks);
}

void dumpper_scope_up(DynArr *chunks)
{
    if (dumpper.scope_ptr + 1 >= DUMPPER_SCOPE_LENGTH)
        assert(0 && "StackOverFlow");

    DumpperScope *scope = &dumpper.scopes[++dumpper.scope_ptr];

    scope->ip = 0;
    scope->chunks = chunks;
}

void dumpper_scope_down()
{
    if (dumpper.scope_ptr - 1 < 0)
        assert(0 && "StackUnderFlow");

    DumpperScope *current_scope = dumpper_scope_current();

    current_scope->ip = 0;
    current_scope->chunks = NULL;

    dumpper.scope_ptr -= 1;

    printf("\n");
}

int32_t dumpper_compose_i32(uint8_t *bytes)
{
    return ((int32_t)bytes[3] << 24) | ((int32_t)bytes[2] << 16) | ((int32_t)bytes[1] << 8) | ((int32_t)bytes[0]);
}

int32_t dumpper_read_i32()
{
    uint8_t bytes[4];

    for (size_t i = 0; i < 4; i++)
        bytes[i] = dumpper_advance();

    return dumpper_compose_i32(bytes);
}

uint8_t dumpper_read_bool_const()
{
    return dumpper_advance();
}

int64_t dummper_read_i64_const()
{
    int32_t index = dumpper_read_i32();
    return *(int64_t *)dynarr_get((size_t)index, DUMPPER_VM->iconsts);
}

char *dumpper_read_str_const()
{
    int32_t index = dumpper_read_i32();
    return (char *)DYNARR_PTR_GET((size_t)index, DUMPPER_VM->strings);
}

void dumpper_execute_nil()
{
    printf("NIL\n");
}

void dumpper_execute_bool()
{
    uint8_t value = dumpper_read_bool_const();
    printf("BOOL %s\n", value ? "true" : "false");
}

void dumpper_execute_int()
{
    int64_t value = dummper_read_i64_const();
    printf("INT %ld\n", value);
}

void dumpper_execute_string()
{
    char *value = dumpper_read_str_const();
    printf("STR '%s'\n", value);
}

void dumpper_execute_object_array()
{
    uint8_t empty = dumpper_advance();
    printf("VARR empty: %s\n", empty ? "true" : "false");
}

void dumpper_execute_array_length()
{
    printf("ARR_LEN\n");
}

void dumpper_execute_get_array_item()
{
    printf("ARR_ITM\n");
}

void dumpper_execute_set_array_item()
{
    printf("ARR_SITM\n");
}

void dumpper_execute_arithmetic(int type)
{
    printf("BINARY ");

    switch (type)
    {
    // addition
    case 1:
        printf("+\n");
        break;

    // subtraction
    case 2:
        printf("-\n");
        break;

    // mutiplication
    case 3:
        printf("*\n");
        break;

    // division
    case 4:
        printf("/\n");
        break;

    // module
    case 5:
        printf("MOD\n");
        break;

    default:
        assert(0 && "Illegal arithmetic operation type value");
    }
}

void dumpper_execute_comparison(int type)
{
    printf("COMPARISON\n");
}

void dumpper_execute_argjmp(int type)
{
    int32_t value = dumpper_read_i32();

    switch (type)
    {
    case 1:
        printf("JIT %d\n", value);
        break;

    case 2:
        printf("JIF %d\n", value);
        break;

    default:
        break;
    }
}

void dumpper_execute_logical(int type)
{
    printf("LOGICAL\n");
}

void dumpper_execute_negation(int type)
{
    printf("NOT\n");
}

void dumpper_execute_concat()
{
    printf("CONCAT\n");
}

void dumpper_execute_str_len()
{
    printf("STR_LEN\n");
}

void dumpper_execute_str_itm()
{
    printf("STR_ITM\n");
}

void dumpper_execute_klass()
{
    int32_t index = dumpper_read_i32();
    printf("CLASS %d\n", index);

    DynArr *entities = dumpper.vm->entities;
    Entity *entity = dynarr_get((size_t)index, entities);
    Klass *klass = (Klass *)entity->raw_symbol;
    LZHTable *methods = klass->methods;

    LZHTableNode *node = methods->nodes;
    Fn *fn = (Fn *)node->value;

    dumpper_scope_up(fn->chunks);
}

void dumpper_execute_get_property()
{
    char *property = dumpper_read_str_const();
    printf("READ_PROPERTY %s\n", property);
}

void dumpper_execute_set_property()
{
    char *property = dumpper_read_str_const();
    printf("SET_PROPERTY %s\n", property);
}

void dumpper_execute_is()
{
    uint8_t type = dumpper_advance();
    printf("IS %d\n", type);
}

void dumpper_execute_from()
{
    char *klass_name = dumpper_read_str_const();
    printf("FROM '%s'\n", klass_name);
}

void dumpper_execute_this()
{
    printf("THIS\n");
}

void dumpper_execute_jmp()
{
    int32_t value = dumpper_read_i32();
    printf("JMP %d\n", value);
}

void dumpper_execute_jmpc()
{
    int32_t value = dumpper_read_i32();
    printf("JMPC %d\n", value);
}

void dumpper_execute_create_local()
{
    printf("LCREATE\n");
}

void dumpper_execute_get_local()
{
    int32_t value = dumpper_advance();
    printf("LREAD %d\n", value);
}

void dumpper_execute_set_local()
{
    int32_t value = dumpper_advance();
    printf("LSET %d\n", value);
}

void dumpper_execute_set_global()
{
    char *value = dumpper_read_str_const();
    printf("GWRITE %s\n", value);
}

void dumpper_execute_get_global()
{
    char *value = dumpper_read_str_const();
    printf("GREAD %s\n", value);
}

void dumpper_execute_load_function()
{
    int32_t entity_index = dumpper_read_i32();
    Entity *symbol = (Entity *)dynarr_get((size_t)entity_index, DUMPPER_VM->entities);

    if (symbol->type == FUNCTION_SYMTYPE)
    {
        Fn *fn = (Fn *)symbol->raw_symbol;
        printf("LOAD function %d '%s'\n", entity_index, fn->name);
    }

    if (symbol->type == NATIVE_SYMTYPE)
    {
        NativeFn *native_fn = (NativeFn *)symbol->raw_symbol;
        printf("LOAD Native function %d '%s'\n", entity_index, native_fn->name);
    }

    if (symbol->type == CLASS_SYMTYPE)
    {
        Klass *klass = (Klass *)symbol->raw_symbol;
        printf("LOAD klass %d '%s'\n", entity_index, klass->name);
    }
}

void dumpper_execute_print()
{
    printf("PRT\n");
}

void dumpper_execute_pop()
{
    printf("POP\n");
}

void dumpper_execute_call()
{
    uint8_t args_count = dumpper_advance();
    printf("CALL args_count: %d\n", args_count);
}

void dumpper_execute_garbage()
{
    printf("GBG\n");
}

void dumpper_execute_return()
{
    printf("RET\n");
}

void dumpper_execute_raw_instruction(uint8_t instruction)
{
    switch (instruction)
    {
    case NIL_OPC:
        dumpper_execute_nil();
        break;

    case BCONST_OPC:
        dumpper_execute_bool();
        break;

    case ICONST_OPC:
        dumpper_execute_int();
        break;

    case SCONST_OPC:
        dumpper_execute_string();
        break;

    case OARR_OPC:
        dumpper_execute_object_array();
        break;

    case ARR_LEN_OPC:
        dumpper_execute_array_length();
        break;

    case ARR_ITM_OPC:
        dumpper_execute_get_array_item();
        break;

    case ARR_SITM_OPC:
        dumpper_execute_set_array_item();
        break;

    case LREAD_OPC:
        dumpper_execute_get_local();
        break;

    case LSET_OPC:
        dumpper_execute_set_local();
        break;

    case GWRITE_OPC:
        dumpper_execute_set_global();
        break;

    case GREAD_OPC:
        dumpper_execute_get_global();
        break;

    case LOAD_OPC:
        dumpper_execute_load_function();
        break;

        // arithmetic
    case ADD_OPC:
        dumpper_execute_arithmetic(1);
        break;

    case SUB_OPC:
        dumpper_execute_arithmetic(2);
        break;

    case MUL_OPC:
        dumpper_execute_arithmetic(3);
        break;

    case DIV_OPC:
        dumpper_execute_arithmetic(4);
        break;

    case MOD_OPC:
        dumpper_execute_arithmetic(5);
        break;

        // comparison
    case LT_OPC:
        dumpper_execute_comparison(1);
        break;

    case GT_OPC:
        dumpper_execute_comparison(2);
        break;

    case LE_OPC:
        dumpper_execute_comparison(3);
        break;

    case GE_OPC:
        dumpper_execute_comparison(4);
        break;

    case EQ_OPC:
        dumpper_execute_comparison(5);
        break;

    case NE_OPC:
        dumpper_execute_comparison(6);
        break;

    // conditional jump with arg
    case JIT_OPC:
        dumpper_execute_argjmp(1);
        break;

    case JIF_OPC:
        dumpper_execute_argjmp(2);
        break;

        // logical
    case OR_OPC:
        dumpper_execute_logical(1);
        break;

    case AND_OPC:
        dumpper_execute_logical(2);
        break;

    case NOT_OPC:
        dumpper_execute_negation(1);
        break;

    case NNOT_OPC:
        dumpper_execute_negation(2);
        break;

    case CONCAT_OPC:
        dumpper_execute_concat();
        break;

    case STR_LEN_OPC:
        dumpper_execute_str_len();
        break;

    case STR_ITM_OPC:
        dumpper_execute_str_itm();
        break;

    case CLASS_OPC:
        dumpper_execute_klass();
        break;

    case GET_PROPERTY_OPC:
        dumpper_execute_get_property();
        break;

    case IS_OPC:
        dumpper_execute_is();
        break;

    case FROM_OPC:
        dumpper_execute_from();
        break;

    case SET_PROPERTY_OPC:
        dumpper_execute_set_property();
        break;

    case THIS_OPC:
        dumpper_execute_this();
        break;

    case JMP_OPC:
        dumpper_execute_jmp();
        break;

    case PRT_OPC:
        dumpper_execute_print();
        break;

    case POP_OPC:
        dumpper_execute_pop();
        break;

    case CALL_OPC:
        dumpper_execute_call();
        break;

    case GBG_OPC:
        dumpper_execute_garbage();
        break;

    case RET_OPC:
        dumpper_execute_return();
        break;

    case HLT_OPC:
        break;

    default:
        assert(0 && "Illegal OPC");
        break;
    }
}

void dumpper_execute_instruction()
{
    printf("%.7ld: ", dumpper_scope_current()->ip);

    dumpper_execute_raw_instruction(dumpper_advance());
}

void dumpper_execute_chunks(DynArr *chunks)
{
    dumpper_scope_up(chunks);
    DumpperScope *scope = dumpper_scope_current();

    while (scope->ip < chunks->used)
        dumpper_execute_instruction();

    dumpper_scope_down();
}

void dumpper_dump_functions(VM *vm)
{
    DynArr *entities = vm->entities;

    for (size_t i = 0; i < entities->used; i++)
    {
        Entity *entity = (Entity *)dynarr_get(i, entities);

        if (entity->type == FUNCTION_SYMTYPE)
        {
            Fn *fn = (Fn *)entity->raw_symbol;

            printf("//> %s\n", fn->name);

            dumpper_execute_chunks(fn->chunks);

            if (i + 1 < entities->used)
                printf("\n");
        }
    }
}

// public implementation
void dumpper_execute(VM *vm)
{
    dumpper.vm = vm;
    dumpper.scope_ptr = 0;

    DumpperScope *scope = &dumpper.scopes[0];

    scope->ip = 0;
    scope->chunks = (DynArr *)lzstack_peek(vm->blocks_stack, NULL);

    dumpper_dump_functions(vm);

    while (dumpper_scope_current()->ip < dumpper_scope_current()->chunks->used)
        dumpper_execute_instruction();
}