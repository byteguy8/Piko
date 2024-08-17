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

void dumpper_execute_raw_instruction(uint8_t instruction)
{
    switch (instruction)
    {
    case NIL_OPC:
    {
        printf("NIL\n");

        break;
    }

    case BCONST_OPC:
    {
        uint8_t value = dumpper_read_bool_const();

        printf("BOOL %s\n", value ? "true" : "false");

        break;
    }

    case ICONST_OPC:
    {
        int64_t value = dummper_read_i64_const();

        printf("INT %ld\n", value);

        break;
    }

    case SCONST_OPC:
    {
        char *value = dumpper_read_str_const();

        printf("STR '%s'\n", value);

        break;
    }

    case ARR_OPC:
    {
        uint8_t is_empty = dumpper_advance();

        printf("VARR empty: %s\n", is_empty ? "true" : "false");

        break;
    }

    case ARR_LEN_OPC:
    {
        printf("ARR_LEN\n");

        break;
    }

    case ARR_ITM_OPC:
    {
        printf("ARR_ITM\n");

        break;
    }

    case ARR_SITM_OPC:
    {
        printf("ARR_SITM\n");

        break;
    }

    case LREAD_OPC:
    {
        int32_t local_index = dumpper_advance();

        printf("LREAD %d\n", local_index);

        break;
    }

    case LSET_OPC:
    {
        int32_t local_index = dumpper_advance();

        printf("LSET %d\n", local_index);

        break;
    }

    case GWRITE_OPC:
    {
        char *global_name = dumpper_read_str_const();

        printf("GWRITE %s\n", global_name);

        break;
    }

    case GREAD_OPC:
    {
        char *global_name = dumpper_read_str_const();

        printf("GREAD %s\n", global_name);

        break;
    }

    case LOAD_OPC:
    {
        int32_t entity_index = dumpper_read_i32();
        Entity *symbol = (Entity *)dynarr_get((size_t)entity_index, DUMPPER_VM->entities);

        if (symbol->type == FUNCTION_SYMTYPE)
        {
            Fn *fn = (Fn *)symbol->raw_symbol;

            printf("LOAD function %d '%s' %d\n", entity_index, fn->name, entity_index);
        }

        if (symbol->type == NATIVE_SYMTYPE)
        {
            NativeFn *native_fn = (NativeFn *)symbol->raw_symbol;

            printf("LOAD Native function %d '%s' %d\n", entity_index, native_fn->name, entity_index);
        }

        if (symbol->type == CLASS_SYMTYPE)
        {
            Klass *klass = (Klass *)symbol->raw_symbol;

            printf("LOAD klass %d '%s' %d\n", entity_index, klass->name, entity_index);
        }

        break;
    }

    // arithmetic
    case ADD_OPC:
    {
        printf("ADD\n");

        break;
    }

    case SUB_OPC:
    {
        printf("SUB\n");

        break;
    }

    case MUL_OPC:
    {
        printf("MUL\n");

        break;
    }

    case DIV_OPC:
    {
        printf("DIV\n");

        break;
    }

    case MOD_OPC:
    {
        printf("MOD\n");

        break;
    }

    // comparison
    case LT_OPC:
    {
        printf("LT\n");

        break;
    }

    case GT_OPC:
    {
        printf("GT\n");

        break;
    }

    case LE_OPC:
    {
        printf("LE\n");

        break;
    }

    case GE_OPC:
    {
        printf("GE\n");

        break;
    }

    case EQ_OPC:
    {
        printf("EQ\n");

        break;
    }

    case NE_OPC:
    {
        printf("NE\n");

        break;
    }

    // logical
    case OR_OPC:
    {
        printf("OR\n");

        break;
    }

    case AND_OPC:
    {
        printf("AND\n");

        break;
    }

    case NOT_OPC:
    {
        printf("NOT\n");

        break;
    }

    case NNOT_OPC:
    {
        printf("NNOT\n");

        break;
    }

    // shift
    case SLEFT_OPC:
    {
        printf("SLEFT\n");

        break;
    }

    case SRIGHT_OPC:
    {
        printf("SRIGHT\n");

        break;
    }

    // bitwise
    case BOR_OPC:
    {
        printf("BOR\n");

        break;
    }

    case BXOR_OPC:
    {
        printf("BXOR\n");

        break;
    }

    case BAND_OPC:
    {
        printf("BAND\n");

        break;
    }

    case BNOT_OPC:
    {
        printf("BNOT\n");

        break;
    }

    // control flow
    case JMP_OPC:
    {
        int32_t value = dumpper_read_i32();

        printf("JMP %d\n", value);

        break;
    }

    case JIT_OPC:
    {
        int32_t value = dumpper_read_i32();

        printf("JIT %d\n", value);

        break;
    }

    case JIF_OPC:
    {
        int32_t value = dumpper_read_i32();

        printf("JIF %d\n", value);

        break;
    }

    case CONCAT_OPC:
    {
        printf("CONCAT\n");

        break;
    }

    case STR_LEN_OPC:
    {
        printf("STR_LEN\n");

        break;
    }

    case STR_ITM_OPC:
    {
        printf("STR_ITM\n");

        break;
    }

    case CLASS_OPC:
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

        break;
    }

    case GET_PROPERTY_OPC:
    {
        char *property = dumpper_read_str_const();

        printf("READ_PROPERTY %s\n", property);

        break;
    }

    case IS_OPC:
    {
        uint8_t type = dumpper_advance();

        printf("IS %d\n", type);

        break;
    }

    case FROM_OPC:
    {
        char *klass_name = dumpper_read_str_const();

        printf("FROM '%s'\n", klass_name);

        break;
    }

    case SET_PROPERTY_OPC:
    {
        char *property = dumpper_read_str_const();

        printf("SET_PROPERTY %s\n", property);

        break;
    }

    case THIS_OPC:
    {
        printf("THIS\n");

        break;
    }

    case PRT_OPC:
    {
        printf("PRT\n");

        break;
    }

    case POP_OPC:
    {
        printf("POP\n");

        break;
    }

    case CALL_OPC:
    {
        uint8_t args_count = dumpper_advance();

        printf("CALL args_count: %d\n", args_count);

        break;
    }

    case GBG_OPC:
    {
        printf("GBG\n");

        break;
    }

    case RET_OPC:
    {
        printf("RET\n");

        break;
    }

    case HLT_OPC:
    {
        printf("HLT\n");

        break;
    }

    default:
        assert(0 && "Illegal OPC");
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

            printf("//> %s %ld\n", fn->name, i);

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