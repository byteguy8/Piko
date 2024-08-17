#define VM_VALIDATE_OPCODES

#include "vm.h"
#include "vm_memory.h"

#include <time.h>
#include <unistd.h>
#include <assert.h>

#define VM_VALIDATE_OPCODES

static int is_str_int(char *str, size_t str_len)
{
    if (str_len == 0)
        return 0;

    for (size_t i = 0; i < str_len; i++)
    {
        char c = str[i];

        if (i == 0 && c == '-')
            continue;

        if (c < '0' || c > '9')
            return 0;
    }

    return 1;
}

// private interface

//> native functions
// strings
static void native_fn_ascii(DynArr *args, VM *vm);
static void native_fn_ascii_code(DynArr *args, VM *vm);

static void native_fn_str_sub(DynArr *args, VM *vm);
static void native_fn_str_lower(DynArr *args, VM *vm);
static void native_fn_str_upper(DynArr *args, VM *vm);
static void native_fn_str_title(DynArr *args, VM *vm);
static void native_fn_str_cmp(DynArr *args, VM *vm);
static void native_fn_str_cmp_ic(DynArr *args, VM *vm);

static void native_fn_is_str_int(DynArr *args, VM *vm);
static void native_fn_str_to_int(DynArr *args, VM *vm);
static void native_fn_int_to_str(DynArr *args, VM *vm);

// time related
static void native_fn_time(DynArr *args, VM *vm);
static void native_fn_sleep(DynArr *args, VM *vm);

// I/O related
static void native_fn_read_line(DynArr *args, VM *vm);
static void native_fn_read_file(DynArr *args, VM *vm);

// sytem
static void native_fn_panic(DynArr *args, VM *vm);
static void native_fn_exit(DynArr *args, VM *vm);

void vm_add_native(char *name, char arity, void (*raw_native_fn)(DynArr *args, VM *), VM *vm);
//< native functions

static void _error_(char *msg, ...);
void vm_err(char *msg, ...);

//> garbage collector
void vm_garbage_report(char *msg, ...);

void vm_garbage_string(Object *object);
void vm_garbage_object_array(Object *object);
void vm_garbage_instance(Object *object);
void vm_garbage_object(Object *object);
void vm_garbage_objects(VM *vm);

int vm_gc_sweep_object(Object *object);
void vm_gc_sweep_objects(VM *vm);

int vm_gc_mark_object_array(Object *object);
int vm_gc_mark_instance(Object *object);
int vm_gc_mark_object(Object *object);
int vm_gc_mark_frame_values(Value *values);
int vm_gc_mark_frames(VM *vm);
int vm_gc_mark_stack(VM *vm);
int vm_gc_mark_globals(VM *vm);
void vm_gc_mark_objects(VM *vm);

void vm_gc(VM *vm);
//< garbage collector

//> helpers
int vm_is_value_nil(Value *value);
int vm_is_value_primitive(Value *value, Primitive **out_primitive);
int vm_is_value_bool(Value *value, Primitive **out_primitive);
int vm_is_value_int(Value *value, Primitive **out_primitive);
int vm_is_value_string(Value *value);
int vm_is_value_obj_value(Value *value);
int vm_is_value_array(Value *value);
int vm_is_value_fn(Value *value);
int vm_is_value_native_fn(Value *value);
int vm_is_value_method(Value *value);
int vm_is_value_klass(Value *value);
int vm_is_value_callable(Value *value);
int vm_is_value_instance(Value *value);

Object *vm_create_method(Object *instance, Fn *function, VM *vm);
inline void vm_validate_opcode(char *code, size_t length, VM *vm);
void vm_descompose_i32(int32_t value, uint8_t *bytes);
int32_t vm_compose_i32(uint8_t *bytes);
int32_t vm_read_i32(VM *vm);
//< helpers

// vm realted
Object *vm_create_object(ObjectType type, VM *vm);
DynArr *vm_current_chunks(VM *vm);
void vm_jmp(int32_t jmp_value, int32_t from, VM *vm);
void vm_print_primitive(Primitive *primitive);
void vm_print_stack_value(Value *value);
void vm_print_object_value(Value *value);
void vm_print_value(Value *value);
int vm_is_at_end(VM *vm);

uint8_t vm_read_bconst(VM *vm);
int64_t vm_read_iconst(VM *vm);
char *vm_read_string(VM *vm);

//> frame
#define VM_FRAME_SIZE(vm) (vm->frame_ptr + 1)
#define VM_FRAME_PTR(vm) (vm->frame_ptr)
#define VM_FRAME_CURRENT(vm) (&(vm->frames[vm->frame_ptr]))

void vm_frame_setup(Frame *frame, Fn *fn, VM *vm);
void vm_frame_up(Fn *fn, Object *instance, int is_constructor, VM *vm);
void vm_frame_down(VM *vm);

void vm_frame_read(size_t index, VM *vm);
void vm_frame_write(int32_t index, Value *value, VM *vm);

uint8_t vm_advance(VM *vm);
//< frame

//> stack
#define VM_STACK_SIZE(vm) (vm->stack_ptr)
#define VM_STACK_PTR(vm) (VM_STACK_SIZE(vm) - 1)
Value *vm_stack_validate_callable(int args_count, VM *vm);
DynArr *vm_stack_pop_args(int args_count, VM *vm);
Value *vm_stack_check_at(int index, VM *vm);
Value *vm_stack_peek(int index, VM *vm);
void vm_stack_check_overflow(VM *vm);
void vm_stack_set_obj(int at, Object *obj, VM *vm);

void vm_stack_push_nil(VM *vm);
void vm_stack_push_bool(uint8_t primitive, VM *vm);
void vm_stack_push_int(int64_t primitive, VM *vm);
void vm_stack_push_object(Object *object, VM *vm);
void vm_stack_push_value(Value *value, VM *vm);

Value *vm_stack_pop(VM *vm);
//< stack

//> globals
void vm_globals_write(char *identifier, Value *value, VM *vm);
Value *vm_globals_read(char *identifier, VM *vm);
//< stack

//> instructions
void vm_execute_nil(VM *vm);
void vm_execute_bool(VM *vm);
void vm_execute_int(VM *vm);
void vm_execute_string(VM *vm);
void vm_execute_array(VM *vm);
void vm_execute_array_length(VM *vm);
void vm_execute_get_array_item(VM *vm);
void vm_execute_set_array_item(VM *vm);
void vm_execute_arithmetic(int type, VM *vm);
void vm_execute_comparison(int type, VM *vm);
void vm_execute_argjmp(int type, VM *vm);
void vm_execute_logical(int type, VM *vm);
void vm_execute_negation(int type, VM *vm);
void vm_execute_shift(int type, VM *vm);
void vm_execute_bitwise(int type, VM *vm);
void vm_execute_concat(VM *vm);
void vm_execute_length_str(VM *vm);
void vm_execute_str_itm(VM *vm);
void vm_execute_class(VM *vm);
void vm_execute_get_property(VM *vm);
void vm_execute_set_property(VM *vm);
void vm_execute_is(VM *vm);
void vm_execute_from(VM *vm);
void vm_execute_this(VM *vm);
void vm_execute_jmp(VM *vm);
void vm_execute_jmpc(VM *vm);
void vm_execute_get_local(VM *vm);
void vm_execute_set_local(VM *vm);
void vm_execute_set_global(VM *vm);
void vm_execute_get_global(VM *vm);
void vm_execute_load_entity(VM *vm);
void vm_execute_print(VM *vm);
void vm_execute_pop(VM *vm);
void vm_execute_call(VM *vm);
void vm_execute_garbage(VM *vm);
void vm_execute_return(VM *vm);
void vm_execute_instruction(VM *vm);
//< instructions

Value *vm_frame_slot(uint8_t index, VM *vm)
{
    Frame *frame = VM_FRAME_CURRENT(vm);
    return &frame->locals[index];
}

// private implementation
static void native_fn_ascii_code(DynArr *args, VM *vm)
{
    Value *str_value = dynarr_get(0, args);
    Value *index_value = dynarr_get(1, args);

    Primitive *index_primitive = NULL;

    if (!vm_is_value_string(str_value))
        vm_err("Failed to execute native function 'char_code'. Expect str as argument 0.");

    if (!vm_is_value_int(index_value, &index_primitive))
        vm_err("Failed to execute native function 'char_code'. Expect int as argument 1.");

    String *str = &str_value->entity.object->value.string;
    int64_t index = index_primitive->i64;

    if (index < 0 || (size_t)index >= str->length)
        vm_err("Failed to execute native function 'char_code'. Argument 0 constraints: 0 <= index (%d) < str_len (%ld)", index, str->length);

    char c = str->buffer[index];

    vm_stack_push_int((int64_t)c, vm);
}

void native_fn_ascii(DynArr *args, VM *vm)
{
    Value *num_value = dynarr_get(0, args);
    Primitive *num_primitive = NULL;

    if (!vm_is_value_int(num_value, &num_primitive))
        vm_err("Failed to execute native function 'int_to_ascii'. Expect int as argument 0.");

    int64_t num = num_primitive->i64;

    if (num < 0 || num > 127)
        vm_err("Failed to execute native function 'int_to_ascii'. Argument 0 constraints: 0 < num <= 127.");

    char *buffer = vm_memory_alloc(2);

    buffer[0] = (char)num;
    buffer[1] = 0;

    Object *new_str_obj = vm_create_object(STR_OTYPE, vm);
    String *new_str = &new_str_obj->value.string;

    new_str->core = 0;
    new_str->buffer = buffer;
    new_str->length = 1;

    vm_stack_push_object(new_str_obj, vm);
}

void native_fn_str_sub(DynArr *args, VM *vm)
{
    Value *str_value = dynarr_get(0, args);
    Value *from_value = dynarr_get(1, args);
    Value *to_value = dynarr_get(2, args);

    Primitive *from_primitive = NULL;
    Primitive *to_primitive = NULL;

    if (!vm_is_value_string(str_value))
        vm_err("Failed to execute native function 'sub_str'. Expect str as argument 0.");

    if (!vm_is_value_int(from_value, &from_primitive))
        vm_err("Failed to execute native function 'sub_str'. Expect int as argument 1.");

    if (!vm_is_value_int(to_value, &to_primitive))
        vm_err("Failed to execute native function 'sub_str'. Expect int as argument 2.");

    String *str = &str_value->entity.object->value.string;
    int64_t from = from_primitive->i64;
    int64_t to = to_primitive->i64;

    if (from < 0 || from > to)
        vm_err("Failed to execute native function 'sub_str'. Argument 1 constraints: 0 <= from (%d) <= to (%d)", from, to);

    if (to < 0 || (size_t)to >= str->length)
        vm_err("Failed to execute native function 'sub_str'. Argument 2 constraints: 0 <= to (%d) < str_len (%ld)", to, str->length);

    size_t sub_str_buff_len = to - from + 1;
    char *sub_str_buff = vm_memory_alloc(sub_str_buff_len + 1); // + 1 for NULL character

    sub_str_buff[sub_str_buff_len] = 0;
    memcpy(sub_str_buff, str->buffer + from, sub_str_buff_len);

    Object *sub_str_obj = vm_create_object(STR_OTYPE, vm);
    String *sub_str = &sub_str_obj->value.string;

    sub_str->core = 0;
    sub_str->buffer = sub_str_buff;
    sub_str->length = sub_str_buff_len;

    vm_stack_push_object(sub_str_obj, vm);
}

void native_fn_str_lower(DynArr *args, VM *vm)
{
    Value *str_value = dynarr_get(0, args);

    if (!vm_is_value_string(str_value))
        vm_err("Failed to execute native function 'str_lower'. Expect str as argument 0.");

    String *str = &str_value->entity.object->value.string;
    size_t str_len = str->length;

    if (str_len == 0)
    {
        vm_stack_push_value(str_value, vm);

        return;
    }

    char *new_str_buffer = vm_memory_alloc(str_len + 1);

    for (size_t i = 0; i < str_len; i++)
    {
        char c = str->buffer[i];

        if (c >= 'A' && c <= 'Z')
            c = c - 65 + 97;

        new_str_buffer[i] = c;
    }

    new_str_buffer[str_len] = 0;

    Object *new_str_obj = vm_create_object(STR_OTYPE, vm);
    String *new_str = &new_str_obj->value.string;

    new_str->core = 0;
    new_str->buffer = new_str_buffer;
    new_str->length = str_len;

    vm_stack_push_object(new_str_obj, vm);
}

void native_fn_str_upper(DynArr *args, VM *vm)
{
    Value *str_value = dynarr_get(0, args);

    if (!vm_is_value_string(str_value))
        vm_err("Failed to execute native function 'str_lower'. Expect str as argument 0.");

    String *str = &str_value->entity.object->value.string;
    size_t str_len = str->length;

    if (str_len == 0)
    {
        vm_stack_push_value(str_value, vm);

        return;
    }

    char *new_str_buffer = vm_memory_alloc(str_len + 1);

    for (size_t i = 0; i < str_len; i++)
    {
        char c = str->buffer[i];

        if (c >= 'a' && c <= 'z')
            c = c - 97 + 65;

        new_str_buffer[i] = c;
    }

    new_str_buffer[str_len] = 0;

    Object *new_str_obj = vm_create_object(STR_OTYPE, vm);
    String *new_str = &new_str_obj->value.string;

    new_str->core = 0;
    new_str->buffer = new_str_buffer;
    new_str->length = str_len;

    vm_stack_push_object(new_str_obj, vm);
}

void native_fn_str_title(DynArr *args, VM *vm)
{
    Value *str_value = dynarr_get(0, args);

    if (!vm_is_value_string(str_value))
        vm_err("Failed to execute native function 'str_lower'. Expect str as argument 0.");

    String *str = &str_value->entity.object->value.string;
    size_t str_len = str->length;

    if (str_len == 0)
    {
        vm_stack_push_value(str_value, vm);

        return;
    }

    int check = 1;
    char *new_str_buffer = vm_memory_alloc(str_len + 1);

    for (size_t i = 0; i < str_len; i++)
    {
        char c = str->buffer[i];

        if (check && (c >= 'a' && c <= 'z'))
            c = c - 97 + 65;

        new_str_buffer[i] = c;

        check = c == ' ' || c == '\t';
    }

    new_str_buffer[str_len] = 0;

    Object *new_str_obj = vm_create_object(STR_OTYPE, vm);
    String *new_str = &new_str_obj->value.string;

    new_str->core = 0;
    new_str->buffer = new_str_buffer;
    new_str->length = str_len;

    vm_stack_push_object(new_str_obj, vm);
}

void native_fn_str_cmp(DynArr *args, VM *vm)
{
    Value *str0_value = dynarr_get(0, args);
    Value *str1_value = dynarr_get(1, args);

    if (!vm_is_value_string(str0_value))
        vm_err("Failed to execute native function 'cmp_str'. Expect str as argument 0.");

    if (!vm_is_value_string(str1_value))
        vm_err("Failed to execute native function 'cmp_str'. Expect str as argument 1.");

    String *str0 = &str0_value->entity.object->value.string;
    String *str1 = &str1_value->entity.object->value.string;

    uint8_t equals = strcmp(str0->buffer, str1->buffer) == 0;

    vm_stack_push_bool(equals, vm);
}

void native_fn_str_cmp_ic(DynArr *args, VM *vm)
{
    Value *str0_value = dynarr_get(0, args);
    Value *str1_value = dynarr_get(1, args);

    if (!vm_is_value_string(str0_value))
        vm_err("Failed to execute native function 'cmp_ic_str'. Expect str as argument 0.");

    if (!vm_is_value_string(str1_value))
        vm_err("Failed to execute native function 'cmp_ic_str'. Expect str as argument 1.");

    String *str0 = &str0_value->entity.object->value.string;
    String *str1 = &str1_value->entity.object->value.string;

    size_t str0_len = str0->length;
    size_t str1_len = str1->length;

    if (str0_len != str1_len)
    {
        vm_stack_push_bool(0, vm);

        return;
    }

    for (size_t i = 0; i < str0_len; i++)
    {
        char a = str0->buffer[i];
        char b = str1->buffer[i];

        if (a >= 'A' && a <= 'Z')
            a = a - 65 + 97;

        if (b >= 'A' && b <= 'Z')
            b = b - 65 + 97;

        if (a != b)
        {
            vm_stack_push_bool(0, vm);

            return;
        }
    }

    vm_stack_push_bool(1, vm);
}

void native_fn_is_str_int(DynArr *args, VM *vm)
{
    Value *str_value = dynarr_get(0, args);

    if (!vm_is_value_string(str_value))
        vm_err("Failed to execute native function 'is_str_int'. Expect str as argument 0.");

    String *str = &str_value->entity.object->value.string;

    vm_stack_push_bool(is_str_int(str->buffer, str->length), vm);
}

void native_fn_str_to_int(DynArr *args, VM *vm)
{
    Value *str_value = dynarr_get(0, args);

    if (!vm_is_value_string(str_value))
        vm_err("Failed to execute native function 'str_to_int'. Argument 0 must be str type.");

    String *str = &str_value->entity.object->value.string;

    char *buffer = str->buffer;
    size_t length = str->length;

    if (!is_str_int(buffer, length))
        vm_err("Failed to execute native function 'str_to_int'. Argument 0 must represent a decimal number.");

    int64_t number = 0;
    int is_negative = 0;

    for (size_t i = 0; i < length; i++)
    {
        char c = buffer[i];

        if (!is_negative && c == '-')
        {
            is_negative = 1;
            continue;
        }

        char digit = c - 48;

        number *= 10;
        number += digit;
    }

    if (is_negative)
        number *= -1;

    vm_stack_push_int(number, vm);
}

void native_fn_int_to_str(DynArr *args, VM *vm)
{
    Value *num_value = dynarr_get(0, args);
    Primitive *num_primitive = NULL;

    if (!vm_is_value_int(num_value, &num_primitive))
        vm_err("Failed to execute native function 'int_to_str'. Expect int as argument 0.");

    int64_t raw_num = num_primitive->i64;

    int is_negative = raw_num < 0;
    int64_t num = is_negative ? raw_num * -1 : raw_num;

    int i = 18;
    char raw_buffer[19];

    while (i >= 0)
    {
        if (num < 10)
        {
            raw_buffer[i] = (char)(48 + num);

            break;
        }

        int64_t last = num % 10;
        int64_t first = num / 10;

        raw_buffer[i] = (char)(48 + last);
        num = first;

        i--;
    }

    size_t length = is_negative ? 19 - i + 1 : 19 - i;
    char *buffer = vm_memory_alloc(length + 1);

    memcpy(is_negative ? buffer + 1 : buffer, raw_buffer + i, length);

    if (is_negative)
        buffer[0] = '-';

    buffer[length] = 0;

    Object *str_obj = vm_create_object(STR_OTYPE, vm);
    String *str = &str_obj->value.string;

    str->core = 0;
    str->length = length;
    str->buffer = buffer;

    vm_stack_push_object(str_obj, vm);
}

void native_fn_time(DynArr *args, VM *vm)
{
    time_t t = time(NULL);
    vm_stack_push_int((int64_t)t, vm);
}

static void native_fn_sleep(DynArr *args, VM *vm)
{
    Value *sleep_value = dynarr_get(0, args);
    Primitive *sleep_primitive = NULL;

    if (!vm_is_value_int(sleep_value, &sleep_primitive))
        vm_err("Failed to execute native function 'sleep'. Expect int as argument 0.");

    int64_t value = sleep_primitive->i64;

    sleep((unsigned int)value);
}

static void native_fn_read_line(DynArr *args, VM *vm)
{
    size_t ptr = 0;
    const size_t raw_buff_len = 1024;
    char raw_buff[raw_buff_len];

    while ((size_t)ptr < raw_buff_len - 1)
    {
        int c = getchar();

        if (c == EOF || c == '\n')
            break;

        raw_buff[ptr] = (char)c;

        ptr++;
    }

    raw_buff[ptr] = 0;

    char *buffer = vm_memory_clone_string(raw_buff);
    Object *str_obj = vm_create_object(STR_OTYPE, vm);
    String *str = &str_obj->value.string;

    str->core = 0;
    str->buffer = buffer;
    str->length = ptr;

    vm_stack_push_object(str_obj, vm);
}

void native_fn_read_file(DynArr *args, VM *vm)
{
    Value *path_value = dynarr_get(0, args);
    Value *position_value = dynarr_get(1, args);
    Value *size_value = dynarr_get(2, args);
    Value *count_value = dynarr_get(3, args);
    Value *buffer_value = dynarr_get(4, args);

    if (!vm_is_value_string(path_value))
        vm_err("Wrong argument 0. Expect str.");

    if (!vm_is_value_int(position_value, NULL))
        vm_err("Wrong argument 1. Expect int.");

    if (!vm_is_value_int(size_value, NULL))
        vm_err("Wrong argument 2. Expect int.");

    if (!vm_is_value_int(count_value, NULL))
        vm_err("Wrong argument 3. Expect int.");

    if (!vm_is_value_array(buffer_value))
        vm_err("Wrong argument 4. Expect array.");

    char *path = path_value->entity.object->value.string.buffer;
    int64_t position = position_value->entity.primitive.i64;
    int64_t size = size_value->entity.primitive.i64;
    int64_t count = count_value->entity.primitive.i64;
    Array *arr = &buffer_value->entity.object->value.array;

    if (position < 0)
        vm_err("wrong argument 1. Constrains: 0 <= position.");

    if (size < 0 || size > 4)
        vm_err("wrong argument 2. Constrains: 0 <= size <= 4.");

    if (count < 0)
        vm_err("wrong argument 3. Constrains: 0 <= count.");

    if ((size_t)count > arr->length)
        vm_err("wrong argument 4. Constrains: count <= buffer_len.");

    FILE *file = fopen(path, "r");

    if (!file)
        vm_err("Failed to open file.");

    if (fseek(file, (long)position, SEEK_SET) != 0)
    {
        fclose(file);
        vm_err("Failed set file position.");
    }

    size_t counter = 0;
    char container[(size_t)size];

    while (fread(&container, (size_t)size, 1, file) == 1)
    {
        Object *value_obj = vm_create_object(VALUE_OTYPE, vm);
        Primitive *value = &value_obj->value.primitive;

        value->type = INT_VTYPE;
        memcpy(&value->i64, container, sizeof(container));
        arr->items[counter] = value_obj;

        counter++;

        if (counter == (size_t)count)
            break;
    }

    vm_stack_push_int((int64_t)counter, vm);
}

void native_fn_panic(DynArr *args, VM *vm)
{
    Value *str_value = dynarr_get(0, args);

    if (!vm_is_value_string(str_value))
        vm_err("Failed to execute native function 'panic'. Expect str as argument 0.");

    String *str = &str_value->entity.object->value.string;

    fprintf(stderr, "PANIC!: ");
    _error_(str->buffer);

    vm->stop = 1;
    vm->rtn_code = 1;
}

static void native_fn_exit(DynArr *args, VM *vm)
{
    Value *code_value = dynarr_get(0, args);
    Primitive *code_primitive = NULL;

    if (!vm_is_value_int(code_value, &code_primitive))
        vm_err("Failed to execute native function 'fn_exit'. Expect a int as argument 0.");

    int64_t code = code_primitive->i64;

    vm->stop = 1;
    vm->rtn_code = (int)code;
}

void vm_add_native(char *name, char arity, void (*raw_native_fn)(DynArr *args, VM *), VM *vm)
{
    NativeFn *native_fn = vm_memory_alloc(sizeof(NativeFn));

    native_fn->name = vm_memory_clone_string(name);
    native_fn->arity = arity;
    native_fn->raw_fn = raw_native_fn;

    Entity entity = {0};

    entity.type = NATIVE_SYMTYPE;
    entity.raw_symbol = (void *)native_fn;

    dynarr_insert((void *)&entity, vm->entities);
}

static void _error_(char *msg, ...)
{
    va_list args;
    va_start(args, msg);

    vfprintf(stderr, msg, args);
    fprintf(stderr, "\n");

    va_end(args);
}

void vm_err(char *msg, ...)
{
    va_list args;
    va_start(args, msg);

    vfprintf(stderr, msg, args);
    printf("\n");

    va_end(args);

    vm_memory_deinit();

    exit(1);
}

void vm_garbage_report(char *msg, ...)
{
#ifdef VM_GARBAGE_VERBOSE
    va_list args;
    va_start(args, msg);

    vfprintf(stderr, msg, args);
    printf("\n");

    va_end(args);
#endif
}

void vm_garbage_string(Object *object)
{
    String *string = &object->value.string;

    if (!string->core)
        vm_memory_dealloc(string->buffer);
}

void vm_garbage_object_array(Object *object)
{
    Array *arr = &object->value.array;

    vm_memory_dealloc(arr->items);
}

void vm_garbage_instance(Object *object)
{
    Instance *instance = (Instance *)&object->value.instance;

    LZHTable *attrs_table = instance->attributes;
    LZHTableNode *last_attr_node = attrs_table->nodes;

    while (last_attr_node)
    {
        LZHTableNode *prev_attr_node = last_attr_node->previous_table_node;
        Value *attr_value = (Value *)last_attr_node->value;

        vm_memory_dealloc(attr_value);

        last_attr_node = prev_attr_node;
    }

    lzhtable_destroy(attrs_table);
}

void vm_garbage_object(Object *object)
{
    assert(object && "object is NULL");

    switch (object->type)
    {
    case VALUE_OTYPE:
        // VALUE object type does not have
        // futher memory that must be released
        break;

    case STR_OTYPE:
        vm_garbage_string(object);
        break;

    case FN_OTYPE:
        // FN object type does not have
        // futher memory that must be released
        break;

    case NATIVE_FN_OTYPE:
        // NATIVE FN object type does not have
        // futher memory that must be released
        break;

    case METHOD_OTYPE:
        // METHOD object type does not have
        // futher memory that must be released
        break;

    case ARR_OTYPE:
        vm_garbage_object_array(object);
        break;

    case CLASS_OTYPE:
        // KLASS object type does not have
        // futher memory that must be released
        break;

    case INSTANCE_OTYPE:
        vm_garbage_instance(object);
        break;

    default:
        assert(0 && "Illegal ObjectType value");
    }

    vm_memory_destroy_object(object);

    vm_garbage_report("Object %p collected", object);
}

void vm_garbage_objects(VM *vm)
{
    Object *object = vm->head_object;

    while (object)
    {
        Object *next = object->next;

        vm_garbage_object(object);

        object = next;
    }
}

int vm_gc_sweep_object(Object *object)
{
    if (object->marked)
    {
        object->marked = 0;

        return 0;
    }

    vm_garbage_object(object);

    return 1;
}

void vm_gc_sweep_objects(VM *vm)
{
    size_t count = 0;

    Object *before = NULL;
    Object *object = vm->head_object;

    while (object)
    {
        Object *next = object->next;

        char is_head = object == vm->head_object;
        char is_tail = object == vm->tail_object;

        char flag = vm_gc_sweep_object(object);

        if (flag)
        {
            vm->size -= sizeof(Object);

            if (is_head)
                vm->head_object = next;

            if (is_tail)
                vm->tail_object = before;

            if (before)
                before->next = next;

            count++;
        }
        else
            before = object;

        object = next;
    }

    vm->size -= sizeof(Object) * count;

    vm_garbage_report("%ld objects collected", count);
}

int vm_gc_mark_object_array(Object *object)
{
    object->marked = 1;

    size_t count = 0;

    Array *arr = &object->value.array;

    for (size_t i = 0; i < arr->length; i++)
    {
        Object *item = arr->items[i];

        if (!item)
            continue;

        count += vm_gc_mark_object(item);
    }

    return count + 1;
}

int vm_gc_mark_instance(Object *object)
{
    object->marked = 1;

    Instance *instance = &object->value.instance;

    LZHTable *attrs_table = instance->attributes;
    LZHTableNode *attr_node = attrs_table->nodes;

    while (attr_node)
    {
        LZHTableNode *prev_attr_node = attr_node->previous_table_node;

        Value *attr_value = (Value *)attr_node->value;

        if (attr_value->type == OBJECT_HTYPE)
        {
            Object *attr_obj = attr_value->entity.object;

            vm_gc_mark_object(attr_obj);
        }

        attr_node = prev_attr_node;
    }

    return 1;
}

int vm_gc_mark_object(Object *object)
{
    if (object->marked)
        return 0;

    vm_garbage_report("Object %p, marked", object);

    switch (object->type)
    {
    case VALUE_OTYPE:
        object->marked = 1;
        return 1;

    case STR_OTYPE:
        object->marked = 1;
        return 1;

    case ARR_OTYPE:
        return vm_gc_mark_object_array(object);

    case FN_OTYPE:
        object->marked = 1;
        return 1;

    case METHOD_OTYPE:
        object->marked = 1;

        Method *method = &object->value.method;

        vm_gc_mark_object((Object *)method->instance);

        return 1;

    case NATIVE_FN_OTYPE:
        object->marked = 1;
        return 1;

    case CLASS_OTYPE:
        object->marked = 1;
        return 1;

    case INSTANCE_OTYPE:
        return vm_gc_mark_instance(object);

    default:
        assert(0 && "Illegal ObjectType value");
    }
}

int vm_gc_mark_frame_values(Value *values)
{
    size_t count = 0;

    size_t values_len = FRAME_VALUES_LENGTH;

    for (size_t i = 0; i < values_len; i++)
    {
        Value *value = &values[i];

        if (value->type != OBJECT_HTYPE)
            continue;

        count += vm_gc_mark_object(value->entity.object);
    }

    return count;
}

int vm_gc_mark_frames(VM *vm)
{
    size_t count = 0;

    for (int i = vm->frame_ptr; i >= 0; i--)
    {
        Frame *frame = &vm->frames[i];
        Object *instance_obj = frame->instance;

        if (instance_obj)
            vm_gc_mark_object(instance_obj);

        count += vm_gc_mark_frame_values(frame->locals);
    }

    return count;
}

int vm_gc_mark_stack(VM *vm)
{
    size_t count = 0;

    for (int i = 0; i < vm->stack_ptr; i++)
    {
        Value *value = &vm->stack[i];

        if (value->type != OBJECT_HTYPE)
            continue;

        count += vm_gc_mark_object(value->entity.object);
    }

    return count;
}

int vm_gc_mark_globals(VM *vm)
{
    size_t count = 0;

    LZHTableNode *node = vm->globals->nodes;

    while (node)
    {
        LZHTableNode *previous = node->previous_table_node;

        Value *value = (Value *)node->value;

        if (value->type == OBJECT_HTYPE)
        {
            Object *object = value->entity.object;

            count += vm_gc_mark_object(object);
        }

        node = previous;
    }

    return count;
}

void vm_gc_mark_objects(VM *vm)
{
    size_t count = 0;

    count += vm_gc_mark_frames(vm);
    count += vm_gc_mark_stack(vm);
    count += vm_gc_mark_globals(vm);

    vm_garbage_report("%ld objects marked", count);
}

void vm_gc(VM *vm)
{
    vm_gc_mark_objects(vm);
    vm_gc_sweep_objects(vm);
}

int vm_is_value_nil(Value *value)
{
    return value->type == NIL_HTYPE;
}

int vm_is_value_primitive(Value *value, Primitive **out_primitive)
{
    if (value->type == NIL_HTYPE)
        return 0;

    Primitive *primitive = NULL;

    if (value->type == VALUE_HTYPE)
        primitive = &value->entity.primitive;
    else
    {
        Object *obj = value->entity.object;

        if (obj->type == VALUE_OTYPE)
            primitive = &obj->value.primitive;
    }

    if (!primitive)
        return 0;

    if (out_primitive)
        *out_primitive = primitive;

    return 1;
}

int vm_is_value_bool(Value *value, Primitive **out_primitive)
{
    Primitive *primitive = NULL;

    if (!vm_is_value_primitive(value, &primitive))
        return 0;

    if (out_primitive)
        *out_primitive = primitive;

    return primitive->type == BOOL_VTYPE;
}

int vm_is_value_int(Value *value, Primitive **out_primitive)
{
    Primitive *primitive = NULL;

    if (!vm_is_value_primitive(value, &primitive))
        return 0;

    if (out_primitive)
        *out_primitive = primitive;

    return primitive->type == INT_VTYPE;
}

int vm_is_value_string(Value *value)
{
    if (value->type != OBJECT_HTYPE)
        return 0;

    return value->entity.object->type == STR_OTYPE;
}

int vm_is_value_obj_value(Value *value)
{
    if (value->type != OBJECT_HTYPE)
        return 0;

    Object *obj = value->entity.object;

    return obj->type == VALUE_OTYPE;
}

int vm_is_value_array(Value *value)
{
    if (value->type != OBJECT_HTYPE)
        return 0;

    Object *object = value->entity.object;

    return object->type == ARR_OTYPE;
}

int vm_is_value_fn(Value *value)
{
    if (value->type != OBJECT_HTYPE)
        return 0;

    return value->entity.object->type == FN_OTYPE;
}

int vm_is_value_native_fn(Value *value)
{
    if (value->type != OBJECT_HTYPE)
        return 0;

    return value->entity.object->type == NATIVE_FN_OTYPE;
}

int vm_is_value_method(Value *value)
{
    if (value->type != OBJECT_HTYPE)
        return 0;

    return value->entity.object->type == METHOD_OTYPE;
}

int vm_is_value_klass(Value *value)
{
    if (value->type != OBJECT_HTYPE)
        return 0;

    return value->entity.object->type == CLASS_OTYPE;
}

int vm_is_value_callable(Value *value)
{
    return vm_is_value_fn(value) ||
           vm_is_value_native_fn(value) ||
           vm_is_value_method(value) ||
           vm_is_value_klass(value);
}

int vm_is_value_instance(Value *value)
{
    if (value->type != OBJECT_HTYPE)
        return 0;

    return value->entity.object->type == INSTANCE_OTYPE;
}

Object *vm_create_method(Object *instance, Fn *function, VM *vm)
{
    Object *method_obj = vm_create_object(METHOD_OTYPE, vm);

    Method *method = (Method *)&method_obj->value.method;

    method->instance = instance;
    method->fn = function;

    return method_obj;
}

void vm_validate_opcode(char *code, size_t length, VM *vm)
{
#ifdef VM_VALIDATE_OPCODES
    Frame *frame = VM_FRAME_CURRENT(vm);

    size_t ip = frame->ip;
    size_t chunks_len = frame->chunks->used;
    size_t diff = chunks_len - ip;

    if (length > diff)
        vm_err("Instruction inconsistency '%s': length is %ld but remain %d.", code, length, diff);
#endif
}

void vm_descompose_i32(int32_t value, uint8_t *bytes)
{
    uint8_t mask = 0b11111111;

    for (size_t i = 0; i < 4; i++)
    {
        uint8_t r = value & mask;
        value = value >> 8;
        bytes[i] = r;
    }
}

int32_t vm_compose_i32(uint8_t *bytes)
{
    return ((int32_t)bytes[3] << 24) | ((int32_t)bytes[2] << 16) | ((int32_t)bytes[1] << 8) | ((int32_t)bytes[0]);
}

int32_t vm_read_i32(VM *vm)
{
    uint8_t bytes[4];

    for (size_t i = 0; i < 4; i++)
        bytes[i] = vm_advance(vm);

    return vm_compose_i32(bytes);
}

Object *vm_create_object(ObjectType type, VM *vm)
{
    // if (vm->size >= 1024)
    // vm_gc(vm);

    Object *obj = vm_memory_create_object(type, vm);

    return obj;
}

DynArr *vm_current_chunks(VM *vm)
{
    return (DynArr *)lzstack_peek(vm->blocks_stack, NULL);
}

void vm_jmp(int32_t jmp_value, int32_t from, VM *vm)
{
    Frame *frame = VM_FRAME_CURRENT(vm);
    size_t current_ip = from;

    if (jmp_value == 0)
        return;

    if (jmp_value < 0)
    {
        if ((size_t)jmp_value * -1 > current_ip)
            vm_err("Failed to execute jmp. current ip %ld, plus jmp value %d, less than 0", current_ip, jmp_value);

        frame->ip = current_ip + jmp_value;
    }
    else
    {
        if (current_ip + jmp_value > frame->chunks->used)
            vm_err("Failed to execute jmp. current ip %ld, plus jmp value %d, greater than chunks length %ld", current_ip, jmp_value, frame->chunks->used);

        frame->ip += jmp_value;
    }
}

void vm_print_primitive(Primitive *primitive)
{
    switch (primitive->type)
    {
    case BOOL_VTYPE:
        int64_t bool_value = primitive->i64;

        if (bool_value)
            printf("true\n");
        else
            printf("false\n");

        break;

    case INT_VTYPE:
        printf("%ld\n", primitive->i64);
        break;

    default:
        assert(0 && "Illegal value type value");
    }
}

void vm_print_stack_value(Value *value)
{
    Primitive *primitive = &value->entity.primitive;
    vm_print_primitive(primitive);
}

void vm_print_object_value(Value *value)
{
    Object *object = value->entity.object;

    switch (object->type)
    {
    case STR_OTYPE:
        String *string = &object->value.string;
        printf("%s\n", string->buffer);
        break;

    case VALUE_OTYPE:
        Primitive *primitive = &value->entity.object->value.primitive;
        vm_print_primitive(primitive);
        break;

    case ARR_OTYPE:
        Array *arr = &value->entity.object->value.array;
        printf("<object array: %ld> at %p\n", arr->length, arr);
        break;

    case FN_OTYPE:
        Fn *fn = value->entity.object->value.fn;
        printf("<fn '%s': %ld> at %p\n", fn->name, fn->params->used, fn);
        break;

    case NATIVE_FN_OTYPE:
        NativeFn *native_fn = value->entity.object->value.native_fn;
        printf("<native fn '%s' %d>\n", native_fn->name, native_fn->arity);
        break;

    case METHOD_OTYPE:
        Method *method = &value->entity.object->value.method;
        Fn *method_fn = method->fn;
        printf("<fn '%s': %ld> at %p\n", method_fn->name, method_fn->params->used, method_fn);
        break;

    case CLASS_OTYPE:
        Klass *class = value->entity.object->value.class;
        printf("<class '%s'> at %p\n", class->name, class);
        break;

    case INSTANCE_OTYPE:
        Instance *instance = &value->entity.object->value.instance;
        printf("<instance of '%s'> at %p\n", instance->klass->name, instance);
        break;

    default:
        assert(0 && "Illegal ObjectType value");
    }
}

void vm_print_value(Value *value)
{
    if (value->type == VALUE_HTYPE)
        vm_print_stack_value(value);
    else if (value->type == OBJECT_HTYPE)
        vm_print_object_value(value);
    else
        printf("NIL\n");
}

int vm_is_at_end(VM *vm)
{
    Frame *frame = VM_FRAME_CURRENT(vm);

    size_t ip = frame->ip;
    size_t length = frame->chunks->used;

    return ip >= length || vm->halt || vm->stop;
}

uint8_t vm_read_bconst(VM *vm)
{
    return vm_advance(vm);
}

int64_t vm_read_iconst(VM *vm)
{
    int32_t index = vm_read_i32(vm);
    size_t len = vm->iconsts->used;

    if ((size_t)index >= len)
        vm_err("Failed to read constant. Length is %ld but got %d", len, index);

    return *(int64_t *)dynarr_get((size_t)index, vm->iconsts);
}

char *vm_read_string(VM *vm)
{
    int32_t index = vm_read_i32(vm);

    size_t len = vm->strings->used;

    if ((size_t)index >= len)
        vm_err("Failed to read string literal. Length is %ld but got %d", len, index);

    return (char *)DYNARR_PTR_GET((size_t)index, vm->strings);
}

void vm_frame_setup(Frame *frame, Fn *fn, VM *vm)
{
    DynArrPtr *params = fn->params;
    size_t args_count = params == NULL ? 0 : params->used;

    for (size_t i = 0; i < args_count; i++)
        memcpy((void *)&frame->locals[i], vm_stack_pop(vm), sizeof(Value));

    Value *callable_value = vm_stack_pop(vm);

    if (!vm_is_value_callable(callable_value))
        vm_err("Frame setup failed. Expect callable after arguments, but got something else.");
}

void vm_frame_up(Fn *fn, Object *instance, int is_constructor, VM *vm)
{
    if (VM_FRAME_PTR(vm) + 1 >= VM_FRAME_LENGTH)
        vm_err("FrameOverFlow");

    Frame *frame = &vm->frames[++vm->frame_ptr];

    vm_frame_setup(frame, fn, vm);

    frame->ip = 0;
    frame->chunks = fn->chunks;
    frame->instance = instance;
    frame->is_constructor = is_constructor;
}

void vm_frame_down(VM *vm)
{
    if (VM_FRAME_PTR(vm) - 1 < 0)
        vm_err("FrameUnderFlow");

    Frame *frame = VM_FRAME_CURRENT(vm);

    frame->ip = 0;
    frame->chunks = NULL;
    frame->instance = NULL;
    frame->is_constructor = 0;

    vm->frame_ptr--;
}

void vm_frame_read(size_t index, VM *vm)
{
    Frame *frame = VM_FRAME_CURRENT(vm);
    Value *locals = frame->locals;

    if (index >= FRAME_VALUES_LENGTH)
        vm_err("Failed to get local. Illegal local index: %ld.", index);

    Value *value = &locals[index];

    vm_stack_push_value(value, vm);
}

void vm_frame_write(int32_t index, Value *value, VM *vm)
{
    Frame *frame = VM_FRAME_CURRENT(vm);
    Value *locals = frame->locals;

    if (index >= FRAME_VALUES_LENGTH)
        vm_err("Failed to set local. Illegal local index: %ld.", index);

    Value *local_value = &locals[index];

    local_value->type = value->type;
    local_value->entity = value->entity;
}

uint8_t vm_advance(VM *vm)
{
    Frame *frame = VM_FRAME_CURRENT(vm);
    return *(uint8_t *)dynarr_get(frame->ip++, frame->chunks);
}

Value *vm_stack_validate_callable(int args_count, VM *vm)
{
    if (args_count > VM_STACK_SIZE(vm))
        vm_err("Stack size is %d, but callable arguments count %d", VM_STACK_SIZE(vm), args_count);

    if (args_count + 1 > VM_STACK_SIZE(vm))
        vm_err("Stack size is %d. Callable arguments count is %d. Can't check callable before arguments.", VM_STACK_SIZE(vm), args_count);

    Value *callable_value = &vm->stack[VM_STACK_PTR(vm) - args_count];

    if (!vm_is_value_callable(callable_value))
        vm_err("Expect calalble just before arguments, but got something else.");

    return callable_value;
}

DynArr *vm_stack_pop_args(int args_count, VM *vm)
{
    Value *callable_value = vm_stack_validate_callable(args_count, vm);
    DynArr *args = vm_memory_create_dynarr(sizeof(Value));

    for (size_t i = 0; i < (size_t)args_count; i++)
    {
        Value *arg = vm_stack_pop(vm);
        dynarr_insert((void *)arg, args);
    }

    if (callable_value != vm_stack_pop(vm))
        assert(0 && "Validated callable is not the same at the end of arguments");

    return args;
}

Value *vm_stack_check_at(int index, VM *vm)
{
    int stack_ptr = VM_STACK_PTR(vm);

    assert(index >= 0 && index <= stack_ptr && "Illegal index");

    return &vm->stack[index];
}

Value *vm_stack_peek(int distance, VM *vm)
{
    assert(distance >= 0 && "'distance' must be > 0");

    int stack_position = VM_STACK_PTR(vm) - distance;

    if (stack_position < 0)
        vm_err("Failed to peek stack. Illegal stack position.");

    return &vm->stack[stack_position];
}

void vm_stack_check_overflow(VM *vm)
{
    int stack_len = (int)(sizeof(vm->stack) / sizeof(vm->stack[0]));

    if (vm->stack_ptr + 1 >= stack_len)
        vm_err("StackOverFlowError");
}

void vm_stack_set_obj(int count, Object *obj, VM *vm)
{
    int top = vm->stack_ptr - 1;
    int remain = VM_STACK_LENGTH - top - 1;
    int at = top == 0 ? 0 : top - count;
    int mov_count = top + 1 - at;

    if (mov_count > remain)
        vm_err("Failed to set obj in stack. Need to move %d, but remian %d.", mov_count, remain);

    Value *stack = vm->stack;

    memmove(stack + at + 1, stack + at, sizeof(Value) * mov_count);

    Value *at_value = &stack[at];

    at_value->type = OBJECT_HTYPE;
    at_value->entity.object = obj;

    vm->stack_ptr++;
}

void vm_stack_push_nil(VM *vm)
{
    Value *value = &vm->stack[vm->stack_ptr++];

    memset((void *)value, 0, sizeof(Value));

    value->type = NIL_HTYPE;
}

void vm_stack_push_bool(uint8_t primitive, VM *vm)
{
    vm_stack_check_overflow(vm);

    Value *value = &vm->stack[vm->stack_ptr++];

    value->type = VALUE_HTYPE;
    value->entity.primitive.type = BOOL_VTYPE;
    value->entity.primitive.i64 = primitive;
}

void vm_stack_push_int(int64_t primitive, VM *vm)
{
    vm_stack_check_overflow(vm);

    Value *value = &vm->stack[vm->stack_ptr++];

    value->type = VALUE_HTYPE;
    value->entity.primitive.type = INT_VTYPE;
    value->entity.primitive.i64 = primitive;
}

void vm_stack_push_object(Object *object, VM *vm)
{
    vm_stack_check_overflow(vm);

    Value *value = &vm->stack[vm->stack_ptr++];

    value->type = OBJECT_HTYPE;
    value->entity.object = object;
}

void vm_stack_push_value(Value *value, VM *vm)
{
    vm_stack_check_overflow(vm);

    if (value->type == VALUE_HTYPE)
    {
        Primitive *primitive = &value->entity.primitive;

        if (primitive->type == BOOL_VTYPE)
            vm_stack_push_bool(primitive->i64, vm);
        else if (primitive->type == INT_VTYPE)
            vm_stack_push_int(primitive->i64, vm);
    }
    else if (value->type == OBJECT_HTYPE)
        vm_stack_push_object(value->entity.object, vm);
    else
        vm_stack_push_nil(vm);
}

Value *vm_stack_pop(VM *vm)
{
    if (VM_STACK_PTR(vm) == -1)
        vm_err("StackUnderFlowError");

    return &vm->stack[--vm->stack_ptr];
}

void vm_globals_write(char *identifier, Value *value, VM *vm)
{
    Value *global_value = (Value *)lzhtable_get((uint8_t *)identifier, strlen(identifier), vm->globals);

    if (!global_value)
    {
        global_value = vm_memory_alloc(sizeof(Value));
        lzhtable_put((uint8_t *)identifier, strlen(identifier), (void *)global_value, vm->globals, NULL);
    }

    global_value->type = value->type;
    global_value->entity = value->entity;
}

Value *vm_globals_read(char *identifier, VM *vm)
{
    void *raw_value = lzhtable_get((uint8_t *)identifier, strlen(identifier), vm->globals);

    if (!raw_value)
        vm_err("Failed to get global: '%s' do not exists.", identifier);

    return (Value *)raw_value;
}

void vm_execute_nil(VM *vm)
{
    vm_stack_push_nil(vm);
}

void vm_execute_bool(VM *vm)
{
    vm_stack_push_bool(vm_read_bconst(vm), vm);
}

void vm_execute_int(VM *vm)
{
    vm_stack_push_int(vm_read_iconst(vm), vm);
}

void vm_execute_string(VM *vm)
{
    char *buff = vm_read_string(vm);
    Object *str_obj = vm_create_object(STR_OTYPE, vm);
    String *str = &str_obj->value.string;

    str->core = 1;
    str->buffer = buff;
    str->length = strlen(buff);

    vm_stack_push_object(str_obj, vm);
}

void vm_execute_array(VM *vm)
{
    Value *len_value = vm_stack_pop(vm);
    Primitive *len_primitive = NULL;

    if (!vm_is_value_int(len_value, &len_primitive))
        vm_err("Failed to create object array. Expect int as length, but got something else.");

    int32_t len = (int32_t)len_primitive->i64;

    if (len < 0 || len > INT32_MAX)
        vm_err("Failed to create object array. Constraints: 0 < length (%d) <= %d.", len, INT32_MAX);

    uint8_t is_empty = vm_advance(vm);

    Object *arr_obj = vm_create_object(ARR_OTYPE, vm);
    Array *arr = &arr_obj->value.array;

    if (len == 0)
    {
        arr->length = 0;
        arr->items = NULL;
    }
    else
    {
        arr->length = (size_t)len;
        arr->items = is_empty ? vm_memory_calloc(sizeof(Object *) * len) : vm_memory_alloc(sizeof(Object *) * len);
    }

    if (!is_empty)
    {
        for (int32_t i = len - 1; i >= 0; i--)
        {
            Value *value = vm_stack_pop(vm);

            if (value->type == VALUE_HTYPE)
            {
                Object *obj = vm_create_object(VALUE_OTYPE, vm);
                Primitive *primitive = &obj->value.primitive;

                primitive->type = value->entity.primitive.type;
                primitive->i64 = value->entity.primitive.i64;

                arr->items[i] = obj;

                continue;
            }

            Object *obj = value->entity.object;
            arr->items[i] = obj;
        }
    }

    vm_stack_push_object(arr_obj, vm);
}

void vm_execute_array_length(VM *vm)
{
    Value *value = vm_stack_pop(vm);

    if (value->type != OBJECT_HTYPE)
        vm_err("Failed to get array length: illegal type.");

    Object *object = value->entity.object;

    if (object->type != ARR_OTYPE)
        vm_err("Failed to get array length: illegal type.");

    vm_stack_push_int((int64_t)object->value.array.length, vm);
}

void vm_execute_get_array_item(VM *vm)
{
    Value *index_value = vm_stack_pop(vm);

    if (!vm_is_value_int(index_value, NULL))
        vm_err("Failed to get array item: expect index, but got something else.");

    Value *arr_value = vm_stack_pop(vm);

    if (!vm_is_value_array(arr_value))
        vm_err("Failed to get array item: expect array but got something else.");

    int64_t index = (int64_t)index_value->entity.primitive.i64;

    Object *arr_obj = arr_value->entity.object;
    Array *arr = &arr_obj->value.array;

    if (index < 0 || (size_t)index >= arr->length)
        vm_err("Failed to get array item. Constraints: 0 < index (%d) < arr_len (%ld).", index, arr->length);

    Object *item_obj = arr->items[index];

    if (item_obj)
        vm_stack_push_object(item_obj, vm);
    else
        vm_stack_push_nil(vm);
}

void vm_execute_set_array_item(VM *vm)
{
    Value *index_value = vm_stack_pop(vm);
    Value *array_value = vm_stack_pop(vm);
    Value *value_value = vm_stack_peek(0, vm);

    Primitive *index_primitive = NULL;

    if (!vm_is_value_int(index_value, &index_primitive))
        vm_err("Failed to assign value to array. Expect a int as index, but got something else.");

    int32_t index = (int32_t)index_primitive->i64;

    if (!vm_is_value_array(array_value))
        vm_err("Failed to assign value to array. Expect a arra, but got something else.");

    Array *array_obj = &array_value->entity.object->value.array;

    if (index < 0 || (size_t)index >= array_obj->length)
        vm_err("Failed to assign value to array. Constraints: 0 < index (%d) < arr_len (%ld).", index, array_obj->length);

    if (value_value->type == NIL_HTYPE)
    {
        array_obj->items[index] = NULL;
        return;
    }

    Primitive *raw_rvalue = NULL;

    if (vm_is_value_primitive(value_value, &raw_rvalue))
    {
        Object *value_obj = vm_create_object(VALUE_OTYPE, vm);
        Primitive *raw_lvalue = &value_obj->value.primitive;

        raw_lvalue->type = raw_rvalue->type;
        raw_lvalue->i64 = raw_rvalue->i64;

        array_obj->items[index] = value_obj;

        return;
    }

    array_obj->items[index] = value_value->entity.object;
}

void vm_execute_arithmetic(int type, VM *vm)
{
    Value *right = vm_stack_pop(vm);
    Value *left = vm_stack_pop(vm);

    Primitive *lvalue = NULL;
    Primitive *rvalue = NULL;

    if (!vm_is_value_int(left, &lvalue))
        vm_err("Failed to execute arithmetic. Left is not int type.");

    if (!vm_is_value_int(right, &rvalue))
        vm_err("Failed to execute arithmetic. Right is not int type.");

    switch (type)
    {
    // addition
    case 1:
        vm_stack_push_int(lvalue->i64 + rvalue->i64, vm);
        break;

    // subtraction
    case 2:
        vm_stack_push_int(lvalue->i64 - rvalue->i64, vm);
        break;

    // mutiplication
    case 3:
        vm_stack_push_int(lvalue->i64 * rvalue->i64, vm);
        break;

    // division
    case 4:
        if (rvalue->i64 == 0)
            vm_err("Division by zero is undefined");

        vm_stack_push_int(lvalue->i64 / rvalue->i64, vm);

        break;

    // module
    case 5:
        if (rvalue->i64 == 0)
            vm_err("Division by zero is undefined");

        vm_stack_push_int(lvalue->i64 % rvalue->i64, vm);

        break;

    default:
        assert(0 && "Illegal arithmetic operation type value");
    }
}

void vm_execute_comparison(int type, VM *vm)
{
    Value *right = vm_stack_pop(vm);
    Value *left = vm_stack_pop(vm);

    Primitive *lvalue = NULL;
    Primitive *rvalue = NULL;

    if (!vm_is_value_int(left, &lvalue))
        vm_err("Failed to execute comparison. Left is not int type.");

    if (!vm_is_value_int(right, &rvalue))
        vm_err("Failed to execute comparison. Right is not int type.");

    switch (type)
    {
    // less
    case 1:
        vm_stack_push_bool(lvalue->i64 < rvalue->i64, vm);
        break;

    // greater
    case 2:
        vm_stack_push_bool(lvalue->i64 > rvalue->i64, vm);
        break;

    // less equals
    case 3:
        vm_stack_push_bool(lvalue->i64 <= rvalue->i64, vm);
        break;

    // greater equals
    case 4:
        vm_stack_push_bool(lvalue->i64 >= rvalue->i64, vm);
        break;

    // equals
    case 5:
        vm_stack_push_bool(lvalue->i64 == rvalue->i64, vm);
        break;

    // not equals
    case 6:
        vm_stack_push_bool(lvalue->i64 != rvalue->i64, vm);
        break;

    default:
        assert(0 && "Illegal comparison operation type value");
    }
}

void vm_execute_argjmp(int type, VM *vm)
{
    Value *value = vm_stack_pop(vm);
    Primitive *primitive = NULL;

    if (!vm_is_value_bool(value, &primitive))
        vm_err("Failed to execute conditional jump. Expect a bool popped from stack.");

    int64_t flag = primitive->i64;

    int jmp_value = vm_read_i32(vm);

    switch (type)
    {
        // jump if true
    case 1:
        if (flag == 1)
            vm_jmp(jmp_value, VM_FRAME_CURRENT(vm)->ip - 5, vm);
        break;

        // jump if false
    case 2:
        if (flag == 0)
            vm_jmp(jmp_value, VM_FRAME_CURRENT(vm)->ip, vm);
        break;

    default:
        assert(0 && "Illegal jump operation type value");
    }
}

void vm_execute_logical(int type, VM *vm)
{
    Value *right = vm_stack_pop(vm);
    Value *left = vm_stack_pop(vm);

    Primitive *lvalue = NULL;
    Primitive *rvalue = NULL;

    if (!vm_is_value_bool(left, &lvalue))
        vm_err("Failed to execute logical. Left is not value.");

    if (!vm_is_value_bool(right, &rvalue))
        vm_err("Failed to execute logical. Right is not value.");

    switch (type)
    {
    case 1:
        vm_stack_push_bool(lvalue->i64 || rvalue->i64, vm);
        break;

    case 2:
        vm_stack_push_bool(lvalue->i64 && rvalue->i64, vm);
        break;

    default:
        assert(0 && "Illegal logical operation type value");
    }
}

void vm_execute_negation(int type, VM *vm)
{
    Value *right = vm_stack_pop(vm);
    Primitive *value = NULL;

    switch (type)
    {
    case 1:
        if (!vm_is_value_bool(right, &value))
            vm_err("Failed to execute negation. Right is not bool.");

        vm_stack_push_bool(!value->i64, vm);

        break;

    case 2:
        if (!vm_is_value_int(right, &value))
            vm_err("Failed to execute negation. Right is not bool.");

        vm_stack_push_int(-value->i64, vm);

        break;

    default:
        assert(0 && "Illegal negation operation type");
    }
}

void vm_execute_shift(int type, VM *vm)
{
    Value *right_value = vm_stack_pop(vm);
    Value *left_value = vm_stack_pop(vm);

    Primitive *left_primitive = NULL;
    Primitive *right_primitive = NULL;

    if (!vm_is_value_int(left_value, &left_primitive))
        vm_err("Failed to execute shift. Expect int at left side, but got something else.");

    if (!vm_is_value_int(right_value, &right_primitive))
        vm_err("Failed to execute shift. Expect int at right side, but got something else.");

    int64_t left = left_primitive->i64;
    int64_t right = right_primitive->i64;

    switch (type)
    {
    case 1: // left shift
        vm_stack_push_int(left << right, vm);
        break;

    case 2: // right shift
        vm_stack_push_int(left >> right, vm);
        break;

    default:
        assert(0 && "Illegal shift operation type");
    }
}

void vm_execute_bitwise(int type, VM *vm)
{
    if (type >= 1 && type <= 3)
    {
        Value *right_value = vm_stack_pop(vm);
        Value *left_value = vm_stack_pop(vm);

        Primitive *left_primitive = NULL;
        Primitive *right_primitive = NULL;

        if (!vm_is_value_int(left_value, &left_primitive))
            vm_err("Failed to execute bitwise. Expect int at left side, but got something else.");

        if (!vm_is_value_int(right_value, &right_primitive))
            vm_err("Failed to execute bitwise. Expect int at right side, but got something else.");

        int64_t left = left_primitive->i64;
        int64_t right = right_primitive->i64;

        switch (type)
        {
        case 1: // or
            vm_stack_push_int(left | right, vm);
            break;

        case 2: // xor
            vm_stack_push_int(left ^ right, vm);
            break;

        case 3: // and
            vm_stack_push_int(left & right, vm);
            break;

        case 4: // not
            vm_stack_push_int(left | right, vm);
            break;

        default:
            assert(0 && "Illegal bitwise operation type");
        }
    }
    else if (type == 4)
    {
        Value *right_value = vm_stack_pop(vm);
        Primitive *right_primitive = NULL;

        if (!vm_is_value_int(right_value, &right_primitive))
            vm_err("Failed to execute bitwise. Expect int at right side, but got something else.");

        int64_t right = right_primitive->i64;

        vm_stack_push_int(~right, vm);
    }
    else
        assert(0 && "Illegal bitwise operation type");
}

void vm_execute_concat(VM *vm)
{
    Value *right = vm_stack_pop(vm);
    Value *left = vm_stack_pop(vm);

    if (!vm_is_value_string(left))
        vm_err("Failed to concat string. Left is not string type.");

    if (!vm_is_value_string(right))
        vm_err("Failed to concat string. Right is not string type.");

    String *lstr = &left->entity.object->value.string;
    String *rstr = &right->entity.object->value.string;

    size_t nstr_len = lstr->length + rstr->length;
    char *buff = vm_memory_alloc(nstr_len + 1);

    memcpy(buff, lstr->buffer, lstr->length);
    memcpy(buff + lstr->length, rstr->buffer, rstr->length);

    buff[nstr_len] = 0;

    Object *str_obj = vm_create_object(STR_OTYPE, vm);
    String *nstr = &str_obj->value.string;

    nstr->buffer = buff;
    nstr->length = nstr_len;

    vm_stack_push_object(str_obj, vm);
}

void vm_execute_length_str(VM *vm)
{
    Value *str_value = vm_stack_pop(vm);

    if (!vm_is_value_string(str_value))
        vm_err("Failed to get str length. Illegal type.");

    String *str = &str_value->entity.object->value.string;

    vm_stack_push_int((int64_t)str->length, vm);
}

void vm_execute_str_itm(VM *vm)
{
    Value *str_value = vm_stack_pop(vm);
    Value *index_value = vm_stack_pop(vm);
    Primitive *index_primitive = NULL;

    if (!vm_is_value_string(str_value))
        vm_err("Failed to get str character. Expect str, but got something else.");

    if (!vm_is_value_int(index_value, &index_primitive))
        vm_err("Failed to get str character. Illegal index type.");

    int64_t index = index_primitive->i64;
    String *str = &str_value->entity.object->value.string;

    if (index < 0 || (size_t)index >= str->length)
        vm_err("Failed to get str character. Constraints: 0 < index (%d) < str_len (%ld).", index, str->length);

    char *clone_char = vm_memory_alloc(2);

    clone_char[0] = str->buffer[index];
    clone_char[1] = 0;

    Object *str_obj = vm_create_object(STR_OTYPE, vm);
    String *new_str = &str_obj->value.string;

    new_str->buffer = clone_char;
    new_str->core = 0;
    new_str->length = 1;

    vm_stack_push_object(str_obj, vm);
}

void vm_execute_class(VM *vm)
{
    int32_t index = vm_read_i32(vm);

    Entity *klass_symbol = (Entity *)dynarr_get((size_t)index, vm->entities);
    Klass *klass = (Klass *)klass_symbol->raw_symbol;

    Object *instance_obj = vm_create_object(INSTANCE_OTYPE, vm);
    Instance *instance = &instance_obj->value.instance;

    instance->klass = klass;

    vm_stack_push_object(instance_obj, vm);
}

void vm_execute_get_property(VM *vm)
{
    char *key = vm_read_string(vm);
    size_t key_size = strlen(key);
    Value *instance_value = vm_stack_pop(vm);

    if (!vm_is_value_instance(instance_value))
        vm_err("Failed to access member of instance. Expect an instance, but got something else.");

    Object *instance_obj = instance_value->entity.object;
    Instance *instance = instance = &instance_obj->value.instance;

    Value *member = (Value *)lzhtable_get((uint8_t *)key, key_size, instance->attributes);

    if (member)
    {
        vm_stack_push_value((Value *)member, vm);
        return;
    }

    member = lzhtable_get((uint8_t *)key, key_size, instance->klass->methods);

    if (!member)
        vm_err("Failed to access instance member. '%s' does not contain '%s'.", instance->klass->name, key);

    Object *method_obj = vm_create_method(instance_obj, (Fn *)member, vm);

    vm_stack_push_object(method_obj, vm);
}

void vm_execute_set_property(VM *vm)
{
    char *key = vm_read_string(vm);
    size_t key_size = strlen(key);

    Value *instance_value = vm_stack_pop(vm);
    Value *input_value = vm_stack_peek(0, vm);

    if (!vm_is_value_instance(instance_value))
        vm_err("Failed to access member of instance. Expect an instance, but got something else.");

    Object *instance_obj = instance_value->entity.object;
    Instance *instance = instance = &instance_obj->value.instance;
    Value *value = lzhtable_get((uint8_t *)key, key_size, instance->attributes);

    if (!value)
        value = (Value *)vm_memory_calloc(sizeof(Value));

    memcpy(value, input_value, sizeof(Value));

    lzhtable_put((uint8_t *)key, key_size, value, instance->attributes, NULL);
}

void vm_execute_is(VM *vm)
{
    Value *obj_value = vm_stack_pop(vm);
    int8_t type = vm_advance(vm);

    switch (type)
    {
    case 0: // nil
        vm_stack_push_bool(vm_is_value_nil(obj_value), vm);
        break;

    case 1: // bool
        vm_stack_push_bool(vm_is_value_bool(obj_value, NULL), vm);
        break;

    case 2: // int
        vm_stack_push_bool(vm_is_value_int(obj_value, NULL), vm);
        break;

    case 3: // str
        vm_stack_push_bool(vm_is_value_string(obj_value), vm);
        break;

    case 4: // arr
        vm_stack_push_bool(vm_is_value_array(obj_value), vm);
        break;

    case 5: // function
        int is_fn = vm_is_value_fn(obj_value) ||
                    vm_is_value_native_fn(obj_value) ||
                    vm_is_value_method(obj_value);

        vm_stack_push_bool(is_fn, vm);

        break;

    case 6: // klass
        vm_stack_push_bool(vm_is_value_klass(obj_value), vm);
        break;

    case 7: // instance
        vm_stack_push_bool(vm_is_value_instance(obj_value), vm);
        break;

    default:
        break;
    }
}

void vm_execute_from(VM *vm)
{
    Value *value = vm_stack_pop(vm);
    char *klass_name = vm_read_string(vm);

    if (!vm_is_value_instance(value))
    {
        vm_stack_push_bool(0, vm);

        return;
    }

    Instance *instance = &value->entity.object->value.instance;
    Klass *klass = instance->klass;

    vm_stack_push_bool((uint8_t)strcmp(klass->name, klass_name) == 0, vm);
}

void vm_execute_this(VM *vm)
{
    Frame *frame = VM_FRAME_CURRENT(vm);
    Object *instance = frame->instance;

    if (!instance)
        vm_err("Failed to execute this. No instance in current frame");

    vm_stack_push_object(instance, vm);
}

void vm_execute_jmp(VM *vm)
{
    Frame *frame = VM_FRAME_CURRENT(vm);

    size_t current_ip = frame->ip - 1;
    int32_t jmp_value = vm_read_i32(vm);

    if (jmp_value == 0)
        return;

    if (jmp_value < 0)
    {
        if ((int)current_ip + jmp_value < 0)
            vm_err("Failed to execute jmp. Current ip %ld, plus jmp value %d, less than 0", current_ip, jmp_value);

        frame->ip = current_ip + jmp_value;
    }
    else
    {
        if (current_ip + jmp_value > frame->chunks->used)
            vm_err("Failed to execute jmp. Current ip %ld, plus jmp value %d, greater than chunks length %ld", current_ip, jmp_value, frame->chunks->used);

        frame->ip += jmp_value;
    }
}

void vm_execute_jmpc(VM *vm)
{
    Value *value = vm_stack_pop(vm);

    if (!vm_is_value_bool(value, NULL))
        vm_err("Filed to execute jmpc: expect a bool.");

    int32_t jmpc_value = vm_read_i32(vm);

    if (jmpc_value < 0)
        vm_err("Failed to execute jmpc: illegal jmpc value (negative): %d.", jmpc_value);

    Frame *frame = VM_FRAME_CURRENT(vm);
    size_t current_ip = frame->ip;

    if (current_ip + jmpc_value > frame->chunks->used)
        vm_err("Failed to execute jmpc: illegal jmpc value %d (%d): added to ip exceeds the number of chunks %d.", jmpc_value, current_ip + jmpc_value, frame->chunks->used);

    if (!value->entity.primitive.i64)
        frame->ip = current_ip + jmpc_value;
}

void vm_execute_get_local(VM *vm)
{
    int32_t index = (int32_t)vm_advance(vm);
    vm_frame_read((size_t)index, vm);
}

void vm_execute_set_local(VM *vm)
{
    int32_t index = (int32_t)vm_advance(vm);
    Value *value = vm_stack_peek(0, vm);

    vm_frame_write(index, value, vm);
}

void vm_execute_set_global(VM *vm)
{
    char *identifier = vm_read_string(vm);
    Value *value = vm_stack_peek(0, vm);

    vm_globals_write(identifier, value, vm);
}

void vm_execute_get_global(VM *vm)
{
    char *identifier = vm_read_string(vm);
    Value *value = (Value *)vm_globals_read(identifier, vm);

    vm_stack_push_value(value, vm);
}

void vm_execute_load_entity(VM *vm)
{
    int32_t index = vm_read_i32(vm);
    DynArr *entities = vm->entities;

    if ((size_t)index >= entities->used)
        vm_err("Failed to execute load function. Index %d out of bounds of %ld.", index, entities->used);

    Object *obj = NULL;
    Entity *entity = (Entity *)dynarr_get((size_t)index, vm->entities);

    switch (entity->type)
    {
    case FUNCTION_SYMTYPE:
        obj = vm_create_object(FN_OTYPE, vm);

        obj->value.fn = (Fn *)entity->raw_symbol;

        break;

    case NATIVE_SYMTYPE:
        obj = vm_create_object(NATIVE_FN_OTYPE, vm);

        obj->value.native_fn = (NativeFn *)entity->raw_symbol;

        break;

    case CLASS_SYMTYPE:
        obj = vm_create_object(CLASS_OTYPE, vm);

        obj->value.class = (Klass *)entity->raw_symbol;

        break;

    default:
        assert(0 && "Illegal entity type value");
    }

    vm_stack_push_object(obj, vm);
}

void vm_execute_print(VM *vm)
{
    Value *value = vm_stack_pop(vm);
    vm_print_value(value);
}

void vm_execute_pop(VM *vm)
{
    vm_stack_pop(vm);
}

void vm_execute_call(VM *vm)
{
    uint8_t args_count = vm_advance(vm);

    Value *callable_value = vm_stack_validate_callable(args_count, vm);
    Object *callable_obj = callable_value->entity.object;

    int setup = 0;

    Fn *callable = NULL;
    Object *frame_instance = NULL;
    int is_constructor = 0;

    if (vm_is_value_fn(callable_value))
    {
        setup = 1;
        callable = callable_obj->value.fn;
    }
    else if (vm_is_value_native_fn(callable_value))
    {
        NativeFn *native_fn = callable_obj->value.native_fn;

        if (args_count != native_fn->arity)
            vm_err("Failed to call callable. Expect %d arguments, but got %d.", native_fn->arity, args_count);

        DynArr *args = vm_stack_pop_args(args_count, vm);

        ((void (*)(DynArr *args, VM *vm))native_fn->raw_fn)(args, vm);

        vm_memory_destroy_dynarr(args);

        return;
    }
    else if (vm_is_value_method(callable_value))
    {
        Method *method = &callable_obj->value.method;

        callable = method->fn;
        frame_instance = method->instance;

        setup = 1;
    }
    else if (vm_is_value_klass(callable_value))
    {
        Klass *klass = callable_obj->value.class;
        Fn *constructor = klass->constructor;

        Object *instance_obj = vm_create_object(INSTANCE_OTYPE, vm);
        Instance *instance = &instance_obj->value.instance;

        instance->attributes = vm_memory_create_lzhtable(21);
        instance->klass = klass;

        if (constructor)
        {
            callable = klass->constructor;
            frame_instance = instance_obj;
            is_constructor = 1;

            setup = 1;
        }
        else
        {
            vm_stack_pop(vm); // the klass object
            vm_stack_push_object(instance_obj, vm);

            return;
        }
    }

    if (setup)
    {
        DynArrPtr *params = callable->params;

        if (params && args_count != params->used)
            vm_err("Failed to call callable. Expect %ld arguments, but got %d.", params->used, args_count);

        vm_frame_up(callable, frame_instance, is_constructor, vm);

        return;
    }

    vm_err("Failed to execute call. Expect a callable.");
}

void vm_execute_garbage(VM *vm)
{
    vm_gc(vm);
}

void vm_execute_return(VM *vm)
{
    if (vm->frame_ptr == 0)
    {
        vm->stop = 1;
        return;
    }

    vm_frame_down(vm);
}

void vm_execute_instruction(VM *vm)
{
    uint8_t opcode = vm_advance(vm);

    switch (opcode)
    {
    case NIL_OPC:
        // do not require to validate op
        vm_execute_nil(vm);
        break;

    case BCONST_OPC:
        vm_validate_opcode("BCONST", 1, vm);
        vm_execute_bool(vm);
        break;

    case ICONST_OPC:
        vm_validate_opcode("ICONST", 4, vm);
        vm_execute_int(vm);
        break;

    case SCONST_OPC:
        vm_validate_opcode("SCONST", 4, vm);
        vm_execute_string(vm);
        break;

    case ARR_OPC:
        vm_validate_opcode("OARR", 1, vm);
        vm_execute_array(vm);
        break;

    case ARR_LEN_OPC:
        // do not require to validate op
        vm_execute_array_length(vm);
        break;

    case ARR_ITM_OPC:
        // do not require to validate op
        vm_execute_get_array_item(vm);
        break;

    case ARR_SITM_OPC:
        // do not require to validate op
        vm_execute_set_array_item(vm);
        break;

    case LREAD_OPC:
        vm_validate_opcode("LREAD", 1, vm);
        vm_execute_get_local(vm);
        break;

    case LSET_OPC:
        vm_validate_opcode("LSET", 1, vm);
        vm_execute_set_local(vm);
        break;

    case GWRITE_OPC:
        vm_validate_opcode("GWRITE", 4, vm);
        vm_execute_set_global(vm);
        break;

    case GREAD_OPC:
        vm_validate_opcode("GREAD", 4, vm);
        vm_execute_get_global(vm);
        break;

    case LOAD_OPC:
        vm_validate_opcode("LOAD", 4, vm);
        vm_execute_load_entity(vm);
        break;

    // arithmetic
    case ADD_OPC:
        // do not require to validate op
        vm_execute_arithmetic(1, vm);
        break;

    case SUB_OPC:
        // do not require to validate op
        vm_execute_arithmetic(2, vm);
        break;

    case MUL_OPC:
        // do not require to validate op
        vm_execute_arithmetic(3, vm);
        break;

    case DIV_OPC:
        // do not require to validate op
        vm_execute_arithmetic(4, vm);
        break;

    case MOD_OPC:
        // do not require to validate op
        vm_execute_arithmetic(5, vm);
        break;

    // comparison
    case LT_OPC:
        // do not require to validate op
        vm_execute_comparison(1, vm);
        break;

    case GT_OPC:
        // do not require to validate op
        vm_execute_comparison(2, vm);
        break;

    case LE_OPC:
        // do not require to validate op
        vm_execute_comparison(3, vm);
        break;

    case GE_OPC:
        // do not require to validate op
        vm_execute_comparison(4, vm);
        break;

    case EQ_OPC:
        // do not require to validate op
        vm_execute_comparison(5, vm);
        break;

    case NE_OPC:
        // do not require to validate op
        vm_execute_comparison(6, vm);
        break;

    // logical
    case OR_OPC:
        // do not require to validate op
        vm_execute_logical(1, vm);
        break;

    case AND_OPC:
        // do not require to validate op
        vm_execute_logical(2, vm);
        break;

    case NOT_OPC:
        // do not require to validate op
        vm_execute_negation(1, vm);
        break;

    case NNOT_OPC:
        // do not require to validate op
        vm_execute_negation(2, vm);
        break;

    // shift
    case SLEFT_OPC:
        vm_execute_shift(1, vm);
        break;

    case SRIGHT_OPC:
        vm_execute_shift(2, vm);
        break;

    // bitwise
    case BOR_OPC:
        // do not require to validate op
        vm_execute_bitwise(1, vm);
        break;

    case BXOR_OPC:
        // do not require to validate op
        vm_execute_bitwise(2, vm);
        break;

    case BAND_OPC:
        // do not require to validate op
        vm_execute_bitwise(3, vm);
        break;

    case BNOT_OPC:
        // do not require to validate op
        vm_execute_bitwise(4, vm);
        break;

    // control flow
    case JMP_OPC:
        vm_validate_opcode("JMPC", 4, vm);
        vm_execute_jmp(vm);
        break;

    case JIT_OPC:
        vm_validate_opcode("JIT", 4, vm);
        vm_execute_argjmp(1, vm);
        break;

    case JIF_OPC:
        vm_validate_opcode("JIF", 4, vm);
        vm_execute_argjmp(2, vm);
        break;

    case CONCAT_OPC:
        // do not require to validate op
        vm_execute_concat(vm);
        break;

    case STR_LEN_OPC:
        // do not require to validate op
        vm_execute_length_str(vm);
        break;

    case STR_ITM_OPC:
        // do not require to validate op
        vm_execute_str_itm(vm);
        break;

    case CLASS_OPC:
        vm_validate_opcode("CLASS", 4, vm);
        vm_execute_class(vm);
        break;

    case GET_PROPERTY_OPC:
        vm_validate_opcode("GET_PROPERTY", 4, vm);
        vm_execute_get_property(vm);
        break;

    case IS_OPC:
        vm_validate_opcode("IS", 1, vm);
        vm_execute_is(vm);
        break;

    case FROM_OPC:
        vm_validate_opcode("FROM", 4, vm);
        vm_execute_from(vm);
        break;

    case SET_PROPERTY_OPC:
        vm_validate_opcode("GET_PROPERTY", 4, vm);
        vm_execute_set_property(vm);
        break;

    case THIS_OPC:
        // do not require to validate op
        vm_execute_this(vm);
        break;

    case PRT_OPC:
        // do not require to validate op
        vm_execute_print(vm);
        break;

    case POP_OPC:
        // do not require to validate op
        vm_execute_pop(vm);
        break;

    case CALL_OPC:
        vm_validate_opcode("CALL", 1, vm);
        vm_execute_call(vm);
        break;

    case GBG_OPC:
        // do not require to validate op
        vm_execute_garbage(vm);
        break;

    case RET_OPC:
        // do not require to validate op
        vm_execute_return(vm);
        break;

    case HLT_OPC:
        // do not require to validate op
        vm->halt = 1;
        break;

    default:
        vm_err("Illegal instruction: %d.", opcode);
        break;
    }
}

// public implementation
VM *vm_create()
{
    VM *vm = (VM *)vm_memory_alloc(sizeof(VM));

    vm->halt = 0;
    vm->stop = 0;
    vm->rtn_code = 0;

    vm->stack_ptr = 0;
    memset(vm->stack, 0, sizeof(Value) * VM_STACK_LENGTH);

    vm->frame_ptr = 0;
    memset(vm->frames, 0, sizeof(Frame) * VM_FRAME_LENGTH);

    vm->iconsts = vm_memory_create_dynarr(sizeof(int64_t));
    vm->strings = vm_memory_create_dynarr_ptr();
    vm->entities = vm_memory_create_dynarr(sizeof(Entity));
    vm->globals = vm_memory_create_lzhtable(1669);

    vm->blocks_stack = vm_memory_create_lzstack();
    vm->fn_def_stack = vm_memory_create_lzstack();
    vm->klass = NULL;

    vm->size = 0;
    vm->head_object = NULL;
    vm->tail_object = NULL;

    DynArr *default_chunks = vm_memory_create_dynarr(sizeof(uint8_t));

    lzstack_push((void *)default_chunks, vm->blocks_stack, NULL);

    Frame *frame = &vm->frames[0];

    frame->ip = 0;
    frame->chunks = default_chunks;

    //> adding natives
    vm_add_native("ascii", 1, native_fn_ascii, vm);
    vm_add_native("ascii_code", 2, native_fn_ascii_code, vm);
    vm_add_native("str_sub", 3, native_fn_str_sub, vm);
    vm_add_native("str_lower", 1, native_fn_str_lower, vm);
    vm_add_native("str_upper", 1, native_fn_str_upper, vm);
    vm_add_native("str_title", 1, native_fn_str_title, vm);
    vm_add_native("str_cmp", 2, native_fn_str_cmp, vm);
    vm_add_native("str_cmp_ic", 2, native_fn_str_cmp_ic, vm);
    vm_add_native("is_str_int", 1, native_fn_is_str_int, vm);
    vm_add_native("str_to_int", 1, native_fn_str_to_int, vm);
    vm_add_native("int_to_str", 1, native_fn_int_to_str, vm);

    vm_add_native("time", 0, native_fn_time, vm);
    vm_add_native("sleep", 0, native_fn_sleep, vm);

    vm_add_native("read_ln", 0, native_fn_read_line, vm);
    vm_add_native("read_file_bytes", 5, native_fn_read_file, vm);

    vm_add_native("panic", 1, native_fn_panic, vm);
    vm_add_native("exit", 1, native_fn_exit, vm);
    //< adding natives

    return vm;
}

void vm_destroy(VM *vm)
{
    if (!vm)
        return;

    //> cleaning up frame
    Frame *frame = &vm->frames[0];
    vm_memory_destroy_dynarr(frame->chunks);
    //< cleaning up frames

    //> cleaning up int constants
    vm_memory_destroy_dynarr(vm->iconsts);
    //< cleaning up int constants

    //> cleaning up strings
    size_t strings_length = vm->strings->used;

    for (size_t i = 0; i < strings_length; i++)
        vm_memory_dealloc(DYNARR_PTR_GET(i, vm->strings));

    vm_memory_destroy_dynarr_ptr(vm->strings);
    //< cleaning up strings

    //> cleaning up entities
    size_t functions_length = vm->entities->used;

    for (size_t i = 0; i < functions_length; i++)
    {
        Entity *entity = (Entity *)dynarr_get(i, vm->entities);

        if (entity->type == FUNCTION_SYMTYPE)
        {
            Fn *fn = (Fn *)entity->raw_symbol;
            vm_memory_destroy_fn(fn);
        }
        else if (entity->type == NATIVE_SYMTYPE)
        {
            NativeFn *native_fn = (NativeFn *)entity->raw_symbol;
            vm_memory_dealloc(native_fn->name);
            vm_memory_dealloc(native_fn);
        }
        else if (entity->type == CLASS_SYMTYPE)
        {
            Klass *klass = (Klass *)entity->raw_symbol;
            vm_memory_destroy_klass(klass);
        }
    }

    vm_memory_destroy_dynarr(vm->entities);
    //< cleaning up entities

    //> cleaning up globals
    LZHTableNode *node = vm->globals->nodes;

    while (node)
    {
        LZHTableNode *previous = node->previous_table_node;

        Value *value = (Value *)node->value;
        vm_memory_dealloc(value);

        node = previous;
    }

    vm_memory_destroy_lzhtable(vm->globals);
    //< cleaning up globals

    //> cleaning up helpers
    vm_memory_destroy_lzstack(vm->blocks_stack);
    vm_memory_destroy_lzstack(vm->fn_def_stack);
    //< cleaning up helpers

    //> cleaning up objects
    vm_garbage_objects(vm);
    //< cleaning up helpers

    vm->halt = 0;
    vm->stop = 0;
    vm->rtn_code = 0;

    vm->stack_ptr = 0;
    vm->frame_ptr = 0;

    vm->iconsts = NULL;
    vm->strings = NULL;
    vm->entities = NULL;
    vm->globals = NULL;

    vm->blocks_stack = NULL;
    vm->fn_def_stack = NULL;
    vm->klass = NULL;

    vm->size = 0;
    vm->head_object = NULL;
    vm->tail_object = NULL;

    vm_memory_dealloc(vm);
}

size_t vm_block_length(VM *vm)
{
    return ((DynArr *)lzstack_peek(vm->blocks_stack, NULL))->used;
}

void vm_print_stack(VM *vm)
{
    if (VM_STACK_SIZE(vm) == 0)
    {
        printf("STACK EMPTY\n");
        return;
    }

    for (int i = 0; i < VM_STACK_SIZE(vm); i++)
    {
        Value *value = &vm->stack[i];

        printf("%d: ", i + 1);
        vm_print_value(value);
    }
}

static void _fn_start_(EntityInfoType type, char *name, VM *vm)
{
    if (type == CONSTRUCTOR_ENTINFTYPE && vm->klass == NULL)
        vm_err("Trying to start class constructor definition, but klass not present");

    if (type == METHOD_ENTINFTYPE && vm->klass == NULL)
        vm_err("Trying to start class method definition, but klass not present");

    Fn *fn = vm_memory_create_fn(name);
    EntityInfo *entity_info = (EntityInfo *)vm_memory_alloc(sizeof(EntityInfo));

    entity_info->index = -1;
    entity_info->raw_entity = fn;
    entity_info->type = type;

    if (type == FUNCTION_ENTINFTYPE)
    {
        entity_info->index = vm->entities->used;
        dynarr_insert((void *)&((EntityInfo){0}), vm->entities);
    }

    lzstack_push((void *)fn->chunks, vm->blocks_stack, NULL);
    lzstack_push((void *)entity_info, vm->fn_def_stack, NULL);
}

static void _fn_end_(EntityInfoType type, VM *vm)
{
    if (type == CONSTRUCTOR_ENTINFTYPE && vm->klass == NULL)
        vm_err("Trying to end class constructor definition, but klass not present");

    if (type == METHOD_ENTINFTYPE && vm->klass == NULL)
        vm_err("Trying to end class method definition, but klass not present");

    EntityInfo *entity_info = (EntityInfo *)lzstack_pop(vm->fn_def_stack);

    if (!entity_info)
    {
        switch (type)
        {
        case FUNCTION_ENTINFTYPE:
            vm_err("Trying to end function definition, but stack is empty");
            break;

        case CONSTRUCTOR_ENTINFTYPE:
            vm_err("Trying to end class constructor definition, but stack is empty");
            break;

        case METHOD_ENTINFTYPE:
            vm_err("Trying to end class method definition, but stack is empty");
            break;

        default:
            assert("Illegal entity type");
        }
    }

    if (type != entity_info->type)
    {
        switch (type)
        {
        case FUNCTION_ENTINFTYPE:
            vm_err("Trying to end function definition, but popped incorrent type");
            break;

        case CONSTRUCTOR_ENTINFTYPE:
            vm_err("Trying to end class constructor definition, but popped incorrent type");
            break;

        case METHOD_ENTINFTYPE:
            vm_err("Trying to end class method definition, but popped incorrent type");
            break;

        default:
            assert("Illegal entity type");
        }
    }

    lzstack_pop(vm->blocks_stack);

    Fn *fn = (Fn *)entity_info->raw_entity;

    switch (type)
    {
    case FUNCTION_ENTINFTYPE:
        Entity entity = {0};

        entity.raw_symbol = fn;
        entity.type = FUNCTION_SYMTYPE;

        dynarr_set((void *)&entity, entity_info->index, vm->entities);

        break;

    case CONSTRUCTOR_ENTINFTYPE:
        vm->klass->constructor = fn;
        break;

    case METHOD_ENTINFTYPE:
        char *key = fn->name;
        size_t key_size = strlen(key);

        lzhtable_put((uint8_t *)key, key_size, (void *)fn, vm->klass->methods, NULL);

        break;

    default:
        assert("Illegal entity type");
    }

    vm_memory_dealloc(entity_info);
}

static void _fn_add_param_(EntityInfoType type, char *param_name, VM *vm)
{
    if (type == CONSTRUCTOR_ENTINFTYPE && vm->klass == NULL)
        vm_err("Trying to add parameter to class constructor, but klass not present");

    if (type == METHOD_ENTINFTYPE && vm->klass == NULL)
        vm_err("Trying to add parameter to class method, but klass not present");

    EntityInfo *entity_info = (EntityInfo *)lzstack_peek(vm->fn_def_stack, NULL);

    if (!entity_info)
    {
        switch (type)
        {
        case FUNCTION_ENTINFTYPE:
            vm_err("Trying to add parameter to function, but stack is empty");
            break;

        case CONSTRUCTOR_ENTINFTYPE:
            vm_err("Trying to add parameter to class constructor, but stack is empty");
            break;

        case METHOD_ENTINFTYPE:
            vm_err("Trying to add parameter to class method, but stack is empty");
            break;

        default:
            assert("Illegal entity type");
        }
    }

    if (type != entity_info->type)
    {
        switch (type)
        {
        case FUNCTION_ENTINFTYPE:
            vm_err("Trying to add parameter to function, but popped incorrent type");
            break;

        case CONSTRUCTOR_ENTINFTYPE:
            vm_err("Trying to add parameter to class constructor, but popped incorrent type");
            break;

        case METHOD_ENTINFTYPE:
            vm_err("Trying to add parameter to class method, but popped incorrent type");
            break;

        default:
            assert("Illegal entity type");
        }
    }

    Fn *fn = (Fn *)entity_info->raw_entity;

    dynarr_ptr_insert((void *)vm_memory_clone_string(param_name), fn->params);
}

void vm_fn_start(char *name, VM *vm)
{
    _fn_start_(FUNCTION_ENTINFTYPE, name, vm);
}

void vm_fn_end(VM *vm)
{
    _fn_end_(FUNCTION_ENTINFTYPE, vm);
}

void vm_fn_add_param(char *name, VM *vm)
{
    _fn_add_param_(FUNCTION_ENTINFTYPE, name, vm);
}

void vm_klass_start(char *name, VM *vm)
{
    Klass *klass = vm_memory_create_klass(name);

    if (!klass)
        vm_err("Trying to start class definition, but class definition present");

    vm->klass = klass;
}

void vm_klass_end(VM *vm)
{
    Klass *klass = vm->klass;

    if (!klass)
        vm_err("Trying to end class definition, but not class definition present");

    Entity symbol = {0};

    symbol.type = CLASS_SYMTYPE;
    symbol.raw_symbol = (void *)klass;

    dynarr_insert((void *)&symbol, vm->entities);

    vm->klass = NULL;
}

void vm_klass_constructor_start(VM *vm)
{
    _fn_start_(CONSTRUCTOR_ENTINFTYPE, "constructor", vm);
}

void vm_klass_constructor_end(VM *vm)
{
    _fn_end_(CONSTRUCTOR_ENTINFTYPE, vm);
}

void vm_klass_constructor_add_param(char *name, VM *vm)
{
    _fn_add_param_(CONSTRUCTOR_ENTINFTYPE, name, vm);
}

void vm_klass_fn_start(char *name, VM *vm)
{
    _fn_start_(METHOD_ENTINFTYPE, name, vm);
}

void vm_klass_fn_end(VM *vm)
{
    _fn_end_(METHOD_ENTINFTYPE, vm);
}

void vm_klass_fn_add_param(char *name, VM *vm)
{
    _fn_add_param_(METHOD_ENTINFTYPE, name, vm);
}

size_t vm_write_chunk(uint8_t chunk, VM *vm)
{
    DynArr *chunks = vm_current_chunks(vm);
    size_t index = chunks->used;

    dynarr_insert(&chunk, chunks);

    return index;
}

size_t vm_write_i32(int32_t value, VM *vm)
{
    DynArr *chunks = vm_current_chunks(vm);
    size_t index = chunks->used;

    uint8_t bytes[4];

    vm_descompose_i32(value, bytes);

    for (size_t i = 0; i < 4; i++)
        vm_write_chunk(bytes[i], vm);

    return index;
}

void vm_update_i32(size_t index, int32_t value, VM *vm)
{
    DynArr *chunks = vm_current_chunks(vm);

    uint8_t bytes[4];

    vm_descompose_i32(value, bytes);

    for (size_t i = 0; i < 4; i++)
        dynarr_set((void *)&bytes[i], index + i, chunks);
}

size_t vm_write_bool_const(uint8_t value, VM *vm)
{
    return vm_write_chunk(value, vm);
}

size_t vm_write_i64_const(int64_t value, VM *vm)
{
    DynArr *chunks = vm_current_chunks(vm);
    size_t index = chunks->used;

    size_t constant_index = vm->iconsts->used;

    dynarr_insert(&value, vm->iconsts);
    vm_write_i32((int32_t)constant_index, vm);

    return index;
}

size_t vm_write_str_const(char *value, VM *vm)
{
    DynArr *chunks = vm_current_chunks(vm);
    size_t index = chunks->used;

    size_t constant_index = vm->strings->used;
    char *clone_string = vm_memory_clone_string(value);

    dynarr_ptr_insert(clone_string, vm->strings);
    vm_write_i32((int32_t)constant_index, vm);

    return index;
}

int vm_execute(VM *vm)
{
    while (!vm_is_at_end(vm))
        vm_execute_instruction(vm);

    if (!vm->halt && !vm->stop && vm->frame_ptr != 0)
        vm_err("Illegal virtual machine end state. The virtual machine must end its execution in the main frame.");

    vm->halt = 0;

    if (vm->stop)
    {
        vm->stop = 0;
        vm->stack_ptr = 0;
        vm->frame_ptr = 0;
    }

    return vm->rtn_code;
}