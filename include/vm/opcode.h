#ifndef _OPCODE_H_
#define _OPCODE_H_

typedef enum _opcode_
{
    NIL_OPC,    // push nil to stack
    BCONST_OPC, // push an bool value to stack
    ICONST_OPC, // push an int value to stack
    SCONST_OPC, // push a str value to stack

    ARR_OPC,      // creates an array
    ARR_LEN_OPC,  // gets the length of an array
    ARR_ITM_OPC,  // gets a item from a array
    ARR_SITM_OPC, // sets a value to a array item

    LREAD_OPC, // get a local variable (push to stack)
    LSET_OPC,  // set a value to a local variable

    GWRITE_OPC, // creates a global variable
    GREAD_OPC,  // gets a global variable

    LOAD_OPC, // load an entity

    // arithmetic
    ADD_OPC,
    SUB_OPC,
    MUL_OPC,
    DIV_OPC,
    MOD_OPC,

    // comparison
    LT_OPC, // less
    GT_OPC, // greater
    LE_OPC, // less equals
    GE_OPC, // greater equals
    EQ_OPC, // equals
    NE_OPC, // not equals

    // logical
    OR_OPC,
    AND_OPC,
    NOT_OPC,
    NNOT_OPC,

    // shift
    SLEFT_OPC,
    SRIGHT_OPC,

    // bitwise
    BOR_OPC,
    BXOR_OPC,
    BAND_OPC,
    BNOT_OPC,

    // control flow
    JMP_OPC, // unconditional jump
    JIT_OPC, // jump if true
    JIF_OPC, // jump if false

    CONCAT_OPC,  // join two strings
    STR_LEN_OPC, // length of a string
    STR_ITM_OPC, // gets string char

    CLASS_OPC,
    THIS_OPC,
    SET_PROPERTY_OPC,
    GET_PROPERTY_OPC,

    IS_OPC,
    FROM_OPC,

    // others
    PRT_OPC,  // prints a value from the stack
    POP_OPC,  // pops a value from the stack
    CALL_OPC, // calls a function
    GBG_OPC,  // Asks he vm to garbage objects
    RET_OPC,
    HLT_OPC
} Opcode;

#endif